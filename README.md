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
    ├── server_stub.h.j2     # template: server handler + dispatch declarations
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

Writing serialize and deserialize functions by hand for every struct gets repetitive, and it is easy to make the two sides disagree. The code generator removes that work. You describe your data and your remote functions once in YAML, and `pyprotoc.py` produces matching C code for both the client and the server. This is the same idea that tools like Protocol Buffers and gRPC use, written small enough to read in one sitting.

#### Run the generator

The script reads `mult.yaml` and the templates from the current directory, so run it from inside `rpc/`:

```bash
cd rpc
python3 pyprotoc.py c
```

The argument `c` picks the target language. A `go` type map also exists in the script, but only C templates are written so far. For every service in the YAML, the generator writes these files into `rpc/codegen/` (shown here for the `Calculator` service):

| Generated file | What it contains |
| --- | --- |
| `calculator.h` | The message header struct, the sentinel helpers, one struct per YAML type, and the serialize/deserialize declarations |
| `calculator.c` | The serialize/deserialize implementations |
| `calculator_client_stub.h` | One client function per RPC, for example `multiply()` |
| `calculator_server_stub.h` | One handler per RPC for you to implement, plus `calculator_dispatch()` |

#### Describe your types and services in YAML

Here is the schema that ships in `rpc/mult.yaml`:

```yaml
package: calc

types:
  - name: MulRequest
    fields:
      - { id: 1, name: a, type: int32 }
      - { id: 2, name: b, type: int32 }

  - name: MulReply
    fields:
      - { id: 1, name: result, type: int32 }

  - name: Person
    fields:
      - { id: 1, name: id, type: int32 }
      - { id: 2, name: name, type: string, maxLength: 30 }

services:
  - name: Calculator
    rpcs:
      - { id: 1, name: Multiply, request: MulRequest, response: MulReply }
```

A field's `type` can be one of three things:

1. **A primitive type** such as `int32`, `uint8`, `float64` or `bool`. The script maps each one to a C type (`int32` becomes `int32_t`, `float64` becomes `double`, and so on). The full list is in `language_datastructure_dict` in `pyprotoc.py`.
2. **A string.** Strings must have a `maxLength`, because they become fixed-size character arrays (see Design rules for the reason).
3. **Another type from the same YAML file.** This is how you nest one message inside another. The nested type must be defined earlier in the file than the type that uses it. This rule also stops a type from containing itself, which would be a struct of infinite size.

The generator checks these rules before writing anything. If a string has no `maxLength`, or a field uses a type the script does not know, it prints an error and exits with status 1.

The `id` on each RPC matters: it is the number the client puts in the message header so the server knows which function to call. The `id` on fields is not used by the generator yet.

#### What the generated code looks like

For the `Person` type above, the generator writes this struct and these two functions into `calculator.h`:

```c
struct person_t {
    int32_t id;
    char name[30];
};

void serialize_person_t(struct person_t *obj, ser_buff_t *buff);
struct person_t *deserialize_person_t(ser_buff_t *buff);
```

The naming follows one convention everywhere: a YAML type called `Person` becomes `struct person_t`, `serialize_person_t` and `deserialize_person_t`. Deserialize allocates the struct on the heap with `calloc`, so the caller must `free` it.

If you added an `Occupation` type and gave `Person` a field `{ id: 3, name: occ, type: Occupation }`, the struct would hold the occupation directly inside it (by value), and the serializer would call the occupation's serializer for that field:

```c
struct person_t {
    int32_t id;
    char name[30];
    struct occupation_t occ;
};

// inside serialize_person_t:
serialize_occupation_t(&obj->occ, buff);
```

On the way back in, `deserialize_person_t` calls `deserialize_occupation_t`, copies the result into `obj->occ`, and frees the temporary copy. That costs one extra allocation per nested field, which is fine for a learning project and keeps every type using the same pair of functions.

#### How a message looks on the wire

Every message, request or reply, has two parts: a fixed 22-byte header followed by the payload.

The header tells the receiver what kind of message this is and how many bytes to read next:

| Field | Size | Meaning |
| --- | --- | --- |
| `magic` | 10 bytes | Always the ASCII text `V4$U8@N5@!` (no terminating zero). If these bytes do not match, the message is rejected. |
| `version` | 2 bytes | Protocol version, currently `1`. |
| `rpc_id` | 4 bytes | Which RPC this message is for. This is the `id` from the YAML. |
| `status` | 2 bytes | `1` for success, `0` for failure. This is meaningful on replies. |
| `payload_len` | 4 bytes | How many bytes of payload follow the header. |

`RPC_HEADER_SIZE` is defined as 22, which is the size on the wire. This is different from `sizeof(struct rpc_header_t)`, which is 24 on a typical 64-bit machine because the compiler adds padding between fields in memory. The header is serialized field by field, so the padding never reaches the network.

The payload is the request or reply struct, serialized field by field. Every serialized object starts with a 4-byte sentinel. The value `0x00000000` means "an object follows", and `0xFFFFFFFF` means "this object is NULL". Because the sentinel is always written, the receiver reads it first and never confuses real field data with the NULL marker. An earlier version only wrote a sentinel for NULL objects, and an object whose first field happened to be `-1` was decoded as NULL.

#### Walkthrough: the bytes for `Multiply(6, 7)`

Here are the real 34 bytes the generated code produces for a `MulRequest` with `a = 6` and `b = 7`, captured on an arm64 Mac (little-endian):

```text
56 34 24 55 38 40 4e 35 40 21   magic "V4$U8@N5@!"   (10 bytes)
01 00                           version 1            (2 bytes)
01 00 00 00                     rpc_id 1 (Multiply)  (4 bytes)
01 00                           status 1             (2 bytes)
0c 00 00 00                     payload_len 12       (4 bytes)
00 00 00 00                     sentinel: present    (4 bytes)
06 00 00 00                     a = 6                (4 bytes)
07 00 00 00                     b = 7                (4 bytes)
```

The payload is 12 bytes: 4 for the sentinel and 4 each for `a` and `b`. Adding the 22-byte header gives 34 bytes in total. The reply for this call would carry a header with the same `rpc_id`, a `payload_len` of 8 (sentinel plus `result`), and then `result = 42`.

Strings are always sent at their full declared size. A `name` field with `maxLength: 30` holding `"vasu"` sends the 4 letters, a terminating zero, and 25 more bytes, for 30 bytes in total. This wastes some space but means the receiver never has to read a length first or allocate memory for the string, because the space is already part of the struct.

To build a message, the sender has to know the payload length before it can write the header. The trick used here is to skip `RPC_HEADER_SIZE` bytes at the start of the buffer, serialize the payload, work out the payload length from the buffer cursor, then move the cursor back to 0 and write the header into the space that was reserved.

#### The stubs (declared now, implementation coming next)

A stub is the piece of code that makes a remote call look like an ordinary function call. The headers for both sides are generated today. The `.c` implementations are the next piece of work, so the code below shows how they are meant to be used once they exist.

On the client side, you connect a socket and call the generated function as if multiplication happened locally:

```c
#include "calculator_client_stub.h"
#include "network_common.h"

int sock = create_send_socket("127.0.0.1", 8080);

struct mul_request_t req = { .a = 6, .b = 7 };
struct mul_reply_t *reply = multiply(sock, &req);

if (reply) {
    printf("6 * 7 = %d\n", reply->result);   // prints 42
    free(reply);
}
close(sock);
```

Inside `multiply()`, the stub will build the request as described in the walkthrough, send it, read the 22-byte reply header, check the magic, the status and the `rpc_id`, read `payload_len` more bytes, and deserialize the reply. If anything goes wrong, it returns NULL.

On the server side, you write only the actual logic, one handler per RPC:

```c
#include "calculator_server_stub.h"

struct mul_reply_t *multiply_handler(struct mul_request_t *req) {
    struct mul_reply_t *reply = calloc(1, sizeof(*reply));
    reply->result = req->a * req->b;
    return reply;   // the stub sends it and then frees it
}
```

Returning NULL from a handler tells the stub to send a failure reply. The generated `calculator_dispatch()` does everything else. Your server's main loop accepts a connection and keeps calling it until the client goes away:

```c
int listen_sock = create_listening_socket("127.0.0.1", 8080);

while (1) {
    int conn = accept(listen_sock, NULL, NULL);
    while (calculator_dispatch(conn) == 0) {
        // each call handles exactly one request on this connection
    }
    close(conn);
}
```

One call to `calculator_dispatch()` will:

1. Read the 22-byte header and check the magic.
2. Read `payload_len` bytes of payload.
3. Look at `rpc_id` and pick the matching handler. For `rpc_id` 1 it deserializes a `MulRequest` and calls `multiply_handler`. This choice is a generated `switch`, with one `case` per RPC in the YAML.
4. Serialize the handler's reply behind a header with the same `rpc_id`, set `status` to 1, and send it. If the handler returned NULL, or the `rpc_id` is unknown, it sends a header with `status` 0 and `payload_len` 0 instead.
5. Return 0 if the connection can be used for another request, or -1 if reading or sending failed, in which case the caller should close the connection.

The client and server functions have different names (`multiply` and `multiply_handler`) so that a single test program can link both sides without a duplicate symbol error.

#### Design rules

These choices keep the generated code small and keep the wire format usable from other languages later, such as Go:

- **No pointers inside generated structs.** Every field has a fixed size, so a struct can be filled directly from the bytes on the wire. A Go version of the same schema can then produce byte-for-byte the same messages.
- **Nested types are stored by value.** A nested field is a real struct inside the parent struct, not a pointer to one. A nested field therefore can never be NULL.
- **Strings need a `maxLength`.** This follows from the first rule: a fixed-size character array has no pointer, so the string travels inside the struct.
- **Nested types are serialized field by field, never copied as one block of memory.** Copying a whole struct with `memcpy` would also copy the compiler's padding bytes, and another language would lay the same struct out differently.

#### Known limitations

These are understood and left for later:

- Numbers are sent in the byte order of the machine that sends them. Both ends must use the same byte order, which is true for two processes on the same laptop but not guaranteed across machines.
- When a string fills all of its `maxLength` characters, there is no room for a terminating zero, and the receiver gets a string that `printf` or `strlen` will read past the end of.
- `deserialize_data` in `stream.c` does not check that enough bytes are left in the buffer, so a short or corrupted message can make it read past the end.
- The message header and sentinel helpers are generated into each service's header. A program that includes two services would define them twice.
- Client stub names are not prefixed with the service name, so two services with an RPC of the same name would clash.

---

## How this points toward RPC

A minimal RPC call looks like this end to end:

1. The client packs arguments into a `ser_buff_t`. The generated `serialize_*` functions do this.
2. The client sends those bytes over a socket, behind a header that names the RPC. The helpers in `networking/` move the bytes, and the generated header code builds the header.
3. The server receives the bytes, deserializes them, and calls the real local function. This is the job of `calculator_dispatch()` and your handler.
4. The server serializes the return value and sends it back.
5. The client deserializes the reply. This is the job of the client stub.

Everything except the two stub implementations is in place: the serialization code and the message header are generated, the networking helpers exist, and the stub declarations fix the shape of the API. The remaining work is writing the client stub and `calculator_dispatch()` as described in The stubs.

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
| `Error: Person.occ has unknown type 'Occupation' (nested types must be defined before use)` | The type is misspelled, or it is defined below the type that uses it in the YAML |
| `deserialize_rpc_header_t` returns NULL | The first 10 bytes are not `V4$U8@N5@!`: the sender did not write a header, or the reader is not at the start of a message |
| Fields come back with wrong values on the other side | The sender and receiver were generated from different versions of the YAML, so their field order or sizes do not match; regenerate both from the same file |
