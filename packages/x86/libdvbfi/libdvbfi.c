/* wrapper for libdvbif.so
 *
 */

#include <stdint.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <string.h>
#include <errno.h>

#if !defined(RTLD_NEXT)
# define RTLD_NEXT   ((void *) -1l)
#endif

/*
A typical call sequence when opening a stream:

	di_alloc_stream_param(0 b728d028 0 c0a80039 1388 d01e 1 b75fe300 10 8065c61) -> b73c1948
	di_alloc_stream(b73c1948) -> b728c000
	di_close_stream(b728c000 bfbd9a98 80527bb) -> 0
	di_tune_stream(b728c000 1f1d1680 694920 0 5 0 5 5) -> 0
	di_open_stream(b728c000) -> 0
	di_add_pid(b728c000 0) -> 0
	di_add_pid(b728c000 10) -> 0
	di_add_pid(b728c000 11) -> 0
	di_add_pid(b728c000 12) -> 0
	di_add_pid(b728c000 14) -> 0
	di_add_pid(b728c000 13ec) -> 0
	di_add_pid(b728c000 13ed) -> 0
	di_add_pid(b728c000 492) -> 0
	di_add_pid(b728c000 498) -> 0
	di_add_pid(b728c000 87b) -> 0
	di_add_pid(b728c000 13ee) -> 0
	di_add_pid(b728c000 13ef) -> 0
	di_add_pid(b728c000 13f0) -> 0
	di_add_pid(b728c000 13f1) -> 0
	di_add_pid(b728c000 13f2) -> 0
	di_add_pid(b728c000 13f4) -> 0
	di_add_pid(b728c000 1434) -> 0
	p_di_recvpid_stream(b728c000 ..)
	...
	di_close_stream(b728c000 bfbd9a98 80527bb) -> 0

*/

typedef float float32_t;

/* Stream parameters
 */
struct stream_param
{
	uint32_t	u1;
	uint32_t	u2;
	uint32_t	u3;
	uint32_t	u4;
	uint32_t	u5;
	uint32_t	u6;
	uint32_t	u7;
	uint32_t	dst_ipv4;
	uint32_t	src_port;
	uint32_t	dst_port;
};

/* libdvbif context structure
 */
struct lib_ctx
{
	struct stream_param *stream_param;
};

#define WRAP_MAGIC	0x600df00d

/* wrapper for libdvbif context
 */
struct wrap_ctx
{
	uint32_t		magic;
	struct lib_ctx 		*lib_ctx;

	uint32_t 		count;
	int			sockfd;
	int			udp_fwd;
	struct sockaddr_in 	peer;
	struct sockaddr_in 	source;

	uint32_t (*client_cb)(uint32_t, uint32_t, uint32_t, uint32_t);
	uint32_t 		client_cb_arg;

	struct wrap_ctx		*next;
};

/* destinatio IP structure passed to csock_sockaddr_set_inaddr()
 */
struct dstip
{
	uint32_t a1;
	uint32_t dstip;
};

#define MAX_DESTS 30

struct udp_dest
{
	int			used;
	char			hspec[50];
	struct sockaddr_in 	peer;
} dest[MAX_DESTS];

/* simple list containing all RTP ports currently used (which are supposed to be blocked)
 */
#define MAX_DST_PORTS 20
int num_dstports = 0;
int dstports[100];

int have_static_portlist = 0;

int udp_size = 1316;


/* Function pointers to libdvbid.so
 */
uint32_t (*p_di_add_pcr_pid)(struct lib_ctx *ctx, uint32_t a2, uint32_t a3);
uint32_t (*p_di_add_pid)(struct lib_ctx *ctx, int16_t a2);
uint32_t (*p_di_add_pids)(struct lib_ctx *ctx, uint32_t * a2);
struct lib_ctx * (*p_di_alloc_stream)(uint32_t a1);
struct stream_param* (*p_di_alloc_stream_param)(char * a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7, uint32_t a8, uint32_t a9, uint32_t a10);
uint32_t (*p_di_automode_supported)(void);
uint32_t (*p_di_close_stream)(struct lib_ctx *ctx, uint32_t a2, uint32_t a3);
uint32_t (*p_di_exit)(void);
uint32_t (*p_di_free_stream)(struct lib_ctx *ctx);
uint32_t (*p_di_free_stream_param)(uint32_t * a1);
uint32_t (*p_di_get_error_rates)(struct lib_ctx *ctx, uint32_t * a2, uint32_t * a3, uint32_t * a4, uint32_t a5);
uint32_t (*p_di_get_input_signal_power)(struct lib_ctx *ctx, float32_t * a2);
uint32_t (*p_di_get_lock_status)(struct lib_ctx *ctx, uint32_t a1);
uint32_t (*p_di_get_number_of_tuners)(void);
uint32_t (*p_di_get_signal_noise_ratio)(struct lib_ctx *ctx, float32_t * a2);
uint32_t (*p_di_get_support_data)(void);
uint32_t (*p_di_init)(uint32_t a1);
uint32_t (*p_di_open_stream)(struct lib_ctx *ctx);
uint32_t (*p_di_recvpid_stream)(struct lib_ctx *ctx, uint32_t (*a2)(uint32_t, uint32_t, uint32_t, uint32_t), uint32_t a3);
uint32_t (*p_di_remove_pid)(struct lib_ctx *ctx, uint32_t a2);
uint32_t (*p_di_remove_pids)(struct lib_ctx *ctx, uint32_t * a2);
uint32_t (*p_di_spectrum_progress)(uint32_t * a1);
uint32_t (*p_di_spectrum_start)(uint32_t a1, uint32_t a2, int64_t a3, uint32_t a4);
uint32_t (*p_di_spectrum_stop)(void);
uint32_t (*p_di_tune_stream)(struct lib_ctx *ctx, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7, uint32_t a8);

uint32_t (*p_csock_sockaddr_set_inaddr) (void *a1, struct dstip *dstip, int dstport, void *a4);

uint32_t my_cableinfo_callback (uint32_t dvb_data, uint32_t a2, uint32_t a3, uint32_t a4);
void udp_init (struct wrap_ctx *ctx);



int is_rtp_port (int port)
{
	int i;

	for (i = 0; i < num_dstports; i++)
		if (dstports[i] == port)
			return 1;
	
	return 0;
}


void add_rtp_port (int port)
{
	if (have_static_portlist)
		return;

	if (is_rtp_port (port))
		return;

	if (num_dstports >= MAX_DST_PORTS)
		return;

	dstports[num_dstports++] = port;

	printf ("+++ Blocking packets to destination port %d\n", port);
}

void del_rtp_port (int port)
{
	int i, j;

	if (have_static_portlist)
		return;

	for (i = j = 0; i < num_dstports; i++, j++)
	{
		if (dstports[i] == port)
		{
			j++;
			num_dstports--;
		}

		dstports[i] = dstports[j];
	}

	printf ("+++ Resuming packets to destination port %d\n", port);
}


int set_peer (char *hspec, struct sockaddr_in *peer)
{
	struct hostent          *host;
	char *t;

	t = strtok (hspec, ":");
	if (!t)
	{
		return -1;
	}

	host = (struct hostent *) gethostbyname (t);
	if (!host)
	{
		return -1;
	}

	memcpy (&peer->sin_addr, host->h_addr_list[0], host->h_length);

	t = strtok (NULL, ":");
	if (!t)
	{
		return -1;
	}

	peer->sin_port = htons (atoi(t));
	peer->sin_family = AF_INET;

	return 0;
}

/* lib initializer
 */
void libinit(void)
{
	char *s, *t;

	void *lh = dlopen("libdvbif.so", RTLD_LAZY);
	if (!lh)
	{
		fprintf (stderr, "*** failed to open libdvbif.so\n");
		exit (1);
	}

	p_di_add_pcr_pid = dlsym (lh, "di_add_pcr_pid");
	p_di_add_pid = dlsym (lh, "di_add_pid");
	p_di_add_pids = dlsym (lh, "di_add_pids");
	p_di_alloc_stream = dlsym (lh, "di_alloc_stream");
	p_di_alloc_stream_param = dlsym (lh, "di_alloc_stream_param");
	p_di_automode_supported = dlsym (lh, "di_automode_supported");
	p_di_close_stream = dlsym (lh, "di_close_stream");
	p_di_exit = dlsym (lh, "di_exit");
	p_di_free_stream = dlsym (lh, "di_free_stream");
	p_di_free_stream_param = dlsym (lh, "di_free_stream_param");
	p_di_get_error_rates = dlsym (lh, "di_get_error_rates");
	p_di_get_input_signal_power = dlsym (lh, "di_get_input_signal_power");
	p_di_get_lock_status = dlsym (lh, "di_get_lock_status");
	p_di_get_number_of_tuners = dlsym (lh, "di_get_number_of_tuners");
	p_di_get_signal_noise_ratio = dlsym (lh, "di_get_signal_noise_ratio");
	p_di_get_support_data = dlsym (lh, "di_get_support_data");
	p_di_init = dlsym (lh, "di_init");
	p_di_open_stream = dlsym (lh, "di_open_stream");
	p_di_recvpid_stream = dlsym (lh, "di_recvpid_stream");
	p_di_remove_pid = dlsym (lh, "di_remove_pid");
	p_di_remove_pids = dlsym (lh, "di_remove_pids");
	p_di_spectrum_progress = dlsym (lh, "di_spectrum_progress");
	p_di_spectrum_start = dlsym (lh, "di_spectrum_start");
	p_di_spectrum_stop = dlsym (lh, "di_spectrum_stop");
	p_di_tune_stream = dlsym (lh, "di_tune_stream");

	/* get original csock_sockaddr_set_inaddr()
 	 */
	p_csock_sockaddr_set_inaddr = dlsym (RTLD_NEXT, "csock_sockaddr_set_inaddr");

	if (p_csock_sockaddr_set_inaddr == NULL)
		printf ("*** Warning: failed to locate csock_sockaddr_set_inaddr()\n");
	
	s = getenv ("RTP_PORTLIST");

	if (s)
	{
		t = strtok (s, ",");
		while (t)
		{
			add_rtp_port (atoi(t));

			t = strtok (NULL, ",");
		}

		have_static_portlist = 1;
	}

	s = getenv ("UDP_SIZE");
	if (s)
	{
		udp_size = atoi(s);
		printf ("+++ UDP fragment size set to %d bytes\n", udp_size);
	}
}

/* wrappers 
 */
uint32_t di_add_pcr_pid(struct wrap_ctx *ctx, uint32_t a2, uint32_t a3)
{
	uint32_t rc = p_di_add_pcr_pid (ctx->lib_ctx, a2, a3);
	printf ("%s(%p 0x%x 0x%x) -> 0x%x\n", __FUNCTION__, ctx, a2, a3, rc);
	return rc;
}
uint32_t di_add_pid(struct wrap_ctx *ctx, int16_t pid)
{
	uint32_t rc = p_di_add_pid (ctx->lib_ctx, pid);
	printf ("%s(%p %d) -> 0x%x\n", __FUNCTION__, ctx, pid, rc);
	return rc;
}
uint32_t di_add_pids(struct wrap_ctx *ctx, uint32_t * pids)
{
	uint32_t rc = p_di_add_pids (ctx->lib_ctx, pids);
	printf ("%s(%p %p) -> 0x%x\n", __FUNCTION__, ctx, pids, rc);
	return rc;
}
struct wrap_ctx *di_alloc_stream(uint32_t a1)
{
	struct lib_ctx *lib_ctx = p_di_alloc_stream (a1);
	struct wrap_ctx *wrap_ctx;

	if (!lib_ctx)
		return NULL;

	wrap_ctx = (struct wrap_ctx*)malloc(sizeof(struct wrap_ctx));

	wrap_ctx->magic = WRAP_MAGIC;
	wrap_ctx->lib_ctx = lib_ctx;
	wrap_ctx->count = 0;

	printf ("%s(0x%x) -> %p\n", __FUNCTION__, a1, wrap_ctx);

	udp_init (wrap_ctx);

	return wrap_ctx;
}
struct stream_param *di_alloc_stream_param(char * a1, uint32_t a2, uint32_t a3, uint32_t dest_ip, uint32_t src_port, uint32_t dest_port, uint32_t a7, uint32_t a8, uint32_t a9, uint32_t a10)
{
	struct stream_param *rc;

//	dest_ip = inet_addr("127.0.0.1");
//	dest_port=10000;

	rc = p_di_alloc_stream_param (a1, a2, a3, dest_ip, src_port, dest_port, a7, a8, a9, a10);

	printf ("%s(%s 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x) -> %p\n", __FUNCTION__, a1, a2, a3, dest_ip, src_port, dest_port, a7, a8, a9, a10, rc);

	printf ("%x %x\n", dest_ip, rc->dst_ipv4);
	return rc;
}
uint32_t di_automode_supported(void)
{
	uint32_t rc = p_di_automode_supported();
	printf ("%s() -> 0x%x\n", __FUNCTION__, rc);
	return rc;
}
uint32_t di_close_stream(struct wrap_ctx *ctx, uint32_t a2, uint32_t a3)
{
	uint32_t rc = p_di_close_stream (ctx->lib_ctx, a2, a3);
	printf ("%s(%p 0x%x 0x%x) -> 0x%x\n", __FUNCTION__, ctx, a2, a3, rc);
	return rc;
}
uint32_t di_exit(void)
{
	uint32_t rc = p_di_exit();
	printf ("%s() -> 0x%x\n", __FUNCTION__, rc);
	return rc;
}
uint32_t di_free_stream(struct wrap_ctx *ctx)
{
	uint32_t rc = 0;

	if (!ctx)
	{
		/* called twice, second time with NULL
		 */
		rc = p_di_free_stream(NULL);
	}
	else
	{
		del_rtp_port (ctx->lib_ctx->stream_param->dst_port);

		rc = p_di_free_stream (ctx->lib_ctx);

		close (ctx->sockfd);
		free (ctx);

		printf ("%s(%p) -> 0x%x\n", __FUNCTION__, ctx, rc);
	}

	return rc;
}
uint32_t di_free_stream_param(uint32_t * a1)
{
	uint32_t rc = p_di_free_stream_param (a1);
	printf ("%s(%p) -> 0x%x\n", __FUNCTION__, a1, rc);
	return rc;
}
uint32_t di_get_error_rates(struct wrap_ctx *ctx, uint32_t * a2, uint32_t * a3, uint32_t * a4, uint32_t a5)
{
	uint32_t rc = p_di_get_error_rates (ctx->lib_ctx, a2, a3, a4, a5);
	printf ("%s(%p 0x%x 0x%x 0x%x 0x%x) -> 0x%x\n", __FUNCTION__, ctx, a2, a3, a4, a5, rc);
	return rc;
}
uint32_t di_get_input_signal_power(struct wrap_ctx *ctx, float32_t * a2)
{
	uint32_t rc = p_di_get_input_signal_power (ctx->lib_ctx, a2);
	printf ("%s(%p 0x%x) -> 0x%x\n", __FUNCTION__, ctx, a2, rc);
	return rc;
}
uint32_t di_get_lock_status(struct wrap_ctx *ctx, uint32_t a1)
{
	uint32_t rc = p_di_get_lock_status (ctx->lib_ctx, a1);
	printf ("%s(%p 0x%x) -> 0x%x\n", __FUNCTION__, ctx, a1, rc);
	return rc;
}
uint32_t di_get_number_of_tuners(void)
{
	uint32_t rc = p_di_get_number_of_tuners();
	printf ("%s() -> 0x%x\n", __FUNCTION__, rc);
	return rc;
}
uint32_t di_get_signal_noise_ratio(struct wrap_ctx *ctx, float32_t * a2)
{
	uint32_t rc = p_di_get_signal_noise_ratio (ctx->lib_ctx, a2);
	printf ("%s(%p 0x%x) -> 0x%x\n", __FUNCTION__, ctx, a2, rc);
	return rc;
}
uint32_t di_get_support_data(void)
{
	uint32_t rc = p_di_get_support_data();
	printf ("%s() -> 0x%x\n", __FUNCTION__, rc);
	return rc;
}
uint32_t di_init(uint32_t a1)
{
	uint32_t rc = p_di_init (a1);
	printf ("%s(0x%x) -> 0x%x\n", __FUNCTION__, a1, rc);
	return rc;
}
uint32_t di_open_stream(struct wrap_ctx *ctx)
{
	uint32_t rc = p_di_open_stream (ctx->lib_ctx);
	printf ("%s(%p) -> 0x%x\n", __FUNCTION__, ctx, rc);
	return rc;
}

uint32_t di_recvpid_stream(struct wrap_ctx *ctx, uint32_t (*cableinfo_callback)(uint32_t, uint32_t, uint32_t, uint32_t), uint32_t a3)
{
	uint32_t rc;

	ctx->client_cb = cableinfo_callback;

	ctx->client_cb_arg = a3;

	rc = p_di_recvpid_stream (ctx->lib_ctx, &my_cableinfo_callback, (uint32_t)ctx);

	if (ctx->count == 0)
		printf ("%s(%p 0x%x 0x%x) -> 0x%x\n", __FUNCTION__, ctx, cableinfo_callback, a3, rc);

	ctx->count++;

	return rc;

}
uint32_t di_remove_pid(struct wrap_ctx *ctx, uint32_t pid)
{
	uint32_t rc = p_di_remove_pid (ctx->lib_ctx, pid);
	printf ("%s(%p %d) -> 0x%x\n", __FUNCTION__, ctx, pid, rc);
	return rc;
}
uint32_t di_remove_pids(struct wrap_ctx *ctx, uint32_t * pids)
{
	uint32_t rc = p_di_remove_pids (ctx->lib_ctx, pids);
	printf ("%s(%p 0x%x) -> 0x%x\n", __FUNCTION__, ctx, pids, rc);
	return rc;
}
uint32_t di_spectrum_progress(uint32_t * a1)
{
	uint32_t rc = p_di_spectrum_progress (a1);
	printf ("%s(0x%x) -> 0x%x\n", __FUNCTION__, a1, rc);
	return rc;
}
uint32_t di_spectrum_start(uint32_t a1, uint32_t a2, int64_t a3, uint32_t a4)
{
	uint32_t rc = p_di_spectrum_start (a1, a2, a3, a4);
	printf ("%s(0x%x 0x%x 0x%x 0x%x) -> 0x%x\n", __FUNCTION__, a1, a2, a3, a4, rc);
	return rc;
}
uint32_t di_spectrum_stop(void)
{
	uint32_t rc = p_di_spectrum_stop();
	printf ("%s() -> 0x%x\n", __FUNCTION__, rc);
	return rc;
}
uint32_t di_tune_stream(struct wrap_ctx *ctx, uint32_t freq, uint32_t symrate, uint32_t specinv, uint32_t mtype, uint32_t a6, uint32_t a7, uint32_t a8)
{
	uint32_t rc = p_di_tune_stream (ctx->lib_ctx, freq, symrate, specinv, mtype, a6, a7, a8);
	printf ("%s(%p 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x) -> 0x%x\n", __FUNCTION__, ctx, freq, symrate, specinv, mtype, a6, a7, a8, rc);
	return rc;
}

int get_free_dest(void)
{
	int i;

	for (i = 0; i < MAX_DESTS; i++)
	{
		if (dest[i].used == 0)
		{
			dest[i].used = 1;
			return i;
		}
	}
	return -1;
}

void udp_init (struct wrap_ctx *ctx)
{
	struct stream_param *stream_param = ctx->lib_ctx->stream_param;
	struct sockaddr_in source;
	char hspec[50];
	
	printf ("+++ Init traffic to %d.%d.%d.%d:%d\n",
		(stream_param->dst_ipv4 >> 24) & 0xff,
		(stream_param->dst_ipv4 >> 16) & 0xff,
		(stream_param->dst_ipv4 >>  8) & 0xff,
		(stream_param->dst_ipv4 >>  0) & 0xff,
		stream_param->dst_port);
	
	ctx->sockfd = socket (AF_INET, SOCK_DGRAM, 0);

	memset (&source, 0, sizeof(source));
	source.sin_family = AF_INET;
	source.sin_addr.s_addr = htonl(INADDR_ANY);
	source.sin_port = htons(stream_param->src_port);

	if (bind(ctx->sockfd, (struct sockaddr *)&source, sizeof(source)) < 0)
	{
		fprintf (stderr, "bind source port to %d failed: %s\n", 
			source.sin_port, strerror(errno));
	}

	if (have_static_portlist && !is_rtp_port(stream_param->dst_port))
	{
		ctx->udp_fwd = 0;
	}
	else
	{
		sprintf (hspec, "%d.%d.%d.%d:%d",
			(stream_param->dst_ipv4 >> 24) & 0xff,
			(stream_param->dst_ipv4 >> 16) & 0xff,
			(stream_param->dst_ipv4 >>  8) & 0xff,
			(stream_param->dst_ipv4 >>  0) & 0xff,
			stream_param->dst_port + 2);

		ctx->udp_fwd = (set_peer (hspec, &ctx->peer) == 0);

		printf ("+++ sending UDP TS to RTP destination port + 2 (%d)\n",
			stream_param->dst_port + 2);
		
	}

	/* tell libavmcsock wrapper to discard all packets to this port
 	 */
	add_rtp_port (stream_param->dst_port);
}

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

void udp_send (struct wrap_ctx *ctx, void *buffer, int len)
{
	int rc;
	int offs, seg;

	offs = 0;
	do 
	{
		seg = MIN(udp_size, len - offs);

		if (offs > len)
			break;

		rc = sendto (ctx->sockfd, buffer + offs, seg, MSG_DONTWAIT, (struct sockaddr *) &ctx->peer,
				   (socklen_t) sizeof (ctx->peer));
		if (rc == -1)
		{
			perror ("sendto");
		}

		offs += seg;
	} while (offs < len);
}

/* Callback provided by cableinfo.
 * Sends out buffer  of size buf_size as up to 20 individual rtp packets.
 * a3 seems to be context data, a4 unknown
 */
uint32_t my_cableinfo_callback (uint32_t buffer, uint32_t buf_size, uint32_t a3, uint32_t a4)
{
	struct wrap_ctx *ctx = (struct wrap_ctx*)a3;
	uint32_t rc = 0;

	if (!ctx->udp_fwd)
	{
		rc = ctx->client_cb (buffer, buf_size, ctx->client_cb_arg, a4);
	}
	else
	{
		udp_send (ctx, (void*)buffer, buf_size);
		rc = ctx->client_cb (buffer, 1300, ctx->client_cb_arg, a4);
	}

	return rc;
}


/******** Wrappers for libavmcsock.so (via LD_PRELOAD) */

uint32_t csock_sockaddr_set_inaddr (void *a1, struct dstip *dstip, int dstport, void *a4)
{
	uint32_t rc;
	uint32_t saved_dstip = dstip->dstip;

	/* forward everything we send to remote RTP port to local discard port so that it does not
	 * reach the original target ..
 	 */
	if (is_rtp_port (dstport))
	{
		dstip->dstip = 0x7f000001;
		dstport = 9;
	}

	rc = p_csock_sockaddr_set_inaddr (a1, dstip, dstport, a4);

	dstip->dstip = saved_dstip;

	return rc;
}
