/* Simple tool to access registers of atheros switch on FB6490.
 *
 * Copyright (C) 2016 - Felix Schmidt
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

#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>


/* Prototypes for libtic (reverse-engineered ..) */

extern int extSwitchReadAthReg (uint32_t reg, uint32_t *val);
extern int extSwitchWriteAthReg (uint32_t reg, uint32_t val);

const char *usage =
"athtool options    Atheros switch tool\n"
" -r <adrs>        : Read register at offset\n"
" -w <ards>,<val>  : Write val to register\n"
;


int main (int argc, char **argv)
{
    int c;
    uint32_t reg, val;
    char *s;

    while (1)
    {
	int option_index = 0;
	static struct option long_options[] = {
	    {"read", required_argument, 0, 'r'},
	    {"write", required_argument, 0, 'w'},
	    {"help", no_argument, 0, 'h'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, "r:w:h", long_options, &option_index);

	if (c == -1)
	    break;

	switch (c)
	{
	case 'r':
	    reg = strtoul (optarg, NULL, 0);

	    printf ("0x%03x : ", reg);

	    if (extSwitchReadAthReg (reg, &val))
	    {
		printf ("ERR\n");
		return 1;
	    }
	    else
	    {
		printf ("0x%04x\n", val);
	    }

	    break;

	case 'w':
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

	    if (extSwitchWriteAthReg (reg, val))
	    {
		printf ("ERR\n");
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
