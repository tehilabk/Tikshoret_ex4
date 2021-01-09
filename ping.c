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
	char msg[1];
};

/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short checksum(void *b, int len)
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
/*--- ping - Create message and send it.                           ---*/
/*--------------------------------------------------------------------*/
void ping(struct sockaddr_in *addr)
{	
	struct timeval stop_m, start_m;

	struct packet paketa;
	struct sockaddr_in address;

	int sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if ( sock < 0 )
	{
		perror("socket");
		return;
	}

	if ( fcntl(sock, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");
		int bool = 1;
	while(bool)
	{
		
		bzero(&paketa, sizeof(paketa));
		paketa.hdr.type = ICMP_ECHO;
		paketa.hdr.un.echo.id = 27;																	//some lucky number for the ID
		paketa.hdr.checksum = checksum(&paketa, sizeof(paketa));
		gettimeofday(&start_m, NULL);
		if ( sendto(sock, &paketa, sizeof(paketa), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");
		printf("Ping\n");
		int addressLength=sizeof(address);
		if ( recvfrom(sock, &paketa, sizeof(paketa), 0, (struct sockaddr*)&address, &addressLength) > 0 ){
			gettimeofday(&stop_m, NULL);
			printf("Pong\n");
			bool = 0;
			printf("RTT time in microseconds is: %ld\n", (stop_m.tv_sec - start_m.tv_sec)* 1000000 +(stop_m.tv_usec - start_m.tv_usec));
	    	printf("RTT time in milliseconds is: %ld\n", ((stop_m.tv_sec - start_m.tv_sec)* 1000000 +(stop_m.tv_usec - start_m.tv_usec))/1000);
		}
		

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
		ping(&addr);
		
		wait(0);
	
	return 0;
}