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

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

#define HOSTNAME "localhost"
#define SERVERM "44575"
#define UDPPORT "43575"
#define MAXBUFLEN 100

struct book{
    char code[5];
    int inv;
};

//global variables
int sockfd;
int numbytes;
socklen_t addr_len;
struct sockaddr_storage their_addr;

void readFile(string, vector<book>&);
void CreateAndSend();

int main(){
    vector<book> lib;
    readFile("history.txt", lib);
    CreateAndSend();
    addr_len = sizeof their_addr;
    //process code
        char buf[MAXBUFLEN];
    while (true){
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {//received book code from main server
            perror("recvfrom");
        }
        buf[numbytes] = '\0';
        int admin = 0;
        if (buf[4] == '!'){//check if it is an admin request
            admin++;
            buf[4] = '\0';
            cout << "Server H received an inventory status request for code " << buf << ".\n";
        } else {
            cout << "Server H received " << buf << " code from the Main Server.\n";
        }
        int invent = -1;
        for (int i=0; i<lib.size(); i++){
            if (lib[i].code == (string)buf){//book code found
                invent = lib[i].inv;
                if (admin == 0 && lib[i].inv > 0){
                    lib[i].inv--;
                }
            }
        }
        if (invent == -1){//book code not found, tell main server
            if ((numbytes = sendto(sockfd, "FAIL", 4, 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
                perror("talker: sendto");
            }
        } else {//book code found, send data to main server
            if (admin != 0){//send inventory for admin request
                string combo = to_string(invent);
                char answer[combo.size()];
                strcpy(answer, combo.c_str());
                if ((numbytes = sendto(sockfd, answer, strlen(answer), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
                    perror("talker: sendto");
                }
                cout << "Server H finished sending the inventory status to the Main server using UDP on port " << UDPPORT << ".\n";
            } else {
                if (invent > 0){//book is available
                    if ((numbytes = sendto(sockfd, "available", 9, 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
                        perror("talker: sendto");
                    }
                } else {
                    if ((numbytes = sendto(sockfd, "notavailable", 12, 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
                        perror("talker: sendto");
                    }
                }
                cout << "Server H finished sending the availability status of code " << buf << " to the Main Server using UDP on port " << UDPPORT << ".\n";
            }
        }
    }
    return 0;
}

void readFile(string fileName, vector<book>& lib){
    FILE *file;
    char name[fileName.size()];
    strcpy(name, fileName.c_str());
    file = fopen(name, "r");
    char buf[100];
    while (fgets(buf, sizeof(buf), file) != NULL) {//reads till end of file
        char *ptr= strtok(buf, " ,");
        book mybook;
        for (int i=0; i<4;i++){
            mybook.code[i] = ptr[i];
        }
        mybook.code[4] = '\0';
        ptr = strtok(NULL, " ,");
        mybook.inv = atoi(ptr);
        lib.push_back(mybook);
    }
    fclose(file);
}

void CreateAndSend(){
    //code from Beej
    struct addrinfo hints, *servinfo, *p;
    struct addrinfo hints2, *serve;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo(HOSTNAME, SERVERM, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
    }
    //must assign ServerH's static port
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_INET; 
    hints2.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo(HOSTNAME, UDPPORT, &hints2, &serve)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }
    if (bind(sockfd, serve->ai_addr, serve->ai_addrlen)==-1){
        perror("listener: bind");
    }
    cout << "Server H is up and running using UDP on port " << UDPPORT <<  "." << endl;
    if ((numbytes = sendto(sockfd, "serverH", 7, 0,
             p->ai_addr, p->ai_addrlen)) == -1) {//send to main server so it can save ServerH's address
        perror("talker: sendto");
    }
    freeaddrinfo(servinfo);
}