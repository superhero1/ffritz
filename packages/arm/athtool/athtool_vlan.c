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

#include "athtool.h"

/*! Execute VTU command
 *
 * If required, VTU_FUNC0 register must be assigned before calling this function
 *
 * \param cmd	VTU command bytes (bits 0..3 of VTU_FUNC1)
 * \param vid	VID part of VTU_FUNC1 (if applicable)
 * \param port  PORT part of VTU_FUNC1 (if applicable)
 * \param reg0	[out] assigned with read VTU_FUNC0 after command was executed
 * \param reg1	[out] assigned with read VTU_FUNC1 after command was executed
 *
 * \returns 0 on success, 1 on error
 */
int ath_vtu_cmd (int cmd, uint32_t vid, uint32_t port, uint32_t *reg0, uint32_t *reg1)
{
    int rc = 0;
    int i;

    if (!reg0 || !reg1)
    {
	SETERR("bad parameters");
	return 1;
    }

    vid &= 0xfff;
    port &= 0xf;
    cmd &= 7;

    cmd = cmd | AR8327_VTU_FUNC1_BUSY |
	(vid << AR8327_VTU_FUNC1_VID_S) | (port << AR8327_VTU_FUNC1_PORT_S);

    ath_rmw (AR8327_REG_VTU_FUNC1, 0xffffffff, cmd, &rc);

    if (rc)
	return 1;

    for (i = 0; i < 100; i++)
    {
	cmd = ath_rmw (AR8327_REG_VTU_FUNC1, 0, 0, &rc);
	if (rc)
	    return 1;
	
	if ((cmd & AR8327_VTU_FUNC1_BUSY) == 0)
	    break;
	
	usleep (100);
    }

    /* operation timed out?
     */
    if (cmd & AR8327_VTU_FUNC1_BUSY)
    {
	SETERR("VTU time-out");
	return 1;
    }

    *reg0 = ath_rmw (AR8327_REG_VTU_FUNC0, 0, 0, &rc);
    *reg1 = cmd;

    return rc;
}

/*! Get attributes of a specific VLAN
 *
 * \param vid	VLAN ID
 *
 * \returns VTU_FUNC_REG0 content on success, 0 on error
 */
uint32_t ath_vid_get (uint32_t vid)
{
    uint32_t reg0, reg1;

    if (ath_vtu_cmd (AR8327_VTU_FUNC1_OP_GET_ONE, vid, 0, &reg0, &reg1))
	return 0;

    if ((reg0 & AR8327_VTU_FUNC0_VALID) == 0)
	return 0;

    return reg0;
}

/*! Modify port egress mode in attribute (VTU_FUNC0) value
 *
 * \param attr attribute (VTU_FUNC0) value
 * \param port port number
 * \mode port egress mode (AR8327_VTU_FUNC0_EG_MODE_KEEP,
 *	AR8327_VTU_FUNC0_EG_MODE_UNTAG,
 *	AR8327_VTU_FUNC0_EG_MODE_TAG
 *
 * \returns new VTU_FUNC0 value
 */
uint32_t ath_attr_set_port (uint32_t attr, uint32_t port, uint32_t mode)
{
    if (port > 6)
	return attr;

    if (mode > 3)
	return attr;

    return (attr & ~(0x30 << (port*2))) | (mode << (4 + port*2));
}

/*! Create new VLAN
 *
 * \param vid	VLAN ID
 *
 * \returns 0 on success, 1 on error
 */
int ath_vlan_create (uint32_t vid, uint32_t attr)
{
    int rc = 0;
    uint32_t r1, r2;

    /* does it already exist?
     */
    if (ath_vid_get (vid))
    {
	SETERR("VLAN exists");
	return 1;
    }

    attr = (attr & (AR8327_VTU_FUNC0_IVL|AR8327_VTU_FUNC0_LLD|AR8327_VTU_FUNC0_PO|AR8327_VTU_FUNC0_PRI|AR8327_VTU_FUNC0_EG_MODE)) |
	    AR8327_VTU_FUNC0_VALID;

    ath_rmw (AR8327_REG_VTU_FUNC0, 0xffffffff, attr, &rc);
    if (rc)
	return 1;

    if (ath_vtu_cmd (AR8327_VTU_FUNC1_OP_LOAD, vid, 0, &r1, &r2))
	return 1;

    return 0;
}

/*! delete a VLAN
 * 
 * \param vid	VLAN id
 *
 * \returns 0 on success, 1 on error
 */
int ath_vlan_delete (uint32_t vid)
{
    uint32_t r1, r2;

    if (ath_vtu_cmd (AR8327_VTU_FUNC1_OP_PURGE, vid, 0, &r1, &r2))
	return 1;

    return 0;
}


/*! Remove a port from a VLAN
 * 
 * \param vid	VLAN id
 * \param port	port number
 *
 * \returns 0 on success, 1 on error
 */
int ath_vlan_port_rm (uint32_t vid, uint32_t port)
{
    uint32_t r1, r2;

    if ((port > 6) || (vid > 4095))
    {
	SETERR("parameter out of range");
	return 1;
    }

    if (ath_vtu_cmd (AR8327_VTU_FUNC1_OP_REMOVE_PORT, vid, port, &r1, &r2))
	return 1;

    return 0;
}

/*! Add a port to a VLAN
 * 
 * \param vid	VLAN id
 * \param port	port number
 * \param port	egress tag mode (see ath_attr_set_port())
 *
 * \returns 0 on success, 1 on error
 */
int ath_vlan_port_add (uint32_t vid, uint32_t port, uint32_t mode)
{
    int rc;
    uint32_t r1, r2;
    uint32_t attr;

    if ((port > 6) || (mode > 2))
    {
	SETERR("parameter out of range");
	return 1;
    }

    attr = ath_vid_get (vid);
    if (attr == 0)
    {
	SETERR("VLAN does not exist");
	return 1;
    }

    attr = (attr & ~(3 << AR8327_VTU_FUNC0_EG_MODE_S(port))) |
	   (mode << AR8327_VTU_FUNC0_EG_MODE_S(port));

    ath_rmw (AR8327_REG_VTU_FUNC0, 0xffffffff, attr, &rc);
    if (rc)
	return 1;

    if (ath_vtu_cmd (AR8327_VTU_FUNC1_OP_LOAD, vid, 0, &r1, &r2))
	return 1;

    return 0;
}

/*! Set default VID for untagged ingress frames
 *
 * \param port port number
 * \param vid  VLAN id
 *
 * \returns 0 on success, 1 on error
 */
int ath_pvid_port (uint32_t port, uint32_t vid)
{
    int rc = 0;

    if ((port > 6) || (vid > 4095))
    {
	SETERR("parameter out of range");
	return 1;
    }

    ath_rmw (AR8327_REG_PORT_VLAN0(port),
	AR8327_PORT_VLAN0_DEF_CVID,
	vid << AR8327_PORT_VLAN0_DEF_CVID_S,
	&rc);

    return rc;
}


/*! get first/next entry in VTU table
 *
 * \param reset	0 to get first entry, 1 for next entry
 * \param reg0	holdt contents of VTU_FUNC0 register
 * \param reg1	holds contents of VTU_FUNC1 register
 *
 * \returns 0 on success, 1 on error
 */
int ath_vtu_get_next (int reset, uint32_t *reg0, uint32_t *reg1)
{
    int rc = 0;
    uint32_t cmd = 0;
    int i;

    if (!reg0 || !reg1)
    {
	SETERR("bad arguments");
	return 1;
    }

    if (reset)
    {
	ath_rmw (AR8327_REG_VTU_FUNC0, 0xffffffff, 0, &rc);
	if (rc)
	    return 1;
    }
	
    cmd = ath_rmw (AR8327_REG_VTU_FUNC1, 0, 0, &rc);
    if (rc)
	return 1;

    /* pending operation? 
     */
    if (cmd & 0x80000000)
    {
	SETERR("VTU operation pending");
	return 1;
    }

    if (reset)
	cmd = 0;

    /* preserve information from previous operation (if requested) and
     * issue get-next command
     */
    cmd = (cmd & ~7) | 0x80000005;
    ath_rmw (AR8327_REG_VTU_FUNC1, 0xffffffff, cmd, &rc);

    if (rc)
	return 1;

    for (i = 0; i < 100; i++)
    {
	cmd = ath_rmw (AR8327_REG_VTU_FUNC1, 0, 0, &rc);
	if (rc)
	    return 1;
	
	if ((cmd & AR8327_VTU_FUNC1_BUSY) == 0)
	    break;
	
	usleep (10);
    }

    /* operation timed out?
     */
    if (cmd & AR8327_VTU_FUNC1_BUSY)
    {
	SETERR("VTU operation timed out");
	return 1;
    }

    *reg0 = ath_rmw (AR8327_REG_VTU_FUNC0, 0, 0, &rc);
    *reg1 = cmd;

    return rc;
}

/*! Show some VLAN related information
 */
void ath_vlan_show (void)
{
    int i, j;
    uint32_t r1[7], r2[7];
    int rc = 0;
    uint32_t vr1, vr2;
    char *s;
    uint32_t val;
    int vid;

    printf ("\n%-10s: ", "Port");
    for (i = 0; i < 7; i++)
    {
	rc = 0;

	r1[i] = ath_rmw (AR8327_REG_PORT_VLAN0(i), 0, 0, &rc);
	r2[i] = ath_rmw (AR8327_REG_PORT_VLAN1(i), 0, 0, &rc);

	printf ("%8d ", i);
    }
    printf ("\n%-10s: ", "----------");
    for (i = 0; i < 7; i++)
	printf ("%8s ", "--------");

    printf ("\n%-10s: ", "DEF_SVID");
    for (i = 0; i < 7; i++)
	printf ("%8d ", (int)(r1[i] & AR8327_PORT_VLAN0_DEF_SVID) >> AR8327_PORT_VLAN0_DEF_SVID_S);

    printf ("\n%-10s: ", "DEF_CVID");
    for (i = 0; i < 7; i++)
	printf ("%8d ", (int)(r1[i] & AR8327_PORT_VLAN0_DEF_CVID) >> AR8327_PORT_VLAN0_DEF_CVID_S);
	
    printf ("\n%-10s: ", "EG_MODE");
    for (i = 0; i < 7; i++)
    {
	switch ((r2[i] & AR8327_PORT_VLAN1_OUT_MODE) >> AR8327_PORT_VLAN1_OUT_MODE_S)
	{
	    case AR8327_PORT_VLAN1_OUT_MODE_UNMOD:   s = "UNMOD"; break;
	    case AR8327_PORT_VLAN1_OUT_MODE_UNTAG:   s = "UNTAG"; break;
	    case AR8327_PORT_VLAN1_OUT_MODE_TAG:     s = "TAG"; break;
	    default:				     s = "NA"; break;
	}
	printf ("%8s ", s);
    }
	
    printf ("\n%-10s: ", "IG_MODE");
    for (i = 0; i < 7; i++)
    {
	switch ((r2[i] & AR8327_PORT_VLAN1_IN_MODE) >> AR8327_PORT_VLAN1_IN_MODE_S)
	{
	    case AR8327_PORT_VLAN1_IN_MODE_ALL:   s = "ALL"; break;
	    case AR8327_PORT_VLAN1_IN_MODE_TAG:   s = "TAG"; break;
	    case AR8327_PORT_VLAN1_IN_MODE_UNTAG: s = "UNTAG"; break;
	    default:				  s = "NA"; break;
	}
	printf ("%8s ", s);
    }

    printf ("\n%-10s: ", "EG_TYPE");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & AR8327_PORT_VLAN1_EG_TYPE_1) ? "TAG" : "ALL");

    printf ("\n%-10s: ", "SPCHECK");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(10)) ? "EN" : "-");

    printf ("\n%-10s: ", "CORE");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(9)) ? "EN" : "-");

    printf ("\n%-10s: ", "FRC_VID");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(8)) ? "EN" : "-");

    printf ("\n%-10s: ", "TLS");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(7)) ? "EN" : "-");

    printf ("\n%-10s: ", "PROP_EN");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(6)) ? "EN" : "-");

    printf ("\n%-10s: ", "CLONE_EN");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(5)) ? "EN" : "-");

    printf ("\n%-10s: ", "PRIO");
    for (i = 0; i < 7; i++)
	printf ("%8s ", (r2[i] & BIT(4)) ? "EN" : "-");

    printf ("\n%-10s: ", "----------");
    for (i = 0; i < 7; i++)
	printf ("%8s ", "--------");
    for (j = 0; j < 100; j++)
    {
	/* get first/next table entry
	 */
	rc = ath_vtu_get_next ((i == 0) ? 1 : 0, &vr1, &vr2);

	if (rc)
	    break;

	vid = (vr2 & AR8327_VTU_FUNC1_VID) >> AR8327_VTU_FUNC1_VID_S;

	/* last entry? 
	 */
	if (vid == 0)
	    break;

	printf ("\nVLAN%6d: ", vid);

	val = vr1 >> 4;
	for (i = 0; i < 7; i++, val >>= 2)
	{
	    switch (val & 3)
	    {
		case AR8327_VTU_FUNC0_EG_MODE_KEEP:	s = "UNMOD"; break;
		case AR8327_VTU_FUNC0_EG_MODE_UNTAG:	s = "UNTAG"; break;
		case AR8327_VTU_FUNC0_EG_MODE_TAG:	s = "TAG";  break;
		default: s = "-"; break;
	    }
	    printf ("%8s ", s);
	}
	printf ("\nVLAN%6d: ", vid);
	printf ("{V=%d IVL=%d LLD=%d PO=%d PRI=%d}",
	    ((vr1 && BIT(20)) != 0),
	    ((vr1 && BIT(19)) != 0),
	    ((vr1 && BIT(18)) != 0),
	    ((vr1 && BIT(3)) != 0),
	    vr1 & 7);
    }


    printf ("\n");
}



