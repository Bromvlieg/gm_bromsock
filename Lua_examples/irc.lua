require("bromsock");
local socket = BromSock();

local function writeline(line)
	local packet = BromPacket();
	packet:WriteLine(line);
	socket:Send(packet, true);
end

local function socketConnect(sock, connected, ip, port)
	if (not connected) then
		print("Unable to connect to IRC server");
		return;
	end
	print("Connected to IRC Server");
	writeline("NICK test2\r\nUSER test2 test2 * :test2");
	
	socket:ReceiveUntil("\r\n");
end
socket:SetCallbackConnect(socketConnect);

local function sockDisconnect(sock)
	print("IRC socket disconnected");
end
socket:SetCallbackDisconnect(sockDisconnect);

local function socketReceive(sock, packet)
	local message = packet:ReadLine():Trim();
	print("IRCdata: " .. message);
	
	socket:ReceiveUntil("\r\n");
end
socket:SetCallbackReceive(socketReceive);

socket:Connect("<insert ip>", 6667);