/* usbplayd - a simple user space audio player
 * Copyright (C) 2016 - Felix Schmidt
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <getopt.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include <libmaru.h>
#include <samplerate.h>

#define MAX_PFDS 10

/* Size of input buffer
 */
#define BUF_SIZE    (32*1024)

/* samples in input buffer (16 bit, 2 channels)
 */
#define BUF_SAMPLES (BUF_SIZE / 4)

/* Max. output-to-input ratio
 */
#define MAX_RATIO   4

/* Volume parameters
 */
maru_volume		     vcur, vmin, vmax;
int			     current_volume;
int			     last_mod = 0;
time_t			     last_check;

/* libmaru device state
 */
maru_context		    *ctx = NULL;
struct maru_audio_device    *list = NULL;
struct maru_stream_desc	    *desc = NULL;
int			     stream = -1;

/* parameters
 */
int			     req_dev = -1;
int			     req_stream = -1;
int			     req_volume = -1;
char			    *volfile = NULL;
int			     converter_type = SRC_SINC_FASTEST;

/* Runtime data
 */
char			    *pfiles[MAX_PFDS];
int			     input_rates[MAX_PFDS];
struct			     pollfd pfd[MAX_PFDS];
nfds_t			     nfds = 0;
int			     output_rate = 0;
int			     src_errcode = 0;
FILE			    *log;

const char *usage =
"usbplayd options    A libusb audio player daemon\n"
" -d device#       : USB device number (0..n)\n"
" -s stream#       : Stream number (0..n)\n"
" -v volume-level  : Static volume level (0..100)\n"
" -V volume-file   : Poll volume level from file and apply it to USB device\n"
" -P pipe:rate     : Read audio data from fifo, assuming given\n"
"                    sample rate (rate conversion if necessary)\n"
"                    The -P argument can be repeated. Player will read\n"
"                    all FIFOs, but play only the last one where data\n"
"                    is available.\n"
" -r rate          : Try to use given output rate. Use default device\n"
"                    rate by default.\n"
" -c resample-algo : If device and FIFO rate differ, use libsamplerate\n"
"                    to perform rate conversion, using given algorithm:\n"
"		     0 - SRC_SINC_BEST_QUALITY\n"
"		     1 - SRC_SINC_MEDIUM_QUALITY\n"
"		     2 - SRC_SINC_FASTEST (default)\n"
"		     3 - SRC_ZERO_ORDER_HOLD\n"
"		     4 - SRC_LINEAR\n"
" -D               : Run in daemon mode\n"
" -l               : List USB audio devices and exit\n"
" -L logfile	   : log to file instead of stderr\n"
"\n"
" Example:\n"
" usbplayd -D -V /tmp/volfile -P /tmp/mpd.fifo:48000 -P /tmp/shairport.fifo:44100\n"
;

void list_devices(void)
{
    struct maru_audio_device *list;
    unsigned num_devices;
    maru_context *ctx;
    maru_volume cur, min, max;
    int stream;
    unsigned num_desc;
    struct maru_stream_desc *desc;
    int i, j;

    assert (maru_list_audio_devices (&list, &num_devices) == LIBMARU_SUCCESS);

    printf ("Found %u devices!\n", num_devices);

    if (num_devices == 0)
	return;

    for (i = 0; i < num_devices; i++)
    {
	printf ("Device %u: VID: 0x%04x, PID: 0x%04x\n", i,
		(unsigned) list[i].vendor_id, (unsigned) list[i].product_id);

	assert (maru_create_context_from_vid_pid (&ctx, list[i].vendor_id,
						  list[i].product_id,
						  &(const struct
						    maru_stream_desc)
						  {
						  .channels = 2,.bits =
						  16}) == LIBMARU_SUCCESS);

	if (maru_stream_get_volume (ctx, LIBMARU_STREAM_MASTER, &cur, &min,
				    &max, 5000000) == LIBMARU_SUCCESS)
	{
	    printf (" Volume: %d, Min: %d, Max: %d\n", cur / 256,
		     min / 256, max / 256);
	}

	printf (" Streams: %d\n", maru_get_num_streams (ctx));

	for (stream = 0; stream < maru_get_num_streams (ctx); stream++)
	{
	    printf (" Stream %d:\n", stream);

	    if (!maru_is_stream_available (ctx, stream))
	    {
		printf ("  Not available\n");
		continue;
	    }

	    if (maru_get_stream_desc (ctx, stream, &desc, &num_desc) != LIBMARU_SUCCESS)
	    {
		printf ("  Failed to get stream descriptor\n");
		continue;
	    }

	    for (j = 0; j < num_desc; j++)
	    {
		printf ("  Descriptor %d:\n", j);
		printf ("   Rate:     %u\n", desc[j].sample_rate);
		printf ("   Channels: %u\n", desc[j].channels);
		printf ("   Bits:     %u\n", desc[j].bits);
	    }
	    free (desc);
	}
    }

    maru_destroy_context (ctx);

    free (list);
}


/* Convert volume level (0..100) to device level
 */
int dev_volume (int vol)
{
    float fvol;

    if (vol < 0)
	return vmin;

    if (vol >= 100)
	return vmax;

    /* convert to range of device
     */
    fvol = (float)(vmin) + ((float)vol / 100.0) * (float)(vmax - vmin) + 0.499;

    return (int)fvol;
}

/* Set hardware volume (0..100)
 */
void set_hardware_volume (int vol)
{
    int err;

    if (!ctx)
	return;

    /* convert to range of device
     */
    vol = dev_volume (vol);

    if (vol == vcur)
	return;

    /* adjust volume in hardware
     */
    err =
	maru_stream_set_volume (ctx, LIBMARU_STREAM_MASTER, vol, 5000000);

    if (err != LIBMARU_SUCCESS)
    {
	fprintf (log, "Failed to set volume to %d\n", vol);
    }

    vcur = vol;
}

/* Check for volume file and apply to hardware if file has changed and content
 * differs to current setting
 */
void handle_volctl ()
{
    struct stat st;
    FILE *fh;
    int vol;

    /* volfile configured ?
     */
    if (!volfile)
	return;

#if 0
    struct timeval tv;
    /* poll only once per second
     */
    if (gettimeofday (&tv, NULL) == 0)
    {
	if (tv.tv_sec == last_check)
	    return;
	last_check = tv.tv_sec;
    }
#endif

    /* check if it exists and has been modified
     */
    if (stat (volfile, &st))
	return;

    if (st.st_mtime == last_mod)
	return;
 
    /* read it
     */
    fh = fopen (volfile, "r");
    if (!fh)
	return;

    if (fscanf (fh, "%d", &vol) != 1)
    {
	fclose (fh);
	return;
    }

    fclose (fh);

    last_mod = st.st_mtime;

    current_volume = vol;

    set_hardware_volume (current_volume);
}


/* Stop USB device
 */
void maru_stop(void)
{
    if (ctx)
    {
	if (stream != -1)
	    maru_stream_close (ctx, stream);
	
	maru_destroy_context (ctx);
    }

    if (desc)
	free (desc);

    if (list)
	free (list);

    list = NULL;
    desc = NULL;
    ctx = NULL;
    stream = -1;
}

/* Start USB device
 */
int maru_start(void)
{
    unsigned num_devices;
    int err;

    /* nop if already initialized
     */
    if (ctx)
	return 0;

    if (maru_list_audio_devices (&list, &num_devices) != LIBMARU_SUCCESS)
	return 1;

    if (num_devices == 0)
    {
	fprintf (log, "No USB audio devices found\n");
	return 1;
    }

    if (req_dev >= num_devices)
    {
	fprintf (log, "Requested device index %d, but only %d available\n", 
	    req_dev, num_devices);
	return 1;
    }

    if (maru_create_context_from_vid_pid (&ctx, list[req_dev].vendor_id,
					      list[req_dev].product_id,
					      &(const struct
						maru_stream_desc)
					      {
					      .channels = 2,.bits =
					      16}) != LIBMARU_SUCCESS)
    {
	return 1;
    }

    err =
	maru_stream_get_volume (ctx, LIBMARU_STREAM_MASTER, &vcur, &vmin,
				&vmax, 5000000);

    if (err != LIBMARU_SUCCESS)
    {
	fprintf (log, "usbplay: failed to get volume limits\n");
	volfile = NULL;
	vmin = -50*256;
	vmax = 0;
	req_volume = -1;
    }

    if ((req_volume != -1) && (volfile == NULL))
    {
	req_volume = dev_volume (req_volume);

	err =
	    maru_stream_set_volume (ctx, LIBMARU_STREAM_MASTER, req_volume, 5000000);

	if (err != LIBMARU_SUCCESS)
	{
	    fprintf (log, "Failed to set volume to %d\n", req_volume/256);
	}
    }

    if (req_stream != -1)
    {
	if (req_stream >= maru_get_num_streams (ctx))
	{
	    fprintf (log, "Requested stream %d, only %d available\n", 
		req_stream, maru_get_num_streams (ctx));

	    maru_stop();

	    return 1;
	}
	stream = req_stream;
    }
    else
    {
	stream = maru_find_available_stream (ctx);

	if (stream < 0)
	{
	    fprintf (log, "No Available stream\n");
	    maru_stop();
	    return 1;
	}
    }

    if (!maru_is_stream_available (ctx, stream))
    {
	fprintf (log, "Stream %d not available\n", stream);
	maru_stop();
	return 1;
    }

    printf ("Using stream %d\n", stream);

    unsigned num_desc;
    if (maru_get_stream_desc (ctx, stream, &desc, &num_desc) !=
	    LIBMARU_SUCCESS)
    {
	maru_stop();
	return 1;
    }

    if (output_rate)
	desc[0].sample_rate = output_rate;

    fprintf (log, "Format:\n");
    fprintf (log, "\tRate: %u\n", desc[0].sample_rate);
    fprintf (log, "\tChannels: %u\n", desc[0].channels);
    fprintf (log, "\tBits: %u\n", desc[0].bits);

    desc[0].buffer_size = 1024 * 128;
    desc[0].fragment_size = 1024 * 32;

    err = maru_stream_open (ctx, stream, desc);

    if (err != LIBMARU_SUCCESS)
    {
	fprintf (log, "Failed to open stream\n");
	maru_stop();
	return 1;
    }

    if (!output_rate)
	output_rate = desc[0].sample_rate;

    return 0;
}

int open_or_create_fifo(char *name, int i)
{
    int fd;

    if (i >= MAX_PFDS)
	return -1;

    if (!pfiles[i] && name)
	pfiles[i] = strdup(name);

    if (!pfiles[i])
	return -1;

    fd = open (name, O_RDONLY|O_NONBLOCK);
    if (fd == -1)
    {
	if (-1 == mkfifo(name, 0666))
	{
	    perror (name);
	    return -1;
	}

	fd = open (name, O_RDONLY|O_NONBLOCK);
	if (fd == -1)
	{
	    perror (name);
	    return -1;
	}
    }
    fchmod (fd, 0666);

    pfd[i].fd = fd;

    return fd;
}

/* Remove an input pipe from list of pipes
 */
void remove_pipe(int index)
{
    int i;

    if (index >= nfds)
	return;

    close (pfd[index].fd);

    for (i = index; i < nfds-1; i++)
    {
	pfd[i] = pfd[i+1];
	pfiles[i] = pfiles[i+1];
	input_rates[i] = input_rates[i+1];
    }
    nfds--;
}

/* convert sample rate of buffer, using given ratio
 * Assume that buffer is large enough for given ratio
 */
int convert_samplerate (SRC_STATE *src_state,
    char *buf, 
    int samples,
    double ratio)
{
    SRC_DATA src_data;
    static float data_in[BUF_SAMPLES * 2];
    static float data_out[BUF_SAMPLES * 2 * MAX_RATIO];
    int rc;

    src_data.data_in = data_in;
    src_data.data_out = data_out;
    src_data.input_frames = samples;
    src_data.output_frames = BUF_SAMPLES * MAX_RATIO;
    src_data.src_ratio = ratio;
    src_data.end_of_input = 0;

    src_short_to_float_array ((const short*)buf, src_data.data_in, src_data.input_frames*2);

    if ((rc = src_process (src_state, &src_data)) != 0)
    {
	fprintf (log, "src_process failed with error %d (%s)\n",
	    rc,
	    src_strerror(rc));

	return 0;
    }

    src_float_to_short_array (src_data.data_out, (short*)buf, src_data.output_frames_gen*2);

    return src_data.output_frames_gen;
}

int
main (int argc, char **argv)
{
    struct timeval tv, prev_tv, start_tv;
    int fd;
    int i;
    int current_input;
    int written = 0;
    int daemon_mode = 0;
    char *ratespec;
    SRC_STATE *src_state;
    double ratio;
    int do_rate_convert = 0;
    char fifo_name[100];
    int rate;
    char *logfile;

    int c;

    memset (pfiles, 0, sizeof(pfiles));
    memset (pfd, 0, sizeof(pfd));
    log = stderr;

    while (1)
    {
	int option_index = 0;
	static struct option long_options[] = {
	    {"device", required_argument, 0, 'd'},
	    {"stream", required_argument, 0, 's'},
	    {"volume", required_argument, 0, 'v'},
	    {"volfile", required_argument, 0, 'V'},
	    {"rate", required_argument, 0, 'r'},
	    {"pipe", required_argument, 0, 'P'},
	    {"converter", required_argument, 0, 'c'},
	    {"logfile", required_argument, 0, 'L'},
	    {"daemon", no_argument, 0, 'D'},
	    {"help", no_argument, 0, 'h'},
	    {"list", no_argument, 0, 'l'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "lr:Dhd:s:v:V:P:c:L:", long_options, &option_index);

	if (c == -1)
	    break;

	switch (c)
	{
	case 'L':
	    logfile = strdup (optarg);
	    break;

	case 'l':
	    list_devices();
	    return 0;
	case 'c':
	    converter_type = atoi (optarg);

	    if ((converter_type < 0) || (converter_type > 4))
	    {
		fprintf (stderr, usage);
		exit (1);
	    }

	    break;

	case 'D':
	    daemon_mode = 1;
	    break;

	case 'P':
	    if (nfds >= MAX_PFDS)
	    {
		fprintf (log, "max. %d input pipes\n", MAX_PFDS);
		exit(1);
	    }

	    strncpy (fifo_name, optarg, sizeof(fifo_name));
	    ratespec = strchr (fifo_name, ':');

	    if (ratespec)
	    {
		ratespec[0] = '\0';
		rate = atoi(&ratespec[1]);
	    }
	    else
	    {
		rate = 0;
	    }

	    fd = open_or_create_fifo (fifo_name, nfds);

	    if (fd ==-1)
		exit (1);

	    input_rates[nfds] = rate;

	    nfds++;

	    break;

	case 'd':
	    req_dev = atoi(optarg);
	    break;
	
	case 's':
	    req_stream = atoi(optarg);
	    break;

	case 'v':
	    req_volume = atoi(optarg);
	    break;

	case 'V':
	    volfile = optarg;
	    break;

	case 'h':
	    printf (usage);
	    exit (0);

	case 'r':
	    rate = atoi(optarg);
	    break;

	default:
	    fprintf (stderr, usage);
	    exit (1);
	}
    }

    if (nfds == 0)
    {
	fprintf (log, "no input pipes defined\n");
	exit (1);
    }

    for (i = 0; i < nfds; i++)
	pfd[i].events = POLLIN;

    if (req_dev == -1)
	req_dev = 0;

    gettimeofday (&start_tv, NULL);
    prev_tv = start_tv;

    if (daemon_mode)
    {
	if (daemon (0, 0))
	{
	    perror ("daemon");
	    exit (1);
	}
    }

    if (logfile)
    {
	log = fopen (logfile, "a");
	if (!log)
	    log = stderr;
	setvbuf (log, NULL, _IONBF, 0);
    }

    src_state = src_new (converter_type, 2, &src_errcode);

    current_input = -1;

    /* Main loop
     */
    for (;;)
    {
	ssize_t ret;
	int read_size;
	int highest_input;
	char buf[BUF_SIZE * MAX_RATIO];

	for (i = 0; i < nfds; i++)
	    pfd[i].revents = 0;

	if ((i = poll(pfd, nfds, -1)) < 0)
	{
	    perror ("poll");
	    sleep (2);
	    continue;
	}

	if (i == 0)
	    continue;

	/* read from all input streams, ascending priority
	 */
	read_size = 0;
	highest_input = -1;
	for (i = 0; i < nfds; i++)
	{
	    if (!pfd[i].revents)
		continue;

	    if (pfd[i].revents & (POLLERR|POLLHUP|POLLNVAL))
	    {
		/* reopen/create pipe if writer has closed it, or some
		 * other error
		 */
		close (pfd[i].fd);
		if (-1 == open_or_create_fifo (pfiles[i], i))
		{
		    /* remove this pipe from the list if this fails
		     * and exit if no more pipes
		     */
		    remove_pipe (i);
		    if (nfds == 0)
		    {
			fprintf (log, "no more pipes\n");
			exit (1);
		    }
		    continue;
		}

		ret = 0;
	    }
	    else
	    {
		ret = read (pfd[i].fd, buf, BUF_SIZE);
	    }

	    if (ret <= 0)
	    {
		/* EOF/error on input
		 */
		if (i == current_input)
		{
		    fprintf (log, "current input %d(%s) closed\n", i, pfiles[i]);
		    current_input = -1;
		}
	    }
	    else
	    {
		read_size = ret;
		highest_input = i;
	    }
	}

	if (highest_input == -1)
	{
	    /* close usb device if currently no writer on any pipe
	     */
	    if (ctx)
	    {
		fprintf (log, "No writers, closing USB device\n");
		maru_stop();
	    }

	    continue;
	}

	if (highest_input > current_input)
	{
	    /* start audio device if necessary
	     */
	    if (maru_start())
	    {
		sleep (1);
		continue;
	    }

	    if (input_rates[highest_input] == output_rate)
	    {
		do_rate_convert = 0;
	    }
	    else if (output_rate > MAX_RATIO*input_rates[highest_input])
	    {
		/* output/input rate too high, ignore this stream
		 */
		fprintf (log, "input rate %d of stream %s too low relative to output rate %d (min is output/%d)\n",
			input_rates[highest_input],
			pfiles[highest_input],
			output_rate,
			MAX_RATIO);

		remove_pipe (highest_input);
		if (nfds == 0)
		{
		    fprintf (log, "no more pipes\n");
		    exit (1);
		}
		continue;
	    }
	    else
	    {
		do_rate_convert = 1;
		ratio = (double)output_rate / (double)input_rates[highest_input];
	    }

	    src_reset (src_state);

	    current_input = highest_input;

	    fprintf (log, "switching to input %d(%s)\n",
		current_input, pfiles[current_input]);
	}
	else if (highest_input < current_input)
	{
	    /* drop data low-pri FIFOs as long as current FIFO has a writer
	     */
	    continue;
	}

	/* handle volume file changes and apply them if required
	 */
	handle_volctl (volfile);

	/* convert sample rate if required
	 */
	if (do_rate_convert)
	{
	    read_size = convert_samplerate (src_state, buf, read_size / 4, ratio) * 4;

	    if (read_size == 0)
		continue;
	}

	/* play data
	 */
	if (maru_stream_write (ctx, stream, buf, read_size) < read_size)
	{
	    fprintf (log, "maru_stream_write() failed\n");
	    continue;
	}

	/* print some stats
	 * FIXME: This is currently required! Otherwise no data played. Seems to
	 * be some race condition
	 */
	maru_stream_write_avail (ctx, stream);
	maru_stream_current_latency (ctx, stream);

	written += read_size;
	gettimeofday (&tv, NULL);
	if (prev_tv.tv_sec != tv.tv_sec)
	{
	    printf ("[%d] latency: %d written: %d avail: %d\n",
		    (int) (tv.tv_sec - start_tv.tv_sec),
		    (int) maru_stream_current_latency (ctx, stream),
		    written, (int) maru_stream_write_avail (ctx, stream));

	    fflush (stdout);
	    prev_tv = tv;
	    written = 0;
	}
	else
	{
	    printf ("[%d] latency: %d written: %d avail: %d\n",
		    0, 0, 0, 0);
	}
    }

    maru_stop();

    return 0;
}
