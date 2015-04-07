# compile hex.c
./hex FpCvm441_v1132.spihex out.bin /S:1984

# got size from dump of srec_cat -intel <in> <out> -binary
dd if=out.bin of=target.bin bs=1 count=2031527
