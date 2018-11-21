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
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>


#include "athtool.h"
#include "counters.h"
#include "libticc.h"

/*! \ingroup pswtool */
/*! \subsection counters */
/*! @{ */

/*! TODO Find out counter use ..
 */
static struct ath_counter_desc cnt_list_tpl[] =
{
	{.name="RX_packets1", .off=0x00, .sz=4, .type=PKT},
	{.name="RX_packets2", .off=0x04, .sz=4, .type=PKT},
	{.name="Counter3", .off=0x08, .sz=4, .type=PKT},
	{.name="Counter4", .off=0x0c, .sz=4, .type=PKT},
	{.name="RX_octets", .off=0x10, .sz=4, .type=OCTET},
	{.name="TX_packets1", .off=0x14, .sz=4, .type=PKT},
	{.name="TX_packets2", .off=0x18, .sz=4, .type=PKT},
	{.name="TX_octets", .off=0x1C, .sz=4, .type=OCTET},
	{.name="Counter9", .off=0x20, .sz=4, .type=PKT},
	{.name="Counter10", .off=0x24, .sz=4, .type=PKT},
	{.name="Counter11", .off=0x28, .sz=4, .type=PKT},
	{.name="Counter12", .off=0x2C, .sz=4, .type=PKT},
	{.name="Counter13", .off=0x30, .sz=4, .type=PKT},
	{.name="Counter14", .off=0x34, .sz=4, .type=PKT},
};

#define NUM_COUNTERS	(sizeof(cnt_list_tpl) / sizeof(cnt_list_tpl[0]))
#define STATE_MEM_SZ	(sizeof(struct ath_counter_state) * NUM_COUNTERS * num_ports)

static int cnt_initialized;
static int num_ports = 8;
static struct ath_counter_state *cnt_state;

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
int psw_counters (struct port_id *port, struct filter *filter, int all, int slot, int reset)
{
    uint32_t v32;
    uint64_t v64;
    uint64_t rtime, dtime;
    struct ath_counter_desc *desc;
    struct ath_counter_state *state;
    int i;
    uint32_t	cnt[14];
    char port_id[5];

    if (slot >= CNT_SLOTS)
	return 1;

    if (cnt_state == NULL)
    {
	cnt_state = get_shm(0xfefed004, STATE_MEM_SZ, &cnt_initialized);
    }
    if (cnt_state == NULL)
    {
	SETERR("get_shm");
	return 1;
    }

    if (port->num == -1)
    {
	for (port->num = 0; port->num < num_ports; port->num++)
	{
	    struct port_id nport;

	    if (match_port(port, cnt_list_tpl[port->num].name, -1))
		continue;

	    nport = *port;

	    if (psw_counters (&nport, filter, all, slot, reset))
		return 1;
	}
	return 0;
    }

    if ((port->num < 0) || (port->num >= num_ports))
    {
	SETERR("port number out of range");
	return 1;
    }

    port->name = cnt_list_tpl[port->num].name;

    if (L2SWITCH_GetPortStats (port->num, cnt))
    {
	SETERR("L2SWITCH_GetPortStats");
	return 1;
    }

    rtime = ullTime();

    sprintf (port_id, "%2d", port->num);

    if (prtg_mode())
	strcat (port_id, "_");

    for (i = 0; i < NUM_COUNTERS; i++)
    {
	desc = &cnt_list_tpl[i];
	state = &cnt_state[port->num * NUM_COUNTERS + i];

	if (match_filter(desc, filter))
	    continue;

	v32 = cnt[i];
	v64 = v32;

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

    cnt_initialized = 1;
    return 0;
}

/*! @} */
