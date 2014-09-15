require("bromsock");
local socket = BromSock();

local function writeline(line)
	local packet = BromPacket();
	packet:WriteLine(line);
	socket:Send(packet, true);
	
	print("IRC WROTE: " .. line);
end

local function socketConnect(sock, connected, ip, port)
	if (not connected) then
		print("Unable to connect to IRC server");
		return;
	end
	
	print("Connected to IRC Server");
	writeline("NICK TEST\r\nUSER TEST TEST * :TEST");
	
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
	
	local parts = string.Explode(":", message)
	if (#parts == 2 and parts[1] == "PING ") then
		writeline("PONG :" .. parts[2]);
	end
	
	socket:ReceiveUntil("\r\n");
end
socket:SetCallbackReceive(socketReceive);

socket:Connect("<insert ip>", 6667);

concommand.Add("irc_send", function(ply, cmd, args)
	writeline(cmd);
end)