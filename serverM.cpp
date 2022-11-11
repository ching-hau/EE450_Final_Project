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


#define LOCAL_HOST "127.0.0.1"
#define MAXBUFSIZE 1024
#define BACKLOG 10
#define TCP_PORT "25653"
#define C_UDP_PORT "21653"
#define CS_UDP_PORT "22653"
#define EE_UDP_PORT "23653"


#define FAIL -1

const string SERVERM_TCP_ERROR = "[ERROR]: serverM fails to accept client";
const string GET_ADDR_ERROR = "[ERROR]: serverM fails to get address info";
const string TCP_SOCKET_ERROR = "[ERROR]: serverM fails to create socket for client";
const string TCP_BIND_ERROR = "[ERROR]: serverM fails to bind";
const string TCP_LISTEN_ERROR = "[ERROR]: serverM fails to listen";
const string TCP_RECV_ERROR = "[ERROR]: serverM fails to receive client";
const string UDPC_SEND_ERROR = "[ERROR]: serverM fails to send to serverc";
const string UDPC_RECV_ERROR = "[ERROR]: serverM fails to receive to serverc";
const string UDPC_SOCKET_ERROR = "[ERROR]: serverM fails to create socket for serverc";
string course;
string category;
char *username;
int server_tcp_socket, tcp_child_socket, serverc_udp_socket;
int addr_info_result, tcp_bind_result, tcp_listen_result, server_recv_result, server_send_udpc_result, server_recv_udpc_result;
int server_send_result;
int udpc_addr_info_result, udpc_bind_result;
int udpcs_addr_info_result, udpee_addr_info_result;
int servercs_udp_socket, serveree_udp_socket;

struct addrinfo server_addr_cond, *server_addr_result;
struct addrinfo serverc_addr_cond, *serverc_addr_result;
struct addrinfo servercs_addr_cond, *servercs_addr_result;
struct addrinfo serveree_addr_cond, *serveree_addr_result;

struct sockaddr_storage client_addr, serverc_addr;
socklen_t client_addr_size, serverc_addr_len;
char buf[MAXBUFSIZE];
char servercs_buf[MAXBUFSIZE];
char serveree_buf[MAXBUFSIZE];
char encrypted_buf[MAXBUFSIZE];
char backup_buf[MAXBUFSIZE];
char serverc_buf[MAXBUFSIZE];


void create_server_tcp_socket();
void error_check(int status, string msg);


void create_server_tcp_socket() {
    memset(&server_addr_cond, 0, sizeof server_addr_cond);
    server_addr_cond.ai_family = AF_UNSPEC;
    server_addr_cond.ai_socktype = SOCK_STREAM;
    server_addr_cond.ai_flags = AI_PASSIVE;

    addr_info_result = getaddrinfo(NULL, TCP_PORT, &server_addr_cond, &server_addr_result);
    error_check(addr_info_result, GET_ADDR_ERROR);

    server_tcp_socket = socket(server_addr_result -> ai_family, server_addr_result -> ai_socktype, server_addr_result -> ai_protocol);
    error_check(server_tcp_socket, TCP_SOCKET_ERROR);
    
    tcp_bind_result = bind(server_tcp_socket, server_addr_result->ai_addr, server_addr_result->ai_addrlen);
    error_check(tcp_bind_result, TCP_BIND_ERROR);

    freeaddrinfo(server_addr_result);

    tcp_listen_result = listen(server_tcp_socket, BACKLOG);
    error_check(tcp_bind_result, TCP_LISTEN_ERROR);
    
}

void create_serverc_udp_socket() {
    memset(&serverc_addr_cond, 0, sizeof serverc_addr_cond);
    serverc_addr_cond.ai_family = AF_UNSPEC;
    serverc_addr_cond.ai_socktype = SOCK_DGRAM;
    udpc_addr_info_result = getaddrinfo(LOCAL_HOST, C_UDP_PORT, &serverc_addr_cond, &serverc_addr_result);
    serverc_udp_socket = socket(serverc_addr_result -> ai_family, serverc_addr_result -> ai_socktype, serverc_addr_result -> ai_protocol);
}

void create_servercs_udp_socket() {
    memset(&servercs_addr_cond, 0, sizeof servercs_addr_cond);
    servercs_addr_cond.ai_family = AF_UNSPEC;
    servercs_addr_cond.ai_socktype = SOCK_DGRAM;
    udpcs_addr_info_result = getaddrinfo(LOCAL_HOST, CS_UDP_PORT, &servercs_addr_cond, &servercs_addr_result);
    servercs_udp_socket = socket(servercs_addr_result -> ai_family, servercs_addr_result -> ai_socktype, servercs_addr_result -> ai_protocol);
}

void create_serveree_udp_socket() {
    memset(&serveree_addr_cond, 0, sizeof serveree_addr_cond);
    serveree_addr_cond.ai_family = AF_UNSPEC;
    serveree_addr_cond.ai_socktype = SOCK_DGRAM;
    udpcs_addr_info_result = getaddrinfo(LOCAL_HOST, EE_UDP_PORT, &serveree_addr_cond, &serveree_addr_result);
    serveree_udp_socket = socket(serveree_addr_result -> ai_family, serveree_addr_result -> ai_socktype, serveree_addr_result -> ai_protocol);
}

void encrypt_user_info(char input[]) {
    for(int i = 0; i < strlen(input);  i ++) {
        int cur_char_code = int(input[i]);
        if(cur_char_code >= 48 && cur_char_code <= 57) {
            cur_char_code += 4;
            if(cur_char_code > 57) {
                cur_char_code -= 10;
            }
            input[i] = (char) cur_char_code;
        }
        else if(cur_char_code >= 65 && cur_char_code <= 90) {
            cur_char_code += 4;
            if(cur_char_code > 90) {
                cur_char_code -= 26;
            }
            input[i] = (char) cur_char_code;
        }
        else if(cur_char_code >= 97 && cur_char_code <= 122) {
            cur_char_code += 4;
            if(cur_char_code > 122) {
                cur_char_code -= 26;
            }
            input[i] = (char) cur_char_code;
        }
    }
}
void error_check(int status, string msg) {
    if(status == FAIL) {
        cout << msg << endl;
        exit(1);
    }
}

void get_query_info(char* buf) {
    string buf_string;
    buf_string.append(buf);
    int p = buf_string.find(" ");
    course = buf_string.substr(0, p);
    category = buf_string.substr(p + 1, buf_string.size() - 1);
}

int main(void) {
    cout << "The main server is up and running." << endl;
    create_server_tcp_socket();
    create_serverc_udp_socket();
    create_servercs_udp_socket();
    create_serveree_udp_socket();

    
    serverc_addr_len = sizeof(serverc_addr);
    cout << "waitiing for connections" << endl;
    tcp_child_socket = accept(server_tcp_socket, (struct sockaddr *)&client_addr, &client_addr_size);
    error_check(tcp_child_socket, SERVERM_TCP_ERROR);

    while(true) {
        server_recv_result = recv(tcp_child_socket, buf, MAXBUFSIZE, 0);
        error_check(server_recv_result, TCP_RECV_ERROR);
 

        strncpy(encrypted_buf, buf, sizeof(buf));
        encrypt_user_info(encrypted_buf);
        strncpy(backup_buf, buf, sizeof(buf));
        username = strtok(backup_buf, " ");

        cout << "The main server received the authentication for " << username << " using TCP over port "<< TCP_PORT << "." << endl;
        serverc_addr_len = sizeof(serverc_addr);
        server_send_udpc_result = sendto(serverc_udp_socket, encrypted_buf, MAXBUFSIZE, 0, serverc_addr_result -> ai_addr, serverc_addr_result -> ai_addrlen);
        cout << "The main server sent an authentication request to serverC." << endl;
        error_check(server_send_udpc_result, UDPC_SEND_ERROR);

        
        server_recv_udpc_result = recvfrom(serverc_udp_socket, serverc_buf, MAXBUFSIZE, 0, serverc_addr_result -> ai_addr, &(serverc_addr_result -> ai_addrlen));
        error_check(server_recv_udpc_result, UDPC_RECV_ERROR);
        cout << "The main server received the result of the authentication request from ServerC using UDP over port " << C_UDP_PORT << "." << endl;

        if(strcmp(serverc_buf, "0") != 0) {
            server_send_result = send(tcp_child_socket, serverc_buf, sizeof(serverc_buf), 0);
            cout << "The main server sent the authentication result to the client." << endl;
            continue;
        }
        else {
            server_send_result = send(tcp_child_socket, serverc_buf, sizeof(serverc_buf), 0);
            cout << "The main server sent the authentication result to the client." << endl;
            break;
        }
        cout << serverc_buf << endl;
    }

    while(true) {
        recv(tcp_child_socket, buf, MAXBUFSIZE, 0);
        get_query_info(buf);
        cout << "The main server received from " << username << "to query course " << course << " about " << category << "." << endl;
        if(buf[0] == 'C' && buf[1] == 'S') {
            sendto(servercs_udp_socket, buf, MAXBUFSIZE, 0, servercs_addr_result -> ai_addr, servercs_addr_result -> ai_addrlen);
            cout << "The main server sent a request to serverCS." << endl;
            recvfrom(servercs_udp_socket, servercs_buf, MAXBUFSIZE, 0, servercs_addr_result -> ai_addr, &(servercs_addr_result -> ai_addrlen));
            cout << "The main server received the response from serverCS using UDP over port " << CS_UDP_PORT "." << endl;
            send(tcp_child_socket, servercs_buf, sizeof(servercs_buf), 0);
            cout << servercs_buf << endl;
        }
        else {
            sendto(serveree_udp_socket, buf, MAXBUFSIZE, 0, serveree_addr_result -> ai_addr, serveree_addr_result -> ai_addrlen);
            cout << "The main server sent a request to serverEE." << endl;
            recvfrom(serveree_udp_socket, serveree_buf, MAXBUFSIZE, 0, serveree_addr_result -> ai_addr, &(serveree_addr_result -> ai_addrlen));
            cout << "The main server received the response from serverEE using UDP over port " << EE_UDP_PORT "." << endl;
            send(tcp_child_socket, serveree_buf, sizeof(serveree_buf), 0);
            cout << "The main server sent the query information to the client." << endl;
            cout << serveree_buf << endl;
        }
    }

}