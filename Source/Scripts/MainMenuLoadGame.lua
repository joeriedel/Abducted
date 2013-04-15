-- MainMenuLoadGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu.LoadGame = MainMenuPanel:New()

function MainMenu.InitLoadGame(self)

	self.loadGamePanel = MainMenu.LoadGame:New()
	
	local r = self.loadGamePanel:Create(nil, self.widgets.root)
	self.loadGamePanel.widgets.panel:SetRect(r)
	UI:CenterWidget(self.loadGamePanel.widgets.panel, self.contentRect)
	self.loadGamePanel.widgets.panel:SetHAlign(kHorizontalAlign_Center)
	self.loadGamePanel.widgets.panel:SetVAlign(kVerticalAlign_Center)
	self.loadGamePanel.widgets.panel:SetVisible(false)
	
end

function MainMenu.LoadGame.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
	
	if (MainMenu.saves == nil) then
		return {0,0,8,8}
	end
	
	local itemWidth = 600 * UI.identityScale[1]
	local inset = 8 * UI.identityScale[1]
	local picSize = 160 * UI.identityScale[1]
	
	self.widgets.vlist = UI:CreateWidget("VListWidget", {rect={0,0,8,8}})
	self.widgets.vlist:SetBlendWithParent(true)
	self.widgets.panel:AddChild(self.widgets.vlist)
	
	local checkpoint = Persistence.ReadNumber(Globals, "checkpoint")
	
	local w
	local r 
	
	w, r = self:CreateGameInfoWidget(MainMenu.saves[checkpoint], 0, inset, picSize, itemWidth)
	self.widgets.vlist:AddItem(w)
	
	local width = r[3]
	local height = r[4]
	
	local numSaves = 1
	local widgets = {}
	
	for k,v in pairs(MainMenu.saves) do
		if (k ~= checkpoint) then
			local w = self:CreateLineBorder(height)
			self.widgets.vlist:AddChild(w)
			table.insert(widgets, w)
			
			height = height + 7
			
			w, r = self:CreateGameInfoWidget(v, height, inset, picSize, itemWidth)
			self.widgets.vlist:AddItem(w)
			width = Max(width, r[3])
			height = height + r[4]
		
			table.insert(widgets, w)
			numSaves = numSaves + 1
		end
	end
	
	local minSlotCount = 3
--	local minHeight = (minSlotCount * (inset*2 + picSize)) + ((minSlotCount-1) * 7)
		
	self.widgets.label = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=MainMenu.typefaces.Gold})
	self.widgets.label:SetBlendWithParent(true)
	local text = StringTable.Get("MM_LOAD_GAME")
	UI:SetLabelText(self.widgets.label, text)
	r = UI:SizeLabelToContents(self.widgets.label)
	
	local headerInset = 12 * UI.identityScale[2]
	local headerSize = headerInset*2+r[4]
	
	UI:CenterWidget(self.widgets.label, {0, 0, width, headerSize})
	self.widgets.panel:AddChild(self.widgets.label)
	
	local maxHeight = (UI.screenHeight*0.8) - headerSize
	
--	height = Clamp(height, minHeight, maxHeight)
	height = Min(height, maxHeight)
	
	self.widgets.border1 = self:CreateLineBorder(headerSize-7)
	self.widgets.panel:AddChild(self.widgets.border1)
	table.insert(widgets, self.widgets.border1)
	
	self.widgets.border2 = self:CreateLineBorder(headerSize+height)
	self.widgets.panel:AddChild(self.widgets.border2)
	table.insert(widgets, self.widgets.border2)
	
	-- make them all the same size
	for k,v in pairs(widgets) do
	
		r = v:Rect()
		r[3] = width
		v:SetRect(r)
	
	end
	
	local panelRect = {
		0,
		headerSize,
		width,
		height
	}
	
	self.widgets.vlist:SetRect(panelRect)
	self.widgets.vlist:SetClipRect(panelRect)
	self.widgets.vlist:SetEndStops({0, panelRect[4]*0.1})
	self.widgets.vlist:RecalcLayout()
	self.widgets.vlist:ScrollTo({0,0}, 0)
	
	panelRect[2] = 0
	panelRect[4] = panelRect[4] + headerSize + 7
	
	return panelRect

end

function MainMenu.LoadGame.CreateLineBorder(self, yOfs)
	local w = UI:CreateWidget("MatWidget", {rect={0,yOfs,8,7}, material=MainMenu.gfx.LineBorder4})
	w:SetBlendWithParent(true)
	return w
end

function MainMenu.LoadGame.PrepareContents(self)
	MainMenuPanel.PrepareContents(self)
	self.widgets.vlist:BlendTo({1,1,1,0}, 0)
	self.widgets.label:BlendTo({1,1,1,0}, 0)
	self.widgets.border1:BlendTo({1,1,1,0}, 0)
	self.widgets.border2:BlendTo({1,1,1,0}, 0)
end

function MainMenu.LoadGame.AnimateContents(self, onComplete)
	self.widgets.vlist:BlendTo({1,1,1,1}, 0.2)
	self.widgets.label:BlendTo({1,1,1,1}, 0.2)
	self.widgets.border1:BlendTo({1,1,1,1}, 0.2)
	self.widgets.border2:BlendTo({1,1,1,1}, 0.2)
	if (onComplete) then
		local f = function()
			onComplete()
		end
		World.globalTimers:Add(f, 0.2, true)
	end
end

function MainMenu.LoadGame.FadeOutContents(self, time)
	self.widgets.vlist:BlendTo({1,1,1,0}, time)
	self.widgets.label:BlendTo({1,1,1,0}, time)
	self.widgets.border1:BlendTo({1,1,1,0}, time)
	self.widgets.border2:BlendTo({1,1,1,0}, time)
end

function MainMenu.LoadGame.CreateGameInfoWidget(self, saveInfo, yOfs, inset, picSize, panelWidth)

	local panel = UI:CreateWidget("MatWidget", {rect={0,yOfs,panelWidth,8}, material=MainMenu.gfx.MMItemBackground})
	panel:SetBlendWithParent(true)
	
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
