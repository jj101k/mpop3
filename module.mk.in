include defaults.mk
TARGET=$(MODULENAME)$(SHARED_EXT)
OBJECTS=$(MODULENAME).o
OTHER_OBJECTS=../util.o
BUILD=gcc -bundle
#BUILD=libtool -dynamic

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(BUILD) $(LIBS) -o $(TARGET) $(OBJECTS) $(OTHER_OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET) $(ALSO_CLEAN)

.c.o:
	$(CC) -I.. $(DEFINES) -c $<
