---

## Secret Handshake: Visual Flow

```text
(Client)                                  (Server)
    |                                          |
    | --[Ephemeral Public Key]---------------->|
    |                                          |
    |<--[Server Ephemeral Key + Auth]-----------|
    |                                          |
    | --[Client Authentication]--------------->|
    |                                          |
 (Shared Secret Established, Encrypted Channel)
```

- After the third step, encryption starts.
- Every message is now encrypted with the derived shared key.

---

