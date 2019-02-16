/*****************************************************************************
 * libdvbfi.c DVB-C library wrapper and TS/RTP forwareder
 *****************************************************************************
 * Copyright (C) 2018 Felix Schmidt
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#include <stdint.h>
#include <malloc.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/uio.h>
#include <sys/time.h>

#include <netdb.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>


#include "rtp.h"

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

#define FWD_CABLEINFO	0
#define FWD_TS		1
#define FWD_RTP		2

#define TS_SIZE 	1316
#define PID_SIZE	188
#define MAX_BLOCKS	20
#define MAX_PID_PER_TS	(1316/188)

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
	uint32_t		seq;
	uint8_t			ssrc[4];

	uint32_t (*client_cb)(uint32_t, uint32_t, uint32_t, uint32_t);
	uint32_t 		client_cb_arg, client_cb_arg4;

	int			write_triggered;
	pthread_mutex_t		mutex;
	pthread_cond_t		cv;
	pthread_t 		ptid;
	uint8_t 		buffer[0xffff];
	int			wrlen;

	struct wrap_ctx		*next;
};

/* destinatio IP structure passed to csock_sockaddr_set_inaddr()
 */
struct dstip
{
	uint32_t a1;
	uint32_t dstip;
};

/* simple list containing all RTP ports currently used (which are supposed to be blocked)
 */
#define MAX_REDIR 20
struct redir
{
	uint32_t	ip;
	int 		port;
	uint32_t	redir_ip;
	uint32_t	redir_port;
} redir[MAX_REDIR];
int num_redir = 0;

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
uint32_t (*p_di_reset)(struct lib_ctx *ctx);

uint32_t (*p_csock_sockaddr_set_inaddr) (void *a1, struct dstip *dstip, int dstport, void *a4);

uint32_t my_cableinfo_callback (uint32_t dvb_data, uint32_t a2, uint32_t a3, uint32_t a4);
void udp_init (struct wrap_ctx *ctx);
void *send_thread(void *arg);
int wr_trigger(struct wrap_ctx *ctx, uint8_t *buffer, int len);
int wr_wait(struct wrap_ctx *ctx);



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
	char ename[30];
	int i;

	void *lh = dlopen("libdvbif_org.so", RTLD_LAZY);
	if (!lh)
	{
		fprintf (stderr, "*** failed to open libdvbif_org.so\n");
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
	p_di_reset = dlsym (lh, "di_reset");

	/* get original csock_sockaddr_set_inaddr()
 	 */
	p_csock_sockaddr_set_inaddr = dlsym (RTLD_NEXT, "csock_sockaddr_set_inaddr");

	if (p_csock_sockaddr_set_inaddr == NULL)
		printf ("*** Warning: failed to locate csock_sockaddr_set_inaddr()\n");
	
	for (i = 0; i < MAX_REDIR; i++)
	{
		sprintf (ename, "RTP_REDIR%d", i);
		s = getenv (ename);

		/* mandatory: IP/port */
		if (!s)
			continue;
		t = strtok (s, ":");

		if (!t)
			continue;

		if (strlen(t))
			redir[num_redir].ip = ntohl(inet_addr (t));
		else
			redir[num_redir].ip = 0;

		t = strtok (NULL, ":");

		redir[num_redir].port = t ?  atoi (t) : -2;

		/* optional: redir IP/port */
		redir[num_redir].redir_ip = redir[num_redir].ip;
		redir[num_redir].redir_port = -1;
		t = strtok (NULL, ":");

		if (t)
		{
			if (strlen(t))
				redir[num_redir].redir_ip = ntohl(inet_addr (t));
			
			t = strtok (NULL, ":");

			redir[num_redir].redir_port = t ? atoi (t) : -1; 
		}

		printf ("+++ redir[%d]: IP=%d.%d.%d.%d PORT=%d\n",
			num_redir,
			(redir[num_redir].ip >> 24) & 0xff,
			(redir[num_redir].ip >> 16) & 0xff,
			(redir[num_redir].ip >>  8) & 0xff,
			(redir[num_redir].ip >>  0) & 0xff,
			redir[num_redir].port);
		printf ("+++ redir[%d]: DST_IP=%d.%d.%d.%d DST_PORT=%d\n",
			num_redir,
			(redir[num_redir].redir_ip >> 24) & 0xff,
			(redir[num_redir].redir_ip >> 16) & 0xff,
			(redir[num_redir].redir_ip >>  8) & 0xff,
			(redir[num_redir].redir_ip >>  0) & 0xff,
			redir[num_redir].redir_port);


		num_redir++;
	}

	s = getenv ("UDP_SIZE");
	if (s)
	{
		udp_size = atoi(s);
		printf ("+++ UDP fragment size set to %d bytes\n", udp_size);
	}
}

int match_dest (uint32_t ip, int port, uint32_t *redir_ip, int *redir_port)
{
	int i;

	for (i = 0; i < num_redir; i++)
	{
		if (redir[i].ip && (redir[i].ip != ip))
			continue;

		if (redir[i].port == -2)
		{
			if (redir_ip)
				*redir_ip = ip;

			if (redir_port)
				*redir_port = port;

			return FWD_RTP;
		}

		if ((redir[i].port != -1) && (redir[i].port != port))
			continue;

		if (redir_ip)
			*redir_ip = redir[i].redir_ip ? redir[i].redir_ip : ip;

		if (redir_port)
		{
			*redir_port = (redir[i].redir_port != -1) ? redir[i].redir_port : port + 2;
		}


		return FWD_TS;
	}

	return FWD_CABLEINFO;
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

	wrap_ctx = (struct wrap_ctx*)calloc(1, sizeof(struct wrap_ctx));

	wrap_ctx->magic = WRAP_MAGIC;
	wrap_ctx->lib_ctx = lib_ctx;
	wrap_ctx->count = 0;

	printf ("%s(0x%x) -> %p\n", __FUNCTION__, a1, wrap_ctx);

	udp_init (wrap_ctx);

	if (pthread_cond_init (&wrap_ctx->cv, NULL))
	{
		perror ("pthread_cond_init");
		free (wrap_ctx);
		return NULL;
	}

	if (pthread_mutex_init (&wrap_ctx->mutex, NULL))
	{
		perror ("pthread_cond_init");
		free (wrap_ctx);
		return NULL;
	}


	if (pthread_create (&wrap_ctx->ptid, NULL, send_thread, (void*)wrap_ctx))
	{
		perror ("pthread_create");
		free (wrap_ctx);
		return NULL;
	}

	return wrap_ctx;
}
struct stream_param *di_alloc_stream_param(char * a1, uint32_t a2, uint32_t a3, uint32_t dest_ip, uint32_t src_port, uint32_t dest_port, uint32_t a7, uint32_t a8, uint32_t a9, uint32_t a10)
{
	struct stream_param *rc;

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

	printf ("%s(%p) -> ", __FUNCTION__, ctx);
	fflush(stdout);

	if (!ctx)
	{
		/* called twice, second time with NULL
		 */
		rc = p_di_free_stream(NULL);
	}
	else if (ctx->magic != WRAP_MAGIC)
	{
		printf ("di_free_stream: bad magic: 0x%08x\n", ctx->magic);
		fflush(stdout);
		return rc;
	}
	else
	{
		printf (" [%p] ", ctx->lib_ctx);
		fflush(stdout);

		pthread_cancel (ctx->ptid);

		rc = p_di_free_stream (ctx->lib_ctx);

		close (ctx->sockfd);
		free (ctx);
	}
	printf ("0x%x\n", rc);

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
uint32_t di_reset(struct wrap_ctx *ctx)
{
	uint32_t rc = 0;

	printf ("%s(%p) -> \n", __FUNCTION__, ctx);
	fflush(stdout);

	if (p_di_reset)
	{
		if (ctx && (ctx->magic == WRAP_MAGIC) && ctx->lib_ctx)
		{
			rc = p_di_reset(ctx->lib_ctx);
		}
		else
		{
			rc = p_di_reset((void*)ctx);
		}
	}
	printf ("0x%x\n", rc);
	return rc;
}

void udp_init (struct wrap_ctx *ctx)
{
	struct stream_param *stream_param = ctx->lib_ctx->stream_param;
	struct sockaddr_in source;
	char hspec[50];
	uint32_t redir_ip;
	int redir_port;
	
	printf ("+++ Init traffic to %d.%d.%d.%d:%d\n",
		(stream_param->dst_ipv4 >> 24) & 0xff,
		(stream_param->dst_ipv4 >> 16) & 0xff,
		(stream_param->dst_ipv4 >>  8) & 0xff,
		(stream_param->dst_ipv4 >>  0) & 0xff,
		stream_param->dst_port);
	
	ctx->sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset (&source, 0, sizeof(source));
	source.sin_family = AF_INET;
	source.sin_addr.s_addr = htonl(INADDR_ANY);
	source.sin_port = htons(stream_param->src_port);

	if (bind(ctx->sockfd, (struct sockaddr *)&source, sizeof(source)) < 0)
	{
		fprintf (stderr, "bind source port to %d failed: %s\n", 
			source.sin_port, strerror(errno));
	}

	ctx->udp_fwd = match_dest (stream_param->dst_ipv4, stream_param->dst_port, &redir_ip, &redir_port);
	if (ctx->udp_fwd)
	{
		sprintf (hspec, "%d.%d.%d.%d:%d",
			(redir_ip >> 24) & 0xff,
			(redir_ip >> 16) & 0xff,
			(redir_ip >>  8) & 0xff,
			(redir_ip >>  0) & 0xff,
			redir_port);

		if (set_peer (hspec, &ctx->peer))
		{
			ctx->udp_fwd = FWD_CABLEINFO;
		}
		else
		{
			printf ("+++ sending UDP TS to %s:%d\n", hspec, redir_port);

			if (connect(ctx->sockfd, (struct sockaddr *) &ctx->peer,
				(socklen_t) sizeof (ctx->peer)) == -1)
			{
				perror ("connect");
				ctx->udp_fwd = FWD_CABLEINFO;
			}
		}
	}
}

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/* send TS as UDP frame */
void udp_send (struct wrap_ctx *ctx, void *buffer, int len)
{
	int rc;
	int offs, seg;

	offs = 0;
	do 
	{
		seg = MIN(udp_size, len - offs);

		rc = sendto (ctx->sockfd, buffer + offs, seg, MSG_DONTWAIT, (struct sockaddr *) &ctx->peer,
				   (socklen_t) sizeof (ctx->peer));
		if (rc == -1)
		{
			perror ("sendto");
		}

		offs += seg;
	} while (offs < len);
}

/* Encode TS as RTP (somewhat) and send */
void rtp_send (struct wrap_ctx *ctx, void *buffer, int len)
{
	int		rc;
	int		offs, seg;
	uint8_t		rtp_header[RTP_HEADER_SIZE];
	struct 		iovec p_iov[2];
	struct 		timeval tv;
	uint64_t 	t;

	rtp_set_hdr (rtp_header);
	rtp_set_type (rtp_header, RTP_TYPE_TS );
	rtp_set_ssrc (rtp_header, ctx->ssrc);

	p_iov[0].iov_base = rtp_header;
	p_iov[0].iov_len = RTP_HEADER_SIZE;

	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec*1000000 + tv.tv_usec;

	offs = 0;
	do
	{
		seg = MIN(udp_size, len - offs);

		rtp_set_seqnum (rtp_header, ctx->seq++);
		rtp_set_timestamp (rtp_header, t * 9 / 100);
		t += 12;

		p_iov[1].iov_base = buffer + offs;
		p_iov[1].iov_len = seg;

		rc = writev (ctx->sockfd, p_iov, 2);
		if (rc == -1)
		{
			perror ("writev");
		}
		offs += seg;
	} while (offs < len);
}

/* experimental */
void rtp_send2 (struct wrap_ctx *ctx, void *buffer, int len)
{
	int		rc;
	int		offs, seg;
	uint8_t		rtp_headers[RTP_HEADER_SIZE*20];
	uint8_t		*rtp_header = rtp_headers;
	struct 		iovec p_iov[40];
	uint8_t 	*payload = buffer;
	struct 		timeval tv;
	uint64_t 	t;
	int i = 0;

	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec*1000000 + tv.tv_usec;

	offs = 0;
	do
	{
		seg = MIN(udp_size, len - offs);

		rtp_set_hdr (rtp_header);
		rtp_set_type (rtp_header, RTP_TYPE_TS );
		rtp_set_ssrc (rtp_header, ctx->ssrc);
		rtp_set_seqnum (rtp_header, ctx->seq++);
		rtp_set_timestamp (rtp_header, t * 9 / 100);
		p_iov[i].iov_base = rtp_header;
		p_iov[i++].iov_len = RTP_HEADER_SIZE;

		t += 12;
		rtp_header += RTP_HEADER_SIZE;

		p_iov[i].iov_base = payload + offs;
		p_iov[i++].iov_len = seg;

		offs += seg;
	} while (offs < len);

	rc = writev (ctx->sockfd, p_iov, i);
	if (rc == -1)
	{
		perror ("writev");
	}
}

/* Callback provided by cableinfo.
 * Sends out buffer  of size buf_size as up to 20 individual rtp packets.
 * a3 seems to be context data, a4 unknown
 */
uint32_t my_cableinfo_callback (uint32_t buffer, uint32_t buf_size, uint32_t a3, uint32_t a4)
{
	struct wrap_ctx *ctx = (struct wrap_ctx*)a3;
	uint32_t rc = 0;

	ctx->client_cb_arg4 = a4;

	/* Let all send methods be handled in a dedicated thread, including the original
	 * callback back to the cableinfo thread.
	 * This seems to work, although we have to fake the return code.
	 */
#if 0
	if (ctx->udp_fwd == FWD_CABLEINFO)
	{
		rc = ctx->client_cb (buffer, buf_size, ctx->client_cb_arg, a4);
	}
	else
#endif
	{
		wr_trigger (ctx, (void*)buffer, buf_size);
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
	if (match_dest (dstip->dstip, dstport, NULL, NULL))
	{
		dstip->dstip = 0x7f000001;
		dstport = 9;
	}

	rc = p_csock_sockaddr_set_inaddr (a1, dstip, dstport, a4);

	dstip->dstip = saved_dstip;

	return rc;
}

int wr_wait(struct wrap_ctx *ctx)
{
	int rc = 0;

	pthread_mutex_lock (&ctx->mutex);

	while (!ctx->write_triggered)
	{
		if (pthread_cond_wait (&ctx->cv, &ctx->mutex))
		{
			perror ("pthread_cond_wait");
			ctx->write_triggered = 1;
			rc = -1;
		}
	}

	pthread_mutex_unlock (&ctx->mutex);

	return rc;
}

int wr_trigger(struct wrap_ctx *ctx, uint8_t *buffer, int len)
{
	/* If send_thread isn't complete yet, just drop the whole frame for now
	 */
	if (ctx->write_triggered)
		return -1;

	pthread_mutex_lock (&ctx->mutex);

	memcpy (ctx->buffer, buffer, len);
	ctx->wrlen = len;
	ctx->write_triggered = 1;

	pthread_cond_signal (&ctx->cv);

	pthread_mutex_unlock (&ctx->mutex);

	return 0;
}

void *send_thread(void *arg)
{
	struct wrap_ctx *ctx = arg;

	while (1)
	{
		if (wr_wait (ctx))
			return NULL;

		switch (ctx->udp_fwd)
		{
			case FWD_CABLEINFO:
				ctx->client_cb ((uint32_t)ctx->buffer, ctx->wrlen,
					ctx->client_cb_arg, ctx->client_cb_arg4);
				break;
				
			case FWD_TS:
				udp_send (ctx, (void*)ctx->buffer, ctx->wrlen);
				ctx->client_cb ((uint32_t)ctx->buffer, ctx->wrlen,
					ctx->client_cb_arg, ctx->client_cb_arg4);
				break;

			case FWD_RTP:
				rtp_send (ctx, (void*)ctx->buffer, ctx->wrlen);
				ctx->client_cb ((uint32_t)ctx->buffer, ctx->wrlen,
					ctx->client_cb_arg, ctx->client_cb_arg4);
				break;
		}

		ctx->write_triggered = 0;
	}

	return NULL;
}
