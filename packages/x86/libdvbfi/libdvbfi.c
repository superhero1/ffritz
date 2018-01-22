/* wrapper for libdvbif.so
 * requires binary patched cableinfo where libdvbif.so is replaced with libdvbfi.so
 */

#include <stdint.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef float float32_t;

/* Function pointers to libdvbid.so
 */
int32_t (*p_di_add_pcr_pid)(int32_t a1, int32_t a2, int32_t a3);
int32_t (*p_di_add_pid)(int32_t a1, int16_t a2);
int32_t (*p_di_add_pids)(int32_t a1, int32_t * a2);
int32_t (*p_di_alloc_stream)(int32_t a1);
int32_t (*p_di_alloc_stream_param)(char * a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8, int32_t a9, int32_t a10);
int32_t (*p_di_automode_supported)(void);
int32_t (*p_di_close_stream)(int32_t a1, int32_t a2, int32_t a3);
int32_t (*p_di_exit)(void);
int32_t (*p_di_free_stream)(int32_t a1);
int32_t (*p_di_free_stream_param)(int32_t * a1);
int32_t (*p_di_get_error_rates)(int32_t a1, int32_t * a2, int32_t * a3, int32_t * a4, int32_t a5);
int32_t (*p_di_get_input_signal_power)(int32_t a1, float32_t * a2);
int32_t (*p_di_get_lock_status)(int32_t a1);
int32_t (*p_di_get_number_of_tuners)(void);
int32_t (*p_di_get_signal_noise_ratio)(int32_t a1, float32_t * a2);
int32_t (*p_di_get_support_data)(void);
int32_t (*p_di_init)(int32_t a1);
int32_t (*p_di_open_stream)(int32_t a1);
int32_t (*p_di_recvpid_stream)(int32_t a1, int32_t (*a2)(int32_t, int32_t, int32_t, int32_t), int32_t a3);
int32_t (*p_di_remove_pid)(int32_t a1, int32_t a2);
int32_t (*p_di_remove_pids)(int32_t a1, int32_t * a2);
int32_t (*p_di_spectrum_progress)(int32_t * a1);
int32_t (*p_di_spectrum_start)(int32_t a1, int32_t a2, int64_t a3, int32_t a4);
int32_t (*p_di_spectrum_stop)(void);
int32_t (*p_di_tune_stream)(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8);

int32_t (*p_cableinfo_callback)(int32_t, int32_t, int32_t, int32_t);
int32_t my_cableinfo_callback (int32_t dvb_data, int32_t a2, int32_t a3, int32_t a4);

/* lib initializer
 */
void libinit(void)
{
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
}

/* wrappers 
 */
int32_t di_add_pcr_pid(int32_t a1, int32_t a2, int32_t a3)
{
	return p_di_add_pcr_pid(a1, a2, a3);
}
int32_t di_add_pid(int32_t a1, int16_t a2)
{
	return p_di_add_pid(a1, a2);
}
int32_t di_add_pids(int32_t a1, int32_t * a2)
{
	return p_di_add_pids(a1, a2);
}
int32_t di_alloc_stream(int32_t a1)
{
	return p_di_alloc_stream(a1);
}
int32_t di_alloc_stream_param(char * a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8, int32_t a9, int32_t a10)
{
	return p_di_alloc_stream_param(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}
int32_t di_automode_supported(void)
{
	return p_di_automode_supported();
}
int32_t di_close_stream(int32_t a1, int32_t a2, int32_t a3)
{
	return p_di_close_stream(a1, a2, a3);
}
int32_t di_exit(void)
{
	return p_di_exit();
}
int32_t di_free_stream(int32_t a1)
{
	return p_di_free_stream(a1);
}
int32_t di_free_stream_param(int32_t * a1)
{
	return p_di_free_stream_param(a1);
}
int32_t di_get_error_rates(int32_t a1, int32_t * a2, int32_t * a3, int32_t * a4, int32_t a5)
{
	return p_di_get_error_rates(a1, a2, a3, a4, a5);
}
int32_t di_get_input_signal_power(int32_t a1, float32_t * a2)
{
	return p_di_get_input_signal_power(a1, a2);
}
int32_t di_get_lock_status(int32_t a1)
{
	return p_di_get_lock_status(a1);
}
int32_t di_get_number_of_tuners(void)
{
	return p_di_get_number_of_tuners();
}
int32_t di_get_signal_noise_ratio(int32_t a1, float32_t * a2)
{
	return p_di_get_signal_noise_ratio(a1, a2);
}
int32_t di_get_support_data(void)
{
	return p_di_get_support_data();
}
int32_t di_init(int32_t a1)
{
	return p_di_init(a1);
}
int32_t di_open_stream(int32_t a1)
{
	return p_di_open_stream(a1);
}

int32_t di_recvpid_stream(int32_t a1, int32_t (*cableinfo_callback)(int32_t, int32_t, int32_t, int32_t), int32_t a3)
{
	/* fixme: should be in dynamic context on stack
	 */
	p_cableinfo_callback = cableinfo_callback;

	return p_di_recvpid_stream(a1, &my_cableinfo_callback, a3);
}
int32_t di_remove_pid(int32_t a1, int32_t a2)
{
	return p_di_remove_pid(a1, a2);
}
int32_t di_remove_pids(int32_t a1, int32_t * a2)
{
	return p_di_remove_pids(a1, a2);
}
int32_t di_spectrum_progress(int32_t * a1)
{
	return p_di_spectrum_progress(a1);
}
int32_t di_spectrum_start(int32_t a1, int32_t a2, int64_t a3, int32_t a4)
{
	return p_di_spectrum_start(a1, a2, a3, a4);
}
int32_t di_spectrum_stop(void)
{
	return p_di_spectrum_stop();
}
int32_t di_tune_stream(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8)
{
	return p_di_tune_stream(a1, a2, a3, a4, a5, a6, a7, a8);
}

/* Callback provided by cableinfo.
 * Sends out buffer  of size buf_size as up to 20 individual rtp packets.
 * a3 seems to be context data, a4 unknown
 */
int32_t my_cableinfo_callback (int32_t buffer, int32_t buf_size, int32_t a3, int32_t a4)
{
	printf ("my_cableinfo_callback: %x %x %x %x\n", buffer, buf_size, a3, a4);
	return p_cableinfo_callback (buffer, buf_size, a3, a4);
}
