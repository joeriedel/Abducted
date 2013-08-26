-- Discovery.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Discovery = Entity:New()
Discovery.Active = LL_New()
Discovery.Popup = nil
Discovery.Widgets = {}
-- 720p sizes
Discovery.kUISize = {300, 400}
Discovery.kArrowSize = {20, 32}
Discovery.kButtonHeight = 90
Discovery.kUIBorder = 8
-- Percent of total screen
Discovery.TouchDistance = 0.1

function Discovery.Spawn(self)

	self.databaseId = self.keys.database_id
	self.left = StringForString(self.keys.fold_out, "left") == "left"
		
	local spriteSize = {80, 80}--Vec2ForString(self.keys.sprite_size, {64, 64})
	local absSize = math.max(spriteSize[1], spriteSize[2])
	
	self:SetMins({-absSize/2, -absSize/2, -absSize/2})
	self:SetMaxs({absSize/2, absSize/2, absSize/2})
	
	self.sprite = World.CreateSpriteBatch(1, 1)
	self.sprite.material = World.Load(self.keys.material)
	self.sprite.dm = self:AttachDrawModel(self.sprite, self.sprite.material)
	self.sprite.dm:SetBounds(self:Mins(), self:Maxs())
	self.sprite.sprite = self.sprite.dm:AllocateSprite()
	self.sprite.dm:SetSpriteData(
		self.sprite.sprite,
		{
			pos = {0, 0, 0}, -- relative to drawmodel
			size = spriteSize,
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.sprite.dm:BlendTo({0,0,0,0}, 0)
	
	local radius = NumberForString(self.keys.activation_radius, 300)
	self.pos = self:WorldPos()
	self.mins = VecSub(self.pos, {radius, radius, radius})
	self.maxs = VecAdd(self.pos, {radius, radius, radius})
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()

	local enabled = StringForString(self.keys.enabled, "true") == "true"
	
	self.active = false
	
	if (enabled) then
		self:Enable()
	else
		self.enabled = enabled
	end
end

function Discovery.Enable(self, enable)
	if (enable == nil) then
		enable = true
	end

	if (self.enabled == enable) then
		return
	end
	
	self.enabled = enable
	self.think = Discovery.CheckPlayerProximity
	self:SetNextThink(1/4)

end

function Discovery.Activate(self, activate)

	if (self.active == activate) then
		return
	end
	
	self.active = activate
	
	if (activate) then
		self.activeItem = LL_Append(Discovery.Active, self.activeItem or {x=self})
		self.sprite.dm:BlendTo({1,1,1,1}, 0.2)
	else
		LL_Remove(Discovery.Active, self.activeItem)
		self.sprite.dm:BlendTo({1,1,1,0}, 0.2)
		if (Discovery.Popup == self) then
			Discovery.Popup:CloseUI()
		end
	end

end

function Discovery.CheckPlayerProximity(self)

	local ents = World.BBoxTouching(self.mins, self.maxs, kEntityClass_Player)
	if (ents) then
		self:Activate(true)
	else
		self:Activate(false)
	end

end

function Discovery.LayoutUI(self)

	local title = nil
	local text = nil
	
	local dbItem = Arm.Discoveries[self.databaseId]
	if (dbItem) then
		local discovered = GameDB:CheckDiscovery(self.databaseId)
		if (dbItem.mysteryTitle) then
			if (discovered ~= "unlocked") then
				-- use mystery title
				title = dbItem.mysteryTitle
				text = dbItem.mysteryText
			end
		end
		
		if (title == nil) then
			-- use regular text
			title = dbItem.title
			text = dbItem.discoveryPopupText or dbItem.text
		end
	end
	
	if (title == nil) then
		title = "bad arm-db item"
		text = "bad arm-db item"
	end
	
	local kBorderSize = Discovery.kUIBorder * UI.identityScale[1]
	local kDialogWidth = Discovery.kUISize[1] - (kBorderSize * 2)
	local kButtonHeight = Discovery.kButtonHeight * UI.identityScale[2]
	
	title = StringTable.Get(title)
	
	local r = UI:LineWrapCenterText(
		Discovery.Widgets.Title,
		kDialogWidth,
		true,
		0,
		title
	)
	
	r[2] = Discovery.kUIBorder * UI.identityScale[1]
	Discovery.Widgets.Title:SetRect(r)
	
	r[2] = r[2] + r[4] + 10*UI.identityScale[1]
	
	Discovery.Widgets.Line1:SetRect({0,r[2],Discovery.kUISize[1],10*UI.identityScale[1]})
	
	r[2] = r[2] + 10*UI.identityScale[1]
	
	local textArea = Discovery.kUISize[2] - r[2] - kButtonHeight - 10*UI.identityScale[1]
	local textRect = {kBorderSize, r[2], kDialogWidth, textArea}
	
	Discovery.Widgets.Scroll:SetRect(textRect)
	Discovery.Widgets.Scroll:SetEndStops({0, textRect[4]*0.1})
	
	if (UI.mode == kGameUIMode_PC) then
		textRect[3] = textRect[3] - 24
	end
	
	Discovery.Widgets.Scroll:SetClipRect(textRect)
	Discovery.Widgets.Scroll:ScrollTo({0,0}, 0)
	
	text = StringTable.Get(text)
	
	UI:LineWrapCenterLJustifyText(
		Discovery.Widgets.Text,
		textRect[3],
		true,
		0,
		text
	)
	
	Discovery.Widgets.Scroll:RecalcLayout()
	
	r[2] = textRect[2] + textRect[4]
	Discovery.Widgets.Line2:SetRect({0,r[2],Discovery.kUISize[1],10*UI.identityScale[1]})
	
	r[2] = r[2] + 10*UI.identityScale[1]
	
	Discovery.Widgets.Button:SetRect({kBorderSize, r[2], kDialogWidth, kButtonHeight})
	Discovery.Widgets.Button.label:SetRect({0,0,kDialogWidth,kButtonHeight})
	Discovery.Widgets.Button.highlight:SetRect({0,0,kDialogWidth,kButtonHeight})
	
	UI:LineWrapCenterText(
		Discovery.Widgets.Button.label,
		nil,
		nil,
		0,
		StringTable.Get("DISCOVERY_OPEN_DB")
	)
		
end

function Discovery.AnimateOpenUI(self)

	if (self.animateTimer) then	
		self.animateTimer:Clean()
		self.animateTimer = nil
	end
	
	self.busy = true
	self:UpdateUI() -- get position
	self.updateTimer = World.globalTimers:Add(function () self:UpdateUI() end, 0, true)

	Discovery.Widgets.Arrow:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Title:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Text:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Scroll:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Line1:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Line2:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Button:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Root:ScaleTo({0,0}, {0,0})
	Discovery.Widgets.Root:SetVisible(true)
	
	local arrowRect = Discovery.Widgets.Arrow:Rect()
	if (self.left) then
		Discovery.Widgets.Arrow:RotateTo({arrowRect[3]/2, arrowRect[4]/2, 0}, {0,0,0})
	else
		Discovery.Widgets.Arrow:RotateTo({arrowRect[3]/2, arrowRect[4]/2, 180}, {0,0,0})
	end
	
	local f = function()
	
		local f = function()
			
			Discovery.Widgets.Arrow:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Title:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Text:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Scroll:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Line1:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Line2:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Button:BlendTo({1,1,1,1}, 0.3)
			Abducted.entity.eatInput = false
			self.animateTimer = nil
			self.busy = false
			
		end
		
		Discovery.Widgets.Root:ScaleTo({1, 1}, {0, 0.15})
		self.animateTimer = World.globalTimers:Add(f, 0.15)
	end

	Discovery.Widgets.Root:ScaleTo({1, 0.1}, {0.15, 0})
	self.animateTimer = World.globalTimers:Add(f, 0.15)

end

function Discovery.AnimateCloseUI(self, callback)

	self.busy = true
	
	if (self.animateTimer) then	
		self.animateTimer:Clean()
		self.animateTimer = nil
	end
	
	local f = function()
	
		local f = function()
			Discovery.Widgets.Root:ScaleTo({0.1, 0}, {0, 0.15})
			
			if (callback) then
				local f = function()
					self.animateTimer = nil
					callback()
				end
				self.animateTimer = World.globalTimers:Add(f, 0.1)
			else
				self.animateTimer = nil
			end
			
			if (self.updateTimer) then
				self.updateTimer:Clean()
				self.updateTimer = nil
			end
			
			self.busy = false
			Discovery.Popup = nil
		end
		
		Discovery.Widgets.Root:ScaleTo({0.1, 1}, {0.15, 0})
		self.animateTimer = World.globalTimers:Add(f, 0.15)
	end
		
	Discovery.Widgets.Arrow:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Title:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Text:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Scroll:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Line1:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Line2:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Button:BlendTo({1,1,1,0}, 0.3)
	self.animateTimer = World.globalTimers:Add(f, 0.3)

end

function Discovery.OpenUI(self)
	Abducted.entity.eatInput = true
	local f = function()
		Discovery.Popup = self
		local f = function()
			GameDB:Discover(self.databaseId, false, true)
			self:LayoutUI()
			self:AnimateOpenUI()
		end
		TerminalScreen.CancelUI(f)
	end
	if (Discovery.Popup) then
		if (Discovery.Popup == self) then
			return
		end
		Discovery.Popup:CloseUI(f)
	else
		f()
	end
end

function Discovery.CloseUI(self, callback)
	self:AnimateCloseUI(callback)
end

function Discovery.UpdateUI(self)

	local panelRect = Discovery.Widgets.Root:Rect()
	local arrowRect = Discovery.Widgets.Arrow:Rect()
	
	local p,r = World.Project(self.pos)
	p = UI:MapToUI(p)
	
	if (self.left) then
		arrowRect[1] = p[1] - arrowRect[3]
		panelRect[1] = arrowRect[1] - panelRect[3] + 4
	else
		arrowRect[1] = p[1]
		panelRect[1] = arrowRect[1] + arrowRect[3] - 4
	end
	
	arrowRect[2] = p[2] - arrowRect[4]/2
	panelRect[2] = p[2] - panelRect[4]/2
	
	Discovery.Widgets.Arrow:SetRect(arrowRect)
	BoundRect(panelRect, TerminalScreen.ScreenBounds)
	Discovery.Widgets.Root:SetRect(panelRect)

end

function Discovery.CheckTouched(self, e)
	local p, r = World.Project(self.pos)
	p = UI:MapToUI(p)
	
	local dx = p[1]-e.data[1]
	local dy = p[2]-e.data[2]
	local dd = math.sqrt(dx*dx+dy*dy)
	if (dd <= UI.screenDiagonal*Discovery.TouchDistance) then
		self:OpenUI()
		return true
	end
	
	return false
end

function Discovery.CheckTouch(e)
	local x = LL_Head(Discovery.Active)
	
	while (x) do
		if (x.x:CheckTouched(e)) then
			return true
		end
		x = LL_Next(x)
	end
	
	return false
end

function Discovery.OpenDBPressed()
	COutLine(kC_Debug, "Discovery OpenDB - Pressed")
	if (Game.entity.manipulate or Game.entity.pulse) then
		return
	end
	Abducted.entity.eatInput = true
	local self = Discovery.Popup
	self:CloseUI(function () World.playerPawn:EnterArm("db", self.databaseId) end)
end

function Discovery.StaticInit()

	local material = World.Load("UI/discovery_arrow_M")
	Discovery.Widgets.Arrow = UI:CreateWidget("MatWidget", {rect={0,0,Discovery.kArrowSize[1]*UI.identityScale[1],Discovery.kArrowSize[2]*UI.identityScale[2]}, material=material})
	UI.widgets.discoveries.Root:AddChild(Discovery.Widgets.Arrow)
	Discovery.Widgets.Arrow:BlendTo({1,1,1,0}, 0)
	
	material = World.Load("UI/DiscoveryBackground_M")
	Discovery.Widgets.Root = UI:CreateWidget("MatWidget", {rect={0,0,Discovery.kUISize[1], Discovery.kUISize[2]}, material=material, OnInputEvent=UI.EatInput})
	UI.widgets.discoveries.Root:AddChild(Discovery.Widgets.Root)
	Discovery.Widgets.Root:SetVisible(false)
	Discovery.Widgets.Root:SetHAlign(kHorizontalAlign_Center)
	Discovery.Widgets.Root:SetVAlign(kVerticalAlign_Center)
	
	local titleTF = World.Load("UI/DiscoveryTitle_TF")
	Discovery.Widgets.Title = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=titleTF})
	Discovery.Widgets.Root:AddChild(Discovery.Widgets.Title)
	
	Discovery.Widgets.Scroll = UI:CreateWidget("VListWidget", {rect={0,0,8,8}})
	Discovery.Widgets.Root:AddChild(Discovery.Widgets.Scroll)
	
	if (UI.mode == kGameUIMode_PC) then
        UI:CreateVListWidgetScrollBar(
            Discovery.Widgets.Scroll,
	        24,
	        24,
			8
       )
    end
		
	local textTF = World.Load("UI/DiscoveryText_TF")
	Discovery.Widgets.Text = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=textTF})
	Discovery.Widgets.Scroll:AddItem(Discovery.Widgets.Text)
	Discovery.Widgets.Text:SetBlendWithParent(true)
		
	Discovery.Widgets.Line1 = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=Arm.gfx.SkillsHorzLine1})
	Discovery.Widgets.Line2 = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=Arm.gfx.SkillsHorzLine1})
	
	Discovery.Widgets.Root:AddChild(Discovery.Widgets.Line1)
	Discovery.Widgets.Root:AddChild(Discovery.Widgets.Line2)
	
	Discovery.Widgets.Button = UI:CreateStylePushButton(
		{0,0,8,8}, 
		Discovery.OpenDBPressed, 
		{fontSize="small", highlight={on={0,0,0,0}}, solidBackground = true}, 
		Discovery.Widgets.Root
	)
end

function Discovery.ResetUIForCheckpoint()
	Discovery.Widgets.Root:SetVisible(false)
	Discovery.Widgets.Arrow:BlendTo({1,1,1,0}, 0)
	if (Discovery.Popup) then
		if (Discovery.Popup.updateTimer) then
			Discovery.Popup.updateTimer:Clean()
			Discovery.Popup.updateTimer = nil
		end
		if (Discovery.Popup.animateTimer) then
			Discovery.Popup.animateTimer:Clean()
			Discovery.Popup.animateTimer = nil
		end
		Discovery.Popup = nil
	end
	Discovery.Active = LL_New()
end

info_discovery = Discovery
