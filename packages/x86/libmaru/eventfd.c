/* libmaru - Userspace USB audio class driver.
 * Copyright (C) 2012 - Hans-Kristian Arntzen
 * Copyright (C) 2012 - Agnes Heyer
 *
 * This library is free software; you can redistribute it and/or
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

#include "libmaru.h"
#include "fifo.h"
#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>

int
eventfd_write (int fd, eventfd_t value)
{
    return (write (fd, &value, sizeof (value)) == sizeof (value)) ? 0 : -1;
}

int
eventfd_read (int fd, eventfd_t * value)
{
    return (read (fd, value, sizeof (*value)) == sizeof (*value)) ? 0 : -1;
}
