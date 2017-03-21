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
#ifndef _athtoolh_
#define _athtoolh_

#include "ar8216.h"
#include "ar8327.h"

extern uint32_t ath_rmw (uint32_t reg, uint32_t mask, uint32_t value, int *err);

extern void ath_vlan_show (void);
extern int ath_vlan_create (uint32_t vid, uint32_t attr);
extern int ath_vlan_delete (uint32_t vid);
extern int ath_vlan_port_rm (uint32_t vid, uint32_t port);
extern int ath_vlan_port_add (uint32_t vid, uint32_t port, uint32_t mode);
extern uint32_t ath_attr_set_port (uint32_t attr, uint32_t port, uint32_t mode);

#endif

