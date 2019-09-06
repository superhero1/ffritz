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

/* ========================================================================= */
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "uimg.h"

/* ========================================================================= */
struct
{
	uint8_t dev;
	char *name;
}
dev_to_name[] =
{
	{  2, "ATOM_KERNEL" },
	{  3, "ATOM_ROOTFS" },
	{  8, "ARM_KERNEL"},
	{  9, "ARM_ROOTFS"},
	{ 10, "GWFS"},
	{  0, NULL }
};

const char *help =
" uimg -u [-n <name>] uimg-file\n"
"   -u   unpack all partitions and write to .bin files\n"
"   -n   name prefix for output files (default: input file name without suffix)\n"
;

#define UPDC32(octet,crc) (crc_32_tab[((crc)\
     ^ ((uint8_t)octet)) & 0xff] ^ ((crc) >> 8))

static uint32_t crc_32_tab[] =
{
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* ========================================================================= */

uint32_t crc32(char *buf, size_t len, uint32_t *old)
{
	uint32_t oldcrc32;

    oldcrc32 = old ? *old : 0xFFFFFFFF;

	for ( ; len; --len, ++buf)
	{
		oldcrc32 = UPDC32(*buf, oldcrc32);
	}

	if (old)
		*old = oldcrc32;

    return ~oldcrc32;
}

void generate(char *fname, char *prefix)
{
	int i, j;
	struct uimg_head head;
	char *in_name = alloca(strlen(prefix) + 20);
	uint32_t in_size;
	char *buffer;
	uint32_t img_crc;
	uint32_t img_crc_cnt = 0xffffffff;
	int part_idx = 0;
	uint32_t crc;
	uint32_t size = sizeof(head);

	int out_fd, in_fd;

	out_fd = open(fname, O_WRONLY|O_CREAT, 0444);
	if (out_fd == 0)
	{
		perror (fname);
		exit (1);
	}

	memset ((void*)&head, 0, sizeof(head));

	/* write empty header, to be filled later */
	if (write (out_fd, &head, sizeof(head)) != sizeof(head))
	{
		perror (fname);
		exit (1);
	}

	for (i = 0; (i < 99) && (part_idx < UIMG_NUM_PARTITIONS) ; i++)
	{
		sprintf (in_name, "%s_%02d", prefix, i);

		for (j = 0; dev_to_name[j].name != NULL; j++)
		{
			if (i == dev_to_name[j].dev)
			{
				strcat (in_name, "_");
				strcat (in_name, dev_to_name[j].name);
				break;
			}
		}
		strcat (in_name, ".bin");

		in_fd = open(in_name, O_RDONLY);
		if (in_fd == 0)
			continue;

		in_size = (uint32_t)lseek (in_fd, 0, SEEK_END);
		if ((in_size == 0) || (in_size == 0xffffffff))
			continue;
		lseek (in_fd, 0, SEEK_SET);

		buffer = malloc (in_size);
		if (buffer == NULL)
		{
			fprintf (stderr, "failed to allocate 0x%x bytes for %s\n",
					 in_size, in_name);
			exit (1);
		}

		if (read (in_fd, buffer, in_size) != in_size)
		{
			perror (in_name);
			exit (1);
		}

		crc = crc32(buffer, in_size, NULL);
		head.part_csum[part_idx] = BE_TO_HOST(crc);
		head.part_size[part_idx] = BE_TO_HOST(in_size);
		head.part_dev[part_idx]  = i;

		img_crc = crc32(buffer, in_size, &img_crc_cnt);

		if (write (out_fd, buffer, in_size) != in_size)
		{
			perror (in_name);
			exit (1);
		}

		free (buffer);
		close (in_fd);

		printf (" %s size=%d crc=0x%08x\n", in_name, in_size, crc);

		part_idx++;
		size += in_size;
	}

	/* update header */
	head.magic = UIMG_MAGIC;
	strcpy (head.name, "Intel_Unified_Image");
	head.unknown1 = 0x0077081B;
	head.unknown2 = 0x00083810;
	head.unknown3_ver = 3;
	head.num_part = part_idx;
	head.size = size;
	head.data_crc = img_crc;

	head.magic = BE_TO_HOST(head.magic);
	head.unknown1 = BE_TO_HOST(head.unknown1);
	head.unknown2 = BE_TO_HOST(head.unknown2);
	head.unknown3_ver = BE_TO_HOST(head.unknown3_ver);
	head.num_part = BE_TO_HOST(head.num_part);
	head.size = BE_TO_HOST(head.size);
	head.data_crc = BE_TO_HOST(head.data_crc);

	head.head_crc = crc32((void*)&head, sizeof(head), NULL);
	head.head_crc = BE_TO_HOST(head.head_crc);

	lseek (out_fd, 0, SEEK_SET);

	if (write (out_fd, &head, sizeof(head)) != sizeof(head))
	{
		perror (fname);
		exit (1);
	}

	close (out_fd);
}

void extract(char *fname, char *prefix)
{
	int in_fd;
	int out_fd;
	int i, j;
	struct uimg_head head;
	char *buffer;
	int psize;

	in_fd = open (fname, O_RDONLY);
	if (in_fd == 0)
	{
		perror (fname);
		exit (1);
	}

	if (read (in_fd, &head, sizeof(head)) != sizeof(head))
	{
		perror (fname);
		exit (1);
	}

	for (i = 0; i < UIMG_NUM_PARTITIONS; i++)
	{
		char part_label[100];

		if (head.part_size[i] == 0)
			break;

		psize = BE_TO_HOST(head.part_size[i]);

		sprintf (part_label, "%s_%02d", prefix, head.part_dev[i]);

		for (j = 0; dev_to_name[j].name != NULL; j++)
		{
			if (head.part_dev[i] == dev_to_name[j].dev)
			{
				strcat (part_label, "_");
				strcat (part_label, dev_to_name[j].name);
				break;
			}
		}
		strcat (part_label, ".bin");

		out_fd = open (part_label, O_WRONLY|O_CREAT, 0444);
		if (out_fd == 0)
		{
			perror (part_label);
			exit (1);
		}

		buffer = malloc (psize);
		if (buffer == NULL)
		{
			fprintf (stderr, "failed to allocate 0x%x bytes for %s\n",
					 head.part_size, part_label);
			exit (1);
		}

		if (read (in_fd, buffer, psize) != psize)
		{
			fprintf (stderr, "failed to read 0x%x bytes for %s\n",
					 psize, part_label);
			exit (1);

		}

		if (write (out_fd, buffer, psize) != psize)
		{
			perror (part_label);
			exit (1);
		}

		printf (" %s size=%d crc=0x%08x\n", part_label, psize,
			BE_TO_HOST(head.part_csum[i]));

		close (out_fd);
		free (buffer);
	}
}

int main (int argc, char **argv)
{
	enum { none, unpack, pack } mode = none;
	char *prefix = NULL;
	char *fname = NULL;
	int i, c;

	while ((c = getopt(argc, argv, "pun:h")) != -1)
	{
		switch (c)
		{
		case 'h':
			printf (help);
			exit (0);
		case 'p':
			mode = pack;
			break;
		case 'u':
			mode = unpack;
			break;
		case 'n':
			prefix = strdup(optarg);
			break;
		default:
			exit(1);
		}
	}

	if (optind >= argc)
		exit (1);

	fname = argv[optind];

	if (!prefix)
	{
		prefix = strdup(fname);
		for (i = strlen(prefix) - 1; i >= 0; i--)
		{
			if (prefix[i] == '.')
			{
				prefix[i] = 0;
				break;
			}
		}
	}

	switch (mode)
	{
		case unpack:
			extract (fname, prefix);
			break; 
		case pack:
			generate (fname, prefix);
			break; 
		default:
			return 1;
	}
	return 0;
}
