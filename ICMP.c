#include<stdio.h>				//For standard things
#include<string.h>				//memset
#include<netinet/ip_icmp.h>		//Provides declarations for icmp header
#include<netinet/ip.h>			//Provides declarations for ip header
#include<sys/socket.h>
#include<arpa/inet.h>

#define bufferSize 65536

void ICMP_Detector(char* buffer, int size);					//Detects if the packet is ICMP
void IP_Printer(char* Buffer);								//Decapsulate the IP header and prints source ip and dest ip

int sock,i,j;
struct sockaddr_in IP_Source,IP_Dest;						//for later use in IP_Printer

void ICMP_Detector(char* buffer, int size)					//Detects if the packet is ICMP
{
	struct iphdr *IP_Header = (struct iphdr*)buffer;		//Get the IP Header part of this packet
	if(IP_Header->protocol ==1) 							//Check the Protocol and do accordingly...
	{
	int IP_HeaderLength = IP_Header->ihl*4;
	struct icmphdr *ICMP_Header = (struct icmphdr *)(buffer + IP_HeaderLength);	//decapsulate
	IP_Printer(buffer);										//sends the packet to print the ip source and dest
	printf("Type		: %d\n",(int)(ICMP_Header->type));		
	printf("Code 		: %d\n",(int)(ICMP_Header->code));
	}
}
 
void IP_Printer(char* buffer)								//Decapsulate the IP header and prints source ip and dest ip
{
	struct iphdr *IP_Header = (struct iphdr *)buffer;		//Get the IP Header part of this packet
	int IP_HeaderLength =IP_Header->ihl*4;					//decapsulation
	memset(&IP_Source, 0, sizeof(IP_Source));				//clears the source
	IP_Source.sin_addr.s_addr = IP_Header->saddr;			//source IP
	memset(&IP_Dest, 0, sizeof(IP_Dest));					//clears the source
	IP_Dest.sin_addr.s_addr = IP_Header->daddr;				//dest IP
	printf("Source IP     	: %s\n",inet_ntoa(IP_Source.sin_addr));
	printf("Destination IP	: %s\n",inet_ntoa(IP_Dest.sin_addr));
}									//^a function that converts the big endian to a string

int main()
{
	int adderLength , stream;
	struct sockaddr address;
	char buffer[bufferSize];
	sock = socket(AF_INET , SOCK_RAW , IPPROTO_ICMP);		//Create a raw socket to start sniffing
	if(sock < 0)											//checks if the socket was created, did you use sudo?
	{
		printf("Socket Error\nDid you use sudo?\n");		//a neccecery print for Tehila's heart problems
		return 1;
	}
	printf("Starting the sniffing process...\nPress CTRL+Z to stop\n");	//aww yea we can sniff now
	while(1)															//where do you keep the "good" packets?
	{
		adderLength = sizeof(address);
		stream = recvfrom(sock , buffer , bufferSize , 0 , &address , &adderLength);//Receive a packet
		if(stream <0 )
		{
			printf("failed when reciving packets\n");
			return 1;
		}
		ICMP_Detector(buffer , stream);								//send the packet to check if its ICMP
		printf("\nContinue sniffing or press CTRL+Z to stop\n\n");
		memset(&buffer, 0, sizeof(buffer));							//clears the buffer to get new packets
	}
	return 0;
}