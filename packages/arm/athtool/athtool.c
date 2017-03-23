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
#include <stdlib.h>

#include "athtool.h"
#include "libticc.h"

/* ========================================================================= */

const char *usage =
"athtool           : Atheros AR8327 switch tool\n"
" -h               : This message\n"
" -v               : Increase verbose level\n"
" --verbose\n"
" ==== Register Access ====\n"
" -r <adrs>        : Read register at offset\n"
" -w <ards>,<val>[,<mask>] : Write val to register, use optional mask\n"
" ==== Mirroring ====\n"
" -M <port>        : Set mirror-to port (0..6), or -1 to disable\n"
"                    If <port> is ?, print current mirror configuration.\n"
" -I [+]<port>     : Set/add ingress/egress mirror-from port\n"
" -E [+]<port>       (-1 to disable mirroring)\n"
"                    + allows adding multiple ports, otherwise only the given\n"
"                      port is mirrored.\n"
" ==== VLANs ====\n"
" -V               : Show VLAN setup of all ports/VLANs\n"
" -D <vid>         : Delete VLAN <vid>\n"
" -R <p>,<vid>     : Remove port <p> from VLAN <vid>\n"
" -A <p>[,<t>]     : Add port <p> to new VLAN (-C) with egress-tag mode t:\n"
"                     keep  : keep existing tag (default)\n"
"                     untag : remove tag\n"
"                     tag   : add/replace with PVID\n"
"                    One or more -A statements are parameters to a subsequent\n"
"                    create command:\n"
" -C <vid>         : Create VLAN <vid>, using previous -A statememts as port\n"
"                    list/attributes.\n"
" -P <p>,<vid>     : Assign default VID (PVID)\n"
" ==== Counters ====\n"
" -c <p>[,<all>[,<filter>]]\n"
"                  : Print counters of port <p> (-1 for all ports).\n"
"                    If <all> is 1 all counters are printed, otherwise only\n"
"                    those that have changed since the previous call.\n"
"                    Use optional <filter> for counter name substring match.\n"
"                    Uses shared memory segment 0xfefec002.\n"
" ==== long options ====\n"
"  -r : --read\n"
"  -w : --write\n"
"  -M : --mirror-to\n"
"  -I : --ingress-mirror\n"
"  -E : --egress-mirror\n"
"  -V : --vlan-show\n"
"  -C : --vlan-create\n"
"  -D : --vlan-delete\n"
"  -A : --vlan-add\n"
"  -R : --vlan-remove\n"
"  -P : --pvid-set\n"
"  -c : --show-counters\n"
"  -v : --verbose\n"
"  -h : --help\n"
;

int _ath_verbose = 0;
const char *_ath_err = NULL;



/* ========================================================================= */

/*! Read-modify-write a switch register
 *
 * \param reg	Register offset
 * \param mask	Which bits to modify. If all one register is not read,
 *		if all zero register is not written.
 * \param value	mask bits values
 * \param err	optional error pointer: set to 1 on error, otherwise not touched
 *
 * \returns The register value on success, 0xffffffff on error
 */
uint32_t ath_rmw (uint32_t reg, uint32_t mask, uint32_t value, int *err)
{
    uint32_t tmp = 0;

    if (mask != 0xffffffff)
    {
	if (extSwitchReadAthReg (reg, &tmp))
	{
	    if (err)
	    {
		*err = 1;
		SETERR("extSwitchReadAthReg failed");
	    }
	    return 0xffffffff;
	}
	
	verb_printf (2, "ath_rmw: 0x%04x -> 0x%08x\n", reg, tmp);
    }

    if (mask != 0)
    {
	tmp = (tmp & ~mask) | (value & mask);

	if (extSwitchWriteAthReg (reg, tmp))
	{
	    if (err)
	    {
		*err = 1;
		SETERR("extSwitchReadAthReg failed");
	    }
	    return 0xffffffff;
	}

	verb_printf (2, "ath_rmw: 0x%04x[0x%08x] <- 0x%08x\n", reg, mask, tmp);
    }

    return tmp;
}

/*! Set mirror-to port 
 *
 * \param port	port number (0..6) or -1 to disable mirroring
 *
 * \returns 0 on success, 1 on error
 */
int ath_mirror_to (int port)
{
    int rc = 0;

    if ((port < -1) || (port > 6))
    {
	SETERR("bad port index");
	return 1;
    }

    ath_rmw (AR8327_REG_FWD_CTRL0,
	AR8327_FWD_CTRL0_MIRROR_PORT,
	(port << AR8327_FWD_CTRL0_MIRROR_PORT_S), &rc);

    return rc;
}

/*! Set ingress mirror source
 *
 * \param port	port number (0..6) or -1 to disable ingress mirror
 * \param multi	0 to allow only one mirror source (all others will be disabled),
 *		1 to allow multiple
 *
 * \returns 0 on success, 1 on error
 */
int ath_ig_mirror_from (int port, int multi)
{
    int i;
    uint32_t reg;
    int rc = 0;

    if ((port < -1) || (port > 6))
    {
	SETERR("bad port index");
	return 1;
    }

    if (port == -1)
	multi = 0;

    for (i = 0; i < 7; i++)
    {
	reg = AR8327_REG_PORT_LOOKUP(i);

	if (i == port)
	{
	    ath_rmw (reg,
		    AR8327_PORT_LOOKUP_ING_MIRROR_EN,
		    AR8327_PORT_LOOKUP_ING_MIRROR_EN,
		    &rc);
	}
	else if (!multi)
	{
	    ath_rmw (reg,
		    AR8327_PORT_LOOKUP_ING_MIRROR_EN,
		    0,
		    &rc);
	}
    }

    return rc;
}

/*! Set egress mirror source
 *
 * \param port	port number (0..6) or -1 to disable egress mirror
 * \param multi	0 to allow only one mirror source (all others will be disabled),
 *		1 to allow multiple
 *
 * \returns 0 on success, 1 on error
 */
int ath_eg_mirror_from (int port, int multi)
{
    int i;
    uint32_t reg;
    int rc = 0;

    if ((port < -1) || (port > 6))
    {
	SETERR("bad port index");
	return 1;
    }

    if (port == -1)
	multi = 0;

    for (i = 0; i < 7; i++)
    {
	reg = AR8327_REG_PORT_HOL_CTRL1(i);

	if (i == port)
	{
	    ath_rmw (reg,
		    AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN,
		    AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN,
		    &rc);
	}
	else if (!multi)
	{
	    ath_rmw (reg,
		    AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN,
		    0,
		    &rc);
	}
    }

    return rc;
}

void ath_mirror_show (void)
{
    uint32_t tmp;
    int i;
    int rc = 0;

    printf ("mirror-to           : ");

    tmp = ath_rmw (AR8327_REG_FWD_CTRL0, 0, 0, &rc);
    if (rc)
    {
	printf ("FAILED\n");
    }
    else
    {
	tmp = (tmp & AR8327_FWD_CTRL0_MIRROR_PORT) >> AR8327_FWD_CTRL0_MIRROR_PORT_S;
	if (tmp < 7)
	    printf ("%d\n", tmp);
	else
	    printf ("DISABLED\n");
    }

    printf ("ingress-mirror-from : ");
    for (i = 0; i < 7; i++)
    {
	tmp = ath_rmw (AR8327_REG_PORT_LOOKUP(i), 0, 0, &rc);
	if (rc)
	{
	    rc = 0;
	    continue;
	}
	
	if (tmp & AR8327_PORT_LOOKUP_ING_MIRROR_EN)
	    printf ("%d ", i);
    }
    printf ("\n");

    printf ("egress-mirror-from  : ");
    for (i = 0; i < 7; i++)
    {
	tmp = ath_rmw (AR8327_REG_PORT_HOL_CTRL1(i), 0, 0, &rc);
	if (rc)
	{
	    rc = 0;
	    continue;
	}
	
	if (tmp & AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN)
	    printf ("%d ", i);
    }
    printf ("\n");
}


int main (int argc, char **argv)
{
    int c;
    uint32_t reg, val, mask;
    char *s;
    int multi;
    int src_port;
    int rc;
    uint32_t port, mode, vid;
    int all = 0;
    const char *filter = NULL;

    /* default vlan create attributes
     */
    mode = AR8327_VTU_FUNC0_IVL|AR8327_VTU_FUNC0_LLD|AR8327_VTU_FUNC0_PO|AR8327_VTU_FUNC0_EG_MODE;

    while (1)
    {
	int option_index = 0;
	static struct option long_options[] = {
	    {"read", required_argument, 0, 'r'},
	    {"write", required_argument, 0, 'w'},
	    {"mirror-to", required_argument, 0, 'M'},
	    {"ingress-mirror", required_argument, 0, 'I'},
	    {"egress-mirror", required_argument, 0, 'E'},
	    {"vlan-show", no_argument, 0, 'V'},
	    {"vlan-create", required_argument, 0, 'C'},
	    {"vlan-delete", required_argument, 0, 'D'},
	    {"vlan-add", required_argument, 0, 'A'},
	    {"vlan-remove", required_argument, 0, 'R'},
	    {"pvid-set", required_argument, 0, 'P'},
	    {"show-counters", required_argument, 0, 'c'},
	    {"verbose", no_argument, 0, 'v'},
	    {"help", no_argument, 0, 'h'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "Vr:w:hM:E:I:vC:D:A:R:P:c:", long_options, &option_index);

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

	    if (ath_counters (port, filter, all))
	    {
		PRERR("ath_counters");
		return 1;
	    }
	    break;


	case 'r':
	    reg = strtoul (optarg, NULL, 0);

	    printf ("0x%03x : ", reg);

	    rc = 0;
	    val = ath_rmw (reg, 0, 0, &rc);

	    if (rc)
	    {
		printf ("ERR\n");
		return 1;
	    }

	    printf ("0x%08x\n", val);

	    break;

	case 'w':
	    mask = 0xffffffff;
	    s = strtok (optarg, ",");
	    if (s)
		reg = strtoul (s, NULL, 0);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		val = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    s = strtok (NULL, ",");
	    if (s)
		mask = strtoul (s, NULL, 0);

	    rc = 0;
	    ath_rmw (reg, mask, val, &rc);

	    if (rc)
	    {
		printf ("ERR\n");
		return 1;
	    }

	    break;
	
	case 'M':
	    if (!strcmp (optarg, "?"))
	    {
		ath_mirror_show();
		break;
	    }

	    if (ath_mirror_to (atoi(optarg)))
	    {
		PRERR ("ath_mirror_to");
		return 1;
	    }
	    break;

	case 'E':
	    if (optarg[0] == '+')
	    {
		multi = 1;
		src_port = atoi (&optarg[1]);
	    }
	    else
	    {
		multi = 0;
		src_port = atoi (optarg);
	    }

	    if (ath_eg_mirror_from (src_port, multi))
	    {
		PRERR ("ath_eg_mirror_from (%d %d)", src_port, multi);
		return 1;
	    }
	    break;

	case 'I':
	    if (optarg[0] == '+')
	    {
		multi = 1;
		src_port = atoi (&optarg[1]);
	    }
	    else
	    {
		multi = 0;
		src_port = atoi (optarg);
	    }

	    if (ath_ig_mirror_from (src_port, multi))
	    {
		PRERR("ath_ig_mirror_from (%d %d)",
		    src_port, multi);
		return 1;
	    }
	    break;
	
	case 'V':
	    ath_vlan_show();
	    break;

	case 'C':
	    if ((mode & AR8327_VTU_FUNC0_EG_MODE) == AR8327_VTU_FUNC0_EG_MODE)
	    {
		PRERR("No ports defined for new VLAN. -C switch requires at least one -A\n");
		return 1;
	    }

	    if (ath_vlan_create (strtoul (optarg, NULL, 0), mode))
	    {
		PRERR("ath_vlan_create");
		return 1;
	    }
	    break;

	case 'D':
	    if (ath_vlan_delete (atoi(optarg)))
	    {
		PRERR("ath_vlan_delete");
		return 1;
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
		val = AR8327_VTU_FUNC0_EG_MODE_KEEP;
	    else if (!strcmp (s, "untag"))
		val = AR8327_VTU_FUNC0_EG_MODE_UNTAG;
	    else if (!strcmp (s, "tag"))
		val = AR8327_VTU_FUNC0_EG_MODE_TAG;
	    else
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    mode = ath_attr_set_port (mode, port, val);
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

	    if (ath_vlan_port_rm (vid, port))
	    {
		PRERR("ath_vlan_port_rm");
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

	    if (ath_pvid_port (port, vid))
	    {
		PRERR("ath_pvid_port");
		return 1;
	    }

	case 'v':
	    _ath_verbose++;
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
