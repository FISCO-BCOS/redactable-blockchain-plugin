#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: bash write_new_block.sh <hash>"
    exit 1
fi

target_file="/home/test/fisco/Redactable-Blockchain/new_block.txt"
if [ ! -e "$target_file" ]; then
    echo "Error: $target_file does not exist."
    exit 1
fi

content=$(cat "$target_file")
thash="$1"

./storageTool -w s_hash_2_tx "$thash" "$content" -H 1
