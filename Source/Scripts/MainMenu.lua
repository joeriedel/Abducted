-- MainMenu.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu = Game:New()

function MainMenu.Initialize(self)

	MainMenu:PopulateSaveGames()
	MainMenu:Load()
	MainMenu:InitUI()

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end
	}

	MainMenu:PlayCinematic("mainmenu", bit.bor(kCinematicFlag_Loop, kCinematicFlag_AnimateCamera))
	
	self.think = Game.Think
	self:SetNextThink(0)
	
end

function MainMenu.Load(self)

	self.gfx = {}
	self.gfx.MMPanel = World.Load("UI/MMPanel_M")
	self.gfx.MMSelPanel = World.Load("UI/MMSelPanel_M")
	self.gfx.MMFacebook = World.Load("UI/facebook_icon_M")
	self.gfx.MMTwitter = World.Load("UI/twitter_icon_M")
	self.gfx.MMLogo = World.Load("UI/abducted_logo1_M")
	self.gfx.MMSelectedItem = World.Load("UI/MMSelectedItem_M")
	self.gfx.LineBorder4 = World.Load("UI/lineborder4_M")
	self.gfx.PortraitSelectArrow = World.Load("UI/charselectarrow_M")
	self.gfx.MMItemBackground = World.Load("UI/MMItemBackground_M")
	self.gfx.MMItemBackground2 = World.Load("UI/MMItemBackground2_M")
	
	self.gfx.Portraits = {}
	
	for k,v in pairs(GameDB.Portraits) do
		self.gfx.Portraits[k] = World.Load(v)
	end
	
	self.typefaces = {}
	self.typefaces.Large = World.Load("UI/MMLarge_TF")
	self.typefaces.Normal = World.Load("UI/MMNormal_TF")
	self.typefaces.Gold = World.Load("UI/MMGold_TF")
	
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
	
	self.mainPanel = MainMenu.MainPanel:New()
	self.mainPanel:Create({rect=rect}, self.widgets.root)
	
	local firstItemY
	
	rect, firstItemY = self.mainPanel:Layout()
	
	self.contentRect = {
		rect[1] + rect[3] + 16 * UI.identityScale[1],
		16 * UI.identityScale[2],
		0,
		0
	}
	
	self.contentRect[3] = UI.screenWidth - self.contentRect[1] - (16 * UI.identityScale[1])
	self.contentRect[4] = UI.screenHeight - self.contentRect[2] - (16 * UI.identityScale[2])
			
	-- How big can the logo be?
	
	local logoPad = 8*UI.identityScale[1]
	local logoSize = UI.screenWidth - (rect[1]+rect[3]) - (logoPad*2)
	local logoRect = {
		logoPad + rect[1] + rect[3],
		-((700/2048) * logoSize),
		logoSize,
		logoSize
	}
	
	self.widgets.black = UI:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=UI.gfx.Solid})
	self.widgets.black:BlendTo({0,0,0,1}, 0)
	self.widgets.black:SetVisible(false)
	self.widgets.root:AddChild(self.widgets.black)
	
	self.widgets.logo = UI:CreateWidget("MatWidget", {rect=logoRect, material=self.gfx.MMLogo})
	self.widgets.root:AddChild(self.widgets.logo)
	
	-- intro
	self.widgets.logo:BlendTo({0,0,0,0}, 0)
	self.mainPanel:Show(false)
	self.mainPanel.widgets.panel:SetVAlign(kVerticalAlign_Bottom)
	
	local f = function()
		self.widgets.logo:BlendTo({1,1,1,1}, 3)
		local f = function()
			self.mainPanel:TransitionIn({1,0}, 0.4)
		end
		World.globalTimers:Add(f, 4, true)
	end
	
	World.globalTimers:Add(f, 1, true)
	
	self:InitNews()
	self:InitNewGame()
	self:InitLoadGame()
	
end

function MainMenu.PlayCinematic(self, name, flags, xfadeCamera)

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end
	}
	
	if (xfadeCamera == nil) then
		xfadeCamera = 0
	end

	World.PlayCinematic(name, flags, xfadeCamera, Game.entity, callbacks)
	
end

function MainMenu.OnMainMenuCommand(self, cmd, args)

	if (MainMenu.command) then
		MainMenu.command()
		MainMenu.command = nil
	end

end

function MainMenu.ValidCheckpoint(self)
	local checkpoint = Persistence.ReadNumber(Globals, "checkpoint")
	if (checkpoint and self.saves) then
		local saveInfo = self.saves[checkpoint]
		if (saveInfo) then
			return true
		end
	end
	return false
end

function MainMenu.SaveGamesExist(self)
	return self.saves ~= nil
end

function MainMenu.LoadSaveGameInfo(self, path)

	GameDB:Load()
	
	local info = {
		playerName = Persistence.ReadString(SaveGame, "playerName", "???"),
		portrait = Persistence.ReadNumber(SaveGame, "portrait", 1),
		level = Persistence.ReadString(SaveGame, "currentLevel"),
		lastPlayed = Persistence.ReadString(SaveGame, "lastPlayed"),
		armDate = GameDB:ArmDateString(),
		path = path
	}
	
	return info
end

function MainMenu.PopulateSaveGames(self)

	local saveGames = Globals.keys.saveGames
	if (saveGames == nil) then
		return
	end
	
	for k,v in pairs(saveGames) do
		self.saves = self.saves or {}
		SaveGame:LoadSavedGame(v)
		local saveInfo = self:LoadSaveGameInfo(v)
		saveInfo.index = tonumber(k)
		self.saves[saveInfo.index] = saveInfo
	end

end

function MainMenu.InputEventFilter(self, e)
	if (MainMenu.mainPanel and MainMenu.mainPanel.busy) then
		return true
	end
	if (MainMenu.eatInput) then
		return true
	end
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
			if (self.item ~= item) then
				widget.ignore = false
				widget.selecting = true
				local f = function()
					widget.selecting = false
					if (self.busy) then
						local f = function()
							data.Action(self, item)
						end
						if (self.unselectItem) then
							self.unselectItem(f)
							self.unselectItem = nil
						else
							f()
						end
					end					
				end
				self:SelectItem(item, f)
			else
				widget.ignore = true
			end
		elseif (Input.IsTouchEnd(e)) then
			widget:SetCapture(false)
			if (not widget.ignore) then
				self.busy = true
				if (not widget.selecting) then
					local f = function()
						data.Action(self, item)
					end
					
					if (self.unselectItem) then
						self.unselectItem(f)
						self.unselectItem = nil
					else
						f()
					end
				end
			end
		end
		
		return true
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
	local f = function()
		self.busy = false
	end
	
	MainMenu.newsPanel:LayoutNews()
	MainMenu.newsPanel:TransitionIn({0,0}, 0.3, f)

	self.unselectItem = function(callback)
		MainMenu.newsPanel:TransitionOut({0,0}, 0.2, 0.3, callback)
	end
	
end

function MainMenu.MainPanel.Continue(self, item)
	self.busy = false
	
	local f = function(result)
		if (result == AlertPanel.YesButton) then
		
			local checkpoint = Persistence.ReadNumber(Globals, "checkpoint")
			local saveInfo = MainMenu.saves[checkpoint]
			SaveGame:LoadSavedGame(saveInfo.path)
			Persistence.WriteBool(Session, "loadCheckpoint", true)
			Session:Save()
		
			MainMenu.command = function()
				World.RequestLoad(saveInfo.level, kUnloadDisposition_Slot)
			end
			
			MainMenu.mainPanel:Exit()
		end
	end
	
	AlertPanel:YesNo("MM_CONTINUE_GAME_TITLE", "MM_CONTINUE_GAME_PROMPT", f, MainMenu.contentRect)
end

function MainMenu.MainPanel.NewGame(self, item)
	local f = function()
		self.busy = false
	end
	
	MainMenu.newGamePanel:TransitionIn({0,0}, 0.3, f)

	self.unselectItem = function(callback)
		MainMenu.newGamePanel:TransitionOut({0,0}, 0.2, 0.3, callback)
	end
end

function MainMenu.MainPanel.LoadGame(self, item)
	local f = function()
		self.busy = false
	end
	
	MainMenu.loadGamePanel:TransitionIn({0,0}, 0.3, f)

	self.unselectItem = function(callback)
		MainMenu.loadGamePanel:TransitionOut({0,0}, 0.2, 0.3, callback)
	end
end

function MainMenu.MainPanel.Store(self, item)
	self.busy = false
end

function MainMenu.MainPanel.Leaderboards(self, item)
	self.busy = false
end

function MainMenu.MainPanel.Achievements(self, item)
	self.busy = false
end

function MainMenu.MainPanel.Credits(self, item)
	self.busy = false
end

function MainMenu.MainPanel.Facebook(self)
	self.busy = false
end

function MainMenu.MainPanel.Twitter(self)
	self.busy = false
end

function MainMenu.MainPanel.ValidCheckpoint(self)
	return MainMenu:ValidCheckpoint()
end

function MainMenu.MainPanel.SaveGamesExist(self)
	return MainMenu:SaveGamesExist()
end

MainMenu.Items = {
	{data={string="MM_NEWS", Action=MainMenu.MainPanel.News}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_CONTINUE", Action=MainMenu.MainPanel.Continue}, Condition=MainMenu.MainPanel.ValidCheckpoint, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_NEW_GAME", Action=MainMenu.MainPanel.NewGame}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_LOAD_GAME", Action=MainMenu.MainPanel.LoadGame}, Condition=MainMenu.MainPanel.SaveGamesExist, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_STORE", Action=MainMenu.MainPanel.Store}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_LEADERBOARDS", Action=MainMenu.MainPanel.Leaderboards}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_ACHIEVEMENTS", Action=MainMenu.MainPanel.Achievements}, Create=MainMenu.MainPanel.CreateMMText},
	{data={string="MM_CREDITS", Action=MainMenu.MainPanel.Credits}, Create=MainMenu.MainPanel.CreateMMText},
	{Create=MainMenu.MainPanel.CreateMMIcons}
}

function MainMenu.MainPanel.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
	
	self.items = {}
	
	self.widgets.selectionIndicator = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=MainMenu.gfx.MMSelectedItem})
	self.widgets.panel:AddChild(self.widgets.selectionIndicator)
	self.widgets.selectionIndicator:SetVisible(false)
	self.widgets.selectionIndicator:SetHAlign(kHorizontalAlign_Left)
		
	for k,v in pairs(MainMenu.Items) do
	
		if ((v.Condition == nil) or (v.Condition(self))) then
			local item = {i = (#self.items + 1)}
			local w = v.Create(self, v.data, item)
			self.widgets.panel:AddChild(w)
			item.w = w
			self.items[item.i] = item
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
			r[3] = maxWidth
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

	x.w:BlendTo({1,1,1,1}, 0.2)
		
	local f = function()
		self:AnimateNextItem(index+1, onComplete)
	end

	World.globalTimers:Add(f, 0.2, true)
	
end

function MainMenu.MainPanel.AnimateContents(self, onComplete)
	self:AnimateNextItem(1, onComplete)
end

function MainMenu.MainPanel.SelectItem(self, item, onComplete)
	local itemRect = item.w:Rect()
	local panelRect = self.widgets.panel:Rect()
	
	local hPad = 16 * UI.identityScale[1]
	local vPad = 8  * UI.identityScale[2]
	
	local rect = {
		-hPad,
		itemRect[2] - vPad,
		panelRect[3] + (hPad*2),
		itemRect[4] + (vPad*2)
	}
	
	if (self.item) then
		-- move selection bar to the selected item
		
		local itemRect = item.w:Rect()
		local time = 0.12
		
		self.widgets.selectionIndicator:MoveTo({rect[1], rect[2]}, {0, time})
		
		if (onComplete) then
			World.globalTimers:Add(onComplete, time, true)
		end
		
		self.item = item
		
	else
		self.item = item
		
		self.widgets.selectionIndicator:SetVisible(true)
		self.widgets.selectionIndicator:SetRect(rect)
		self.widgets.selectionIndicator:ScaleTo({0,1}, {0,0})
		self.widgets.selectionIndicator:ScaleTo({1,1}, {0.3, 0})
		
		if (onComplete) then
			World.globalTimers:Add(onComplete, 0.3, true)
		end
	end
	
	UI.sfx.Command:Play(kSoundChannel_UI, 0)
end

function MainMenu.MainPanel.Exit(self)
	MainMenu.eatInput = true
	
	local f = function()
		UI:BlendTo({1,1,1,1}, 0.3)
		
		local f = function()
			self.widgets.panel:SetVisible(false)
--			local r = self.widgets.panel:Rect()
--			self.widgets.panel:MoveTo({-r[1]-r[3], 0}, {0.3, 0})
		
		-- center the abducted title
			r = MainMenu.widgets.logo:Rect()
			r[1] = (UI.screenWidth-r[3]) / 2
			MainMenu.widgets.logo:SetRect(r)
			r[2] = (UI.screenHeight-r[4]) / 2
			MainMenu.widgets.logo:MoveTo({r[1], r[2]}, {0, 16})
			MainMenu.widgets.logo:ScaleTo({2, 2}, {16, 16})
	--		MainMenu.widgets.black:SetVisible(true)
--			World.SetDrawUIOnly(true)
			MainMenu:PlayCinematic("flyaway", kCinematicFlag_AnimateCamera, 0.2)
			
			UI:BlendTo({1,1,1,0}, 0.3)
			
			--[[local f = function()
				UI:BlendTo({0,0,0,1}, 0.5)
				World.SoundFadeMasterVolume(0, 0.5)
				if (MainMenu.command) then
					local f = function()
						MainMenu.command()
					end
					World.globalTimers:Add(f, 0.55, true)
				end
			end
			
			World.globalTimers:Add(f, 8, true)]]--
		end
		World.globalTimers:Add(f, 0.3, true)
--		MainMenu.widgets.logo:MoveTo({r[1], r[2]}, {3, 0})

	end
	
	if (MainMenu.mainPanel.unselectItem) then
		MainMenu.mainPanel.unselectItem(f)
		MainMenu.mainPanel.unselectItem = nil
	else
		f()
	end
	
end