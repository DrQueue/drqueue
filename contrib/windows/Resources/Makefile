MAKE = make

.PHONY: clean

all: drqueue.res

drqueue.res: drqueue.rc
	windres $< -O COFF $@	

clean:
	rm -f drqueue.res

