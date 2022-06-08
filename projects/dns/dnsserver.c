#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define BUF_SIZE 1500
#define MIN(x, y) ((x) <= (y) ? (x) : (y))

/*
* This software is licensed under the CC0.
*
* This is a _basic_ DNS Server for educational use.
* It does not prevent invalid packets from crashing
* the server.
*
* To test start the program and issue a DNS request:
*  dig @127.0.0.1 -p 9090 www.cnn.com
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

/* Operation Code */
enum {
  QUERY_OperationCode = 0, /* standard query */
  IQUERY_OperationCode = 1, /* inverse query */
  STATUS_OperationCode = 2, /* server status request */
  NOTIFY_OperationCode = 4, /* request zone transfer */
  UPDATE_OperationCode = 5 /* change resource records */
};

/* Response Code */
enum {
  NoError_ResponseCode = 0,
  FormatError_ResponseCode = 1,
  ServerFailure_ResponseCode = 2,
  NameError_ResponseCode = 3
};

/* Query Type */
enum {
  IXFR_QueryType = 251,
  AXFR_QueryType = 252,
  MAILB_QueryType = 253,
  MAILA_QueryType = 254,
  STAR_QueryType = 255
};

/*
* Types.
*/

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

        		for (i = 0; i < 4; ++i)
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

size_t get16bits(const uint8_t **buffer)
{
	uint16_t value;

	memcpy(&value, *buffer, 2);
	/* increment the pointer by 2 bytes. */
	*buffer += 2;

	return ntohs(value);
}

void put8bits(uint8_t **buffer, uint8_t value)
{
  memcpy(*buffer, &value, 1);
  *buffer += 1;
}

void put16bits(uint8_t **buffer, uint16_t value)
{
  value = htons(value);
  memcpy(*buffer, &value, 2);
  *buffer += 2;
}

void put32bits(uint8_t **buffer, uint32_t value)
{
  value = htonl(value);
  memcpy(*buffer, &value, 4);
  *buffer += 4;
}


/*
* Deconding/Encoding functions.
*/

/* this is what we receive from the client: 3www3cnn3com0, which is in *buf, and we want to convert it to: www.cnn.com. */
char *decode_domain_name(const uint8_t **buf, int len)
{
	char domain[256];
	int i=0;
//printf("buf is %s\n",*buf);
	for (i=1; i < MIN(256, len); i++) {
		uint8_t c = (*buf)[i];
//printf("c is %c, len is %d\n",c,len);
		/* the original domain name always contains a 0 at the very end... */
		if (c == 0) {
			domain[i - 1] = 0;
			*buf += i + 1;
			/* we call strdup() to duplicate a string. */
//printf("domain is %s\n",domain);
			return strdup(domain);
		/* letter A's corresponding ascii code is 65. any*/
		} else if (c <= 64) {
			domain[i - 1] = '.';
//printf("ever here?\n");
		} else {
			domain[i - 1] = c;
		}
	}

	return NULL;
}

void decode_header(struct Message *msg, const uint8_t **buffer)
{
	msg->id = get16bits(buffer);

	/* the next field is for flags. */
	uint16_t flags = get16bits(buffer);
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

	msg->qdCount = get16bits(buffer);
	msg->anCount = get16bits(buffer);

	/* we don't really care about nsCount and arCount,
	 * they should always be 0 in our case,
	 * but here we call get16bits() just to move the pointer... */
	msg->nsCount = get16bits(buffer);
	msg->arCount = get16bits(buffer);
	/* FIXME: not sure why, but without this line, arCount seems to be 1... */
	msg->arCount = 0;
}

int decode_msg(struct Message *msg, const uint8_t *buffer, int size)
{
	decode_header(msg, &buffer);

	// parse the question
	uint32_t qcount = msg->qdCount;
	/* we only support one single question. */
	if(qcount!=1){
		printf("we only support one single question!\n");
		return -1;
	}
//printf("buffer is %s\n",buffer);
	struct Question *q = malloc(sizeof(struct Question));
	q->qName = decode_domain_name(&buffer, size);
	q->qType = get16bits(&buffer);
	q->qClass = get16bits(&buffer);
	if (q->qName == NULL) {
		printf("Failed to decode domain name!\n");
		return -1;
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

void encode_header(struct Message *msg, uint8_t **buffer)
{
	put16bits(buffer, msg->id);
	int flags = 0;
	/* we need to set to 1 so as to indicate that it is a response. */
	flags |= (1 << 15) & 0x8000;
	flags |= (msg->rcode << 0) & 0x000F; // bit 3:0.
	/* TODO: insert the rest of the flags */
	put16bits(buffer, flags);

	put16bits(buffer, msg->qdCount);
	put16bits(buffer, msg->anCount);
	put16bits(buffer, msg->nsCount);
	put16bits(buffer, msg->arCount);
}

/* this is what we have right now: www.cnn.com, but the client is expected to get: 3www3cnn3com0, thus we need to convert it to: www.cnn.com. */
void encode_domain_name(uint8_t **buffer, const char *domain)
{
	uint8_t *buf = *buffer;
	const char *beg = domain;
	const char *pos;
	int len = 0;
	int i = 0;

	/* we call strchr() to locate character in string. */
	while ((pos = strchr(beg, '.'))) {
		len = pos - beg;
		/* first we store the length... */
		buf[i] = len;
		i += 1;
		/* then we store whatever in between two dots... */
		memcpy(buf+i, beg, len);
		i += len;
		beg = pos + 1;
	}

	/* copy the remaining: whatever after the last '.', i.e., the top level domain, "edu" or "com". */
	len = strlen(domain) - (beg - domain);
	/* still, we store the lenght first. */
	buf[i] = len;
	i += 1;
	memcpy(buf + i, beg, len);
	i += len;
	/* we store a zero at the very end. */
	buf[i] = 0;
	i += 1;
	*buffer += i;
}

/* fill in buffer based on the information in rr; return 0 upon failure, 1 upon success */
int encode_resource_records(struct ResourceRecord *rr, uint8_t **buffer)
{
	int i;
	if(rr) {
		// answer question by attaching resource sections.
		encode_domain_name(buffer, rr->name);
		put16bits(buffer, rr->type);
		put16bits(buffer, rr->class);
		put32bits(buffer, rr->ttl);
		put16bits(buffer, rr->rd_length);
		if(rr->type==1) {
        		for (i = 0; i < 4; ++i)
          			put8bits(buffer, rr->rd_data.a_record.addr[i]);
		}else{
			fprintf(stderr, "Unknown type %u. => Ignore resource record.\n", rr->type);
      			return 1;
    		}
  	}

	return 0;
}

/* @return 0 upon failure, 1 upon success */
int encode_msg(struct Message *msg, uint8_t **buffer)
{
  struct Question *q;
  int rc;

  encode_header(msg, buffer);

  q = msg->question;
  if(q!=NULL) {
    encode_domain_name(buffer, q->qName);
    put16bits(buffer, q->qType);
    put16bits(buffer, q->qClass);

  }

  rc = 0;
  rc |= encode_resource_records(msg->answer, buffer);

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
	// buffer for input/output binary packet
	uint8_t buffer[BUF_SIZE];
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
		if (decode_msg(&msg, buffer, nbytes) != 0) {
			continue;
		}

		/* print query, i.e., at this moment, msg contains the query only. */
		print_message(&msg);

		/* resolve query and put the answer into the query message */
		resolve_query(&msg);

		/* print response, now msg contains both the query and the answer... */
		print_message(&msg);

		uint8_t *p = buffer;
		/* parse msg and fill in buffer accordingly... */
		if (encode_msg(&msg, &p) != 0) {
			continue;
		}

		/* we call sendto() function to send a message on a socket,
		 * and we send out whatever content is in the buffer.
		 * we expect buffer contains a DNS response. */
		int buflen = p - buffer;
		sendto(sock, buffer, buflen, 0, (struct sockaddr*) &client_addr, addr_len);
	}
}
