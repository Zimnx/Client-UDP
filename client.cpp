/* 
 * Klient UDP
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>
#include <string>
#include "sockwrap.h"

#define MAXMSG 65535
using namespace std;
char buffer[MAXMSG+1];

map<int,string> requests;
int port;
string filename;
int bytes;

int main (int argc, char** argv)
{
    if(argc != 4) { printf("Usage: client-udp <port> <output> <bytes>\n"); exit(1);}

    port = atoi(argv[1]);
    filename = string(argv[2]);
    bytes = atoi(argv[3]);
    int pack = 600;


    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // Struktura opisująca IP i port serwera

    struct sockaddr_in server_address;
    bzero (&server_address, sizeof(server_address));
    server_address.sin_family   = AF_INET;
    server_address.sin_port     = htons(atoi(argv[1]));
    inet_pton(AF_INET, "156.17.4.30", &server_address.sin_addr); // aisd.ii.uni.wroc.pl

    // Wysyłanie jakiegos napisu do serwera

    char sending_buffer[MAXMSG];
    strcpy (sending_buffer, "Hello server!");
    Sendto(sockfd, sending_buffer, strlen(sending_buffer),
            0, &server_address, sizeof(server_address));

    // Otrzymywanie informacji do serwera (prawdopodobnie od serwera, 
    // nie sprawdzamy tego!)
    char receiving_buffer[MAXMSG];
    int n = Recvfrom(sockfd, receiving_buffer, MAXMSG, 0, NULL, NULL);  
    receiving_buffer[n] = 0;
    printf ("server reply: %s\n", receiving_buffer);
}

