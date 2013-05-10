// Maciej Zimnoch - 248104
// Sieci Komputerowe - Pracownia 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <algorithm>
#include <vector>
#include "sockwrap.h"

#define MAXMSG 65535

using namespace std;

vector<int> requests;
int pack = 1000; // bytes
int requestsAtOnce = 20;

int port;
char* filename;
int bytes;
char address[] = "156.17.4.30"; // aisd.ii.uni.wroc.pl

void sendDatagram(int sockfd, sockaddr_in server_address, int start, int bytes);
void deleteRequest(int start);

int main (int argc, char** argv)
{
    if(argc != 4) { printf("Usage: client-udp <port> <output> <bytes>\n"); exit(1);}

    port = atoi(argv[1]);
    filename = argv[2];
    bytes = atoi(argv[3]);
    char* outbuff = new char[bytes];
    FILE* out;
    out = fopen(filename,"wb");
    if(out == NULL) {printf("Cant open %s",filename); exit(1); }

    int totalRequests = bytes/pack + 1;
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_address;
    bzero (&server_address, sizeof(server_address));
    server_address.sin_family   = AF_INET;
    server_address.sin_port     = htons(port);
    inet_pton(AF_INET, address, &server_address.sin_addr); 

    for(int i = 0; i < totalRequests ; ++ i)
    {
        int start = i*pack;
        requests.push_back(start);
    }

    int got = 0;

    while(requests.size() > 0)
    {
        for(int i = 0 ; i < requestsAtOnce ; ++i)
        {
            if(i < requests.size())
            {
                int start = requests[i];
                int requestBytes = (bytes - start >= pack) ? pack : (bytes - start);
                if(requestBytes == 0)
                    deleteRequest(start);
                for(int j = 0 ; j < 6 ; ++j)
                    sendDatagram(sockfd, server_address, start, requestBytes);
            }
            
        }
        
        while(1)
        {
            sockaddr_in client_address;
            socklen_t len = sizeof(client_address);

            fd_set descriptors;
            
            FD_ZERO(&descriptors);
            FD_SET(sockfd,&descriptors);    
            timeval timeout; timeout.tv_sec = 0; timeout.tv_usec = 1000;
            int ready = Select(sockfd+1, &descriptors, NULL, NULL, &timeout);

            if(!ready)
            {
                break;
            }            
            char* buffer = new char[MAXMSG+1];

            Recvfrom (sockfd, buffer, MAXMSG, 0, &client_address, &len);
            char ip_address[20];
            inet_ntop (AF_INET, &client_address.sin_addr, ip_address, sizeof(ip_address));
            

            if(strcmp(address,ip_address) == 0 && ntohs(client_address.sin_port) == port)
            {
               
                int start = 0;
                int len = 0;
                sscanf(buffer, "DATA %d %d\n", &start, &len);

                if(find(requests.begin(), requests.end(), start) == requests.end())
                    continue;
                while (*buffer != '\n') { ++buffer; };
                ++buffer;

                memcpy(outbuff+ start, buffer, len);
                   
                got++;
                printf("done %.2lf %% \n ",(got*100)/(float)totalRequests);
                
                deleteRequest(start);
            }

        }
    }
    fwrite(outbuff,1,bytes,out);

    return 0;

}

void sendDatagram(int sockfd, sockaddr_in server_address, int start, int bytes)
{
    
    char request[256];
    sprintf(request, "GET %d %d\n", start, bytes);
    Sendto(sockfd, request, strlen(request), 0, &server_address, sizeof(server_address));
}


void deleteRequest(int start)
{
    for(vector<int>::iterator i = requests.begin(); i != requests.end(); ++i)
    {
        if((*i) == start)
        {
            requests.erase(i);
            break;
        }
    }
}
