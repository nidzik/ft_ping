# ft_ping

Send ICMP ECHO_REQUEST packets to network hosts.

# Synopsis
usage : ft_ping [-fmtci] <hostname>
```
	-f			Flood ping.  Outputs packets as fast as they come back.
	-m ttl		Set the IP Time To Live for outgoing packets.
	-t timeout	Specify a timeout, in seconds, before ping exits regardless of
				how many packets have been received.
	-c count 	Stop after sending (and receiving) count ECHO_RESPONSE packets.
	-i wait		Wait wait seconds between sending each packet.	
```

# Result
```
$ sudo ./ft_ping localhost
PING localhost 56(84) bytes of data.
64 bytes from 127.0.0.1 icmp_seq=1 ttl=64 time=0.246 ms
64 bytes from 127.0.0.1 icmp_seq=2 ttl=64 time=0.146 ms
64 bytes from 127.0.0.1 icmp_seq=3 ttl=64 time=0.146 ms
64 bytes from 127.0.0.1 icmp_seq=4 ttl=64 time=0.144 ms
64 bytes from 127.0.0.1 icmp_seq=5 ttl=64 time=0.149 ms
^C
--- localhost ping statistics ---
5 packets transmitted, 5 reveived, 0% packet loss, time 4459ms
rtt min/avg/max/stddev = 0.144/0.166/0.246/0.102 ms
```