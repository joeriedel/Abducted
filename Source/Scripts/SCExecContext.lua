-- SCExecContext.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- SCExec CVar execution context

SCExecContext = Class:New()

function SCExecContext.Spawn(self)

	SCExecContext.entity = self
	
end

function SCExecContext.Think(self)

	self.think = nil
	
	if (SCExecContext.f) then
		local f = SCExecContext.f
		SCExecContext.f = nil
		f()
	end

end

scexec_code = SCExecContext
