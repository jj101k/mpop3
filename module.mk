include defaults.mk
TARGET=$(MODULENAME)$(SHARED_EXT)
OBJECTS=$(MODULENAME).o
OTHER_OBJECTS=../util.o
CC=gcc -Wall -g -I..
LIBS=-lcrypto
DEFINES=-DUSE_OPENSSL=1
BUILD=gcc -bundle
#BUILD=libtool -dynamic

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(BUILD) $(LIBS) -o $(TARGET) $(OBJECTS) $(OTHER_OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET) $(ALSO_CLEAN)

.c.o:
	$(CC) $(DEFINES) -c $<
