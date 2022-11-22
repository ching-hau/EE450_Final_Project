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
#include <fstream>

using namespace std;


#define LOCAL_HOST "127.0.0.1"
#define MAXBUFSIZE 1024
#define C_UDP_PORT "21653"
#define FAIL -1
#define INVALID_USER '2'
#define INVALID_PWD '1'
#define VALID_USER '0'
#define CREDFILE "cred.txt"


int serverc_udp_socket;
int addr_info_result, udp_bind_result, serverc_recv_result, serverc_send_result;
struct addrinfo serverc_addr_cond, *serverc_addr_result;
struct sockaddr_storage their_addr;
socklen_t addr_len;
char buf[MAXBUFSIZE];
char verify_result[1];


void create_server_udpc_socket() {
    memset(&serverc_addr_cond, 0, sizeof serverc_addr_cond);
    serverc_addr_cond.ai_family = AF_UNSPEC;
    serverc_addr_cond.ai_socktype = SOCK_DGRAM;
    serverc_addr_cond.ai_flags = AI_PASSIVE;
    addr_info_result = getaddrinfo(NULL, C_UDP_PORT, &serverc_addr_cond, &serverc_addr_result);
    if(addr_info_result != 0) {
        cout << "ERROR: serverC fails to get address info" << endl;
        return;
    }

    serverc_udp_socket = socket(serverc_addr_result -> ai_family, serverc_addr_result -> ai_socktype, serverc_addr_result -> ai_protocol);

    if(serverc_udp_socket == FAIL) {
        perror("ERROR: serverC fails to create socket for client");
        exit(1);
    }

    udp_bind_result = bind(serverc_udp_socket, serverc_addr_result->ai_addr, serverc_addr_result->ai_addrlen);

    if(udp_bind_result == FAIL) {
        perror("ERROR: serverC fails to bind");
        close(serverc_udp_socket);
    }
    freeaddrinfo(serverc_addr_result);
    cout << "The ServerC is up and running using UDP on port " << C_UDP_PORT << " ." << endl;
}

char verify_user_info(char* buf) {
    string buf_string;
    buf_string.append(buf);
    int p = buf_string.find(" ");
    string username = buf_string.substr(0, p);
    string password = buf_string.substr(p + 1, buf_string.size() - 1);
    string cred_line;
    ifstream MyReadFile(CREDFILE);
    while(getline(MyReadFile, cred_line)) {
        if(int(cred_line[cred_line.length() - 1]) == 13) {
            cred_line = cred_line.substr(0, cred_line.length() - 1);
        }
        int del_pos = cred_line.find(",");
        string cur_username = cred_line.substr(0, del_pos);
        string cur_password = cred_line.substr(del_pos + 1, cred_line.length());
        if(cur_username == username) {
            if(cur_password == password) {
                return '0';
            }
            else {
                return '1';
            }
        }

    }
    return '2';
}


int main() {
    create_server_udpc_socket();
    while(true) {
        addr_len = sizeof(their_addr);
        serverc_recv_result = recvfrom(serverc_udp_socket, buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len);
        if(serverc_recv_result == FAIL) {
            perror("Error: serverc fails to recvfrom");
            exit(1);
        }
        cout << "The ServerC received an authentication request from the Main Server." << endl;
        verify_result[0] = verify_user_info(buf);
        serverc_send_result = sendto(serverc_udp_socket, verify_result, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len);
        cout << "The ServerC finished sending the response to the Main Server." << endl;
    }

    close(serverc_udp_socket);
}