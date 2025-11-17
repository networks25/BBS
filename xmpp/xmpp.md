# Introduction to the XMPP Protocol

## 1. What Is XMPP?
**XMPP (Extensible Messaging and Presence Protocol)** is an open, decentralized communication protocol originally designed for instant messaging.
It is based on **XML streams** and allows real-time exchange of structured data between network entities.

XMPP is:
- **Open** — defined in IETF RFCs, free to implement.
- **Federated** — like email: anyone can run a server, and servers talk to each other.
- **Extensible** — new features are added using XEPs (XMPP Extension Protocols).
- **Real-time** — optimized for quick delivery of small messages (text, presence, notifications, events).

Today XMPP is used for chat, IoT messaging, push notifications, collaboration tools, and publish/subscribe systems.

---

## 2. Basic Architecture

### a. Entities
The main XMPP entities are:
- **Client**: user application (e.g., Conversations, Movim, Pidgin).
- **Server**: accepts connections from clients, routes stanzas, handles authentication.
- **Gateway/Component**: optional add-ons providing extra services (pubsub, MUC rooms, transports).

### b. Addressing (JID)
Every XMPP identity is a **JID** (Jabber ID):

```
localpart@domain/resource
```

Examples:
- `alice@example.com`
- `bob@example.org/phone`
- `printer@office.lan/printer1`

The **resource** identifies a specific device or client instance.

---

## 3. XML Streams and Stanzas

When a client connects, it and the server exchange **two long-lived XML streams** (one in each direction) instead of sending separate HTTP-style requests.

Inside these streams, all communication happens via **three stanza types**:

### 3.1 `<message/>`
For sending chat messages, notifications, alerts.
```xml
<message to="bob@example.org" type="chat">
  <body>Hello!</body>
</message>
```

### 3.2 `<presence/>`
Advertises availability (online, away, do-not-disturb). Also used for joining group chats.
```xml
<presence>
  <show>away</show>
  <status>Making coffee</status>
</presence>
```

### 3.3 `<iq/>` — Info/Query
Request/response mechanism (like RPC).
```xml
<iq type="get" id="1" to="server.example.org">
  <query xmlns="jabber:iq:roster"/>
</iq>
```

---

## 4. Authentication and Encryption

### 4.1 TLS
The XMPP client negotiates TLS immediately after stream setup.
Modern XMPP requires TLS (similar to HTTPS).

### 4.2 SASL
After TLS, authentication happens via **SASL** mechanisms:
- PLAIN (inside TLS)
- SCRAM-SHA-1, SCRAM-SHA-256
- EXTERNAL (client certificates)
- OAUTHBEARER

---

## 5. Server-to-Server Federation (S2S)

Just like email:
- Alice on `example.org`
- Bob on `another.net`

Servers authenticate each other using **DNS SRV** + **TLS**.
This enables global federation: any server can talk to any other server.

---

## 6. Extensions (XEPs)

XMPP’s power comes from its extension library (XEP = XMPP Extension Protocol).
Some important ones:

- **XEP-0030** — Service Discovery
- **XEP-0045** — Multi-User Chat (group chat)
- **XEP-0054 / vCard** — User profiles
- **XEP-0060** — PubSub (publish/subscribe events)
- **XEP-0085** — Chat states (typing…)
- **XEP-0163** — PEP (Personal Eventing Protocol)
- **XEP-0184** — Message Delivery Receipts
- **XEP-0198** — Stream Management (acks, resume after reconnect)
- **XEP-0363** — HTTP File Upload

Servers selectively support extensions; clients decide which ones to use.

---

## 7. Typical Client Workflow (Step by Step)

1. **DNS lookup** for server SRV record (`_xmpp-client._tcp.domain`).
2. **TCP connection** (port 5222).
3. **Initial XML stream** (client → server).
4. **TLS negotiation**.
5. **Restart XML stream** (now inside TLS).
6. **SASL authentication**.
7. **Resource binding** (server assigns full JID).
8. **Roster (contact list) retrieval**.
9. **Presence broadcast** (“I’m online”).
10. **Normal messaging** begins.

---

## 8. PubSub and PEP (Event-Based XMPP)

XMPP is not only chat.
With **XEP-0060** (PubSub) and **XEP-0163** (PEP), XMPP works like MQTT or a notification bus:

- Publishers send items to nodes.
- Subscribers receive updates.
- Used for IoT sensors, social feed updates, microblogging.

Example (very simplified):
```xml
<iq type="set" id="pub1">
  <pubsub xmlns="http://jabber.org/protocol/pubsub">
    <publish node="temperature">
      <item id="42"><temp>21.5</temp></item>
    </publish>
  </pubsub>
</iq>
```

---

## 9. Advantages of XMPP

- **Federated** — no central authority.
- **Open standards**.
- **Extensible with XML**.
- **Works well for mobile** with Stream Management and message receipts.
- **Supports IoT and eventing** (PubSub/PEP).
- **Mature** — used in production for 20+ years.
- **Cross-language libraries** available (Pascal, C, Go, Python, JS…).

---

## 10. Comparison With Other Protocols

| Feature | XMPP | MQTT | Matrix | IRC |
|--------|------|------|--------|-----|
| Transport | TCP + XML stream | TCP, lightweight binary | HTTP/JSON over federation | TCP line-based |
| Decentralized | Yes | No (broker-based) | Yes | Yes |
| Extensible | Very | Limited | Very | Limited |
| Use Case | Messaging, presence, IoT | IoT, sensors | Chat, rooms, history sync | Simple chat |
| Offline delivery | Yes | Yes (if broker stores) | Yes | No (unless bouncer) |

---

## 11. Minimal Example Conversation

### Client wants to chat:

Client → Server:
```xml
<message to="bob@example.org" type="chat">
  <body>Hello, Bob!</body>
</message>
```

Server → Bob:
```xml
<message from="alice@example.com" type="chat">
  <body>Hello, Bob!</body>
</message>
```

Bob replies:
```xml
<message to="alice@example.com" type="chat">
  <body>Hi!</body>
</message>
```

---

## 12. Summary

- XMPP is an open, federated, extensible protocol for real-time communication.
- It uses persistent XML streams with three stanza types: **message**, **presence**, **iq**.
- Authentication uses **TLS + SASL**.
- Extensions (XEPs) add rich functionality like group chat, file upload, pubsub, and typing notifications.
- XMPP is suitable for messaging, collaboration tools, and IoT/automation.


