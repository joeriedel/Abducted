-- Library.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Print
-----------------------------------------------------------------------------]]

function COutLineEvent(class, cmd, args)
	if (args) then
		COutLine(kC_Debug, "%s.OnEvent(%s, %s)", class, cmd, args)
	else
		COutLine(kC_Debug, "%s.OnEvent(%s)", class, cmd)
	end
end

--[[---------------------------------------------------------------------------
	Touch Radius Checks
-----------------------------------------------------------------------------]]

function CheckWorldTouch(pos, x, y, maxDistance)

	local r, dd = WorldTouchDistance(pos, x, y)
	return (r and (dd <= maxDistance)), dd

end

function WorldTouchDistance(pos, x, y)

	local p, r = World.Project(pos)
	local dd = 0
	
	if (r) then
		local dx = p[1] - x
		local dy = p[2] - y
		dd = math.sqrt(dx*dx + dy*dy)
	end
	
	return r, dd
	
end

--[[---------------------------------------------------------------------------
	Rect
-----------------------------------------------------------------------------]]

function ExpandRect(r, w, h)
	return { r[1]-w/2, r[2]-h/2, r[3]+w, r[4]+h }
end

function CenterRectInRect(outer, inner)
	local r = {
		outer[1] + ((outer[3]-inner[3])/2),
		outer[2] + ((outer[4]-inner[4])/2),
		inner[3],
		inner[4]
	}
	return r
end

function CenterChildRectInRect(outer, inner)
	local r = {
		((outer[3]-inner[3])/2),
		((outer[4]-inner[4])/2),
		inner[3],
		inner[4]
	}
	return r
end

function BoundRect(rect, bounds)
	
	if ((rect[1]+rect[3]) > bounds[3]) then
		rect[1] = bounds[3] - rect[3]
	end
	
	if ((rect[2]+rect[4]) > bounds[4]) then
		rect[2] = bounds[4] - rect[4]
	end

	if (rect[1] < bounds[1]) then
		rect[1] = 0
	end
	
	if (rect[2] < bounds[2]) then
		rect[2] = bounds[2]
	end

	return r
	
end

--[[---------------------------------------------------------------------------
	split string
	http://lua-users.org/wiki/SplitJoin
-----------------------------------------------------------------------------]]

function string:split(delim, maxNb)
    -- Eliminate bad cases...
    if self:find(delim) == nil then
        return { self }
    end
    if maxNb == nil or maxNb < 1 then
        maxNb = 0    -- No limit
    end
    local result = {}
    local pat = "(.-)" .. delim .. "()"
    local nb = 0
    local lastPos
    for part, pos in self:gfind(pat) do
        nb = nb + 1
        result[nb] = part
        lastPos = pos
        if nb == maxNb then break end
    end
    -- Handle the last field
    if nb ~= maxNb then
		local z = self:sub(lastPos)
		if (z ~= "") then
			result[nb + 1] = z
		end
    end
    return result
end

--[[---------------------------------------------------------------------------
	Tokenize a string
-----------------------------------------------------------------------------]]

function map(list, f, ...)
	for k,v in pairs(list) do
		list[k] = f(v, ...)
	end
	return list
end

--[[---------------------------------------------------------------------------
	Tokenize a string
-----------------------------------------------------------------------------]]

function FindArrayElement(array, value)
	for k,v in pairs(array) do
		if (v == value) then
			return true, k
		end
	end
	return false
end

function Tokenize(s)

	local x = {}
	
	if ((s == nil) or (s == "")) then
		return x
	end
	
	local z = ""
	
	local i = 1
	while (i <= #s) do
		local c = s:sub(i, i)
		local b = s:byte(i)
		
		if (b <= 32) then
			if (z ~= "") then
				table.insert(x, z)
				z = "" -- new token
			end
		else
			if (c == "\"") then -- quoted
			
				if (z ~= "") then
					table.insert(x, z)
					z = "" -- new token
				end
			
				-- go until end of quote
				local k = i+1
				while (k <= #s) do
					c = s:sub(k, k)
					if (c == "\"") then
						if (z ~= "") then
							table.insert(x, z)
							z = "" -- new token
						end
						break
					else
						z = z..c
					end
					k = k+1
				end
				
				i = k
				
				if (z ~= "") then
					table.insert(x, z)
					z = "" -- new token
				end
			else
				z = z..c -- build token
			end
		end
	
		i = i+1
	end
	
	if (z ~= "") then
		table.insert(x, z)
		z = "" -- new token
	end
	
	return x

end

function GetToken(s, i)

	if (i == nil) then
		i = 1
	end
	
	local z = ""
	
	while (i <= #s) do
		local c = s:sub(i, i)
		local b = s:byte(i)
		
		if (b <= 32) then
			if (z ~= "") then
				return z, i
			end
		else
			if (c == "\"") then -- quoted
			
				if (z ~= "") then
					return z, i
				end
			
				-- go until end of quote
				local k = i+1
				while (k <= #s) do
					c = s:sub(k, k)
					if (c == "\"") then
						return z, (k+1)
					else
						z = z..c
					end
					k = k+1
				end
				
				i = k
				
				if (z ~= "") then
					return z, i
				end
			else
				z = z..c -- build token
			end
		end
	
		i = i+1
	end
	
	if (z == "") then
		z = nil
	end
	
	return z, i

end

function SkipWhitespace(s, i)

	if (i == nil) then
		i = 1
	end
	
	while (i <= #s) do
		local b = s:byte(i)
		if (b > 32) then
			return i
		end
		i = i + 1
	end
	
	return nil

end

--[[---------------------------------------------------------------------------
	Linked List
-----------------------------------------------------------------------------]]

function LL_New(list)

	list = list or {}
	list.ll_head = nil
	list.ll_tail = nil
	list.ll_size = 0
	
	return list

end

function LL_List(item)
	return item.ll_list
end

function LL_Head(list)
	return list.ll_head
end

function LL_Tail(list)
	return list.ll_tail
end

function LL_Next(item)
	return item.ll_next
end

function LL_Prev(item)
	return item.ll_prev
end

function LL_Size(list)
	return list.ll_size
end

function LL_Empty(list)
	return list.ll_head == nil
end

function LL_Append(list, item)
	return LL_Insert(list, item, list.ll_tail)
end

function LL_Do(list, f)

	local item = LL_Head(list)
	
	while (item) do
		f(item)
		item = LL_Next(item)
	end

end

function LL_Insert(list, item, after)

	assert(item.ll_list == nil)
	
	-- special case insert at head
	if after == nil then
		item.ll_prev = nil
		item.ll_next = list.ll_head
		if list.ll_head then
			list.ll_head.ll_prev = item
		end
		list.ll_head = item
		if list.ll_tail == nil then
			list.ll_tail = item
		end
	else
		if after.ll_next then
			after.ll_next.ll_prev = item
		end
		item.ll_next = after.ll_next
		item.ll_prev = after
		after.ll_next = item
		
		if list.ll_tail == after then
			list.ll_tail = item
		end
	end
	
	item.ll_list = list
	list.ll_size = list.ll_size + 1
	
	return item

end

function LL_Remove(list, item) -- removes item, and returns next item in list

	if not item then
		return nil
	end
	
	assert(item.ll_list == list)
	
	local next = item.ll_next
	
	if (list.ll_head == item) then
		list.ll_head = item.ll_next
	end
	if (list.ll_tail == item) then
		list.ll_tail = item.ll_prev
	end
	
	if item.ll_prev then
		item.ll_prev.ll_next = item.ll_next
	end
	if item.ll_next then
		item.ll_next.ll_prev = item.ll_prev
	end
	
	item.ll_next = nil
	item.ll_prev = nil
	item.ll_list = nil
	
	list.ll_size = list.ll_size - 1
	
	return next

end

function LL_Pop(list)
	local item = list.ll_head
	LL_Remove(list, item)
	return item
end

function LL_PopTail(list)
	local item = list.ll_tail
	LL_Remove(list, item)
	return item
end

--[[---------------------------------------------------------------------------
	CurrentDateAndTimeString
-----------------------------------------------------------------------------]]

function CurrentDateAndTimeString()
	local td = System.CurrentDateAndTime()
	
	local suf = "am"
	local hour = td.hour
	
	if (hour > 12) then
		suf = "pm"
		hour = hour - 12
	end
	
	if hour == 0 then
		hour = 12
	end
	
	return string.format(
		"%d/%d/%d - %02d:%02d:%02d %s", 
		td.month, 
		td.day, 
		td.year, 
		hour, 
		td.minute, 
		td.second,
		suf
	)
	
end

--[[---------------------------------------------------------------------------
	Support for parsing entity keys
-----------------------------------------------------------------------------]]

function Vec3ToString(vec)

	return string.format("%f %f %f", vec[1], vec[2], vec[3])

end

function Vec2ForString(str, default)
	if str == nil then
		return default
	end
	x, y = str:match("(%-?%d*.?%d*) (%-?%d*.?%d*)")
	return { tonumber(x), tonumber(y) }
end

function Vec3ForString(str, default)
	if str == nil then
		return default
	end
	x, y, z = str:match("(%-?%d*.?%d*) (%-?%d*.?%d*) (%-?%d*.?%d*)")
	return { tonumber(x), tonumber(y), tonumber(z) }
end

function Color4ToString(color)

	return string.format("%d %d %d %d", color[1]*255, color[2]*255, color[3]*255, color[4]*255)
	
end

function Color4ForString(str, default)
	if str == nil then
		return default
	end
	r, g, b, a = str:match("(%d+) (%d+) (%d+) (%d+)")
	return { tonumber(r)/255, tonumber(g)/255, tonumber(b)/255, tonumber(a)/255 }
end

function StringForString(str, default)
	if str == nil or str == "" then
		return default
	end
	return str
end

function BoolForString(str, default)
	if str == nil then
		return default
	end
	return str == "true"
end

function NumberForString(str, default)
	if (str == nil) then
		return default
	end
	local x = tonumber(str)
	if (x == nil) then
		return default
	end
	return x
end

function TimeStringForNumber(n)

	local millisPerSec = 1000
	local millisPerMin = millisPerSec*60
	local millisPerHour = millisPerMin*60
	
	local hours = math.floor(n/millisPerHour)
	n = n - hours*millisPerHour
	local minutes = math.floor(n/millisPerMin)
	n = n - minutes*millisPerMin
	local seconds = math.floor(n/millisPerSec)
	
	return string.format("%02d:%02d:%02d", hours, minutes, seconds)

end

--[[---------------------------------------------------------------------------
	COut/COutLine
-----------------------------------------------------------------------------]]
	
function COut(level, str, ...)
	str = str:format(...)
	System.COut(level, str)
end

function COutLine(level, str, ...)
	str = str:format(...).."\n"
	System.COut(level, str)
end

--[[---------------------------------------------------------------------------
	String Table / Languages
-----------------------------------------------------------------------------]]

StringTable = {}

-- System.SystemLanguage() is controlled by the App object.

StringTable.Language = System.SystemLanguage()

function StringTable.Get(name, stringTable, useDefault, default, language)
	
	if (useDefault == nil) then
		useDefault = false
	end
	
	if (stringTable == nil) then
	    stringTable = StringTable.Global
	end
	
	if (language == nil) then
		language = StringTable.Language
	end
	
	local x = default
	if name then
		local z = stringTable:Find(name, language)
		if (z == nil) then
			if (not useDefault) then
				x = "??? ("..name..") ???"
			end
		else
			x = z
		end
	elseif (not useDefault) then
		x = "nil"
	end
	
	if (x) then
		x = x:gsub("\\n", "\n")
	end
	
	return x
end