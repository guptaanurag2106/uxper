# DNS

Implementing DNS building, receiving, parsing for A records

Currently calls A, AAAA record in parllel
Follows CNAME records

## Context
- [DNS](https://en.wikipedia.org/wiki/Domain_Name_System)
- [RFC](https://datatracker.ietf.org/doc/html/rfc1035)

To get a pcap dump of dns requests
```bash
sudo tcpdump -w dns.pcap -i wlp0s20f3 port 53 & // start tcpdump in background
dig @8.8.8.8 google.com A  // ask 8.8.8.8 for google.com A records
fg // bring tcpdump to foreground
<C-c> // close tcpdump to write to dns.pcap
```
Use wireshark or https://app.packetsafari.com for reading pcap

## Build & Run

```bash
go run main.go google.com 1.1.1.1

### Response:
Asking 1.1.1.1 for A records of www.google.com.
Response:
transaction_id: 53921
domains: [www.google.com.]
flags: authoritative=false truncated=false rcode=0(NOERROR)
answers:
  1) ip=142.251.153.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  2) ip=142.251.150.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  3) ip=142.251.151.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  4) ip=142.251.154.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  5) ip=142.251.155.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  6) ip=142.251.152.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  7) ip=142.251.156.119 type=1(A) class=1(IN) ttl=300ms data_len=4
  8) ip=142.251.157.119 type=1(A) class=1(IN) ttl=300ms data_len=4
```
