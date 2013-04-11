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
	
	-- download news
	self.httpGet = System.NewHTTPGet()
	self.httpGet:SendRequest("www.sunsidegames.com", "/abducted_news_en.txt", "text/plain")
	self.newsPending = true
	
end

function MainMenu.Load(self)

	self.gfx = {}
	self.gfx.MMPanel = World.Load("UI/MMPanel_M")
	self.gfx.MMSelPanel = World.Load("UI/MMSelPanel_M")
	self.gfx.MMFacebook = World.Load("UI/facebook_icon_M")
	self.gfx.MMTwitter = World.Load("UI/twitter_icon_M")
	self.gfx.MMLogo = World.Load("UI/abducted_logo1_M")
	
	self.typefaces = {}
	self.typefaces.Large = World.Load("UI/MMLarge_TF")
	self.typefaces.Normal = World.Load("UI/MMNormal_TF")

end

function MainMenu.InitUI(self)

	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_MainMenu)
	
	local rect = {
		32*UI.identityScale[1],
		0,
		32,
		UI.screenHeight
	}
	
	self.widgets.mainPanel = MainMenu.MainPanel:New()
	self.widgets.mainPanel:Create({rect=rect}, self.widgets.root)
	
	local firstItemY
	
	rect, firstItemY = self.widgets.mainPanel:Layout()
	
	-- How big can the logo be?
	
	local logoPad = 8*UI.identityScale[1]
	local logoSize = UI.screenWidth - (rect[1]+rect[3]) - (logoPad*2)
	local logoRect = {
		logoPad + rect[1] + rect[3],
		-((700/2048) * logoSize),
		logoSize,
		logoSize
	}
	
	self.widgets.logo = UI:CreateWidget("MatWidget", {rect=logoRect, material=self.gfx.MMLogo})
	self.widgets.root:AddChild(self.widgets.logo)
	
	-- intro
	self.widgets.logo:BlendTo({0,0,0,0}, 0)
	self.widgets.mainPanel:Show(false)
	self.widgets.mainPanel.widgets.panel:SetVAlign(kVerticalAlign_Bottom)
	
	local f = function()
		self.widgets.logo:BlendTo({1,1,1,1}, 3)
		local f = function()
			self.widgets.mainPanel:TransitionIn({1,0}, 0.4)
		end
		World.globalTimers:Add(f, 4, true)
	end
	
	World.globalTimers:Add(f, 1, true)
	
end

function MainMenu.ValidCheckpoint(self)
	return true
end

function MainMenu.InputEventFilter(self, e)
	return false
end

function MainMenu.InputGestureFilter(self, g)
	return false
end

function MainMenu.OnInputEvent(self, e)
	return false
end

function MainMenu.OnInputGesture(self, g)
	return false
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
	
	local w = UI:CreateWidget("TextLabel", {rect={0, 0, 8, 8}, typeface=MainMenu.typefaces.Normal, OnInputEvent=OnInputEvent})
	UI:SetLabelText(w, text)
	UI:SizeLabelToContents(w)
	
	return w

end

function MainMenu.MainPanel.CreateMMIcons(self, data, item)

	local OnInputEvent = function (widget, e, action)
		if (Input.IsTouchBegin(e)) then
			widget:SetCapture(true)
		elseif (Input.IsTouchEnd(e)) then
			widget:SetCapture(false)
			action(self)
		end
	end
	
	local iconSize = 112 * UI.identityScale[1]
	local iconHSpace = 24 * UI.identityScale[1]
	local iconVSpace = 8 * UI.identityScale[2]
	local widgetSize = iconSize*2 + iconHSpace
	
	local w = UI:CreateWidget("Widget", {rect={0,0,widgetSize, iconSize+(iconVSpace*2)}})
	
	local fbEvent = function(widget, e)
		OnInputEvent(widget, e, MainMenu.MainPanel.Facebook)
	end
	
	local fb = UI:CreateWidget("MatWidget", {rect={0,iconVSpace,iconSize,iconSize}, material=MainMenu.gfx.MMFacebook, OnInputEvent=fbEvent})
	w:AddChild(fb)
	fb:SetBlendWithParent(true)
	
	local twEvent = function(widget, e)
		OnInputEvent(widget, e, MainMenu.MainPanel.Twitter)
	end
	
	local tw = UI:CreateWidget("MatWidget", {rect={iconSize+iconHSpace,iconVSpace,iconSize,iconSize}, material=MainMenu.gfx.MMTwitter, OnInputEvent=twEvent})
	w:AddChild(tw)
	tw:SetBlendWithParent(true)

	w.center = true
	w.alignBottom = true
	
	return w
end

function MainMenu.MainPanel.News(self, item)
	
end

function MainMenu.MainPanel.Continue(self, item)

end

function MainMenu.MainPanel.NewGame(self, item)
	
end

function MainMenu.MainPanel.LoadGame(self, item)
	
end

function MainMenu.MainPanel.Store(self, item)
	
end

function MainMenu.MainPanel.Leaderboards(self, item)
	
end

function MainMenu.MainPanel.Achievements(self, item)
	
end

function MainMenu.MainPanel.Credits(self, item)
	
end

function MainMenu.MainPanel.Facebook(self)

end

function MainMenu.MainPanel.Twitter(self)

end

function MainMenu.MainPanel.ValidCheckpoint(self)
	return MainMenu:ValidCheckpoint()
end

MainMenu.Items = {
	{data={string="MM_NEWS", Action=MainMenu.MainPanel.News}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_CONTINUE", Action=MainMenu.MainPanel.Continue}, Condition=MainMenu.MainPanel.ValidCheckpoint, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_NEW_GAME", Action=MainMenu.MainPanel.NewGame}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_LOAD_GAME", Action=MainMenu.MainPanel.LoadGame}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_STORE", Action=MainMenu.MainPanel.Store}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_LEADERBOARDS", Action=MainMenu.MainPanel.Leaderboards}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_ACHIEVEMENTS", Action=MainMenu.MainPanel.Achievements}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_CREDITS", Action=MainMenu.MainPanel.Credits}, Create=MainMenu.MainPanel.CreateMMText},
	{Create=MainMenu.MainPanel.CreateMMIcons}
}

function MainMenu.MainPanel.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
	
	self.items = {}
	
	for k,v in pairs(MainMenu.Items) do
	
		if ((v.Condition == nil) or (v.Condition(self))) then
			local item = {i = k}
			local w = v.Create(self, v.data, item)
			self.widgets.panel:AddChild(w)
			item.w = w
			self.items[k] = item
		end
	end

end

function MainMenu.MainPanel.Layout(self)

	MainMenuPanel.Layout(self)
	
	local xInset = 16 * UI.identityScale[1]
	local vSpace = 16 * UI.identityScale[2]
	local maxWidth = 0
	
	local yStart = 0
	local lastY = 0
	
	-- figure out content size
	for k,v in pairs(self.items) do
		local r = v.w:Rect()
		maxWidth = Max(maxWidth, r[3])
		
		if (not v.w.alignBottom) then
			lastY = yStart + r[4]
			yStart = lastY + vSpace
		end
		
	end
	
	local panelRect = self.widgets.panel:Rect()
	panelRect[3] = maxWidth + (xInset*2)
	self.widgets.panel:SetRect(panelRect)
	
	yStart = (panelRect[4]-lastY) / 2
	
	local firstItemY = yStart
		
	for k,v in pairs(self.items) do
	
		local r = v.w:Rect()
		
		if (v.w.center) then
			r[1] = xInset + (maxWidth-r[3]) / 2
		else
			r[1] = xInset
		end
		
		if (v.w.alignBottom) then
			r[2] = panelRect[4] - r[4]
		else
			r[2] = yStart
			lastY = yStart + r[4]
			yStart = lastY + vSpace
		end
		
		v.w:SetRect(r)
	end

	return panelRect, firstItemY
	
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

	x.w:BlendTo({1,1,1,1}, 0.32)
		
	local f = function()
		self:AnimateNextItem(index+1, onComplete)
	end

	World.globalTimers:Add(f, 0.32, true)
	
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
