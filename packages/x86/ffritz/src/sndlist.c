/* sndlist - list audio devices
 * Copyright (C) 2016 - Felix Schmidt
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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
#include <libmaru.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


int
main (void)
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
