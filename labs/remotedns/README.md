## Remote DNS Cache Poisoning Attack

### Requirement

In this lab, you will poison the cache of a local DNS server, and thus affect clients who rely on this DNS server. More specifically, we want clients who access www.cnn.com go to fakenews.com.

### Setup

3 Linux VMs: Victim (DNS) Client, Victim (DNS) Server, Attacker. All 3 VMs are located in the same network - however, we are not allowed to exploit this fact for the attacks, in other words, the attacker in this lab is not allowed to use wireshark to sniff any packets, or use the *netwox* command to inject forged DNS responses. After all, this lab is trying to demonstrate how attackers from a remote network can still perform DNS cache poisoning.

The following is the IP addresses for the VMs used in this README.

| VM  |  IP Address   |              Role                     |
|-----|---------------|---------------------------------------|
| VM1 | 172.16.77.128 |   victim DNS client                   |
| VM2 | 172.16.77.129 |   victim local DNS server             |
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

2. setting up the local DNS server

2.1. add the following into /etc/bind/named.conf (so that it forwards all requests for the attacker32.com domain to the malicious DNS server).

```console
zone "attacker32.com" {
    type forward;
    forwarders {
        172.16.77.130; // replace 172.16.77.130 with your attacker VM's IP address, do not remove the ";".
    };
};
```

**Explanation**: the added lines are saying, for DNS inquiries regarding this attacker32.com domain, please forward the inquiries to 172.16.77.130. we do this so that we don't need to actually purchase the domain attacker32.com.

2.2. copy named.conf.options into /etc/bind/ directory, and also copy named.conf.default-zones into /etc/bind/ directory.

```console
[05/29/22]seed@VM:~/.../remotedns$ sudo cp named.conf.default-zones /etc/bind/
[05/29/22]seed@VM:~/.../remotedns$ sudo cp named.conf.options /etc/bind/
```

**Explanation**: these two files will overwrite the default files, and with these two files, now the victim local DNS server is configured to forward all DNS requests to 1.2.3.4, which does not provide DNS services, and then later on the attacker will impersonate 1.2.3.4 to send forged responses to the victim local DNS server.

2.3. restart DNS server so the above changes will take effect:

```console
$ sudo service bind9 restart
```

3. setting up the attacker DNS server.

3.1. copy attacker32.com.zone and cnn.com.zone into /etc/bind/ directory.

3.2. add the following into /etc/bind/named.conf (so that the above two configuration files will be used):

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

3.3. restart attacker's DNS server so the above changes will take effect:

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

5. launch the attack: on the attacker's VM,

```console
[05/29/22]seed@VM:~/.../remotedns$ gcc -o attack attack.c
[05/29/22]seed@VM:~/.../remotedns$ sudo ./attack 172.16.77.130 172.16.77.129
```

**Note**: replace 172.16.77.130 with your attacker VM's IP address, replace 172.16.77.129 with your victim server VM's IP address.

6. the attack may take a couple of minutes. on victim DNS server VM, we can check the cache to verify if the cache is poisoned or not.

```console
[05/29/22]seed@VM:~$ sudo rndc dumpdb -cache
[05/29/22]seed@VM:~$ cat /var/cache/bind/dump.db | grep attacker
ns.attacker32.com.	9992	\-AAAA	;-$NXRRSET
; attacker32.com. SOA ns.attacker32.com. admin.attacker32.com. 2008111001 28800 7200 2419200 86400
cnn.com.		65529	NS	ns.attacker32.com.
; ns.attacker32.com [v4 TTL 1792] [v6 TTL 9992] [v4 success] [v6 nxrrset]
```

as long as we see this NS record which associates cnn.com. to ns.attacker32.com., then we know the cache is now poisoned.

7. we can then verify the result from victim client. on the victim client VM, we just need to send a query.

```console
# dig www.cnn.com 
```

```console
# dig @ns.attacker32.com www.cnn.com
```

these two commands should now show the same result, which is www.cnn.com is mapped to 188.126.71.216, which is the IP address of fakenews.com.

7. you are recommended to remove the line you added on the victim client VM in step 3, in this file: /etc/resolvconf/resolv.conf.d/head, so that your future experiments won't be affected.

8. you are also recommended to restore the two files on the victim server VM:

```console
[05/29/22]seed@VM:~/.../remotedns$ sudo cp named.conf.default-zones.orig /etc/bind/
[05/29/22]seed@VM:~/.../remotedns$ sudo cp named.conf.options.orig /etc/bind/
```
