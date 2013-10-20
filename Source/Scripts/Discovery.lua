-- Discovery.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Discovery = Entity:New()
Discovery.Active = LL_New()
Discovery.Popup = nil
Discovery.Widgets = {}
Discovery.kSkillReward = 25
-- 720p sizes
Discovery.kUISize = {300, 500}
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
	self.sprite.dm:Skin()
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
	
	local io = {
		Save = function()
			return self:SaveState()
		end,
		Load = function(s, x)
			return self:LoadState(x)
		end
	}
	
	GameDB.PersistentObjects[self.keys.uuid] = io
end

function Discovery.Enable(self, enable)
	if (enable == nil) then
		enable = true
	end

	if (self.enabled == enable) then
		return
	end
	
	self.enabled = enable
	
	if (enable) then
		self.think = Discovery.CheckPlayerProximity
		self:SetNextThink(1/4)
	else
		self.think = nil
		self:Activate(false)
	end
	
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

function Discovery.LayoutUI(self, awardSkillPoints)

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
	local kDialogWidth = (Discovery.kUISize[1]*UI.identityScale[1]) - (kBorderSize * 2)
	local kButtonHeight = Discovery.kButtonHeight * UI.identityScale[2]
	
	title = StringTable.Get(title)
	
	local r = UI:LineWrapCenterText(
		Discovery.Widgets.Title,
		kDialogWidth,
		true,
		0,
		title
	)
	
	r[1] = r[1] + Discovery.kUIBorder * UI.identityScale[1]
	r[2] = Discovery.kUIBorder * UI.identityScale[1]
	Discovery.Widgets.Title:SetRect(r)
	
	r[2] = r[2] + r[4]
	
	if (awardSkillPoints) then
		local rect = {kBorderSize, r[2]+8*UI.identityScale[1], kDialogWidth, Discovery.Widgets.SkillReward.Height}
		Discovery.Widgets.SkillReward:SetVisible(true)
		Discovery.Widgets.SkillReward:BlendTo({1,1,1,0}, 0)
		Discovery.Widgets.SkillReward:SetRect(rect)
		r[2] = r[2] + 8*UI.identityScale[1]
		
		local text = "+"..tostring(Discovery.kSkillReward)
		if (Discovery.kSkillReward > 1) then
			text = text.." "..StringTable.Get("ARM_REWARD_SKILLPOINTS")
		else
			text = text.." "..StringTable.Get("ARM_REWARD_SKILLPOINT")
		end
		
		text = text.."\n"..StringTable.Get("ARM_REWARD_DISCOVERY")
		
		local textRect = UI:LineWrapCenterText(
			Discovery.Widgets.SkillReward,
			kDialogWidth,
			true,
			0,
			text
		)
		
		r[2] = r[2] + textRect[4]
		Discovery.Widgets.SkillReward.Height = textRect[4]
		
		rect = UI:HCenterLabel(Discovery.Widgets.SkillReward, rect)
		Discovery.Widgets.SkillReward.pos = {rect[1], rect[2]}
		Discovery.Widgets.SkillReward:SetClipRect(rect)
	else
		Discovery.Widgets.SkillReward:SetVisible(false)
	end

	r[2] = r[2] + 10*UI.identityScale[1]
	
	Discovery.Widgets.Line1:SetRect({0,r[2],Discovery.kUISize[1]*UI.identityScale[1],10*UI.identityScale[1]})
	
	r[2] = r[2] + 10*UI.identityScale[1]
	
	local textArea = (Discovery.kUISize[2]*UI.identityScale[2]) - r[2] - kButtonHeight - 10*UI.identityScale[1]
	local textRect = {kBorderSize, r[2], kDialogWidth, textArea}
	
	Discovery.Widgets.Scroll:SetRect(textRect)
	Discovery.Widgets.Scroll:SetEndStops({0, textRect[4]*0.1})
	
	if (UI.mode == kGameUIMode_PC) then
		textRect[3] = textRect[3] - 24
	end
	
	Discovery.Widgets.Scroll:SetClipRect(textRect)
	Discovery.Widgets.Scroll:ScrollTo({0,0}, 0)
	
	text = StringTable.Get(text)
	
	local scrollTextRect = UI:LineWrapAlignTopLJustifyText(
		Discovery.Widgets.Text,
		textRect[3]-kBorderSize*4,
		true,
		0,
		text
	)
	
	scrollTextRect[1] = kBorderSize*2
	Discovery.Widgets.Text:SetRect(scrollTextRect)
	
	Discovery.Widgets.Scroll:RecalcLayout()
	
	r[2] = textRect[2] + textRect[4]
	Discovery.Widgets.Line2:SetRect({0,r[2],Discovery.kUISize[1]*UI.identityScale[1],10*UI.identityScale[1]})
	
	r[2] = r[2] + 10*UI.identityScale[1]
	
	Discovery.Widgets.Button:SetRect({kBorderSize, r[2], kDialogWidth, kButtonHeight})
	Discovery.Widgets.Button.label:SetRect({0,0,kDialogWidth,kButtonHeight})
	Discovery.Widgets.Button.highlight:SetRect({0,0,kDialogWidth,kButtonHeight})
	
	UI:LineWrapCenterText(
		Discovery.Widgets.Button.label,
		kDialogWidth*0.9,
		nil,
		0,
		StringTable.Get("DISCOVERY_OPEN_DB")
	)
		
end

function Discovery.AnimateOpenUI(self, awardSkillPoints)

	if (self.animateTimer) then	
		self.animateTimer:Clean()
		self.animateTimer = nil
	end
	
	self.busy = true
	self:UpdateUI() -- get position
	self.updateTimer = World.globalTimers:Add(function () self:UpdateUI() end, 0, true)

	Discovery.Widgets.Arrow:BlendTo({1,1,1,0}, 0)
	Discovery.Widgets.Close:BlendTo({1,1,1,0}, 0)
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
			Discovery.Widgets.Close:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Title:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Text:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Scroll:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Line1:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Line2:BlendTo({1,1,1,1}, 0.3)
			Discovery.Widgets.Button:BlendTo({1,1,1,1}, 0.3)
			Abducted.entity.eatInput = false
			self.busy = false
			
			if (awardSkillPoints) then
			
				local f = function()
				
					Arm.sfx.Reward:Play(kSoundChannel_UI, 0)
					local w = Discovery.Widgets.SkillReward
					w:MoveTo({w.pos[1], w.pos[2]+w.Height}, {0,0})
					w:MoveTo({w.pos[1], w.pos[2]}, {0,0.3})
					w:BlendTo({1,1,1,1}, 0.3)
					self.animateTime = nil
					
				end
			
				self.animateTimer = World.globalTimers:Add(f, 0.4)
			else
				self.animateTimer = nil
			end
			
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
	Discovery.Widgets.Close:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Title:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Text:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Scroll:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Line1:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Line2:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.Button:BlendTo({1,1,1,0}, 0.3)
	Discovery.Widgets.SkillReward:BlendTo({1,1,1,0}, 0.3)
	self.animateTimer = World.globalTimers:Add(f, 0.3)

end

function Discovery.OpenUI(self)
	Abducted.entity.eatInput = true
	
	if (self.keys.on_activated) then
		World.PostEvent(self.keys.on_activated)
	end
	
	local f = function()
		Discovery.Popup = self
		local awardSkillPoints = GameDB:Discover(self.databaseId, "world", false, true)
		self:LayoutUI(awardSkillPoints)
		self:AnimateOpenUI(awardSkillPoints)
		self:AddLookTarget()
	end
	if (Discovery.Popup) then
		if (Discovery.Popup == self) then
			Abducted.entity.eatInput = false
			return
		end
		Discovery.Popup:CloseUI(f)
	else
		f()
	end
end

function Discovery.CloseUI(self, callback)
	local f = function()
		if (BoolForString(self.keys.disable_on_close, false)) then
			self:Enable(false)
		end
		if (self.keys.on_closed) then
			World.PostEvent(self.keys.on_closed)
		end
		if (callback) then
			callback()
		end
	end
	self:AnimateCloseUI(f)
	self:RemoveLookTarget()
end

function Discovery.AddLookTarget(self)

	assert(self.lookTargetId == nil)
	self.lookTargetId = World.viewController:BlendToLookTarget(
		self.pos,
		0.8,
		0.8,
		-1,
		0.9,
		1,
		1,
		0.57
	)
	
	World.playerPawn:LookAt(self.pos)

end

function Discovery.RemoveLookTarget(self)
	if (self.lookTargetId) then
		World.viewController:FadeOutLookTarget(self.lookTargetId, 0.2)
		self.lookTargetId = nil
	end
end

function Discovery.UpdateUI(self)

	local panelRect = Discovery.Widgets.Root:Rect()
	local arrowRect = Discovery.Widgets.Arrow:Rect()
	
	local p,r = World.Project(self.pos)
	p = UI:MapToUI(p)
	
	if (self.left) then
		arrowRect[1] = p[1] - arrowRect[3]
		panelRect[1] = arrowRect[1] - panelRect[3]
	else
		arrowRect[1] = p[1]
		panelRect[1] = arrowRect[1] + arrowRect[3]
	end
	
	arrowRect[2] = p[2] - arrowRect[4]/2
	panelRect[2] = p[2] - panelRect[4]/2
	
	Discovery.Widgets.Arrow:SetRect(arrowRect)
	BoundRect(panelRect, TerminalScreen.ScreenBounds)
	Discovery.Widgets.Root:SetRect(panelRect)

end

function Discovery.CheckTouched(self, e)
	local p, r = World.Project(self.pos)
	if (not r) then
		return false
	end
	
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

function Discovery.ClosePressed()
	Discovery.Popup:CloseUI()
end

function Discovery.StaticInit()

	local material = World.Load("UI/discovery_arrow_M")
	Discovery.Widgets.Arrow = UI:CreateWidget("MatWidget", {rect={0,0,Discovery.kArrowSize[1]*UI.identityScale[1],Discovery.kArrowSize[2]*UI.identityScale[2]}, material=material})
	UI.widgets.discoveries.Root:AddChild(Discovery.Widgets.Arrow)
	Discovery.Widgets.Arrow:BlendTo({1,1,1,0}, 0)
	
	material = World.Load("UI/DiscoveryBackground_M")
	Discovery.Widgets.Root = UI:CreateWidget("MatWidget", {rect={0,0,Discovery.kUISize[1]*UI.identityScale[1], Discovery.kUISize[2]*UI.identityScale[2]}, material=material, OnInputEvent=UI.EatInput})
	UI.widgets.discoveries.Root:AddChild(Discovery.Widgets.Root)
	Discovery.Widgets.Root:SetVisible(false)
	Discovery.Widgets.Root:SetHAlign(kHorizontalAlign_Center)
	Discovery.Widgets.Root:SetVAlign(kVerticalAlign_Center)
	
	local closeButtonSize = 64*UI.identityScale[1]
	Discovery.Widgets.Close = UIPushButton:Create(
		{Discovery.kUISize[1]*UI.identityScale[1]-(closeButtonSize/2),-(closeButtonSize/2),closeButtonSize,closeButtonSize}, 
		{
			pressed = World.Load("UI/discovery_close_x_pressed_M"),
			enabled = World.Load("UI/discovery_close_x_M")
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed = Discovery.ClosePressed
		},
		nil,
		Discovery.Widgets.Root
	)
	
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
	
	local textTF = Arm.typefaces.ChatReward
	Discovery.Widgets.SkillReward = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=textTF})
	Discovery.Widgets.SkillReward.Height = UI:FontAdvanceSize(textTF)
	Discovery.Widgets.Root:AddChild(Discovery.Widgets.SkillReward)
	Discovery.Widgets.SkillReward:SetBlendWithParent(true)
			
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
		Discovery.Popup.lookTargetId = nil
		Discovery.Popup = nil
	end
	Discovery.Active = LL_New()
end

function Discovery.SaveState(self)
	
	local state = {
		enabled = tostring(self.enabled)
	}
	
	return state
end

function Discovery.LoadState(self, state)
	
	self.think = nil
	self.activeItem = nil
	self.sprite.dm:BlendTo({1,1,1,0}, 0)
	self.enabled = false
	
	if (state.enabled == "true") then
		self:Enable()
	end
	
end

info_discovery = Discovery
