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
    "go": {
        # Signed integers
        "int": "int",
        "int8": "int8",
        "int16": "int16",
        "int32": "int32",
        "int64": "int64",
        # Unsigned integers
        "uint": "uint",
        "uint8": "uint8",
        "uint16": "uint16",
        "uint32": "uint32",
        "uint64": "uint64",
        "byte": "byte",
        "rune": "rune",
        # Floating-point numbers
        "float32": "float32",
        "float64": "float64",
        # Boolean & String
        "bool": "bool",
        "string": "string",
    },
    "c": {
        # Signed integers (<stdint.h>)
        "int": "int",
        "int8": "int8_t",
        "int16": "int16_t",
        "int32": "int32_t",
        "int64": "int64_t",
        # Unsigned integers (<stdint.h>)
        "uint": "unsigned int",
        "uint8": "uint8_t",
        "uint16": "uint16_t",
        "uint32": "uint32_t",
        "uint64": "uint64_t",
        "byte": "uint8_t",
        "rune": "int32_t",
        # Floating-point numbers
        "float32": "float",
        "float64": "double",
        # Boolean (<stdbool.h>)
        "bool": "bool",
        "string": "char",
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
for stream_file in ["stream.h", "stream.c"]:
    shutil.copy(f"./templates/{stream_file}", f"./codegen/{stream_file}")
