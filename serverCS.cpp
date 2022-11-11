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
#define CS_UDP_PORT "22653"
#define FAIL -1
#define CSFILE "cs.txt"


int servercs_udp_socket;
int addr_info_result, udp_bind_result, servercs_recv_result, servercs_send_result;
struct addrinfo servercs_addr_cond, *servercs_addr_result;
struct sockaddr_storage their_addr;
socklen_t addr_len;
string course;
string category;
char buf[MAXBUFSIZE];
char result_buf[MAXBUFSIZE];

const string GET_ADDR_ERROR = "[ERROR]: serverCS fails to get address info.";
const string SERVERCS_SOCKET_ERROR = "[ERRROR]: serverCS fails to create socket for client.";
const string SERVERCS_BIND_ERROR = "[ERRROR]: serverCS fails to bind.";

void error_check(int status, string msg) {
    if(status == FAIL) {
        cout << msg << endl;
        exit(1);
    }
}

void create_servercs_udp_socket() {
    memset(&servercs_addr_cond, 0, sizeof servercs_addr_cond);
    servercs_addr_cond.ai_family = AF_UNSPEC;
    servercs_addr_cond.ai_socktype = SOCK_DGRAM;
    servercs_addr_cond.ai_flags = AI_PASSIVE;
    addr_info_result = getaddrinfo(NULL, CS_UDP_PORT, &servercs_addr_cond, &servercs_addr_result);
    error_check(addr_info_result, GET_ADDR_ERROR);

    servercs_udp_socket = socket(servercs_addr_result -> ai_family, servercs_addr_result -> ai_socktype, servercs_addr_result -> ai_protocol);
    error_check(servercs_udp_socket, SERVERCS_SOCKET_ERROR);

    udp_bind_result = bind(servercs_udp_socket, servercs_addr_result->ai_addr, servercs_addr_result->ai_addrlen);
    error_check(udp_bind_result, SERVERCS_BIND_ERROR);

    freeaddrinfo(servercs_addr_result);
    cout << "The Server CS is up and running using UDP on port " << CS_UDP_PORT << "." << endl;

}

string inspect_course_info(char* buf) {
    string buf_string;
    buf_string.append(buf);
    int p = buf_string.find(" ");
    course = buf_string.substr(0, p);
    category = buf_string.substr(p + 1, buf_string.size() - 1);
    cout << "The Server CS received a request from the Main Server about the " << category << " of " << course << "." << endl;
    string cred_line;
    ifstream MyReadFile(CSFILE);
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
                result = "The Credits of " + course + " is " + cur_credit + ".";
                cout << "The course information has been found: The " << category << " of " << course << " is " << cur_credit << "." << endl;

            }
            else if(category == "Professor") {
                result = "The Professor of " + course + " is " + cur_professor + ".";
                cout << "The course information has been found: The " << category << " of " << course << " is " << cur_professor << "." << endl;
            }
            else if(category == "Days") {
                result = "The Days of " + course + " is " + cur_days + ".";
                cout << "The course information has been found: The " << category << " of " << course << " is " << cur_days << "." << endl;
            }
            else if(category == "CourseName") {
                result = "The Course Name of " + course + " is " + cur_coursename + ".";
                cout << "The course information has been found: The " << category << " of " << course << " is " << cur_coursename << "." << endl;
            } 
            break;
        }
    }
    return result;
}

int main() {
    
    create_servercs_udp_socket();
    while(true) {
        addr_len = sizeof(their_addr);
        servercs_recv_result = recvfrom(servercs_udp_socket, buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len);
        string result = inspect_course_info(buf);
        strncpy(result_buf, result.c_str(), MAXBUFSIZE);
        servercs_send_result = sendto(servercs_udp_socket, result_buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len);
        cout << "The Server CS finished sending the response to the Main Server." << endl;
    }
}