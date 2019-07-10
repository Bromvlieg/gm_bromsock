gm_bromsock
===========

A networking module for Garry's Mod created by [Bromvlieg](https://github.com/Bromvlieg)

Facepunch forum thread: http://facepunch.com/showthread.php?t=1393640

##Example code
*	[HTTP host](Lua_examples/httphost.lua)
*	[HTTP request](Lua_examples/httprequest.lua)
*	[IRC](Lua_examples/irc.lua)
*	[UDP](Lua_examples/udp.lua)
*	[Host and Connect](Lua_examples/host_and_connect.lua)

##License
This is distributed under the MIT license.
[(Read More)](LICENSE)


##Notes
	Windows .dll is compiled with Visual studio 2013 using the 2013 redist (https://www.microsoft.com/en-us/download/details.aspx?id=40784)

	Regarding the documentation below, arguments in [tags] are optional and can be ignored.
    If you use callbacks(which you should), then the functions which use callbacks will return nothing

	To receive and send data to non bromsock sockets, you'll have to add the true argument to the sock:Send(packet, [dontsendlength])
	And you'll have to provide a number of bytes to receive in the sock:Receive([count]) (or use sock:ReceiveUntil(string))

	If you want to receive more than 10MB in one sock:Receive([count]) call, you'll have to use sock:SetMaxReceiveSize(maxbytes) first
	to prevent the sanity checker throwing errors

    Every socket has 2 worker threads. A worker will block(unless you set blocking to false) until the requested action is done.
    This means, that you can call 2 functions at the same time. Example: Send and Receive.
    But if you call Receive twice, and send after that, it'll be stuck on receiving until it got some data, after that the Send gets executed
    if you want more workers (Which I don't think you need, but screw it) sock:AddWorker()

    WARNING: if you disable blocking, then callbacks WILL generate disconnect events. Callbacks expect blocking, if not, it WILL fuck up

##Packet
```lua
local packet = BromPacket()
-- Add socket as argument for streaming(bad practice, but sometimes useful)

WriteString(str) -- adds a short containing the string size infront
WriteStringNT(str) -- adds a null byte at the end
WriteStringRaw(str) -- only writes the string, nothing more, nothing less
WriteLine(str) --adds \r\n at the end
WriteByte(num) -- byte is unsigned, use SByte for signed version
WriteSByte(num)
WriteShort(num)
WriteUShort(num)
WriteFloat(num)
WriteInt(num)
WriteUInt(num)
WriteDouble(num)
WriteLong(num)
WriteULong(num)
WritePacket(packet) -- writes the outbuffer of the packet to the current packet

ReadString([length]) -- returns string (reads a short as length index if you don't supply your own length)
ReadStringNT() -- returns string (reads until a 0 byte)
ReadStringAll() -- returns string (reads the entire remains of the packet as a string and returns this)
ReadLine() -- returns string (reads until \r\n)
ReadUntil(str) -- returns string (reads until it encounter str, the return value includes str at the end.)
ReadByte() -- returns number, byte is unsigned, use SByte for signed version
ReadSByte()
ReadShort() -- bla bla bla bla look my hands are typing words
ReadUShort()
ReadFloat()
ReadInt()
ReadUInt()
ReadDouble()
ReadLong()
ReadULong()

InPos() -- returns the position of the In buffer
InSize() -- returns the size of the In buffer

-- About the buffer, the entire buffer is bigger than your packet, this is because it pre-allocates space for you
-- to speed up the writing process, this can be a bit confusing but if you think about it, it makes sense.
OutPos() -- the current location of the out buffer, also being the size of your writen packet
OutSize() -- the size of the entire out buffer
-- So if you want to know the size of YOUR data in the out buffer, use OutPos() and not OutSize()

Clear() -- empties the inbuffer and outbuffer
Copy() -- returns a copy of the current packet
```

##Socket
```lua
local socket = BromSock([protocol]) -- BROMSOCK_TCP or BROMSOCK_UDP. Defaults to TCP

SetCallbackReceive(func) -- args in func: sock, packet. (failed: packet == nil)
SetCallbackReceiveFrom(func) -- hostsock, packet, ip, port < THIS IS FOR UDP (failed: packet == nil)
SetCallbackSend(func) -- sock, datalengthsent.
SetCallbackSendTo(func) -- sock, datalengthsent, ip, port < THIS IS FOR UDP
SetCallbackConnect(func) -- sock, suceeded, ip, port
SetCallbackDisconnect(func) -- sock
SetCallbackAccept(func) -- sock, clientsock (failed: clientsock == nil)

SetTimeout(miliseconds) -- returns true on success
-- Warning: you need to do this BEFORE you call receive or send, and AFTER you call Connect, Bind, or Listen.

Connect(ip, port) -- returns true/false, get error message using GetLastError if fail.
Bind(ip, port) or (port) -- returns true/false, get error message using GetLastError if fail.
Listen(ip, port) or (port) -- returns true/false, get error message using GetLastError if fail.
Accept() -- returns a socket or false, get error message using GetLastError if fail.
Close()
Disconnect() -- Alias of Close
GetIP() -- Returns the IP as a string
GetPort() -- Returns the port as a number
StartSSLClient() -- true/false, starts a TLS v1.2 session, and will use this for all following read and writes
SetMaxReceiveSize(maxbytes) -- sets the sanity checkers limit, defaults to 10MB max
IsValid() -- Returns true if the socket is listening or connected
GetLastError() -- Returns a string describing the last socket error that has happened. Use immediately after one of the supported socket functions fail.


SetBlocking(bool)
-- default this is true. You should not touch this, unless you know what you're doing

Create()
-- this creates the socket object prematurely. This is required if you want to use SetOption BEFORE you use listen/bind/connect.

ReceiveFrom([bytecount], [ip], [port])
-- if no bytecount is given (or nil), it'll receive one diagram. If ip and port also supplied it'll try to receive from that addr.

Receive([bytecount])
-- returns packet or false. if no bytecount is given, it will internally call receive(4), read an int from that, and then receive the result of that int.

ReceiveUntil(str)
-- Reads until that string is encountered. ex: "\r\n". the search string will be included in the result you get back at the receive callback

SendTo(packet, ip, port)
-- Send packet to ip:port. Returns nothing, this uses the callback/threading system. Setting a callback is not required for this one.

Send(packet, [dontsendlength])
-- if dontsendlength is true, it WILL NOT add an int in front of the packet to indicate how large the incoming data is. use true if you want to communicate with anything that does not use this way of packeting

-- Implemented and functional meta methods:
__eq
__tostring
__gc
```

##For advanced networking users
```lua
sock:SetOption(level, option, value)
-- returns the number the internal setoption returns (0 often means success, and -1 failure)

sock:AddWorker()
-- Adds another worker thread
```

```c
//Levels
SOL_SOCKET = 0xFFFF
IPPROTO_TCP = 0x6

//Options
TCP_NODELAY = 0x0001 // enable/disable Nagle's algorithm (this requires IPPROTO_TCP as level)
SO_DEBUG = 0x0001 // turn on debugging info recording (this, and the rest of the SO_ options require SOL_SOCKET as level)
SO_ACCEPTCONN = 0x0002 // socket has had listen()
SO_REUSEADDR = 0x0004 // allow local address reuse
SO_KEEPALIVE = 0x0008 // keep connections alive
SO_DONTROUTE = 0x0010 // just use interface addresses
SO_BROADCAST = 0x0020 // permit sending of broadcast msgs
SO_USELOOPBACK = 0x0040 // bypass hardware when possible
SO_LINGER = 0x0080 // linger on close if data present
SO_OOBINLINE = 0x0100 // leave received OOB data in line
SO_SNDBUF = 0x1001 // send buffer size
SO_RCVBUF = 0x1002 // receive buffer size
SO_SNDLOWAT = 0x1003 // send low-water mark
SO_RCVLOWAT = 0x1004 // receive low-water mark
SO_SNDTIMEO = 0x1005 // send timeout
SO_RCVTIMEO = 0x1006 // receive timeout
SO_ERROR = 0x1007 // get error status and clear
SO_TYPE = 0x1008 // get socket type
```
