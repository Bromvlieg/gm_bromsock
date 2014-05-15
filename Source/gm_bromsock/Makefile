INCLUDES		= 
LIBS			= -lstdc++ 

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
	rm -f obj/*.o gmsv_bromsock_linux.so

install:
	cp gmsv_bromsock_linux.so ~/.steam/steam/SteamApps/common/GarrysMod/garrysmod/lua/bin/gmsv_bromsock_linux.dll
	cp gmsv_bromsock_linux.so ~/.steam/steam/SteamApps/common/GarrysMod/garrysmod/lua/bin/gmcl_bromsock_linux.dll
