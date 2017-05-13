/*
**
** Copyright 2008, The Android Open Source Project
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
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>

extern int daemon2 (char *pdfile, int delay, int nochdir, int noclose);

const char *usage = "usage: %s [-n] [-r user] [-p pidfile] [-i interval] command args ...\n";

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
    char *pidfile = NULL;
    int nargs;
    int interval = 2;

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
	    case 'n':
	    	daemon_mode = 0;
		break;
	    case 'h':
	        printf (usage, argv[0]);
		exit (0);
	    case 'i':
	    	if (i < argc-1)
		{
		    i++;
		    interval = atoi(argv[i]);
		    break;
		}
		/* fall through */
	    case 'p':
	    	if (i < argc-1)
		{
		    i++;
		    pidfile = argv[i];
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

    if (daemon_mode) {
	if (daemon2 (pidfile, interval, 0, 0))
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
