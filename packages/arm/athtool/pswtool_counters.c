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
#include "libticc.h"

/*! \ingroup pswtool */
/*! \subsection counters */
/*! @{ */

static inline uint64_t ullTime(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec);
}

static struct ath_counter_desc cnt_list_tpl[] =
{
	{.name="Counter1", .off=0x00, .sz=4 },
	{.name="Counter2", .off=0x04, .sz=4 },
	{.name="Counter3", .off=0x08, .sz=4},
	{.name="Counter4", .off=0x0c, .sz=4},
	{.name="Counter5", .off=0x10, .sz=4},
	{.name="Counter6", .off=0x14, .sz=4},
	{.name="Counter7", .off=0x18, .sz=4},
	{.name="Counter8", .off=0x1C, .sz=4},
	{.name="Counter9", .off=0x20, .sz=4},
	{.name="Counter10", .off=0x24, .sz=4},
	{.name="Counter11", .off=0x28, .sz=4},
	{.name="Counter12", .off=0x2C, .sz=4},
	{.name="Counter13", .off=0x30, .sz=4},
	{.name="Counter14", .off=0x34, .sz=4},
};

#define NUM_COUNTERS	(sizeof(cnt_list_tpl) / sizeof(cnt_list_tpl[0]))
#define STATE_MEM_SZ	(sizeof(struct ath_counter_state) * NUM_COUNTERS * num_ports)

static int cnt_initialized;
static int num_ports = 8;
static struct ath_counter_state *cnt_state;

#undef SETERR
#define SETERR(...)	{fprintf (stderr, __VA_ARGS__); fprintf (stderr, "\n");}


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

/*! show a counter
 */
static void cntShow (int port, uint64_t val, uint64_t *psum, const char *name, 
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
int psw_counters (int port, const char *filter, int all)
{
    uint32_t v32;
    uint64_t v64;
    uint64_t rtime, dtime;
    struct ath_counter_desc *desc;
    struct ath_counter_state *state;
    int shmid;
    int i;
    uint32_t	cnt[14];


    if (cnt_state == NULL)
    {
	/* data is kept in IPC shared memory so that we can determine
	 * per-second information
	 */
	shmid = shmget(0xfefed003, STATE_MEM_SZ, 0);

	if (shmid == -1)
	{
	    shmid = shmget(0xfefed003, STATE_MEM_SZ, IPC_CREAT);
	    cnt_initialized = 0;
	}
	else
	{
	    cnt_initialized = 1;
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

	cnt_state = shmat (shmid, NULL, 0);

	if (cnt_state == ((void *) -1))
	{
	    perror ("shmat");
	    SETERR("shmat");
	    return 1;
	}

	if (cnt_initialized == 0)
	{
	    memset (cnt_state, 0, STATE_MEM_SZ);

	    cnt_initialized = 0;
	}
	else
	{
	    cnt_initialized = 1;
	}
    }

    if (port == -1)
    {
	for (port = 0; port < num_ports; port++)
	    if (psw_counters (port, filter, all))
		return 1;
	return 0;
    }

    if ((port < 0) || (port >= num_ports))
    {
	SETERR("port number out of range");
	return 1;
    }

    if (L2SWITCH_GetPortStats (port, cnt))
    {
	SETERR("L2SWITCH_GetPortStats");
	return 1;
    }

    for (i = 0; i < NUM_COUNTERS; i++)
    {
	desc = &cnt_list_tpl[i];
	state = &cnt_state[port * NUM_COUNTERS + i];

	if (filter && (!strstr (desc->name, filter)))
	    continue;

	v32 = cnt[i];
	v64 = v32;

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

    cnt_initialized = 1;
    return 0;
}

/*! @} */
