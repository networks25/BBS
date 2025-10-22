# Learning tcpdump — a concise practical guide

This document gives step-by-step, copy‑pasta commands and examples so you can run `tcpdump` on your server and capture *only* the traffic you care about (Gemini on port 1965, DNS on 53, a test `nc` chat on a chosen port, etc.). It focuses on filtering to avoid mixing other traffic, and on safe useful viewing/analysis.

> Assumptions: you have `tcpdump` installed on the server, you can run commands with `sudo` (or as root), and you know the IP addresses of the machines involved (server and client). Replace `IFACE`, `SERVER_IP`, `CLIENT_IP`, and ports with your values.

---

## 1. Quick preparation: pick the interface and confirm permissions

1. Find network interfaces `tcpdump` can use:

```bash
sudo tcpdump -D
```

Note the interface index/name you will capture on (e.g. `eth0`, `ens3`, `enp1s0`, `lo` for loopback).

2. If your test traffic is local (you `nc` locally on the server and connect from the same host), use interface `lo` (loopback). For real network traffic use the interface that carries the packets.

3. You usually need root:

```bash
sudo -v     # confirm sudo works
```

---

## 2. Core command form and useful flags

A recommended baseline capture command that shows packets live in a readable way:

```bash
sudo tcpdump -i IFACE -nn -s 0 -A -vvv 'FILTER'  # live, ascii payload, verbose
```

Meaning of flags:

* `-i IFACE` — which interface to listen on.
* `-nn` — don't resolve names (speeds output, shows numeric IPs and ports).
* `-s 0` — capture full packet (important if you want payload).
* `-A` — print packet payload in ASCII (useful for plaintext protocols like plain `nc`, HTTP, DNS text parts).
* `-X` — print hex and ASCII (useful if ASCII is not clean).
* `-vvv` — very verbose (more details about each packet).
* `-w file.pcap` — write binary capture to a file for later analysis with Wireshark/tshark.
* `-r file.pcap` — read a saved capture file.

**Important:** if the traffic is encrypted (TLS on Gemini `1965`), `-A` will show ciphertext — you will not see readable pages. See the TLS note below.

---

## 3. Building filters to show *only* relevant traffic

`tcpdump` uses Berkeley Packet Filter (BPF) syntax. Some common filters:

* Capture only traffic to/from a host:

  * `host 1.2.3.4`
  * `src host 1.2.3.4` or `dst host 1.2.3.4`
* Capture a single port (TCP or UDP):

  * `port 1965`  # matches TCP and UDP port 1965
  * `tcp port 1965` or `udp port 53`
* Combine host + port:

  * `host SERVER_IP and port 1965`
  * `src host CLIENT_IP and dst host SERVER_IP and tcp port 12345`
* Logical operators: `and`, `or`, `not`.

Examples:

```bash
# Only Gemini connections (TCP port 1965) to/from SERVER_IP
sudo tcpdump -i IFACE -nn -s 0 -A 'host SERVER_IP and tcp port 1965'

# Only DNS (UDP or TCP port 53) seen on this interface
sudo tcpdump -i IFACE -nn -s 0 -A 'port 53'

# Only traffic between server and client on a test port (nc)
sudo tcpdump -i IFACE -nn -s 0 -A 'host SERVER_IP and host CLIENT_IP and tcp port 12345'
```

**Tip:** when in doubt, start with `-nn` and simple `host` + `port` filters; that usually keeps unrelated chatter out.

---

## 4. Typical workflows and examples

### A. Watch Gemini (port 1965)

Gemini normally runs on TCP port **1965** and uses TLS. To capture the network packets (you will see IP/TCP and TLS handshake but not decrypted content in general):

```bash
sudo tcpdump -i IFACE -nn -s 0 -X 'host SERVER_IP and tcp port 1965'
```

You will see connection setup (SYN/SYN-ACK/ACK) and TLS records (`TLSv1.2`/`TLSv1.3` sizes) but not the page contents because TLS encrypts the application data. If you want to *see* the Gemini payload you must capture before the TLS layer (for example: run a plain test server without TLS) or analyze server logs. Modern TLS (ECDHE) prevents passive decryption with just the server private key.

### B. Watch DNS queries (UDP 53 and sometimes TCP 53)

```bash
sudo tcpdump -i IFACE -nn -s 0 -A 'udp port 53 or tcp port 53'
```

DNS queries are mostly plaintext; `-A` will show the query name in the payload. To filter only queries that target your server:

```bash
sudo tcpdump -i IFACE -nn -s 0 -A 'dst host SERVER_IP and (udp port 53 or tcp port 53)'
```

### C. Test chat using `nc` and capture the conversation

On your server, start a listening netcat (replace port `12345` with your choice):

```bash
# server: listen and print what client sends
nc -l 12345
```

On the client machine, connect and type messages:

```bash
nc SERVER_IP 12345
```

Capture only that conversation from the server side (loopback or real interface):

```bash
# If client is remote
sudo tcpdump -i IFACE -nn -s 0 -A 'host SERVER_IP and host CLIENT_IP and tcp port 12345'

# If both ends are on the server (loopback), capture on lo
sudo tcpdump -i lo -nn -s 0 -A 'tcp port 12345'
```

With `-A` you should see the ASCII you typed in the `nc` session in the tcpdump output.

### D. Save capture to file and analyze later

```bash
sudo tcpdump -i IFACE -nn -s 0 -w /tmp/mycap.pcap 'host SERVER_IP and tcp port 12345'
# later, on your desktop with Wireshark/tshark or on server:
sudo tcpdump -r /tmp/mycap.pcap -nn -A
```

`-w` writes the pcap (binary) which is the preferred method for longer captures and for using Wireshark.

---

## 5. Interpreting `tcpdump` output — the important fields

A typical `tcpdump -nn -vvv` line looks like:

```
12:34:56.789012 IP (tos 0x0, ttl 64, id 12345, proto TCP (6), length 60)
    10.0.0.2.45678 > 10.0.0.1.1965: Flags [S], seq 123456789, win 64240, options [mss 1460,sackOK,TS val 123456 ecr 0,nop,wscale 7], length 0
```

Key parts:

* Timestamp (`12:34:56.789012`)
* Protocol and meta (IP, ttl, length)
* Source and destination (`10.0.0.2.45678 > 10.0.0.1.1965`) — IP.port
* TCP flags (`[S]` = SYN, `[.]` data ack, `[P.]` push+ack, `[F.]` FIN+ACK)
* `seq` and `ack` numbers and `length` of payload

Payload printed by `-A` follows as ASCII lines below packet headers.

---

## 6. Useful targeted filters (cheat sheet)

```bash
# Only incoming connections to your server on a port
sudo tcpdump -i IFACE -nn -s 0 -A 'dst host SERVER_IP and tcp port 12345'

# Only packets from a client to server
sudo tcpdump -i IFACE -nn -s 0 -A 'src host CLIENT_IP and dst host SERVER_IP and tcp port 12345'

# Only show TCP SYN packets (new connection attempts)
sudo tcpdump -i IFACE -nn 'tcp[tcpflags] & tcp-syn != 0 and tcp[tcpflags] & tcp-ack == 0 and dst host SERVER_IP and tcp port 12345'

# Capture N packets then exit
sudo tcpdump -i IFACE -nn -c 100 -s 0 -A 'host SERVER_IP and tcp port 12345'
```

---

## 7. About encrypted protocols (TLS / Gemini note)

* Gemini uses TLS on port 1965. That means you will see IP/TCP/TLS record sizes but **not** the page content when you capture passively with `tcpdump`.
* To inspect the actual Gemini page you either:

  * Run a non‑TLS test server (e.g., `nc -l`), or
  * Use server application logs (service logs), or
  * Use a TLS debugging setup where you terminate TLS before the application (reverse proxy that logs), or
  * Use Wireshark with pre‑master secrets saved (only possible if you control the client and export TLS keys or use old non‑ECDHE ciphers).

In short: for Gemini you will mostly verify connections, TLS handshakes and timing with `tcpdump`, not the page body.

---

## 8. When tcpdump doesn't show what you expect

* If you see no packets, check:

  * Are you capturing on the correct interface? `tcpdump -D`
  * Is the firewall blocking or NAT rewriting traffic?
  * Are you filtering too narrowly? Try removing parts of the filter.
* If payload is unreadable, likely it's encrypted or truncated — use `-s 0` to avoid truncation.
* If capturing local (same host) traffic and you use the physical interface, you might miss `lo` traffic — capture on `lo` if both ends are local.

---

## 9. Advanced tips (optional)

* Use `tcpdump -w` and open the `.pcap` in Wireshark for GUI analysis and `Follow TCP Stream`.
* `tshark` (command line Wireshark) can follow TCP streams and extract printable payloads:

```bash
tshark -r /tmp/mycap.pcap -q -z follow,tcp,ascii,0
```

* `tcpflow` stores full TCP flows in separate files (handy for reconstructing conversations):

```bash
sudo tcpflow -i IFACE 'host SERVER_IP and tcp port 12345'
```

* For long captures, rotate output files by size or time with `-C` (size) or `-G` (seconds) and `-w file-%Y%m%d%H%M.pcap`.

---

## 10. Example: end-to-end test (nc chat)

1. On server: start netcat *and* start tcpdump in another terminal:

```bash
# terminal A (server): listen
nc -l 12345

# terminal B (server): capture conversation (if client remote)
sudo tcpdump -i IFACE -nn -s 0 -A 'host SERVER_IP and host CLIENT_IP and tcp port 12345'

# If client is on same server (loopback):
sudo tcpdump -i lo -nn -s 0 -A 'tcp port 12345'
```

2. On client: connect and type messages:

```bash
nc SERVER_IP 12345
```

3. Watch the `tcpdump` terminal — you should see the ASCII you type.

---

## 11. References and further reading

* `man tcpdump` and `man pcap-filter` (BPF syntax) are the canonical references.
* `tcpdump -D` to list interfaces.
* Wireshark/Tshark documentation for deeper packet analysis.

---

tcpdump -i eth0 -nn -s 0 -A 'host 37.252.77.193 and tcp port 1965'


# tcpdumping nc

tcpdump -i eth0 -nn -q 'host 37.252.77.193 and tcp port 1965'

or

tcpdump -i eth0 -nn -t 'host 37.252.77.193 and tcp port 12345'

 nc -l -p 12345

 tcpdump -i eth0 -nn -s 0 -A 'host 37.252.77.193 and tcp port 12345'

 on client

 nc 37.252.77.193 12345



# seq

seq — TCP sequence numbers

TCP is a byte stream protocol. Sequence numbers count bytes, not packets.

Each side chooses a random 32-bit Initial Sequence Number (ISN) when opening a connection.

Client picked 977328230.

Server picked 306085128.

Every byte of data sent increases the sequence number by 1.

Example: if the client sends 100 bytes, next packet will show seq 977328231 and then increase up to 977328330.

Why random? For security (makes guessing harder) and to avoid confusion with old, delayed packets.


# ack

ack — Acknowledgment numbers

ack tells the other side “I’ve successfully received all bytes up to N-1, and I’m expecting byte number N next.”

Example: Server’s SYN had seq 306085128 and length 0. Client replies with ack 306085129, meaning “I got your SYN, I expect the next byte to be 306085129.”


# win — TCP receive window

win (window size) is the number of bytes this side is currently willing to accept into its receive buffer.

It’s a flow-control mechanism: the sender must not exceed this advertised window.

Example:

Client advertises win 64240 (raw value). That means “I can receive up to 64 KB of data right now.”

Server advertises win 31856 (~31 KB).

Client’s third packet shows win 502 — but that’s the unscaled raw field. With the negotiated window scale option (wscale 7), the actual window is 502 × 2^7 = 502 × 128 ≈ 64,256 bytes.
→ So really, client can still accept ~64 KB.

So, win is not how many bytes are pending, it’s the advertised capacity. If it ever drops to 0, it means “stop sending, my buffer is full.”

# mss, sackOK, TS, wscale

These are TCP options exchanged during SYN:

mss 1460 = Maximum Segment Size (largest chunk of data it wants in one TCP segment).

sackOK = Selective Acknowledgment permitted (allows more efficient recovery from packet loss).

TS val ... ecr ... = Timestamps for RTT measurement and PAWS (Protection Against Wrapped Sequence numbers).

wscale 7 = Window scaling factor (multiply window by 2^7).

reminder: RTT is the time you get response to your packet.



for seeing bytes

tcpdump -i eth0 -nn -s 0 -A 'host 37.252.77.193 and tcp port 1965'

tcpdump -i eth0 -nn -s 0 -X 'host 37.252.77.193 and tcp port 12345'


nc -l -p 12345
nc 37.252.77.193 12345




same on port 1965

```
02:56:40.806247 IP 5.77.200.249.29656 > 37.252.77.193.1965: Flags [P.], seq 1:518, ack 1, win 502, options [nop,nop,TS val 749952261 ecr 3079416183], length 517
        0x0000:  4500 0239 747d 4000 3e06 843e 054d c8f9  E..9t}@.>..>.M..
        0x0010:  25fc 4dc1 73d8 07ad db1e 53ba 7740 a3b2  %.M.s.....S.w@..
        0x0020:  8018 01f6 fd24 0000 0101 080a 2cb3 5d05  .....$......,.].
        0x0030:  b78c 2977 1603 0102 0001 0001 fc03 0375  ..)w...........u
        0x0040:  4624 40a7 f86b 0608 180c bd85 340b 971c  F$@..k......4...
        0x0050:  ac50 1dde e109 b0f9 594f 7be0 c709 3d20  .P......YO{...=.
        0x0060:  48d6 64a7 e7c9 fb72 58b6 b6e4 c33a 166e  H.d....rX....:.n
        0x0070:  1e70 79e1 eb16 e914 9e49 f05f 0f99 8e30  .py......I._...0
        0x0080:  0014 1302 1303 1301 c02c cca9 c02b c030  .........,...+.0
        0x0090:  cca8 c02f 009f 0100 019f ff01 0001 0000  .../............
        0x00a0:  0000 0e00 0c00 0009 6e6f 7261 7972 2e61  ........norayr.a
        0x00b0:  6d00 0b00 0403 0001 0200 0a00 1600 1400  m...............
        0x00c0:  1d00 1700 1e00 1900 1801 0001 0101 0201  ................
        0x00d0:  0301 0400 2300 0000 1600 0000 1700 0000  ....#...........
        0x00e0:  0d00 3000 2e04 0305 0306 0308 0708 0808  ..0.............
        0x00f0:  1a08 1b08 1c08 0908 0a08 0b08 0408 0508  ................
        0x0100:  0604 0105 0106 0103 0303 0103 0204 0205  ................
        0x0110:  0206 0200 2b00 0504 0304 0303 002d 0002  ....+........-..
        0x0120:  0101 0033 0026 0024 001d 0020 34ef 6382  ...3.&.$....4.c.
```






That is the SNI (Server Name Indication) inside the TLS handshake.

It’s part of the TLS extension where the client says: “I want to talk to host norayr.am.”

This is not a Gemini protocol thing, it’s part of TLS.

Every modern TLS client sends SNI so servers can host multiple domains on one IP.

That’s why you see your domain in cleartext before encryption starts.
