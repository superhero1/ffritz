/* A command to release (V) the MDIO semaphore which protects access to the
 * external switch.
 * This can't be done in an application linked to libticc, since the 
 * loader of libticc itself attempts to obtain the lock and hangs forever.
 *
 * A dangling lock can be caused by a process crashing while the lock is held.
 * Did AVM forget to set SEM_UNDO on purpose .. ?
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>



int main (int argc, char **argv)
{
    struct sembuf sops;

    int semid = semget(0x61010760, 1, 0);

    if (semid == -1)
    {
	perror ("semget");
	return 1;
    }

    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop (semid, &sops, 1))
    {
	perror ("semop");
	return 1;
    }
    return 0;
}
