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
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>

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
int ath_counters (struct ath_dev *dev, int port, const char *filter, int all)
{
    uint32_t v32;
    uint64_t v64;
    int rc = 0;
    uint64_t rtime, dtime;
    struct ath_counter_desc *desc;
    struct ath_counter_state *state;
    int shmid;
    int i;

    if (dev->cnt_state == NULL)
    {
	/* data is kept in IPC shared memory so that we can determine
	 * per-second information
	 */
	shmid = shmget(0xfefec003 + dev->instance, STATE_MEM_SZ, 0);

	if (shmid == -1)
	{
	    shmid = shmget(0xfefec003 + dev->instance, STATE_MEM_SZ, IPC_CREAT);
	    dev->cnt_initialized = 0;
	}
	else
	{
	    dev->cnt_initialized = 1;
	}

	if (shmid == -1)
	{
	    perror ("shmget(IPC_CREAT)");
	    SETERR("shmget");
	    return 1;
	}

	if (all == 2)
	{
	    if (shmctl (shmid, IPC_RMID, NULL))
	    {
		perror ("shmctl(IPC_RMID)");
	    }
	    else
	    {
		printf ("destroyed\n");
		return 1;
	    }
	}

	dev->cnt_state = shmat (shmid, NULL, 0);

	if (dev->cnt_state == ((void *) -1))
	{
	    perror ("shmat");
	    SETERR("shmat");
	    return 1;
	}

	if (dev->cnt_initialized == 0)
	{
	    memset (dev->cnt_state, 0, STATE_MEM_SZ);

	    dev->cnt_initialized = 0;
	}
	else
	{
	    dev->cnt_initialized = 1;
	}
    }

    if (port == -1)
    {
	for (port = 0; port < dev->num_ports; port++)
	    if (ath_counters (dev, port, filter, all))
		return 1;
	return 0;
    }

    if ((port < 0) || (port >= dev->num_ports))
    {
	SETERR("port number out of range");
	return 1;
    }

    for (i = 0; i < NUM_COUNTERS; i++)
    {
	desc = &cnt_list_tpl[i];
	state = &dev->cnt_state[port * NUM_COUNTERS + i];

	if (filter && (!strstr (desc->name, filter)))
	    continue;

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

	if (state->lastReadTime == 0)
	{
	    dtime = 0;
	    state->sum = 0;
	}
	else
	{
	    rtime = ullTime();
	    dtime = rtime - state->lastReadTime;
	}

	state->lastReadTime = ullTime();

	cntShow (port, v64, &state->sum, desc->name, 0, dtime, all);
    }

    dev->cnt_initialized = 1;
    return 0;
}

/*! @} */
