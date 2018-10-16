/*
* vim: si ai sw=4 ts=8
**
** Copyright 2008, The Android Open Source Project
** Copyright 2017, Felix Schmidt                       
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
**
*/
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <alloca.h>

#include "usbplayd.h"


#define SNAME "ffd_service_name"

int service_status (char *service_name, int *cpid);

//extern int daemon2 (char *pdfile, int delay, int loops, int nochdir, int noclose);

const char *usage =
"usage: %s [-nCL] [-r user] [-i interval] [-l loops] [-N|K|R service] command args ...\n"
"       -n : No daemon mode\n"
"       -C : Do not close FDs\n"
"       -r : run as user[:group]\n"
"       -i : Restart delay after program terminates\n"
"       -l : Number of loops to run (0 = default = endless)\n"
"       -N : Name service rather than using the executable name\n"
"       -L : List all services\n"
"       -K : Kill named service (%all for all)\n"
"       -R : Restart named service (%all for all)\n"
;

static char *basename (char *path)
{
    char *rc = path;
    while (*path != '\0')
    {
	if (*path == '/')
	    rc = path+1;

	path++;
    }

    return rc;
}

static char *get_pidfile(char *service_name)
{
    static char pf[1000];

    sprintf (pf, "/var/run/ffd_%s.pid", service_name);

    return pf;
}

static char *get_client_pidfile(char *service_name)
{
    static char pf[1000];

    sprintf (pf, "/var/run/.ffd_%s.pid", service_name);

    return pf;
}

char *get_service_name (int pid)
{
    int fd;
    char tmpstr[100];
    char *buf = NULL, *b;
    size_t len;
    char *rc = NULL;

    /* check proc entry */
    sprintf (tmpstr, "/proc/%d/environ", pid);

    fd = open (tmpstr, O_RDONLY);

    if (fd == -1)
	return NULL;

    len = 0x10000;

    /* read environment */
    b = buf = malloc (len+1);

    if (!buf)
    {
	close (fd);
	free (b);
	return NULL;
    }

    if ((len = read(fd, buf, len)) < 0)
    {
	perror ("read");
	close (fd);
	free (b);
	return NULL;
    }

    close (fd);
    buf[len] = '\0';

    /* look for ffd_service_name in environment */
    while (strlen(buf) != 0)
    {
	if (!strncmp(buf, SNAME, strlen(SNAME)))
	{
	    buf += strlen(SNAME)+1;
	    len = strlen(buf);

	    if (len == 0)
	    {
		free (b);
		return NULL;
	    }

	    rc = malloc(len+1);

	    strcpy (rc, buf);

	    break;
	}
	buf += strlen(buf)+1;
    }
    free (b);

    return rc;
}

int service_status (char *service_name, int *cpid)
{
    int fd;
    char tmpstr[100];
    char *buf = NULL;
    int pid = 0;
    char *pidfile = get_pidfile (service_name);
    char *cpidfile = get_client_pidfile (service_name);
    int len;

    /* check existing pidfile */
    fd = open (pidfile, O_RDONLY);

    if (fd == -1)
	return 0;

    /* check process */
    if ((len = read(fd, tmpstr, sizeof(tmpstr))) > 0)
    {
	tmpstr[len] = '\0';

	pid = atoi(tmpstr);
    }
    close (fd);

    if (!pid)
	return -1;

    /* check if ffd_service_name environment variable of process matches */
    buf = get_service_name (pid);

    if (!buf)
	return -1;

    if (strcmp (service_name, buf))
	pid = -1;

    free (buf);

    /* check existing client pidfile */
    if (cpid)
    {
	*cpid = 0;

	fd = open (cpidfile, O_RDONLY);

	if (fd != -1)
	{
	    /* check process */
	    if ((len = read(fd, tmpstr, sizeof(tmpstr))) > 0)
	    {
		tmpstr[len] = '\0';

		*cpid = atoi(tmpstr);
	    }
	    close (fd);
	}
    }

    return pid;
}

static void list_services(void)
{
    DIR *dir = opendir ("/var/run");
    struct dirent *de = NULL;
    char service_name[256];
    int status;
    int cpid;

    if (!dir)
    {
	perror ("opendir");
	return;
    }

    while ((de = readdir(dir)) != NULL)
    {
	if (strncmp (de->d_name, "ffd_", 4) ||
	    (strstr(de->d_name, ".pid") == NULL))
	    continue;

	strncpy (service_name, de->d_name+4, sizeof(service_name));

	*strstr(service_name, ".pid") = '\0';

	status = service_status (service_name, &cpid);

	if (status == 0)
	    continue;

	printf ("%-15s : ", service_name); 

	if (status == -1)
	{
	    printf ("STALE\n");
	}
	else
	{
	    printf ("PID %d", status);

	    if (cpid)
		printf (" CPID %d", cpid);

	    printf ("\n");
	}
    }; 

    closedir (dir);
}

static void iterate_services(int (*cb)(char *service_name, int arg), int arg)
{
    DIR *dir = opendir ("/var/run");
    struct dirent *de = NULL;
    char service_name[256];
    int status;

    if (!dir)
    {
	perror ("opendir");
	return;
    }

    while ((de = readdir(dir)) != NULL)
    {
	if (strncmp (de->d_name, "ffd_", 4) ||
	    (strstr(de->d_name, ".pid") == NULL))
	    continue;

	strncpy (service_name, de->d_name+4, sizeof(service_name));

	*strstr(service_name, ".pid") = '\0';

	status = service_status (service_name, NULL);

	if (status > 0)
	    (void)cb (service_name, arg);
    }; 

    closedir (dir);
}

static int purge_service (char *service_name)
{
    int pid = 0;

    pid = service_status (service_name, NULL);

    if (pid > 0)
    {
	fprintf (stderr, "service %s already running (%d)\n", service_name, pid);

	return 1;
    }

    if (pid == -1)
    {
	char *pidfile = get_pidfile (service_name);

	fprintf (stderr, "Removing stale PID file %s for service %s\n", pidfile, service_name);

        /* remove pidfile if no longer running */
	if (unlink (pidfile))
	{
	    perror (pidfile);
	    return 1;
	}
    }


    return 0;
}

static int kill_service (char *service_name, int sig)
{
    int pid;

    if (!strcmp(service_name, "%all"))
    {
	iterate_services (kill_service, sig);
	return 0;
    }

    pid = service_status (service_name, NULL);

    if (pid <= 0)
    {
	fprintf (stderr, "service %s: not running\n", service_name);
	return 0;
    }

    printf ("Sending signal %d to %s\n", sig, service_name);
    if (kill (pid, sig))
    {
	perror ("kill");
    }

    return 0;
}

/*
 * SU can be given a specific command to exec. UID _must_ be
 * specified for this (ie argc => 3).
 *
 * Usage:
 * su 1000
 * su 1000 ls -l
 */
int main(int argc, char **argv)
{
    struct passwd *pw;
    int uid = -1, gid = -1, myuid;
    int i, arg_id = 0;
    int daemon_mode = 1;
    int nargs;
    int interval = 2;
    int loops = 0;
    int noclose = 0;
    char *service_name = NULL;

    /* Until we have something better, only root and the shell can use su. */
    myuid = getuid();
    if (myuid != 0) {
        fprintf(stderr,"su: uid %d not allowed to su\n", myuid);
        return 1;
    }

    for (i = 1; (i < argc) && (arg_id == 0); i++)
    {
	if (argv[i][0] != '-')
	{
	    arg_id = i;
	    break;
	}
	switch (argv[i][1])
	{
	    case 'L':
	        list_services();
		exit (0);
	    case 'C':
		noclose = 1;
		break;
	    case 'n':
	    	daemon_mode = 0;
		break;
	    case 'h':
	        printf (usage, argv[0]);
		exit (0);
	    case 'N':
	    	if (i < argc-1)
		{
		    service_name = argv[++i];
		    break;
		}
		/* fall through */
	    case 'R':
	    	if (i < argc-1)
		{
		    service_name = argv[++i];
		    if (kill_service (service_name, SIGUSR1))
			exit (1);

		    exit (0);
		}
		/* fall through */
	    case 'K':
	    	if (i < argc-1)
		{
		    service_name = argv[++i];
		    if (kill_service(service_name, SIGTERM))
			exit (1);

		    exit (0);
		}
		/* fall through */
	    case 'i':
	    	if (i < argc-1)
		{
		    i++;
		    interval = atoi(argv[i]);
		    break;
		}
		/* fall through */
	    case 'l':
	    	if (i < argc-1)
		{
		    i++;
		    loops = atoi(argv[i]);
		    break;
		}
		/* fall through */
	    case 'r':
	    	if (i < argc-1)
		{
		    i++;

		    pw = getpwnam(argv[i]);
		    if(pw == 0) {
			uid = gid = atoi(argv[i]);
		    } else {
			uid = pw->pw_uid;
			gid = pw->pw_gid;
		    }
		    break;
		}
		/* fall through */
	    default:
	        fprintf (stderr, usage, argv[0]);
		exit (1);
	}
    }

    if ((arg_id == 0) || (arg_id >= argc)) {
	fprintf (stderr, usage, argv[0]);
	exit (1);
    }

    if (service_name == NULL)
	service_name = basename (argv[arg_id]);

    /* exec ourselves to make sure ffd_service_name appears in 
     * /proc/pid/environ
     */
    if (getenv(SNAME) == NULL)
    {
	setenv (SNAME, service_name, 1);

	if (execvp (argv[0], argv))
	{
	    perror ("execvp");
	    exit(1);
	}
    }

    /* but dont inherit to other exec'ed children */
    unsetenv (SNAME);

    if (service_name == NULL)
	exit(1);

    if (purge_service(service_name))
	exit(1);

    if (daemon_mode) {
	if (daemon2 (strdup(get_pidfile(service_name)), interval, loops, 0, noclose, service_name))
	    exit (1);
    }

    /* now running as slave */

    if (uid != -1) {
	if(setgid(gid) || setuid(uid)) {
	    fprintf(stderr,"su: permission denied\n");
	    return 1;
	}
    }

    nargs = argc - arg_id;

    /* User specified command for exec. */
    if (nargs == 1 ) {
        if (execlp(argv[arg_id], argv[arg_id], NULL) < 0) {
            fprintf(stderr, "su: exec failed for %s Error:%s\n", argv[arg_id],
                    strerror(errno));
            return -errno;
        }
    } else {
        /* Copy the rest of the args from main. */
        char *exec_args[nargs+1];
        memset(exec_args, 0, sizeof(exec_args));
        memcpy(exec_args, &argv[arg_id], sizeof(exec_args));
        if (execvp(argv[arg_id], exec_args) < 0) {
            fprintf(stderr, "su: exec failed for %s Error:%s\n", argv[arg_id],
                    strerror(errno));
            return -errno;
        }
    }

    return 0;
}
