//Author: Lucas (Deuce) Palmer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define HOSTNAME "localhost"
#define UDPPORT "44575"
#define TCPPORT "45575"
#define BACKLOG 10000
#define MAXBUFLEN 100

struct login{
    string username;
    string password;
};

//global variables
char buf[MAXBUFLEN];
//TCP
int sockfd, new_fd;
struct sockaddr_storage their_addr; 
socklen_t sin_size;
char s[INET6_ADDRSTRLEN];
//UDP global variables
int udpsock;
socklen_t addr_lenserverS, addr_lenserverL, addr_lenserverH;
struct sockaddr_storage theirudpserverS, theirudpserverL, theirudpserverH;
int numbytes;

void readFile(string, vector<login>&);
void CreateAndBind();
void Listen();
void Accept();
void UDPCreateAndBind();

int main(void) {
    cout << "Main Server is up and running.\n";
    vector<login> members;
    readFile("member.txt", members);
    UDPCreateAndBind();
    CreateAndBind();
    Listen();
    Accept();
    if (!fork()) {
        int match = 0;
        while (match == 0){//until valid login is received
            close(sockfd);
            int size;
            if ((size = recv(new_fd, buf, 1000, 0)) == -1){//login from client
                perror("rec");
            }
            buf[size] = '\0';
            int i=0, z=0, y=0;
            char user[size], pass[size];
            //parse username and password
            while (buf[i] != ','){
                user[z] = buf[i];
                z++; i++;
            }
            user[i] = '\0';
            i+=2;
            while (i!=size){
                pass[y] = buf[i];
                y++; i++;
            }
            pass[y] = '\0';
            int userchk = 0;
            cout << "Main Server received the username and password from the client using TCP over port " << TCPPORT << ".\n";
            for (int i=0; i<members.size(); i++){
                if (user == members[i].username){//valid usernamee
                    userchk++;
                    if (pass == members[i].password){//valid password
                        match++;
                    } else {
                    }
                }
            }
            if (userchk == 0){
                cout << user << " is not registered. Send a reply to the client.\n";
                if (send(new_fd, "userfail", 8, 0) == -1){
                    perror("send");
                }
            } else if (match == 0){
                cout << "Password " << pass << " does not match the username. Send a reply to the client.\n";
                if (send(new_fd, "fail", 4, 0) == -1){
                    perror("send");
                }
            } else {
                cout << "Password " << pass << " matches the username. Send a reply to the client.\n";
                if (send(new_fd, "success", 7, 0) == -1){
                    perror("send");
                }
            }
        }
        while (true){//loop to recieve book codes from client
            int size;
            int notfound = 0;
            if ((size = recv(new_fd, buf, 1000, 0)) == -1){//book code from client
                perror("rec");
            }
            buf[size] = '\0';
            cout << "Main Server received the book request from client using TCP over port " << TCPPORT << ".\n";
            char res[MAXBUFLEN];
            //send book code to corresponding backend server
            if (buf[0] == 'S'){
                cout << "Found " << ((string)buf).substr(0,4) << " located at Server S. Send to Server S.\n";
                if ((numbytes = sendto(udpsock, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&theirudpserverS, addr_lenserverS)) == -1) {
                    perror("sendto");
                }
                if ((numbytes = recvfrom(udpsock, res, MAXBUFLEN-1 , 0,
                (struct sockaddr *)&theirudpserverS, &addr_lenserverS)) == -1) {
                    perror("recvfrom");
                }
                res[numbytes] = '\0';
                if (strcmp(res, "FAIL") != 0){//only print if book is found
                    if (buf[4] == '!'){//different printout for admin
                           cout << "Main Server received from server S the book status result using UDP over port " << UDPPORT << ":\nNumber of books " << ((string)buf).substr(0,4) << " available is: " << res << ".\n";
                    } else {
                        if (strcmp(res, "notavailable") == 0){
                            cout << "Main Server received from server S the book status result using UDP over port " << UDPPORT << ":\nBook code " << ((string)buf).substr(0,4) << " is NOT available.\n";
                        } else {
                            cout << "Main Server received from server S the book status result using UDP over port " << UDPPORT << ":\nBook code " << ((string)buf).substr(0,4) << " is available.\n";
                        }
                    }
                }
            } else if (buf[0] == 'L'){
                cout << "Found " << ((string)buf).substr(0,4) << " located at Server L. Send to Server L.\n";
                if ((numbytes = sendto(udpsock, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&theirudpserverL, addr_lenserverL)) == -1) {
                    perror("sendto");
                }
                if ((numbytes = recvfrom(udpsock, res, MAXBUFLEN-1 , 0,
                (struct sockaddr *)&theirudpserverL, &addr_lenserverL)) == -1) {
                    perror("recvfrom");
                }
                res[numbytes] = '\0';
                if (strcmp(res, "FAIL") != 0){//only print if book is found
                    if (buf[4] == '!'){//different printout for admin
                        cout << "Main Server received from server L the book status result using UDP over port " << UDPPORT << ":\nNumber of books " << ((string)buf).substr(0,4) << " available is: " << res << ".\n";
                    } else {
                        if (strcmp(res, "notavailable") == 0){
                            cout << "Main Server received from server L the book status result using UDP over port " << UDPPORT << ":\nBook code " << ((string)buf).substr(0,4) << " is NOT available.\n";
                        } else {
                            cout << "Main Server received from server L the book status result using UDP over port " << UDPPORT << ":\nBook code " << ((string)buf).substr(0,4) << " is available.\n";
                        }
                    }
                }
            } else if (buf[0] == 'H'){
                cout << "Found " << ((string)buf).substr(0,4) << " located at Server H. Send to Server H.\n";
                if ((numbytes = sendto(udpsock, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&theirudpserverH, addr_lenserverH)) == -1) {
                    perror("sendto");
                }
                if ((numbytes = recvfrom(udpsock, res, MAXBUFLEN-1 , 0,
                (struct sockaddr *)&theirudpserverH, &addr_lenserverH)) == -1) {
                    perror("recvfrom");
                }
                res[numbytes] = '\0';
                if (strcmp(res, "FAIL") != 0){//only print if book is found
                    if (buf[4] == '!'){//different printout for admin
                        cout << "Main Server received from server H the book status result using UDP over port " << UDPPORT << ":\nNumber of books " << ((string)buf).substr(0,4) << " available is: " << res << ".\n";
                    } else {
                        if (strcmp(res, "notavailable") == 0){
                            cout << "Main Server received from server H the book status result using UDP over port " << UDPPORT << ":\nBook code " << ((string)buf).substr(0,4) << " NOT available.\n";
                        } else {
                            cout << "Main Server received from server H the book status result using UDP over port " << UDPPORT << ":\nBook code " << ((string)buf).substr(0,4) << " available.\n";
                        }
                    }
                }
            } else {//book code does not start with 'S', 'L', or 'H'
                cout << "Did not find " << ((string)buf).substr(0,4) << " in the book code list.\n";
                notfound++;
            }
            if (notfound == 1){
                if (send(new_fd, "FAIL", 4, 0) == -1){
                        perror("send");
                }
            }else if (strcmp(res, "FAIL") == 0){
                if (send(new_fd, "FAIL", 4, 0) == -1){
                        perror("send");
                }
                //while the book code started with 'S', 'L', or 'H' the specific code was not found
                cout << "While the corresponding backend server was found. Book code " << ((string)buf).substr(0,4) << " was not found.\n";
            } else {//book was found
                if (send(new_fd, res, MAXBUFLEN-1, 0) == -1){
                    perror("send");
                }
            }
            cout << "Main Server sent the book status to the client.\n";
        }
        close(udpsock);
    }
    close(new_fd);
    return 0;
}

void readFile(string fileName, vector<login>& members){
    ifstream myfile; 
    myfile.open(fileName);
    string str;
    while (!myfile.eof()){//reads till end of file
        getline(myfile, str, ',');
        string usernme = str;
        getline(myfile, str, ' ');//skip the space
        getline(myfile, str, '\r');
        login mylogin;
        mylogin.username = usernme;
        mylogin.password = str;
        members.push_back(mylogin);
        getline(myfile, str, '\n');//get to next line
    }
    myfile.close();
    cout << "Main Server loaded the member list.\n";
}
void CreateAndBind(){
    //code from Beej
    int yes=1;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(HOSTNAME, TCPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
    }
}

void Listen(){
    //code from Beej
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
    }
}
void Accept(){
    //code from Beej
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
    }
}

void UDPCreateAndBind(){
    //code from Beej
    int yes=1;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char buf[MAXBUFLEN];
    char ss[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo(HOSTNAME, UDPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((udpsock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
        }
        if (bind(udpsock, p->ai_addr, p->ai_addrlen) == -1) {
            close(udpsock);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
    }
    freeaddrinfo(servinfo);
    //ServerS
    addr_lenserverS = sizeof theirudpserverS;
    if ((numbytes = recvfrom(udpsock, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&theirudpserverS, &addr_lenserverS)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    //ServerL
    addr_lenserverL = sizeof theirudpserverL;
    if ((numbytes = recvfrom(udpsock, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&theirudpserverL, &addr_lenserverL)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    //ServerH
    addr_lenserverH = sizeof theirudpserverH;
    if ((numbytes = recvfrom(udpsock, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&theirudpserverH, &addr_lenserverH)) == -1) {
        perror("recvfrom");
        exit(1);
    }
}