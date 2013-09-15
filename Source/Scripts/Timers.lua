-- Timers.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TimerList = Class:New()

function TimerList.Tick(self, time, ...)
	local dt = time - self.time
	self.time = time
	
	local ll_head = LL_Head
	local ll_list = LL_List
	local ll_next = LL_Next
	local ll_remove = LL_Remove
	
	local item = ll_head(self.timers)
	self.tick = self.tick + 1
	
	while (item) do
		local next = ll_next(item)
		
		if (item.tick ~= self.tick) then
			-- don't tick new timers added in timer refresh
			if (item.tick ~= 0) then
				item.time = item.time + dt
				if (item.time >= item.delay) then
					item.fn(item.time, time, ...)
					item.time = 0
					if ((not item.repeats) and (ll_list(item) == self.timers)) then
						LL_Remove(self.timers, item)
					end
				end
			end
			
			item.tick = self.tick
		end
		
		-- next timer was removed, breaking our ability to move forward
		if (next and (ll_list(next) ~= self.timers)) then
			item = ll_head(self.timers) -- start from the top again
		else
			item = next
		end
	end
end

function TimerList.TimerSetDelay(self, delay)
	self.delay = delay
end

function TimerList.TimerClean(self)
	local list = LL_List(self)
	if (list) then
		LL_Remove(list, self)
	end
end

function TimerList.Add(self, fn, delay, repeats)
	local x = {
		fn = fn, 
		delay = delay, 
		time = 0,
		tick = 0,
		repeats = repeats,
		SetDelay = TimerList.TimerSetDelay,
		Clean = TimerList.TimerClean
	}
	return LL_Append(self.timers, x)
end

function TimerList.Remove(self, item)
	return LL_Remove(self.timers, item)
end

function TimerList.Create(self)
	local fnList = {}
	
	fnList.timers = LL_New()
	fnList.time = 0
	fnList.tick = 0
	
	fnList.Tick = TimerList.Tick
	fnList.Add = TimerList.Add
	fnList.Remove = TimerList.Remove
	
	return fnList
end
