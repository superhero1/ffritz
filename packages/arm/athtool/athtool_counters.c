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

static inline uint64_t ullTime(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec);
}

struct counter
{
    const char  name[64];
    int		 off;
    int		 sz;

    uint64_t	 sum;
    uint64_t	 lastReadTime;
};

static int _initialized = 0;
struct counter cnt_list_tpl[] =
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

	{.sz=0}
};

struct counter *cnt_list = NULL;

/* sprintf value into string and insert , separators 
 * (like the ' printf format character).
 */
static char *fmt1000 (uint64_t val, char *str, size_t slen)
{
    int c;
    char buf[64];
    char *p; 
    char *nstr = str;

    snprintf(buf, sizeof(buf), "%lld", val); 
    c = 2 - strlen(buf) % 3;

    for (p = buf; *p != 0; p++)
    {
        slen--;
        if (slen == 0)
            return "NA";
        
        *nstr++ = *p;
    
        if (c == 1)
            *nstr++ = ',';

        c = (c + 1) % 3; 
    }  
    *--nstr = 0;
    
    return str;
}

void cntShow (int port, uint64_t val, uint64_t *psum, const char *name, 
                     int cor, int dtime, int showAll)
{
    uint64_t sum;
    char v1[200];
    
    sum = *psum;
    
    if (!cor)
    {
	if (val < sum)
	    sum = val;

        val -= sum;
    }
    
    if ((val == 0) && !showAll)
        return;

    
    sum += val;
    
    *psum = sum;
 
    printf ("%2d: %-23s : +%-16s ", 
        port, 
        name, fmt1000(val, v1, sizeof(v1)));
    printf ("%17s ", fmt1000(sum, v1, sizeof(v1)));
    if (dtime)
    {
        printf ("%s/sec", 
            fmt1000 ((uint64_t)(val * 1000000) / dtime, v1, sizeof(v1))
            );
    }
    printf ("\n");
}

int ath_counters (int port, const char *filter, int all)
{
    uint32_t v32;
    uint64_t v64;
    int rc = 0;
    uint64_t rtime, dtime;
    struct counter *c;
    int shmid;


    if (cnt_list == NULL)
    {
	/* data is kept in IPC shared memory so that we can determine
	 * per-second information
	 */
	shmid = shmget(0xfefec002, sizeof(cnt_list_tpl), IPC_CREAT);

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

	cnt_list = shmat (shmid, NULL, 0);

	if (cnt_list == ((void *) -1))
	{
	    perror ("shmat");
	    SETERR("shmat");
	    return 1;
	}

	if (cnt_list[0].sz == 0)
	{
	    memcpy (cnt_list, cnt_list_tpl, sizeof(cnt_list_tpl));
	    _initialized = 0;
	}
	else
	{
	    _initialized = 1;
	}
    }

    if (port == -1)
    {
	for (port = 0; port < 7; port++)
	    if (ath_counters (port, filter, all))
		return 1;
	return 0;
    }

    if ((port < 0) || (port > 6))
    {
	SETERR("port number out of range");
	return 1;
    }

    for (c = cnt_list; c->sz != 0; c++)
    {
	if (filter && (!strstr (c->name, filter)))
	    continue;

	v32 = ath_rmw (0x1000 + port*0x100 + c->off, 0, 0, &rc);
	if (rc)
	    return 1;

	if (c->sz == 8)
	{
	    v64 = v32;
	    v32 = ath_rmw (0x1000 + port*0x100 + c->off + 4, 0, 0, &rc);
	    if (rc)
		return 1;
	    v64 |= ((uint64_t)v32) << 32;
	}
	else
	{
	    v64 = v32;
	}

	if (!_initialized)
	{
	    dtime = 0;
	    c->sum = 0;
	}
	else
	{
	    rtime = ullTime();
	    dtime = rtime - c->lastReadTime;
	}

	c->lastReadTime = ullTime();

	cntShow (port, v64, &c->sum, c->name, 0, dtime, all);
    }

    _initialized = 1;
    return 0;
}

