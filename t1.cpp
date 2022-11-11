#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <signal.h>
#include <sys/wait.h>
using namespace std;

#define PORT "25000"

#define MAXDATASIZE 100


void *get_in_addr(struct sockaddr *sa) {
    if(sa -> sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa) -> sin_addr);
    }
    return &(((struct sockaddr_in6*)sa) -> sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if(argc != 2) {
        cout << "error argc" << endl;
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        cout << "getaddinfo error" << endl;
        return 1;
    }

    p = servinfo;
    while(p != NULL) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        break;
    }

    if(p == NULL) {
        cout << "client: fail to connect" << endl;
        return 2;
    }

    char *msg;
    cout << "Please enter the username:" << endl;
    cin >> msg;

    int len; 
    len = strlen(msg);
    if(send(sockfd, msg, len, 0) == -1) {
        cout << "fail" << endl;
    }

    if(send(sockfd, "msg", len, 0) == -1) {
        cout << "fail" << endl;
    }
    close(sockfd);
    return 0;

}
