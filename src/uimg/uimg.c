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

/* ========================================================================= */

int main (int argc, char **argv)
{
	enum { none, unpack, pack } mode = none;
	int in_fd;
	int out_fd;
	char *prefix = NULL;
	char *fname = NULL;
	int c;
	int i, j;
	struct uimg_head head;
	int num_partitions = 0;
	char *buffer;
	int psize;

	while ((c = getopt(argc, argv, "un:h")) != -1)
	{
		switch (c)
		{
		case 'h':
			printf (help);
			exit (0);
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

	fname = argv[optind];

	if (mode != unpack)
		exit (1);

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

	printf ("%s %s\n", fname, prefix);

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

		close (out_fd);
		free (buffer);

		printf ("  %s 0x%x\n", part_label, psize);
	}
}
