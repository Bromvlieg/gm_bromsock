INCLUDES		= 
LIBS			= -lstdc++ -lssl -lcrypto 

sourcefiles		:= $(shell find . -name "*.cpp")
objfiles		:= $(patsubst %.cpp,obj/%.o,$(sourcefiles))

.PHONY: all install
all: gmsv_bromsock_linux.so

gmsv_bromsock_linux.so: $(objfiles)
	gcc -m32 -shared -fPIC -std=c++11 $(INCLUDES) -o gmsv_bromsock_linux.so $(objfiles) $(LIBS)

obj/%.o : %.cpp
	gcc -m32 -fPIC -std=c++11 $(INCLUDES) $(LIBS) -c $< -o $@

.PHONY: clean
clean:
	rm -f obj/*.o obj/Objects/*.o gmsv_bromsock_linux.so

release:
	cp gmsv_bromsock_linux.so ../../Builds/gmsv_bromsock_linux.dll

install:
	cp gmsv_bromsock_linux.so ~/Servers/GarrysMod/garrysmod/lua/bin/gmsv_bromsock_linux.dll
	cp gmsv_bromsock_linux.so ~/Servers/GarrysMod/garrysmod/lua/bin/gmcl_bromsock_linux.dll
