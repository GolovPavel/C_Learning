#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>

#define PROC_COUNT 40

int main(){

    int i;
    int parent_pid = getpid();
    pid_t cpid;
    int semid = semget(IPC_PRIVATE, PROC_COUNT, IPC_CREAT | 0666);
    printf("semid: %d\n", semid);
    union semun {
      int val;
      struct semid_ds *buf;
      ushort *array;
    } arg;


    /* Initialize semophore */
    arg.val = 1;
    int res = semctl(semid, 0, SETVAL, arg);

    /* Fork and print in order */
    for (i = 0; i < PROC_COUNT; i++){
        fork();

        if(getpid() != parent_pid) {
            while(semctl(semid, i, GETVAL, 0) != 1);
            printf("%d\n", i);
            arg.val = 1;
            semctl(semid, ++i, SETVAL, arg);
            exit(0);
        }
    }

    /* Wait chiild process */
    while((cpid = wait(NULL)) > 0);

    /* Delete semophore */
    semctl(semid, 0, IPC_RMID);

    exit(0);
}
