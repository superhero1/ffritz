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
#include <fcntl.h>
#include <unistd.h>

#include "athtool.h"
#include "libticc.h"
#include "counters.h"

/* ========================================================================= */

/*! \defgroup athtool Atheros AR8327 switch tool
 * A tool for operating an Atheros AR8327 switch on FritzBox 6490 Cable.
 *
 * It supports
 *  - Register access
 *  - Port mirroring
 *  - Controlling VLANs
 *  - Counter output, rate measurement
 *  - L2 table handling
 *
 * Register access is accomplished via reverse-engineered MDIO functions from
 * libticc.so.
 *
 * For usage details refer to "athtool -h".
 */
/*! @{ */

static const char *usage =
"athtool           : Atheros AR8327 switch tool\n"
" --help|-h        : This message\n"
" --verbose|-v     : Increase verbose level\n"
" ==== Register Access ====\n"
" --read|-r <adrs> : Read register at offset\n"
" --write|-w <ards>,<val>[,<mask>]\n"
"                  : Write val to register, use optional mask\n"
" ==== Mirroring ====\n"
" --mirror-to|-M <port>\n"
"                  : Set mirror-to port (0..6), or -1 to disable\n"
"                    If <port> is ?, print current mirror configuration.\n"
" --ingress-mirror|-I [+]<port>\n"
" --egress-mirror|-E [+]<port>\n"
"                  : Set/add ingress/egress mirror-from port\n"
"                    (-1 to disable mirroring).\n"
"                    + allows adding multiple ports, otherwise only the given\n"
"                      port is mirrored.\n"
" ==== VLANs ====\n"
" --vlan-show|-V   : Show VLAN setup of all ports/VLANs\n"
" --vlan-delete|-D <vid>\n"
"                  : Delete VLAN <vid>\n"
" --vlan-remove|-R <p>,<vid>\n"
"                  : Remove port <p> from VLAN <vid>\n"
" --vlan-add|-A <p>[,<t>]\n"
"                  : Add port <p> to new VLAN (-C) with egress-tag mode t:\n"
"                     keep  : keep existing tag (default)\n"
"                     untag : remove tag\n"
"                     tag   : add/replace with PVID\n"
"                    One or more -A statements are parameters to a subsequent\n"
"                    create command:\n"
" --vlan-create|-C <vid>\n"
"                  : Create VLAN <vid>, using previous -A statememts as port\n"
"                    list/attributes.\n"
" --pvid-set|-P <p>,<vid>\n"
"                  : Assign default VID (CVID)\n"
" --svid-set|-S <p>,<vid>\n"
"                  : Assign default SVID (SVID)\n"
" --port-flags|-F <p>,<flags>\n"
"                  : Colon separated list of port flags (name=value)\n"
"                    EGTYPE : only tagged frames can be\n"
"                    OUTMODE : 0=no modify, 1=no tag, 2=with tag\n"
"                    INMODE : 0=all, 1=tag only, 2=untagged only\n"
"                    SPCHECK : Source port check enable\n"
"                    CORE : 1=core, 0=edge port\n"
"                    FRC_VID : force to use port default VID\n"
"                    TLS : 1=TLS mode\n"
"                    PROP_EN : 1=enable port base vlan propagate\n"
"                    CLONE_EN : 1=enable port clone\n"
"                    PRIO : vlan priority propagation enable\n"
" ==== Counters ====\n"
" --show-counters|-c <p>[,<all>[,<filter>]]\n"
"                  : Print counters of port <p> (-1 for all ports).\n"
"                    If <all> is 1 all counters are printed, otherwise only\n"
"                    those that have changed since the previous call.\n"
"                    Use optional <filter> for counter name substring match.\n"
"                    Uses shared memory segment 0xfefec002.\n"
" ==== L2 Table ====\n"
" --l2-show|-t     : Dump L2 table\n"
" --l2-add|-m <MAC>,<ports>[,<opt>]\n"
"                  : Add static L2 entry with given MAC.\n"
"                    <ports> is a comma separated list of ports\n"
"                    (at least on e is required).\n"
"                    <opt> is a list of options:\n"
"        VID=<vid> : VLAN id (required)\n"
"        PRI=<pri> : Priority field override\n"
"      SVL_LEARNED : SVL learned instead of IVL learned\n"
"           MIRROR : Mirror to mirrot-port\n"
"             DROP : Drop packets\n"
"            LEAKY : leaky VLAN enable\n"
"        REDIR_CPU : Redirect to CPU port\n"
"         COPY_CPU : Copy to CPU port\n"
" --l2-del|-d <MAC>,<ports>[,<opt>]\n"
"                  : Delete specific L2 entry. Same syntax as -m.\n"
"                    MAC address, ports and VID must match.\n"
;

int _ath_verbose = 0;

#define AR8327_VTU_FUNC0_DFL (AR8327_VTU_FUNC0_IVL|AR8327_VTU_FUNC0_PO|AR8327_VTU_FUNC0_EG_MODE)

/* ========================================================================= */

#if defined(ATOM_BUILD)

/* an ungly hack to access switch registers via switch_reg proc entry.
 * reg write is just writing, reg read is extracting the read result from the 
 * kernel log :-(
 */

#define SWREG_FILE "/proc/driver/avmnet/ar8327/switch_reg"
#define KMSG "/proc/kmsg"

int extSwitchReadAthReg (uint32_t reg, uint32_t *val)
{
    int fd;
    int kmsg_fd;
    char cmd[100];
    char kmsg[1000];
    char *s;

    fd = open (SWREG_FILE, O_RDWR);
    if (fd == -1)
    {
	perror (SWREG_FILE);
	exit(1);
    }

    kmsg_fd = open (KMSG, O_RDWR|O_NONBLOCK);
    if (kmsg_fd == -1)
    {
	perror (KMSG);
	exit(1);
    }

    /* flush /kmsg */
    while (read(kmsg_fd, kmsg, sizeof(kmsg)) != -1)
	;

    sprintf (cmd, "read 0x%x\n", reg);

    if (write(fd, cmd, strlen(cmd)) == -1)
    {
	perror ("write" SWREG_FILE);
	return 1;
    }

    /* get read response from kmsg, hope that its already there
     * ena no one else is using this feature in parallel
     */
    memset (kmsg, 0, sizeof(kmsg));
    if (read(kmsg_fd, kmsg, sizeof(kmsg)) == -1)
    {
	perror ("read " KMSG);
	exit (1);
    }

    close (fd);
    close (kmsg_fd);

    s = strstr(kmsg, "register_read");
    if (s)
	s = strstr(s, "=");

    if (s)
	*val = strtoul(s+1, NULL, 16);
    else
	return 1;

    return 0;
}

int extSwitchWriteAthReg (uint32_t reg, uint32_t val)
{
    int fd;
    char cmd[100];

    fd = open (SWREG_FILE, O_RDWR);
    if (fd == -1)
    {
	perror (SWREG_FILE);
	exit(1);
    }

    sprintf (cmd, "write 0x%x 0x%x\n", reg, val);

    if (write(fd, cmd, strlen(cmd)) == -1)
    {
	perror ("write" SWREG_FILE);
	return 1;
    }
    return 0;
}


#endif

/*! Read-modify-write a switch register
 *
 * \param dev	Device handle
 * \param reg	Register offset
 * \param mask	Which bits to modify. If all one register is not read,
 *		if all zero register is not written.
 * \param value	mask bits values
 * \param err	optional error pointer: set to 1 on error, otherwise not touched
 *
 * \returns The register value on success, 0xffffffff on error
 */
uint32_t ath_rmw (struct ath_dev *dev, uint32_t reg, uint32_t mask, uint32_t value, int *err)
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
 * \param dev	Device handle
 * \param port	port number (0..6) or -1 to disable mirroring
 *
 * \returns 0 on success, 1 on error
 */
int ath_mirror_to (struct ath_dev *dev, int port)
{
    int rc = 0;

    if ((port < -1) || (port >= dev->num_ports))
    {
	SETERR("bad port index");
	return 1;
    }

    ath_rmw (dev, AR8327_REG_FWD_CTRL0,
	AR8327_FWD_CTRL0_MIRROR_PORT,
	(port << AR8327_FWD_CTRL0_MIRROR_PORT_S), &rc);

    return rc;
}

/*! Set ingress mirror source
 *
 * \param dev	Device handle
 * \param port	port number (0..6) or -1 to disable ingress mirror
 * \param multi	0 to allow only one mirror source (all others will be disabled),
 *		1 to allow multiple
 *
 * \returns 0 on success, 1 on error
 */
int ath_ig_mirror_from (struct ath_dev *dev, int port, int multi)
{
    int i;
    uint32_t reg;
    int rc = 0;

    if ((port < -1) || (port >= dev->num_ports))
    {
	SETERR("bad port index");
	return 1;
    }

    if (port == -1)
	multi = 0;

    for (i = 0; i < dev->num_ports; i++)
    {
	reg = AR8327_REG_PORT_LOOKUP(i);

	if (i == port)
	{
	    ath_rmw (dev, reg,
		    AR8327_PORT_LOOKUP_ING_MIRROR_EN,
		    AR8327_PORT_LOOKUP_ING_MIRROR_EN,
		    &rc);
	}
	else if (!multi)
	{
	    ath_rmw (dev, reg,
		    AR8327_PORT_LOOKUP_ING_MIRROR_EN,
		    0,
		    &rc);
	}
    }

    return rc;
}

/*! Set egress mirror source
 *
 * \param dev	Device handle
 * \param port	port number (0..n) or -1 to disable egress mirror
 * \param multi	0 to allow only one mirror source (all others will be disabled),
 *		1 to allow multiple
 *
 * \returns 0 on success, 1 on error
 */
int ath_eg_mirror_from (struct ath_dev *dev, int port, int multi)
{
    int i;
    uint32_t reg;
    int rc = 0;

    if ((port < -1) || (port >= dev->num_ports))
    {
	SETERR("bad port index");
	return 1;
    }

    if (port == -1)
	multi = 0;

    for (i = 0; i < dev->num_ports; i++)
    {
	reg = AR8327_REG_PORT_HOL_CTRL1(i);

	if (i == port)
	{
	    ath_rmw (dev, reg,
		    AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN,
		    AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN,
		    &rc);
	}
	else if (!multi)
	{
	    ath_rmw (dev, reg,
		    AR8327_PORT_HOL_CTRL1_EG_MIRROR_EN,
		    0,
		    &rc);
	}
    }

    return rc;
}

/*! Show mirroring configuration
 *
 * \param dev device handle
 */
void ath_mirror_show (struct ath_dev *dev)
{
    uint32_t tmp;
    int i;
    int rc = 0;

    printf ("mirror-to           : ");

    tmp = ath_rmw (dev, AR8327_REG_FWD_CTRL0, 0, 0, &rc);
    if (rc)
    {
	printf ("FAILED\n");
    }
    else
    {
	tmp = (tmp & AR8327_FWD_CTRL0_MIRROR_PORT) >> AR8327_FWD_CTRL0_MIRROR_PORT_S;
	if (tmp < dev->num_ports)
	    printf ("%d\n", tmp);
	else
	    printf ("DISABLED\n");
    }

    printf ("ingress-mirror-from : ");
    for (i = 0; i < dev->num_ports; i++)
    {
	tmp = ath_rmw (dev, AR8327_REG_PORT_LOOKUP(i), 0, 0, &rc);
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
    for (i = 0; i < dev->num_ports; i++)
    {
	tmp = ath_rmw (dev, AR8327_REG_PORT_HOL_CTRL1(i), 0, 0, &rc);
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


/*! athtool main
 */
int main (int argc, char **argv)
{
    int c;
    uint32_t reg, val, mask;
    char *s;
    int multi;
    int src_port;
    int rc;
    uint32_t mode, vid;
    int all = 0;
    struct filter filter;
    struct ath_dev *dev;
    struct ath_arl_entry entry;
    struct port_id port;
    uint32_t avalue, amask;

    dev = calloc (1, sizeof(*dev));
    if (!dev)
    {
	perror ("calloc");
	return 1;
    }

    if (extSwitchReadAthReg (0, &dev->dev_id))
    {
	fprintf (stderr, "athtool :: Failed to read ID register\n");
	return 1;
    }

    switch (dev->dev_id & 0xff00)
    {
	case 0x1200:	dev->num_ports = 7; break;
	default:	fprintf (stderr, "athtool :: Warning: unknown device ID: 0x%x\n", dev->dev_id);
			dev->num_ports = 7; break;
    }
    dev->ath_rmw = ath_rmw;


    /* default vlan create attributes
     */
    mode = AR8327_VTU_FUNC0_DFL;

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
	    {"svid-set", required_argument, 0, 'S'},
	    {"port-flags", required_argument, 0, 'F'},
	    {"show-counters", required_argument, 0, 'c'},
	    {"l2-show", no_argument, 0, 't'},
	    {"l2-add", required_argument, 0, 'm'},
	    {"l2-del", required_argument, 0, 'd'},
	    {"verbose", no_argument, 0, 'v'},
	    {"help", no_argument, 0, 'h'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "Vr:w:hM:E:I:vC:D:A:R:P:S:c:tm:d:F:", long_options, &option_index);

	if (c == -1)
	    break;

	switch (c)
	{
	case 'm':
	    if (ath_arl_flags_parse (dev, &entry, optarg))
	    {
		PRERR("ath_arl_flags_parse");
		return 1;
	    }
	    if (entry.ports == 0)
	    {
		fprintf (stderr, "athtool :: at least one port must be given\n");
		return 1;
	    }
	    if (entry.vid == 0)
	    {
		fprintf (stderr, "athtool :: VID attribute must be given and not be 0\n");
		return 1;
	    }

	    if (ath_arl_add (dev, &entry))
	    {
		PRERR("ath_arl_add");
		return 1;
	    }
	    break;

	case 'd':
	    if (ath_arl_flags_parse (dev, &entry, optarg))
	    {
		PRERR("ath_arl_flags_parse");
		return 1;
	    }
	    if (entry.ports == 0)
	    {
		fprintf (stderr, "athtool :: at least one port must be given\n");
		return 1;
	    }
	    if (entry.vid == 0)
	    {
		fprintf (stderr, "athtool :: VID attribute must be given and not be 0\n");
		return 1;
	    }


	    if (ath_arl_rm (dev, &entry))
	    {
		PRERR("ath_arl_rm");
		return 1;
	    }
	    break;
	case 't':
	    ath_arl_dump (dev);
	    break;

	case 'c':
	    make_port(&port, NULL);
	    memset (&filter, 0, sizeof(filter));

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

	    if (ath_counters (dev, &port, &filter, all, 0, 0))
	    {
		PRERR("ath_counters");
		return 1;
	    }
	    break;


	case 'r':
	    reg = strtoul (optarg, NULL, 0);

	    printf ("0x%03x : ", reg);

	    rc = 0;
	    val = ath_rmw (dev, reg, 0, 0, &rc);

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
	    ath_rmw (dev, reg, mask, val, &rc);

	    if (rc)
	    {
		printf ("ERR\n");
		return 1;
	    }

	    break;
	
	case 'M':
	    if (!strcmp (optarg, "?"))
	    {
		ath_mirror_show(dev);
		break;
	    }

	    if (ath_mirror_to (dev, atoi(optarg)))
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

	    if (ath_eg_mirror_from (dev, src_port, multi))
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

	    if (ath_ig_mirror_from (dev, src_port, multi))
	    {
		PRERR("ath_ig_mirror_from (%d %d)",
		    src_port, multi);
		return 1;
	    }
	    break;
	
	case 'V':
	    ath_vlan_show(dev);
	    break;

	case 'C':
	    if ((mode & AR8327_VTU_FUNC0_EG_MODE) == AR8327_VTU_FUNC0_EG_MODE)
	    {
		PRERR("No ports defined for new VLAN. -C switch requires at least one -A\n");
		return 1;
	    }

	    if (ath_vlan_create (dev, strtoul (optarg, NULL, 0), mode))
	    {
		PRERR("ath_vlan_create");
		return 1;
	    }

	    /* reset mode for next VLAN definition */
	    mode = AR8327_VTU_FUNC0_DFL;
	    break;

	case 'D':
	    if (ath_vlan_delete (dev, atoi(optarg)))
	    {
		PRERR("ath_vlan_delete");
		return 1;
	    }
	    break;

	case 'A':
	    s = strtok (optarg, ",");
	    if (s)
		make_port(&port, s);

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

	    mode = ath_attr_set_port (dev, mode, port.num, val);
	    break;
	    
	case 'R':
	    s = strtok (optarg, ",");
	    if (s)
		make_port(&port, s);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		vid = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    if (ath_vlan_port_rm (dev, vid, port.num))
	    {
		PRERR("ath_vlan_port_rm");
		return 1;
	    }
	    break;

	case 'P':
	    s = strtok (optarg, ",");
	    if (s)
		make_port(&port, s);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		vid = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    if (ath_pvid_port (dev, port.num, vid))
	    {
		PRERR("ath_pvid_port");
		return 1;
	    }
	    break;

	case 'S':
	    s = strtok (optarg, ",");
	    if (s)
		make_port(&port, s);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		vid = strtoul (s, NULL, 0);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    if (ath_svid_port (dev, port.num, vid))
	    {
		PRERR("ath_svid_port");
		return 1;
	    }
	    break;

	case 'F':
		
	    s = strtok (optarg, ",");
	    if (s)
		make_port(&port, s);
	    if (s)
		s = strtok (NULL, ",");
	    if (s)
		ath_port_vlan_attr_parse(s, &avalue, &amask);

	    if (!s)
	    {
		fprintf (stderr, usage);
		return 1;
	    }

	    if (ath_vattr_port (dev, port.num, avalue, amask))
	    {
		PRERR("ath_vattr_port");
		return 1;
	    }
	    break;

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
/*! @} */
