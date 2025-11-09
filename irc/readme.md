# IRC Protocol Overview

---

## What is IRC?

- IRC = **Internet Relay Chat**.
- A simple **text-based** protocol over TCP.
- Originally designed for group communication in channels.

---

## How Connection Happens

1. **TCP Connect**:
   - Client connects to IRC server on port `6667` (default, plaintext IRC).

2. **Registration**:
   - Client must send:
     - `NICK` (nickname)
     - `USER` (username + real name)

3. **Server Replies**:
   - Server sends welcome messages and server info.

4. **Optional: Authentication**:
   - Some servers require `PASS` before `NICK`/`USER`.
   - Some users authenticate via `NickServ` bot after connection.

---

## Example: Connection Start

```text
C: TCP SYN
S: TCP SYN/ACK
C: TCP ACK
```

- TCP connection established.

```text
C: PASS hunter2
C: NICK coolstudent
C: USER myuser 0 * :My Real Name
```

- `PASS` is optional unless server requires it.

---

## Server Welcome Example

```text
S: :irc.example.com 001 coolstudent :Welcome to the IRC Network coolstudent
S: :irc.example.com 002 coolstudent :Your host is irc.example.com, running version InspIRCd-3
S: :irc.example.com 003 coolstudent :This server was created 2024-04-20
S: :irc.example.com 004 coolstudent irc.example.com InspIRCd-3 o o
```

---

## Presence: Joining and Leaving Channels

### Joining a Channel

```text
C: JOIN #example
```

- Server responds with topic, user list, etc.

### Leaving a Channel

```text
C: PART #example :Leaving now
```

---

## Messaging: How Messages Are Sent

### Send a Message to a Channel

```text
C: PRIVMSG #example :Hello everyone!
```

- Sends a chat message to the whole channel.

### Send a Private Message to a User

```text
C: PRIVMSG friend :Hey, how are you?
```

---

## Important IRC Commands (Summary)

| Command    | Purpose                         |
|:-----------|:--------------------------------|
| `PASS`     | Authenticate before login (optional) |
| `NICK`     | Set your nickname |
| `USER`     | Set your username and real name |
| `JOIN`     | Join a channel |
| `PART`     | Leave a channel |
| `PRIVMSG`  | Send a message (channel or user) |
| `PING`/`PONG` | Keep the connection alive |

---

## Server Pings and Pongs

- Server periodically sends `PING` to check if the client is alive.

Example:

```text
S: PING :irc.example.com
C: PONG :irc.example.com
```

- Clients must respond with `PONG` to avoid being disconnected.

---

## Key Ideas for Students

- IRC is a **line-based** protocol: one command per line.
- Clients must **respond to PING** to stay connected.
- Messaging is **simple text commands**.
- Channels (`#example`) organize conversations.

---

# End of Presentation

