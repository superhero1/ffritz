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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/shm.h>


#include "athtool.h"

#define DATA1_FLAGS (AR8327_ATU_PRI | AR8327_ATU_SVL_LEARNED | AR8327_ATU_PRI_OVR | AR8327_ATU_MIRROR | AR8327_ATU_DROP)
#define DATA2_FLAGS (AR8327_ATU_STATUS | AR8327_ATU_LEAKY | AR8327_ATU_REDIR_CPU | AR8327_ATU_COPY_CPU | AR8327_ATU_SHORT_LOOP)

/*! \ingroup athtool */
/*! @{ */

/*! Wait for ATU BUSY clear
 *
 * \param dev device handle
 *
 * \returns 0 on success, 1 on error, 2 if ATU_FUNC_FULL_VIO is set
 */
static int atu_wait (struct ath_dev *dev)
{
    int i;
    uint32_t tmp;
    int rc = 0;

    for (i = 0; i < 100; i++)
    {
	tmp = ath_rmw (dev, AR8327_REG_ATU_FUNC, 0, 0, &rc);
	if (rc)
	    return 1;
	
	if ((tmp & AR8327_ATU_FUNC_BUSY) == 0)
	{
	    if (tmp & AR8327_ATU_FUNC_FULL_VIO)
	    {
		SETERR("ARL table full or entry not found");
		return 2;
	    }
	    return 0;
	}
	
	usleep (10);
    }
    SETERR("ATU timeout");
    return 1;
}

/*! Parse L2 definitition string and encode data
 *
 * The format of the string is MAC,port-list[,options], where
 * - MAC is a MAC address in colon format (aa:bb:cc:dd:ee:ff)
 * - port-list is a comma separated list of ports (0..n)
 * - options is a comma separated list of options:
 *	- VID=n (VLAN ID)
 *	- PRI=n (to override priority with given level)
 *	- SVL_LEARNED
 *	- MIRROR
 *	- DROP
 *	- LEAKY
 *	- REDIR_CPU
 *	- COPY_CPU
 *
 * \param dev device handle
 * \param entry pointer of entry to be filled
 * \param entry_spec string containing L2 entry spec
 *
 * \returns 0 on success, 1 on error
 */
int ath_arl_flags_parse (struct ath_dev *dev, struct ath_arl_entry *entry, char *entry_spec)
{
    char *s;
    char *flags;
    char tmp[20];
    int i;
    static struct
    {
	const char *ID;
	uint32_t flag;
    }
    flag_list[] =
    {
	{ "SVL_LEARNED", AR8327_ATU_SVL_LEARNED },
	{ "PRI", AR8327_ATU_PRI_OVR },
	{ "MIRROR", AR8327_ATU_MIRROR },
	{ "DROP", AR8327_ATU_DROP },
	{ "LEAKY", AR8327_ATU_LEAKY },
	{ "REDIR_CPU", AR8327_ATU_REDIR_CPU },
	{ "COPY_CPU", AR8327_ATU_COPY_CPU },
	{ "SHORT_LOOP", AR8327_ATU_SHORT_LOOP },
	{ NULL, 0 }
    };

    if (!entry || !entry_spec)
    {
	SETERR("bad arguments");
	return 1;
    }

    flags = alloca (strlen(entry_spec)+1);
    strcpy (flags, entry_spec);

    memset (entry, 0, sizeof(*entry));

    s = strtok (entry_spec, ":");
    for (i = 0; i < 6; i++)
    {
	if (!s)
	{
	    SETERR ("malformed MAC address");
	    return 1;
	}
	entry->mac[i] = strtoul (s, NULL, 16);

	s = strtok (NULL, ":,");
    }

    verb_printf (1, "MAC=%x:%x:%x:%x:%x:%x\n",
	entry->mac[0],
	entry->mac[1],
	entry->mac[2],
	entry->mac[3],
	entry->mac[4],
	entry->mac[5]);

    if (!s)
	return 0;

    /* next ist list of ports */
    while (s && (*s >= '0') && (*s <= '9'))
    {
	entry->ports |= 1<<atoi(s);

	s = strtok (NULL, ",");
    }
    verb_printf (1, "ports=0x%x\n", entry->ports);

    if (!s)
	return 0;

    verb_printf (1, "flags=%s\n", flags);

    for (i = 0; flag_list[i].ID != NULL; i++)
    {
	sprintf (tmp, "-%s", flag_list[i].ID);
	if (strstr(flags, tmp))
	{
	    verb_printf (1, "-0x%x\n", flag_list[i].flag);
	    entry->flags &= ~flag_list[i].flag;
	    continue;
	}
	if (strstr (flags, flag_list[i].ID))
	{
	    verb_printf (1, "+0x%x\n", flag_list[i].flag);
	    entry->flags |= flag_list[i].flag;
	}
    }

    s = strstr (flags, "PRI=");

    if (s)
    {
	uint32_t pri;

	if (sscanf (s, "PRI=%d", &pri) == 1)
	{
	    entry->flags = (entry->flags & ~AR8327_ATU_PRI) |
			    ((pri << AR8327_ATU_PRI_S) & AR8327_ATU_PRI);
	}
    }

    s = strstr (flags, "VID=");
    if (s)
    {
	sscanf (s, "VID=%d", &entry->vid);
    }

    verb_printf (1, "flags=%x\n", entry->flags);

    return 0;
}

/*! Get first/next ATU entry
 *
 *
 * \returns 0 on success, 1 on error, 2 if no more entries exist
 */
int ath_arl_get_next (struct ath_dev *dev, int first, struct ath_arl_entry *entry)
{
    int rc = 0;
    uint32_t	data0, data1, data2;

    if (first)
    {
	ath_rmw (dev, AR8327_REG_ATU_DATA0, 0xffffffff, 0, &rc);
	ath_rmw (dev, AR8327_REG_ATU_DATA1, 0xffffffff, 0, &rc);
	ath_rmw (dev, AR8327_REG_ATU_DATA2, 0xffffffff, 0, &rc);
	if (rc)
	    return rc;
    }

    ath_rmw (dev, AR8327_REG_ATU_FUNC, 0xffffffff, AR8327_ATU_FUNC_OP_GET_NEXT | AR8327_ATU_FUNC_BUSY, &rc);
    if (rc)
	return rc;

    if (atu_wait (dev))
	return rc;

    data0 = ath_rmw (dev, AR8327_REG_ATU_DATA0, 0, 0, &rc);
    data1 = ath_rmw (dev, AR8327_REG_ATU_DATA1, 0, 0, &rc);
    data2 = ath_rmw (dev, AR8327_REG_ATU_DATA2, 0, 0, &rc);
    if (rc)
	return rc;

    if ((data0 == 0) && ((data1 & 0xffff) == 0) && 
	((data2 & (AR8327_ATU_VID|AR8327_ATU_STATUS)) == 0))
	return 2;

    entry->mac[5] = (data0 >>  0) & 0xff;
    entry->mac[4] = (data0 >>  8) & 0xff;
    entry->mac[3] = (data0 >> 16) & 0xff;
    entry->mac[2] = (data0 >> 24) & 0xff;
    entry->mac[1] = (data1 >>  0) & 0xff;
    entry->mac[0] = (data1 >>  8) & 0xff;
    entry->ports  = (data1 & AR8327_ATU_PORTS) >> 16;

    if ((DATA1_FLAGS & DATA2_FLAGS) != 0)
	printf ("ERROR: DATA1_FLAGS & DATA2_FLAGS overlap: %x\n", (int)(DATA1_FLAGS & DATA2_FLAGS));

    entry->flags = (data1 & DATA1_FLAGS) | (data2 & DATA2_FLAGS);
    entry->vid  = (data2 & AR8327_ATU_VID) >> AR8327_ATU_VID_S;

    return 0;
}

/*! Print contents of L2 table entry
 */
static void arl_print (struct ath_arl_entry *entry)
{
    uint32_t	status;
    char	*comma="";
    int i;

    status = entry->flags & AR8327_ATU_STATUS;
    if (status == 0)
	return;

    printf ("MAC=%02x:%02x:%02x:%02x:%02x:%02x ", 
	entry->mac[0],
	entry->mac[1],
	entry->mac[2],
	entry->mac[3],
	entry->mac[4],
	entry->mac[5]);

    printf ("ports=");
    for (i = 0; i < 7; i++)
    {
	if (entry->ports & (1<<i))
	{
	    printf ("%s%d", comma, i);
	    comma = ",";
	}
    }
    printf (" ");

    if (status <= 7)
	printf ("DYN ");
    else if (status < 0xf)
	printf ("DYN_NOCHG ");
    else
	printf ("STATIC ");

    if (entry->vid)
	printf ("VID=%d ", entry->vid);

    if (entry->flags & AR8327_ATU_PRI)		printf ("PRI ");
    if (entry->flags & AR8327_ATU_SVL_LEARNED)	printf ("SVL ");
    if (entry->flags & AR8327_ATU_SVL_LEARNED)	printf ("SVL ");
    if (entry->flags & AR8327_ATU_PRI_OVR)	printf ("PRI=%d ", (int)(entry->flags & AR8327_ATU_PRI) >> AR8327_ATU_PRI_S);
    if (entry->flags & AR8327_ATU_MIRROR)	printf ("MIRROR ");
    if (entry->flags & AR8327_ATU_DROP)		printf ("DROP ");
    if (entry->flags & AR8327_ATU_LEAKY)	printf ("LEAKY ");
    if (entry->flags & AR8327_ATU_REDIR_CPU)	printf ("REDIR_CPU ");
    if (entry->flags & AR8327_ATU_COPY_CPU)	printf ("COPY_CPU ");
    if (entry->flags & AR8327_ATU_SHORT_LOOP)	printf ("SHORTLOOP ");
    printf ("\n");
}

/*! Dump complete L2 table
 *
 * \param dev device handle
 */
void ath_arl_dump (struct ath_dev *dev)
{
    int first = 1;
    struct ath_arl_entry entry;

    while (1)
    {
	if (ath_arl_get_next (dev, first, &entry))
	    break;
	
	first = 0;

	arl_print (&entry);
    }
}

/*! remove specific entry from L2 table
 * 
 * Entries are matched against MAC address, port list and VID.
 *
 * \param dev device handle
 * \param entry pointer to ARL entry data
 *
 * \returns 0 on success, 1 on error, 2 if entry was not found
 */
int ath_arl_rm (struct ath_dev *dev, struct ath_arl_entry *entry)
{
    int rc = 0;

    ath_rmw (dev, AR8327_REG_ATU_DATA0, 
	    0xffffffff,
	    entry->mac[5] |
	    (entry->mac[4] << 8) | 
	    (entry->mac[3] << 16) | 
	    (entry->mac[2] << 24),
	    &rc);

    ath_rmw (dev, AR8327_REG_ATU_DATA1, 
	    0xffffffff,
	     entry->mac[1] |
	    (entry->mac[0] << 8) | 
	    ((entry->ports << AR8327_ATU_PORTS_S) & AR8327_ATU_PORTS) |
	    (entry->flags & DATA1_FLAGS),
	    &rc);

    ath_rmw (dev, AR8327_REG_ATU_DATA2, 
	    0xffffffff,
	    0xf |
	    (entry->flags & DATA2_FLAGS) |
	    ((entry->vid << AR8327_ATU_VID_S) & AR8327_ATU_VID),
	    &rc);

    if (rc)
	return rc;

    ath_rmw (dev, AR8327_REG_ATU_FUNC, 0xffffffff, AR8327_ATU_FUNC_OP_PURGE | AR8327_ATU_FUNC_BUSY, &rc);
    if (rc)
	return rc;

    if ((rc = atu_wait (dev)) != 0)
    {
	if (rc == 2)
	    SETERR("Entry not found");
	return rc;
    }

    return 0;
}

/*! Add/change entry to/in L2 table
 *
 * \param dev device handle
 * \param entry pointer to ARL entry data
 *
 * \returns 0 on success, 1 on error, 2 if table full error
 */
int ath_arl_add (struct ath_dev *dev, struct ath_arl_entry *entry)
{
    int rc = 0;

    ath_rmw (dev, AR8327_REG_ATU_DATA0, 
	    0xffffffff,
	    entry->mac[5] |
	    (entry->mac[4] << 8) | 
	    (entry->mac[3] << 16) | 
	    (entry->mac[2] << 24),
	    &rc);

    ath_rmw (dev, AR8327_REG_ATU_DATA1, 
	    0xffffffff,
	     entry->mac[1] |
	    (entry->mac[0] << 8) | 
	    ((entry->ports << AR8327_ATU_PORTS_S) & AR8327_ATU_PORTS) |
	    (entry->flags & DATA1_FLAGS),
	    &rc);

    ath_rmw (dev, AR8327_REG_ATU_DATA2, 
	    0xffffffff,
	    0xf |
	    (entry->flags & DATA2_FLAGS) |
	    ((entry->vid << AR8327_ATU_VID_S) & AR8327_ATU_VID),
	    &rc);

    if (rc)
	return rc;

    ath_rmw (dev, AR8327_REG_ATU_FUNC, 0xffffffff, AR8327_ATU_FUNC_OP_LOAD | AR8327_ATU_FUNC_BUSY, &rc);
    if (rc)
	return rc;

    if ((rc = atu_wait (dev)) != 0)
    {
	if (rc == 2)
	    SETERR("ATU table full");
	return rc;
    }

    return 0;
}

/*! @} */
