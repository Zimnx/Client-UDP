// Maciej Zimnoch - 248104
// Sieci Komputerowe - Pracownia 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>
#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include "sockwrap.h"

#define MAXMSG 65535

using namespace std;

vector<int> requests;
int pack = 1000; // bytes
struct wrapper 
{ 
    char *tab;
    wrapper()
    {
        tab = new char[pack];
    }
    wrapper(char *t,int bytes)
    {
        tab = new char[bytes];
        memcpy(tab,t,bytes);
    }
};
int port;
string filename;
int bytes;
char address[] = "156.17.4.30"; // aisd.ii.uni.wroc.pl

int requestsAtOnce = 20;
map<int,wrapper> results;

void sendDatagram(int sockfd, sockaddr_in server_address, int start, int bytes);
string getStart(char* buffer);
void deleteRequest(int start);
int findChar(const char* buffer,int size, char c);

int main (int argc, char** argv)
{
    if(argc != 4) { printf("Usage: client-udp <port> <output> <bytes>\n"); exit(1);}

    port = atoi(argv[1]);
    filename = string(argv[2]);
    bytes = atoi(argv[3]);
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
        int got = 0;
        while(1)
        {
            sockaddr_in client_address;
            socklen_t len = sizeof(client_address);

            fd_set descriptors;
            
            FD_ZERO(&descriptors);
            FD_SET(sockfd,&descriptors);    
            timeval timeout; timeout.tv_sec = 1; timeout.tv_usec = 0;
            int ready = Select(sockfd+1, &descriptors, NULL, NULL, &timeout);
            if(!ready)
            {
                break;
            }            
            char buffer[MAXMSG+1];
            char* buffer_ptr = buffer;
            int n = Recvfrom (sockfd, buffer, MAXMSG, 0, &client_address, &len);
            char ip_address[20];
            inet_ntop (AF_INET, &client_address.sin_addr, ip_address, sizeof(ip_address));
            string ip = string(ip_address);

            if(strcmp(address,ip.c_str()) == 0)
            {
                
                int nl = findChar(buffer,n,'\n');
                char* info = new char[nl+2];
                strncpy(info,buffer,nl+2);
                int start = atoi(getStart(info).c_str());
                if(find(requests.begin(), requests.end(), start) == requests.end())
                    continue;
                int requestBytes = (bytes - start >= pack) ? pack : (bytes - start);
                
                for(int i = 0 ; i <= nl ; ++i)
                {
                    buffer_ptr++;
                }
                wrapper data;
                memcpy(data.tab,buffer_ptr,requestBytes);                
                
                results[start] = data;
                printf("done %d %% \n",(got*100)/totalRequests);
                got++;
                deleteRequest(start);
            }

        }
    }
    FILE* out;
    out = fopen(filename.c_str(),"wb");
    if(out == NULL) {printf("Cant open %s",filename.c_str()); exit(1); }
    for(map<int,wrapper>::iterator i = results.begin(); i != results.end(); ++i)
    {
        char* buffer = (*i).second.tab;
        int start = (*i).first;
        int requestBytes = (bytes - start >= pack) ? pack : (bytes - start);
        fwrite(buffer,1,requestBytes,out);
    }
    return 0;

}

void sendDatagram(int sockfd, sockaddr_in server_address, int start, int bytes)
{
    
    ostringstream ss;
    ss << "GET " << start << " " << bytes << "\n";
    string request = ss.str();
   // printf("sending: %s",request.c_str());
    Sendto(sockfd, request.c_str(), strlen(request.c_str()),
            0, &server_address, sizeof(server_address));
}

string getStart(char* buffer)
{
    string info(buffer);
    int firstSpace = info.find_first_of(' ');
    int lastSpace = info.find_last_of(' ');
    return info.substr(firstSpace + 1,lastSpace - firstSpace - 1);
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
int findChar(const char* buffer,int size, char c)
{
    for(int i = 0 ; i < size ; ++i)
    {
        if(buffer[i] == c)
            return i;
    }
    return -1;
}