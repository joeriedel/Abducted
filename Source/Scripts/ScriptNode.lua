-- ScriptNode.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ScriptNode = Entity:New()

function ScriptNode.Spawn(self)

	self.inputs = {}
	local num = NumberForString(self.keys.inputs, 1)
	for i=1,num do
		self.inputs[i] = false
	end
	
	self.triggers = { 0, 0 }
	self.threshold = NumberForString(self.keys.threshold, 1)
	self.count = NumberForString(self.keys.count, 0)
	self.triggers[1] = NumberForString(self.keys.initialcount, 0)
	self.reps = NumberForString(self.keys["repeat"], 0)
	self.delay = Vec2ForString(self.keys.delay, "0 0")
	self.type = StringForString(self.keys.type, "trigger")
	self.enabled = BoolForString(self.keys.enabled, false)
	self.script = StringForString(self.keys.script)
	self.outputs = StringForString(self.keys.outputs)
	self.autorepeat = NumberForString(self.keys.autorepeatdelay, -1)
	self.name = StringForString(self.keys.targetname, "Unnamed")
	
	if BoolForString(self.keys.triggerOnLevelStart, false) then
		self.think = ScriptNode.Emit
		self.thinkFunc = "emit"
		self.thinkTime = 0
		self:SetNextThink(0)
	end
	
	self.busy = false
	
	if self.type == "any" then
		self.Test = ScriptNode.Emit
	elseif self.type == "all" then
		self.Test = ScriptNode.And
	else
		self.Test = ScriptNode.Threshold
	end
	
	local io = {
		Save = function()
			return self:SaveState()
		end,
		Load = function(s, x)
			return self:LoadState(x)
		end
	}
	
	GameDB.PersistentObjects[self.keys.uuid] = io

end

function ScriptNode.Trigger(self, input, reset)

	if not self.enabled then
		COutLine(kC_Debug, "ScriptNode(%s).Trigger(%d) -- DISABLED IGNORING", self.name, input)
		return
	end
	
	COutLine(kC_Debug, "ScriptNode(%s).Trigger(%d)", self.name, input)

	self.inputs[input] = true
	self:Test()
	
	if reset then
		self.inputs[input] = false
	end

end

function ScriptNode.Reset(self, input)

	self.think = nil

	if input then
		COutLine(kC_Debug, "ScriptNode(%s).Reset(%d)", self.name, input)
		self.inputs[input] = false
	else
		COutLine(kC_Debug, "ScriptNode(%s).Reset(all)", self.name, input)
		for k,v in pairs(self.inputs) do
			self.inputs[k] = false
		end
	end
end

function ScriptNode.Emit(self)

	-- reset inputs
	for k,v in pairs(self.inputs) do
		self.inputs[k] = false
	end
	
	if (self.busy) then
		COutLine(kC_Debug, "ScriptNode(%s).Emit - BUSY", self.name)
		return
	end
	
	self.triggers[1] = self.triggers[1] + 1
	if (self.triggers[1] < self.count) then
		COutLine(kC_Debug, "ScriptNode(%s).Emit(%d/%d)", self.name, self.triggers[1], self.count)
		return
	end
	
	self.triggers[2] = self.triggers[2] + 1
	self.triggers[1] = 0
	
	if ((self.reps > -1) and (self.triggers[2] > self.reps)) then
		COutLine(kC_Debug, "ScriptNode(%s).Emit - Suppressed due to max trigger count", self.name)
		return
	end
	
	self.busy = true
	
	if self.delay[2] > 0 then
		local delay = FloatRand(self.delay[1], self.delay[2])*1000
		self:SetNextThink(delay)
		self.think = ScriptNode.Post
		self.thinkTime = GameDB.realTime + delay
		self.thinkFunc = "post"
		COutLine(kC_Debug, "ScriptNode(%s).Emit(Delayed - %dms)", self.name, delay)
	else
		self:Post()
	end
	
end

function ScriptNode.Post(self)

	COutLine(kC_Debug, "ScriptNode(%s) - Firing", self.name)
	
	self.think = nil
	
	if self.script then
		World.PostEvent(self.script)
	end
	if self.outputs then
		World.DispatchEvent(self.outputs)
	end
	
	if self.autorepeat > -1 then
		COutLine(kC_Debug, "ScriptNode(%s) - AutoRepeatDelay(%f)", self.name, self.autorepeat)
		self.think = ScriptNode.Emit
		self.thinkTime = GameDB.realTime + (self.autorepeat*1000)
		self.thinkFunc = "emit"
		self:SetNextThink(self.autorepeat * 1000)
	end
	
	self.busy = false
	
end

function ScriptNode.Threshold(self)

	local c = 0
	
	for k,v in pairs(self.inputs) do
		if v then
			c = c + 1
			if c >= self.threshold then
				self:Emit()
				break
			end
		end
	end
	
end

function ScriptNode.And(self)

	local r = true
	
	for k,v in pairs(self.inputs) do
		r = r and v
	end
	
	if r then	
		self:Emit()
	end

end

function ScriptNode.OnEvent(self, cmd, args)

	COutLineEvent("ScriptNode", cmd, args)

	if cmd == "trigger" then
		local input = 1
		if args and args ~= "" then
			input = tonumber(args)
		end
		self:Trigger(input)
		return true
	elseif cmd == "trigger_test" then
		local input = 1
		if args and args ~= "" then
			input = tonumber(args)
		end
		self:Trigger(input, true)
		return true
	elseif cmd == "reset" then
		local input = 1
		if args and args ~= "" then
			input = tonumber(args)
		end
		self:Reset(input)
		self.triggers[1] = 0
		self.think = nil
		self.busy = false
		return true
	elseif cmd == "enable" then
		if not self.enabled then
			COutLine(kC_Debug, "ScriptNode(%s).Enabling -- Reseting State", self.name)
			self.enabled = true
			self:Reset()
		end
	elseif cmd == "disable" then
		self.think = nil
		self.busy = false
		if self.enabled then
			self.enabled = false
			COutLine(kC_Debug, "ScriptNode(%s).Disable", self.name)
		end
	end

	return false

end

function ScriptNode.SaveState(self)

	local state = {
		busy = tostring(self.busy),
		trigger1 = tostring(self.triggers[1]),
		trigger2 = tostring(self.triggers[2])
	}
	
	if (self.think) then
		state.think = self.thinkFunc
		state.thinkTime = tostring(self.thinkTime)
	end
	
	return state

end

function ScriptNode.LoadState(self, state)

	self.busy = state.busy == "true"
	self.triggers[1] = tonumber(state.trigger1)
	self.triggers[2] = tonumber(state.trigger2)
	
	if (state.think) then
		self.thinkFunc = state.thinkFunc
		self.thinkTime = tonumber(state.thinkTime)
		
		if (state.thinkFunc == "emit") then
			self.think = ScriptNode.Emit
		elseif (state.thinkFunc == "post") then
			self.think = ScriptNode.Post
		else
			self.think = nil
		end
		
		if (self.think ~= nil) then
			if (self.thinkTime < 1) then
				self:SetNextThink(0)
			else
				local delay = self.thinkTime - GameDB.realTime
				assert(delay >= 0)
				self:SetNextThink(delay)
			end
		end
	end

end

info_script_node = ScriptNode

