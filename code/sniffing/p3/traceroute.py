#!/usr/bin/python
from scapy.all import *

host = sys.argv[1]
print("Traceroute " + host)
ttl = 1
while 1:
        IPLayer = IP()
        IPLayer.dst = host
        IPLayer.ttl = ttl
        ICMPpkt = ICMP()
        pkt = IPLayer/ICMPpkt

        # sends packets and waits for first answer
        replypkt = sr1(pkt, verbose=0)
        if replypkt is None:
                break
        elif replypkt[ICMP].type == 0:
                print "%d hops away: " %ttl,replypkt[IP].src
                print "Done", replypkt[IP].src
                break
        else:
                print "%d hops away: " %ttl,replypkt[IP].src
                ttl+=1


