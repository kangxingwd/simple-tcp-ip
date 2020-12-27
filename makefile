INSTALL_DIR := out

all: 
	cd source && make

install:
	mkdir -p $(INSTALL_DIR)
	cd source && make install

clean:
	rm -rf out
	cd source && make clean
