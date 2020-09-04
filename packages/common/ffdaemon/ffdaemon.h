/* ffdaemon - a simple user space audio player
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

#ifndef _INCFFDAEMONH_
#define _INCFFDAEMONH_

#define SN_DIR "/var/run"
#define SN_PREFIX "ffd_"
#define SNAME SN_PREFIX "service_name"

void log_put (const char *format, ...);
void log_set (const char *logf, int consOut);
int daemon2 (int delay, int start_delay, int loops, int nochdir, int noclose, const char *service_name);
int do_limits (char *limits, int check_only);
char *get_pidfile(const char *service_name);
char *get_client_pidfile(const char *service_name);
char *get_service_info(const char *service_name);



#endif
