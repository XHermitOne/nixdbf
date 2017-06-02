# Makefile for NixDBF project
#

# Basic stuff
SHELL = /bin/sh

top_srcdir = .
srcdir = .
prefix = /usr
exec_prefix = ${prefix}
bindir = $(exec_prefix)/bin
infodir = $(prefix)/info
libdir = $(prefix)/lib
mandir = $(prefix)/man/man1
includedir = $(prefix)/include

CC = gcc
# CC = g++
DEFS = -DHAVE_CONFIG_H
#CFLAGS = -g -O2 -Wall
CFLAGS =
#Флаг -g необходим для отладки!!!
#Флаг -w отключает warning!!!
#CPPFLAGS = -g
CPPFLAGS = -g -w
# Флаг линковщика -lm подключает математические функции
LDFLAGS = -v -lm
LIBS = -lm 
BASELIBS = -lm 
X11_INC = 
X11_LIB = 
CAIRO_LIBS = $(shell pkg-config --cflags --libs cairo)

# Directories
TOPSRCDIR = .
TOPOBJDIR = .
SRCDIR    = .
CAIRO_INCLUDEDIR = $(includedir)/cairo

# CPPFLAGS += $(CXX_FLAGS)
# CPPFLAGS += -I$(CAIRO_INCLUDEDIR)
# LDFLAGS += $(CAIRO_LIBS)


# ВНИМАНИЕ! Сначала ставиться -o <выходной файл> затем <объектные файлы> и лишь в конце <флаги линковщика>
nixdbf: main.o run.o version.o log.o strfunc.o tools.o config.o dbf.o
	$(CC) -o nixdbf ./obj/main.o ./obj/run.o ./obj/version.o ./obj/log.o ./obj/strfunc.o ./obj/tools.o ./obj/config.o ./obj/dbf.o $(LDFLAGS)

main.o: ./src/main.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/main.c
	mv main.o ./obj/main.o

run.o: ./src/run.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/run.c
	mv run.o ./obj/run.o

version.o: ./src/version.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/version.c
	mv version.o ./obj/version.o

log.o: ./src/log.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/log.c
	mv log.o ./obj/log.o

strfunc.o: ./src/strfunc.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/strfunc.c
	mv strfunc.o ./obj/strfunc.o

tools.o: ./src/tools.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/tools.c
	mv tools.o ./obj/tools.o

config.o: ./src/config.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/config.c
	mv config.o ./obj/config.o

dbf.o: ./src/dbf.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/dbf.c
	mv dbf.o ./obj/dbf.o

clean:
	rm -f ./src/*.o ./obj/*.o ./*.o ./tst/* nixdbf test

