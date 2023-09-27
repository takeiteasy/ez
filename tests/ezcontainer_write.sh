#!/usr/bin/env sh
# ./bcf.sh [files] > out.bcf

print_int() {
    printf "%.8x" $1 | sed -E 's/(..)(..)(..)(..)/0: \4\3\2\1/' | xxd -r -g00
}

print_long() {
    printf "%.16x" $1 | sed -E 's/(..)(..)(..)(..)(..)(..)(..)(..)/0: \8\7\6\5\4\3\2\1/' | xxd -r -g00
}

printf "%b" '\x46\x43\x00\x00'
print_int $#

HEADERSZ=$(echo "8 + 32 * $#" | bc)
for VAR in "$@"
do
    HEADERSZ=$(echo "$HEADERSZ + ${#VAR}" | bc)
done
OFFSET=$(echo $HEADERSZ)

for VAR in "$@"
do
    print_long ${#VAR}
    FSZ=$(wc -c < $VAR)
    print_long $FSZ
    print_long $OFFSET
    OFFSET=$(echo "$OFFSET + $FSZ" | bc)
    print_long $(printf "%d" "0x"$(crc32 $VAR))
    printf "%s" $VAR
done

for VAR in "$@"
do
    cat $VAR
done
