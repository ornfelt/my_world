#!/usr/bin/env python3
import subprocess
import typing
from os import path
from pathlib import Path
from subprocess import run as run_cmd

SCRIPT_DIR = Path(path.dirname(path.realpath(__file__)))
INCLUDE_DIR = SCRIPT_DIR / "include" / "wow_srp"
CPP_INCLUDE_DIR = SCRIPT_DIR / "wow_srp_cpp" / "include" / "wow_srp_cpp"

DEFINES_TO_FILES: dict[str, str] = {
    "VALUES": "wow_srp",
    "CLIENT": "client",
    "SERVER": "server",
    "WRATH": "wrath",
    "TBC": "tbc",
    "VANILLA": "vanilla"
}


def define_to_file(define: str) -> str:
    for key in DEFINES_TO_FILES:
        if key in define:
            return DEFINES_TO_FILES[key]

    raise Exception(f"invalid define '{define}'")


def split_includes(output: str) -> dict[str, list[str]]:
    C_INCLUDES = ["#pragma once", "", "#include <stdint.h>", ""]

    files = {}
    for value in DEFINES_TO_FILES.values():
        files[value] = C_INCLUDES.copy()

    current_file: typing.Optional[str] = None

    for line in output.splitlines():
        # C89 compatibility of headers
        line = line.replace('// extern "C"', '/* extern "C" */')
        line = line.replace('// __cplusplus', '/* __cplusplus */')
        if "#include" in line:
            continue
        elif line.strip() == "":
            continue
        elif "#if defined" in line:
            current_file = define_to_file(line)
        elif "WOW_SRP_LARGE_SAFE_PRIME_LITTLE_ENDIAN" in line:
            files["wow_srp"].append(line)
        elif "extern \"C\"" in line or "__cplusplus" in line:
            for value in DEFINES_TO_FILES.values():
                files[value].append(line)
        elif "#endif" in line:
            files[current_file].append('\n')
            current_file = None

        elif current_file is not None:
            files[current_file].append(line)
        else:
            raise Exception(f"invalid line '{line}'")

    return files


def write_files(files: dict[str, list[str]]):
    for file, content in files.items():
        content = '\n'.join(content)
        content += '\n'
        filename = f"{file}.h"
        with open(INCLUDE_DIR / filename, "w") as f:
            f.write(content)


def write_cpp_general(content: list[str]):
    file_contents = []

    for line in content:
        if "#include <stdint.h>" in line:
            file_contents.append("#include <cstdint>")
            file_contents.append("#include <array>")
            file_contents.append("\n")
            file_contents.append("namespace wow_srp {")
        elif "#define " in line:
            line = line.split(" ")
            name = line[1].replace("WOW_SRP_", "")
            value = line[2]
            file_contents.append(f"constexpr int {name} = {value};")
        elif "extern const uint8_t" in line:
            file_contents.append(
                "constexpr std::array<uint8_t, 32> LARGE_SAFE_PRIME {0xb7, 0x9b, 0x3e, 0x2a, 0x87, 0x82, 0x3c, 0xab, 0x8f, 0x5e, 0xbf, 0xbf, 0x8e, 0xb1, 0x1, 0x8, 0x53, 0x50, 0x6, 0x29, 0x8b, 0x5b, 0xad, 0xbd, 0x5b, 0x53, 0xe1, 0x89, 0x5e, 0x64, 0x4b, 0x89};")
            file_contents.append("\n")
        elif "__cplusplus" in line or "extern \"C\"" in line:
            continue
        else:
            file_contents.append(line)

    file_contents.append("using ProofArray = std::array<uint8_t, PROOF_LENGTH>;")
    file_contents.append("")
    file_contents.append("using SessionKeyArray = std::array<uint8_t, SESSION_KEY_LENGTH>;")
    file_contents.append("")
    file_contents.append("using KeyArray = std::array<uint8_t, KEY_LENGTH>;")
    file_contents.append("")
    file_contents.append("using ReconnectDataArray = std::array<uint8_t, RECONNECT_DATA_LENGTH>;")
    file_contents.append("")

    file_contents.append("} // namespace wow_srp")

    with open(CPP_INCLUDE_DIR / "wow_srp.hpp", "w") as f:
        f.write("\n".join(file_contents))


def main():
    c_output = run_cmd(["cbindgen", "--cpp-compat", "--lang", "c"], stdout=subprocess.PIPE, check=True).stdout.decode(
        'utf-8')
    files = split_includes(c_output)
    write_files(files)
    write_cpp_general(files["wow_srp"])


if __name__ == "__main__":
    main()
