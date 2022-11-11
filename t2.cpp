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

#define BACKLOG 10

#define MAXDATASIZE 100

void sigchld_handler(int s) {
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0); 
    
    errno = saved_errno;

}

void *get_in_addr(struct sockaddr *sa) {
    if(sa -> sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa) -> sin_addr);
    }
    return &(((struct sockaddr_in6*)sa) -> sin6_addr);
}

int main(void) {
    int sockfd, new_fd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        cout << "getadddrinfo" << endl;
        return 1;
    }
    p = servinfo;
    while(p != NULL) {
        if((sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        // int s;
        // s = bind(sockfd, p -> ai_addr, p -> ai_addrlen);
        if(bind(sockfd, p -> ai_addr, p -> ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        p = p -> ai_next;
        break;
    }
    freeaddrinfo(servinfo);

    if(p == NULL) {
        cout << "server: fail to bind" << endl;
    }

    if(listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }


    while(1) {
        // char *username;
        // char *password;
        char username[MAXDATASIZE];
        char password[MAXDATASIZE];
        cout << "waitiing for connections" << endl;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        cout << "server: got connection from " << s << endl;
        int count = 2;
        while(count > 0) {
            if(count == 2) {
                read(new_fd, username, 1024);
            }
            else {
                read(new_fd, password, 1024);
            }
            count --;
        }
        cout << "user " << username << endl;

        cout << "password " << password << endl;
        // int valread;
        // read(new_fd, username, 1024);
        // cout << username << endl;
        // read(new_fd, password, 1024);
        // cout << password << endl;

        // valread = read(new_fd, buf, 1024);
        // password = buf;
        // cout << username << " and " << password << endl;
        close(new_fd);
    }


    // while(1) {
    //     sin_size = sizeof their_addr;
    //     new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    //     if(new_fd == -1) {
    //         perror("accept");
    //         continue;
    //     }
    //     inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    //     cout << "server: got connection from " << s << endl;



    //     // while(new_fd) {
    //     //     cout << "here" << endl;
    //     //     numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
    //     //     buf[numbytes] = '\0';
    //     //     cout << "username " << buf << endl;
    //     // }
    //     if(!fork()) {
    //         close(sockfd);
    //         // if(send(new_fd, "Hello, world!", 13, 0) == -1) {
    //         //     perror("send");
    //         // }
    //         if(recv(sockfd, buf, MAXDATASIZE - 1, 0) == -1) {
    //             perror("recv");
    //         }
    //         cout << "username " << buf << endl;
    //         close(new_fd);
    //         exit(0);
    //     }
    //     close(new_fd);
    // }
    return 0;
}