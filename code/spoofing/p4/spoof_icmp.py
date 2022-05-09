#!/usr/bin/python
from scapy.all import *
print("SENDING SPOOFED ICMP PACKETS...")
IPLayer = IP()
IPLayer.src = "2.3.4.5"
IPLayer.dst = "192.168.243.129"
ICMPpkt = ICMP()
pkt = IPLayer/ICMPpkt
pkt.show()
send(pkt, verbose=0)
