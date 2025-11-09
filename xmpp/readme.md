# XMPP Protocol Overview

---

## What is XMPP?

- XMPP = **Extensible Messaging and Presence Protocol**.
- Used for real-time communication: messages, presence, and more.
- Based on **XML** streams over TCP.

---

## How Connection Happens

1. **TCP Connect**:
   - Client connects to server on port `5222` (standard client-to-server port).

2. **Stream Initiation**:
   - Client sends opening `<stream>` tag.
   - Server responds with its own `<stream>` tag.

3. **Authentication (SASL)**:
   - Server offers supported authentication methods (PLAIN, SCRAM, etc.).
   - Client authenticates using one of the offered methods.

4. **Resource Binding**:
   - After authentication, client requests a resource (like `/laptop`, `/phone`).

5. **Session Establishment**:
   - Client asks to start a session for messaging and presence.

---

## Example: Connection Start

```text
C: TCP SYN
S: TCP SYN/ACK
C: TCP ACK
```

- TCP connection established.

```xml
<!-- Client opens stream -->
<stream:stream to="example.com" xmlns="jabber:client" xmlns:stream="http://etherx.jabber.org/streams" version="1.0">

<!-- Server responds -->
<stream:stream from="example.com" xmlns="jabber:client" xmlns:stream="http://etherx.jabber.org/streams" version="1.0">
```

---

## Example: Authentication (SASL PLAIN)

```xml
<!-- Server lists authentication mechanisms -->
<stream:features>
  <mechanisms xmlns="urn:ietf:params:xml:ns:xmpp-sasl">
    <mechanism>PLAIN</mechanism>
  </mechanisms>
</stream:features>

<!-- Client chooses PLAIN, sends credentials -->
<auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" mechanism="PLAIN">BASE64(username\0username\0password)</auth>

<!-- Server replies -->
<success xmlns="urn:ietf:params:xml:ns:xmpp-sasl"/>
```

---

## After Authentication

- Client must **re-open** the stream.
- Server announces available features again (like resource binding).

```xml
<stream:features>
  <bind xmlns="urn:ietf:params:xml:ns:xmpp-bind"/>
  <session xmlns="urn:ietf:params:xml:ns:xmpp-session"/>
</stream:features>
```

---

## Resource Binding and Session Start

```xml
<!-- Client requests resource binding -->
<iq type="set" id="bind_1">
  <bind xmlns="urn:ietf:params:xml:ns:xmpp-bind">
    <resource>laptop</resource>
  </bind>
</iq>

<!-- Server confirms -->
<iq type="result" id="bind_1">
  <bind xmlns="urn:ietf:params:xml:ns:xmpp-bind">
    <jid>user@example.com/laptop</jid>
  </bind>
</iq>

<!-- Client requests session start -->
<iq type="set" id="sess_1">
  <session xmlns="urn:ietf:params:xml:ns:xmpp-session"/>
</iq>

<!-- Server confirms -->
<iq type="result" id="sess_1"/>
```

---

## Presence: What is It?

- Presence tells others if you are **online**, **away**, **busy**, etc.
- Clients send presence stanzas.

### Example: Announce Available

```xml
<presence/>
```

(Default = available)

### Example: Announce Away

```xml
<presence>
  <show>away</show>
  <status>Out for lunch</status>
</presence>
```

---

## Messaging: How Messages Are Sent

- Messages are sent using `<message>` stanzas.

### Example: Sending a Message

```xml
<message to="friend@example.com" type="chat">
  <body>Hello, are you there?</body>
</message>
```

- Server routes the message to the correct connected client or stores it if offline.

---

## Important XML Stanzas (Summary)

| Type      | Description                        |
|:----------|:-----------------------------------|
| `<stream>` | Start XML stream |
| `<auth>`   | Authenticate via SASL |
| `<iq>`     | "Info/Query" - for binding, session, etc. |
| `<presence>` | Online status announcement |
| `<message>` | Actual chat message |

---

## Key Ideas

- XMPP is **streaming XML** over TCP.
- Each message (stanza) is small and self-contained inside the stream.
- Presence is a *first-class* feature, not an add-on.
- After authentication and binding, clients are ready to exchange presence and messages.

---

# End of Presentation

