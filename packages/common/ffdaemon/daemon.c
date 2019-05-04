/* 
 * Copyright (C) 2016 - Felix Schmidt
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

#include "ffdaemon.h"

/*================================== DEFINES ================================*/

/*================================== TYPEDEFS ===============================*/

/*================================== IMPORTS ================================*/

/*================================== GLOBALS ================================*/

/*================================== LOCALS =================================*/
static int worker_pid = 0;
static char *pidfile = NULL;
static char *client_pidfile = NULL;
static FILE *dflConsOut = NULL;
static FILE *logFile = NULL;
static int client_restarts = 0;
static struct timeval client_starttime = {0};
static uint64_t client_total_runtime = 0;

/*================================== IMPLEMENTATION =========================*/

static int purge_service (char *service_name, char *pidfile)
{
    int fd;
    char tmpstr[100];
    int pid = 0;
    int rc = 0;

    /* check existing pidfile */
    fd = open (pidfile, O_RDONLY);

    if (fd == -1)
	return 0;

    /* check process */
    if (read(fd, tmpstr, sizeof(tmpstr)) > 0)
    {
	pid = atoi(tmpstr);

	if (pid)
	{
	    struct stat st;

	    sprintf (tmpstr, "/proc/%d", pid);

	    if (stat (tmpstr, &st) == 0)
	    {
		rc = 1;

		log_put ("service %s already running\n", service_name);
	    }
	}
    }

    close (fd);

    if (rc == 0)
    {
	log_put ("Removing stale PID file for service %s\n", service_name);

	if (unlink (pidfile))
	{
	    perror (pidfile);
	    return 1;
	}

	if (client_pidfile)
	    unlink (client_pidfile);
    }

    /* remove pidfile if no longer running */

    return rc;

}

static void stop_worker (int sig)
{
    if (worker_pid > 0)
    {
	kill (worker_pid, SIGTERM);
	sleep (2);
	kill (worker_pid, SIGKILL);
    }
}

/*! \brief Called via atexit()
*/
static void exit_fcn (void)
{

    if (worker_pid > 0)
    {
	kill (worker_pid, SIGTERM);
	sleep (2);
	kill (worker_pid, SIGKILL);
	worker_pid = 0;
    }

    if (pidfile)
	unlink (pidfile);

    pidfile = NULL;

    if (client_pidfile)
    {
	unlink (client_pidfile);
	free (client_pidfile);
	client_pidfile = NULL;
    }
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
    char *s1, *s2;

    atexit (exit_fcn);
    signal (SIGINT, cleanup);
    signal (SIGQUIT, cleanup);
    signal (SIGHUP, cleanup);
    signal (SIGKILL, cleanup);
    signal (SIGTERM, cleanup);
    signal (SIGBUS, cleanup);

    if (pdfile)
    {
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

	if ((s1 = strstr (pidfile, "ffd_")) != NULL)
	{
	    client_pidfile = malloc(strlen(pidfile) + 1);
	    strcpy (client_pidfile, pidfile);

	    s2 = strstr(client_pidfile, "ffd_");

	    sprintf (s2, ".%s", s1);
	}
    }
}

static void client_pid_file (int pid)
{
    FILE *pf;
    struct timeval tv;
    time_t client_runtime = 0;

    if (pid)
    {
	client_restarts++;
	gettimeofday (&client_starttime, NULL);
	worker_pid = pid;
    }
    else if (worker_pid)
    {
	gettimeofday (&tv, NULL);
	client_runtime = tv.tv_sec - client_starttime.tv_sec;
	client_total_runtime += client_runtime;
	worker_pid = 0;
    }

    if (!client_pidfile)
	return;

    pf = fopen (client_pidfile, "w");

    if (pf)
    {
	fprintf (pf, "%d %d %d\n", pid, client_restarts, client_restarts ? (int)(client_total_runtime / client_restarts) : 0);
	fclose (pf);
    }
    else
    {
	log_put ("Failed to write client PID file %s\n", pidfile);    
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

    if (logf && strlen(logf))
    {
	logFile = fopen (logf, "a");

	if (!logFile)
	{
	    log_put ("!Failed to open log file %s\n", logf);
	    return;
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


int daemon2 (char *pdfile, int interval, int loops, int nochdir, int noclose, char *service_name)
{
    int status;
    int rc;
    int loop = 0;
    int pid;
    
    if (interval == 0)
	interval = 2;
    
    if (purge_service (service_name, pdfile))
	return 1;

    if (daemon (nochdir, noclose))
    {
        log_put ("daemon: %s", strerror(errno));
        return 1;
    }

    prep_pid_file (pdfile);

    /* master process loop
     */
    while (1)
    {
        pid = fork();

        if (pid == -1)
        {
            log_put ("fork: %s", strerror(errno));
            sleep (interval);
            continue;
        }

        /* resume with worker process?
         */
        if (pid == 0)
	{
            break;
	}
	client_pid_file (pid);

	signal (SIGUSR1, stop_worker);

	while (1)
        {
	    rc = waitpid (pid, &status, 0);

	    if (rc == pid)
		break;

	    sleep (1);
	}

        if (status)
            log_put ("worker process terminated with status %d (pid %d)\n", 
                    status, pid);

	client_pid_file (0);

 	loop++;
	if ((loops > 0) && (loop >= loops))
	    return 2;

        sleep (interval);
    }
    
    return 0;
}
