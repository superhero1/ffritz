/*
 * Copyright (C) 2018 - Felix Schmidt
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

#define MAX_NETDEVS 30

static struct ath_counter_desc cnt_list_tpl[] =
{
        {.name="RX_octets", .type=OCTET},
        {.name="RX_packets", .type=PKT},
        {.name="RX_errs", .type=PKT},
        {.name="RX_drop", .type=PKT},
        {.name="RX_fifo", .type=PKT},
        {.name="RX_fcs", .type=PKT},
        {.name="RX_compressed", .type=PKT},
        {.name="RX_multicast", .type=PKT},

        {.name="TX_octets", .type=OCTET},
        {.name="TX_packets", .type=PKT},
        {.name="TX_errs", .type=PKT},
        {.name="TX_drop", .type=PKT},
        {.name="TX_fifo", .type=PKT},
        {.name="TX_colls", .type=PKT},
        {.name="TX_carrier", .type=PKT},
        {.name="TX_compressed", .type=PKT},
};

#define NUM_COUNTERS	(sizeof(cnt_list_tpl) / sizeof(cnt_list_tpl[0]))

struct ifstate
{
    char id_to_name[MAX_NAME_LEN][MAX_NETDEVS];

    struct ath_counter_state counters[NUM_COUNTERS * MAX_NETDEVS];
};

#define STATE_MEM_SZ	sizeof(struct ifstate)

static int cnt_initialized;
static struct ifstate *cnt_state = NULL;
static char buffer_valid = 0;
static char file_buffer[10000];
static char *port_ptr[MAX_NETDEVS];
static int num_ports = 0;
static uint64_t rtime;

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
int netdev_counters (struct port_id *port, struct filter *filter, int all, int slot, int reset)
{
    uint64_t dtime;
    struct ath_counter_desc *desc;
    struct ath_counter_state *state;
    int i, j;
    int fd;
    ssize_t rs;
    uint64_t	cnt[16];
    char *s, *name;
    char port_id[MAX_NAME_LEN+10];

    if (slot >= CNT_SLOTS)
	return 1;

    if (cnt_state == NULL)
    {
	cnt_state = get_shm(0xfefed006, STATE_MEM_SZ, &cnt_initialized);
    }
    if (cnt_state == NULL)
    {
	return 1;
    }

    if (!buffer_valid)
    {
	memset (file_buffer, 0, sizeof(file_buffer));
	fd = open ("/proc/net/dev", O_RDONLY);

	if (fd == -1)
	{
	    perror ("/proc/net/dev");
	    return 1;
	}

	rs = read (fd, file_buffer, sizeof(file_buffer)-1);
	rtime = ullTime();

	close (fd);

	if (rs <= 0)
	    return 1;
	
	buffer_valid = 1;

	/* find all names */
	s = file_buffer;
	for (num_ports = 0; num_ports < MAX_NETDEVS; num_ports++)
	{
	    s = strstr(s, ":");
	    if (!s)
		break;
	    
	    *s = '\0';
	    name = s-1;
	    while ((name > file_buffer) && 
		   (*name != ' ') &&
		   (*name != '\n'))
	    {
		name--;
	    }
	    name++;

	    if (strlen(name) > MAX_NAME_LEN)
	    {
		fprintf (stderr, "if name too long\n");
		continue;
	    }
	    if (strlen(name) == 0)
		continue;

	    /* is name known?
	     */
	    for (j = 0; j < MAX_NETDEVS; j++)
	    {
		if (strlen(cnt_state->id_to_name[j]) == 0)
		{
		    /* unused port entry, assign it */
		    strcpy (cnt_state->id_to_name[j], name);
		    break;
		}
		if (!strcmp (name, cnt_state->id_to_name[j]))
		{
		    /* known port name */
		    break;
		}
	    }

	    if (j >= MAX_NETDEVS)
	    {
		fprintf (stderr, "too many ports, %s ignored\n", name);
		continue;
	    }

	    /* pointer to where statistics start */
	    port_ptr[j] = s+1;

	    s++;
	}
    }

    if (port->num == -1)
    {
	for (port->num = 0; port->num < num_ports; port->num++)
	{
	    struct port_id nport;

	    if (match_port(port, cnt_state->id_to_name[port->num], -1))
		continue;
	    
	    nport = *port;

	    if (netdev_counters (&nport, filter, all, slot, reset))
		return 1;
	}
	return 0;
    }

    if ((port->num < 0) || (port->num >= num_ports))
    {
	SETERR("port number out of range");
	return 1;
    }

    port->name = cnt_state->id_to_name[port->num];

    if (sscanf (port_ptr[port->num],
	"%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu",
	&cnt[0], 
	&cnt[1], 
	&cnt[2], 
	&cnt[3], 
	&cnt[4], 
	&cnt[5], 
	&cnt[6], 
	&cnt[7], 
	&cnt[8], 
	&cnt[9], 
	&cnt[10], 
	&cnt[11], 
	&cnt[12], 
	&cnt[13], 
	&cnt[14], 
	&cnt[15]) != 16)
    {
	fprintf (stderr, "port %d: bad string\n", port->num);
	return 1;
    }


    if (prtg_mode())
	sprintf (port_id, "%s_", port->name);
    else
	sprintf (port_id, "%12s[%d]", port->name, port->num);

    for (i = 0; i < NUM_COUNTERS; i++)
    {
	desc = &cnt_list_tpl[i];
	state = &cnt_state->counters[port->num * NUM_COUNTERS + i];

	if (match_filter(desc, filter))
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

	cntShow (port_id, cnt[i], &state->sum[slot], desc->name, 0, dtime, all, &state->max_rate_per_sec, cnt_list_tpl[i].type);

	if (reset)
	    state->max_rate_per_sec = 0;
    }
    return 0;
}

/*! @} */
