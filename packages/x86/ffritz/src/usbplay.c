#include <libmaru.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <getopt.h>



int
main (int argc, char **argv)
{
    struct maru_audio_device *list;
    unsigned num_devices;
    struct timeval tv, prev_tv, start_tv;
    int seconds = 0;
    int req_dev = -1;
    int req_stream = -1;
    int req_volume = 0;
    maru_volume cur, min, max;
    int stream;
    int err;
    maru_context *ctx;

    int c;
    int digit_optind = 0;

    while (1)
    {
	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
	    {"list", required_argument, 0, 'l'},
	    {"device", required_argument, 0, 'd'},
	    {"stream", required_argument, 0, 's'},
	    {"volume", required_argument, 0, 'v'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "ld:s:v:", long_options, &option_index);

	if (c == -1)
	    break;

	switch (c)
	{
	case 'l':
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
	}
    }

    if (req_dev == -1)
	req_dev = 0;

    assert (maru_list_audio_devices (&list, &num_devices) == LIBMARU_SUCCESS);

    if (num_devices == 0)
    {
	fprintf (stderr, "No USB audio devices found\n");
	return 1;
    }

    if (req_dev >= num_devices)
    {
	fprintf (stderr, "Requested device index %d, but only %d available\n", 
	    req_dev, num_devices);
	return 1;
    }

    assert (maru_create_context_from_vid_pid (&ctx, list[req_dev].vendor_id,
					      list[req_dev].product_id,
					      &(const struct
						maru_stream_desc)
					      {
					      .channels = 2,.bits =
					      16}) == LIBMARU_SUCCESS);

    err =
	maru_stream_get_volume (ctx, LIBMARU_STREAM_MASTER, &cur, &min,
				&max, 5000000);

    if (err == LIBMARU_SUCCESS)
    {
	if (req_volume > max / 256)
	    req_volume = max / 256;

	err =
	    maru_stream_set_volume (ctx, LIBMARU_STREAM_MASTER, req_volume * 256, 5000000);

	if (err != LIBMARU_SUCCESS)
	{
	    fprintf (stderr, "Failed to set volume to %d\n", req_volume);
	}
    }

    if (req_stream != -1)
    {
	if (req_stream >= maru_get_num_streams (ctx))
	{
	    fprintf (stderr, "Requested stream %d, only %d available\n", 
		req_stream, maru_get_num_streams (ctx));
	    return 1;
	}
	stream = req_stream;
    }
    else
    {
	stream = maru_find_available_stream (ctx);

	if (stream < 0)
	{
	    fprintf (stderr, "No Available stream\n");
	    return 1;
	}
    }

    if (!maru_is_stream_available (ctx, stream))
    {
	fprintf (stderr, "Stream %d not available\n", stream);
	return 1;
    }

    printf ("Using stream %d\n", stream);

    unsigned num_desc;
    struct maru_stream_desc *desc;
    assert (maru_get_stream_desc (ctx, stream, &desc, &num_desc) ==
	    LIBMARU_SUCCESS);

    fprintf (stderr, "Format:\n");
    fprintf (stderr, "\tRate: %u\n", desc[0].sample_rate);
    fprintf (stderr, "\tChannels: %u\n", desc[0].channels);
    fprintf (stderr, "\tBits: %u\n", desc[0].bits);

    desc[0].buffer_size = 1024 * 128;
    desc[0].fragment_size = 1024 * 32;

    err = maru_stream_open (ctx, stream, desc);

    if (err != LIBMARU_SUCCESS)
    {
	fprintf (stderr, "Failed to open stream\n");
	return 1;
    }

    int written = 0;

    gettimeofday (&start_tv, NULL);
    prev_tv = start_tv;
    size_t avail;

    for (;;)
    {
	char buf[32 * 1024];

	ssize_t ret = read (0, buf, sizeof (buf));
	if (ret <= 0)
	    break;

	if (maru_stream_write (ctx, stream, buf, ret) < ret)
	{
	    fprintf (stderr, "maru_stream_write() failed\n");
	    break;
	}

	maru_stream_write_avail (ctx, stream);
	maru_stream_current_latency (ctx, stream);

	written += ret;
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

    assert (maru_stream_close (ctx, stream) == LIBMARU_SUCCESS);
    free (desc);
    maru_destroy_context (ctx);

    free (list);
}
