import sys
import hashlib

input_file = sys.argv[1]
out_dir = sys.argv[2]

for line in open(input_file):
    input = bytes.fromhex(line)
    if not input:
        continue
    input_hash = hashlib.sha1(input).hexdigest()

    out_file_name = out_dir + "/mainnet-" + input_hash
    with open(out_file_name, "wb") as out_file:
        out_file.write(input)
