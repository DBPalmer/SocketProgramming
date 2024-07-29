//Author: Lucas (Deuce) Palmer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define HOSTNAME "localhost"
#define SERVERMTCP "45575"
#define MAXDATASIZE 10000

//global variables
int sockfd, numbytes;  
char buf[MAXDATASIZE];
struct addrinfo hints, *servinfo, *p;
int rv;
char s[INET6_ADDRSTRLEN];
struct sockaddr_in my_addr;
int addrlen;


void CreateAndConnect();
string encrypt(string);

int main(){
    CreateAndConnect();
    int match = 0;
    string us, pa, user, pass;
    while (match == 0){//until user enters valid credentials
        cout << "Please enter the username: ";
        getline(cin, us);
        user = encrypt(us);//encrypt username
        cout << "Please enter the password: ";
        getline(cin, pa);
        pass = encrypt(pa);//encrypt password
        string combo = user + ", " + pass;//combine to send both in one message
        char userAndpass[combo.size()];
        strcpy(userAndpass, combo.c_str());
        if (send(sockfd, userAndpass, strlen(userAndpass), 0) == -1) {//send credentials to main server
            perror("send");
        }
        cout << user << " sent an authentication request to the Main Server.\n";
        int size;
        if ((size = recv(sockfd, buf, 1000, 0)) == -1){//receive response from main server
            perror("rec");
        }
        buf[size] = '\0';
        if (strcmp(buf ,"success") == 0){
            cout << user << " received the result of authentication from Main Server using TCP over port " << (int)ntohs(my_addr.sin_port) << ".\nAuthentication is successful.\n";
            match = 1;
        } 
        if (strcmp(buf, "userfail") == 0){
            cout << user << " received the result of authentication from Main Server using TCP over port " << (int)ntohs(my_addr.sin_port) << ".\nAuthentication failed: Username not found.\n";
        }
        if (strcmp(buf, "fail") == 0){
            cout << user << " received the result of authentication from Main Server using TCP over port " << (int)ntohs(my_addr.sin_port) << ".\nAuthentication failed: Password does not match.\n";
        }
    }
    while (true){//loop for user to enter book codes
        char code[5], adcode[5];
        string str;
        cout << "Please enter book code to query: ";
        getline(cin, str);
        if (us == "admin"){
            str = str + '!';//so servers know that the request is from an admin, and can act accordingly
            strcpy(code, str.c_str());
            if (send(sockfd, code, strlen(code), 0) == -1) {//send admin book code to main server
                perror("send");
            }
            cout << "Request sent to the Main Server with Admin rights.\n";
        } else {
            strcpy(code, str.c_str());
            if (send(sockfd, code, strlen(code), 0) == -1) {//send book code to main server
                perror("send");
            }
            cout << user << " sent the request to the Main Server.\n";
        }
        int size;
        if ((size = recv(sockfd, buf, 1000, 0)) == -1){//response from main server after connecting to backend servers
            perror("rec");
        }
        cout << "Response received from the Main Server on TCP port: " << (int)ntohs(my_addr.sin_port) << ".\n";
        buf[size] = '\0';
        if (us == "admin"){//different printouts for admin
            if (strcmp(buf, "FAIL") == 0){//book not found
                cout << "Not able to find the book-code " << ((string)code).substr(0,4) << " in the system.\n";
                cout << "—- Start a new query —-\n";
            } else {//book found
                cout << "Total number of book " << ((string)code).substr(0,4) << " available = " << buf << "\n";
                cout << "—- Start a new query —-\n";
            }
        } else {
            if (strcmp(buf, "notavailable") == 0){//book found, but not availble
                cout << "The requested book " << code << " is NOT available in the library.\n";
                cout << "—- Start a new query —-\n";
            } else if (strcmp(buf, "FAIL") == 0){//book not found
                cout << "Not able to find the book-code " << code << " in the system.\n";
                cout << "—- Start a new query —-\n";
            } else {//book found
                cout << "The requested book " << code << " is available in the library.\n";
                cout << "—- Start a new query —-\n";
            }
        }
    }
    close(sockfd);
    return 0;
}

string encrypt(string plain){
    //case sensitive
    char encrypted[plain.size()+1];
    for (int i=0; i<plain.size(); i++){
        if (((int)plain[i] >= 65 && (int)plain[i]<= 85) || ((int)plain[i] >= 97 && (int)plain[i]<= 117)){
            encrypted[i] = plain[i]+5;
        } else if (((int)plain[i] >= 86 && (int)plain[i]<= 90) || ((int)plain[i] >= 118 && (int)plain[i]<= 122)){
            encrypted[i] = plain[i] - 21;
        } else if ((int)plain[i] >= 48 && (int)plain[i]<= 52){
            encrypted[i] = plain[i] + 5;
        } else if ((int)plain[i] >= 53 && (int)plain[i]<= 57){
            encrypted[i] = plain[i] - 5;
        } else {
            encrypted[i] = plain[i];
        }
    }
    encrypted[plain.size()] = '\0';
    return encrypted;
}

void CreateAndConnect(){
    //code from Beej
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(HOSTNAME, SERVERMTCP, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        //getsockname() code from project handout
        addrlen = sizeof(my_addr);
        //saves dynamically assigned port number in my_addr
        int getsock_check = getsockname(sockfd, (struct sockaddr*)&my_addr, (socklen_t *)&addrlen);
        if (getsock_check== -1) { 
            perror("getsockname"); 
            }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
    }
    cout << "Client is up and running.\n";
    freeaddrinfo(servinfo); 
}