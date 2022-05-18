## Change NIC MAC address with ethtool

### Requirement

In this exercise, we will permanently change the MAC address of your NIC (the NIC of the VM, not the real NIC); specifically, we will change it to *66:66:66:66:66:66*.

### Setup

The provided VM, just one VM.

### Background

The tool *ethtool* can be used to diagnose your ethernet network interface card (NIC). The following command shows some basic information about your NIC.

```console
[05/18/22]seed@VM:~$ sudo ethtool enp0s3 (assume your NIC's name is enp0s3; if it's not, then in all of below commands, always remember to replace the enp0s3 to your corresponding NIC name)
Settings for enp0s3:
	Supported ports: [ TP ]
	Supported link modes:   10baseT/Half 10baseT/Full 
	                        100baseT/Half 100baseT/Full 
	                        1000baseT/Full 
	Supported pause frame use: No
	Supports auto-negotiation: Yes
	Advertised link modes:  10baseT/Half 10baseT/Full 
	                        100baseT/Half 100baseT/Full 
	                        1000baseT/Full 
	Advertised pause frame use: No
	Advertised auto-negotiation: Yes
	Speed: 1000Mb/s
	Duplex: Full
	Port: Twisted Pair
	PHYAD: 0
	Transceiver: internal
	Auto-negotiation: on
	MDI-X: off (auto)
	Supports Wake-on: umbg
	Wake-on: d
	Current message level: 0x00000007 (7)
			       drv probe link
	Link detected: yes
```

The following command shows some basic information about the driver for your NIC.

```console
[05/18/22]seed@VM:~$ sudo ethtool -i enp0s3
driver: e1000
version: 7.3.21-k8-NAPI
firmware-version: 
expansion-rom-version: 
bus-info: 0000:00:03.0
supports-statistics: yes
supports-test: yes
supports-eeprom-access: yes
supports-register-dump: yes
supports-priv-flags: no
```

**Note**: if the above command in your VM shows **supports-eeprom-access: no**, then you can't change the MAC address of your NIC.

### Steps

1. run *ifconfig* to determine your current MAC address. For example,

```console
[05/18/22]seed@VM:~$ ifconfig 
enp0s3    Link encap:Ethernet  HWaddr 08:00:27:73:3b:2f  
          inet addr:192.168.56.111  Bcast:192.168.56.255  Mask:255.255.255.0
          inet6 addr: fe80::4334:97a4:f923:a289/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:72 errors:0 dropped:0 overruns:0 frame:0
          TX packets:150 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:13078 (13.0 KB)  TX bytes:20059 (20.0 KB)
```

This command shows the current MAC address is 08:00:27:73:3b:2f.

2. a component called *EEPROM* stores hardware information about the NIC. And this includes the MAC address. Using ethtool you can read or write the *EEPROM*. For example, the following command reads everything from the *EEPROM*.

```console
[05/18/22]seed@VM:~$ sudo ethtool -e enp0s3
Offset		Values
------		------
0x0000:		08 00 27 73 3b 2f 00 00 ff ff 00 00 00 00 00 00 
0x0010:		00 00 00 00 08 44 1e 00 86 80 0e 10 86 80 40 30 
0x0020:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0030:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0040:		00 00 61 70 0c 28 c8 00 c8 00 00 00 00 00 00 00 
0x0050:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 06 
0x0060:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0070:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 d2 f2 
```

This command shows the first 6 bytes are actually the MAC address (08:00:27:73:3b:2f). Therefore we just need to change the first 6 bytes of the *EEPROM*.

3. Before make any changes to *EEPROM*, we can use the following command to back up the *EEPROM* (in case the *EEPROM* will be damaged - although this is not necessarily important for a VM)

```console
# sudo ethtool -e enp0s3 raw on > eeprom-backup.bin
```

4. A command like below allows us to change the *EEPROM*'s offset at \<OO> to value \<VV>.

```console
# sudo ethtool -E enp03s magic <MM> offset <OO> value <VV>
```

But we need the figure out the magic number, the magic number is the PCI vendor ID (lower 2 bytes) plus the PCI device ID (higher 2 bytes) of your NIC. i.e., magic number = (vendor_id | (device_id << 16)). Use the following command to find out it:

```console
[05/18/22]seed@VM:~$ lspci -nn | grep Ethernet
00:03.0 Ethernet controller [0200]: Intel Corporation 82540EM Gigabit Ethernet Controller [8086:100e] (rev 02)
```

This command shows the the PCI vendor ID is 0x**8086**, the PCI device ID is 0x**100e**. Therefore the magic number for the ethtool is 0x*100e8086*.

5. With the magic number, now we can change our MAC address, which is at offset 0,1,2,3,4,5 of the *EEPROM*. Following commands change our MAC address to *66:66:66:66:66:66*:

```console
[05/18/22]seed@VM:~$ sudo ethtool -E enp0s3 magic 0x100e8086 offset 0x00 value 0x66
[05/18/22]seed@VM:~$ sudo ethtool -E enp0s3 magic 0x100e8086 offset 0x01 value 0x66
[05/18/22]seed@VM:~$ sudo ethtool -E enp0s3 magic 0x100e8086 offset 0x02 value 0x66
[05/18/22]seed@VM:~$ sudo ethtool -E enp0s3 magic 0x100e8086 offset 0x03 value 0x66
[05/18/22]seed@VM:~$ sudo ethtool -E enp0s3 magic 0x100e8086 offset 0x04 value 0x66
[05/18/22]seed@VM:~$ sudo ethtool -E enp0s3 magic 0x100e8086 offset 0x05 value 0x66
```

6. Read *EEPROM* again to verify the change is made correctly.

```console
[05/18/22]seed@VM:~$ sudo ethtool -e enp0s3
Offset		Values
------		------
0x0000:		66 66 66 66 66 66 00 00 ff ff 00 00 00 00 00 00 
0x0010:		00 00 00 00 08 44 1e 00 86 80 0e 10 86 80 40 30 
0x0020:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0030:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0040:		00 00 61 70 0c 28 c8 00 c8 00 00 00 00 00 00 00 
0x0050:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 06 
0x0060:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0070:		00 00 00 00 00 00 00 00 00 00 00 00 00 00 0a 62 
```

As we can see the *EEPROM* is changed.

7. Remove the NIC driver and reinstall it; and then use *ifconfig* to confirm the change is made successfully. (Question: how do I know my driver is e1000? See the output of this command "sudo ethtool -i enp0s3".)

```console
[05/18/22]seed@VM:~$ sudo rmmod e1000
[05/18/22]seed@VM:~$ sudo modprobe e1000
[05/18/22]seed@VM:~$ ifconfig 
enp0s3    Link encap:Ethernet  HWaddr 66:66:66:66:66:66  
          inet addr:192.168.56.111  Bcast:192.168.56.255  Mask:255.255.255.0
          inet6 addr: fe80::b234:35b8:d97b:dccc/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:48 errors:0 dropped:0 overruns:0 frame:0
          TX packets:106 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:6773 (6.7 KB)  TX bytes:14825 (14.8 KB)
```

8 (optional). Generate some network packets (such as ping your default gateway) and use wireshark to capture the packets, and see the new MAC address in wireshark.

### Reference:

How to permanently change a MAC address using ethtool: https://blog.kanbach.org/post/how-to-permanently-change-a-mac-address-using-ethtool/
