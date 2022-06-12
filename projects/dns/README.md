# Overview

In this assignment, we will write a very simple DNS server. You should develop your code in the provided virtual machine. Submissions that fail to compile or run in the provided virtual machine will not be graded.

## Learning Objectives

- Understanding the DNS protocol.
- Understanding DNS message format.

## Specification

The starter code implements a simple DNS server, but two functions are missing:

You are required to implement the following functions:

```c
char *decode_domain_name(int len);
void encode_domain_name(const char *domain);
```

when dig www.cnn.com, the dig command will send you a domain in such a format: 3www3cnn3com0, here, the first '3' means there are 3 characters in 'www', the second '3' means there are 3 characters in 'cnn', the third '3' means there are 3 characters in 'com', the '0' means this is the end of the domain.

when receive the DNS query, the server first calls decode_domain_name() which translates 3www3cnn3com0 to www.cnn.com; before the server responds, the server calls encode_domain_name() which translates www.cnn.com back to 3www3cnn3com0. Thus, doing such translation is the job of the two functions you are going to implement.

After your implemention, the DNS server should translate www.cnn.com to the ip address of fakenews.com, which, as of today (06/08/2022), is 188.126.71.216.

**Note**: you are recommended to read the starter code, although it is not necessary for this project, it helps you understand how a DNS server works.

## Global Variable

The starter code defines the following global variables.

```c
uint8_t buffer[BUF_SIZE];
char *buf_p=(char *)buffer;
```

When dig www.cnn.com, the dig command will sent to the DNS server a DNS query message, this entire DNS query message will be stored in *buffer*[]. This message contains the domain name in this format *3www3cnn3com0*, this entire message's length will be passed as the argument to the function *decode_domain_name*(). When the server starts, *buf_p* points to be beginning of *buffer*[]. But when *decode_domain_name*() is called, *buf_p* points to the beginning of the string "3www3cnn3com0", which is the address of number '3'; when *decode_domain_name*() returns, you must set *buf_p* to the address right after the number '0'. For example, if the DNS query message contains "3www3cnn3com0abcdefg", then when *decode_domain_name*() returns, *buf_p* should be pointing to the letter 'a'.

Before responding to the DNS client, the DNS server will call *encode_domain_name*(), whose argument *domain* stores the string "www.cnn.com", and *encode_domain_name*() will convert it to "3www3cnn3com0". The moment when *encode_domain_name*() is called, *buf_p* is pointing to an address where you should store the converted domain, i.e., "3www3cnn3com0"; when *encode_domain_name() returns, you must set *buf_p* to the address right after the trailing '0'. For example, if the DNS response message contains "3www3cnn3com0abcdefg", then when this function returns, buf_p must point to 'a'.

## Testing 

To test the program, we need to have two VMs, one serves as the client, and one serves as the server. After running make on the server side, you can run this to start the DNS server:

```console
[06/08/22]seed@VM:~/dns-server$ ./dnsserver -p 9090
Listening on port 9090.
```

## Expected Results

Once the server is running, on the client machine, assume the DNS server's IP address is 172.16.77.129, you can run:

```console
[06/08/22]seed@VM:~$ dig @172.16.77.129 -p 9090 www.cnn.com

; <<>> DiG 9.10.3-P4-Ubuntu <<>> @172.16.77.129 -p 9090 www.cnn.com
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 3818
;; flags: qr; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0

;; QUESTION SECTION:
;www.cnn.com.			IN	A

;; ANSWER SECTION:
www.cnn.com.		3600	IN	A	188.126.71.216

;; Query time: 1 msec
;; SERVER: 172.16.77.129#9090(172.16.77.129)
;; WHEN: Wed Jun 08 10:14:08 EDT 2022
;; MSG SIZE  rcvd: 56
```

## Submission

Due: 23:59pm, June 20th, 2022. Late submission will not be accepted/graded. Please submit this assignment on canvas, in the format of a pdf report, the report should include:

1. your implementation of the two functions.
2. your testing results: just paste screenshots, similar to what you provide in the lab reports.
3. one or two paragraph(s) of reflection describing what you learned or what problems you experienced and/or how you solved the problem(s).


## Grading Rubric
Grade: /15

You get the above testing result (showed by your screenshot) when running the above dig command: 12pts.  
Reflection: 3pts.
