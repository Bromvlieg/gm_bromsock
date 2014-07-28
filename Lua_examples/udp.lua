require("bromsock")

concommand.Add("hostUDP", function()
	if serversock then serversock:Close() end -- to make sure we can host. Would be rather silly if we'd try to host while it's still open!
	serversock = BromSock(BROMSOCK_UDP)

	serversock:SetCallbackReceiveFrom(function(sockobj, packet, ip, port)
		print("[S] Received:", packet, ip, port)
		print("[S] R_Str:", packet:ReadStringAll())
		
		packet:WriteString("From server!")
		serversock:SendTo(packet, ip, port)
		
		serversock:ReceiveFrom()
	end)

	serversock:Bind(1337)
	serversock:ReceiveFrom()
end)


concommand.Add("sendUDP", function()
	local clientsock = BromSock(BROMSOCK_UDP)
	local packet = BromPacket()
	
	packet:WriteStringRaw("From client!")
	clientsock:SendTo(packet, "127.0.0.1", 1337)
	
	clientsock:Close()
	packet:Clear()
	clientsock = nil
	packet = nil
end)