#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <strings.h>

#define MAXCONN 100

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
        exit(1)
    }

    /*Make socket NON-BLOCKIN */
    int status = fcntl(ssock, F_SETFL, fcntl(ssock, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1){
      perror("Could not make socket non-blocking");
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
    efd = epool_create1(0);
    event.data.fd = ssock;
    event.events = EPOLLIN;

    s = epoll_ctl(efd, EPOLL_CTL_ADD, ssock, &event);





    close(ssock);
    close(efd);
}
