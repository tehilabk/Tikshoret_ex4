#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DESTINATION_IP "127.0.0.1"
#define PACKETSIZE	64
struct packet
{
    struct icmphdr ICMP_header;
    char message[PACKETSIZE-sizeof(struct icmphdr)];
};

int packet_ID = 27;

//--------------------------------------------------------------------/
//--- checksum - standard 1s complement checksum                   ---/
//--------------------------------------------------------------------/
unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += (unsigned char)*buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void listener(void)
{
    struct sockaddr_in address;
    unsigned char buffer[27];
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if ( sock < 0 )
    {
        perror("socket failed");
        return;
    }
    int bool =1;
    while(bool)
    {
        int headerLen = sizeof(address);
        bzero(buffer, sizeof(buffer));
        int stream = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &headerLen);
        if ( stream > 0 ){
            printf("*Pong!*\n\n");
            bool=0;
        }
        else{
            perror("recieve ping failed");
        }
    }
    return;
}

//--------------------------------------------------------------------/
//--- ping - Create message and send it.                           ---/
//--------------------------------------------------------------------/
void ping(struct sockaddr_in *addr)
{
    const int val=255;
    int i;
    struct packet packeta;
    struct sockaddr_in address;

    int sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);;
    if ( sock < 0 )
    {
        perror("socket");
        return;
    }

    int bool = 1;
    while(bool)
    {
        //TODO measure time
        int sender = sendto(sock, &packeta, sizeof(packeta), 0, (struct sockaddr*)addr, sizeof(*addr));
        if (sender <= 0 ){
            perror("Ping failed");
        }
        
        printf("*Ping!*\n\n");
        bool = 0;

        int addressLen=sizeof(address);
        int recive = recvfrom(sock, &packeta, sizeof(packeta), 0, (struct sockaddr*)&address, &addressLen);
        packeta.ICMP_header.checksum = checksum(&packeta, sizeof(packeta));

        bzero(&packeta, sizeof(packeta));

        packeta.ICMP_header.type = ICMP_ECHO;
        packeta.ICMP_header.un.echo.id = packet_ID;

    }
}

//--------------------------------------------------------------------/
//--- main - look up host and start ping processes.                ---/
//--------------------------------------------------------------------/
int main()
{	struct hostent *hname;
    struct sockaddr_in addr;

    //packet_ID = getpid();
    hname = gethostbyname(DESTINATION_IP);
    bzero(&addr, sizeof(addr));
    addr.sin_family = hname->h_addrtype;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = (long)hname->h_addr;

    struct timeval stop_m, start_m, stop_s, start_s;

    if ( fork() == 0 ){
        listener();
    }
    else{

        gettimeofday(&start_m, NULL);
        gettimeofday(&start_s, NULL);
        ping(&addr);
        gettimeofday(&stop_m, NULL);
        gettimeofday(&stop_s, NULL);


    }
    wait(0);

    printf("RTT time in microseconds is: %ld\n", (stop_m.tv_sec - start_m.tv_sec)* 1000000 +(stop_m.tv_usec - start_m.tv_usec));
    printf("RTT time in milliseconds is: %ld\n", ((stop_m.tv_sec - start_m.tv_sec)* 1000000 +(stop_m.tv_usec - start_m.tv_usec))/1000);


    return 0;
}