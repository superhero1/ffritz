/*
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

#include "libticc.h"
#include "counters.h"

/* ========================================================================= */

#define SETERR(_s)  dev->ath_err = _s;
#define PRERR(...) {fprintf (stderr, "pcount :: ");\
    fprintf (stderr, __VA_ARGS__);\
    fprintf (stderr, " :: ERROR\n");\
    fflush(stderr);}

/*! \defgroup pcount puma6 switch tool
 * A tool for operating the internal l2 switch of the puma6 CPU on FritzBox
 * 6490 Cable.
 * 
 * The tool is VERY limited, since knowledge on API and registers is sparse.
 *
 * It supports
 *  - Controlling VLANs
 *  - Counter output (limited)
 *
 *
 * For usage details refer to "pcount -h".
 */
/*! @{ */

static const char *usage =
"pcount            : Counter tool \n"
" --help|-h        : This message\n"
#if !defined(ATOM_BUILD)
" --pp-counters|-p [<all>[,<filter>]]        (arm packet processor)\n"
" --l2sw-counters|-l <p>[,<all>[,<filter>]]  (internal l2 switch)\n"
#else
" --extsw-counters|-e <p>[,<all>[,<filter>]] (external switch)\n"
#endif
" --netif-counters|-i <p>[,<all>[,<filter>]] (network interfaces)\n"
"                  : Print counters of port <p> (-1 for all ports).\n"
"                    p is either a number or a port name.\n"
"                    If <all> is 1 all counters are printed,\n"
"                    otherwise only those that have changed since the \n"
"                    previous call.\n"
"                    Use optional <filter> for counter name/type match:\n"
"                     filter:=[substr][%%type]\n"
"                      substr : substring match for counter name\n"
"                      type   : byte/pkt for byte/packet counters\n"
" --reset|-z       : Reset max. rate after output\n"
" --slot|-s <num>  : A storage slot where counter history is kept.\n"
"                    Usable for different average times, e.g. slot 0 for\n"
"                    fast read and 1 for slow read\n"
" --prtg|-x <mode> : Output in PRTG extended sensor (XML) format:\n"
"                    1 : Absolute values\n"
"                    2 : Relative values\n"
"                    3 : Rate (1/s)\n"
"                    4 : Maximum rate (1/s)\n"
"                    5 : Both rate and maximum rate\n"
" --bps|-B             : Display rates in bits per second\n"
" --raw|-R             : Display rates as raw value\n"
;

/* ========================================================================= */


/*! pcount main
 */
int main (int argc, char **argv)
{
    int c;
    char *s;
    int all = 0;
    struct filter filter;
    struct port_id port;
    int reset = 0;
    int slot = 0;
    int pass;
#if defined(ATOM_BUILD)
    void *ath_dev = ath_st_alloc();
#endif

    for (pass = 0; pass < 2; pass++)
    {
        optind=1;

	if ((pass == 1) && prtg_mode())
	    printf ("<prtg>\n");

	while (1)
	{
	    int option_index = 0;
	    static struct option long_options[] = {
#if !defined(ATOM_BUILD)
		{"pp-counters", required_argument, 0, 'p'},
		{"l2sw-counters", required_argument, 0, 'l'},
#else
		{"extsw-counters", required_argument, 0, 'e'},
#endif
		{"netif-counters", required_argument, 0, 'i'},
		{"bps", no_argument, 0, 'B'},
		{"raw", no_argument, 0, 'R'},
		{"slot", required_argument, 0, 's'},
		{"reset", no_argument, 0, 'z'},
		{"prtg", required_argument, 0, 'x'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	    };


	    c = getopt_long (argc, argv, "e:p:l:i:s:zhx:BR", long_options, &option_index);

	    if (c == -1)
		break;

	    slot = 0;
	    memset (&filter, 0, sizeof(filter));
	    make_port(&port, NULL);

	    switch (c)
	    {
	    case 'R':
	    	set_hr_mode (0);
		break;
	    case 'B':
	        set_bps_mode ();
		break;
	    case 'x':
		set_prtg_mode((enum prtg_mode)atoi(optarg));
		break;
	    case 's':
		slot = atoi(optarg);
		break;
	    case 'z':
		reset = 1;
		break;
#if !defined(ATOM_BUILD)
	    case 'p':
	    	if (pass == 0)
		    break;

		s = strtok (optarg, ",");
		if (s)
		    all = atoi(s);
		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    make_filter(&filter, s);

		if (pp_counters (&filter, all, slot, reset))
		{
		    PRERR("pp_counters");
		}
		break;

	    case 'l':
	    	if (pass == 0)
		    break;

		s = strtok (optarg, ",");
		if (s)
		    make_port(&port, s);

		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    all = atoi(s);
		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    make_filter(&filter, s);

		if (psw_counters (&port, &filter, all, slot, reset))
		{
		    PRERR("psw_counters");
		}
		break;
#else
	    case 'e':
	    	if (pass == 0)
		    break;

		s = strtok (optarg, ",");
		if (s)
		    make_port(&port, s);
		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    all = atoi(s);
		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    make_filter(&filter, s);

		if (ath_counters (ath_dev, &port, &filter, all, slot, reset))
		{
		    PRERR("ath_counters");
		}
		break;
#endif

	    case 'i':
	    	if (pass == 0)
		    break;

		s = strtok (optarg, ",");
		if (s)
		    make_port(&port, s);
		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    all = atoi(s);
		if (s)
		    s = strtok (NULL, ",");
		if (s)
		    make_filter(&filter, s);

		if (netdev_counters (&port, &filter, all, slot, reset))
		{
		    PRERR("netdev_counters");
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
	if ((pass == 1) && prtg_mode())
	    printf ("</prtg>\n");
    }


    return 0;
}
/*! @} */

/* linker issues with libubacktrace.so .. but we dont need those ..
 */
void backtrace(void)
{
}

void backtrace_symbols_fd(void)
{
}
