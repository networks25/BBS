---

## XMPP Connection: Visual Flow

```text
(Client)                                  (Server)
    |                                          |
    | --[Open XML <stream>]------------------->|
    |                                          |
    |<--[Server <stream> Response]--------------|
    |                                          |
    |<--[Supported SASL Mechanisms]-------------|
    |                                          |
    | --[SASL Auth Request (e.g., PLAIN)]------>|
    |                                          |
    |<--[SASL Success]--------------------------|
    |                                          |
    | --[Re-open XML <stream>]----------------->|
    |                                          |
    |<--[Stream Features (Bind, Session)]-------|
    |                                          |
    | --[Bind Resource Request]---------------->|
    |<--[Bind Resource Response (JID)]----------|
    |                                          |
    | --[Session Start Request]---------------->|
    |<--[Session Confirmation]------------------|
    |                                          |
    | --[Presence Available]------------------->|
```

- After this, messaging and further presence updates can start.

---

