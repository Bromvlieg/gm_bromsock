-- see bottom of file for useage

require( "bromsock" );

-- url: http://4o3.nl
-- method: GET or POST
-- postdatatbl: nil, unles you have POST set as method, then a key/value table containing the data
-- callback: function(table headers, string body)
function HTTPRequest(url, method, postdatatbl, callback)
	if (string.StartWith(url, "http://")) then
		url = string.Right(url, #url - 7)
	end

	local host = ""
	local path = ""
	local postdata = ""
	local bigbooty = ""
	
	local headers = nil
	local chunkedmode = false
	local chunkedmodedata = false
	
	local pathindex = string.IndexOf("/", url)
	if (pathindex > -1) then
		host = string.sub(url, 1, pathindex - 1)
		path = string.sub(url, pathindex)
	else
		host = url
	end
	
	if (#path == 0) then path = "/" end
	
	if (postdatatbl) then
		for k, v in pairs(data) do
			postdata = postdata .. k .. "=" .. v .. "&"
		end
		
		if (#postdata > 0) then
			postdata = string.Left(postdata, #postdata - 1)
		end
	end

	local pClient = BromSock();
	local pPacket = BromPacket();
	
	local function sendline(szString)
		pPacket:WriteLine(szString)
		pClient:Send( pPacket, true );
	end
	
	pClient:SetCallbackConnect( function( pSocket, bConnected, szIP, iPort )
		if (not bConnected) then
			callback(nil, nil)
			return;
		end
		
		sendline(method .. " " .. path .. " HTTP/1.1");
		sendline("Host: " .. host);
		if (method:lower() == "post") then
			sendline("Content-Type: application/x-www-form-urlencoded");
			sendline("Content-Length: " .. #postdata);
		end
		
		sendline("");

		pClient:ReceiveUntil( "\r\n\r\n" );
	end );
	
	pClient:SetCallbackReceive( function( pSocket, pPacket )
		local iSize = pPacket && pPacket:InSize() || -1;
		
		local szMessage = pPacket:ReadStringAll():Trim()
		
		if (not headers) then
			local headers_tmp = string.Explode("\r\n", szMessage)
			headers = {}
			
			local statusrow = headers_tmp[1]
			table.remove(headers_tmp, 1)
			
			headers["status"] = statusrow:sub(10)
			for k, v in ipairs(headers_tmp) do
				local tmp = string.Explode(": ", v)
				headers[tmp[1]:lower()] = tmp[2]
			end
			
			if (headers["content-length"]) then
				pClient:Receive(tonumber(headers["content-length"]));
			elseif (headers["transfer-encoding"] and headers["transfer-encoding"] == "chunked") then
				chunkedmode = true
				pClient:ReceiveUntil( "\r\n" );
			else
				-- This is why we can't have nice fucking things.
				pClient:Receive(99999);
			end
		elseif (chunkedmode) then
			if (chunkedmodedata) then
				bigbooty = bigbooty .. szMessage
				chunkedmodedata = false
				pClient:ReceiveUntil( "\r\n" );
			else
				local len = tonumber(szMessage, 16)
				if (len == 0) then
					callback(headers, bigbooty)
					pClient:Close()
					return
				end
				
				chunkedmodedata = true
				pClient:Receive(len + 2) -- + 2 for \r\n, stilly chunked mode
			end
		else
			callback(headers, szMessage)
			pClient:Close()
		end
	end)
	
	pClient:Connect(host, 80);
end

-- headers is a table containing all the headers lowercase
-- body is the source of the webpage
HTTPRequest("http://4o3.nl", "GET", nil, function(headers, body)
	if (not headers) then
		print("request failed")
		return
	end
	
	print("yay content")
end)