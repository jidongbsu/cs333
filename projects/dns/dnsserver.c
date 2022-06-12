#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#define BUF_SIZE 1500

/* buffer for input/output binary packet */
uint8_t buffer[BUF_SIZE];

/* we use this pointer to traverse buffer. */
char *buf_p=(char *)buffer;

/*
* This software is licensed under the CC0.
*
* This is a _basic_ DNS Server for educational use.
* It does not prevent invalid packets from crashing
* the server.
*
* To test start the program and issue a DNS request:
* on the server side, run:
* # ./dnsserver -p 9090
* 
* and then on the client side, run:
* # dig @dns_server_ip -p 9090 www.cnn.com
*/

/* Response Type */
enum {
  Ok_ResponseType = 0,
  FormatError_ResponseType = 1,
  ServerFailure_ResponseType = 2,
  NameError_ResponseType = 3,
  NotImplemented_ResponseType = 4,
  Refused_ResponseType = 5
};

/* Question Section */
struct Question {
  char *qName;
  uint16_t qType;
  uint16_t qClass;
};

/* Data part of a Resource Record */
union ResourceData {
  struct {
    uint8_t txt_data_len;
    char *txt_data;
  } txt_record;
  struct {
    uint8_t addr[4];
  } a_record;
  struct {
    uint8_t addr[16];
  } aaaa_record;
};

/* Resource Record Section */
struct ResourceRecord {
  char *name;
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t rd_length;
  union ResourceData rd_data;
};

struct Message {
	uint16_t id; /* Identifier */

	/* Flags */
	uint16_t qr; /* Query/Response Flag */
	uint16_t opcode; /* Operation Code */
	uint16_t aa; /* Authoritative Answer Flag */
	uint16_t tc; /* Truncation Flag */
	uint16_t rd; /* Recursion Desired */
	uint16_t ra; /* Recursion Available */
	uint16_t rcode; /* Response Code */

	uint16_t qdCount; /* Question Count */
	uint16_t anCount; /* Answer Record Count */
	uint16_t nsCount; /* Authority Record Count */
	uint16_t arCount; /* Additional Record Count */

	/* one question only; */
	struct Question *question;
	/* one answer only */
	struct ResourceRecord *answer;
};

int get_A_Record(uint8_t addr[4], const char domain_name[])
{
  if (strcmp("www.cnn.com", domain_name) == 0) {
    addr[0] = 188;
    addr[1] = 126;
    addr[2] = 71;
    addr[3] = 216;
    return 0;
  } else {
    return -1;
  }
}

/*
* Debugging functions.
*/

void print_resource_record(struct ResourceRecord *rr)
{
	int i;
	if (rr) {
		printf("  ResourceRecord { name '%s', type %u, class %u, ttl %u, rd_length %u, ",
			rr->name,
			rr->type,
			rr->class,
			rr->ttl,
			rr->rd_length);

		union ResourceData *rd = &rr->rd_data;
		if(rr->type==1) {
			printf("Address Resource Record { address ");

        		for (i = 0; i < 4; i++)
          			printf("%s%u", (i ? "." : ""), rd->a_record.addr[i]);
        		printf(" }");
		}else{
			printf("Unknown Resource Record { ??? }");
		}
		printf("}\n");
	}
}

void print_message(struct Message *msg)
{
  struct Question *q;

  printf("QUERY { ID: %02x", msg->id);
  printf(". FLAGS: [ QR: %u, OpCode: %u ]", msg->qr, msg->opcode);
  printf(", QDcount: %u", msg->qdCount);
  printf(", ANcount: %u", msg->anCount);
  printf(", NScount: %u", msg->nsCount);
  printf(", ARcount: %u,\n", msg->arCount);

  q = msg->question;
  if(q) {
    printf("  Question { qName '%s', qType %u, qClass %u }\n",
      q->qName,
      q->qType,
      q->qClass
    );
  }

  print_resource_record(msg->answer);

  printf("}\n");
}


/*
* Basic memory operations.
*/

size_t get16bits(void)
{
	uint16_t value;
	memcpy(&value, buf_p, 2);
	return ntohs(value);
}

void put8bits(uint8_t value)
{
  memcpy(buf_p, &value, 1);
}

void put16bits(uint16_t value)
{
  value = htons(value);
  memcpy(buf_p, &value, 2);
}

void put32bits(uint32_t value)
{
  value = htonl(value);
  memcpy(buf_p, &value, 4);
}


/*
* Deconding/Encoding functions.
*/

/* the parameter len indicates the length of the whole buffer
 * which stores the DNS query message the client sends to us.
 * Inside this message, the domain name is stored in this format: 3www3cnn3com0, 
 * which is stored at address pointed to by this pointer: char *buf_p, 
 * and we want to convert it to: www.cnn.com
 * the moment this function is called, you can assume buf_p is pointing to
 * the very beginning of this domain name, which is '3'.
 * if successful, this function returns a buffer which stores "www.cnn.com",
 * you must allocate memory for this buffer, and you can assume the caller
 * will release the memory.
 * if somehow not successful, return NULL.
 * note: when this function returns, the pointer buf_p must point to the address right after the trailing '0'.
 * for example, if the message contains "3www3cnn3com0abcdefg", 
 * then when this function returns, buf_p must point to 'a'.
 */
char *decode_domain_name(int len)
{
	/* add your code here */
}

/* information is in buffer, we want to store it in msg. */
void decode_header(struct Message *msg)
{
	msg->id = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	/* the next field is for flags. */
	uint16_t flags = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	/* bit 15 is QR, query/response flag. when 0, message is a query. when 1, message is response. */
	msg->qr = flags >> 15;
	/* bit 14:11 is opcode, operation code. tells receiving machine the intent of the message. 
	 * generally 0 meaning normal query, however there are other valid options such as 1 for reverse query
	 * and 2 for server status. 0x7800 = 0111 1000 0000 0000, which basically allows us to take bit 14:11. */
	msg->opcode = (flags & 0x7800) >> 11;
	/* bit 10 is AA, authoritative answer.
	 * set only when the responding machine is the authoritative name server of the queried domain.
	 * 0x400 = 0000 0100 0000 0000, which allows us to take bit 10.  */
	msg->aa = (flags & 0x400) >> 10;
	/* bit 9 is TC, truncated. set if packet is larger than the UDP maximum size of 512 bytes.
	 * 0x200 is 0000 0010 0000 0000, which allows us to get bit 9. */
	msg->tc = (flags & 0x200) >> 9;
	/* bit 8 is RD, recursion desired.
	 * if 0, the query is an iterative query.
	 * if 1, the query is recursive.
	 * 0x100 is 0000 0001 0000 0000, which allows us to get bit 8. */
	msg->rd = (flags & 0x100) >> 8;
	/* bit 7 is RA, recursion available.
	 * 0x80 is 1000 0000, which allows us to get bit 7. */
	msg->ra = (flags & 0x80) >> 7;
	/* bit 3 to bit 0 is Rcode, return code.
	 * it will generally be 0 for no error,
	 * or 3 if the name does not exist.
	 * 0xF is 0000 0000 0000 1111, which allows us to get bit 3:0. */
	msg->rcode = (flags & 0xF) >> 0;

	msg->qdCount = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	msg->anCount = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	/* we don't really care about nsCount and arCount,
	 * they should always be 0 in our case,
	 * but here we call get16bits() just to move the pointer... */
	msg->nsCount = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	msg->arCount = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;
	/* FIXME: not sure why, but without this line, arCount seems to be 1... */
	msg->arCount = 0;
}

int decode_msg(struct Message *msg, int size)
{
	decode_header(msg);

	// parse the question
	uint32_t qcount = msg->qdCount;
	/* we only support one single question. */
	if(qcount!=1){
		printf("we only support one single question!\n");
		return -1;
	}
	if(size>=256){
		printf("we only support messages who size is smaller than 256 bytes");
		return -1;
	}
//printf("buffer is %s\n",buffer);
	struct Question *q = malloc(sizeof(struct Question));
	q->qName = decode_domain_name(size);
	/* we expect type to be 1, which means A record */
	q->qType = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;
	/* we expect class to be 1, which means Internet  */
	q->qClass = get16bits();
	/* take 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;
	if (q->qName == NULL) {
		printf("Failed to decode domain name!\n");
		return -1;
	}else{
		printf("qname is %s\n", q->qName);
	}

	msg->question = q;

	return 0;
}

/* for every question in the message,
 * add a appropiate resource record in the answer section.
 */
void resolve_query(struct Message *msg)
{
	struct ResourceRecord *rr;
	struct Question *q;
	int rc;

	// leave most values intact for response
	msg->qr = 1; // this is a response
	msg->aa = 1; // this server is authoritative
	msg->ra = 0; // no recursion available
	msg->rcode = Ok_ResponseType;

	// for every question append resource records
	q = msg->question;
	if(q!=NULL) {
		rr = malloc(sizeof(struct ResourceRecord));
		memset(rr, 0, sizeof(struct ResourceRecord));

		rr->name = strdup(q->qName);
		rr->type = q->qType;
		rr->class = q->qClass;
		rr->ttl = 60*60; // in seconds; 0 means no caching

		printf("Query for '%s'\n", q->qName);

		/* we only support A record queries as of now..., and type 1 is A record */
		if(q->qType==1) {
			rr->rd_length = 4;
			rc = get_A_Record(rr->rd_data.a_record.addr, q->qName);
			if (rc < 0)
			{
				free(rr->name);
				free(rr);
				return;
			}
		}else{
			free(rr);
			msg->rcode = NotImplemented_ResponseType;
			printf("Cannot answer question of type %d.\n", q->qType);
			exit(EXIT_FAILURE);
		}

		/* we only provide one answer. */
		msg->anCount=1;

		// prepend resource record to answers list
		msg->answer = rr;
	}
}

/* information is in msg, and we want to store it in buffer. */
void encode_header(struct Message *msg)
{
	int flags = 0;

	/* transaction id */
	put16bits(msg->id);
	/* write 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	/* we need to set to 1 so as to indicate that it is a response. */
	flags |= (1 << 15) & 0x8000;
	flags |= (msg->rcode << 0) & 0x000F; // bit 3:0.
	/* TODO: insert the rest of the flags */
	put16bits(flags);
	/* write 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	put16bits(msg->qdCount);
	/* write 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	//put16bits(0);
	put16bits(msg->anCount);
	/* write 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	put16bits(msg->nsCount);
	/* write 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;

	put16bits(msg->arCount);
	/* write 2 bytes, move pointer bytes forward */
	buf_p=buf_p+2;
}

/* this is what we have right now: www.cnn.com, but the client is expected to get: 3www3cnn3com0, 
 * thus we need to convert it to: 3www3cnn3com0.
 * the domain name is currently stored at the address pointed to by domain, 
 * and we want to store it in buffer, which contains a DNS response message we want to send to the client.
 * when this function is called, char *buf_p, which is a global pointer,
 * is pointing to an address where you should store the converted domain, i.e., 3www3cnn3com0
 * note: when this function returns, the pointer buf_p must point to the address right after the trailing '0'.
 * for example, if the message contains "3www3cnn3com0abcdefg", 
 * then when this function returns, buf_p must point to 'a'.
 */
void encode_domain_name(const char *domain)
{
	/* add your code here */
}

/* fill in buffer based on the information in rr; return 0 upon failure, 1 upon success */
int encode_resource_records(struct ResourceRecord *rr)
{
	int i;
	if(rr) {
		// answer question by attaching resource sections.
		encode_domain_name(rr->name);

		put16bits(rr->type);
		/* write 2 bytes, move pointer 2 bytes forward */
		buf_p=buf_p+2;

		put16bits(rr->class);
		/* write 2 bytes, move pointer 2 bytes forward */
		buf_p=buf_p+2;

		/* ttl is 4 bytes */
		put32bits(rr->ttl);
		/* write 4 bytes, move pointer 4 bytes forward */
		buf_p=buf_p+4;

		put16bits(rr->rd_length);
		/* write 2 bytes, move pointer 2 bytes forward */
		buf_p=buf_p+2;

		if(rr->type==1) {
        		for (i = 0; i < 4; i++){
          			put8bits(rr->rd_data.a_record.addr[i]);
				/* write 1 byte, move pointer 1 byte forward */
				buf_p=buf_p+1;
			}
		}else{
			fprintf(stderr, "Unknown type %u. => Ignore resource record.\n", rr->type);
      			return 1;
    		}
  	}

	return 0;
}

/* @return 0 upon failure, 1 upon success.
 * information is in msg, and we want to store it into buffer.
 */
int encode_msg(struct Message *msg)
{
	struct Question *q;
	int rc;

	encode_header(msg);

	/* construct the question section. */
	q = msg->question;
	if(q!=NULL) {
		encode_domain_name(q->qName);
		put16bits(q->qType);
		/* write 2 bytes, move pointer bytes forward */
		buf_p=buf_p+2;
		put16bits(q->qClass);
		/* write 2 bytes, move pointer bytes forward */
		buf_p=buf_p+2;
	}

	/* construct the answer section. */
	rc = 0;
	rc |= encode_resource_records(msg->answer);

	return rc;
}

void free_resource_records(struct ResourceRecord *rr)
{
  if(rr) {
    free(rr->name);
    free(rr);
  }
}

void free_question(struct Question *qq)
{
  if(qq) {
    free(qq->qName);
    free(qq);
  }
}

int main(int argc, char *argv[])
{
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	int nbytes, rc;
	int sock;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s -p <port number> \n", argv[0]);
		exit(1);
	}

        int port = atoi(argv[2]);

	struct Message msg;
	memset(&msg, 0, sizeof(struct Message));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	rc = bind(sock, (struct sockaddr*) &addr, addr_len);

	if (rc != 0) {
		printf("Could not bind: %s\n", strerror(errno));
		return -1;
	}

	printf("Listening on port %u.\n", port);

	while (1) {
		free_question(msg.question);
		free_resource_records(msg.answer);
		memset(&msg, 0, sizeof(struct Message));

		/* we call recvfrom() function to receive a message from a socket, 
		 * and we expect the message to be a DNS query, and the message will now be stored in buffer,
		 * this function returns the length of the message on successful completion. */
		nbytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &addr_len);

//printf("at first, buffer is %s,nbytes is %d\n",buffer,nbytes);
		/* this function decode buffer and then fill in msg accordingly... */
		if (decode_msg(&msg, nbytes) != 0) {
			continue;
		}

		/* print query, i.e., at this moment, msg contains the query only. */
		print_message(&msg);

		/* resolve query and put the answer into the query message */
		resolve_query(&msg);

		/* print response, now msg contains both the query and the answer... */
		print_message(&msg);

		/* let buf_p still point to the beginning of the buffer */
		buf_p = (char *)buffer;
		/* parse msg and fill in buffer accordingly... */
		if (encode_msg(&msg) != 0) {
			continue;
		}

		/* we call sendto() function to send a message on a socket,
		 * and we send out whatever content is in the buffer.
		 * we expect buffer contains a DNS response. */
		int buflen = buf_p - (char *)buffer;
		sendto(sock, buffer, buflen, 0, (struct sockaddr*) &client_addr, addr_len);
	}
}
