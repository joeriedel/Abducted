-- Timers.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TimerList = Class:New()

function TimerList.Create(self)
	local fnList = {}
	
	fnList.list = LL_New()
	fnList.time = 0
	fnList.Tick = function (self, time, ...)
		self.time = time
		local x = LL_Head(self.list)
		local dt
		while (x and (LL_List(x) == self.list)) do
			local next = LL_Next(x)
			dt = time - x.time
			if (dt >= x.freq) then
				x.fn(dt, time, ...)
				x.time = time
				if (x.singleShot and (LL_List(x) == self.list)) then
					LL_Remove(self.list, x)
				end
			end
			x = next
		end
	end
	fnList.Add = function(self, fn, freq, singleShot)
		local x = {
			fn = fn, 
			freq = freq, 
			time = self.time,
			singleShot = singleShot,
			SetFreq = function(self, freq)
				self.freq = freq
			end,
			Clean = function(self)
				local list = LL_List(self)
				if (list) then
					LL_Remove(list, self)
				end
			end
		}
		return LL_Append(self.list, x)
	end
	fnList.Remove = function(self, item)
		return LL_Remove(self.list, item)
	end
	
	return fnList
end
