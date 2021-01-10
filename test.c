#include<stdio.h>	//For standard things
#include<stdlib.h>	//malloc
#include<string.h>	//memset
#include<netinet/ip_icmp.h>	//Provides declarations for icmp header
#include<netinet/udp.h>	//Provides declarations for udp header
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header
#include<sys/socket.h>
#include<arpa/inet.h>

#define bufferSize 65536

int sock;
int i,j;
struct sockaddr_in source,dest;

void ProcessPacket(char* buffer, int size)
{
	//Get the IP Header part of this packet
	struct iphdr *iph = (struct iphdr*)buffer;
	if(iph->protocol ==1) //Check the Protocol and do accordingly...
	{
	short iphdrlen;
	struct iphdr *iph = (struct iphdr *)buffer;
	iphdrlen = iph->ihl*4;
	struct icmphdr *icmph = (struct icmphdr *)(buffer + iphdrlen);	
	print_ip_header(buffer);
	printf("Type : %d\n",(int)(icmph->type));		
	printf("Code : %d\n",(int)(icmph->code));
	}
}
 
void print_ip_header(char* Buffer)
{
	short iphdrlen;	
	struct iphdr *iph = (struct iphdr *)Buffer;
	iphdrlen =iph->ihl*4;	
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iph->saddr;
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = iph->daddr;
	printf("Source IP        : %s\n",inet_ntoa(source.sin_addr));
	printf("Destination IP   : %s\n",inet_ntoa(dest.sin_addr));
}

int main()
{
	int adderLength , stream;
	struct sockaddr address;
	struct in_addr in;
	char buffer[bufferSize];
	sock = socket(AF_INET , SOCK_RAW , IPPROTO_ICMP);//Create a raw socket that shall sniff
	if(sock < 0)
	{
		printf("Socket Error\n");
		return 1;
	}
	printf("Starting the sniffing proccec...\nPress CTRL+Z to stop\n");
	while(1)
	{
		adderLength = sizeof(address);
		//Receive a packet
		stream = recvfrom(sock , buffer , bufferSize , 0 , &address , &adderLength);
		if(stream <0 )
		{
			printf("failed to get packets\n");
			return 1;
		}
		//Now process the packet
		ProcessPacket(buffer , stream);
		printf("Continue sniffing or press CTRL+Z to stop\n");
		memset(&buffer, 0, sizeof(buffer));
	}
	close(sock);
	printf("Finished");
	return 0;
}
