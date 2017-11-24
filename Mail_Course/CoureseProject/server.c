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

#define MAXCONN 100
#define MAXEVENTS 64
#define MAXHOSTSIZE 32
#define MAXPORTSIZE 16


int main(){
    int ssock;
    struct sockaddr_in servaddr;

    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    /* Creating server socket */
    ssock = socket(AF_INET, SOCK_STREAM, 0);
    if(ssock < 0){
        perror("Could not create socket");
        exit(1);
    }

    /*Make socket NON-BLOCKIN */
    int status = fcntl(ssock, F_SETFL, fcntl(ssock, F_GETFL, 0) | O_NONBLOCK);
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

    events = calloc(MAXEVENTS, sizeof(event));

    /* Server loop */
    while(1){
        /* Count of events occured */
        int i, n;

        n = epoll_wait(efd, events, MAXEVENTS, -1);

        for(int i = 0; i < n; i++){

            /* If error occured or socket is not ready for read */
            if((events[i].events & EPOLLERR)
                || (events[i].events & EPOLLHUP)
                || (!(events[i].events & EPOLLIN))){
                printf("Error ocured with socket %d" % events[i].data.fd);
                close(events[i].data.fd);
                continue;
            } else if (ssock == events[i].data.fd) {
                /* Event occured on listener socket => new connection[s] recieved */

                /* Get all new connections */
                while(1){
                    struct sockaddr_in clientaddr;
                    int csock;
                    char port[MAXPORTSIZE];
                    char host[MAXHOSTSIZE];

                    csock = accept(ssock, (struct sockaddr*) &clientaddr,
                        sizeof(clientaddr));

                    if (csock == -1){
                        /* No connnections enough */
                        if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
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
                        printf("Client with fd %d and address %s:%s connected",
                            csock, host, port);
                    }

                    /*Make socket NON-BLOCKIN */
                    status = fcntl(csock, F_SETFL, fcntl(csock, F_GETFL, 0) | O_NONBLOCK);
                    if (status == -1){
                        perror("Could not make client socket non-blocking");
                        exit(1);
                    }

                    event.data.fd = csock;
                    event.events = EPOLLIN;

                    status = epoll_ctl(efd, EPOLL_CTL_ADD, csock, &event);
                    if(status == -1){
                        perror("Could not client ctl epoll");
                        exit(1);
                    }
                }
                continue;

            } else{
                //TODO READ DATA FROM USER
            }
        }
    }



    close(ssock);
    close(efd);
}
