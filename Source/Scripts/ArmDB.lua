-- ArmDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.DBBorder = {8, 8, -8, -8}

function Arm.SpawnDB(self)

	local rect = {
		Arm.DBBorder[1] * UI.identityScale[1],
		Arm.DBBorder[2] * UI.identityScale[2],
		0,
		0
	}
	
	rect[3] = self.workspaceLeftSize[3] - rect[1] + (Arm.DBBorder[3] * UI.identityScale[1])
	rect[4] = self.workspaceLeftSize[4] - rect[2] + (Arm.DBBorder[4] * UI.identityScale[2])
	
	self.dbRootWorkspaceSize = {
		0,
		0,
		rect[3],
		rect[4]
	}

	self.widgets.db = {}
	self.widgets.db.Root = UI:CreateWidget("Widget", {rect=rect})
	self.widgets.db.Root:SetVisible(false)
	self.widgets.WorkspaceLeft:AddChild(self.widgets.db.Root)
	
	rect = {0,39,198,576}--UI:MaterialSize(self.gfx.CharTab)
	local scale = self.dbRootWorkspaceSize[4] / rect[4]
	rect[4] = self.dbRootWorkspaceSize[4]
	rect[3] = rect[3] * scale
	
	self.widgets.db.Tab = UIPushButton:Create(
		rect,
		{
			pressed = self.gfx.CharTabPressed,
			enabled = self.gfx.CharTab
		},
		{
			pressed = self.sfx.Button
		},
		{
			pressed = Arm.DBTabPressed
		},
		nil,
		self.widgets.db.Root
	)
	
	self.widgets.db.Tab.rect = rect
	self.widgets.db.Tab:SetBlendWithParent(true)
	self.widgets.db.Tab:SetHAlign(kHorizontalAlign_Right)
	
	self.widgets.db.TabTitle = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.StandardButton})
	self.widgets.db.Root:AddChild(self.widgets.db.TabTitle)
	self.widgets.db.TabTitle:SetBlendWithParent(true)
	self.widgets.db.TabTitle.rect = {0, 0, rect[3], rect[2]}
	
	self.dbWorkspace = {
		rect[3] + (4 * UI.identityScale[1]),
		0,
		0,
		0
	}
	
	self.dbWorkspace[3] = self.dbRootWorkspaceSize[3] - self.dbWorkspace[1]
	self.dbWorkspace[4] = self.dbRootWorkspaceSize[4]
	
	self.dbWorkspaceSize = {
		0,
		0,
		self.dbWorkspace[3],
		self.dbWorkspace[4]
	}
	
	Arm:SpawnCharacterDB()
	Arm:SpawnEventDB()
	Arm:SpawnDiscoveriesDB()
end

function Arm.StartDB(self)
	if (self.requestedDBTopic) then
		self.dbMode = "discoveries"
		Persistence.WriteString(Session, "dbMode", "discoveries")
	else
		self.dbMode = Persistence.ReadString(Session, "dbMode", "char")
	end
	Arm:DBResetAll()
	Arm:DBIntro(self.dbMode)
end

function Arm.EndDB(self, callback)

	if (self.dbTimer) then
		self.dbTimer:Clean()
		self.dbTimer = nil
	end
	
	self.widgets.db.Root:BlendTo({1,1,1,0}, 0.2)
	
	local f = function()
		self.widgets.db.Root:SetVisible(false)
		if (self.dbActive) then
			self.dbActive(self, false)
			self.dbActive = nil
		end
		callback()
	end
	
	World.globalTimers:Add(f, 0.2)

end

function Arm.DBTabPressed(self)
	local self = Arm
	
	if (self.dbMode == "char") then
		self.dbMode = "log"
	elseif (self.dbMode == "log") then
		self.dbMode = "discoveries"
	elseif (self.dbMode == "discoveries") then
		self.dbMode = "char"
	end
	
	Persistence.WriteString(Session, "dbMode", self.dbMode)
	Session:Save()
	
	self:DBShowPanel(true, self.dbMode)
	
end

function Arm.DBHide(self)
	if (self.dbTimer) then
		self.dbTimer:Clean()
		self.dbTimer = nil
	end
	
	if (self.dbActive) then
		self.dbActive(self, false)
		self.dbActive = nil
	end
	
	Arm:ClearLog()
	Arm:ClearDiscoveries()
	self.widgets.db.Root:SetVisible(false)
end

function Arm.DBResetAll(self)

	self.widgets.db.Root:BlendTo({1,1,1,1}, 0)
	self.widgets.db.Root:SetVisible(true)
	self.widgets.db.CharRoot:BlendTo({1,1,1,0}, 0)
	self.widgets.db.EventLog:BlendTo({1,1,1,0}, 0)
	self.widgets.db.DiscoveriesRoot:BlendTo({1,1,1,0}, 0)
	self.widgets.db.Tab:BlendTo({1,1,1,0}, 0)
	self.widgets.db.TabTitle:BlendTo({1,1,1,0}, 0)
	
end

function Arm.DBIntro(self, mode)
	self.widgets.db.Tab:MoveTo({self.dbRootWorkspaceSize[3], self.widgets.db.Tab.rect[2]}, {0, 0})
	self.widgets.db.Tab:MoveTo({0, self.widgets.db.Tab.rect[2]}, {0.3, 0})
	self.widgets.db.Tab:BlendTo({1,1,1,1}, 0.3)
	
	local f = function()
		Arm:DBShowPanel(true, mode)
	end
	
	World.globalTimers:Add(f, 0.3)
	
end

function Arm.ChangeDBTitleText(self, text, time)
	
	UI:SetLabelText(self.widgets.db.TabTitle, text)
	UI:SizeLabelToContents(self.widgets.db.TabTitle)
	UI:CenterLabel(self.widgets.db.TabTitle, self.widgets.db.TabTitle.rect)
	self.widgets.db.TabTitle:BlendTo({1,1,1,1}, time)
	
end

function Arm.DBShowPanel(self, show, mode, callback, animateTab)
	if (show) then
		if (self.dbActive) then
			local f = function()
				Arm:DBShowPanel(true, mode, callback, true)
			end
			Arm:DBShowPanel(false, nil, f)
		else
			local enabled = nil
			local pressed = nil
			local title = nil
			if (mode == "char") then
				self.dbActive = Arm.EnterCharDB
				enabled = self.gfx.CharTab
				pressed = self.gfx.CharTabPressed
				title = StringTable.Get("DBTAB_CHARACTER")
			elseif (mode == "log") then
				self.dbActive = Arm.EnterLogDB
				enabled = self.gfx.LogTab
				pressed = self.gfx.LogTabPressed
				title = StringTable.Get("DBTAB_EVENTLOG")
			elseif (mode == "discoveries") then
				self.dbActive = Arm.EnterDiscoveriesDB
				enabled = self.gfx.DiscoveriesTab
				pressed = self.gfx.DiscoveriesTabPressed
				title = StringTable.Get("ARM_DATABASE_BTN")
			end
			self.widgets.db.Tab.class:ChangeGfx(self.widgets.db.Tab, {enabled=enabled, pressed=pressed})
			if (animateTab) then
				self.widgets.db.Tab:BlendTo({1,1,1,1}, 0)
				self.widgets.db.Tab:ScaleTo({0,1}, {0,0})
				self.widgets.db.Tab:ScaleTo({1,1}, {0.2,0})
				self:ChangeDBTitleText(title, 0.2)
			else
				self:ChangeDBTitleText(title, 0)
			end
			self.dbActive(self, true, callback, 0.2)
		end
	else
		self.widgets.db.Tab:BlendTo({1,1,1,0}, 0.2)
		self.widgets.db.TabTitle:BlendTo({1,1,1,0}, 0.2)
		self.dbActive(self, false, callback, 0.2)
		self.dbActive = nil
	end
end
