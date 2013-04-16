-- MainMenuLoadGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu.LoadGame = MainMenuPanel:New()

function MainMenu.InitLoadGame(self)

	self.loadGamePanel = MainMenu.LoadGame:New()
	
	self.loadGamePanel:Create({rect=self.contentRect}, self.widgets.root)
	self.loadGamePanel:Layout()
	UI:CenterWidget(self.loadGamePanel.widgets.panel, self.contentRect)
	self.loadGamePanel.widgets.panel:SetHAlign(kHorizontalAlign_Left)
	self.loadGamePanel.widgets.panel:SetVAlign(kVerticalAlign_Bottom)
	self.loadGamePanel.widgets.panel:SetVisible(false)
	
end

function MainMenu.LoadGame.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
	
	self.widgets.controls = {}
	
	if (MainMenu.saves == nil) then
		return options
	end

	self.widgets.vlist = UI:CreateWidget("VListWidget", {rect={0,0,8,8}})
	self.widgets.vlist:SetBlendWithParent(true)
	self.widgets.panel:AddChild(self.widgets.vlist)
	
end

function MainMenu.LoadGame.Layout(self)

	for k,v in pairs(self.widgets.controls) do
		v:Unmap()
	end
	
	self.widgets.controls = {}
	
	if (MainMenu.saves == nil) then
		return
	end
	
	self.widgets.vlist:Clear()
	
	local parentRect = self.widgets.panel:Rect()

	local inset = 8 * UI.identityScale[1]
	local picSize = 160 * UI.identityScale[1]
	
	local w
	local r 
	local width = 0
	local height = 0
	
	local checkpoint = Persistence.ReadNumber(Globals, "checkpoint")
	local first = true
	
	if (checkpoint) then
		w, r = self:CreateGameInfoWidget(MainMenu.saves[checkpoint], 0, inset, picSize, parentRect[3])
		self.widgets.vlist:AddItem(w)
		table.insert(self.widgets.controls, w)
		
		width = r[3]
		height = r[4]
	
		first = false
	end
		
	local numSaves = 1
	local widgets = {}
	
	for k,v in pairs(MainMenu.saves) do
		if (k ~= checkpoint) then
			local w
			
			if (not first) then
				w,r = self:CreateLineBorder(height, parentRect[3])
				self.widgets.vlist:AddChild(w)
				table.insert(widgets, w)
				table.insert(self.widgets.controls, w)
				height = height + 7
			end
			
			w, r = self:CreateGameInfoWidget(v, height, inset, picSize, parentRect[3])
			self.widgets.vlist:AddItem(w)
			width = Max(width, r[3])
			height = height + r[4]
		
			table.insert(widgets, w)
			table.insert(self.widgets.controls, w)
			numSaves = numSaves + 1
			first = false
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
		parentRect[3],
		parentRect[4]
	}
	
	self.widgets.vlist:SetRect(panelRect)
	self.widgets.vlist:SetClipRect(panelRect)
	self.widgets.vlist:SetEndStops({0, panelRect[4]*0.1})
	self.widgets.vlist:RecalcLayout()
	self.widgets.vlist:ScrollTo({0,0}, 0)

	collectgarbage()
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

function MainMenu.LoadGame.DeleteGame(self, saveInfo)

	local f = function(result)
		if (result == AlertPanel.YesButton) then
			local checkpoint = Persistence.ReadNumber(Globals, "checkpoint")
			if (saveInfo.index == checkpoint) then
				Persistence.DeleteKey(Globals, "checkpoint")
			end
			Globals.keys.saveGames[tostring(saveInfo.index)] = nil
			Globals:Save()
			MainMenu.saves = nil
			MainMenu:PopulateSaveGames()
			self:Layout()
		end
	end
	
	AlertPanel:YesNo("MM_CONFIRM_DELETE_TITLE", "MM_CONFIRM_DELETE_PROMPT", f, MainMenu.contentRect)
	
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
	w:Unmap()
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
	w:Unmap()
	
	local height = fontAdvance
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, height+inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	local text = StringTable.Get(saveInfo.level)
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	w:Unmap()
	
	height = height + fontAdvance
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, height+inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	text = StringTable.Get("MM_LAST_PLAYED")
	text = text.." "..saveInfo.lastPlayed
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	w:Unmap()
	
	height = height + fontAdvance
	
	w = UI:CreateWidget("TextLabel", {rect={yMargin, height+inset, 8, 8}, typeface=typeface})
	w:SetBlendWithParent(true)
	panel:AddChild(w)
	text = StringTable.Get("MM_ARM_DATE")
	text = text.." "..saveInfo.armDate
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	table.insert(labels, w)
	w:Unmap()
	
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
	
	-- add a "delete" button.
	w = UI:CreateStylePushButton({0, 0, 8, 8}, function () self:DeleteGame(saveInfo) end, { highlight={on={0,0,0,0}} }, nil)
	text = StringTable.Get("MM_DELETE")
	UI:SetLabelText(w.label, text)

	local labelRect = UI:SizeLabelToContents(w.label)
	local buttonPadd = {32 * UI.identityScale[1], 8 * UI.identityScale[2]}
	
	local buttonRect = {
		r[3] - labelRect[3] - buttonPadd[1],
		r[4] - labelRect[4] - buttonPadd[2],
		labelRect[3] + buttonPadd[1],
		labelRect[4] + buttonPadd[2]
	}
	
	w:SetRect(buttonRect)
	UI:CenterLabel(w.label, {0,0,buttonRect[3],buttonRect[4]})
	w.highlight:SetRect({0,0,buttonRect[3],buttonRect[4]})
	w:SetBlendWithParent(true)
	
	local black = UI:CreateWidget("MatWidget", {rect=buttonRect, material=UI.gfx.Solid})
	black:BlendTo({0,0,0,1}, 0)
	black:SetBlendWithParent(true)
	panel:AddChild(black)
	black:Unmap()
	
	panel:AddChild(w)
	table.insert(self.widgets.controls, w)
	
	return panel, r
end
