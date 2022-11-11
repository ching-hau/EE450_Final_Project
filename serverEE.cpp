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
#define EE_UDP_PORT "23653"
#define FAIL -1
#define CSFILE "ee.txt"


int serverEE_udp_socket;
int addr_info_result, udp_bind_result, serverEE_recv_result, serverEE_send_result;
struct addrinfo serverEE_addr_cond, *serverEE_addr_result;
struct sockaddr_storage their_addr;
socklen_t addr_len;
string course;
string category;
char buf[MAXBUFSIZE];
char result_buf[MAXBUFSIZE];

const string GET_ADDR_ERROR = "[ERROR]: serverEE fails to get address info.";
const string SERVEREE_SOCKET_ERROR = "[ERRROR]: serverEE fails to create socket for client.";
const string SERVEREE_BIND_ERROR = "[ERRROR]: serverEE fails to bind.";

void error_check(int status, string msg) {
    if(status == FAIL) {
        cout << msg << endl;
        exit(1);
    }
}

void create_serveree_udp_socket() {
    memset(&serverEE_addr_cond, 0, sizeof serverEE_addr_cond);
    serverEE_addr_cond.ai_family = AF_UNSPEC;
    serverEE_addr_cond.ai_socktype = SOCK_DGRAM;
    serverEE_addr_cond.ai_flags = AI_PASSIVE;
    addr_info_result = getaddrinfo(NULL, EE_UDP_PORT, &serverEE_addr_cond, &serverEE_addr_result);
    error_check(addr_info_result, GET_ADDR_ERROR);

    serverEE_udp_socket = socket(serverEE_addr_result -> ai_family, serverEE_addr_result -> ai_socktype, serverEE_addr_result -> ai_protocol);
    error_check(serverEE_udp_socket, SERVEREE_SOCKET_ERROR);

    udp_bind_result = bind(serverEE_udp_socket, serverEE_addr_result->ai_addr, serverEE_addr_result->ai_addrlen);
    error_check(udp_bind_result, SERVEREE_BIND_ERROR);

    freeaddrinfo(serverEE_addr_result);
    cout << "The Server EE is up and running using UDP on port " << EE_UDP_PORT << "." << endl;

}

string inspect_course_info(char* buf) {
    string buf_string;
    buf_string.append(buf);
    int p = buf_string.find(" ");
    course = buf_string.substr(0, p);
    category = buf_string.substr(p + 1, buf_string.size() - 1);
    string cred_line;
    ifstream MyReadFile(CSFILE);
    cout << course << endl;
    string result = "Didn't find the course: " + course;
    while(getline(MyReadFile, cred_line)) {
        cred_line.pop_back();
        char *cstr = new char[cred_line.length() + 1];
        strcpy(cstr, cred_line.c_str());
        string cur_course = strtok(cstr, ",");
        if(course == cur_course) {
            string cur_credit = strtok(NULL, ",");
            string cur_professor = strtok(NULL, ",");
            string cur_days = strtok(NULL, ",");
            string cur_coursename = strtok(NULL, ",");

            if(category == "Credit") {
                result = "The Credits of " + course + " " + cur_credit + ".";
            }
            else if(category == "Professor") {
                result = "The Professor of " + course + " " + cur_professor + ".";
            }
            else if(category == "Days") {
                result = "The Days of " + course + " " + cur_days + ".";
            }
            else if(category == "CourseName") {
                result = "The Course Name of " + course + " " + cur_coursename + ".";
            } 
            break;
        }
    }
    return result;
}

int main() {
    
    create_serveree_udp_socket();
    while(true) {
        addr_len = sizeof(their_addr);
        serverEE_recv_result = recvfrom(serverEE_udp_socket, buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len);
        string result = inspect_course_info(buf);
        cout << "The Server EE received a request from the Main Server about the " << category << " of " << course << "." << endl;
        strncpy(result_buf, result.c_str(), MAXBUFSIZE);
        serverEE_send_result = sendto(serverEE_udp_socket, result_buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len);
        cout << "The Server EE finished sending the response to the Main Server." << endl;
    }
}