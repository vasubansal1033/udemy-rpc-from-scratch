import re
import shutil
import sys
from pathlib import Path

import yaml
from jinja2 import Environment, FileSystemLoader

# Ensure at least one argument was provided
if len(sys.argv) < 2:
    print("Usage: python pyprotoc.py <language-name>")
    sys.exit(1)


def to_lower_snake(text):
    # Inserts an underscore before capital letters and converts to uppercase
    return re.sub(r"(?<!^)(?=[A-Z])", "_", text).lower()


def to_upper_snake(text):
    # Inserts an underscore before capital letters and converts to uppercase
    return re.sub(r"(?<!^)(?=[A-Z])", "_", text).upper()


language_name = sys.argv[1].lower()
language_datastructure_dict = {
    # "type": name in the target language, "size": bytes on the wire.
    # string size is per character; total is size * maxLength.
    # Assuming 64 bit architecture
    "go": {
        # Signed integers
        "int": {"type": "int", "size": 8},
        "int8": {"type": "int8", "size": 1},
        "int16": {"type": "int16", "size": 2},
        "int32": {"type": "int32", "size": 4},
        "int64": {"type": "int64", "size": 8},
        # Unsigned integers
        "uint": {"type": "uint", "size": 8},
        "uint8": {"type": "uint8", "size": 1},
        "uint16": {"type": "uint16", "size": 2},
        "uint32": {"type": "uint32", "size": 4},
        "uint64": {"type": "uint64", "size": 8},
        "byte": {"type": "byte", "size": 1},
        "rune": {"type": "rune", "size": 4},
        # Floating-point numbers
        "float32": {"type": "float32", "size": 4},
        "float64": {"type": "float64", "size": 8},
        # Boolean & String
        "bool": {"type": "bool", "size": 1},
        "string": {"type": "string", "size": 1},
    },
    "c": {
        # Signed integers (<stdint.h>)
        "int": {"type": "int", "size": 4},
        "int8": {"type": "int8_t", "size": 1},
        "int16": {"type": "int16_t", "size": 2},
        "int32": {"type": "int32_t", "size": 4},
        "int64": {"type": "int64_t", "size": 8},
        # Unsigned integers (<stdint.h>)
        "uint": {"type": "unsigned int", "size": 4},
        "uint8": {"type": "uint8_t", "size": 1},
        "uint16": {"type": "uint16_t", "size": 2},
        "uint32": {"type": "uint32_t", "size": 4},
        "uint64": {"type": "uint64_t", "size": 8},
        "byte": {"type": "uint8_t", "size": 1},
        "rune": {"type": "int32_t", "size": 4},
        # Floating-point numbers
        "float32": {"type": "float", "size": 4},
        "float64": {"type": "double", "size": 8},
        # Boolean (<stdbool.h>)
        "bool": {"type": "bool", "size": 1},
        "string": {"type": "char", "size": 1},
    },
}

data = {}
try:
    with open("mult.yaml", "r", encoding="utf-8") as file:
        data = yaml.safe_load(file)
except yaml.YAMLError as exc:
    print(f"Error parsing YAML: {exc}")

package_name = data["package"]

types = {}
for i, type in enumerate(data["types"]):
    for field in type["fields"]:
        if field["type"] == "string" and not field.get("maxLength"):
            print(
                f"Error: {type['name']}.{field['name']} is a string without maxLength"
            )
            sys.exit(1)

        # nested types must be defined earlier in the YAML (also rules out self-nesting)
        if (
            field["type"] not in language_datastructure_dict[language_name]
            and field["type"] not in types
        ):
            print(
                f"Error: {type['name']}.{field['name']} has unknown type "
                f"'{field['type']}' (nested types must be defined before use)"
            )
            sys.exit(1)

    types[type["name"]] = {}
    types[type["name"]]["fields"] = type["fields"]

# print(types)
# print(types["MulRequest"])

env = Environment(
    loader=FileSystemLoader("./templates"),
    trim_blocks=True,
    lstrip_blocks=True,
    keep_trailing_newline=True,
)
env.filters["upper_snake"] = to_upper_snake
env.filters["lower_snake"] = to_lower_snake

# 3. Load and render the template
context = {
    "types": types,
    "language_datastructure_dict": language_datastructure_dict[language_name],
}

header_template = env.get_template("service_types.h.j2")
source_template = env.get_template("service_types.c.j2")
client_stub_template = env.get_template("client_stub.h.j2")
server_stub_template = env.get_template("server_stub.h.j2")

for i, service in enumerate(data["services"]):
    # print(data["services"][i])
    service_name = service["name"]

    # generate service_types.h
    context["service_name"] = service_name
    header_code = header_template.render(context)
    source_code = source_template.render(context)

    header_filepath = Path(f"./codegen/{to_lower_snake(service_name)}.h")
    header_filepath.parent.mkdir(parents=True, exist_ok=True)
    with open(header_filepath, "w") as f:
        f.write(header_code)

    source_filepath = Path(f"./codegen/{to_lower_snake(service_name)}.c")
    source_filepath.parent.mkdir(parents=True, exist_ok=True)
    with open(source_filepath, "w") as f:
        f.write(source_code)

    context["rpcs"] = service["rpcs"]
    client_stub_code = client_stub_template.render(context)
    client_stub_filepath = Path(
        f"./codegen/{to_lower_snake(service_name)}_client_stub.h"
    )
    with open(client_stub_filepath, "w") as f:
        f.write(client_stub_code)

    server_stub_code = server_stub_template.render(context)
    server_stub_filepath = Path(
        f"./codegen/{to_lower_snake(service_name)}_server_stub.h"
    )
    with open(server_stub_filepath, "w") as f:
        f.write(server_stub_code)

    for i, rpc in enumerate(service["rpcs"]):
        rpc_id = rpc["id"]
        rpc_name = rpc["name"]
        rpc_request = rpc["request"]
        rpc_response = rpc["response"]

        print(rpc_id, rpc_name, rpc_request, rpc_response)

# generated code depends on the serialization buffer, ship it alongside
for stream_file in ["stream.h", "stream.c", "network_common.c", "network_common.h"]:
    shutil.copy(f"./templates/{stream_file}", f"./codegen/{stream_file}")
