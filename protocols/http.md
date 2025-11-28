# HTTP — A Gentle but Detailed Introduction

HTTP, the **HyperText Transfer Protocol**, is the conversational glue of the web: a structured exchange of text messages between a client and a server.
One side asks, the other answers. No persistent memory, no long-term attachment — each request is a postcard the server reads, processes, and forgets.

---

# 1. The Structure of HTTP

HTTP uses **request → response** exchanges over a TCP connection (usually ports 80 or 443).

A **client** (browser, curl, your Python script, your students’ NC-based lab tool) sends a request.
The **server** replies with a response.

No “session” is built into the protocol itself — state is added through cookies, tokens, or session stores.

---

# 2. HTTP Request — What the Client Sends

A request consists of:

1. **Request line**
2. **Headers**
3. *(Optional)* Body

Example:

```
POST /login HTTP/1.1
Host: example.com
User-Agent: curl/8.2
Content-Type: application/json
Content-Length: 34

{"username":"alice","password":"123"}
```

### Request Line

Format:

`METHOD PATH VERSION`

Examples:

- `GET /index.html HTTP/1.1`
- `POST /api/user HTTP/1.1`
- `HEAD /image.jpg HTTP/1.1`
- `DELETE /post/42 HTTP/1.1`

### Common Methods

- **GET** — retrieve a resource
- **POST** — send data (forms, JSON, uploads)
- **PUT** — replace resource
- **PATCH** — partial modification
- **DELETE** — remove resource
- **HEAD** — like GET but without a body
- **OPTIONS** — ask the server what is allowed

---

# 3. HTTP Headers — Metadata That Guides the Conversation

Headers are key–value lines describing the request or response.

They behave like small labels attached to the message envelope: instructions, hints, expectations, warnings.

### Important Request Headers

- **Host:** mandatory in HTTP/1.1
- **User-Agent:** identifies client
- **Accept:** content types the client prefers
- **Accept-Language:** language preferences
- **Accept-Encoding:** gzip, br, deflate
- **Content-Type:** MIME type of body (`application/json`, `multipart/form-data`, etc.)
- **Content-Length:** size of the body (if known)
- **Authorization:** credentials for login
- **Cookie:** key-value pairs stored by the server

### Important Response Headers

- **Content-Type:** MIME type of response
- **Content-Length:** size of body
- **Server:** server software version
- **Cache-Control:** caching rules
- **ETag:** version identifier for resource (helps caching)
- **Set-Cookie:** sets cookies in browser
- **Location:** for redirects (301/302/etc.)
- **Transfer-Encoding:** `chunked` when length unknown

Headers end with a blank line (`\r\n\r\n`). After that, the body begins (if present).

---

# 4. HTTP Response — What the Server Sends Back

An HTTP response has:

1. **Status line**
2. **Headers**
3. **Body**

Example:

```
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 14

Hello, world!
```

### Status Codes

- **2xx Success:** 200 OK, 201 Created
- **3xx Redirection:** 301, 302, 304
- **4xx Client Errors:** 400, 403, 404
- **5xx Server Errors:** 500, 503

---

# 5. Basic Authentication

BasicAuth is the simplest HTTP authentication scheme.
The client sends a header:

```
Authorization: Basic <BASE64(username:password)>
```

Example (username `alice`, password `secret`):

`alice:secret` → Base64 → `YWxpY2U6c2VjcmV0`

So request contains:

```
Authorization: Basic YWxpY2U6c2VjcmV0
```

**Important:** It is *not encryption*.
It is just Base64 — a mild disguise.
BasicAuth is only safe over **HTTPS**, where TLS actually protects it.

Servers challenge clients by sending:

```
HTTP/1.1 401 Unauthorized
WWW-Authenticate: Basic realm="Members"
```

The browser (or curl) then retries with the Authorization header.

---

# 6. Transfer-Encoding: chunked vs non-chunked

HTTP needs a way to tell the client: *how long is the body?*
There are two broad approaches.

## A. Non-chunked — Known Length (Using Content-Length)

When the server **knows the exact size** of the response body, it sends:

```
Content-Length: 1234
```

The client reads exactly 1234 bytes and knows the response is finished.

This is fastest and simplest.

Common for:

- static files
- JSON responses
- known-size content

## B. Chunked Transfer Encoding — Unknown Length

Sometimes the server doesn't know content size ahead of time:

- dynamic streaming
- long-running responses
- server-sent events
- templates rendered on-the-fly

Then it sends:

```
Transfer-Encoding: chunked
```

Instead of one body, the server sends **chunks**, each preceded by the chunk’s size in hex:

```
4
Wiki
5
pedia
0

```

Interpretation:

- Chunk size: `4` → “Wiki”
- Chunk size: `5` → “pedia”
- Chunk size: `0` → termination

This allows:

- starting to send data immediately
- sending data as it becomes available
- avoiding buffering entire page in memory

Chunked transfer is standard in HTTP/1.1.

---

# 7. HTTPS — HTTP in an Encrypted Tunnel

HTTPS is just HTTP carried inside TLS:

- identity verification
- confidentiality
- integrity

The structure of HTTP messages is unchanged.

---

# 8. Tools for Learning

### curl

```
curl -v https://example.com
curl -u alice:secret https://example.com/secure
```

### nc (netcat)

```
printf "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n" | nc example.com 80
```

This lets students touch raw HTTP like archaeologists brushing dust from an ancient protocol tablet.

### Browser DevTools
Network tab → click request → see headers, body, timing.

---

# Summary

HTTP is a line-oriented protocol built from requests, responses, headers, and bodies.
It thrives on simplicity: clear intentions, simple structure, and readable messages.

With understanding of BasicAuth, headers, content length, and chunked transfer, students can begin to see HTTP not as magic but as a sequence of honest text messages traveling over TCP.

