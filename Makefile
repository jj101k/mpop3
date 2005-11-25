include defaults.mk
TARGET=mpop3
OBJECTS=main.o storage_default.o pop_commands.o pop3.o util.o modules.o auth_default.o parse_conffile.o net.o
ALSO_CLEAN=.gdb_history parse_conffile.c rendezvous_test_prog
SUBDIRS=auth_file storage_mbox

.PHONY: all clean
.SUFFIXES: .lex

all: $(TARGET) $(TARGET).conf
	for dir in $(SUBDIRS); do $(MAKE) -I .. -C $$dir all; done

$(TARGET): $(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) -o $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(ALSO_CLEAN) 
	for dir in $(SUBDIRS); do $(MAKE) -I .. -C $$dir clean; done

distclean: clean
	rm -rf config.* autom4te.cache

.lex.c:
	$(LEX) -o$@ $<

.c.o:
	$(CC) $(DEFINES) -c $<

$(TARGET).conf: $(TARGET).conf.sample
	sed 's/\$$SHARED_EXT/'$(SHARED_EXT)'/' $(TARGET).conf.sample > $(TARGET).conf

rendezvous_test_prog:
	gcc -o rendezvous_test_prog test_rendezvous.c

testrendezvous: rendezvous_test_prog
	./rendezvous_test_prog
