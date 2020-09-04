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
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <alloca.h>
#include <time.h>
#include <sys/time.h>

#include "ffdaemon.h"

struct client_status
{
    pid_t		cpid;
    unsigned int	restarts;
    unsigned int        start_time;
    unsigned int	last_runtime;
    unsigned int	total_runtime;
    int			last_exit_code;
    char		args[ARG_MAX];
};

static int service_status (char *service_name, struct client_status *st);
static char service_args[ARG_MAX] = "";

const char *usage =
"usage: %s [-nCL] [-i sec] [-b sec] [-r user] [-i interval] [-l loops] [-N|K|R service] [-o dir] [-H limits] command args ...\n"
"   -n : No daemon mode\n"
"   -C : Do not close FDs\n"
"   -r : run as user[:group]\n"
"   -i : Restart delay after program terminates\n"
"   -b : Delay before first execution\n"
"   -l : Number of loops to run (0 = default = endless)\n"
"   -N : Name service rather than using the executable name\n"
"   -L : List all services\n"
"   -I : List a specific service services\n"
"   -K : Kill named service (%%all for all)\n"
"   -R : Restart named service (%%all for all)\n"
"   -o : Run service after chroot to dir\n"
"   -H : Hard limits for spawned process as comma separated list of: \n"
"        SPEC=number\n"
"        statements, where SPEC is the suffix of a setrlimit(2) resource id:\n"
"        AS, CORE, CPU, DATA, FSIZE, LOCKS, MEMLOCK, MSGQUEUE, NICE, NOFILE,\n"
"        NPROC, RSS, RTPRIO, RTTIME, SIGPENDING, STACK\n"
"        number is a decimal/hexadecimal number with optional k/m/g suffix,\n"
"        a value of 0 means no limit.\n"
"   -v : Verbose output for -I/-L.\n"
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

static void svc_info (const char *fmt, int val, char **str)
{
    int l;
    char *p = &service_args[strlen(service_args)];

    l = sizeof(service_args) - strlen(service_args) - 2;

    if (str)
	l = snprintf (p, l, fmt, (*str == NULL) ? "none" : *str);
    else
	l = snprintf (p, l, fmt, val);

    strcat (service_args, " ");

    if (l >= sizeof(service_args))
    {
	fprintf (stderr, "service_args too long: %d, max %d\n", l, sizeof(service_args));
	exit (1);
    }
}

char *get_service_info(const char *service_name)
{
    return service_args;
}

char *get_pidfile(const char *service_name)
{
    static char pf[1000];

    sprintf (pf, "%s/%s%s.pid", SN_DIR, SN_PREFIX, service_name);

    return pf;
}

char *get_client_pidfile(const char *service_name)
{
    static char pf[1000];

    sprintf (pf, "%s/.%s%s.pid", SN_DIR, SN_PREFIX, service_name);

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

/* returns
* 0  - service does not exist
* -1 - error (reading pid file, malloc, ...)
* >0 - PID of control process
*/
static int service_status (char *service_name, struct client_status *st)
{
    FILE *fd;
    char *buf = NULL;
    int pid = 0;
    char *pidfile = get_pidfile (service_name);
    char *cpidfile = get_client_pidfile (service_name);
    size_t count;

    /* check existing pidfile */
    fd = fopen (pidfile, "r");

    if (fd == NULL)
	return 0;

    if (fscanf (fd, "%d", &pid) != 1)
	return 0;

    if (!pid)
	return -1;

    if (!st)
	return pid;

    memset ((void*)st, 0, sizeof(st));

    /* skip leading space */
    fread (st->args, 1, 1, fd);
	
    count = fread (st->args, 1, sizeof(service_args), fd);
    st->args[count] = '\0';
    if (strstr(st->args, "=") == NULL)
	strcpy (st->args, "[bad service args]");

    fclose (fd);

    /* check if ffd_service_name environment variable of process matches */
    buf = get_service_name (pid);

    if (!buf)
	return -1;

    if (strcmp (service_name, buf))
	pid = -1;

    free (buf);

    fd = fopen (cpidfile, "r");

    if (fd != NULL)
    {
	fscanf (fd, "%d %u %u %u %u %d",
		&st->cpid, 
		&st->restarts, 
		&st->start_time,
		&st->last_runtime, 
		&st->total_runtime, 
		&st->last_exit_code);

	fclose (fd);
    }

    return pid;
}

static char *pid_info (int pid, int args)
{
    static char buffer[ARG_MAX];
    char fname[PATH_MAX];
    char *rc = "na";
    char *cmd, *arg;
    int fd = -1;
    int len;
    int i;

    sprintf (fname, "/proc/%d/cmdline", pid);
    fd = open (fname, O_RDONLY);

    if (fd == -1)
	goto out;

    if ((len = read (fd, buffer, sizeof(buffer))) == -1)
    {
	perror ("read");
	goto out;
    }

    cmd = buffer;
    arg = buffer + strlen(buffer) + 1;

    for (i = strlen(cmd)+1+strlen(arg); i < len;)
    {
	if (i+1 >= len)
	    break;

	buffer[i] = ' ';
	i += strlen(&buffer[i]);
    }

    rc = (args == 0) ? cmd : arg;

out:
    if (fd != -1)
	close (fd);

    return rc;
}

static int service_info (char *service_name, int verbose)
{
    int status;
    struct client_status st;
    struct timeval tv;
    unsigned int cur_runtime;

    status = service_status (service_name, &st);

    printf ("%-14s : ", service_name); 

    if (status == 0)
    {
	printf ("Not running\n");
	return 1;
    }

    if (status == -1)
    {
	printf ("STALE\n");
	return 1;
    }

    gettimeofday (&tv, NULL);
    cur_runtime = st.start_time ? tv.tv_sec - st.start_time : 0;

    printf ("PID=%-5d", status);
    printf (" cpid=%d times=%d/%d/%d/%d restarts=%d ",
    	st.cpid, 
	cur_runtime,
	st.last_runtime,
	st.total_runtime + cur_runtime,
	(st.total_runtime + cur_runtime) / (st.restarts + 1),
	st.restarts);

    if (st.restarts)
    {
	if (WIFEXITED(st.last_exit_code))
	    printf ("exit(%d)", WEXITSTATUS(st.last_exit_code));
	if (WIFSIGNALED(st.last_exit_code))
	    printf ("signal(%d)", WTERMSIG(st.last_exit_code));
	if (__WCOREDUMP(st.last_exit_code))
	    printf ("coredump");
	if (WIFSTOPPED(st.last_exit_code))
	    printf ("stopped(%d)", WSTOPSIG(st.last_exit_code));
	if (WIFCONTINUED(st.last_exit_code))
	    printf ("resumed");
    }
 
    printf ("\n");

    if (verbose)
    {
	printf ("%-14s : %s\n", service_name, st.args);
	printf ("%-14s : CMD=%s\n", service_name, pid_info (st.cpid, 0));
	printf ("%-14s : ARGS=%s\n", service_name, pid_info (st.cpid, 1));
    }
    return 0;
}
 
static void list_services(int verbose)
{
    DIR *dir = opendir (SN_DIR);
    struct dirent *de = NULL;
    char service_name[256];

    if (!dir)
    {
	perror ("opendir");
	return;
    }

    while ((de = readdir(dir)) != NULL)
    {
	if (strncmp (de->d_name, SN_PREFIX, strlen(SN_PREFIX)) ||
	    (strstr(de->d_name, ".pid") == NULL))
	    continue;

	strncpy (service_name, de->d_name+4, sizeof(service_name));

	*strstr(service_name, ".pid") = '\0';

	service_info (service_name, verbose);
    }; 

    closedir (dir);
}

static void iterate_services(int (*cb)(char *service_name, int arg), int arg)
{
    DIR *dir = opendir (SN_DIR);
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
	if (strncmp (de->d_name, SN_PREFIX, strlen(SN_PREFIX)) ||
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

int do_limits (char *limits, int check_only)
{
    int i;
    struct rlimit rlim;
    char *s;
    char *copy;
    rlim_t lim, mul;
    int resource;

    static struct
    {
	char *name;
	int resource;
    }
    ids[] =
    {
    	{ "AS", RLIMIT_AS },
    	{ "CORE", RLIMIT_CORE },
    	{ "CPU", RLIMIT_CPU },
    	{ "DATA", RLIMIT_DATA },
    	{ "FSIZE", RLIMIT_FSIZE },
    	{ "LOCKS", RLIMIT_LOCKS },
    	{ "MEMLOCK", RLIMIT_MEMLOCK },
    	{ "MSGQUEUE", RLIMIT_MSGQUEUE },
    	{ "NICE", RLIMIT_NICE },
    	{ "NOFILE", RLIMIT_NOFILE },
    	{ "NPROC", RLIMIT_NPROC },
    	{ "RSS", RLIMIT_RSS },
    	{ "RTPRIO", RLIMIT_RTPRIO },
    	{ "RTTIME", RLIMIT_RTTIME },
    	{ "SIGPENDING", RLIMIT_SIGPENDING },
    	{ "STACK", RLIMIT_STACK },
    	{ NULL, 0 }
    };

    if (!limits)
	return 0;

    copy = alloca (strlen(limits)+1);
    strcpy (copy, limits);

    for (s = strtok (copy, "="); s != NULL; s = strtok (NULL, "="))
    {
	resource = -1;
	for (i = 0; ids[i].name != NULL; i++)
	{
	    if (!strcmp (ids[i].name, s))
	    {
		resource = ids[i].resource;
		break;
	    }
	}

	if (resource == -1)
	{
	    fprintf (stderr, "Unknown resource ID: %s\n", s);
	    return 1;
	}

	s = strtok (NULL, ",");

	if ((s == NULL) || (strlen(s) == 0))
	{
	    fprintf (stderr, "Missing limit for resource ID %s\n", ids[i].name);
	    return 1;
	}

	if (check_only)
	    continue;

	switch (s[strlen(s)-1])
	{
	    case 'k':
	    case 'K':
	    	mul = 1024;
		s[strlen(s)-1] = 0;
		break;
	    case 'm':
	    case 'M':
	    	mul = 1024*1024;
		s[strlen(s)-1] = 0;
		break;
	    case 'g':
	    case 'G':
	    	mul = 1024*1024*1024;
		s[strlen(s)-1] = 0;
		break;
	    default:
	    	mul = 1;
	}
	
        lim = strtoul(s, NULL, 0) * mul;

	if (getrlimit (resource, &rlim))
	{
	    fprintf (stderr, "WARNING: getrlimit(%s): %s\n", ids[i].name, strerror(errno));
	    continue;
	}

	rlim.rlim_max = lim;
	if (rlim.rlim_cur > rlim.rlim_max)
	    rlim.rlim_cur = rlim.rlim_max;

	if (setrlimit (resource, (const struct rlimit*)&rlim))
	{
	    fprintf (stderr, "WARNING: setrlimit(%s): %s\n", ids[i].name, strerror(errno));
	}

	printf ("%s -> 0x%lx 0x%lx\n", ids[i].name, rlim.rlim_cur, rlim.rlim_max);
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
    int start_delay = 0;
    int loops = 0;
    int noclose = 0;
    char *service_name = NULL;
    char *chroot_path = NULL;
    char *limits = NULL;
    int verbose = 0;

    /* Until we have something better, only root and the shell can use su. */
    myuid = getuid();
    if (myuid != 0) {
        fprintf(stderr,"su: uid %d not allowed to su\n", myuid);
        return 1;
    }

    strcpy (service_args, "");

    for (i = 1; (i < argc) && (arg_id == 0); i++)
    {
	if (argv[i][0] != '-')
	{
	    arg_id = i;
	    break;
	}
	switch (argv[i][1])
	{
	    case 'v':
		verbose = 1;
		break;
	    case 'L':
	        list_services(verbose);
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
	    case 'b':
	    	if (i < argc-1)
		{
		    i++;
		    start_delay = atoi(argv[i]);
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
	    case 'o':
	    	if (i < argc-1)
		{
		    i++;
		    chroot_path = argv[i];
		    break;
		}
		/* fall through */
	    case 'H':
	    	if (i < argc-1)
		{
		    limits = argv[++i];
		    break;
		}
		/* fall through */
	    case 'I':
	    	if (i < argc-1)
		{
		    return (service_info (argv[++i], verbose));
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

    if (do_limits (limits, 1))
	exit (1);

    if (service_name == NULL)
	service_name = basename (argv[arg_id]);

    if (service_name == NULL)
	exit(1);

    if (purge_service(service_name))
	exit(1);

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

    /* set config info to be stored in pidfile */
    svc_info ("VERB=%d", verbose, NULL);
    svc_info ("NOCLOSE=%d", noclose, NULL);
    svc_info ("DAEMON=%d", daemon_mode, NULL);
    svc_info ("INTERVAL=%d", interval, NULL);
    svc_info ("LOOPS=%d", loops, NULL);
    svc_info ("UID=%d", uid, NULL);
    svc_info ("GID=%d", uid, NULL);
    svc_info ("CHROOT=%s", 0, &chroot_path);
    svc_info ("RLIM=%s", 0, &limits);
 
    if (daemon_mode) {
	switch (daemon2 (interval, start_delay, loops, 0, noclose, service_name))
	{
	    case 0:
	    	/* running as slave */
	    	break;
	    case 2:
	    	/* client exit, eceeded loops.
		 * Don't exit so that the service stats are still
		 * available
		 */
		while (1)
		    sleep (1000);
		break;
	    default:
	    	/* fatal error */
	    	exit (1);
	}
    }

    /* now running as slave */

    if (chroot_path)
    {
	if (chroot(chroot_path))
	{
	    perror("chroot");
	    return 1;
	}
	chdir (getenv("HOME"));
    }

    do_limits (limits, 0);

    if (uid != -1) {
	if(setgid(gid) || setuid(uid)) {
	    fprintf(stderr,"su: permission denied\n");
	    return 1;
	}
	chdir (getenv("HOME"));
    }

    nargs = argc - arg_id;

    /* User specified command for exec. */
    if (nargs == 1 ) {
        if (execlp(argv[arg_id], argv[arg_id], NULL) < 0) {
            fprintf(stderr, "exec failed for %s : %s\n", argv[arg_id],
                    strerror(errno));
            return -errno;
        }
    } else {
        /* Copy the rest of the args from main. */
        char *exec_args[nargs+1];
        memset(exec_args, 0, sizeof(exec_args));
        memcpy(exec_args, &argv[arg_id], sizeof(exec_args));
        if (execvp(argv[arg_id], exec_args) < 0) {
            fprintf(stderr, "exec failed for %s : %s\n", argv[arg_id],
                    strerror(errno));
            return -errno;
        }
    }

    return 0;
}
