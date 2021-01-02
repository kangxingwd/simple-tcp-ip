include env.mk
PWD := $(shell pwd)
export TARGETDIR = $(PWD)/out

SUBDIES = source

all: project

project: $(SUBDIES:=.all)

clean: $(SUBDIES:=.clean)
	rm -rf $(TARGETDIR)

install: $(SUBDIES:=.install)

%.all: 
	cd $* && { $(MAKE) 2>&1; echo $$? > .result; } | tee .testundfined; grep "0" .result
	cd $* && cat .testundfined | grep WARNING | grep undefined && exit 1 || exit 0

%.clean:
	cd $* && $(MAKE) clean

%.install:
	cd $* && $(MAKE) install

.PHONY: all clean install
