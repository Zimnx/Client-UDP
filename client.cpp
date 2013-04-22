/* 
 * Klient UDP
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>
#include <queue>
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

int requestsAtOnce = 100;
map<int,wrapper> results;

void sendDatagram(int sockfd, sockaddr_in server_address, int start, int bytes);
string getStart(string info);
void deleteRequest(int start);
char* getData(char* buffer, int size);
void print_bytes (int count,char* buffer_ptr)
{
    for (int i=0; i<count; i++) {
        printf ("%.2x ", *buffer_ptr);
        buffer_ptr++;
    }
    printf("\n");
}
int main (int argc, char** argv)
{
    if(argc != 4) { printf("Usage: client-udp <port> <output> <bytes>\n"); exit(1);}

    port = atoi(argv[1]);
    filename = string(argv[2]);
    bytes = atoi(argv[3]);

    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_address;
    bzero (&server_address, sizeof(server_address));
    server_address.sin_family   = AF_INET;
    server_address.sin_port     = htons(port);
    inet_pton(AF_INET, address, &server_address.sin_addr); 
    
    for(int i = 0; i < (bytes/pack) + 1 ; ++ i)
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
            timeval timeout; timeout.tv_sec = 0; timeout.tv_usec = 100000;
            int ready = Select(sockfd+1, &descriptors, NULL, NULL, &timeout);
            if(ready)
            {
                char buffer[MAXMSG+1];
                char* buffer_ptr = buffer;
                Recvfrom (sockfd, buffer, MAXMSG, 0, &client_address, &len);
                
                string buf = string(buffer);
                int nl = buf.find_first_of('\n');
                string info = buf.substr(0,nl+1);
                int start = atoi(getStart(info).c_str());
                int requestBytes = (bytes - start >= pack) ? pack : (bytes - start);
                char *data = new char[requestBytes];
                for(int i = 0 ; i <= nl ; ++i)
                {
                    buffer_ptr++;
                }
                //copy(buffer_ptr,buffer_ptr + pack, data);
                //strcpy(data,buffer_ptr);
                //strncat(data,buffer[nl+1],pack);
                //strncat(data,buffer[nl+1],pack);
                memcpy(data,buffer_ptr,requestBytes);

                char ip_address[20];
                inet_ntop (AF_INET, &client_address.sin_addr, ip_address, sizeof(ip_address));
                string ip = string(ip_address);
                if(strcmp(address,ip.c_str()) == 0)
                {
                    
                    results[start] = wrapper(data,requestBytes);
                    deleteRequest(start);
                   // printf("got:%d %d %d %d %d | %s\n",start,n,nl,sizeof(data),sizeof(buffer),info.c_str());
                    //print_bytes(pack, data);
                }

            }
            else
            {
                break;
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
        int requestBytes = (bytes - start >= pack) ? pack : (bytes - start);//(*i).first;
        //printf("sizeof(buffer)=%d bytes=%d",sizeof(buffer),requestBytes);
        fwrite(buffer,1,requestBytes,out);
    }
    return 0;

}

void sendDatagram(int sockfd, sockaddr_in server_address, int start, int bytes)
{
    
    ostringstream ss;
    ss << "GET " << start << " " << bytes << "\n";
    string request = ss.str();
    printf("sending: %s",request.c_str());
    Sendto(sockfd, request.c_str(), strlen(request.c_str()),
            0, &server_address, sizeof(server_address));
}

string getStart(string info)
{
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
char* getData(char* buffer, int size)
{
    char* result = new char[size];
    int i = 0;
    while(buffer[i++] != '\n');
    for(int j = 0 ; i < size; ++i,j++)
        result[j] = buffer[i];
    return result;
}