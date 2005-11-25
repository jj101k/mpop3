TARGET=mpop3
OBJECTS=main.o auth_file.o storage_mbox.o pop_commands.o pop3.o
ALSO_CLEAN=.gdb_history
CC=gcc -Wall -g
LIBS=-lssl -lcrypto
DEFINES=-DUSE_OPENSSL=1

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) -o $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(ALSO_CLEAN)

.c.o:
	$(CC) $(DEFINES) -c $<
