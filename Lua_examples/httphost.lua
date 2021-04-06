-- at the bottom: sockServ = HTTPHost(4665)

require( "bromsock" );

if (sockServ) then
	sockServ:Close()
end

local HTTPContentTypes = {
	["html"] = "text/html; charset=UTF-8",
	["txt"] = "text/plain"
}

function HTTPHost(port)
	servsock = BromSock();
	
	if (not servsock:Listen(port)) then
		print("[BS:S] Failed to listen!")
	else
		print("[BS:S] Server listening...")
	end
	
	servsock:SetCallbackAccept(function(serversock, clientsock)
		print("[BS:S] Accepted:", serversock, clientsock)
		
		clientsock:SetCallbackReceive(function(sock, packet)
			print("[BS:S] Received:", sock, packet)
			
			local headerdata = packet:ReadStringAll()
			local rawheaders = string.Explode("\r\n", headerdata)
			
			local headers = {}
			local requestlinedata = nil
			for _, header in pairs(rawheaders) do
				if (not requestlinedata) then
					requestlinedata = string.Explode(" ", header)
				end
			
				local splited = string.Explode(":", header)
				headers[splited[1]] = #splited > 1 and splited[2] or ""
			end
			
			local method = string.lower(requestlinedata[1])
			local path = string.Right(requestlinedata[2], #requestlinedata[2] - 1)
			local httpver = string.lower(requestlinedata[3])
			local filetype = "text/plain"
			
			if (path == "") then
				path = "index.html"
			end
			
			local extentiondata = string.Explode(".", path)
			local extention = string.lower(extentiondata[#extentiondata])
			local contentype = HTTPContentTypes[extention]
			
			print("handleing request: ", httpver, method, path, filetype, extention)
			
			local statuscode = "200 OK"
			
			local filedata = nil
			if (not file.Exists(path, "DATA")) then
				filedata = "<h1>file not found</h1>"
				contentype = HTTPContentTypes["html"]
				statuscode = "404 FILE NOT FOUND"
			else
				filedata = file.Read(path, "DATA")
			end
			
			local pPacket = BromPacket()
			pPacket:WriteLine("HTTP/1.1 " .. statuscode)
			pPacket:WriteLine("Content-Type: " .. (contentype or "text/plain"))
			pPacket:WriteLine("Server:bromsock")
			pPacket:WriteLine("Content-length: " .. #filedata)
			pPacket:WriteLine("")
			pPacket:WriteStringRaw(filedata)
			
			sock:Send(pPacket, true)
		end)
		
		clientsock:SetCallbackDisconnect(function(sock)
			print("[BS:S] Disconnected:", sock)
		end)
		
		clientsock:SetCallbackSend(function(sock, a, b, c)
			print("[BS:S] Send packet:", sock, a, b, c)
		end)
		
		clientsock:SetTimeout(1000)
		clientsock:ReceiveUntil("\r\n\r")
		
		serversock:Accept()
	end)
	
	servsock:Accept()
	return servsock
end

sockServ = HTTPHost(4665)