CC := gcc
CXX := g++
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
INCLUDE := ./include
CFLAGS = -g $(INCLUDE:%=-I%)
LDFLAGS = -wl,rpath=$(LIBDIR)

TARGETDIR ?= ./
TARGET = $(TARGETDIR)/my_tcpip

.PHONY: clean

$(TARGET): $(OBJS)
	mkdir -p $(TARGETDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)
	#$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(TARGET)