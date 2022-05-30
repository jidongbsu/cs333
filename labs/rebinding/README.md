## DNS Rebinding Attack

### Requirement

In this lab, you will demonstrate the DNS rebinding attack. The goal of the attacker is, whenever the victim visits www.attacker32.com, the victim's smart thermostat's temperature will be changed to 88 Celsius degree.

### Setup

3 Linux VMs: Victim Client - acting as 2 roles, a web client, and an IoT server (www.seediot32.com), a Local DNS Server, Attacker - acting as 2 roles, a malicious web server (www.attacker32.com), and a malicious DNS serve (which is responsible for the attacker32.com domain).

**Warning**: this is the hardest lab, and if you missed the in-class discussion, you are expected to get lost in this lab; but you may still be able to complete the lab by just following the instructions, you just won't really understand why you are doing each step.

### Steps

Step 1: setting up the client.

Step 1.1. reduce Firefox’s DNS caching time: Type about:config in the URL field, and then change network.dnsCacheExpiration from 60 to 10.

Step 1.2. change /etc/hosts: add the following entry in the file:

CLIENT_VM_IP	www.seediot32.com (note: change CLIENT_VM_IP to the Client VM's IP address)

Step 1.3: configure DNS server information, i.e., let the client know the IP address of the DNS server.
Step 1.3.1: add this line to the end of file /etc/resolvconf/resolv.conf.d/head
nameserver DNS_SERVER_IP (change DNS_SERVER_IP to the local DNS server's IP address)
Step 1.3.2: run the following command so the above change will take effect:

```console
$ sudo resolvconf -u
```

Step 2: still setting up the client, but this time setting up the IoT server role.

Step 2.1. install a web framework called Flask.

```console
$ sudo pip3 install Flask==1.1.1
```

Step 2.2. download the IoT server code: http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/user_vm.zip

```console
$ wget http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/user_vm.zip
```

Step 2.3. then start the IoT server:

```console
$ unzip user_vm.zip
$ cd user_vm
$ sudo ./start_iot.sh
```

The above script will start a web server and listen on port 8080.

Step 2.4. test the IoT server:

http://www.seediot32.com:8080 (access this from the firefox browser)

Step 3. setting up the attacker web server.

Step 3.1. install a web framework called Flask.

```console
$ sudo pip3 install Flask==1.1.1
```

Step 3.2. download the attacker web server code: http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/attacker_vm.zip

```console
$ wget http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/attacker_vm.zip
```

Step 3.3. then start the attacker web server.

```console
$ unzip attacker_vm.zip
$ cd attacker_vm
$ sudo ./start_webserver.sh
```

The above script will start a web server and listen on port 8080.

Step 3.4. test the attacker web server:

http://localhost:8080 (access this from the firefox browser)

Step 4. setting up the attacker DNS server.

Step 4.1. the above attacker_vm folder contains a DNS configuration file called attacker32.com.zone, copy this file into /etc/bind. In this file, change 10.0.2.8 to the attacker VM's IP address, and change the TTL (which is the first entry in this file) from 10000 to 10, i.e., records in the cache expire in 10 seconds.

Step 4.2. add the following into /etc/bind/named.conf (so that the above configuration file will be used):

```console
zone "attacker32.com" {
	type master;
	file "/etc/bind/attacker32.com.zone";
};
```

Step 4.3. restart DNS server so the above changes will take effect:

```console
$ sudo service bind9 restart
```

Step 5. setting up the local DNS server (so that we don't need to actually purchase the domain attacker32.com).

Step 5.1. add the following into /etc/bind/named.conf (so that it forwards all requests for the attacker32.com domain to the malicious DNS server).

```console
zone "attacker32.com" {
	type forward;
	forwarders { 10.0.2.8; }; (replace 10.0.2.8 with your attacker VM's IP address, do not remove the ";".)
};
```

Step 5.2. restart DNS server so the above changes will take effect:

```console
$ sudo service bind9 restart
```

Step 6: at this point, from the client browser, you should be able to access these 3 URLs (open 3 tabs to access these 3 URLs, and leave these tabs open):

URL 1: http://www.seediot32.com:8080

URL 2: http://www.seediot32.com:8080/change

URL 3: http://www.attacker32.com:8080/change

However, you can change the temperature from URL 2, but not URL 3, even though they run the exact same code. (Reason? SOP)

Step 7. launch the attack:

Step 7.1. change the javascript code on the attacker VM. It is still in the attacker_vm folder, and it's this file: attacker_vm/rebind_malware/templates/js/change.js.

Change this line:

let url_prefix = ’http://www.seediot32.com:8080’

to this line:

let url_prefix = ’http://www.attacker32.com:8080’

Step 7.2. restart the malicious web server: press ctrl-c to stop the script start_webserver.sh (see Step 3.3), and then re-run the script.

Step 7.3. let the victim client access web page: http://www.attacker32.com:8080/. You should be able to see a page with a timer, which goes down from 10 to 0. Once it reaches 0, the JavaScript code on this page will send the set-temperature request to http://www.attacker32.com:8080, and then reset the timer value to 10. 

Step 7.4. on the attacker VM, open this file: /etc/bind/attacker32.com.zone, when the count down timer is counting from 10 to 0, change the A record for www.attacker32.com, so that it points to the IoT server's IP address.

Before change:
www IN A Attacker_VM_IP

After change:
www IN A IoT_VM_IP (remember IoT VM is also the victim client VM)

Step 7.5. reload the configuration file so the change will take effect.

```console
$ sudo rndc reload attacker32.com
```

Step 7.6. go back to the victim client browser and see if the temperature is changed (check the URL1 page). If so, then the attack is successful.

Step 8. You are recommended to remove the line you added in step 1.3, in this file: /etc/resolvconf/resolv.conf.d/head, so that your future experiments won't be affected.
