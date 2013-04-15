-- MainMenuLoadGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu.LoadGame = MainMenuPanel:New()

function MainMenu.InitLoadGame(self)

	self.loadGamePanel = MainMenu.LoadGame:New()
	
	self.loadGamePanel:Create({rect=self.contentRect}, self.widgets.root)
	UI:CenterWidget(self.loadGamePanel.widgets.panel, self.contentRect)
	self.loadGamePanel.widgets.panel:SetHAlign(kHorizontalAlign_Left)
	self.loadGamePanel.widgets.panel:SetVAlign(kVerticalAlign_Bottom)
	self.loadGamePanel.widgets.panel:SetVisible(false)
	
end

function MainMenu.LoadGame.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
	
	if (MainMenu.saves == nil) then
		return {0,0,8,8}
	end
	
	local inset = 8 * UI.identityScale[1]
	local picSize = 160 * UI.identityScale[1]
	
	self.widgets.vlist = UI:CreateWidget("VListWidget", {rect={0,0,8,8}})
	self.widgets.vlist:SetBlendWithParent(true)
	self.widgets.panel:AddChild(self.widgets.vlist)
	
	local checkpoint = Persistence.ReadNumber(Globals, "checkpoint")
	
	local w
	local r 
	
	w, r = self:CreateGameInfoWidget(MainMenu.saves[checkpoint], 0, inset, picSize, options.rect[3])
	self.widgets.vlist:AddItem(w)
	
	local width = r[3]
	local height = r[4]
	
	local numSaves = 1
	local widgets = {}
	
	for k,v in pairs(MainMenu.saves) do
		if (k ~= checkpoint) then
			local w = self:CreateLineBorder(height, options.rect[3])
			self.widgets.vlist:AddChild(w)
			table.insert(widgets, w)
			
			height = height + 7
			
			w, r = self:CreateGameInfoWidget(v, height, inset, picSize, options.rect[3])
			self.widgets.vlist:AddItem(w)
			width = Max(width, r[3])
			height = height + r[4]
		
			table.insert(widgets, w)
			numSaves = numSaves + 1
		end
	end
	
	-- make them all the same size
	for k,v in pairs(widgets) do
	
		r = v:Rect()
		r[3] = width
		v:SetRect(r)
	
	end
	
	local panelRect = {
		0,
		0,
		options.rect[3],
		options.rect[4]
	}
	
	self.widgets.vlist:SetRect(panelRect)
	self.widgets.vlist:SetClipRect(panelRect)
	self.widgets.vlist:SetEndStops({0, panelRect[4]*0.1})
	self.widgets.vlist:RecalcLayout()
	self.widgets.vlist:ScrollTo({0,0}, 0)
	
	return panelRect

end

function MainMenu.LoadGame.CreateLineBorder(self, yOfs, width)
	local w = UI:CreateWidget("MatWidget", {rect={0,yOfs,width,7}, material=MainMenu.gfx.LineBorder4})
	w:SetBlendWithParent(true)
	return w
end

function MainMenu.LoadGame.PrepareContents(self)
	MainMenuPanel.PrepareContents(self)
	self.widgets.vlist:BlendTo({1,1,1,0}, 0)
end

function MainMenu.LoadGame.AnimateContents(self, onComplete)
	self.widgets.vlist:BlendTo({1,1,1,1}, 0.2)
	if (onComplete) then
		local f = function()
			onComplete()
		end
		World.globalTimers:Add(f, 0.2, true)
	end
end

function MainMenu.LoadGame.FadeOutContents(self, time)
	self.widgets.vlist:BlendTo({1,1,1,0}, time)
end

function MainMenu.LoadGame.LoadGame(self, saveInfo)
	Persistence.WriteBool(Session, "loadCheckpoint", true)
	Session:Save()
	
	Persistence.WriteNumber(Globals, "checkpoint", saveInfo.index)
	Globals:Save()
	
	SaveGame:LoadSavedGame(saveInfo.path)
	
	MainMenu.command = function()
		World.RequestLoad(saveInfo.level, kUnloadDisposition_Slot)
	end
	
	MainMenu.mainPanel:Exit()
end

function MainMenu.LoadGame.OnSaveGameInputEvent(self, panel, saveInfo, e)

	if (Input.IsTouchBegin(e)) then
		panel:SetCapture(true)
		panel:SetMaterial(MainMenu.gfx.MMItemBackground2)
	elseif (Input.IsTouchEnd(e)) then
		panel:SetCapture(false)
		panel:SetMaterial(MainMenu.gfx.MMItemBackground)
		if (e.type ~= kI_TouchCancelled) then
			UI.sfx.Command:Play(kSoundChannel_UI, 0)
			self:LoadGame(saveInfo)
		end
	end

	return true
	
end

function MainMenu.LoadGame.CreateGameInfoWidget(self, saveInfo, yOfs, inset, picSize, panelWidth)

	local panel = UI:CreateWidget("MatWidget", {rect={0,yOfs,panelWidth,8}, material=MainMenu.gfx.MMItemBackground})
	panel:SetBlendWithParent(true)
	
	local OnInputEvent = function(widget, e)
		return self:OnSaveGameInputEvent(panel, saveInfo, e)
	end
	
	panel.OnInputEvent = OnInputEvent
	
	local labels = {}
	
	local w = UI:CreateWidget("MatWidget", {rect={inset,inset, picSize, picSize}, material=MainMenu.gfx.Portraits[saveInfo.portrait]})
	w:SetBlendWithParent(true)	
	panel:AddChild(w)
	
	local yMargin = inset + picSize + (8 * UI.identityScale[1])
	
	local typeface = UI.typefaces.StandardButtonSmall
	local fontAdvance = UI:FontAdvanceSize(typeface) + (4*UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	UI:SetLabelText(w, saveInfo.playerName)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	
	local height = fontAdvance
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, height+inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	local text = StringTable.Get(saveInfo.level)
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	
	height = height + fontAdvance
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, height+inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	local text = StringTable.Get("MM_LAST_PLAYED")
	text = text.." "..saveInfo.lastPlayed
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	
	height = height + fontAdvance
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, height+inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	local text = StringTable.Get("MM_ARM_DATE")
	text = text.." "..saveInfo.armDate
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	
	height = height + fontAdvance

	local centerY = (picSize-height) / 2
	for k,v in pairs(labels) do
	
		local r = v:Rect()
		r[2] = centerY
		v:SetRect(r)
		
		centerY = centerY + fontAdvance
	
	end
	
	height = Max(height, picSize)
	
	local r = panel:Rect()
	r[4] = height+inset*2
	panel:SetRect(r)

	return panel, r
end
