-- Run this file in both SV and CL to see working output.

require("bromsock")

-- SERVER EXAMPLE

if SERVER then
	if server then server:Close() end
	server = BromSock()
	
	if (not server:Listen(6789)) then
		print("[BS:S] Failed to listen!")
	else
		print("[BS:S] Server listening...")
	end

	server:SetCallbackAccept(function(serversock, clientsock)
		print("[BS:S] Accepted:", serversock, clientsock)
		
		clientsock:SetCallbackReceive(function(sock, packet)
			print("[BS:S] Received:", sock, packet)
			print("[BS:S] R_Num:", packet:ReadInt())
			
			packet:WriteString("Woop woop woop a string")
			sock:Send(packet)
			
			-- normaly you'd want to call Receive again to read the next packet. However, we know that the client ain't going to send more, so fuck it.
			-- theres only one way to see if a client disconnected, and that's when a error occurs while sending/receiving.
			-- this is why most applications have a disconnect packet in their code, so that the client informs the server that he exited cleanly. There's no other way.
			-- We set a timeout, so let's be stupid and hope there's another packet incoBSing. It'll timeout and disconnect.
			sock:Receive()
		end)
		
		clientsock:SetCallbackDisconnect(function(sock)
			print("[BS:S] Disconnected:", sock)
		end)
		
		clientsock:SetTimeout(1000) -- timeout send/recv coBSands in 1 second. This will generate a Disconnect event if you're using callbacks
		
		clientsock:Receive()
		
		-- Who's next in line?
		serversock:Accept()
	end)
	
	server:Accept()
end

-- CLIENT EXAMPLE

if CLIENT then
	if client then client:Close() end
	client = BromSock()
	
	client:SetCallbackConnect(function(sock, ret, ip, port)
		if (not ret) then
			print("[BS:C] Failed to connect to: ", ret, ip, port)
			return
		end
	
		print("[BS:C] Connected to server:", sock, ret, ip, port)
		local packet_client = BromPacket(client)
		packet_client:WriteInt(13000)
		client:Send(packet_client)
	end)

	client:SetCallbackReceive(function(sock, packet)
		print("[BS:C] Received:", sock, packet, packet and packet:InSize() or -1)
		print("[BS:C] R_Str:", packet:ReadString())
		
		-- normaly you'd call Receive here again, instead of disconnect
		sock:Disconnect()
	end)

	client:SetCallbackSend(function(sock, datasent)
		print("[BS:C] Sent:", "", sock, datasent)
		
		-- we expect a response form the server after he received this, so instead of calling Receive at the connect callback, we do it here.
		client:Receive()
	end)

	client:Connect("127.0.0.1", 6789)
end