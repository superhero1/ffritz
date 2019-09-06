/* 
 * Copyright (C) 2019 - Felix Schmidt
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

/* A simple tool to handle uimg images */

#ifndef _uimg_h_
#define _uimg_h_

#define UIMG_NAME_LEN       20
#define UIMG_NUM_PARTITIONS 20
#define UIMG_MAGIC    (((uint32_t)'V' << 24) | ((uint32_t)'E' << 16) | ((uint32_t)'R' << 8) | ((uint32_t)'3'))

#define BE_TO_HOST(v) (((((uint8_t*)&(v))[0]) << 24) | ((((uint8_t*)&(v))[1]) << 16) | ((((uint8_t*)&(v))[2]) << 8) | (((uint8_t*)&(v))[3]))

struct uimg_head
{
	uint32_t magic;
	char     name[UIMG_NAME_LEN];
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t unknown3_ver;
	uint32_t size;
	uint32_t num_part;
	uint32_t head_crc; /* CRC32 of head with head_crc = 0 */
	uint32_t data_crc; /* CRC32 of all data partitions without head */
	uint32_t part_csum[UIMG_NUM_PARTITIONS];
	uint32_t part_size[UIMG_NUM_PARTITIONS];
	uint8_t  part_dev[UIMG_NUM_PARTITIONS];
	uint8_t  part_data1[UIMG_NUM_PARTITIONS];
};

#endif
