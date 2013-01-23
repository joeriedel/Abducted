-- Timers.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TimerList = Class:New()

function TimerList.Create(self)
	local fnList = {}
	
	fnList.list = LL_New()
	fnList.Tick = function (self, time, ...)
		local x = LL_Head(self.list)
		local dt
		while (x) do
			dt = time - x.time
			if (dt >= x.freq) then
				x.fn(dt, time, ...)
				x.time = time
			end
			x = LL_Next(x)
		end
	end
	fnList.Add = function(self, fn, freq)
		local x = {
			fn = fn, 
			freq = freq, 
			time = 0,
			SetFreq = function(self, freq)
				self.freq = freq
			end
		}
		return LL_Append(self.list, x)
	end
	fnList.Remove = function(self, item)
		return LL_Remove(self.list, item)
	end
	
	return fnList
end
