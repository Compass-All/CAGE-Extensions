CC ?= aarch64-linux-android-clang
ARCH ?=arm64

all: reg_rw dma_to_device dma_from_device performance test_chrdev

dma_to_device: dma_to_device.o
	$(CC) -lrt -o $@ $<  -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

dma_from_device: dma_from_device.o
	$(CC) -lrt -o $@ $<  -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

performance: performance.o
	$(CC) -o $@ $<  -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

reg_rw: reg_rw.o
	$(CC) -o $@ $<

test_chrdev: test_chrdev.o
	$(CC) -o $@ $<

%.o: %.c
	$(CC) -c -std=c99 -o $@ $<  -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

clean:
	rm -rf reg_rw *.o *.bin dma_to_device dma_from_device performance test_chrdev
