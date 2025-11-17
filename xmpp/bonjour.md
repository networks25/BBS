# Introduction to Bonjour (Serverless XMPP via mDNS)

This document introduces **Bonjour messaging** (XEP-0174), also known as
**serverless XMPP** or **link-local messaging**.
It explains how peers discover each other using **mDNS**, how they build a
**temporary roster**, and how they exchange **XMPP-like XML stanzas** directly
over TCP — without any server.

Inside this single markdown file:

- All code examples use **tildes `~~~` fenced blocks** (safe for ChatGPT UI).
- The entire document is **one uninterrupted markdown block**.

---

# 1. What Is Bonjour Messaging?

Bonjour messaging is a **peer-to-peer variant of XMPP** designed for local
networks. Unlike traditional XMPP:

- **No server exists.**
- All clients **advertise themselves** via mDNS/DNS-SD.
- All clients **listen for TCP connections** and **connect directly** to peers.
- The roster (contact list) is **temporary** and derived from active mDNS
  records.

This makes Bonjour excellent for:

- LAN chat in classrooms
- Ad-hoc communication between local devices
- Environments without an XMPP server

---

# 2. Discovery Layer: mDNS + DNS-SD

Every Bonjour/XEP-0174 client advertises a DNS-SD service:

    _presence._tcp.local

This advertisement includes:

- hostname (e.g. `alice-laptop.local`)
- TCP port (e.g. 5298)
- TXT fields with metadata:
  - nickname
  - status
  - optional “JID”
  - capabilities

### Example mDNS TXT Record

~~~txt
fullname=Alice
status=chat
email=alice@example.com
node=BonjourClient
version=1.0
~~~

Clients continuously browse for `_presence._tcp.local`.
Every discovered service becomes one roster entry.

---

# 3. When to Add or Remove Contacts

### Add to roster when:
- You detect a new `_presence._tcp.local` service.
- You resolve its hostname/IP and port.
- You have enough metadata to display a user.

### Remove from roster when:
- The mDNS advertisement disappears.
- mDNS times out (peer unplugged, Wi-Fi off, laptop asleep).
- The peer explicitly closes its Bonjour session.

The roster is **ephemeral** and reflects **current LAN presence only**.

---

# 4. Starting a Peer-to-Peer Connection

Once two peers discover each other:

1. Both clients **listen** on an advertised TCP port.
2. When Alice wants to talk to Bob:
   - Alice opens a TCP connection to Bob’s IP:port.
3. They exchange a **simplified XMPP stream**.

---

# 5. Opening an XMPP Stream (Peer ↔ Peer)

### Alice → Bob (stream start)

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="alice@local"
    to="bob@local"
    version="1.0">
~~~

### Bob → Alice (stream response)

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="bob@local"
    to="alice@local"
    version="1.0">
~~~

After this, normal stanza flow begins (messages, presence, iq).

---

# 6. Presence Stanzas in Bonjour

Presence informs other peers of your live state.
Unlike servered XMPP, presence does **not propagate globally**, only directly
between peers that have opened streams.

### Available

~~~xml
<presence from="alice@local">
  <show>chat</show>
  <status>Ready to talk</status>
</presence>
~~~

### Away

~~~xml
<presence from="alice@local">
  <show>away</show>
  <status>Making tea</status>
</presence>
~~~

### Going offline

~~~xml
<presence type="unavailable" from="alice@local"/>
~~~

---

# 7. Sending Messages

Messages in serverless XMPP look identical to standard XMPP messages.

### Alice → Bob

~~~xml
<message
    from="alice@local"
    to="bob@local"
    type="chat">
  <body>Hello Bob, Bonjour works!</body>
</message>
~~~

### Bob replies

~~~xml
<message
    from="bob@local"
    to="alice@local"
    type="chat">
  <body>Hi Alice! I see you.</body>
</message>
~~~

---

# 8. Chat States (“typing…”) — Optional

If supported by both sides:

~~~xml
<message from="alice@local" to="bob@local" type="chat">
  <composing xmlns="http://jabber.org/protocol/chatstates"/>
</message>
~~~

Or “paused”:

~~~xml
<message from="alice@local" to="bob@local" type="chat">
  <paused xmlns="http://jabber.org/protocol/chatstates"/>
</message>
~~~

---

# 9. Avatars

Bonjour has **no server** to store avatars, so avatars are exchanged directly
peer-to-peer using either:

- **vCard (XEP-0054)**
- **PEP-like custom events**
- **XEP-0084 User Avatars**, if a client supports it
- **Ad-hoc IQs** specific to each implementation (Pidgin uses a custom one)

### Example: Avatar via vCard (base64 image)

~~~xml
<iq type="result" id="v1" from="alice@local" to="bob@local">
  <vCard xmlns="vcard-temp">
    <PHOTO>
      <TYPE>image/jpeg</TYPE>
      <BINVAL>/9j/4AAQSkZJRgABAQ...</BINVAL>
    </PHOTO>
  </vCard>
</iq>
~~~

### Bob requests Alice's vCard:

~~~xml
<iq type="get" id="v1" to="alice@local">
  <vCard xmlns="vcard-temp"/>
</iq>
~~~

Clients often cache avatars locally as long as the peer stays online.

---

# 10. Exchanging Capabilities

A peer may announce its feature capabilities (chatstates, vcard, avatars):

~~~xml
<presence from="alice@local">
  <c xmlns="http://jabber.org/protocol/caps"
     hash="sha-1"
     node="http://example.org/alice"
     ver="abcd1234"/>
</presence>
~~~

---

# 11. IQ Queries

Even without a server, peers may use IQs for:

- requesting vCards
- asking for capabilities
- requesting application-specific data
- sending file metadata

### Example: “ping” (XEP-0199)

~~~xml
<iq from="alice@local" to="bob@local" type="get" id="ping1">
  <ping xmlns="urn:xmpp:ping"/>
</iq>
~~~

### Response:

~~~xml
<iq from="bob@local" to="alice@local" type="result" id="ping1"/>
~~~

---

# 12. File Transfer (Simplified Example)

Bonjour clients often use direct SOCKS5, IBB, or custom transfer IQs.

### Offer:

~~~xml
<iq type="set" id="f1" to="bob@local">
  <file xmlns="urn:xmpp:myfile">
    <name>notes.txt</name>
    <size>1337</size>
  </file>
</iq>
~~~

### Acceptance:

~~~xml
<iq type="result" id="f1" from="bob@local"/>
~~~

Then a direct TCP or UDP connection is negotiated between peers.

---

# 13. Stream Closure

When done:

~~~xml
</stream:stream>
~~~

The TCP connection closes immediately after.

---

# 14. Summary

- Bonjour messaging is **serverless XMPP** using **mDNS discovery**.
- Contacts appear only when a device advertises `_presence._tcp.local`.
- After discovery, peers use **direct TCP** and exchange **XMPP stanzas**.
- Roster is **live and temporary**.
- Messages, presence, avatars, capabilities, and file transfers all use normal
  XMPP stanzas.
- All routing is **peer-to-peer**.

Bonjour is ideal for local, ad-hoc, offline-first communication environments.


