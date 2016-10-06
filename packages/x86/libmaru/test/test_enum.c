#include <libmaru.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


int main(void)
{
   struct maru_audio_device *list;
   unsigned num_devices;
   struct timeval tv, prev_tv, start_tv;
   int seconds = 0;

   assert(maru_list_audio_devices(&list, &num_devices) == LIBMARU_SUCCESS);

   fprintf(stderr, "Found %u devices!\n", num_devices);
   for (unsigned i = 0; i < num_devices; i++)
   {
      fprintf(stderr, "Device %u: VID: 0x%04x, PID: 0x%04x\n",
            i, (unsigned)list[i].vendor_id, (unsigned)list[i].product_id);
   }

   if (num_devices > 0)
   {
      maru_context *ctx;
      assert(maru_create_context_from_vid_pid(&ctx, list[0].vendor_id,
               list[0].product_id, &(const struct maru_stream_desc) { .channels = 2, .bits = 16 }) == LIBMARU_SUCCESS);

      maru_error err = maru_stream_set_volume(ctx, LIBMARU_STREAM_MASTER, -20 * 256, 5000000);

      maru_volume cur, min, max;
      err = maru_stream_get_volume(ctx, LIBMARU_STREAM_MASTER, &cur, &min, &max, 5000000);

      fprintf(stderr, "Current: %d, Min: %d, Max: %d\n", cur / 256, min / 256, max / 256);
      err = maru_stream_set_volume(ctx, LIBMARU_STREAM_MASTER, max, 5000000);

      fprintf(stderr, "Streams: %d\n", maru_get_num_streams(ctx));

      int stream = maru_find_available_stream(ctx);
      assert(stream >= 0);

      unsigned num_desc;
      struct maru_stream_desc *desc;
      assert(maru_get_stream_desc(ctx, stream, &desc, &num_desc) == LIBMARU_SUCCESS);

      fprintf(stderr, "Format:\n");
      fprintf(stderr, "\tRate: %u\n", desc[0].sample_rate);
      fprintf(stderr, "\tChannels: %u\n", desc[0].channels);
      fprintf(stderr, "\tBits: %u\n", desc[0].bits);

      desc[0].buffer_size = 1024 * 128;
      desc[0].fragment_size = 1024 * 32;

      err = maru_stream_open(ctx, stream, desc);

#if 0
      size_t total_write = 0;
      bool toggle = false;
#endif

     int written = 0;

     gettimeofday (&start_tv, NULL);
     prev_tv = start_tv;
     size_t avail;

      for (;;)
      {
         char buf[32*1024];

//	 while ((avail = maru_stream_write_avail (ctx, stream)) < sizeof(buf))
//	     printf ("%d ..\n", (int)avail);

         ssize_t ret = read(0, buf, sizeof(buf));
         if (ret <= 0)
            break;

         if (maru_stream_write(ctx, stream, buf, ret) < ret)
         {
            fprintf(stderr, "maru_stream_write() failed\n");
            break;
         }

	 pthread_yield();
	 maru_stream_write_avail (ctx, stream);
	 maru_stream_current_latency (ctx, stream);

	 written += ret;
	 gettimeofday (&tv, NULL);
	 if (prev_tv.tv_sec != tv.tv_sec)
	 {
	     printf("[%d] latency: %d written: %d avail: %d\n",
		(int)(tv.tv_sec - start_tv.tv_sec),
		(int)maru_stream_current_latency (ctx, stream),
		written,
		(int)maru_stream_write_avail (ctx, stream));

	     fflush(stdout);
	     prev_tv = tv;
	     written = 0;

	     seconds++;
	 }
	 else
	 {
	     usleep (100000);
	     printf("[%d] latency: %d written: %d avail: %d\n",
		0,
		0,
		0,
		0
		);

	     seconds++;

	 }
      }

      printf ("done: %d seconds\n", seconds);

      assert(maru_stream_close(ctx, stream) == LIBMARU_SUCCESS);
      free(desc);
      maru_destroy_context(ctx);
   }

   free(list);
}

