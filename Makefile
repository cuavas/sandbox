all: test

test: test.cpp
	clang++ -o $@ -framework CoreFoundation $<

check: test test.plist
	./test

.PHONY: all check
