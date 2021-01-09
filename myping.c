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

int packet_ID = -1; 
struct protoent *proto=NULL;


/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short checksum(void *b, int len)
{	
	unsigned short *buf = b;
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
/*--- display - present echo info                                  ---*/
/*--------------------------------------------------------------------*/
void display(void *buf, int bytes)
{
	int i;
	struct iphdr *ip = buf;
	struct icmphdr *icmp = buf+ip->ihl*4;


	printf("\n");
	printf("IPV%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d src=%s ",
		ip->version, ip->ihl*4, ntohs(ip->tot_len), ip->protocol,
		ip->ttl, inet_ntoa(ip->saddr));
	printf("dst=%s\n", inet_ntoa(ip->daddr));
	if (icmp->un.echo.id == packet_ID )
	{
		printf("ICMP: type[%d/%d] checksum[%d] id[%d] seq[%d]\n",
			icmp->type, icmp->code, ntohs(icmp->checksum),
			icmp->un.echo.id, icmp->un.echo.sequence);
	}
}

void listener(void)
{	
	struct sockaddr_in address;
	unsigned char buffer[1024];
	
	int sock = socket(AF_INET, SOCK_RAW, proto->p_proto);
	if ( sock < 0 )
	{
		printf("socket failed");
		return;
	}
	int bool =1;
	while(bool)
	{	
		int addressLen = sizeof(address);
		bzero(buffer, sizeof(buffer));
		int stream = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &addressLen);
		if ( stream > 0 ){
			display(buffer, stream);
			printf("**Pong!**\n\n");
			bool=0;
		}	
		else{
			printf("recieve ping failed");
		}
	}
	return;
}

/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*--------------------------------------------------------------------*/
void ping(struct sockaddr_in *addr)
{
	const int val=255;
	int i;
	struct packet packeta;
	struct sockaddr_in r_addr;

	int sock = socket (AF_INET, SOCK_RAW, proto->p_proto);;
	if ( sock < 0 )
	{
		perror("socket");
		return;
	}
	if ( setsockopt(sock, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
		perror("Set TTL option");
	if ( fcntl(sock, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");


		int addressLen=sizeof(r_addr);
		int recive = recvfrom(sock, &packeta, sizeof(packeta), 0, (struct sockaddr*)&r_addr, &addressLen); 	
		
	
		bzero(&packeta, sizeof(packeta));
		packeta.ICMP_header.type = ICMP_ECHO;
		packeta.ICMP_header.un.echo.id = packet_ID;

		packeta.ICMP_header.checksum = 0;
		packeta.ICMP_header.checksum = checksum(&packeta, sizeof(packeta));		
	// int bool = 1;	
	// while(bool)
	// {	
		//TODO measure time
		int sender = sendto(sock, &packeta, sizeof(packeta), 0, (struct sockaddr*)addr, sizeof(*addr));
		if (sender <= 0 ){
			printf("Ping failed");
		}	
		printf("**Ping!**\n\n");
		// bool = 0;	

		// packeta.ICMP_header.checksum = 0;
		
	// }
}

/*--------------------------------------------------------------------*/
/*--- main - look up host and start ping processes.                ---*/
/*--------------------------------------------------------------------*/
int main()
{	struct hostent *hname;
	struct sockaddr_in addr;

		packet_ID = getpid();
		proto = getprotobyname("ICMP");
		hname = gethostbyname(DESTINATION_IP);
		bzero(&addr, sizeof(addr));
		addr.sin_family = hname->h_addrtype;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = *(long*)hname->h_addr;

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