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
    
	-- scroll bar added by dennis
	if (UI.mode == kGameUIMode_PC) then
	    UI:CreateVListWidgetScrollBar(
	        self.widgets.db.EventLog,
	        24,
	        24,
	        8
        )
        self.eventDBWorkspaceSize[3] = self.eventDBWorkspaceSize[3] - 24
    end
    -- end scroll bar add
    
	self.widgets.db.EventLog:SetClipRect(self.eventDBWorkspace)
	self.widgets.db.EventLog:SetEndStops({0, self.eventDBWorkspace[4]*0.1})
	
	self.logTime = -1
	
end

function Arm.EnterLogDB(self, enter, callback, time)
	if (time == nil) then	
		time = 0
	end
	
	if (self.eventLogButtons) then
		for k,v in pairs(self.eventLogButtons) do
			v.class:ResetHighlight(v, true)
		end
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

function Arm.UnmapEventLogButtons(self)

	if (self.eventLogButtons) then
		for k,v in pairs(self.eventLogButtons) do
			if (v.highlight) then
				v.highlight:Unmap()
			end
			if (v.label) then
				v.label:Unmap()
			end
			v:Unmap()
		end
	end
	
	self.eventLogButtons = {}
	
end

function Arm.LoadLog(self)

	if (self.logTime >= EventLog.time) then
		return
	end
	
	self:UnmapEventLogButtons()
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
		local buttonFn = nil
		local buttonLabel = nil
		
		if (v.style == "!ARM_REPLY") then
			text = kEventLogArm.." "..Arm:FindChatString(v.text)
			typeface = self.typefaces.Chat
		elseif (v.style == "!ARM_LOCKED_REPLY") then
			text = kEventLogArm.." "..Arm:FindChatString(v.text)
			typeface = self.typefaces.ChatLocked
		elseif (v.style == "!ARM_ASK") then
			text = kEventLogYou.." "..Arm:FindChatString(v.text)
			typeface = self.typefaces.LogArmAsk
		elseif (v.style == "!ARM_LOCKED") then
			text = kEventLogArmLocked
			typeface = self.typefaces.ChatLocked
		elseif (v.style == "!DISCOVERY") then
			local strings = v.text:split(";")
			local dbId = strings[1]
			
			local dbItem = Arm.Discoveries[dbId]
			local dbItemTitle = nil
			if (dbItem) then
				local unlocked = GameDB:CheckDiscoveryUnlocked(dbId)
				
				if (unlocked or (dbItem.mysteryTitle == nil)) then
					dbItemTitle = StringTable.Get(dbItem.title)
				else
					dbItemTitle = StringTable.Get(dbItem.mysteryTitle)
				end
			end
			
			local strings2 = {}
			for i=2,#strings do
				table.insert(strings2, strings[i])
			end
			strings = strings2
			map(strings, StringTable.Get)
			if (strings[2]) then
				text = string.format(strings[1], select(2, unpack(strings)))
			else
				text = strings[1]
			end
			
			if (dbItemTitle) then
				text = StringTable.Get("ARM_REWARD_DISCOVERY")..": "..dbItemTitle..". "..text
			end
			
			typeface = self.typefaces.LogDiscovery
			buttonFn = function()
				Arm.mode = nil -- hack
				Arm:OpenDatabaseItem(dbId)
			end
			buttonLabel = StringTable.Get("DISCOVERY_OPEN_DB")
		elseif (v.style == "!TOPIC") then
			local topicName = Arm:FindChatString(v.text)
			text = StringTable.Get("EVENT_LOG_NEW_TOPIC"):format(topicName)
			typeface = self.typefaces.LogUnlockTopic
			buttonFn = function()
				self.requestedTopic = v.text
				self.topic = nil
				Arm:TalkPressed()
			end
			buttonLabel = StringTable.Get("EVENT_LOG_TALK_ABOUT"):format(topicName)
		elseif (v.style == "!SKILLPOINTS") then
			local num = tonumber(v.text)
			text = "+"
			
			if (num == 1) then
				text = text..v.text.." "..StringTable.Get("ARM_REWARD_SKILLPOINT")
			else
				text = text..v.text.." "..StringTable.Get("ARM_REWARD_SKILLPOINTS")
			end
			
			typeface = self.typefaces.LogSkillPoints
		else
			text = StringTable.Get(v.text)
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
		
		local yOfs = 0
		
		local strings = text:split("\n")
		
		for kk, vv in pairs(strings) do
			if (vv:len() > 0) then
				text = UI:WordWrap(typeface, vv, marginSpace)
				
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
			else
				yOfs = yOfs + advance
			end
		end
		
		local buttonSpace = 0
		
		if (buttonFn) then
					
			local b = UI:CreateStylePushButton(
				{0, 0, 8, 8},
				buttonFn,
				{
					fontSize="small",
					--background = false,
					highlight = { 
						on = {0.75, 0.75, 0.75, 1}, 
						time = 0.2 
					}, 
					pressed = self.sfx.Button
				}
			)
			
			local labelRect = UI:LineWrapCenterText(
				b.label,
				marginSpace,
				true,
				0,
				buttonLabel
			)
			
			local buttonRect = ExpandRect(
				labelRect, 
				16*UI.identityScale[1],
				96*UI.identityScale[2]
			)
			
			buttonRect[3] = buttonRect[1] + buttonRect[3]
			buttonRect[4] = buttonRect[2] + buttonRect[4]
			buttonRect[1] = 0
			buttonRect[2] = 0
			
			labelRect = CenterRectInRect(labelRect, buttonRect)
			b.label:SetRect(labelRect)
			
			buttonRect[1] = textMargin + (marginSpace-buttonRect[3])/2
			buttonRect[2] = y + yOfs + 8*UI.identityScale[2]
			b:SetRect(buttonRect)
			b.highlight:SetRect({0,0,buttonRect[3],buttonRect[4]})
			
			self.widgets.db.EventLog:AddItem(b)
			b:SetBlendWithParent(true)
			table.insert(self.eventLogButtons, b)
			
			buttonSpace = buttonRect[4] + 8*UI.identityScale[2]
		end
		
		local w = UI:CreateWidget("TextLabel", {rect={0, y, self.eventDBWorkspaceSize[3], yOfs}, typeface=typeface})
		self.widgets.db.EventLog:AddItem(w)
		w:SetBlendWithParent(true)
		w:SetText(modelStrings)
		w:Unmap() -- mark for gc
		
		y = y + yOfs + (Arm.EventDBTextSpace[2] * UI.fontScale[2]) + buttonSpace
	end
	
	self.widgets.db.EventLog:RecalcLayout()
	self.widgets.db.EventLog:ScrollTo({0, y}, 0)
	collectgarbage()
	
end

function Arm.ClearLog(self)
	self.logTime = -1
	self.widgets.db.EventLog:Clear()
end
