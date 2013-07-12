-- ArmEventDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.EventDBInset = { 16, 16 }
Arm.EventDBTextSpace = {12, 4}

function Arm.SpawnEventDB(self)
	
	self.eventDBWorkspace = {
		self.dbWorkspace[1] + (Arm.EventDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[2] + (Arm.EventDBInset[2] * UI.identityScale[2]),
		self.dbWorkspace[3] - (Arm.EventDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[4] - (Arm.EventDBInset[2] * UI.identityScale[2])
	}
	
	self.eventDBWorkspaceSize = {
		0,
		0,
		self.eventDBWorkspace[3],
		self.eventDBWorkspace[4]
	}
	
	-- big scrolling list of text
	self.widgets.db.EventLog = UI:CreateWidget("VListWidget", {rect=self.eventDBWorkspace})
	self.widgets.db.Root:AddChild(self.widgets.db.EventLog)
	self.widgets.db.EventLog:SetBlendWithParent(true)
	self.widgets.db.EventLog:SetClipRect(self.eventDBWorkspace)
	self.widgets.db.EventLog:SetEndStops({0, self.eventDBWorkspace[4]*0.1})
	
	self.logTime = -1
	
end

function Arm.EnterLogDB(self, enter, callback, time)
	if (time == nil) then	
		time = 0
	end
	
	if (enter) then
		Arm:LoadLog()
		self.widgets.db.EventLog:BlendTo({1,1,1,1}, time)
	else
		self.widgets.db.EventLog:BlendTo({0,0,0,0}, time)
	end
	
	if (callback and (time > 0)) then
		self.dbTimer = World.globalTimers:Add(callback, time)
	elseif (callback) then
		callback()
	end
end

function Arm.LoadLog(self)

	if (self.logTime >= EventLog.time) then
		return
	end
	
	self.widgets.db.EventLog:Clear()
	
	local events = EventLog:LoadList()
	
	local y = 0
	local advance = UI:FontAdvanceSize(self.typefaces.Chat)
	local textMargin = UI:StringDimensions(self.typefaces.Chat, "000-00:00:00")
	textMargin = textMargin + (Arm.EventDBTextSpace[1] + UI.fontScale[1])
	local marginSpace = self.eventDBWorkspaceSize[3] - textMargin
	
	local kEventLogArm = StringTable.Get("EVENT_LOG_ARM")
	local kEventLogYou = StringTable.Get("EVENT_LOG_YOU")
	local kEventLogArmLocked = StringTable.Get("EVENT_LOG_ARM_LOCKED")
	
	for k,v in pairs(events) do
	
		-- parse the kind of event
		local time = v.time
		
		local text = nil
		local typeface = nil
		
		if (v.style == "!ARM_REPLY") then
			text = kEventLogArm.." "..v.text
			typeface = self.typefaces.Chat
		elseif (v.style == "!ARM_LOCKED_REPLY") then
			text = kEventLogArm.." "..v.text
			typeface = self.typefaces.ChatLocked
		elseif (v.style == "!ARM_ASK") then
			text = kEventLogYou.." "..v.text
			typeface = self.typefaces.LogArmAsk
		elseif (v.style == "!ARM_LOCKED") then
			text = kEventLogArmLocked
			typeface = self.typefaces.ChatLocked
		elseif (v.style == "!DISCOVERY") then
			text = StringTable.Get(strings[2])
			typeface = self.typefaces.LogDiscovery
		elseif (v.style == "!DISCOVERY_TEXT") then
			text = strings[2]
			typeface = self.typefaces.LogDiscovery
		else
			text = StringTable.Get(strings[2])
			typeface = self.typefaces.LogEvent
		end
		
		-- first string is time
		local modelStrings = {}
		
		modelStrings[1] = {
			x = 0,
			y = 0,
			text = time,
			scaleX = UI.fontScale[1],
			scaleY = UI.fontScale[2]
		}
		
		text = UI:WordWrap(typeface, text, marginSpace)
		
		local yOfs = 0
		
		for k,v in pairs(text) do
		
			local s = {
				x = textMargin,
				y = yOfs,
				text = v,
				scaleX = UI.fontScale[1],
				scaleY = UI.fontScale[2]
			}
			
			yOfs = yOfs + advance
			
			table.insert(modelStrings, s)
		
		end
		
		local w = UI:CreateWidget("TextLabel", {rect={0, y, self.eventDBWorkspaceSize[3], yOfs}, typeface=typeface})
		self.widgets.db.EventLog:AddItem(w)
		w:SetBlendWithParent(true)
		w:SetText(modelStrings)
		w:Unmap() -- mark for gc
		
		y = y + yOfs + (Arm.EventDBTextSpace[2] * UI.fontScale[2])
	end
	
	self.widgets.db.EventLog:RecalcLayout()
	self.widgets.db.EventLog:ScrollTo({0, y}, 0)
	collectgarbage()
	
end

function Arm.ClearLog(self)
	self.logTime = -1
	self.widgets.db.EventLog:Clear()
end
