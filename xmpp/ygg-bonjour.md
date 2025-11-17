# Ygg-Bonjour: A Serverless XMPP-Like Protocol for Yggdrasil

**Ygg-Bonjour** is a simple, serverless messaging protocol inspired by
XEP-0174 (serverless XMPP / Bonjour), adapted for use over **Yggdrasil** where
mDNS is not available.

The design goal:
> *“Bonjour without mDNS: manual buddies, direct TCP, XMPP stanzas.”*

This document is intended as a **small spec** so that anyone can implement a
Ygg-Bonjour client (in Pascal, C, Python, etc.) with minimal guesswork.

- Discovery is replaced by **manual buddy configuration**.
- Communication uses **direct TCP connections over Yggdrasil**.
- Protocol on the wire is **XMPP-like XML streams** (very small subset).

---

## 1. Architecture Overview

Ygg-Bonjour has three main layers:

1. **Transport**:
   - TCP connection over Yggdrasil
   - Default port (suggested): **5298**, but configurable

2. **Addressing**:
   - Each endpoint has a **Ygg address** (IPv6) and a local nickname.
   - We define a simple ID format called **YJID** (Yggdrasil JID).

3. **Stanzas** (XML):
   - `<stream:stream>` wrapping the connection
   - `<presence>` for availability
   - `<message>` for chat messages
   - `<iq>` for pings, vCards, small RPC

No servers, no federation. Every node is both client and server.

---

## 2. Addressing and YJID

### 2.1 YJID Format

A **YJID** (Yggdrasil JID) is a simple identifier:

- `localname@ygg` (no DNS)
- Optional resource: `localname@ygg/device`

Examples:

- `alice@ygg`
- `bob@ygg/pinephone`
- `sensor01@ygg`

The mapping from YJID → Yggdrasil IP + port is stored locally in a **buddy
configuration**.

### 2.2 Example Buddy Configuration

A client may use JSON, INI, YAML, etc. Here is a **JSON example**:

~~~json
{
  "self": {
    "yjID": "alice@ygg",
    "listen_port": 5298
  },
  "buddies": [
    {
      "yjID": "bob@ygg",
      "ygg_ip": "200:1111:2222:3333:4444:5555:6666:7777",
      "port": 5298,
      "nickname": "Bob on Ygg"
    },
    {
      "yjID": "carol@ygg/laptop",
      "ygg_ip": "200:aaaa:bbbb:cccc:dddd:eeee:ffff:0001",
      "port": 5300,
      "nickname": "Carol"
    }
  ]
}
~~~

Implementation note:
Clients may use any internal format; this document only shows one possible
representation.

---

## 3. Transport and Connection Lifecycle

### 3.1 Listening

Each Ygg-Bonjour client:

- Opens a TCP listening socket on a configured port (e.g. `5298`).
- Accepts incoming connections from buddies.

### 3.2 Outgoing Connections

To send the first message to a buddy:

1. Look up buddy in config (YJID → `ygg_ip` + `port`).
2. Open TCP connection to `ygg_ip:port`.
3. Initiate an XML stream (see below).
4. Send `<presence>` and `<message>` etc.

Connections may be:

- reused for multiple messages
- closed after inactivity

---

## 4. XML Stream Initialization

Ygg-Bonjour uses a **single XML stream** in each direction over the same TCP
connection, similar to XMPP.

### 4.1 Alice connects to Bob

**Alice → Bob:**

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="alice@ygg"
    to="bob@ygg"
    version="1.0">
~~~

**Bob → Alice (reply):**

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="bob@ygg"
    to="alice@ygg"
    version="1.0">
~~~

After this, they exchange `<presence>`, `<message>`, `<iq>` stanzas within the
stream. The stream is closed with:

~~~xml
</stream:stream>
~~~

---

## 5. Presence Model

There is **no mDNS**, so presence is determined by:

- **TCP reachability** (can we connect?)
- **Stream health** (is connection alive?)
- **Ping stanzas** (see section 7)

### 5.1 When Do We Consider a Buddy “Online”?

A client considers `bob@ygg` **online** if:

- It currently has an open TCP connection to Bob **or**
- It recently received a valid `<presence>` from Bob on an active stream

Upon connection, each side **announces presence**.

### 5.2 Initial Presence Example

**Alice → Bob:**

~~~xml
<presence from="alice@ygg">
  <show>chat</show>
  <status>On Yggdrasil, ready to chat</status>
</presence>
~~~

**Bob → Alice:**

~~~xml
<presence from="bob@ygg">
  <show>away</show>
  <status>Tea break</status>
</presence>
~~~

### 5.3 Going Offline

When closing the stream intentionally:

~~~xml
<presence type="unavailable" from="alice@ygg"/>
</stream:stream>
~~~

If the connection drops unexpectedly (timeout, error), the client should mark
that buddy as **offline**.

---

## 6. Contact List and Roster Semantics

In classical Bonjour, the roster is dynamic from mDNS.
In Ygg-Bonjour, **the roster comes from the local configuration**.

### 6.1 Adding a Contact

Implementation policy:

- The user adds a buddy in the UI or config file with:
  - YJID: `bob@ygg`
  - Yggdrasil IP: `200:1111:...`
  - port: 5298

The client loads this and shows Bob in the buddy list.

### 6.2 Determining Online/Offline State

For each buddy:

1. On startup or periodically, try to connect:
   - If TCP connects and stream is established → `online`.
   - If connect fails → `offline`.
2. When a connection is lost:
   - Mark buddy `offline`.

Optionally, a client may delay “offline” status until a ping timeout.

### 6.3 Example: Alice tries to reach Bob

- Alice attempts TCP connection to Bob:
  - If success: mark `bob@ygg` as online.
  - If failure: keep `bob@ygg` as offline and show “cannot connect”.

This is under client control; the protocol simply defines the stanzas.

---

## 7. Ping and Keepalive (Liveness)

To avoid considering someone online when the TCP link is dead, Ygg-Bonjour
reuses the XMPP ping idea.

### 7.1 Ping Namespace

We use a simple ping namespace:

- `urn:yggb:ping`

### 7.2 Ping Request

**Alice → Bob:**

~~~xml
<iq type="get" id="ping-1" from="alice@ygg" to="bob@ygg">
  <ping xmlns="urn:yggb:ping"/>
</iq>
~~~

### 7.3 Ping Response

**Bob → Alice:**

~~~xml
<iq type="result" id="ping-1" from="bob@ygg" to="alice@ygg"/>
~~~

If Alice doesn’t receive a result in X seconds:

- It may assume the connection is dead
- Close the TCP socket
- Mark `bob@ygg` as offline

---

## 8. Messaging

### 8.1 Simple Text Message

**Alice → Bob:**

~~~xml
<message
    from="alice@ygg"
    to="bob@ygg"
    type="chat">
  <body>Hello Bob, greetings from Ygg-Bonjour!</body>
</message>
~~~

### 8.2 Message with Subject

~~~xml
<message
    from="alice@ygg"
    to="bob@ygg"
    type="chat">
  <subject>Question</subject>
  <body>Are you free this evening?</body>
</message>
~~~

### 8.3 Message with Chat State (Typing Indicator)

Using the standard chatstates namespace:

- `http://jabber.org/protocol/chatstates`

**Alice is typing:**

~~~xml
<message from="alice@ygg" to="bob@ygg" type="chat">
  <composing xmlns="http://jabber.org/protocol/chatstates"/>
</message>
~~~

**Alice paused:**

~~~xml
<message from="alice@ygg" to="bob@ygg" type="chat">
  <paused xmlns="http://jabber.org/protocol/chatstates"/>
</message>
~~~

### 8.4 Delivery Receipt (Optional)

For implementations that want simple delivery receipts, a very small format:

Namespace: `urn:yggb:receipts`

**Alice sends message with an ID:**

~~~xml
<message from="alice@ygg" to="bob@ygg" type="chat" id="msg-123">
  <body>Did this get through?</body>
  <request xmlns="urn:yggb:receipts"/>
</message>
~~~

**Bob acknowledges:**

~~~xml
<message from="bob@ygg" to="alice@ygg">
  <received xmlns="urn:yggb:receipts" id="msg-123"/>
</message>
~~~

---

## 9. Avatars

As in Bonjour, there is no server storage. Avatars are shared directly between
peers.

Two simple options:

1. **vCard-based** (like `vcard-temp`)
2. **Direct avatar IQ** (simpler, custom namespace)

### 9.1 Option 1: vCard Avatar

**Bob requests Alice’s vCard:**

~~~xml
<iq type="get" id="v1" from="bob@ygg" to="alice@ygg">
  <vCard xmlns="vcard-temp"/>
</iq>
~~~

**Alice responds with embedded avatar:**

~~~xml
<iq type="result" id="v1" from="alice@ygg" to="bob@ygg">
  <vCard xmlns="vcard-temp">
    <FN>Alice</FN>
    <PHOTO>
      <TYPE>image/png</TYPE>
      <BINVAL>iVBORw0KGgoAAAANSUhEUgAA...</BINVAL>
    </PHOTO>
  </vCard>
</iq>
~~~

Clients may cache avatars and only request when missing or changed.

### 9.2 Option 2: Simple Avatar IQ (Custom)

Namespace: `urn:yggb:avatar`

**Bob requests avatar:**

~~~xml
<iq type="get" id="av1" from="bob@ygg" to="alice@ygg">
  <avatar xmlns="urn:yggb:avatar">
    <request/>
  </avatar>
</iq>
~~~

**Alice responds:**

~~~xml
<iq type="result" id="av1" from="alice@ygg" to="bob@ygg">
  <avatar xmlns="urn:yggb:avatar">
    <type>image/png</type>
    <data>iVBORw0KGgoAAAANSUhEUgAA...</data>
  </avatar>
</iq>
~~~

---

## 10. IQ (Info/Query) Semantics

`<iq>` stanzas follow the XMPP request/response pattern:

- `type="get"` or `type="set"` → request
- `type="result"` → successful response
- `type="error"` → error

### 10.1 Example: Capabilities Query

Namespace: `urn:yggb:disco`

**Bob asks Alice what she supports:**

~~~xml
<iq type="get" id="cap1" from="bob@ygg" to="alice@ygg">
  <query xmlns="urn:yggb:disco"/>
</iq>
~~~

**Alice replies:**

~~~xml
<iq type="result" id="cap1" from="alice@ygg" to="bob@ygg">
  <query xmlns="urn:yggb:disco">
    <feature var="chat"/>
    <feature var="chatstates"/>
    <feature var="avatar-vcard"/>
    <feature var="ping"/>
  </query>
</iq>
~~~

### 10.2 Example: Error IQ

If Alice receives an unknown IQ:

~~~xml
<iq type="error" id="cap1" from="alice@ygg" to="bob@ygg">
  <error type="cancel">
    <feature-not-implemented xmlns="urn:ietf:params:xml:ns:xmpp-stanzas"/>
  </error>
</iq>
~~~

---

## 11. Security and Encryption

Ygg-Bonjour itself does not *mandate* TLS, but implementers are encouraged to
support one of:

1. **Plain TCP over Yggdrasil**, trusting Ygg as the secure overlay.
2. **TLS over TCP over Yggdrasil**, where:
   - Stream start is wrapped in TLS
   - After TLS, normal `<stream:stream>` begins

### 11.1 Example: TLS-then-Stream

Transport sequence:

1. TCP connect to `ygg_ip:port`.
2. TLS handshake.
3. Inside TLS, exchange:

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="alice@ygg"
    to="bob@ygg"
    version="1.0">
~~~

The rest of the protocol is unchanged.

---

## 12. Minimal Client State Machine (High-Level)

For each buddy:

1. **Config / Roster Load**
   - Read YJID, IP, port.

2. **Connect**
   - Try TCP connect.
   - On success:
     - Start XML stream.
     - Send initial `<presence>`.
     - Mark buddy as online.
   - On failure:
     - Mark buddy as offline.

3. **During Connection**
   - Parse incoming XML stanzas.
   - Handle `<message>`, `<presence>`, `<iq>`.
   - Periodically send ping IQ.
   - If no pong in timeout window → close connection, mark offline.

4. **Disconnect**
   - On local shutdown:
     - Send `<presence type="unavailable">`.
     - Send `</stream:stream>`.
   - Close socket.

5. **UI Integration**
   - Display roster from config.
   - Update status icons based on reachability + stream state.

---

## 13. Example Full Session (Alice ↔ Bob)

This is a simplified example of a complete sequence.

### 13.1 Alice connects to Bob

- TCP connect `200:1111:...` port `5298`.

**Alice → Bob:**

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="alice@ygg"
    to="bob@ygg"
    version="1.0">
<presence from="alice@ygg">
  <show>chat</show>
  <status>Online via Ygg-Bonjour</status>
</presence>
<iq type="get" id="cap1" from="alice@ygg" to="bob@ygg">
  <query xmlns="urn:yggb:disco"/>
</iq>
~~~

**Bob → Alice:**

~~~xml
<stream:stream
    xmlns="jabber:client"
    xmlns:stream="http://etherx.jabber.org/streams"
    from="bob@ygg"
    to="alice@ygg"
    version="1.0">
<presence from="bob@ygg">
  <show>chat</show>
  <status>Also on Ygg-Bonjour</status>
</presence>
<iq type="result" id="cap1" from="bob@ygg" to="alice@ygg">
  <query xmlns="urn:yggb:disco">
    <feature var="chat"/>
    <feature var="chatstates"/>
    <feature var="ping"/>
  </query>
</iq>
~~~

### 13.2 Alice sends a message

**Alice → Bob:**

~~~xml
<message from="alice@ygg" to="bob@ygg" type="chat" id="m42">
  <body>Hey Bob, can you see this?</body>
  <request xmlns="urn:yggb:receipts"/>
</message>
~~~

**Bob → Alice (receipt):**

~~~xml
<message from="bob@ygg" to="alice@ygg">
  <received xmlns="urn:yggb:receipts" id="m42"/>
</message>
~~~

**Bob → Alice (reply):**

~~~xml
<message from="bob@ygg" to="alice@ygg" type="chat">
  <body>Yes, I see you loud and clear.</body>
</message>
~~~

### 13.3 Ping for Liveness

**Alice → Bob:**

~~~xml
<iq type="get" id="ping-2" from="alice@ygg" to="bob@ygg">
  <ping xmlns="urn:yggb:ping"/>
</iq>
~~~

**Bob → Alice:**

~~~xml
<iq type="result" id="ping-2" from="bob@ygg" to="alice@ygg"/>
~~~

### 13.4 Disconnect

When Alice exits:

**Alice → Bob:**

~~~xml
<presence type="unavailable" from="alice@ygg"/>
</stream:stream>
~~~

(Then TCP connection closes.)

**Bob** marks `alice@ygg` as offline.

---

## 14. Summary

- **Ygg-Bonjour** is “Bonjour without mDNS but with manual buddies”.
- Roster is configured manually (YJID → Ygg IP + port).
- TCP over Yggdrasil carries an XMPP-like XML stream.
- Presence is based on **reachability** plus optional **ping**.
- Messages, avatars, and simple IQs reuse standard XMPP patterns.
- Implementers can start with:
  - one listener,
  - one outbound connector,
  - a tiny XML parser,
  - and the stanzas defined here.

This should be enough to implement a usable Ygg-Bonjour client in Pascal, C,
Python, or any language that can do TCP + XML.

