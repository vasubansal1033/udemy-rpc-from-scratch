## Building RPC from scratch in C

This repo is a hands-on learning project. The goal is to understand the pieces that sit under a typical RPC stack by writing them yourself in C, instead of jumping straight into a framework.

RPC (Remote Procedure Call) is the idea that one process can call a function that lives in another process, often on another machine, and get a result back. To make that work you need at least two building blocks:

1. A way to turn in-memory data into a byte stream (and turn that stream back into data). That is serialization and deserialization.
2. A way to move those bytes between processes over the network.

The first two modules (`serdes/` and `networking/`) cover those two foundations by hand. The third module (`rpc/`) is a small code generator: you describe your messages and services in a YAML file, and it writes the C structs, the serialization code, a message header, and the stub declarations for you. The code is split into modules so each idea stays readable on its own.

#### What you need

- A C compiler (`clang` or `gcc`)
- CMake 3.10 or newer
- `make`
- A Unix-like system with TCP sockets (macOS and Linux both work)
- For the code generator in `rpc/`: Python 3 with `pyyaml` and `jinja2` (`pip install pyyaml jinja2`)

Checked on macOS with AppleClang and CMake 4.x (November 2026). The code generator was checked with Python 3.14.7, PyYAML 6.0.3 and Jinja2 3.1.6 on 2026-11-16.

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
├── networking/              # TCP client and server helpers
│   ├── network_common.h / network_common.c
│   ├── client.c
│   └── server.c
└── rpc/                     # YAML-driven code generator
    ├── mult.yaml            # schema: message types and services
    ├── pyprotoc.py          # reads the YAML, renders the templates
    ├── service_types.h.j2   # template: structs, rpc header, declarations
    ├── service_types.c.j2   # template: serialize / deserialize functions
    ├── client_stub.h.j2     # template: client stub declarations
    ├── server_stub.h.j2     # template: server handler declarations
    └── codegen/             # generated output (plus a copy of stream.h / stream.c)
```

Build outputs land under `build/`, so the source tree stays clean. The `rpc/` module is not part of the CMake build yet; see RPC code generator for how to run it.

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

## RPC code generator (`rpc/`)

Writing serialize and deserialize functions by hand for every struct is repetitive, and it is easy for the client and server to disagree. The generator solves this: you describe your messages and RPCs once in YAML, and `pyprotoc.py` writes matching C code for both sides.

#### Run it

The script reads `mult.yaml` and the templates from the current directory, so run it from inside `rpc/`:

```bash
cd rpc
python3 pyprotoc.py c
```

For the `Calculator` service it writes four files into `rpc/codegen/`:

| File | Contents |
| --- | --- |
| `calculator.h` / `calculator.c` | The message header, one struct per YAML type, and their serialize/deserialize functions |
| `calculator_client_stub.h` | One client function per RPC, for example `multiply()` |
| `calculator_server_stub.h` | One handler per RPC for you to implement, for example `multiply_handler()` |

#### Describe your messages in YAML

```yaml
types:
  - name: MulRequest
    fields:
      - { id: 1, name: a, type: int32 }
      - { id: 2, name: b, type: int32 }
  - name: MulReply
    fields:
      - { id: 1, name: result, type: int32 }

services:
  - name: Calculator
    rpcs:
      - { id: 1, name: Multiply, request: MulRequest, response: MulReply }
```

A YAML type `MulRequest` becomes `struct mul_request_t` with `serialize_mul_request_t()` and `deserialize_mul_request_t()`. Deserialize allocates the struct, so the caller frees it. A field type can be a primitive (`int32`, `float64`, `bool`, and so on), a `string` with a `maxLength`, or another type defined earlier in the same file, which gets embedded inside the parent struct. The generator exits with an error if a string has no `maxLength` or a type is unknown.

#### How a message looks on the wire

Every message is a 22-byte header followed by the payload:

| Field | Size | Meaning |
| --- | --- | --- |
| `magic` | 10 bytes | The ASCII text `V4$U8@N5@!`; anything else is rejected |
| `version` | 2 bytes | Currently `1` |
| `rpc_id` | 4 bytes | The RPC `id` from the YAML, so the server knows which function to run |
| `status` | 2 bytes | `1` for success, `0` for failure |
| `payload_len` | 4 bytes | Number of payload bytes after the header |

In the payload, every object starts with a 4-byte sentinel (`0x00000000` for present, `0xFFFFFFFF` for NULL), followed by its fields in YAML order. Strings are always sent at their full `maxLength`.

Here are the real 34 bytes the generated code produces for `Multiply(6, 7)`, captured on an arm64 Mac:

```text
56 34 24 55 38 40 4e 35 40 21   magic "V4$U8@N5@!"
01 00                           version 1
01 00 00 00                     rpc_id 1 (Multiply)
01 00                           status 1
0c 00 00 00                     payload_len 12
00 00 00 00                     sentinel: present
06 00 00 00                     a = 6
07 00 00 00                     b = 7
```

#### The stubs (implementation coming next)

Only the stub headers are generated today. Once the `.c` files exist, a client will call a remote function like a local one:

```c
int sock = create_send_socket("127.0.0.1", 8080);

struct mul_request_t req = { .a = 6, .b = 7 };
struct mul_reply_t *reply = multiply(sock, &req);   // NULL on failure
if (reply) {
    printf("%d\n", reply->result);                  // 42
    free(reply);
}
```

On the server, you write only the logic. Returning NULL means the call failed.

```c
struct mul_reply_t *multiply_handler(struct mul_request_t *req) {
    struct mul_reply_t *reply = calloc(1, sizeof(*reply));
    reply->result = req->a * req->b;
    return reply;
}
```

#### Design rules and known gaps

Generated structs contain no pointers, so every message has a fixed layout that a future Go generator can match byte for byte. That is also why strings need a `maxLength`.

Left for later: numbers are sent in the sender's byte order, a string that fills its whole `maxLength` has no terminating zero, and `deserialize_data` does not check for short messages.

---

## How this points toward RPC

An RPC call is five steps: the client serializes the arguments, sends them behind a header, the server deserializes them and runs the handler, then sends the serialized result back for the client to deserialize. Serialization, the header and the networking helpers all exist now. The remaining work is the two stub implementations that connect them.

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
| `ModuleNotFoundError: No module named 'yaml'` (or `'jinja2'`) | The generator's Python packages are missing; run `pip install pyyaml jinja2` |
| `FileNotFoundError: ... 'mult.yaml'` or `TemplateNotFound` | `pyprotoc.py` was run from outside `rpc/`; it reads its inputs from the current directory |
| `Error: Person.name is a string without maxLength` | A `string` field in the YAML has no `maxLength`; every string needs one |
| `deserialize_rpc_header_t` returns NULL | The first 10 bytes are not `V4$U8@N5@!`: the sender did not write a header, or the reader is not at the start of a message |
