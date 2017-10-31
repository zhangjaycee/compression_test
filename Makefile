all:
	make bin_lz4
	make bin_zlib
bin_lz4: lz4.c lz4hc.c test_lz4.c
	gcc -O2 -o bin_lz4 lz4.c lz4hc.c test_lz4.c -I .
bin_zlib: test_zlib.c
	gcc -O2 -o bin_zlib test_zlib.c -lz
clean:
	rm bin*

