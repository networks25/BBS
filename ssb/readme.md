# Secure Scuttlebutt (SSB) Protocol Overview

---

## What is SSB?

- Secure Scuttlebutt (SSB) is a **peer-to-peer** (P2P) communication protocol.
- It is **append-only**: messages are immutable once published.
- Data is shared by **gossip** between peers.

---

## Basic Concepts

- **Feeds**: a sequence of signed messages (like a personal blockchain).
- **Messages**: JSON objects, cryptographically signed.
- **Peers**: nodes that replicate each other’s feeds.

---

## Networking Basics

- **P2P over TCP** (default port: `8008`).
- **Optional Plugins**: e.g., secret-handshake encryption layer, blobs, muxrpc services.

---

## How Connections Happen

1. **Discovery**: peers find each other via:
   - LAN multicast
   - Hardcoded addresses
   - Public "pub" servers (special peers)

2. **TCP Connect**:
   - Client connects to peer's IP and port.

3. **Secret Handshake**:
   - A mutual authentication and encryption handshake.
   - Uses Ed25519 public keys.
   - Both sides authenticate *and* establish a shared secret for encryption.

---

## Secret Handshake Overview

- Both peers have long-term public keys.
- Exchange happens in 3 steps:
  1. **ClientHello** (client sends its ephemeral public key).
  2. **ServerHello** (server sends ephemeral key and authentication).
  3. **ClientAuth** (client sends authentication).

- After this:
  - A shared key is derived.
  - Communication is encrypted (e.g., using NaCl/secretbox).

> **Note**: Secret-handshake is its own *tiny protocol* inside TCP.

---

## Example: TCP + Secret Handshake Start

```text
C: TCP SYN
S: TCP SYN/ACK
C: TCP ACK
```

- TCP connection established.

```text
C: [32 bytes] Ephemeral public key
S: [32 bytes] Server ephemeral key + authentication
C: [32 bytes] Client authentication
```

- Now both sides encrypt messages.

---

## Message Format (After Handshake)

Each encrypted packet:
- Length-prefixed (4-byte big endian integer)
- Followed by encrypted payload.

Payload example (before encryption):
```json
{
  "previous": "sha256-hash",
  "author": "public-key",
  "sequence": 42,
  "timestamp": 1683052190,
  "hash": "sha256",
  "content": {
    "type": "post",
    "text": "hello world"
  },
  "signature": "sig"
}
```

---

## Gossip (Replication)

- Peers exchange **have/want** information.
- "Do you have feed X up to sequence Y?"
- "Please send me new messages!"

> Gossip traffic is just normal encrypted packets after handshake.

---

## Important Byte Flows (Summary)

| Stage | Data Sent | Size |
|:-----|:----------|:-----|
| TCP SYN | — | — |
| Client Ephemeral Key | 32 bytes |
| Server Ephemeral Key + Auth | 32 + auth |
| Client Authentication | 32 + auth |
| Encrypted Messages | 4-byte length + ciphertext |

---

## Key Ideas

- No central server required.
- Every feed is cryptographically signed and verifiable.
- Data is encrypted *and* authenticated at transport level.
- Gossip spreads the data eventually to all peers.

---

# End of Presentation

