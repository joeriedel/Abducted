-- ArmChat.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.ChatInset = { 0, 0 }
Arm.ChatChoiceInset = { 16, 0 }
Arm.ChatLineSpace = 4
Arm.ChatChoiceSpace = 4
Arm.ChatChoiceExtraSpaceAfterPrompt = 6
Arm.ChatChoiceHorzSpace = 16
Arm.ChatChoiceButtonPadd = { 16, 10 }
Arm.ChatClipAdjust = { 0, 0, 0, 0 }
Arm.MaxChangeConversationTimes = 8

function Arm.SpawnChat(self)

	self.widgets.chat = {}
	self.widgets.chat.Root = UI:CreateWidget("Widget", {rect=self.workspaceLeftSize})
	self.widgets.chat.Root:BlendTo({1,1,1,0},0)
	self.widgets.WorkspaceLeft:AddChild(self.widgets.chat.Root)
	
	self.chatRect = {
		12*UI.identityScale[1], 
		10*UI.identityScale[2],
		self.workspaceLeft[3] - (24*UI.identityScale[1]), 
		self.workspaceLeft[4] - (20*UI.identityScale[2])
	}
	
	self.widgets.chat.ChatList = UI:CreateWidget("VListWidget", {rect=self.chatRect,OnInputEvent=Arm.SkipText})
	
	if (UI.mode == kGameUIMode_PC) then
		UI:CreateVListWidgetScrollBar(
			self.widgets.chat.ChatList,
			24,
			24,
			8
		)
		
		self.chatRect[3] = self.chatRect[3] - 24
	end
	
	self.widgets.chat.ChatList:SetClipRect({
		self.chatRect[1] + (Arm.ChatClipAdjust[1]*UI.identityScale[1]), 
		self.chatRect[2] + (Arm.ChatClipAdjust[2]*UI.identityScale[2]), 
		self.chatRect[3] - (Arm.ChatClipAdjust[3]*UI.identityScale[1]), 
		self.chatRect[4] - (Arm.ChatClipAdjust[4]*UI.identityScale[2])
	})
	
	self.widgets.chat.Root:AddChild(self.widgets.chat.ChatList)
	self.widgets.chat.ChatList:SetEndStops({0, self.chatRect[4]*0.1})
	self.widgets.chat.ChatList:SetBlendWithParent(true)
	
end

function Arm.SkipText(widget, e)
	if (Input.IsTouchBegin(e)) then
		if (Arm.skipFun) then
			Arm:skipFun()
			Arm.skipFun = nil
		end
	end
	return false
end

function Arm.ResetChat(self)
	self:EnableChangeTopic(false)
	GameDB:CheckChatLockout()
	self.widgets.chat.Root:BlendTo({1,1,1,0}, 0)
end

function Arm.StartChat(self, delay)
	if (not GameDB.chatLockout) then
		Arm:SwapToChange()
		self.widgets.chat.Root:BlendTo({1,1,1,1}, 0.2)
		
		if (self.currentRewardWidgets) then
			for k,v in pairs(self.currentRewardWidgets) do
				if (v.class) then
					v.class:ResetHighlight(v, true)
				end
			end
		end
		
		if (self.topic == nil) then
			Arm:StartConversation()
		else
			if (delay) then
				local f = function()
					Arm:EnableChangeTopic(true)
				end
				World.globalTimers:Add(f, delay)
			else
				Arm:EnableChangeTopic(true)
			end
		end
	else
		Arm:DisplayChatLockout()
	end
end

function Arm.EndChat(self, callback)
	Arm:SwapToTalk()
	
	self.widgets.chat.Root:BlendTo({1,1,1,0}, 0.2)
	
	if (self.chatTimer2) then
		self.chatTimer2:Clean()
		self.chatTimer2 = nil
	end
	
	local f = nil
	
	if (GameDB.chatLockout) then
		Arm:ShowSymbol(false, 0.2)
		f = function()
			Arm:ClearChat()
			callback()
		end
	else
		f = callback
	end
		
	World.globalTimers:Add(f, 0.2)
end

function Arm.ClearChat(self)

	self.topic = nil
	
	if (self.chatTimer) then
		self.chatTimer:Clean()
		self.chatTimer = nil
	end
	
	if (self.chatTimer2) then
		self.chatTimer2:Clean()
		self.chatTimer2 = nil
	end
	
	if (self.promptState and self.promptState.labels) then
		for k,v in pairs(self.promptState.labels) do
			v:Unmap() -- mark for gc
		end
	end
	
	if (self.choiceWidgets) then
		for k,v in pairs(self.choiceWidgets) do
			v:Unmap() -- mark for gc
		end
	end
	
	if (self.rewardWidgets) then
		for k,v in pairs(self.rewardWidgets) do
			v:Unmap() -- mark for gc
		end
	end
	
	self.promptState = nil
	self.choiceWidgets = nil
	self.rewardWidgets = nil
	self.currentRewardWidgets = nil
	self.choices = nil
	self.widgets.chat.ChatList:Clear()
	collectgarbage()
end

function Arm.HideChat(self)

	self.widgets.chat.Root:BlendTo({1,1,1,0}, 0)
	
	if (self.chatTimer2) then
		self.chatTimer2:Clean()
		self.chatTimer2 = nil
	end

end

function Arm.LoadChats(self, chats)

    if (chats) then
		Arm:LoadChatList(chats)
    end
    
    Arm:LoadCommonChat()
end

function Arm.StartConversation(self)

	self:ClearChat()
	Arm:EnableChangeTopic(false)
	
	self.chatPos = {Arm.ChatInset[1]*UI.identityScale[1], Arm.ChatInset[2]*UI.identityScale[2]}
	self.inReqTopic = false
	self.inContextTopic = false
	
	if (self.horrorTopic) then
		self.changeConversationCount = 0
		self.topic = Arm.Chats.Loaded[self.horrorTopic]
	elseif (self.contextTopic) then
		self.changeConversationCount = 0
		self.topic = Arm.Chats.Loaded[self.contextTopic]
		self.inContextTopic = true
	elseif (self.requestedTopic) then
		self.changeConversationCount = 0
		self.topic = Arm.Chats.Loaded[self.requestedTopic]
		self.requestedTopic = nil
	elseif (self.requiredTopic) then
		self.changeConversationCount = 0
		self.topic = Arm.Chats.Loaded[self.requiredTopic]
		self.inReqTopic = true
	else
		self.changeConversationCount = self.changeConversationCount + 1
		self.topic = self:SelectChatRoot()
	end
	
	self.topicTree = {}
	self.chatStart = true
	
	if (self.topic) then
		self:ChatPrompt()
	end

end

function Arm.ProcessActions(self, actions)
	-- rewards?
	for k,v in pairs(actions) do
		local tokens = Tokenize(v)
		if (#tokens > 0) then
			self:ProcessActionTokens(tokens)
		end
	end
	
	if (self.rewardSkillPoints) then
		PlayerSkills:AwardSkillPoints(self.rewardSkillPoints)
	end
end

function Arm.GetSkillAwardAmount(self, token, context)
	
	local num
	
	if (token:sub(1,1) == "x") then
		num = tonumber(token:sub(2))
		if (context == "arm") then
			num = num * PlayerSkills.kArmChatReward
		else
			num = num * PlayerSkills.kTerminalReward
		end
	else
		num = tonumber(token)
	end
	
	return num
end

function Arm.ProcessActionTokens(self, tokens)

	if (tokens[1] == "trigger") then
		self:ProcessTriggerTokens(tokens)
	elseif (tokens[1] == "unlock_topic") then
		if (Arm:UnlockTopic(tokens[2])) then
			self.rewardTopic = self.rewardTopic or {}
			table.insert(self.rewardTopic, tokens[2])
		end
	elseif (tokens[1] == "lock_topic") then
		Arm:LockTopic(tokens[2])
	elseif (tokens[1] == "message") then
		self.rewardMessage = self.rewardMessage or {}
		table.insert(self.rewardMessage, tokens[2])
		EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", tokens[2])
	elseif (tokens[1] == "award") then
		if (not Arm:CheckTopicReward(self.topic, "skillpoints")) then
			self.rewardSkillPoints = self.rewardSkillPoints or 0
			self.rewardSkillPoints = self.rewardSkillPoints + Arm:GetSkillAwardAmount(tokens[2], "arm")
			Arm:SaveTopicReward(self.topic, "skillpoints")
		end
	elseif (tokens[1] == "unlock_skill") then
		if (not Arm:CheckTopicReward(self.topic, "unlock_skill")) then
			self.rewardSkill = tokens[2]
			Arm:SaveTopicReward(self.topic, "unlock_skill")
		end
	elseif (tokens[1] == "discover") then
		if (GameDB:Discover(tokens[2], "arm", true)) then
			self.rewardDiscover = self.rewardDiscover or {}
			table.insert(self.rewardDiscover, tokens[2])
			self.rewardSkillPoints = self.rewardSkillPoints or 0
			self.rewardSkillPoints = self.rewardSkillPoints + PlayerSkills.kDiscoveryReward
		end
	elseif (tokens[1] == "clear_topic") then
		self.requiredTopic = nil
		self:ClearSignal()
	elseif (tokens[1] == "push_topic") then
		-- insert a topic into the convo tree so it doesn't show up again.
		if (self.topicTree) then
			self.topicTree[tokens[2]] = true
		end
	elseif (tokens[1] == "downgrade_yes") then
		TerminalScreen.Downgrade()
	elseif (tokens[1] == "clear_context") then
		if (self.clearContext) then
			self.clearContext()
			self.clearContext = nil
			self.contextTopic = nil
			self:ClearSignal()
		end
	end
end

function Arm.ProcessTriggerTokens(self, tokens)
	local s = nil
	for i=2,#tokens do
		if (s) then
			s = s.." \""..tokens[i].."\""
		else
			s = tokens[i]
		end
	end
	
	if (s) then
		World.PostEvent(s)
	end
end

function Arm.CreateRewardActionButton(self, text, inset, maxWidth, f)
	local sizeW, sizeH = UI:StringDimensions(self.typefaces.ChatReward, text)
	
	local r = {0, 0, sizeW, sizeH}
	
	local w = UI:CreateStylePushButton(
		r,
		function (w)
			w.class:ResetHighlight(w)
			if (f) then
				f()
			end
		end,
		{
			--background = false,
			highlight = { 
				on = {0.75, 0.75, 0.75, 1}, 
				time = 0.2 
			}, 
			typeface = self.typefaces.ChatReward,
			pressed = self.sfx.Button
		}
	)
	
	if (maxWidth and (sizeW > maxWidth)) then
		r = UI:LineWrapCenterText(w.label, maxWidth, true, Arm.ChatChoiceSpace, text)
	else
		UI:SetLabelText(w.label, text)
		r = UI:SizeLabelToContents(w.label)
	end
	
	-- expand the button around the label
	local buttonRect = {
		self.chatPos[1]+inset-Arm.ChatChoiceButtonPadd[1]*UI.identityScale[1], 
		self.chatPos[2],
		r[3]+Arm.ChatChoiceButtonPadd[1]*UI.identityScale[1]*2,
		r[4]+Arm.ChatChoiceButtonPadd[2]*UI.identityScale[2]
	}
	
	w:SetRect(buttonRect)
	w:SetBlendWithParent(true)
	w.highlight:SetRect({0, 0, buttonRect[3], buttonRect[4]})
	r = CenterChildRectInRect(r, buttonRect)
	w.label:SetRect(r)
	
	w.x = buttonRect[1]
	w.y = self.chatPos[2]
	
	self.chatPos[2] = buttonRect[2] + buttonRect[4] + (Arm.ChatLineSpace*UI.identityScale[2])
	
	return w,buttonRect
end

function Arm.CreateRewardActionText(self, text, inset, maxWidth)
	local sizeW, sizeH = UI:StringDimensions(self.typefaces.ChatReward, text)
	
	local label = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.ChatReward})
	local r = nil

	if (maxWidth and (sizeW > maxWidth)) then
		r = UI:LineWrapCenterText(label, maxWidth, true, Arm.ChatChoiceSpace, text)
	else
		UI:SetLabelText(label, text)
		r = UI:SizeLabelToContents(label)
	end
	
	r[1] = self.chatPos[1]+inset
	r[2] = self.chatPos[2]
	r[4] = r[4]+Arm.ChatChoiceButtonPadd[2]*UI.identityScale[2]
	
	label:SetRect(r)
	
	label.x = r[1]
	label.y = self.chatPos[2]
		
	self.chatPos[2] = r[2] + r[4] + (Arm.ChatLineSpace*UI.identityScale[2])
	return label
end

function Arm.CreateRewardText(self)

	if (not (self.rewardTopic or self.rewardMessage or self.rewardSkillPoints or self.rewardSkill or self.rewardDiscover)) then
		return nil
	end
	
	local inset = Arm.ChatChoiceInset[1] * UI.identityScale[1]
	local maxWidth = self.chatRect[3] - self.chatPos[1] - inset - (Arm.ChatClipAdjust[1]*UI.identityScale[1]) - 4
	self.chatPos[2] = self.chatPos[2] + (Arm.ChatChoiceExtraSpaceAfterPrompt*UI.identityScale[2])

	local startY = self.chatPos[2]
	self.currentRewardWidgets = {}
	
	if (self.rewardSkillPoints) then
		local msg = "+"..tostring(self.rewardSkillPoints)
		if (self.rewardSkillPoints > 1) then
			msg = msg.." "..StringTable.Get("ARM_REWARD_SKILLPOINTS")
		else
			msg = msg.." "..StringTable.Get("ARM_REWARD_SKILLPOINT")
		end
		
		local w = self:CreateRewardActionText(
			msg,
			inset,
			maxWidth
		)
		
		table.insert(self.currentRewardWidgets, w)
		self.widgets.chat.ChatList:AddItem(w)
		w:SetBlendWithParent(true)
		w:BlendTo({1,1,1,0}, 0)
	end
	
	if (self.rewardTopic) then
		for k,v in pairs(self.rewardTopic) do
			local rewardTopic = v -- this may change as the conversation does so record it
			local w, r = self:CreateRewardActionButton(
				"> "..StringTable.Get("ARM_REWARD_TOPIC").." "..Arm:FindChatString(rewardTopic), 
				inset,
				maxWidth,
				function ()
					self.requestedTopic = rewardTopic
					Arm:StartConversation()
				end
			)
			
			table.insert(self.currentRewardWidgets, w)
			self.widgets.chat.ChatList:AddItem(w)
			w:SetBlendWithParent(true)
			w:BlendTo({1,1,1,0}, 0)
		end
	end
	
	if (self.rewardDiscover) then
		for k,v in pairs(self.rewardDiscover) do
			local dbItem = Arm.Discoveries[v]
			if (dbItem) then
				local w, r = self:CreateRewardActionButton(
					"> "..StringTable.Get("ARM_REWARD_DISCOVERY")..": "..StringTable.Get(dbItem.title), 
					inset,
					maxWidth,
					function ()
						Arm:OpenDatabaseItem(v)
					end
				)
				
				table.insert(self.currentRewardWidgets, w)
				self.widgets.chat.ChatList:AddItem(w)
				w:SetBlendWithParent(true)
				w:BlendTo({1,1,1,0}, 0)
			else
				COutLine(kC_Error, "There is no item called '%s' in the game database.", v)
			end
		end
	end
	
	if (self.rewardMessage) then
		for k,v in pairs(self.rewardMessage) do
			local w = self:CreateRewardActionText(
				StringTable.Get(v),
				inset,
				maxWidth
			)
			
			table.insert(self.currentRewardWidgets, w)
			self.widgets.chat.ChatList:AddItem(w)
			w:SetBlendWithParent(true)
			w:BlendTo({1,1,1,0}, 0)
		end
	end
	
	self.currentRewardWidgetsHeight = self.chatPos[2] - startY
end

function Arm.ChatPrompt(self, extraDelay)

	-- create the necessary chat controls and append them to the VList
	self.choiceWidgets = nil
	
	if (self.topic.priority) then
		self.topicPriority = self.topic.priority
	end
	
	assert(self.topicPriority)
	
	self.choices = Arm:ChatChoices(self.topic, self.topicPriority)
	
	if (next(self.topic.reply) == nil) then
		return
	end
	
	self.prompt = Arm:ChooseChatPrompt(self.topic.reply)
	assert(self.topic.stringTable)
		
	local promptText = StringTable.Get(self.prompt[1], self.topic.stringTable):gsub("%$name%$", GameDB.playerName)
	self.promptText = "> "..promptText
	
	local lock = false
	
	self.rewardTopic = nil
	self.rewardSkillPoints = nil
	self.rewardTrigger = nil
	self.rewardMessage = nil
	self.rewardSkill = nil
	self.rewardDiscover = nil
	self.currentRewardWidgets = nil
	
	if (self.topic.action) then
		local actions = string.split(self.topic.action, ";")
		lock = FindArrayElement(actions, "lock")
		
		if (self.horrorTopic) then
			if (FindArrayElement(actions, "clear_topic")) then
				self.clearTopicPending = true
			end
		end
		
		if (FindArrayElement(actions, "abort")) then
			Arm:ProcessActions(actions)
			if (self.clearTopicPending) then
				Arm:EndHorrorTopic()
			end
			return
		end
	end
	
	if (self.chatStart) then
		self.chatStart = false
	else
		if (lock) then
			EventLog:AddEvent(GameDB:ArmDateString(), "!ARM_LOCKED_REPLY", self.prompt[1])
		else
			EventLog:AddEvent(GameDB:ArmDateString(), "!ARM_REPLY", self.prompt[1])
		end
	end
	
	if (self.topic.action) then
		local actions = string.split(self.topic.action, ";")
		Arm:ProcessActions(actions)
	end
		
	if (lock) then
		EventLog:AddEvent(GameDB:ArmDateString(), "!ARM_LOCKED")
		Arm:ChatLockout() -- until a trigger.
	end
	
	self.promptState = {}
	
	-- Create text controls
	self.promptState.labels = {}
	local promptLineSize = UI:FontAdvanceSize(self.typefaces.Chat) + (Arm.ChatLineSpace * UI.identityScale[2])
	local scrollPos = {0, self.chatPos[2]}
	
	local typeface = self.typefaces.Chat
	if (GameDB.chatLockout) then
		typeface = self.typefaces.ChatLocked
	end
	
	-- how many lines?
	self.promptState.line = 1
	self.promptState.char = 0 -- utf32
	
	self.promptState.lines = {}
	
	local lines = string.split(self.promptText, "\n")
	for k,v in pairs(lines) do
	
		if (v:len() > 0) then
			local wraped = UI:WordWrap(
				self.typefaces.Chat, 
				v, 
				self.chatRect[3] - self.chatPos[1]
			)
			
			for k,v in pairs(wraped) do
				local r = {self.chatPos[1], self.chatPos[2], self.chatRect[3]-self.chatPos[1], promptLineSize}
				local w = UI:CreateWidget("TextLabel", {rect=r, typeface=typeface})
				self.widgets.chat.ChatList:AddItem(w)
				w:SetBlendWithParent(true)
				w:AllocateText(v)
				table.insert(self.promptState.labels, w)
				self.chatPos[2] = self.chatPos[2] + promptLineSize
				
				-- convert prompt text to UTF32 so we can reveal character by character in extended character sets
				table.insert(self.promptState.lines, System.UTF8To32(v))
			end
		else
			self.chatPos[2] = self.chatPos[2] + promptLineSize
		end
	end
		
	self:CreateRewardText()
	
	self.widgets.chat.ChatList:RecalcLayout()
	self.widgets.chat.ChatList:ScrollTo(scrollPos, 0.4)
	
	local thinkTime
	if (self.prompt.time == nil) then
		thinkTime = FloatRand(1, 2)
	else
		thinkTime = FloatRand(self.prompt.time[1], self.prompt.time[2])
	end
	
	extraDelay = extraDelay or 0
	
	self.promptState.flashDelay = Game.sysTime + extraDelay
	self.promptState.flashDuration = Game.sysTime + thinkTime + extraDelay
	
	local f = function()
		Arm:FlashCursor()
	end
	
	UI:SetLabelText(self.promptState.labels[1], ">");
	self.promptState.labels[1]:SetVisible(false)
	self.chatTimer = World.globalTimers:Add(f, 0.2, true)
	
	self.skipFun = function () Arm:SkipPrompt() end
end

function Arm.FlashCursor(self)
	local time = Game.sysTime
	
	if (self.promptState.flashDelay and (time < self.promptState.flashDelay)) then
		return
	end
	
	if (time >= self.promptState.flashDuration) then
		self.chatTimer:Clean()
		self.chatTimer = nil
		self.promptState.labels[1]:SetVisible(true)
		self.widgets.chat.ChatList:RecalcLayout()
		self:AnimatePrompt()
		return
	end
	
	local visible = not self.promptState.labels[1]:Visible()
	self.promptState.labels[1]:SetVisible(visible)
	
	if (self.promptState.flashDelay) then
		self.promptState.flashDelay = nil
		self.widgets.chat.ChatList:RecalcLayout()
	end
	
end

function Arm.AnimatePrompt(self)
	local f = function()
		Arm:TickPrompt()
	end
	
	local charactersPerSecond = 33
	self.chatTimer = World.globalTimers:Add(f, 1/charactersPerSecond, true)
end

function Arm.SkipRewards(self)
	if (self.currentRewardWidgets) then
		if (Arm.active and (Arm.mode == "chat")) then
			Arm.sfx.Reward:Play(kSoundChannel_UI, 0)
		end
		for k,v in pairs(self.currentRewardWidgets) do
			v:MoveTo({v.x, v.y}, {0,0})
			v:BlendTo({1,1,1,1}, 0.1)
		end
	end
	
	Arm:SkipChoices()
	collectgarbage()
end

function Arm.DisplayRewards(self)
	if (self.currentRewardWidgets) then
		if (Arm.active and (Arm.mode == "chat")) then
			Arm.sfx.Reward:Play(kSoundChannel_UI, 0)
		end
		for k,v in pairs(self.currentRewardWidgets) do
			v:MoveTo({v.x, v.y+self.currentRewardWidgetsHeight}, {0,0})
			v:MoveTo({v.x, v.y}, {0,0.3})
			v:BlendTo({1,1,1,1}, 0.1)
		end
		
		local f = function()
			Arm:DisplayChoices()
			collectgarbage()
		end
		self.chatTimer = World.globalTimers:Add(f, 0.8)
	else
		Arm:DisplayChoices()
		collectgarbage()
	end
end

function Arm.SkipPrompt(self)

	if (self.chatTimer) then
		self.chatTimer:Clean()
		self.chatTimer = nil
	end
	
	if (self.chatTimer2) then
		self.chatTimer2:Clean()
		self.chatTimer2 = nil
	end
	
	self.skipFun = nil
	if (self.promptState.labels[1]) then
		self.promptState.labels[1]:SetVisible(true)
	end
	
	for i=self.promptState.line,#self.promptState.lines do
	
		local utf32 = self.promptState.lines[i]
		local w = self.promptState.labels[i]
		self.promptState.labels[i] = nil
		
		local utf8 = System.UTF32To8(utf32)
		UI:SetLabelText(w, utf8)
		
		w:Unmap()
	end
	
	if (Arm.active and (Arm.mode == "chat")) then
		if (GameDB.chatLockout) then
			local f = function()
				Arm:DisplayChatLockout()
			end
			self.chatTimer2 = World.globalTimers:Add(f, 3)
		else
			Arm:EnableChangeTopic(true)
		end
	end
			
	Arm:SkipRewards()
	self.widgets.chat.ChatList:RecalcLayout()
	self.widgets.chat.ChatList:SetCapture(false)
	
end

function Arm.TickPrompt(self)
	local utf32 = self.promptState.lines[self.promptState.line]
	local w = self.promptState.labels[self.promptState.line]
	
	self.promptState.char = self.promptState.char + 4 -- utf32
		
	if (self.promptState.char > utf32:len()) then
		w:Unmap() -- mark for gc
		self.promptState.labels[self.promptState.line] = nil
		
		-- next line
		self.promptState.char = 4 -- utf32
		self.promptState.line = self.promptState.line + 1
		
		if (self.promptState.line > #self.promptState.lines) then
			self.chatTimer:Clean()
			self.chatTimer = nil
			self.skipFun = nil
			if (Arm.active and (Arm.mode == "chat")) then
				if (GameDB.chatLockout) then
					local f = function()
						Arm:DisplayChatLockout()
					end
					self.chatTimer2 = World.globalTimers:Add(f, 3)
				else
					local f = function()
						Arm:EnableChangeTopic(true)
					end
					self.chatTimer2 = World.globalTimers:Add(f, 2)
				end
			end
			Arm:DisplayRewards()
			return
		end
		
		utf32 = self.promptState.lines[self.promptState.line]
		w = self.promptState.labels[self.promptState.line]
	end
	
	utf32 = utf32:sub(1, self.promptState.char)
	local utf8 = System.UTF32To8(utf32)
	UI:SetLabelText(w, utf8)
end

function Arm.DisplayChatLockout(self)
	self.widgets.chat.Root:BlendTo({1,1,1,0}, 0.5)
		
	local f = function()
		self.armLockTimer = nil
		Abducted.entity.eatInput = false
		Arm:ShowSymbol(true, 0.1)
		Arm:ClearChat()
	end
	
	Abducted.entity.eatInput = true
	self.armLockTimer = World.globalTimers:Add(f, 0.5)
end

function Arm.ChatLockout(self, time)
	GameDB:LockoutChat(time)
	Arm:SwapToTalk()	
end

function Arm.CheckLockout(self)
	GameDB:CheckChatLockout()
end

function Arm.ClearLockout(self)
	GameDB:ClearChatLockout()
end

function Arm.SkipChoices(self)
	Arm:DisplayChoices(true)
end

function Arm.DisplayChoices(self, skip)

	if (self.clearTopicPending) then
		Arm:EndHorrorTopic()
		return
	end

	if (self.choices == nil) then
		return
	end

	local prompts = {}
	local promptText = {}
	local promptSizes = {}
	local lineWidth = 0
	
	self.choiceWidgets = {}
	
	self.chatPos[2] = self.chatPos[2] + (Arm.ChatChoiceExtraSpaceAfterPrompt*UI.identityScale[2])
	
	for k,v in pairs(self.choices) do
		assert(v.stringTable)
		
		local prompt = self:ChooseChatPrompt(v.prompt)
		prompts[k] = prompt
		
		local text = StringTable.Get(prompt[1], v.stringTable)
		if (text) then
			text = text:gsub("%$name%$", GameDB.playerName)
		end
		promptText[k] = text
		
		local w,h = UI:StringDimensions(self.typefaces.ChatChoice, text)
		promptSizes[k] = {w, h}
		
		lineWidth = lineWidth + w
		
	end
	
	-- did we exceed the maximum line width?
	local inset = Arm.ChatChoiceInset[1] * UI.identityScale[1]
	local maxWidth = self.chatRect[3] - self.chatPos[1] - inset - (Arm.ChatClipAdjust[1]*UI.identityScale[1]) - 4
	local horzSpace = Arm.ChatChoiceHorzSpace * UI.identityScale[1]
	local horzSpacePerItem = (horzSpace+(Arm.ChatChoiceButtonPadd[1]*UI.identityScale[1]))
	local totalHorzSpacing = horzSpacePerItem * (#prompts - 1)
	
	lineWidth = totalHorzSpacing + lineWidth
	
	if (lineWidth > maxWidth) then
		-- we have to do word-wrap to make this all fit
		local spaceForControls = maxWidth - totalHorzSpacing
		maxWidth = spaceForControls / #prompts -- max space per prompt
		
	else
		maxWidth = nil -- no word wrap needed
	end
	
	-- generate choice buttons
	local lineHeight = 0
	local xPos = self.chatPos[1]+inset
	local lastRect
	local promptsLeft = #prompts
	
	for k,v in pairs(prompts) do
	
		local prompt = v
		local text = promptText[k]
		local size = promptSizes[k]
		
		local r = {0, 0, size[1], size[2]}
		local f = function(widget)
			Arm:ChoiceSelected(widget, self.choices[k], prompt)
		end
		
		local w = UI:CreateStylePushButton(
			r,
			f,
			{
				background = false,
				highlight = { 
					on = {0.75, 0.75, 0.75, 1}, 
					time = 0.2 
				}, 
				typeface = self.typefaces.ChatChoice,
				pressed = self.sfx.Button
			}
		)
		
		if (maxWidth and (size[1] > maxWidth)) then
			r = UI:LineWrapCenterText(w.label, maxWidth, true, Arm.ChatChoiceSpace, text)
		else
			UI:SetLabelText(w.label, text)
			r = UI:SizeLabelToContents(w.label)
		end
		
		-- expand the button around the label
		local buttonRect = {
			xPos, 
			self.chatPos[2],
			r[3]+Arm.ChatChoiceButtonPadd[1]*UI.identityScale[1],
			r[4]+Arm.ChatChoiceButtonPadd[2]*UI.identityScale[2]
		}
		
		w:SetRect(buttonRect)
		w:SetBlendWithParent(true)
		w.highlight:SetRect({0, 0, buttonRect[3], buttonRect[4]})
		r = CenterChildRectInRect(r, buttonRect)
		w.label:SetRect(r)
		
		xPos = xPos + buttonRect[3] + horzSpace
		lineHeight = Max(lineHeight, buttonRect[4])
		
		w:BlendTo({1,1,1,0}, 0)
		self.widgets.chat.ChatList:AddItem(w)
		table.insert(self.choiceWidgets, w)
		
		lastRect = buttonRect
		
		promptsLeft = promptsLeft - 1
		-- re-adjust some items won't use all the space
		if (maxWidth) then
			maxWidth = self.chatRect[3] - xPos
			local spaceForControls = maxWidth - (horzSpacePerItem * promptsLeft)
			maxWidth = spaceForControls / promptsLeft
		end
	end
	
	self.widgets.chat.ChatList:RecalcLayout()
	self.widgets.chat.ChatList:ScrollTo({0, lastRect[2]}, 0.4)
	
	self.chatPos[2] = self.chatPos[2] + lineHeight + (Arm.ChatLineSpace*UI.identityScale[2])
	
	if (skip) then
		for k,v in pairs(self.choiceWidgets) do
			v:BlendTo({1,1,1,1}, 0)
		end
	else
		local f = function()
		
			for k,v in pairs(self.choiceWidgets) do
				v:BlendTo({1,1,1,1}, 0.75)
			end
		
		end
		
		self.chatTimer = World.globalTimers:Add(f, 1)
	end
end

function Arm.ChoiceSelected(self, widget, choice, prompt, text)

	Achievements:DiscussedTopic()
	
	self.changeConversationCount = 0
	
	-- add event
	EventLog:AddEvent(GameDB:ArmDateString(), "!ARM_ASK", prompt[1])
	
	-- disable all choices
	for k,v in pairs(self.choiceWidgets) do
	
		v.disableGfxChanges = true
		v.class:SetEnabled(v, false)
		
		if (v ~= widget) then
			v:BlendTo({0.5, 0.5, 0.5, 1}, 0.3)
		end
		
		v:Unmap() -- allow gc to collect this
	
	end
	
	self.choiceWidgets = nil
	self.topic = choice
	
	if (choice.generated) then -- flag as used
		self.topicTree[choice.generated] = choice
		Persistence.WriteBool(SaveGame, "armGeneratedTopicChosen", true, choice.generated)
	end
	
	Arm:ChatPrompt(1.5)
end

function Arm.SelectChatRoot(self)

	if (self.changeConversationCount > Arm.MaxChangeConversationTimes) then
		EventLog:AddEvent(GameDB:ArmDateString(), "!ARM_LOCKED")
		Arm:ChatLockout(FloatRand(35, 95))
		return {reply={{"ARM_CHAT_WE_WILL_TALK_LATER"}}, stringTable=StringTable.Global}
	end

	local roots = {}
	local sum = 0
	
	for k,v in pairs(Arm.Chats.Available) do
		if (v == nil) then
			error(string.format("Error in arm chat table for chat root %s", k))
		end
		local num = Persistence.ReadNumber(SaveGame, "chatRoots", 0, k)
		if (num == 0) then -- never seen this one
			local p = v.prob
			if (p == nil) then
				p = {1, 1}
			end
			v.n = FloatRand(p[1], p[2])
			sum = sum + v.n
			roots[k] = v
		end
	
	end
	
	if (next(roots) == nil) then
		-- no unviewed chat roots, add all roots
		for k,v in pairs(Arm.Chats.Available) do
			local p = v.prob
			if (p == nil) then
				p = {1, 1}
			end
			v.n = FloatRand(p[1], p[2])
			sum = sum + v.n
			roots[k] = v
		end
	end
	
	if (sum == 0) then
		sum = 1
	end
	
	-- normalize probabilities
	local ofs = 0

	for k,v in pairs(roots) do
	
		local n = v.n / sum
		v.n = ofs + n
		ofs = ofs + n
	
	end
	
	-- random topic
	local last, name
	local p = math.random()
	
	for k,v in pairs(roots) do
		if (p <= v.n) then
			local num = Persistence.ReadNumber(SaveGame, "chatRoots", 0, k)
			num = num + 1
			Persistence.WriteNumber(SaveGame, "chatRoots", num, k)
			return v
		end
		last = v
		name = k
	end
	
	return last, name -- precision
end

function Arm.ChooseChatPrompt(self, root)

	-- randomly choose prompt
	local sum = 0
	
	for k,v in pairs(root) do
		if (v == nil) then
			error(string.format("Error in arm chat table for prompt %s", k))
		end
		local p = v.prob
		if (p == nil) then
			p = {1, 1}
		end
		v.n = FloatRand(p[1], p[2])
		sum = sum + v.n
	end
	
	if (sum == 0) then
		sum = 1
	end
	
	-- normalize 
	local ofs = 0
	for k,v in pairs(root) do
		local n = v.n / sum
		v.n = ofs + n
		ofs = v.n
	end
	
	-- choose
	local last
	local p = math.random()
	
	for k,v in pairs(root) do
		if (p <= v.n) then
			return v
		end
		last = v
	end

	return last -- precision
end

function Arm.ChatChoices(self, root, priority)

	local responses = {}
	
	if (root.choices) then
		for k,v in pairs(root.choices) do
			if (v == nil) then
				error(string.format("Error in arm chat table for choice %s", k))
			end
			if (v.prob) then
				local p = math.random()
				if (p <= v.prob) then
					table.insert(responses, v)
				end
			else
				table.insert(responses, v)
			end
		end
	end
	
	if (bit.band(root.flags, kArmChatFlag_AutoGenerate) ~= 0) then
		Arm:FillInChoices(responses, priority)
	end
	
	if (bit.band(root.flags, kArmChatFlag_ShuffleChoices) ~= 0) then
		Arm:ShuffleResponses(responses)
	end
	
	if (#responses == 0) then
		responses = nil
	end
	
	return responses

end

function Arm.ShuffleResponses(self, responses)

	return ShuffleArray(responses)

end

Arm.AutoFillChoiceCount = 3

function Arm.FillInChoices(self, responses, priority)
	if (#responses >= Arm.AutoFillChoiceCount) then
		return
	end
	
	if (priority < 1) then
		priority = 1
	end
	
	local exit = false
	local added = {}
	
	local proceduralRoot = Arm:TopicIsProcedural(self.topic)
	
	for k,v in pairs(responses) do
		added[v.name] = true -- don't let procedurals have their own choices repeated
		if (proceduralRoot) then
			v.generated = v.name
		end
	end
	
	local groups = self.topic.group
	local defaultGroup = {"Default"}
	local noDefault = false
	
	if (groups) then
		noDefault = FindArrayElement(groups, "NoDefault")
	end
		
	local prioritySort = {}
	
	for i = priority,10 do
		table.insert(prioritySort, i)
	end
	
	for z = 1,4 do
		if (z == 3) then
			prioritySort = {}
			for i = 1,10 do
				table.insert(prioritySort, i)
			end
		end
		for i = 1,#prioritySort do
			local t = nil
			if ((z == 1) or (z == 2)) then
				t = Arm:SortProceduralTopics(prioritySort[i], groups or defaultGroup)
			elseif (not noDefault) then
				t = Arm:SortProceduralTopics(prioritySort[i], defaultGroup)
			end
			
			if (t ~= nil) then
				for k,v in pairs(t) do
					if (self.topicTree[v.name] == nil) then -- has not been selected by user
						local chosen = false
						if ((z == 1) or (z == 3)) then
							-- knock out this choices if the user has selected it before
							chosen = Persistence.ReadBool(SaveGame, "armGeneratedTopicChosen", false, v.name)
						end
						
						if (chosen) then
							-- 15 % chance an item that was chosen before will show up
							-- after they have swapped conversations 4 times
							if ((self.changeConversationCount >= 4) and (math.random() <= 0.15)) then
								chosen = false
							end
						end
						
						if (not chosen) then
							if (added[v.name]) then
								chosen = true
							end
						end
						
						if (not chosen) then
							v.generated = v.name -- flag as a generated option
							table.insert(responses, v)
							added[v.name] = true
						end
					end
					
					if (#responses >= Arm.AutoFillChoiceCount) then
						exit = true
						break
					end
				end
				
				if (exit) then
					break
				end
			end
		end
		if (exit) then
			break
		end
		-- next time through we randomize the priority.
		prioritySort = ShuffleArray(prioritySort)
	end	
end
