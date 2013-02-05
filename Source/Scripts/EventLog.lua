-- EventLog.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

EventLog = Class:New()
EventLog.MaxLines = 1000

function EventLog.Load(self)

	self.numLines = Persistence.ReadNumber(SaveGame, "numLogEvents", 0)	

end

function EventLog.ShiftLines(self, numLines)

	for i = 1+numLines, self.numLines do
	
		local v = Peristence.ReadString(SaveGame, "logEvent", i)
		Persistence.WriteString(SaveGame, "logEvent", v, i-numLines)
	
	end

end

function EventLog.AddEvent(self, event)

	if (self.numLines >= EventLog.MaxLines) then
		EventLog:ShiftLines(1)
	else
		self.numLines = self.numLines + 1
	end
	
	Persistence.WriteNumber(SaveGame, "numLogEvents", self.numLines)
	Persistence.WriteString(SaveGame, "logEvent", event, self.numLines)
	SaveGame:Save()
	
end