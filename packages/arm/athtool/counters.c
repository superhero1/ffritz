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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>

#include "counters.h"

/* ========================================================================= */

/*! \defgroup counters
 */
/*! @{ */

static enum prtg_mode _prtg_mode = PRTG_OFF;

uint64_t ullTime(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec);
}

/* sprintf value into string and insert , separators 
 * (like the ' printf format character).
 */
char *fmt1000 (uint64_t val, char *str, size_t slen)
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

void set_prtg_mode(enum prtg_mode mode)
{
    _prtg_mode = mode;

}

enum prtg_mode prtg_mode(void)
{
    return _prtg_mode;
}

/*! show a counter
 */
void cntShow (char *port_id, uint64_t val, uint64_t *psum, const char *name, 
                     int cor, int dtime, int showAll, uint64_t *max_rate)
{
    uint64_t sum;
    uint64_t rate = 0;
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

    if (dtime)
    {
	rate = (uint64_t)(val * 1000000) / dtime;

	if (rate > *max_rate)
	    *max_rate = rate;
    }
 
    if (prtg_mode())
    {
	switch (prtg_mode())
	{
	    case PRTG_ABS:
		printf ("<result>\n");
		printf ("<channel>%s%s</channel>\n", port_id, name);
	    	printf ("<value>%llu</value>\n", sum);
		printf ("<Unit>Count</Unit>\n");
		printf ("<Mode>Absolute</Mode>\n");
		printf ("</result>\n");
		break;
	    case PRTG_REL:
		printf ("<result>\n");
		printf ("<channel>%s%s</channel>\n", port_id, name);
	    	printf ("<value>%llu</value>\n", val);
		printf ("<Unit>Count</Unit>\n");
		printf ("<Mode>Relative</Mode>\n");
		printf ("</result>\n");
		break;
	    case PRTG_RATE:
		printf ("<result>\n");
		printf ("<channel>%s%s</channel>\n", port_id, name);
	    	printf ("<value>%llu</value>\n", rate);
		printf ("<Unit>Count</Unit>\n");
		printf ("<Mode>Absolute</Mode>\n");
		printf ("<CustomUnit>pps</CustomUnit>\n");
		printf ("</result>\n");
		break;
	    case PRTG_MAXRATE:
		printf ("<result>\n");
		printf ("<channel>%s%s</channel>\n", port_id, name);
	    	printf ("<value>%llu</value>\n", *max_rate);
		printf ("<Unit>Count</Unit>\n");
		printf ("<Mode>Absolute</Mode>\n");
		printf ("</result>\n");
		break;
	    case PRTG_RATE_AND_MAX:
		printf ("<result>\n");
		printf ("<channel>%s%s</channel>\n", port_id, name);
	    	printf ("<value>%llu</value>\n", rate);
		printf ("<Unit>Count</Unit>\n");
		printf ("<Mode>Absolute</Mode>\n");
		printf ("</result>\n");

		printf ("<result>\n");
		printf ("<channel>%s%s_max</channel>\n", port_id, name);
	    	printf ("<value>%llu</value>\n", *max_rate);
		printf ("<Unit>Count</Unit>\n");
		printf ("<Mode>Absolute</Mode>\n");
		printf ("</result>\n");
		break;
	    default:
	    	break;
	}
    }
    else
    {
	printf ("%s: %-33s : +%-16s ", 
	    port_id, name, fmt1000(val, v1, sizeof(v1)));
	printf ("%17s ", fmt1000(sum, v1, sizeof(v1)));
	if (dtime)
	{
	    printf ("%s/s", 
		fmt1000 (rate, v1, sizeof(v1))
		);

	    printf ("  < %s/s", 
		fmt1000 (*max_rate, v1, sizeof(v1))
		);
	}
	printf ("\n");
    }
}

void *get_shm(unsigned int key, size_t size, int *cnt_initialized)
{
    int shmid;
    void *ptr;

    /* data is kept in IPC shared memory so that we can determine
     * per-second information
     */
    shmid = shmget(key, size, 0);

    if (shmid == -1)
    {
	if ((shmid = shmget(key, 1, 0)) != -1)
	{
	    shmctl (shmid, IPC_RMID, NULL);
	    fprintf (stderr, "existing ID removed for key %x\n", key);
	}

	shmid = shmget(key, size, IPC_CREAT);
	*cnt_initialized = 0;
    }
    else
    {
	*cnt_initialized = 1;
    }

    if (shmid == -1)
    {
	perror ("shmget(IPC_CREAT)");
	return NULL;
    }

    ptr = shmat (shmid, NULL, 0);

    if (ptr == ((void *) -1))
    {
	perror ("shmat");
	return NULL;
    }

    if (*cnt_initialized == 0)
    {
	memset (ptr, 0, size);

	*cnt_initialized = 0;
    }
    else
    {
	*cnt_initialized = 1;
    }

    return ptr;
}

/*! @} */
