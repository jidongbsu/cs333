#!/usr/bin/python
from scapy.all import *

print("SNIFFING PACKETS...")

def print_pkt(pkt):
        pkt.show()

pkt = sniff(filter='tcp and (src host clientip and dst port 23)', prn=print_pkt)

