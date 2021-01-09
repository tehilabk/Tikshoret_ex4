#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h> 

// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8 

// Checksum algo
unsigned short calculate_checksum(unsigned short * paddress, int len);

#define SOURCE_IP "10.0.2.15"
#define DESTINATION_IP "127.0.0.1"


int main ()
{
    struct ip iphdr; // IPv4 header
    struct icmp icmphdr; // ICMP-header
    char data[IP_MAXPACKET] = "Ping.\n";
    int datalen = strlen(data) + 1;

    //==================
    // IP header
    //==================

    iphdr.ip_v = 4; // IP protocol version 4

    //TODO check if not workung
    iphdr.ip_hl = IP4_HDRLEN / 4; // IP header length
    iphdr.ip_tos = 0; // not using, zero.
    iphdr.ip_len = htons (IP4_HDRLEN + ICMP_HDRLEN + datalen);  // Total length of datagram (16 bits): IP header + ICMP header + ICMP data - converted to big endian
    iphdr.ip_id = 0; // ID sequence- not in use since we do not allow fragmentation
    int ip_flags[4] ={0}; // fragmentation
    // ip_flags[0] = 0;  Reserved bit
    // ip_flags[1] = 0; "Do not fragment" bit
    // ip_flags[2] = 0; "More fragments" bit
    // ip_flags[3] = 0;  Fragmentation offset (13 bits)

    //TODO check if need to delete
    iphdr.ip_off = htons ((ip_flags[0] << 15) + (ip_flags[1] << 14) + (ip_flags[2] << 13) +  ip_flags[3]);
         
    iphdr.ip_ttl = 1024; // TTL 

    iphdr.ip_p = IPPROTO_ICMP; // Upper protocol (8 bits): ICMP 

    
    if (inet_pton (AF_INET, SOURCE_IP, &(iphdr.ip_src)) <= 0) // Source IP
    {
        printf ("inet_pton() failed for source-ip with error: %d");
        return -1;
    }

    // Destination IPv
    if (inet_pton (AF_INET, DESTINATION_IP, &(iphdr.ip_dst)) <= 0)
    {
        printf ("inet_pton() failed for destination-ip with error: %d"); 
        return -1;
    }

    iphdr.ip_sum = 0;
    iphdr.ip_sum = calculate_checksum((unsigned short *) &iphdr, IP4_HDRLEN);


    //===================
    // ICMP header
    //===================

    icmphdr.icmp_type = ICMP_ECHO; // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_code = 0; // Message Code (8 bits): echo request
    icmphdr.icmp_id = 27; // dor's lucky number
    icmphdr.icmp_seq = 0;
    icmphdr.icmp_cksum = 0;  // ICMP header checksum
    char packet[IP_MAXPACKET]; // maximum packet size

    //===================
    // Encapsulation process
    //===================

    memcpy (packet, &iphdr, IP4_HDRLEN); // First header- IP header.
    memcpy ((packet + IP4_HDRLEN), &icmphdr, ICMP_HDRLEN); // Next header- ICMP header
    memcpy (packet + IP4_HDRLEN + ICMP_HDRLEN, data, datalen); // After ICMP header, add the ICMP data.
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *) (packet + IP4_HDRLEN), ICMP_HDRLEN + datalen);     // Calculate the ICMP header checksum

    //TODO check if delete
    memcpy ((packet + IP4_HDRLEN), &icmphdr, ICMP_HDRLEN);

    struct sockaddr_in dest_in; 
    memset (&dest_in, 0, sizeof (struct sockaddr_in)); //Zero the port dest (ICMP dosnt use ports)
    dest_in.sin_family = AF_INET; //IPV4
    dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;

    
    // Create raw socket for IP-RAW (make IP-header by yourself)
    int sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock== -1) 
    {
        printf ( "socket() failed with error: %d");
        printf ("To create a raw socket, the process needs to be run by Admin/root user.\n");
        return -1;
    }
        printf("socket open sucsses\n");

    //TODO check if not need setsockopt
    const int flagOne = 1;
    if (setsockopt (sock, IPPROTO_IP, IP_HDRINCL, &flagOne, sizeof (flagOne)) == -1) 
    {
        printf ("setsockopt() failed with error: %d");	
        return -1;
    }
    int send = sendto(sock, packet, IP4_HDRLEN + ICMP_HDRLEN + datalen, 0, (struct sockaddr *) &dest_in, sizeof (dest_in));
    if(send == -1)  
    {
        printf ("sendto() failed with error: %d");
        return -1;
    }
         printf("ping sent sucssesfuly\n");

  
   close(sock);

   return 0;
}

// Compute checksum- code from moodlearn.
unsigned short calculate_checksum(unsigned short * paddress, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short * w = paddress;
	unsigned short answer = 0;

	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1)
	{
		*((unsigned char *)&answer) = *((unsigned char *)w);
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
	sum += (sum >> 16);                 // add carry
	answer = ~sum;                      // truncate to 16 bits

	return answer;
}
