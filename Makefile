TARGET=mpop3
OBJECTS=main.o storage_default.o pop_commands.o pop3.o util.o modules.o auth_default.o parse_conffile.o
ALSO_CLEAN=.gdb_history parse_conffile.c
CC=gcc -Wall -g
LIBS=-lssl -lcrypto
DEFINES=-DUSE_OPENSSL=1
SUBDIRS=auth_file storage_mbox
include defaults.mk

.PHONY: all clean
.SUFFIXES: .lex

all: $(TARGET) $(TARGET).conf
	for dir in $(SUBDIRS); do $(MAKE) -I .. -C $$dir all; done

$(TARGET): $(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) -o $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(ALSO_CLEAN)
	for dir in $(SUBDIRS); do $(MAKE) -I .. -C $$dir clean; done

.lex.c:
	lex -o$@ $<

.c.o:
	$(CC) $(DEFINES) -c $<

$(TARGET).conf: $(TARGET).conf.sample
	sed 's/\$$SHARED_EXT/'$(SHARED_EXT)'/' $(TARGET).conf.sample > $(TARGET).conf
