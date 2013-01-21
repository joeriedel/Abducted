-- Library.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Common Utilities
-----------------------------------------------------------------------------]]

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

function LL_Empty(list)
	return list.ll_head == nil
end

function LL_Append(list, item)
	return LL_Insert(list, item, list.ll_tail)
end

function LL_Insert(list, item, after)

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
	
	list.ll_size = list.ll_size + 1
	
	return item

end

function LL_Remove(list, item) -- removes item, and returns next item in list

	if not item then
		return nil
	end
	
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
		"%d/%d/%d - %d:%02d:%02d %s", 
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

function StringTable.Get(name, useDefault, default)
	
	if (useDefault == nil) then
		useDefault = false
	end
	
	local x = default
	if name then
		local z = System.GetLangString(name, StringTable.Language)
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
	
	return x
end