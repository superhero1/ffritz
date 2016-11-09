/* usbplayd - a simple user space audio player
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

/*================================== INCLUDES ===============================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

#include "usbplayd.h"

/*================================== DEFINES ================================*/

/*================================== TYPEDEFS ===============================*/

/*================================== IMPORTS ================================*/

/*================================== GLOBALS ================================*/

/*================================== LOCALS =================================*/
static int worker_pid = 0;
static char *pidfile = "/var/run/mcmsgd.pid";
static FILE *dflConsOut = NULL;
static FILE *logFile = NULL;

/*================================== IMPLEMENTATION =========================*/

/*! \brief Called via atexit()
*/
static void exit_fcn (void)
{
    unlink (pidfile);
}

static void cleanup()
{
    exit_fcn();
    exit (1);
}

/*! \brief Create PID file for this process and make sure its removed at exit
*/
static void prep_pid_file (char *pdfile)
{
    FILE *pf;

    atexit (exit_fcn);
    signal (SIGINT, cleanup);
    signal (SIGHUP, cleanup);
    signal (SIGKILL, cleanup);
    signal (SIGTERM, cleanup);
    signal (SIGBUS, cleanup);

    pidfile = pdfile;
    
    pf = fopen (pidfile, "w");

    if (pf)
    {
        fprintf (pf, "%d", getpid());
        fclose (pf);
    }
    else
    {
        log_put ("Failed to write PID file %s\n", pidfile);    
    }
}

/* \brief Put message to log
 * The first character may be
 * + : No time prefix
 * ! : Treated as error
 *
 * \param format printf style format string 
 */
void log_put (const char *format, ...)
{
    char buff[2048];
    char ct[100];
    va_list args;
    int i;
    int off=0;
    char *prefix="";
    struct timeval t;

    if (format == NULL)
	return;

    switch (format[0])
    {
        case '+':
            off = 1;
            break;
        case '!':
            off = 1;
            prefix = "ERROR:";
            break;
    }

    va_start(args, format);
    i = vsnprintf(buff, sizeof(buff), &format[off], args);
    va_end(args);

    if (format[0] != '+')
    {
        gettimeofday (&t, NULL);
        ctime_r (&t.tv_sec, ct);

        for (i = 0; i < strlen(ct); i++)
        {
            if (ct[i] == '\n')
            {
                ct[i] = ':';
                break;
            }
        }
    }
    else
    {
        ct[0] = '\0';
    }

    if (dflConsOut)
        fprintf (dflConsOut, "%s:%s%s", ct, prefix, buff);
    
    if (!logFile)
        return;

    fprintf (logFile, "%s%s%s", ct, prefix, buff);
}

void log_set (const char *logf, int consOut)
{
    dflConsOut = stderr;

    if (logf)
    {
	logFile = fopen (logf, "a");

	if (!logFile)
	{
	    log_put ("!Failed to open log file %s\n", logf);
	}
	setvbuf (logFile, NULL, _IONBF, 0);
    }

    switch (consOut)
    {
	case 1:
	    dflConsOut = stdout;
	    break; 
	case 2:
	    dflConsOut = stderr;
	    break; 
	default:
	    dflConsOut = (logFile == NULL) ? stderr : NULL;
	    break; 
    }
}


int daemon2 (char *pdfile, int nochdir, int noclose)
{
    int first = 1;
    int status;
    struct stat st;
    
    
    if (pdfile)
    {
	if (stat (pdfile, &st) == 0)
	{
	    log_put ("daemon already running, %s exists\n", pdfile);
	    return 1;
	}
    }
    
    if (daemon (nochdir, noclose))
    {
        log_put ("daemon: %s", strerror(errno));
        return 1;
    }

    /* master process loop
     */
    while (1)
    {
        worker_pid = fork();

        if (worker_pid == -1)
        {
            log_put ("fork: %s", strerror(errno));
            sleep (2);
            continue;
        }

        /* resume with worker process?
         */
        if (worker_pid == 0)
            break;

        /* atexit/pid file is only done in the master process
         */
        if (first)
            prep_pid_file (pdfile);

        first = 0;
        sleep (2);
        if (wait (&status) == -1)
            log_put ("wait: %s", strerror(errno));

        if (status)
            log_put ("worker process terminated with status %d (pid %d)\n", 
                    status, worker_pid);
    }
    
    return 0;
}
