## Forensic Analysis: Decrypt SSL/TLS Packets with Wireshark

### Requirements 

In this lab, we assume your employee Bob has secretly downloaded some files into your network, but fortunately your wireshark has captured the suspicious traffic for you to analyze. You can download the capture file named forensics.pcapng from here: http://cs.boisestate.edu/~jxiao/cs333/info/tls/forensics.pcapng. Your goal is to find out the files (and their contents) Bob has downloaded. (Note that 10.142.0.3 is the private ip address assigned to Bob in your network.)

### Setup

Only one VM is sufficient.

### Steps

1. use wireshark to open the capture file forensics.pcapng. From the capture, we can see there is FTP traffic between Bob's machine 10.142.0.3 and a remote server 104.155.183.43. Locate the first FTP packet, right click -> follow -> TCP stream, this shows Bob transferred a file called key.zip. (in FTP, retr command means retrieve a file.)

![alt text](lab9-import.png "Lab 9 Import")

2. go back to the main interface of wireshark, locate the FTP packet (packet 55) that actually transfers this key.zip (After step 1, you need to clear the filter at the top of wireshark, otherwise you won't see the FTP-DATA packet, which is packet 55), and then do the following:

2.1. right click -> follow -> tcp stream
2.2. show and save data as: raw
2.3. save as: key.zip

3. unzip key.zip

it will generate a file called server_key.pem, in the directory of user/ftp/files/private/. This is a private key. We can use cat command to view the content of this file. For example,

```console
# cat server_key.pem 
-----BEGIN RSA PRIVATE KEY-----
MIICXQIBAAKBgQDJ3egkT4J5h4klOdnXiPB8wra4dFMZXPYZk3NWBYmlu5ImhMg7
Iao7qrv/rNe4SNN/vbTLmuo/BJ0/L5O1QDYaZMFt63tFnhTjMWPAtx7uUGBJ+JZk
PobNutrv463/tlTV0yvncL0x6Epapgtrur1b8vAl60nqcBFLxQ6EUGEUpQIDAQAB
AoGBAIywHnkn/MwMPPX30q4Xn1ukLMpzL/MuScpbbHeYVm1uQ5aa1h34AXeiL3Iq
mRGzl8uggF+icP1IjvsZgn1A6jLOATCKhmaoEldHj4OciaGpys3InFkY8sqlgjV+
OHwbYYVt7a3hrcphN6I5nUAbK8VXz7ne3r3PwN6DIubqfeMhAkEA/nPdOGK3Cgdx
vpTHnEK39XwWHOjDJURTsW78KNo1sqQbDYhGQAAlvXNoClz4dpjoVCxkIuHZ3zMC
+5IrKW0KaQJBAMsYLSGyM2WCl9NAcQ2l9lFQ1matTNy6cFmZckRv15nrF/X2mrtM
BqyxJAY/rw/FjEoQde30kWr4ZrpbI7DzWN0CQQD8Km7t1kZ3VfFPYbqADU6ppZN0
iuf6IsCectLK2ZWluCRnQMn92yeLnLdardA+GEMGSAfk2dZE8BTgo3bK0xkhAkBU
X9/oHtuBcabXye7t56QUlkvsblT/YqloX/p7+icNOAFFW0VfSK+BMTqxKeX13HFz
F7GWwUNkuHwdMd37Fq41AkBJC0pZtkbzdUzwZre8XylK8rI62xJUUdOmyowaSmi/
vEJTGtlc2+b5ztlsYjzsPFaI49fw8QNawtZj1e3CRc7w
-----END RSA PRIVATE KEY-----
```

4. now we load this private key into wireshark, so that wireshark can decrypt TLS packets. In wireshark, do the following:

4.1. edit -> preferences -> protocols -> ssl or tls (depending on your version, you may have ssl, or tls, but not both).
4.2. rsa keys list -> edit
4.3. add one entry with following information:

IP Address: 104.155.183.43
Port: 443
Protocol: ftp
Key File: server_key.pem (you need to either browse or manually type the full path of this key file.)
Password: (empty)

5. now we can examine the TLS packets. Each "Client Hello" represents one TLS conversation. Locate one tls packet, and use "right click->follow->tls (or ssl) stream", we can see the 1st conversation downloads the file index.html - and this html file contains nothing but a jpg file called flag.jpg; and the 2nd conversation downloads the file flag.jpg. So this flag.jpg file could be something we are interested in. Therefore we locate any of the TLS packet in the 2nd conversation (i.e., a TLS packet that is after the 2nd "Client Hello"), do the following:

5.1. right click->follow->tls (or ssl) stream,
5.2. show and save data as: raw
5.3. save as: flag.jpg.

6. this flag.jpg right now contains HTTP header information as well as the jpg file, we need to remove the HTTP header information. Search "jpeg file signature" in google, it shows jpg file starts with signature bytes of ff d8 ff.

7. view flag.jpg with a hex viewer command: xxd.

```console
# xxd flag.jpg | less
00000000: 4745 5420 2f66 6c61 672e 6a70 6720 4854  GET /flag.jpg HT
00000010: 5450 2f31 2e31 0d0a 486f 7374 3a20 3130  TP/1.1..Host: 10
00000020: 342e 3135 352e 3138 332e 3433 0d0a 4163  4.155.183.43..Ac
00000030: 6365 7074 2d45 6e63 6f64 696e 673a 2067  cept-Encoding: g
00000040: 7a69 702c 2064 6566 6c61 7465 2c20 636f  zip, deflate, co
00000050: 6d70 7265 7373 0d0a 4163 6365 7074 3a20  mpress..Accept: 
00000060: 2a2f 2a0d 0a55 7365 722d 4167 656e 743a  */*..User-Agent:
00000070: 2070 7974 686f 6e2d 7265 7175 6573 7473   python-requests
00000080: 2f32 2e32 2e31 2043 5079 7468 6f6e 2f32  /2.2.1 CPython/2
00000090: 2e37 2e36 204c 696e 7578 2f34 2e34 2e30  .7.6 Linux/4.4.0
000000a0: 2d31 3330 2d67 656e 6572 6963 0d0a 0d0a  -130-generic....
000000b0: 4854 5450 2f31 2e30 2032 3030 206f 6b0d  HTTP/1.0 200 ok.
000000c0: 0a43 6f6e 7465 6e74 2d74 7970 653a 2074  .Content-type: t
000000d0: 6578 742f 706c 6169 6e0d 0a0d 0aff d8ff  ext/plain.......
000000e0: e000 104a 4649 4600 0101 0000 0100 0100  ...JFIF.........
```

The above output tells us ff d8 ff is at address byte 222. (Each line is 16 bytes. ff d8 ff starts at the 14th line, with an offset of 14. Therefore the address is 13*16+14=222)

The following command allows us the cut the first 221 bytes from file flag.jpg, and generate a file flag2.jpg.

# dd if=flag.jpg of=flag2.jpg bs=1 skip=221

8. we are done, flag2.jpg is the image file we are looking for. You can open this file and view its content.

### References:

tjctf-2018-writeups: https://github.com/zst123/tjctf-2018-writeups/tree/master/Solved/Ssleepy
Wireshark Decrypting SSL | "Ssleepy" TJCTF 2018: https://www.youtube.com/watch?v=WOb4UrwV72I
