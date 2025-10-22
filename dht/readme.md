# Distributed Hash Table (DHT) Overview

---

## What is a DHT?

- **DHT = Distributed Hash Table**.
- A **database** that is **shared across many computers** (peers).
- No single server: **everyone** helps store and find data.
- Used in:
  - **BitTorrent** (finding peers with a file)
  - **IPFS** (finding content)
  - **Yggdrasil** (peer discovery)
  - **Other P2P networks**

---

## Basic Concepts

- **Key**: A unique identifier (e.g., file hash, document ID).
- **Value**: Information associated with the key (e.g., IP address, file location).
- **Peers**: Nodes that store parts of the DHT and help find data.

> A DHT is like a giant key-value store, but split across thousands of computers.

---

## Very Simple: What is Hashing?

- A **hash function** takes any input (file, name, etc.) and gives a **fixed-size random-looking number**.

Example:

```text
Input: "hello"
Hash:  2cf24dba5fb0a...

Input: "hello!"
Hash:  f0a5b3e4c6c1...
```

> Even small changes in input make **completely different** outputs!

---

## Why Hashes Are Good for DHT

| Property             | Why It Helps DHT                               |
|:---------------------|:----------------------------------------------|
| Uniformly distributed | Makes sure data spreads evenly across peers. |
| Hard to guess         | Peers cannot predict IDs easily (security). |
| Easy to compare       | IDs are just numbers → fast to compare (XOR). |

✅ Hashes make DHTs balanced, secure, and efficient.

---

## How Peers Know Who is Closer

- Every **peer** and every **key** has an ID (a big number, usually a hash).
- To check who is closer to a target key:
  - Compute `XOR(peer ID, target key)`
  - The **smaller** the result, the **closer** the peer.

### Example:

```text
Target key:  10110010
Peer A ID:   10010010
Peer B ID:   10100010

XOR(Target, Peer A) = 00100000
XOR(Target, Peer B) = 00010000
```

- `00010000` is smaller → **Peer B is closer**.

✅ XOR distance is fast and simple to compute!

---

## How Peer IDs and Content Keys Are Connected

- **Peer IDs** are random (hash of public key, or random number).
- **Content Keys** are random (hash of content, like a file).

There is **no direct connection** between peer ID and content key.

Instead:
- Peers are **responsible** for **keys that are mathematically closest** to their ID.
- Responsibility is automatic based on the DHT rules.

---

### Example:

Suppose the DHT keyspace is 0 to 65535.

| Peer ID  | Range of Responsibility |
|:---------|:-------------------------|
| 10000    | Keys near 10000 |
| 20000    | Keys near 20000 |
| 30000    | Keys near 30000 |

If a new file has key **10200**:
- It is closer to Peer 10000 than to 20000 or 30000.
- So **Peer 10000 becomes responsible** for it.

✅ Peers don't choose content.
✅ The DHT decides automatically.

---

## How DHT Nodes Find Other DHT Nodes

- When a DHT node starts, it needs some **starting points** (called **bootstrap nodes**).
- Bootstrap nodes are:
  - Hardcoded addresses (well-known stable nodes),
  - Saved from previous sessions,
  - Manually configured if needed.

### Bootstrap Process

1. Connect to one or more bootstrap nodes.
2. Ask them for known peers.
3. Contact new peers.
4. Expand your routing table.

✅ After bootstrapping, the node keeps learning about more peers over time.

---

## DHT is Self-Healing

- If some nodes go offline, the DHT still works.
- Nodes constantly update their peer lists.
- Lookups find alternate routes if some peers disappear.
- New nodes can join easily without needing central coordination.

✅ This makes DHTs highly **resilient** and **decentralized**.

---

## How Lookup Works

1. You want to find the **value** for a **key**.
2. You ask your known peers.
3. Peers **don't search everything** — they forward you to peers *closer* to the target key.
4. You move closer and closer, until you find the right peer.

---

## Visual Flow of a Lookup

```text
Want to find key: 0xABCD

Start at Peer A
 |
 |-- "I don't have it, but Peer B is closer."
 |
V
Peer B
 |
 |-- "I don't have it either, but Peer C is even closer."
 |
V
Peer C
 |
 |-- "I have the value for 0xABCD!"
```

Each step:
- You ask a peer.
- They tell you "I don't know, but here are better peers."
- You **hop** closer until you find the key.

---

## ASCII Diagram: Hopping Closer

```text
Peers organized by distance to Key 0xABCD:

[Peer A] --> [Peer B] --> [Peer C] --> [Peer D]

You start here.
         |
         V
     (Hop, hop, hop)
         |
         V
  Reach Peer D (has the value)
```

- **Each arrow** is a step closer.
- Peers are organized by **key distance** (not physical location).

---

## DHT Example in BitTorrent

- Keys = info-hash of a torrent.
- Values = list of IP addresses of peers who have the file.

Example:

```text
Key: SHA1("ubuntu-22.04.iso")
Value: [IP:Port, IP:Port, IP:Port]
```

You use the DHT to find peers **without** needing a tracker server.

✅ If you are a member of DHT, you do **not need a centralized tracker** anymore!

---

## Key Ideas

- A DHT **spreads** storage and **shares** the load across the network.
- Lookup is **like guided search** (asking smarter and smarter peers).
- Hashing makes IDs and keys **balanced and fair**.
- XOR distance makes **finding the right peer fast and efficient**.
- DHTs **self-heal** when nodes go offline.
- Being part of the DHT **eliminates the need for a tracker**.

---

# End of Presentation

