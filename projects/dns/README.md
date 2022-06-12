# Overview

In this assignment, we will write a very simple DNS server. You should develop your code in the provided virtual machine. Submissions that fail to compile or run in the provided virtual machine will not be graded.

## Learning Objectives

- Understanding the DNS protocol.
- Understanding DNS message format.

## Specification

The starter code implements a simple DNS server, but two functions are missing:

You are required to implement the following functions:

```c
char *decode_domain_name(const uint8_t **buf, int len);
void encode_domain_name(uint8_t **buffer, const char *domain);
```

when dig www.cnn.com, the dig command will send you a domain in such a format: 3www3cnn3com0, here, the first '3' means there are 3 characters in 'www', the second '3' means there are 3 characters in 'cnn', the third '3' means there are 3 characters in 'com', the '0' means this is the end of the domain.

when receive the DNS query, the server first calls decode_domain_name() which translates 3www3cnn3com0 to www.cnn.com; before the server responds, the server calls encode_domain_name() which translates www.cnn.com back to 3www3cnn3com0. Thus, doing such translation is the job of the two functions you are going to implement.

After your implemention, the DNS server should translate www.cnn.com to the ip address of fakenews.com, which, as of today (06/08/2022), is 188.126.71.216.

## Testing 

After running make on the server side, you can run this to start the DNS server:

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
3. one or two paragraph(s) of reflection describing what you learned or what problems you experience and/or how you solve the problem(s).


## Grading Rubric
Grade: /15

You get the above testing result (showed by your screenshot) when running the above dig command: 12pts.
Reflection: 3pts
