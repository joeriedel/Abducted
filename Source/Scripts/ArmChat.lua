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
Arm.MaxChangeConversationTimes = 4

function Arm.SpawnChat(self)

	self.widgets.chat = {}
	self.widgets.chat.Root = UI:CreateWidget("Widget", {rect=self.workspaceLeftSize})
	self.widgets.chat.Root:SetVisible(false)
	self.widgets.WorkspaceLeft:AddChild(self.widgets.chat.Root)
	
	self.chatRect = {
		12*UI.identityScale[1], 
		10*UI.identityScale[2],
		self.workspaceLeft[3] - (24*UI.identityScale[1]), 
		self.workspaceLeft[4] - (20*UI.identityScale[2])
	}
	
	self.widgets.chat.ChatList = UI:CreateWidget("VListWidget", {rect=self.chatRect})
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

function Arm.ResetChat(self)
	GameDB:CheckChatLockout()
	self.widgets.chat.Root:SetVisible(false)
	self.widgets.chat.Root:BlendTo({1,1,1,1}, 0)
end

function Arm.StartChat(self)
	if (not GameDB.chatLockout) then
		Arm:SwapToChange()
		self.widgets.chat.Root:SetVisible(true)
		self.widgets.chat.Root:BlendTo({1,1,1,1}, 0)
		self.changeConversationCount = 0
		Arm:StartConversation()
	else
		Arm:DisplayChatLockout()
	end
end

function Arm.EndChat(self, callback)
	Arm:SwapToTalk()
	
	self.widgets.chat.Root:BlendTo({0,0,0,0}, 0.2)
	
	if (GameDB.chatLockout) then
		Arm:ShowSymbol(false, 0.2)
	end
	
	local f = function()
		Arm:ClearChat()
		self.widgets.chat.Root:SetVisible(false)
		callback()
	end
	
	World.globalTimers:Add(f, 0.2, true)
end

function Arm.ClearChat(self)
	if (self.chatTimer) then
		self.chatTimer:Clean()
		self.chatTimer = nil
	end
	
	if (self.chatTimer2) then
		self.chatTimer2:Clean()
		self.chatTimer2 = nil
	end
	
	self.promptState = nil
	
	self.choices = nil
	self.choiceWidgets = nil
	self.widgets.chat.ChatList:Clear()
	collectgarbage()
end

function Arm.HideChat(self)

	Arm:ClearChat()

end

function Arm.LoadChats(self, chats)

	if (chats == "Ep1Crushed") then
		Arm:LoadEp1Crushed()
	elseif (chats == "Ep1Eaten") then
		Arm:LoadEp1Eaten()
	elseif (chats == "Ep1Falling") then
		Arm:LoadEp1Falling()
	end

end

function Arm.StartConversation(self)

	Arm:EnableChangeTopic(false)
	
	self.chatPos = {Arm.ChatInset[1]*UI.identityScale[1], Arm.ChatInset[2]*UI.identityScale[2]}
	
	if (self.requiredTopic) then
		self.changeConversationCount = 0
		self.topic = Arm.Chats.Root[self.requiredTopic]
	else
		self.changeConversationCount = self.changeConversationCount + 1
		self.topic = self:SelectChatRoot()
	end
	
	if (self.topic) then
		self:ChatPrompt()
	end

end

function Arm.ChatPrompt(self)

	-- create the necessaray chat controls and append them to the VList
	self.choiceWidgets = nil
	
	if (self.topic.choices) then
		self.choices = Arm:ChatChoices(self.topic)
	else
		self.choices = nil
	end
	
	if (self.topic.reply == nil) then
		self.topic.reply = {{"ARM_CHAT_I_DONT_KNOW"}}
	end
	
	self.prompt = Arm:ChooseChatPrompt(self.topic.reply)
	self.promptText = "> "..StringTable.Get(self.prompt[1])
	
	if (self.prompt[1] ~= "ARM_CHAT_DEFAULT_PROMPT") then
		EventLog:AddEvent(GameDB:CurrentTimeString().." !ARM_REPLY "..self.prompt[1])
	end
	
	if (self.topic.action) then
		if (self.topic.action == "lock") then
			EventLog:AddEvent(GameDB:CurrentTimeString().." !ARM_LOCKED")
			Arm:ChatLockout() -- until a trigger.
		end
	end
	
	-- how many lines?
	self.promptState = {}
	self.promptState.line = 1
	self.promptState.char = 1 -- always start with actual prompt text
	self.promptState.lines = UI:WordWrap(
		self.typefaces.Chat, 
		self.promptText, 
		self.chatRect[3] - self.chatPos[1]
	)
	
	-- Create text controls
	self.promptState.labels = {}
	local promptLineSize = UI:FontAdvanceSize(self.typefaces.Chat) + (Arm.ChatLineSpace * UI.identityScale[2])
	local scrollPos = {0, self.chatPos[2]}
	
	for k,v in pairs(self.promptState.lines) do
		
		local r = {self.chatPos[1], self.chatPos[2], self.chatRect[3]-self.chatPos[1], promptLineSize}
		local w = UI:CreateWidget("TextLabel", {rect=r, typeface=self.typefaces.Chat})
		self.widgets.chat.ChatList:AddItem(w)
		w:SetBlendWithParent(true)
		w:AllocateText(v)
		table.insert(self.promptState.labels, w)
		self.chatPos[2] = self.chatPos[2] + promptLineSize
	end

	self.widgets.chat.ChatList:RecalcLayout()
	self.widgets.chat.ChatList:ScrollTo(scrollPos, 0.4)
	
	local thinkTime
	if (self.prompt.time == nil) then
		thinkTime = FloatRand(1, 2)
	else
		thinkTime = FloatRand(self.prompt.time[1], self.prompt.time[2])
	end
	
	self.promptState.flashDuration = Game.sysTime + thinkTime
	
	local f = function()
		Arm:FlashCursor()
	end
	
	UI:SetLabelText(self.promptState.labels[1], ">");
	self.promptState.labels[1]:SetVisible(false)
	self.chatTimer = World.globalTimers:Add(f, 0.2, false)
	
end

function Arm.FlashCursor(self)
	local time = Game.sysTime
	
	if (time >= self.promptState.flashDuration) then
		self.chatTimer:Clean()
		self.chatTimer = nil
		self.promptState.labels[1]:SetVisible(true)
		self:AnimatePrompt()
		return;
	end
	
	local visible = not self.promptState.labels[1]:Visible()
	self.promptState.labels[1]:SetVisible(visible)
end

function Arm.AnimatePrompt(self)
	local f = function()
		Arm:TickPrompt()
	end
	
	local charactersPerSecond = 30
	self.chatTimer = World.globalTimers:Add(f, 1/charactersPerSecond, false)
end

function Arm.TickPrompt(self)
	local str = self.promptState.lines[self.promptState.line]
	
	self.promptState.char = self.promptState.char + 1
	
	if (self.promptState.char > str:len()) then
		-- next line
		self.promptState.char = 1
		self.promptState.line = self.promptState.line + 1
		
		if (self.promptState.line > #self.promptState.lines) then
			self.chatTimer:Clean()
			self.chatTimer = nil
			if (GameDB.chatLockout) then
				local f = function()
					Arm:DisplayChatLockout()
				end
				self.chatTimer2 = World.globalTimers:Add(f, 3, true)
			else
				local f = function()
					Arm:EnableChangeTopic(true)
				end
				self.chatTimer2 = World.globalTimers:Add(f, 2, true)
			end
			Arm:DisplayChoices()
			collectgarbage()
			return
		end
		
		str = self.promptState.lines[self.promptState.line]
	end
	
	local w = self.promptState.labels[self.promptState.line]
	str = str:sub(1, self.promptState.char)
	UI:SetLabelText(w, str)
end

function Arm.DisplayChatLockout(self)
	self.widgets.chat.Root:BlendTo({0,0,0,0}, 0.5)
		
	local f = function()
		Abducted.entity.eatInput = false
		self:ShowSymbol(true, 0.1)
	end
	
	Abducted.entity.eatInput = true
	World.globalTimers:Add(f, 0.5, true)
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

function Arm.DisplayChoices(self)

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
	
		local prompt = self:ChooseChatPrompt(v.prompt)
		prompts[k] = prompt
		
		local text = StringTable.Get(prompt[1])
		promptText[k] = text
		
		local w,h = UI:StringDimensions(self.typefaces.ChatChoice, text)
		promptSizes[k] = {w, h}
		
		lineWidth = lineWidth + w
		
	end
	
	-- did we exceed the maximum line width?
	local inset = Arm.ChatChoiceInset[1] * UI.identityScale[1]
	local maxWidth = self.chatRect[3] - self.chatPos[1] - inset
	local horzSpace = Arm.ChatChoiceHorzSpace * UI.identityScale[1]
	if (lineWidth > maxWidth) then
	
		-- we have to do word-wrap to make this all fit
		local totalHorzSpacing = horzSpace
		local spaceForControls = maxWidth - totalHorzSpacing
		maxWidth = spaceForControls / #prompts -- max space per prompt
		
	else
		maxWidth = nil -- no word wrap needed
	end
	
	-- generate choice buttons
	local lineHeight = 0
	local xPos = self.chatPos[1]+inset
	local lastRect
	
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
		r = CenterChildRectInRect(buttonRect, r)
		w.label:SetRect(r)
		
		xPos = xPos + buttonRect[3] + horzSpace
		lineHeight = Max(lineHeight, buttonRect[4])
		
		w:BlendTo({0,0,0,0}, 0)
		self.widgets.chat.ChatList:AddItem(w)
		table.insert(self.choiceWidgets, w)
		
		lastRect = buttonRect
	end
	
	self.widgets.chat.ChatList:RecalcLayout()
	self.widgets.chat.ChatList:ScrollTo({0, lastRect[2]}, 0.4)
	
	self.chatPos[2] = self.chatPos[2] + lineHeight + (Arm.ChatLineSpace*UI.identityScale[2])
	
	local f = function()
	
		for k,v in pairs(self.choiceWidgets) do
			v:BlendTo({1,1,1,1}, 0.75)
		end
	
	end
	
	self.chatTimer = World.globalTimers:Add(f, 1, true)
end

function Arm.ChoiceSelected(self, widget, choice, prompt)

	self.changeConversationCount = 0
	
	-- add event
	EventLog:AddEvent(GameDB:CurrentTimeString().." !ARM_ASK "..prompt[1])
	
	-- disable all choices
	for k,v in pairs(self.choiceWidgets) do
	
		v.disableGfxChanges = true
		v.class:SetEnabled(v, false)
		
		if (v ~= widget) then
			v:BlendTo({0.5, 0.5, 0.5, 1}, 0.3)
		end
	
	end
	
	self.topic = choice
	
	local f = function()
		Arm:ChatPrompt()
	end
	
	self.chatTimer = World.globalTimers:Add(f, 1.5, true)

end

function Arm.SelectChatRoot(self)

	if (self.changeConversationCount > Arm.MaxChangeConversationTimes) then
		EventLog:AddEvent(GameDB:CurrentTimeString().." !ARM_LOCKED")
		Arm:ChatLockout(FloatRand(35, 95))
		return {reply={{"ARM_CHAT_WE_WILL_TALK_LATER"}}}
	end

	local roots = {}
	local sum = 0
	
	for k,v in pairs(Arm.Chats.Available) do
		if (v == nil) then
			error("Error in arm chat table for chat root %s", k)
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
	local last
	local p = math.random()
	
	for k,v in pairs(roots) do
		if (p <= v.n) then
			local num = Persistence.ReadNumber(SaveGame, "chatRoots", 0, k)
			num = num + 1
			Persistence.WriteNumber(SaveGame, "chatRoots", num, k)
			return v
		end
		last = v
	end
	
	return last -- precision
end

function Arm.ChooseChatPrompt(self, root)

	-- randomly choose prompt
	local sum = 0
	
	for k,v in pairs(root) do
		if (v == nil) then
			error("Error in arm chat table for prompt %s", k)
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
		ofs = ofs + n
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

	return v -- precision
end

function Arm.ChatChoices(self, root)

	local responses = {}
	
	for k,v in pairs(root.choices) do
		if (v == nil) then
			error("Error in arm chat table for choice %s", k)
		end
		if (v.prob) then
			local p = math.random()
			if (p <= v.prob) then
				table.insert(responses, v[1])
			end
		else
			table.insert(responses, v[1])
		end
	end
	
	return responses

end