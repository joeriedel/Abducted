-- MainMenuNewGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu.NewGame = MainMenuPanel:New()
MainMenu.NewGame.StartingLevel = "Ep1/ep1_falling"

function MainMenu.InitNewGame(self)
	self.newGamePanel = MainMenu.NewGame:New()
	
	local rect = {
		0,
		0,
		384 * UI.identityScale[1],
		600 * UI.identityScale[2]
	}
	
	self.newGamePanel:Create({rect=rect}, self.widgets.root)
	UI:CenterWidget(self.newGamePanel.widgets.panel, self.contentRect)
	
	self.newGamePanel:Layout()
	self.newGamePanel.widgets.panel:SetHAlign(kHorizontalAlign_Center)
	self.newGamePanel.widgets.panel:SetVAlign(kVerticalAlign_Center)
	self.newGamePanel.widgets.panel:SetVisible(false)
end

function MainMenu.NewGame.Create(self, options, parent)
	
	option = MainMenuPanel.Create(self, options, parent)
	
	self.widgets.group = UI:CreateWidget("Widget", {rect={0, 0, options.rect[3], options.rect[4]}})
	self.widgets.group:SetBlendWithParent(true)
	self.widgets.panel:AddChild(self.widgets.group)
	
	self.xInset = 16 * UI.identityScale[1]
	
	local y = 16 * UI.identityScale[2]
	
	local w = 256 * UI.identityScale[1]
	local h = w
	local x = (options.rect[3] - (self.xInset*2) - w) / 2
	
	self.widgets.portrait = UI:CreateStylePushButton(
		{x+self.xInset, y, w, h},
		function() self:NextPortrait() end,
		{ nolabel = true, highlight={on={0,0,0,0}} },
		self.widgets.group
	)
	
	self.widgets.portrait:SetBlendWithParent(true)
	
	local widget
	
	widget = UI:CreateWidget("MatWidget", {rect={x+self.xInset+w, y, 64*UI.identityScale[1], h}, material=MainMenu.gfx.PortraitSelectArrow})
	widget:SetBlendWithParent(true)
	self.widgets.group:AddChild(widget)
		
	y = y + h + (16 * UI.identityScale[2])

	widget = UI:CreateWidget("TextLabel", {rect = {self.xInset, y, 8, 8}, typeface = MainMenu.typefaces.Normal})
	self.widgets.group:AddChild(widget)
	widget:SetBlendWithParent(true)
	UI:SetLabelText(widget, StringTable.Get("ARM_CHARDB_NAME"))
	local r = UI:SizeLabelToContents(widget)
	UI:HCenterWidget(widget, {self.xInset,0,options.rect[3]-(self.xInset*2), options.rect[4]})
	y = y + r[4] + (8*UI.identityScale[2])
	
	self.widgets.name = UI:CreateWidget("TextLabel", {rect = {self.xInset, y, 8, 8}, typeface = MainMenu.typefaces.Gold})
	self.widgets.group:AddChild(self.widgets.name)
	self.widgets.name:SetBlendWithParent(true)
	
	y = y + r[4] + (24*UI.identityScale[2])
	
	widget = UI:CreateWidget("MatWidget", {rect={0, y, options.rect[3], 7}, material = MainMenu.gfx.LineBorder4})
	self.widgets.group:AddChild(widget)
	widget:SetBlendWithParent(true)
	
	y = y + 7 + (24*UI.identityScale[2])
	
	w = 221 * UI.identityScale[1]
	h = 78 * UI.identityScale[2]
	x = (options.rect[3] - (self.xInset*2) - w) / 2
	
	widget = UI:CreateWidget("MatWidget", {rect={x+self.xInset,y,w,h}, material=UI.gfx.Solid})
	widget:BlendTo({0,0,0,1}, 0)
	widget:SetBlendWithParent(true)
	self.widgets.group:AddChild(widget)
	
	widget = UI:CreateStylePushButton(
		{ x+self.xInset, y, w, h },
		function () self:RenameCharacter() end,
		{ highlight={on={0,0,0,0}} },
		self.widgets.group
	)
	widget:SetBlendWithParent(true)
	
	local text = StringTable.Get("MM_RENAME_CHARACTER")
	UI:LineWrapCenterText(
		widget.label, 
		nil, 
		nil, 
		0, 
		text, 
		UI.identityScale,
		UI.invIdentityScale
	)
	
	y = y + h + (8 * UI.identityScale[2])
	
	widget = UI:CreateWidget("MatWidget", {rect={x+self.xInset,y,w,h}, material=UI.gfx.Solid})
	widget:BlendTo({0,0,0,1}, 0)
	widget:SetBlendWithParent(true)
	self.widgets.group:AddChild(widget)
	
	widget = UI:CreateStylePushButton(
		{ x+self.xInset, y, w, h },
		function () self:StartGame() end,
		{ highlight={on={0,0,0,0}} },
		self.widgets.group
	)
	widget:SetBlendWithParent(true)
	
	local text = StringTable.Get("MM_START_GAME")
	UI:LineWrapCenterText(
		widget.label, 
		nil, 
		nil, 
		0, 
		text, 
		UI.identityScale,
		UI.invIdentityScale
	)
	
end

function MainMenu.NewGame.PrepareContents(self)
	self.playerName = "Eve"
	UI:SetLabelText(self.widgets.name, self.playerName)
	UI:SizeLabelToContents(self.widgets.name)
	local r = self.widgets.panel:Rect()
	UI:HCenterWidget(self.widgets.name, {self.xInset,0,r[3]-(self.xInset*2),r[4]})
	
	-- start with portrait 1
	self.portrait = 1
	
	local gfx = {
		enabled = MainMenu.gfx.Portraits[self.portrait],
		highlight = UI.gfx.ButtonOverbright
	}
	
	self.widgets.portrait.class:ChangeGfx(self.widgets.portrait, gfx)
	
	MainMenuPanel.PrepareContents(self)
	self.widgets.group:BlendTo({1,1,1,0}, 0)
end

function MainMenu.NewGame.AnimateContents(self, onComplete)
	COutLine(kC_Debug, "NewGame.AnimateContents")
	self.widgets.group:BlendTo({1,1,1,1}, 0.2)
	if (onComplete) then
		local f = function()
			onComplete()
		end
		World.globalTimers:Add(f, 0.2)
	end
end

function MainMenu.NewGame.FadeOutContents(self, time)
	self.widgets.group:BlendTo({1,1,1,0}, time)
end

function MainMenu.NewGame.NextPortrait(self)
	self.portrait = self.portrait + 1
	if (MainMenu.gfx.Portraits[self.portrait] == nil) then
		self.portrait = 1
	end
	
	local gfx = {
		enabled = MainMenu.gfx.Portraits[self.portrait],
		highlight = UI.gfx.ButtonOverbright
	}
	self.widgets.portrait.class:ChangeGfx(self.widgets.portrait, gfx)
end

function MainMenu.NewGame.RenameCharacter(self)

end

function MainMenu.NewGame.StartGame(self)
	-- ok lets make a new game
	local index = 1
	if (MainMenu.saves) then
		index = #MainMenu.saves + 1
	end
	
	local filename = "Sav"..index..".dat"
	local saveGames = Globals.keys.saveGames or {}
	saveGames[tostring(index)] = filename
	Globals.keys.saveGames = saveGames
	
	SaveGame:Create(filename)
	Persistence.WriteString(SaveGame, "playerName", self.playerName)
	Persistence.WriteNumber(SaveGame, "portrait", self.portrait)
	Persistence.WriteString(SaveGame, "currentLevel", MainMenu.NewGame.StartingLevel)
	Persistence.WriteString(SaveGame, "lastPlayed", CurrentDateAndTimeString())
	SaveGame:Save()
		
	Persistence.WriteBool(Session, "loadCheckpoint", false)
	Session:Save()
	
	Persistence.WriteNumber(Globals, "checkpoint", index)
	Globals:Save()
	
	MainMenu.command = function()
		World.RequestLoad(MainMenu.NewGame.StartingLevel, kUnloadDisposition_Slot)
	end
	
	MainMenu.mainPanel:Exit()
end
