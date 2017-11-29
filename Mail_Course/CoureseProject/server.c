#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <sys/signalfd.h>

#define MAXCONN 100
#define MAXEVENTS 64
#define MAXHOSTSIZE 32
#define MAXPORTSIZE 16
#define BUFFERSIZE 4096


void write_log(FILE*, int, char[], int count);

int main(int argc, char* argv[]){

    if(argc < 2) {
        printf("Please, type name of log file as second arg.\n");
        exit(1);
    }

    char* filename = argv[1];

    int sigfd;
    sigset_t mask;

    int ssock;
    struct sockaddr_in servaddr;

    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    int status;

    /* Open file for logging */
    FILE* fm = fopen(filename, "a+");

    /* Signal processor */
    status = sigemptyset(&mask);
    if(status < 0){
        perror("Sigemptyset error");
        exit(1);
    }
    status = sigaddset(&mask, SIGTSTP);
    if(status < 0){
        perror("Sigaddset error");
        exit(1);
    }
    status = sigprocmask(SIG_BLOCK, &mask, NULL);
    if(status < 0){
        perror("Sigprocmask error");
        exit(1);
    }

    sigfd = signalfd(-1, &mask, 0);
    if(sigfd < 0){
        perror("Signalfd error");
        exit(1);
    }

    /* Creating server socket */
    ssock = socket(AF_INET, SOCK_STREAM, 0);
    if(ssock < 0){
        perror("Could not create socket");
        exit(1);
    }

    /*Make socket NON-BLOCKIN */
    status = fcntl(ssock, F_SETFL, fcntl(ssock, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1){
      perror("Could not make server socket non-blocking");
      exit(1);
    }

    /* Binding server socket */
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    inet_aton("127.0.0.1", &servaddr.sin_addr.s_addr);

    if(bind(ssock, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1){
        perror("Could not bind socket");
        exit(1);
    }

    if(listen(ssock, MAXCONN) == -1) {
        perror("Could not listen connections");
        exit(1);
    }

    /* Create and config epool */
    efd = epoll_create1(0);

    event.data.fd = ssock;
    event.events = EPOLLIN;
    status = epoll_ctl(efd, EPOLL_CTL_ADD, ssock, &event);
    if(status == -1){
        perror("Could not server ctl epoll");
        exit(1);
    }

    event.data.fd = sigfd;
    event.events = EPOLLIN;
    status = epoll_ctl(efd, EPOLL_CTL_ADD, sigfd, &event);
    if(status == -1){
        perror("Could not signal ctl epoll");
    }

    events = calloc(MAXEVENTS, sizeof(event));
    printf("Server started. Use Ctrl+z to stop server.\n");

    /* Server loop */
    while(1){
        /* Count of events occured */
        int i, n;

        n = epoll_wait(efd, events, MAXEVENTS, -1);

        for(int i = 0; i < n; i++){

            /* If error occured or socket is not ready for read */
            if((events[i].events & EPOLLERR)
                || (events[i].events & EPOLLHUP)) {
                printf("Error ocured with socket %d", events[i].data.fd);
                close(events[i].data.fd);
                continue;
            } else if(sigfd == events[i].data.fd) {
                /* Ctrl + z => terminate programm */
                printf("\nServer stoped\n");
                free(events);
                close(ssock);
                close(sigfd);
                close(efd);
                fclose(fm);
                exit(0);
            } else if(ssock == events[i].data.fd) {
                /* Event occured on listener socket => new connection[s] recieved */

                /* Get all new connections */
                while(1){
                    char port[MAXPORTSIZE];
                    char host[MAXHOSTSIZE];

                    struct sockaddr_in clientaddr;
                    socklen_t in_len;
                    int csock;

                    in_len = sizeof(clientaddr);

                    csock = accept(ssock, (struct sockaddr*) &clientaddr,
                        &in_len);

                    if (csock == -1){
                        /* No connnections enough */
                        if((errno == EAGAIN)){
                            break;
                        }
                        else {
                            perror("Problems with connect user");
                            break;
                        }
                    }

                    status = getnameinfo((struct sockaddr*) &clientaddr, sizeof(clientaddr),
                        host, sizeof(host),
                        port, sizeof(port),
                        NI_NUMERICHOST | NI_NUMERICSERV);

                    if(status == 0) {
                        printf("Client with fd %d and address %s:%s connected\n",
                            csock, host, port);
                    }

                    /*Make socket NON-BLOCKIN */
                    status = fcntl(csock, F_SETFL, fcntl(csock, F_GETFL, 0) | O_NONBLOCK);
                    if (status == -1){
                        perror("Could not make client socket non-blocking");
                        exit(1);
                    }

                    event.data.fd = csock;
                    event.events = EPOLLIN | EPOLLOUT;

                    status = epoll_ctl(efd, EPOLL_CTL_ADD, csock, &event);
                    if(status == -1){
                        perror("Could not client ctl epoll");
                        exit(1);
                    }
                }
                continue;

            } else if(events[i].events & EPOLLIN){

                /* User write data => we should read it and send to everyone, except this user */

                int clfd = events[i].data.fd;

                while(1) {
                    char buf[BUFFERSIZE];
                    int count;

                    count = recv(clfd, buf, sizeof(buf), 0);

                    /* Some error? */
                    if(count == -1) {
                        /* errno == EAGAIN means, that we don't have any data */
                        if(errno != EAGAIN){
                            perror("Problems while reading from client socket");
                            close(clfd);
                        } else{
                            break;
                        }
                    } else if(count == 0) {
                        /* Client shotdown */
                        close(clfd);
                    }

                    /* Write data to other sockets */
                    int j, outfd;

                    for(j = 0; j < n; j++){
                        if((events[j].data.fd == clfd)
                        || !(events[j].events & EPOLLOUT)) {
                            continue;
                        }

                        outfd = events[j].data.fd;

                        status = send(outfd, buf, count, 0);

                        if(status == -1){
                            perror("Could not send data to socket");
                            close(outfd);
                        }
                    }

                    /* Write data to log */

                    write_log(fm, clfd, buf, count);
                }
            }
        }
    }
}

void write_log(FILE* fm, int sock, char msg[], int msglen){
    char host[MAXHOSTSIZE];
    char port[MAXPORTSIZE];
    int status;

    struct sockaddr claddr;
    socklen_t cl_len = sizeof(claddr);
    status = getpeername(sock, &claddr, &cl_len);
    if(status == -1) {
        perror("Could not getpeername");
    }
    status = getnameinfo(&claddr, sizeof(claddr),
        host, sizeof(host),
        port, sizeof(port),
        NI_NUMERICHOST | NI_NUMERICSERV);
    if(status == -1) {
        perror("Could not getnameinfo");
    }

    long int ttime = time(NULL);
    fprintf(fm, "Time: %sAddress: %s:%s\nMessage: ", ctime(&ttime), host, port);
    fwrite(msg, 1, msglen, fm);
    fprintf(fm, "\n\n");
}

