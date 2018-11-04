/* Simple tool to access registers of atheros switch on FB6490.
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
#include <sys/shm.h>
#include <malloc.h>

#include "athtool.h"
#include "counters.h"

/*! \ingroup athtool */
/*! \subsection counters */
/*! @{ */

static struct ath_counter_desc cnt_list_tpl[] =
{
	{.name="RxBroad", .off=0x00, .sz=4 },
	{.name="RxPause", .off=0x04, .sz=4 },
	{.name="RxMulti", .off=0x08, .sz=4},
	{.name="RxFcsErr", .off=0x0c, .sz=4},
	{.name="RxAllignErr", .off=0x10, .sz=4},
	{.name="RxUndersize", .off=0x14, .sz=4},
	{.name="RxFragment", .off=0x18, .sz=4},
	{.name="Rx64Byte", .off=0x1C, .sz=4},
	{.name="Rx128Byte", .off=0x20, .sz=4},
	{.name="Rx256Byte", .off=0x24, .sz=4},
	{.name="Rx512Byte", .off=0x28, .sz=4},
	{.name="Rx1024Byte", .off=0x2C, .sz=4},
	{.name="Rx1518Byte", .off=0x30, .sz=4},
	{.name="RxMaxByte", .off=0x34, .sz=4},
	{.name="RxTooLong", .off=0x38, .sz=4},
	{.name="RxGoodByte", .off=0x3C, .sz=8},
	{.name="RXBadByte", .off=0x44, .sz=8},
	{.name="RxOverFlow", .off=0x4C, .sz=4},
	{.name="Filtered", .off=0x50, .sz=4},
	{.name="TxBroad", .off=0x54, .sz=4},
	{.name="TxPause", .off=0x58, .sz=4},
	{.name="TxMulti", .off=0x5C, .sz=4},
	{.name="TxUnderRun", .off=0x60, .sz=4},
	{.name="Tx64Byte", .off=0x64, .sz=4},
	{.name="Tx128Byte", .off=0x68, .sz=4},
	{.name="Tx256Byte", .off=0x6C, .sz=4},
	{.name="Tx512Byte", .off=0x70, .sz=4},
	{.name="Tx1024Byte", .off=0x74, .sz=4},
	{.name="Tx1518Byte", .off=0x78, .sz=4},
	{.name="TxMaxByte", .off=0x7C, .sz=4},
	{.name="TxOverSize", .off=0x80, .sz=4},
	{.name="TxByte", .off=0x84, .sz=8},
	{.name="TxCollision", .off=0x8C, .sz=4},
	{.name="TxAbortCol", .off=0x90, .sz=4},
	{.name="TxMultiCol", .off=0x94, .sz=4},
	{.name="TxSingleCol", .off=0x98, .sz=4},
	{.name="TxExcDefer", .off=0x9C, .sz=4},
	{.name="TxDefer", .off=0xA0, .sz=4},
	{.name="TXLateCol", .off=0xA4, .sz=4},
};

#define NUM_COUNTERS	(sizeof(cnt_list_tpl) / sizeof(cnt_list_tpl[0]))
#define STATE_MEM_SZ	(sizeof(struct ath_counter_state) * NUM_COUNTERS * dev->num_ports)

void *ath_st_alloc(void)
{
    struct ath_dev *dev = calloc(1, sizeof(struct ath_dev));

    dev->num_ports = 5;

    return dev;
}

/*! Print some/all per-port counters
 *
 * The function stores counter data in a shared memory segment, so that it is
 * possible to print counter changes and rates for subsequent calls.
 *
 * \param dev	Device handle
 * \param port	port to show, or -1 for all
 * \param filter if not NULL, print only counters that satisfy substring match
 * \param all print all counters if 1, instead of only the changed ones.
 *	A value of 2 causes the shared memory block to be destroyed and an error 
 *	return code.
 *
 * \returns 0 on success, 1 on error
 */
int ath_counters (void *devp, int port, const char *filter, int all, int slot, int reset)
{
#if !defined(PROC_FILE_ACCESS)
    uint32_t v32;
#endif
    uint64_t v64;
    uint64_t dtime;
    struct ath_counter_desc *desc;
    struct ath_counter_state *state;
    int i;
    int fd;
    ssize_t rs;
    uint64_t pcount[MAX_PORTS];
    uint64_t pcount_hi[MAX_PORTS];
    struct ath_dev *dev = devp;
    char port_id[50];

    if (dev->cnt_state == NULL)
    {
	dev->cnt_state = get_shm(0xfefec003, STATE_MEM_SZ, &dev->cnt_initialized);
    }
    if (dev->cnt_state == NULL)
    {
	return 1;
    }

    if (port == -1)
    {
	for (port = 0; port < dev->num_ports; port++)
	    if (ath_counters (dev, port, filter, all, slot, reset))
		return 1;
	return 0;
    }

    if ((port < 0) || (port >= dev->num_ports))
    {
	SETERR("port number out of range");
	return 1;
    }

#if defined(PROC_FILE_ACCESS)
    if (dev->file_buffer == NULL)
    {
	dev->file_buffer = calloc(1, FILEBUF_SIZE);

	if (!dev->file_buffer)
	{
	    SETERR("Failed to allocate ");
	    return 1;
	}

	fd = open ("/proc/driver/avmnet/ar8327/rmon_all", O_RDONLY);
	if (fd == -1)
	{
	    SETERR("Failed to open /proc/net/pp/global");
	    return 1;
	}

	if ((rs = read (fd, dev->file_buffer, FILEBUF_SIZE-1) == -1))
	{
	    SETERR("Failed to read /proc/net/pp/global");
	    return 1;
	}
	close (fd);

	rs = strlen(dev->file_buffer);

	dev->rtime = ullTime();
    }
#endif

    if (!prtg_mode())
    {
	sprintf (port_id, "%2d", port);
    }
    else
    {
	if (port == 0)
	    strcpy (port_id, "cpu");
	else
	    sprintf (port_id, "p%d", port);
    }

    for (i = 0; i < NUM_COUNTERS; i++)
    {
	desc = &cnt_list_tpl[i];
	state = &dev->cnt_state[port * NUM_COUNTERS + i];

	if (filter && (!strstr (desc->name, filter)))
	    continue;

#if !defined(PROC_FILE_ACCESS)
	v32 = ath_rmw (dev, 0x1000 + port*0x100 + desc->off, 0, 0, &rc);
	if (rc)
	    return 1;

	if (desc->sz == 8)
	{
	    v64 = v32;
	    v32 = ath_rmw (dev, 0x1000 + port*0x100 + desc->off + 4, 0, 0, &rc);
	    if (rc)
		return 1;
	    v64 |= ((uint64_t)v32) << 32;
	}
	else
	{
	    v64 = v32;
	}
	dev->rtime = ullTime();
#else
	char *s = strstr(dev->file_buffer, desc->name);

	if (!s)
	    continue;
	
	s += strlen(desc->name);
	if (desc->sz == 8)
	    s += sizeof("Low");

	sscanf(s, "%lld%lld%lld%lld%lld%lld", 
		&pcount[0], 
		&pcount[1], 
		&pcount[2], 
		&pcount[3], 
		&pcount[4], 
		&pcount[5]);

	if (desc->sz == 8)
	{
	    s = strstr(s, desc->name);
	    if (s)
	    {
		s += strlen(desc->name) + sizeof("High");

		sscanf(s, "%lld%lld%lld%lld%lld%lld", 
			&pcount_hi[0], 
			&pcount_hi[1], 
			&pcount_hi[2], 
			&pcount_hi[3], 
			&pcount_hi[4], 
			&pcount_hi[5]);
	    }
	}
	else
	{
	    memset (pcount_hi, 0, sizeof(pcount_hi));
	}

	v64 = pcount[port] + pcount_hi[port]*0x100000000ULL;
#endif

	if (state->lastReadTime[slot] == 0)
	{
	    dtime = 0;
	    state->max_rate_per_sec = 0;
	}
	else
	{
	    dtime = dev->rtime - state->lastReadTime[slot];
	}

	state->lastReadTime[slot] = dev->rtime;

	cntShow (port_id, v64, &state->sum[slot], desc->name, 0, dtime, all, &state->max_rate_per_sec);

	if (reset)
	    state->max_rate_per_sec = 0;
    }

    dev->cnt_initialized = 1;
    return 0;
}

/*! @} */
