/*****************************************************************************
 * ffad.c 
 *****************************************************************************
 * Copyright (C) 2019 Felix Schmidt
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cash.h"

#define SYSCMD(__fct) 						\
int __fct (char *args)						\
{								\
    char *cmd = alloca(strlen(__FUNCTION__) + strlen(args) + 20);\
    sprintf (cmd, "/usr/local/bin/%s %s", __FUNCTION__, args);		\
    return system (cmd);					\
}

SYSCMD(mpc)
SYSCMD(ffdaemon)
SYSCMD(irsend)

void ffad_ext_init()
{
    DECL_PROTO (mpc, PA32, PA64);
    DECL_PROTO (irsend, PA32, PA64);
    DECL_PROTO (ffdaemon, PA32, PA64);
}
