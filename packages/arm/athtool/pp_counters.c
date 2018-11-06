/* Tool to access internal switch of puma6
 *
 * Copyright (C) 2017 - Felix Schmidt
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

/* ========================================================================= */

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>

#include "athtool.h"
#include "counters.h"

/*! \ingroup pswtool */
/*! \subsection counters */
/*! @{ */

static struct ath_counter_desc cnt_list_tpl[] =
{
        {.name="PPDSP_rx_pkts", .type=PKT},
        {.name="PPDSP_pkts_frwrd_to_cpdsp1", .type=PKT},
        {.name="PPDSP_not_enough_descriptors", .type=PKT},

        {.name="CPDSP1_rx_pkts", .type=PKT},
        {.name="CPDSP1_lut1_search_attempts", .type=PKT},
        {.name="CPDSP1_lut1_matches", .type=PKT},
        {.name="CPDSP1_pkts_frwrd_to_cpdsp2", .type=PKT},

        {.name="CPDSP2_rx_pkts", .type=PKT},
        {.name="CPDSP2_lut2_search_attempts", .type=PKT},
        {.name="CPDSP2_lut2_matches", .type=PKT},
        {.name="CPDSP2_pkts_frwrd_to_mpdsp", .type=PKT},
        {.name="CPDSP2_synch_timeout_events", .type=PKT},
        {.name="CPDSP2_reassembly_db_full", .type=PKT},
        {.name="CPDSP2_reassembly_db_timeout", .type=PKT},

        {.name="MPDSP_rx_pkts", .type=PKT},
        {.name="MPDSP_ipv4_rx_pkts", .type=PKT},
        {.name="MPDSP_ipv6_rx_pkts", .type=PKT},
        {.name="MPDSP_frwrd_to_host", .type=PKT},
        {.name="MPDSP_frwrd_to_qpdsp", .type=PKT},
        {.name="MPDSP_frwrd_to_synch_q", .type=PKT},
        {.name="MPDSP_discards", .type=PKT},
        {.name="MPDSP_synchq_overflow_events", .type=PKT},

        {.name="PrxPDSP_popped_from_In_queues", .type=PKT},
        {.name="PrxPDSP_forward_to_L2switch", .type=PKT},
        {.name="PrxPDSP_from_L2switch", .type=PKT},
        {.name="PrxPDSP_pushed_to_Prefetcher", .type=PKT},
        {.name="PrxPDSP_Not_enough_buffers", .type=PKT},
        {.name="PrxPDSP_Not_enough_Descriptors", .type=PKT},
        {.name="PrxPDSP_pktsToSmallForPadding", .type=PKT},

        {.name="QPDSP_ooo_discards", .type=PKT},

};

#define NUM_COUNTERS	(sizeof(cnt_list_tpl) / sizeof(cnt_list_tpl[0]))
#define STATE_MEM_SZ	(sizeof(struct ath_counter_state) * NUM_COUNTERS * num_ports)

static int cnt_initialized;
static int num_ports = 1;
static struct ath_counter_state *cnt_state = NULL;

#undef SETERR
#define SETERR(...)	{fprintf (stderr, __VA_ARGS__); fprintf (stderr, "\n");}


/*! Print some/all per-port counters
 *
 * The function stores counter data in a shared memory segment, so that it is
 * possible to print counter changes and rates for subsequent calls.
 *
 * \param port	port to show, or -1 for all
 * \param filter if not NULL, print only counters that satisfy substring match
 * \param all print all counters if 1, instead of only the changed ones.
 *	A value of 2 causes the shared memory block to be destroyed and an error 
 *	return code.
 *
 * \returns 0 on success, 1 on error
 */
int pp_counters (struct filter *filter, int all, int slot, int reset)
{
    uint64_t v64;
    uint64_t rtime, dtime;
    struct ath_counter_desc *desc;
    struct ath_counter_state *state;
    int i;
    char file_buffer[3000];
    int fd;
    ssize_t rs;
    int retry;
    char port_id[10] = "";
    int port = 0;

    if (slot >= CNT_SLOTS)
	return 1;

    if (cnt_state == NULL)
    {
	cnt_state = get_shm(0xfefed014, STATE_MEM_SZ, &cnt_initialized);
    }
    if (cnt_state == NULL)
    {
	return 1;
    }

    for (retry = 0; retry < 3; retry++)
    {

	fd = open ("/proc/net/pp/global", O_RDONLY);
	if (fd == -1)
	{
	    SETERR("Failed to open /proc/net/pp/global");
	    return 1;
	}

	memset (file_buffer, 0, sizeof(file_buffer));

	if ((rs = read (fd, file_buffer, 2000) == -1))
	{
	    SETERR("Failed to read /proc/net/pp/global");
	    return 1;
	}
	close (fd);

	rs = strlen(file_buffer);

	if (rs < 10)
	    continue;

        rtime = ullTime();

	if (!prtg_mode())
	    sprintf (port_id, "%2d", 0);

	for (i = 0; i < NUM_COUNTERS; i++)
	{
	    char prefix[50];
	    char *suffix;
	    char *s;
	    int found;

	    desc = &cnt_list_tpl[i];
	    state = &cnt_state[port * NUM_COUNTERS + i];

	    if (match_filter(desc, filter))
		continue;

	    strcpy (prefix, cnt_list_tpl[i].name);
	    s = strstr(prefix, "_");
	    if (s)
	    {
		    *s = '\0';
		    suffix = s+1;
	    }
	    else
	    {
		return 1;
	    }

	    if (!strcmp (cnt_list_tpl[i].name, "QPDSP_ooo_discards"))
	    {
		suffix = "Out Of Order";
	    }

	    found = 0;
	    s = strstr(file_buffer, prefix);
	    if (s)
	    {
		s = strstr (s, suffix);

		if (s)
		{
		    s = strstr(s, "=");

		    if (s)
		    {
			v64 = atoll(s+2);
			found = 1;
		    }
		}
	    }

	    if (!found)
		continue;

	    if (state->lastReadTime[slot] == 0)
	    {
		dtime = 0;
		state->max_rate_per_sec = 0;
	    }
	    else
	    {
		dtime = rtime - state->lastReadTime[slot];
	    }

	    state->lastReadTime[slot] = rtime;

	    cntShow (port_id, v64, &state->sum[slot], desc->name, 0, dtime, all, &state->max_rate_per_sec, cnt_list_tpl[i].type);

	    if (reset)
		state->max_rate_per_sec = 0;
	}
	break;
    }

    cnt_initialized = 1;
    return 0;
}

/*! @} */
