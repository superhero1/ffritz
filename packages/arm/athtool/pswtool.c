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

#include "libticc.h"

/* ========================================================================= */

#define SETERR(_s)  dev->ath_err = _s;
#define PRERR(...) {fprintf (stderr, "pswtool :: ");\
    fprintf (stderr, __VA_ARGS__);\
    fprintf (stderr, " :: ERROR\n");\
    fflush(stderr);}

/*! \defgroup pswtool puma6 switch tool
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
 * For usage details refer to "pswtool -h".
 */
/*! @{ */

static const char *usage =
"pswtool           : puma6 l2 switch tool (USE WITH CARE)\n"
" --help|-h        : This message\n"
" ==== VLANs ====\n"
" --vlan-remove|-R <p>,<vid>\n"
"                  : Remove port <p> from VLAN <vid>\n"
" --vlan-add|-A <p>[,<t>]\n"
"                  : Add port <p> to new VLAN (-C) with egress-tag mode t:\n"
"                     untag : remove tag\n"
"                     tag   : add/replace with PVID\n"
"                    One or more -A statements are parameters to a subsequent\n"
"                    create command:\n"
" --vlan-create|-C <vid>\n"
"                  : Create VLAN <vid>, using previous -A statememts as port\n"
"                    list/attributes.\n"
" --pvid-set|-P <p>,<vid>\n"
"                  : Assign default VID (PVID)\n"
" ==== Counters ====\n"
" --show-counters|-c <p>[,<all>[,<filter>]]\n"
"                  : Print counters of port <p> (-1 for all ports).\n"
"                    If <all> is 1 all counters are printed, otherwise only\n"
"                    those that have changed since the previous call.\n"
"                    Use optional <filter> for counter name substring match.\n"
"                    Uses shared memory segment 0xfefec002.\n"
;

/* ========================================================================= */

extern int psw_counters (int port, const char *filter, int all);

/*! pswtool main
 */
int main (int argc, char **argv)
{
    int c;
    uint32_t val;
    char *s;
    int rc;
    uint32_t port, mode, vid;
    int all = 0;
    const char *filter = NULL;

    /* default vlan create attributes
     */
    mode = 0;

    while (1)
    {
	int option_index = 0;
	static struct option long_options[] = {
	    {"vlan-show", no_argument, 0, 'V'},
	    {"vlan-create", required_argument, 0, 'C'},
	    {"vlan-delete", required_argument, 0, 'D'},
	    {"vlan-add", required_argument, 0, 'A'},
	    {"vlan-remove", required_argument, 0, 'R'},
	    {"pvid-set", required_argument, 0, 'P'},
	    {"show-counters", required_argument, 0, 'c'},
	    {"help", no_argument, 0, 'h'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "Vr:w:hM:E:I:vC:D:A:R:P:c:tm:d:", long_options, &option_index);

	if (c == -1)
	    break;

	switch (c)
	{
	case 'c':
	    s = strtok (optarg, ",");
	    if (s)
		port = atoi(s);
	    else
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    s = strtok (NULL, ",");
	    if (s)
		all = atoi(s);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		filter = strdup (s);

	    if (psw_counters (port, filter, all))
	    {
		PRERR("psw_counters");
		return 1;
	    }
	    break;

	case 'C':
	    if (mode == 0)
	    {
		PRERR("No ports defined for new VLAN. -C switch requires at least one -A\n");
		return 1;
	    }

	    vid = strtoul (optarg, NULL, 0);

	    if (vid == 0)
	    {
		PRERR("VID must not be 0");
		return 1;
	    }

	    for (port = 0; port < 8; port++)
	    {
		val = (mode >> (port*2)) & 3;

		if (!val)
		    continue;

		printf ("port %d: vid=%d val=%d\n", port, vid, val);

		rc = L2SWITCH_AddPortToVlan (port, vid, (val == 2) ? 0 : 1);


		if (rc)
		{
		    PRERR("L2SWITCH_AddPortToVlan (port %d) failed", port);
		    return 1;
		}
	    }
	    break;

	case 'A':
	    s = strtok (optarg, ",");
	    if (s)
		port = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    s = strtok (NULL, ",");
	    if ((s == NULL) || (!strcmp (s, "keep")))
		val = 1;
	    else if (!strcmp (s, "untag"))
		val = 2;
	    else if (!strcmp (s, "tag"))
		val = 3;
	    else
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    mode |= val << (port*2);
	    break;
	    
	case 'R':
	    s = strtok (optarg, ",");
	    if (s)
		port = strtoul (s, NULL, 0);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		vid = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    if (L2SWITCH_RemovePortFromVlan (port, vid))
	    {
		PRERR("L2SWITCH_RemovePortFromVlan failed");
		return 1;
	    }
	    break;

	case 'P':
	    s = strtok (optarg, ",");
	    if (s)
		port = strtoul (s, NULL, 0);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		vid = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    if (L2SWITCH_SetPortNativeVlan (port, vid))
	    {
		PRERR("L2SWITCH_SetPortNativeVlan");
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
