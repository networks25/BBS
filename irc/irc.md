# Introduction to the IRC Protocol

Internet Relay Chat (IRC) is one of the oldest and simplest text-based messaging protocols still in use today. Designed in 1988, it uses a lightweight, line-oriented format that makes it easy to implement in any programming language.

This document gives a concise introduction to how IRC works on the wire: connections, messages, commands, and the basic client–server model.

---

## 1. Architecture Overview

IRC uses a **client–server** model:

- Clients connect to an **IRC server** (also known as a daemon).
- Servers can connect to **other servers**, forming a network.
- Clients never talk directly to each other; everything goes through the server.

IRC typically communicates over:

- **Plain TCP** (default ports 6660–6669, 6679)
- **TLS** encrypted TCP (commonly port 6697)

---

## 2. IRC Message Format

Every IRC message is a single line of text terminated by CRLF (`\r\n`).
General format:

```
[:prefix] command [params ...] [:trailing]
```

Where:

- **prefix** – optional identifier of sender (server or another client)
- **command** – text command (`PING`, `PRIVMSG`, etc.) or a numeric code
- **params** – space-separated arguments
- **trailing** – free-form text (may contain spaces)

### Example message

```
:norayr!user@host PRIVMSG #gentoo :hello everyone
```

Breakdown:

- Prefix: `norayr!user@host`
- Command: `PRIVMSG`
- Parameter: `#gentoo`
- Trailing text: `hello everyone`

---

## 3. Connecting to an IRC Server

After establishing a TCP or TLS connection, the client must send two commands:

### NICK
Sets your nickname.

```
NICK mynick
```

### USER
Provides username/realname information.

```
USER mylogin 0 * :My Real Name
```

Once authenticated, the server sends a **welcome burst** with numeric replies.

---

## 4. Basic IRC Commands

### PING / PONG

Server checks if you’re alive:

```
PING :abcdef
```

Client replies:

```
PONG :abcdef
```

### JOIN

Enter a channel.

```
JOIN #mychannel
```

### PART

Leave a channel.

```
PART #mychannel :goodbye
```

### PRIVMSG

Send a message to a channel or user.

```
PRIVMSG #mychannel :hello there
```

### NOTICE

Similar to `PRIVMSG` but should not trigger auto-replies.

```
NOTICE nick :you are connected
```

### QUIT

Disconnect.

```
QUIT :leaving
```

---

## 5. Server Numerics

Servers use 3-digit numeric replies for status messages.

Example:

```
001 nick :Welcome to the IRC network
353 nick = #chan :n1 n2 n3
```

Some important numerics:

- `001` — Welcome
- `376` — End of MOTD
- `353` — Channel user list
- `366` — End of NAMES list

---

## 6. Channels

Channels:

- Start with `#` (public)
- Have modes: moderated, invite-only, topic-locked, etc.

Example: set topic

```
TOPIC #chat :New topic here
```

Example: channel modes

```
MODE #chat +m
MODE #chat +o myfriend
```

`+m` = moderated, `+o` = give operator rights.

---

## 7. User Masks & Prefixes

Users are identified as:

```
nick!ident@hostname
```

Example prefix in a message:

```
:alice!~a@1.2.3.4 JOIN :#test
```

---

## 8. Typical IRC Session (Minimal)

A minimal raw session might look like:

```
NICK guest123
USER guest 0 * :Temporary User
PING :12345
PONG :12345
JOIN #test
PRIVMSG #test :hi all
QUIT :bye
```

---

## 9. Why IRC Is Simple and Hackable

- Plain ASCII lines
- No binary framing
- Easy to test with netcat or telnet
- Flexible for bots and custom clients
- Networks can share channels via server links

---

## 10. Try IRC with Netcat

```
nc irc.libera.chat 6667
NICK test123
USER test 0 * :Test User
JOIN #libera
PRIVMSG #libera :hello from raw IRC
```

---

This is only the beginning, but with this you can already open a socket and write your own IRC client or bot from scratch.


when we get:

```
PING :molybdenum.libera.chat
```

we should respond with

```
PONG :molybdenum.libera.chat
```

---------

NAMES #oberon

----------

Send a message to a channel:

```
PRIVMSG #oberon :hello from raw IRC
```

Send a private message to a user:

```
PRIVMSG alice :hello alice
```

Set the channel topic:
```
TOPIC #oberon :Oberon systems discussion
```

Change your nick
```
NICK inky2
```

Get the topic:
```
TOPIC #oberon
```

List available channels:
```
LIST
```

Get user info:
```
WHOIS inky
```

Leave a channel:
```
PART #oberon :gtg
```

CTCP ACTION (the /me command)
What `/me waves` does in IRC clients:
```
PRIVMSG #oberon ::\x01ACTION waves\x01
```
Server info:
```
TIME
VERSION
LUSERS
MOTD
```

Send a notice

```
NOTICE #oberon :This is a notice
```

See list of users matching mask

```
WHO #oberon
WHO *
```

Ask server what channels a user is in
```
WHOIS user
```

Get your current hostmask
```
USERHOST inky
```

Minimal example session:
```
nc irc.libera.chat 6667
NICK inky
USER inky 0 * :My Real Name

JOIN #oberon
PRIVMSG #oberon :hello everyone

PING :xyz
PONG :xyz

WHOIS inky
TOPIC #oberon

QUIT :see you
```

