## Building RPC from scratch in C

This repo is a hands-on learning project. The goal is to understand the pieces that sit under a typical RPC stack by writing them yourself in C, instead of jumping straight into a framework.

RPC (Remote Procedure Call) is the idea that one process can call a function that lives in another process, often on another machine, and get a result back. To make that work you need at least two building blocks:

1. A way to turn in-memory data into a byte stream (and turn that stream back into data). That is serialization and deserialization.
2. A way to move those bytes between processes over the network.

This project currently covers those two foundations. The code is split into modules so each idea stays readable on its own. Later pieces (request framing, procedure dispatch, stubs) can build on top of what is already here.

#### What you need

- A C compiler (`clang` or `gcc`)
- CMake 3.10 or newer
- `make`
- A Unix-like system with TCP sockets (macOS and Linux both work)

Checked on macOS with AppleClang and CMake 4.x (November 2026).

#### Project layout

```
.
├── Makefile                 # thin wrapper around CMake
├── CMakeLists.txt           # top-level project; pulls in both modules
├── serdes/                  # serialization / deserialization
│   ├── stream.h / stream.c  # growable byte buffer + read/write helpers
│   ├── primitive_ds_ser_deser.c
│   ├── struct_ds_ser_deser.c
│   ├── nested_struct_ds_ser_deser.c
│   └── nested_pointer_struct_ds_ser_deser.c
└── networking/              # TCP client and server helpers
    ├── network_common.h / network_common.c
    ├── client.c
    └── server.c
```

Build outputs land under `build/`, so the source tree stays clean.

#### Build everything

From the repo root:

```bash
make
```

That configures CMake into `build/` and compiles both modules. You should see targets for the serdes demos plus `client` and `server`.

To wipe the build tree:

```bash
make clean
```

---

## Serialization (`serdes/`)

The shared piece is `ser_buff_t` in `stream.h`. It is a simple byte buffer with a cursor:

- `b` points at the start of the allocated memory
- `size` is the total capacity
- `next` is the offset where the next read or write happens

`serialize_data` copies bytes into the buffer and advances the cursor. `deserialize_data` copies bytes out and advances the cursor the same way. To read what you just wrote, reset `buff->next` to `0` and deserialize in the same order you serialized.

#### Walkthrough: packing an int, a string, and a float

Open `serdes/primitive_ds_ser_deser.c`. The program does this, in order:

1. Create a buffer with `init_serialized_buffer`.
2. Write an `int` (`42`), a short C string (`"hello"`), and a `float` (`3.14`).
3. Print how many bytes were written.
4. Reset the cursor to the start.
5. Read the three values back into new locals.
6. Print them and compare against the originals.

Run it:

```bash
make run-primitive
```

Example output:

```text
wrote 14 bytes (capacity 100)
int:   42 (want 42)
str:   hello (want hello)
float: 3.140000 (want 3.14)
PASS
```

Fourteen bytes matches `sizeof(int) + sizeof("hello") + sizeof(float)` on a typical 64-bit machine (`4 + 6 + 4`). The important lesson is that the wire format is just the raw bytes you chose to write, in the order you chose to write them. Both sides of an RPC call must agree on that layout.

#### Structs and nested data

The other demos build on the same buffer API:

| Command | What it practices |
| --- | --- |
| `make run-struct` | Serialize a flat `person_t` (name, age, weight) |
| `make run-nested-struct` | Serialize a person that embeds an `occupation_t` by value |
| `make run-nested-pointer-struct` | Same idea, but occupation is a pointer (NULL sentinel on the wire) |

Example (`make run-struct`):

```text
Printing deserialized person_t
	Vasu (should be Vasu)
	28 (should be 28)
	85 (should be 85)
```

Example (`make run-nested-struct` and `make run-nested-pointer-struct`):

```text
Printing deserialized person_t
	Vasu (should be Vasu)
	85 (should be 85)
	28 (should be 28)
	AI Platform (should be AI Platform)
	99 (should be 99)
```

Nested pointer serialization is the closer cousin of real RPC payloads: heap objects cannot be sent as raw addresses, so you write a sentinel for NULL and otherwise write the fields the pointed-to object contains.

---

## Networking (`networking/`)

This module is a small TCP helper layer:

- `create_listening_socket` binds and listens
- `create_send_socket` connects as a client
- `send_data` / `recv_data` move a fixed-size byte blob

The demo server listens on `127.0.0.1:8080`. The demo client connects and sends `"Hello world!"`.

#### Run the demo (two terminals)

Terminal 1, start the server first:

```bash
make run-server
```

```text
Server listening on port 8080
```

Terminal 2, run the client:

```bash
make run-client
```

```text
Send 30 bytes
```

Back in the server terminal you should see something like:

```text
Client connected succesfully
Recv 30 bytes
server received: Hello world!
```

The client and server today exchange a fixed 30-byte buffer. That is intentional for learning: once you can move raw bytes, the next step is to put a serialized `ser_buff_t` payload inside that send/recv path.

---

## How this points toward RPC

A minimal RPC call looks like this end to end:

1. The client packs arguments into a `ser_buff_t` (the demos in `serdes/` already do this part).
2. The client sends those bytes over a socket (the demos in `networking/` already do the send/recv part).
3. The server receives the bytes, deserializes them, and calls the real local function.
4. The server serializes the return value and sends it back.
5. The client deserializes the reply.

You already have steps 1, 2, 4, and 5 in separate form. The remaining work is mostly glue: a shared message header (procedure id, payload length), matching stubs on both sides, and agreement on byte order and field layout.

---

## Common issues

| What you see | Likely cause |
| --- | --- |
| `cmake: command not found` | CMake is not installed, or not on your `PATH` |
| Build fails right after a file move | Stale `build/` tree; run `make clean && make` |
| `make run-client` hangs or fails to connect | Server is not running yet, or something else already bound port `8080` |
| `Address already in use` from the server | A previous server process is still holding the port; stop it and try again |
| Deserialized values look like garbage | Cursor was not reset before reading, or serialize/deserialize field order does not match |
| Nested pointer demo crashes or returns NULL occupancy | The occupation pointer was NULL on the sender side, so the wire sentinel correctly produced a NULL on the receiver |
| Networking demo prints only part of a string | Send and recv sizes must match the buffer you allocated; the current demos use a fixed 30-byte length |
