/* Tool to monitor pp counters
 *
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
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "counters.h"

/* ========================================================================= */

#define SETERR(_s)  dev->ath_err = _s;
#define PRERR(...) {fprintf (stderr, "pptool :: ");\
    fprintf (stderr, __VA_ARGS__);\
    fprintf (stderr, " :: ERROR\n");\
    fflush(stderr);}

/*! \defgroup pptool puma6 switch tool
 * For usage details refer to "pptool -h".
 */
/*! @{ */

static const char *usage =
"pswtool           : puma6 l2 switch tool (USE WITH CARE)\n"
" --help|-h        : This message\n"
" ==== Counters ====\n"
" --show-counters|-c <p>[,<all>[,<filter>]]\n"
"                  : Print counters of port <p> (-1 for all ports).\n"
"                    If <all> is 1 all counters are printed, otherwise only\n"
"                    those that have changed since the previous call.\n"
"                    Use optional <filter> for counter name substring match.\n"
"                    Uses shared memory segment 0xfefec003.\n"
" --reset|-z       : Reset counters\n"
" --slot|-s <num>  : A storage slot where counter history is kept.\n"
"                    Usable for different average times, e.g. slot 0 for\n"
"                    fast read and 1 for slow read\n"
;

/* ========================================================================= */


/*! pswtool main
 */
int main (int argc, char **argv)
{
    int c;
    char *s;
    int all = 0;
    struct filter filter;
    int slot = 0;
    int reset = 0;

    while (1)
    {
	int option_index = 0;
	static struct option long_options[] = {
	    {"show-counters", required_argument, 0, 'c'},
	    {"slot", required_argument, 0, 's'},
	    {"reset", no_argument, 0, 'z'},
	    {"help", no_argument, 0, 'h'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "c:hs:z", long_options, &option_index);

	if (c == -1)
	    break;

	switch (c)
	{
	case 's':
	    slot = atoi(optarg);
	    break;
	case 'z':
	    reset = 1;
	    break;
	case 'c':
	    s = strtok (optarg, ",");
	    if (s)
		all = atoi(s);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		make_filter(&filter, s);

	    if (pp_counters (&filter, all, slot, reset))
	    {
		PRERR("psw_counters");
		return 1;
	    }
	    break;

	case 'h':
	    printf (usage);
	    return 0;

	default:
	    fprintf (stderr, usage);
	    return 1;
	}
    }

    return 0;
}
/*! @} */
