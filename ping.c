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

#define IPAddress "127.0.0.1"
#define PACKETSIZE	64
struct packet
{
	struct icmphdr hdr;
	char msg[1];														//keep empty, will send garbage
};
/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short checksum(void *b, int len)								//we dont know what happend here, we use it and it works
{	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;
	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}
/*--------------------------------------------------------------------*/
/*--- ping -message send.                           ---*/
/*--------------------------------------------------------------------*/
void ping(struct sockaddr_in *addr)
{	
	struct timeval stop, start;											//for measuring time
	struct packet paketa;												//packet to send
	struct sockaddr_in address;											//where we send it
	int sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);					//raw socket 
	if ( sock < 0 )
	{
		perror("socket");
		return;
	}
	if ( fcntl(sock, F_SETFL, O_NONBLOCK) != 0 )						//we cant remove it, it wont work without it
		perror("Request nonblocking I/O");
		int bool = 1;													//boolean for the while loop
	while(bool)
	{
		bzero(&paketa, sizeof(paketa));									//"cleans" the packet
		paketa.hdr.type = ICMP_ECHO;									//puts the header 
		paketa.hdr.un.echo.id = 27;										//some lucky number for the ID
		paketa.hdr.checksum = checksum(&paketa, sizeof(paketa));		//calculates the checksum and applies it
		gettimeofday(&start, NULL);										//starts the timer
		if ( sendto(sock, &paketa, sizeof(paketa), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("error occoured while sending the packet");			//sends the packet
		else printf("Ping\n");										
		int addressLength=sizeof(address);								//checks if recieved the "ping"
		if ( recvfrom(sock, &paketa, sizeof(paketa), 0, (struct sockaddr*)&address, &addressLength) > 0 ){
			gettimeofday(&stop, NULL);									//ends the timer
			printf("Pong\n");
			bool = 0;													//stops the loop
			printf("RTT time in microseconds is: %ld\n", (stop.tv_sec - start.tv_sec)* 1000000 +(stop.tv_usec - start.tv_usec));
	    	printf("RTT time in milliseconds is: %ld\n", ((stop.tv_sec - start.tv_sec)* 1000000 +(stop.tv_usec - start.tv_usec))/1000);
		}	//prints the time in microseconds and milliseconds, at our machine it took about 50-150 microseconds and it wont show milliseconds only after 1000 microseconds
	}
}
/*--------------------------------------------------------------------*/
/*--- main - look up host and start ping processes.                ---*/
/*--------------------------------------------------------------------*/
int main()
{	struct hostent *hname;
	struct sockaddr_in addr;
		hname = gethostbyname(IPAddress);
		bzero(&addr, sizeof(addr));
		addr.sin_family = hname->h_addrtype;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = *(long*)hname->h_addr;
		ping(&addr);									//the ping and pong function
		wait(0);
	return 0;
}