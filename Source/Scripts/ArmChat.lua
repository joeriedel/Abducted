-- ArmChat.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function Arm.SpawnChat(self)

	self.widgets.chat = {}
	self.widgets.chat.Root = UI:CreateWidget("Widget", {rect=self.workspaceLeft})
	self.widgets.chat.Root:SetVisible(false)
	
	self.widgets.chat.ChatList = UI:CreateWidget("VListWidget", {rect=self.workspaceLeftSize})
	self.widgets.WorkspaceLeft:AddChild(self.widgets.chat.ChatList)
	self.widgets.chat.ChatList:SetEndStops({0, self.workspaceLeftSize[4]*0.1})
	
end

function Arm.ResetChat(self)
	self.widgets.chat.Root:SetVisible(false)
end

function Arm.StartChat(self)
	Arm:SwapToChange()
	Arm:EnableChangeTopic(false)
	Arm:TestChatList()
end

function Arm.EndChat(self)
	Arm:ClearChat()
	self:SwapToTalk()
end

function Arm.TestChatList(self)
	self.widgets.chat.ChatList:SetVisible(true)
	
	for i=1,4 do
		local w = UI:CreateWidget("MatWidget", {rect=UI:MaterialSize(self.gfx.Symbol), material=self.gfx.Symbol})
		self.widgets.chat.ChatList:AddItem(w)
	end
	
	self.widgets.chat.ChatList:DoVerticalLayout()
	
end

function Arm.ClearChat(self)
	self.widgets.chat.ChatList:Clear()
	collectgarbage()
end

