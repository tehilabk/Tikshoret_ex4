#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#define IPAddress "127.0.0.1"
#define PACKETSIZE	64
struct packet
{
	struct icmphdr hdr;
	char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;
struct protoent *proto=NULL;

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
{	const int val=255;
	int i, sd, cnt=1;
	struct packet pckt;
	struct sockaddr_in r_addr;

	sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if ( sd < 0 )
	{
		perror("socket");
		return;
	}

	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");
		int bool = 1;
	while(bool)
	{
		int len=sizeof(r_addr);

		bzero(&pckt, sizeof(pckt));
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = pid;
		for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
			pckt.msg[i] = i+'0';
		pckt.msg[i] = 0;
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
		if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");

		printf("Ping\n");
		if ( recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) > 0 ){
			printf("Pong\n");
			bool = 0;
		}

	}
}


/*--------------------------------------------------------------------*/
/*--- main - look up host and start ping processes.                ---*/
/*--------------------------------------------------------------------*/
int main()
{	struct hostent *hname;
	struct sockaddr_in addr;

		pid = getpid();
		proto = getprotobyname("ICMP");
		hname = gethostbyname(IPAddress);
		bzero(&addr, sizeof(addr));
		addr.sin_family = hname->h_addrtype;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = *(long*)hname->h_addr;
		ping(&addr);
		wait(0);
	
	return 0;
}