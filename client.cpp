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
#define TCP_PORT "25653"
#define BUFSIZE 1024
#define FAIL -1


int server_addr_info;
int client_send_result, client_recv_result;
int client_tcp_socket;
int client_tcp_port;
struct addrinfo client_addr_cond, *client_addr_result;
char send_user_info_buf[BUFSIZE];
char recv_user_info_buf[BUFSIZE];
char send_course_info_buf[BUFSIZE];
char recv_course_info_buf[BUFSIZE];
string username;
string password;
string course;
string query_item;



void create_client_tcp_connection() {
    memset(&client_addr_cond, 0, sizeof client_addr_cond);
    client_addr_cond.ai_family = AF_UNSPEC;
    client_addr_cond.ai_socktype = SOCK_STREAM;
    client_addr_cond.ai_flags = AI_PASSIVE;
    server_addr_info = getaddrinfo(LOCAL_HOST, TCP_PORT, &client_addr_cond, &client_addr_result);

    if(server_addr_info != 0) {
        cout << "ERROR: serverM fails to get address info" << endl;
        return;
    }

    client_tcp_socket = socket(client_addr_result -> ai_family, client_addr_result -> ai_socktype, client_addr_result -> ai_protocol);
    cout << username << " sent an authentication request to the main server." << endl;

    if(client_tcp_socket == FAIL) {
        perror("ERROR: serverM fails to create socket for client");
        exit(1);
    }

    if (connect(client_tcp_socket, client_addr_result ->ai_addr, client_addr_result ->ai_addrlen) == -1) {
        close(client_tcp_socket);
        perror("client: connect");
        exit(1);
    }    
    client_tcp_port = ((struct sockaddr_in*) (void*) client_addr_result->ai_addr)->sin_port;

}

string get_user_info() {
    string str;
    cout <<"Please enter the username: ";
    cin >> username;
    cout << "Please enter the password: ";
    cin >> password;
    str = username + " " + password;
    return str;    
}

string get_user_query() {
    string str;
    cout << "Please enter the course code to query: ";
    cin >> course;
    cout << "Please enter the category (Credit / Professor / Days / CourseName): ";
    cin >> query_item;
    str = course + " " + query_item;
    return str;
}

int main(int argc, char *argv[]) {
    cout << "The client is up and running." << endl;
    int attempt = 3;
    string user_info = get_user_info();
    strncpy(send_user_info_buf, user_info.c_str(), BUFSIZE);
    create_client_tcp_connection();

    while(attempt >= 0) {
        attempt --;
        if(attempt < 2) {
            user_info = get_user_info();
            strncpy(send_user_info_buf, user_info.c_str(), BUFSIZE);
        }
        client_send_result = send(client_tcp_socket, send_user_info_buf, sizeof(send_user_info_buf), 0);
        if(client_send_result == FAIL) {
            perror("ERROR: client fails to send");
        }

        client_recv_result = recv(client_tcp_socket, recv_user_info_buf, BUFSIZE, 0);

        if(strcmp(recv_user_info_buf, "0") == 0) {
            while(true) {
                string query_info = get_user_query();
                strncpy(send_course_info_buf, query_info.c_str(), BUFSIZE);
                send(client_tcp_socket, send_course_info_buf, sizeof(send_user_info_buf), 0);
                cout << username << " sent a request to the main server." << endl;
                recv(client_tcp_socket, recv_course_info_buf, BUFSIZE, 0);
                cout << "The client received the response from the Main server using TCP over port " << client_tcp_port << "." << endl;
                cout << recv_course_info_buf << endl;
                cout << "-----Start a new request-----" << endl;
            }
        }
        else {
            if(attempt == -1) {
                cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
                exit(1);
            }
            
            cout << username << " received the result of authentication using TCP over port " << client_tcp_port << ". Authentication failed: " << endl;
            if(strcmp(recv_user_info_buf, "1") == 0) {
                cout << "Password does not match" << endl;
            }
            else {
                cout << "Username Does not exist" << endl;
            }
            memset(&recv_user_info_buf, 0, sizeof(recv_user_info_buf));
            cout << "Attempts remaining:" << attempt << endl;
        }
    }
}