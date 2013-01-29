-- ArmChat.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function Arm.SpawnChat(self)

	self.widgets.chat = {}
	self.widgets.chat.Root = UI:CreateWidget("Widget", {rect=self.workspaceLeft})
	
	self.widgets.chat.ChatList = UI:CreateWidget("VListWidget", {rect=self.workspaceLeftSize})
	self.widgets.WorkspaceLeft:AddChild(self.widgets.chat.ChatList)
	self.widgets.chat.ChatList:SetVisible(false)
	self.widgets.chat.ChatList:SetEndStops({0, self.workspaceLeftSize[4]*0.1})
	
--	local spring = self.widgets.chat.ChatList:StopSpring()
--	spring.elasticity = 200
--	spring.length = 0.01
--	spring.tolerance = 0.003
--	self.widgets.chat.ChatList:SetStopSpring(spring)
	
--	local vertex = self.widgets.chat.ChatList:StopSpringVertex()
--	vertex.mass = 5
--	vertex.inner = true
--	vertex.outer = true
--	vertex.drag[1] = 10
--	vertex.drag[2] = 10
--	self.widgets.chat.ChatList:SetStopSpringVertex(vertex)
	
end

function Arm.StartChat(self)
	Arm:ClearChat()
	Arm:TestChatList()
end

function Arm.TestChatList(self)
	self.widgets.chat.ChatList:SetVisible(true)
	
	for i=1,4 do
		local w = UI:CreateWidget("MatWidget", {rect=UI:MaterialSize(self.gfx.Symbol), material=self.gfx.Symbol})
		self.widgets.chat.ChatList:AddItem(w)
	end
	
	self.widgets.chat.ChatList:ItemChanged()
	
end

function Arm.ClearChat(self)
	self.widgets.chat.ChatList:Clear()
	collectgarbage()
end

