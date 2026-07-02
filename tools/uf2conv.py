#!/usr/bin/env python3
import argparse
import os
import struct

UF2_MAGIC_START0 = 0x0A324655
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END = 0x0AB16F30
UF2_FLAG_FAMILY_ID_PRESENT = 0x00002000
UF2_FAMILY_RP2350 = 0x540DDF62  # RP2350 (Pico 2 W)
UF2_BLOCK_SIZE = 512
UF2_PAYLOAD_SIZE = 256


def make_block(data: bytes, target_addr: int, block_no: int, num_blocks: int, file_size: int) -> bytes:
    block = bytearray(UF2_BLOCK_SIZE)
    struct.pack_into('<IIIIIII', block, 0,
                     UF2_MAGIC_START0,
                     UF2_MAGIC_START1,
                     UF2_FLAG_FAMILY_ID_PRESENT,
                     target_addr,
                     len(data),
                     block_no,
                     num_blocks)
    struct.pack_into('<I', block, 28, UF2_FAMILY_RP2350)  # Family ID at offset 28 when flag is set
    block[32:32 + len(data)] = data
    struct.pack_into('<I', block, UF2_BLOCK_SIZE - 4, UF2_MAGIC_END)
    return bytes(block)


def main():
    parser = argparse.ArgumentParser(description='Convert a binary file to UF2 for RP2350')
    parser.add_argument('-b', '--base', type=lambda x: int(x, 0), default=0x1000,
                        help='Flash base address (default: 0x1000)')
    parser.add_argument('-o', '--output', required=True, help='Output UF2 file')
    parser.add_argument('input', help='Input binary file')
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        raise SystemExit(f'Input file not found: {args.input}')

    data = open(args.input, 'rb').read()
    num_blocks = (len(data) + UF2_PAYLOAD_SIZE - 1) // UF2_PAYLOAD_SIZE

    with open(args.output, 'wb') as out:
        for block_no in range(num_blocks):
            chunk = data[block_no * UF2_PAYLOAD_SIZE:(block_no + 1) * UF2_PAYLOAD_SIZE]
            target_addr = args.base + block_no * UF2_PAYLOAD_SIZE
            out.write(make_block(chunk, target_addr, block_no, num_blocks, len(data)))

    print(f'Wrote UF2: {args.output}')


if __name__ == '__main__':
    main()
