-- MainMenu.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu = Game:New()

function MainMenu.Initialize(self)

	MainMenu:Load()
	MainMenu:InitUI()

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end
	}

	World.PlayCinematic("mainmenu", bit.bor(kCinematicFlag_Loop, kCinematicFlag_AnimateCamera), 0, Game.entity, callbacks)
	
	self.think = Game.Think
	self:SetNextThink(0)
end

function MainMenu.Load(self)

	self.gfx = {}
	self.gfx.MMPanel = World.Load("UI/MMPanel_M")
	self.gfx.MMSelPanel = World.Load("UI/MMSelPanel_M")
	
	self.typefaces = {}
	self.typefaces.Large = World.Load("UI/MMLarge_TF")
	self.typefaces.Normal = World.Load("UI/MMNormal_TF")

end

function MainMenu.InitUI(self)

	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_MainMenu)
	
	local r = {
		0,
		16 * UI.identityScale[1],
		32,
		32
	}
	
	self.widgets.mainPanel = MainMenu.MainPanel:New()
	self.widgets.mainPanel:Create({rect=r}, self.widgets.root)
	self.widgets.mainPanel:Layout()
	
end

--[[---------------------------------------------------------------------------
	Main Menu Panel
-----------------------------------------------------------------------------]]

MainMenu.MainPanel = MainMenuPanel:New()

function MainMenu.MainPanel.CreateMMText(self, data, item)

	local text = StringTable.Get(data.string)
	
	local OnInputEvent = function (widget, e)
	
		if (Input.IsTouchBegin(e)) then
			widget:SetCapture(true)
			self:SelectItem(item)
		elseif (Input.IsTouchEnd(e)) then
			widget:SetCapture(false)
			data.Action(self, item)
		end
		
	end
	
	local w = UI:CreateWidget("Label", {rect={0, 0, 8, 8}, typeface=MainMenu.typefaces.Normal, OnInputEvent=OnInputEvent})
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	
	return w

end

function MainMenu.MainPanel.NewGame(self, item)
	
end

MainMenu.Items = {
	{data={string="MM_NEW_GAME", Action=MainMenu.MainPanel.NewGame}, Create=MainMenu.MainPanel.CreateMMText}
}

function MainMenu.MainPanel.Create(self, options)

	options = MainMenuPanel.Create(self, options)
	
	self.items = {}
	
	for k,v in pairs(MainMenu.Items) do
	
		local item = {i = k}
		local w = v.Create(self, v.data, item)
		self.widgets.panel:AddChild(w)
		item.w = w
		self.items[k] = item
	end

end

function MainMenu.MainPanel.Layout(self)

	MainMenuPanel.Layout(self)
	
	local xInset = 16 * UI.identityScale[1]
	local yStart = 32 * UI.identityScale[2]
	local vSpace = 24 * UI.identityScale[2]
	local maxWidth = 0
	
	for k,v in pairs(self.items) do
	
		local r = v.w:Rect()
		r[1] = xInset
		r[2] = yStart
		v:SetRect(r)
		
		maxWidth = Max(maxWidth, r[3])
		
		yStart = yStart + r[4] + vSpace
	end
	
	local r = self.widgets.panel:Rect()
	r[3] = maxWidth + (2*xInset)
	self.widgets.panel:SetRect(r)

	return r
	
end

function MainMenu.MainPanel.PrepareContents(self)

	MainMenuPanel.PrepareContents(self)
	
	for k,v in pairs(self.items) do
		v.w:BlendTo({1,1,1,0}, 0)
	end

end

function MainMenu.MainPanel.AnimateNextItem(self, index, onComplete)

	local x = self.items[index]
	if (x == nil) then
		if (onComplete) then
			onComplete()
		end
		return
	end

	x:BlendTo({1,1,1,1}, 0.3)
		
	local f = function()
		self:AnimateNext(index+1, onComplete)
	end

	World.globalTimers:Add(f, 0.3, true)
	
end

function MainMenu.MainPanel.AnimateContents(self, onComplete)
	self:AnimateNextItem(1, onComplete)
end

function MainMenu.MainPanel.SelectItem(self, item)

end

function MainMenu.MainPanel.OnInputEvent(self, e)
	if (self.busy) then
		return true
	end
	
	return MainMenuPanel.OnInputEvent(self, e)
end
