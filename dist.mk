dist:
	autoconf
	./configure --with-safe-user=nobody
	make dist
