struct simpleConfig {
	char (*function)(char const *);
	char const * tag;
};

extern struct simpleConfig *current_tags;
extern void set_pidfile(char *new_pidfile);
