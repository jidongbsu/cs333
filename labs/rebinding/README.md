## DNS Rebinding Attack

### Requirement

In this lab, you will demonstrate the DNS rebinding attack. The goal of the attacker is, whenever the victim visits www.attacker32.com, the victim's smart thermostat's temperature will be changed to 88 Celsius degree.

### Setup

3 Linux VMs: Victim Client - acting as 2 roles, a web client, and an IoT server (www.seediot32.com), a Local DNS Server, Attacker - acting as 2 roles, a malicious web server (www.attacker32.com), and a malicious DNS serve (which is responsible for the attacker32.com domain).

The following is the IP addresses for the VMs used in this README.

| VM  |  IP Address   |                          Role                                         |
|-----|---------------|-----------------------------------------------------------------------|
| VM1 | 172.16.77.128 |   victim client, also runs an IoT server                              |
| VM2 | 172.16.77.129 |   victim server, acts as a local DNS server                           |
| VM3 | 172.16.77.130 |   attacker, runs a malicious web server, and a malicious DNS server   |

**Warning**: this is the hardest lab, and if you missed the in-class discussion, you are expected to get lost in this lab; but you may still be able to complete the lab by just following the instructions, you just won't really understand why you are doing each step.

### Steps

1. setting up the client.

1.1. reduce Firefox’s DNS caching time: Type about:config in the URL field, and then change network.dnsCacheExpiration from 60 to 10.
 
![alt text](images/lab-rebinding-firefox-setting-p1.png "change cache expiration time")
![alt text](images/lab-rebinding-firefox-setting-p2.png "change cache expiration time")
![alt text](images/lab-rebinding-firefox-setting-p3.png "change cache expiration time")
![alt text](images/lab-rebinding-firefox-setting-p4.png "change cache expiration time")
![alt text](images/lab-rebinding-firefox-setting-p5.png "change cache expiration time")
![alt text](images/lab-rebinding-firefox-setting-p6.png "change cache expiration time")

1.2. change /etc/hosts: add the following entry in the file:

CLIENT_VM_IP	www.seediot32.com (note: change CLIENT_VM_IP to the Client VM's IP address)

this screenshot shows editing the file in *vi*:
![alt text](images/lab-rebinding-change-hosts.png "change /etc/hosts")

this screenshot shows the file is now edited:
![alt text](images/lab-rebinding-change-hosts-done.png "change /etc/hosts")

1.3. configure DNS server information, i.e., let the client know the IP address of the DNS server.

1.3.1. add this line to the end of file /etc/resolvconf/resolv.conf.d/head
nameserver DNS_SERVER_IP (change DNS_SERVER_IP to the local DNS server's IP address)

this screenshot shows editing the file in *vi*:
![alt text](images/lab-rebinding-edit-file.png "edit the file")

this screenshot shows the file is now edited:
![alt text](images/lab-rebinding-configure-dns.png "configure dns")

1.3.2. run the following command so the above change will take effect:

```console
$ sudo resolvconf -u
```

![alt text](images/lab-rebinding-resolvconf.png "resolvconf command")

2. still setting up the victim client, but this time setting up the IoT server role.

2.1. install a web framework called Flask.

```console
$ sudo pip3 install Flask==1.1.1
```

![alt text](images/lab-rebinding-victim-web-server.png "command to install Flask")
![alt text](images/lab-rebinding-victim-web-server-installing.png "command to install Flask")

2.2. download the IoT server code: http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/user_vm.zip

```console
$ wget http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/user_vm.zip
```

2.3. then start the IoT server:

```console
$ unzip user_vm.zip
$ cd user_vm
$ sudo ./start_iot.sh
```

The above script will start a web server and listen on port 8080.

![alt text](images/lab-rebinding-start-iot.png "command to start iot web server")
![alt text](images/lab-rebinding-iot-started.png "iot server is started")

2.4. test the IoT server:

http://www.seediot32.com:8080 (access this from the firefox browser)

![alt text](images/lab-rebinding-test-iot-p1.png "test iot server")
![alt text](images/lab-rebinding-test-iot-p2.png "test iot server success")

3. on the attacker VM, set up the attacker web server:

3.1. install a web framework called Flask.

```console
$ sudo pip3 install Flask==1.1.1
```

3.2. download the attacker web server code: http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/attacker_vm.zip

```console
$ wget http://cs.boisestate.edu/~jxiao/cs333/code/rebinding/attacker_vm.zip
```

3.3. then start the attacker web server.

```console
$ unzip attacker_vm.zip
$ cd attacker_vm
$ sudo ./start_webserver.sh
```

The above script will start a web server and listen on port 8080.

![alt text](images/lab-rebinding-start-attacker-web-server.png "command to start attacker web server")
![alt text](images/lab-rebinding-attacker-web-server-started.png "attacker web server is started")

3.4. test the attacker web server:

http://localhost:8080 (access this from the firefox browser)

![alt text](images/lab-rebinding-test-attacker-web-p1.png "test attacker web server")
![alt text](images/lab-rebinding-test-attacker-web-p2.png "test attacker web server success")

4. open a new terminal window on the attacker VM, and set up the attacker DNS server:

4.1. the above attacker_vm folder contains a DNS configuration file called attacker32.com.zone, copy this file into /etc/bind. In this file, change 10.0.2.8 to the attacker VM's IP address, and change the TTL (which is the first entry in this file) from 10000 to 10, i.e., records in the cache expire in 10 seconds.

before change:
![alt text](images/lab-rebinding-attacker-DNS-server-before-change.png "test attacker DNS server")

after change:
![alt text](images/lab-rebinding-attacker-DNS-server-after-change.png "attacker DNS server done")

4.2. add the following into /etc/bind/named.conf (so that the above configuration file will be used):

```console
zone "attacker32.com" {
	type master;
	file "/etc/bind/attacker32.com.zone";
};
```

![alt text](images/lab-rebinding-attacker-adding-zone.png "change zone file")

4.3. restart DNS server so the above changes will take effect:

```console
$ sudo service bind9 restart
```

5. setting up the local DNS server (so that we don't need to actually purchase the domain attacker32.com).

5.1. add the following into /etc/bind/named.conf (so that it forwards all requests for the attacker32.com domain to the malicious DNS server).

```console
zone "attacker32.com" {
	type forward;
	forwarders {
	    172.16.77.130; // replace 172.16.77.130 with your attacker VM's IP address, do not remove the ";"
	};
};
```

![alt text](images/lab-rebinding-setting-local-DNS-server.png "setting local DNS server")

5.2. restart DNS server so the above changes will take effect:

```console
$ sudo service bind9 restart
```

6: at this point, from the victim client's browser, you should be able to access these 3 URLs (open 3 tabs to access these 3 URLs, and leave these tabs open):

URL 1: http://www.seediot32.com:8080

URL 2: http://www.seediot32.com:8080/change

![alt text](images/lab-rebinding-victim-iot-change.png "test iot server")

URL 3: http://www.attacker32.com:8080/change

![alt text](images/lab-rebinding-attacker-change.png "test attacker server")

However, you can change the temperature from URL 2, but not URL 3, even though they run the exact same code. (Reason? SOP)

7. launch the attack:

7.1. on the attacker VM, change the javascript code on the attacker VM. It is still in the attacker_vm folder, and it's this file: attacker_vm/rebind_malware/templates/js/change.js.

change this line:

let url_prefix = ’http://www.seediot32.com:8080’

to this line:

let url_prefix = ’http://www.attacker32.com:8080’

the screenshots show the change. before change:
![alt text](images/lab-rebinding-js-before-change.png "before changing the javascript")

after change:
![alt text](images/lab-rebinding-js-after-change.png "after changing the javascript")

7.2. on the attacker VM, restart the malicious web server: press ctrl-c to stop the script start_webserver.sh (see Step 3.3), and then re-run the script.

this screenshot shows we press ctrl-c to stop the attacker web server:
![alt text](images/lab-rebinding-stop-attacker-web-server.png "ctrl-c to stop attacker web server")

this screenshot shows we then run the same script again to re-start the attacker web server:
![alt text](images/lab-rebinding-restart-attacker-web-server.png "run script to re-start attacker web server")

7.3. on the victim client VM, let the victim client access web page: http://www.attacker32.com:8080/. You should be able to see a page with a timer, which goes down from 10 to 0. Once it reaches 0, the JavaScript code on this page will send the set-temperature request to http://www.attacker32.com:8080, and then reset the timer value to 10. 

7.4. on the attacker VM, open this file: /etc/bind/attacker32.com.zone, when the count down timer is counting from 10 to 0, change the A record for www.attacker32.com, so that it points to the IoT server's IP address.

Before change:
www IN A Attacker_VM_IP

![alt text](images/lab-rebinding-before-rebinding.png "launch attack: before rebinding")

After change:
www IN A IoT_VM_IP (remember IoT VM is also the victim client VM)

![alt text](images/lab-rebinding-after-rebinding.png "launch attack: after rebinding")

7.5. on the attacker VM, reload the configuration file so the change will take effect.

```console
$ sudo rndc reload attacker32.com
```

7.6. on the victim client VM, go back to the firefox browser and see if the temperature, within 10 seconds, is changed to 88 Celsius degree (check the URL1 page). If so, then the attack is successful.

the screenshots here show the attack is successful. the tab which shows access to www.attacker32.com:
![alt text](images/lab-rebinding-attack-success-p1.png "attack success: tab 2")

the tab which shows access to the thermometer:
![alt text](images/lab-rebinding-attack-success-p2.png "attack success: tab 1")

8. You are recommended to remove the line you added in step 1.3, in this file: /etc/resolvconf/resolv.conf.d/head on the victim client VM, so that your future experiments won't be affected.
