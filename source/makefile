CC := gcc
CXX := g++
INCLUDE := ./include
INSTALL_DIR := ../out


all: udp

udp:
	$(CC) -I$(INCLUDE) src/udp.c -o udp_test

install:
	cp udp_test $(INSTALL_DIR)/

clean:
	rm -rf *.o udp_test