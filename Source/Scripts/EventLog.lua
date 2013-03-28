-- EventLog.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

EventLog = Class:New()
EventLog.MaxLines = 1000

function EventLog.Load(self)

	self.numLines = Persistence.ReadNumber(SaveGame, "numLogEvents", 0)	
	self.time = Persistence.ReadNumber(SaveGame, "logTime", 0)
	
end

function EventLog.ShiftLines(self, numLines)

	for i = 1+numLines, self.numLines do
	
		local src = "logEvent"..i
		local dst = "logEvent"..(i-numLines)
	
		local v = Peristence.ReadString(SaveGame, src.."/time")
		Persistence.WriteString(SaveGame, dst.."/time", v)
		
		local v = Peristence.ReadString(SaveGame, src.."/style")
		Persistence.WriteString(SaveGame, dst.."/style", v)
		
		local v = Peristence.ReadString(SaveGame, src.."/text")
		if (v) then
			Persistence.WriteString(SaveGame, dst.."/text", v)
		else
			Persistence.DeleteKey(SaveGame, dst.."/text")
		end
	end

end

function EventLog.AddEvent(self, time, style, text)

	if (self.numLines >= EventLog.MaxLines) then
		EventLog:ShiftLines(1)
	else
		self.numLines = self.numLines + 1
	end
	
	self.time = GameDB.time
	
	Persistence.WriteNumber(SaveGame, "numLogEvents", self.numLines)
	Persistence.WriteString(SaveGame, "logEvent", event, self.numLines)
	Persistence.WriteNumber(SaveGame, "logTime", self.time)
	
	local dst = "logEvent"..self.numLines
	Persistence.WriteString(SaveGame, dst.."/time", time)
	Persistence.WriteString(SaveGame, dst.."/style", style)
	
	if (text) then
		Persistence.WriteString(SaveGame, dst.."/text", text)
	else
		Persistence.DeleteKey(SaveGame, dst.."/text")
	end
	
end

function EventLog.LoadList(self)

	local x = {}
	
	for i = 1, self.numLines do
	
		local src = "logEvent"..i
		local time = Persistence.ReadString(SaveGame, src.."/time")
		local style = Persistence.ReadString(SaveGame, src.."/style")
		local text = Persistence.ReadString(SaveGame, src.."/text")
		
		if (time and style) then
			table.insert(x, {time=time, style=style, text=text})
		end
	
	end
	
	return x

end