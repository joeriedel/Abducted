-- HUD.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

HUD = Class:New()

function HUD.Spawn(self)
	
	HUD:Load()
	HUD:Layout()
	HUD.enabled = true
	HUD.visible = true
	
	World.globalTimers:Add(
		function () HUD:Think() end,
		0,
		true
	)
end

function HUD.SetVisible(self, visible)
	self.visible = visible
	self.widgets.Root:SetVisible(visible)
end

function HUD.Load(self)
	self.widgets = {}
	self.widgets.Root = UI:CreateRoot(UI.kLayer_HUD)
	
	if (UI.mode == kGameUIMode_Mobile) then
		self.gfx = {
			Arm = "UI/arm_button_M",
			ArmPressed = "UI/arm_button_pressed_M",
			ArmSignaled = "UI/arm_button_signaled_M",
			ManipulateDisabled = "UI/manipulate_button_charging_M",
			ManipulateEnabled = "UI/manipulate_button_M",
			ManipulateFlashing = "UI/manipulate_button_flashing_M",
			RechargeShimmer = "UI/ability_recharge_shimmer_M",
			PulseDisabled = "UI/pulse_button_charging_M",
			PulseEnabled = "UI/pulse_button_M",
			PulseFlashing = "UI/pulse_button_flashing_M",
			ShieldDisabled = "UI/shield_button_charging_M",
			ShieldEnabled = "UI/shield_button_M"
		}
	else
		self.gfx = {
			Arm = "UI/arm_button_pc_M",
			ArmPressed = "UI/arm_button_pressed_pc_M",
			ArmSignaled = "UI/arm_button_signaled_pc_M",
			ManipulateDisabled = "UI/manipulate_button_charging_pc_M",
			ManipulateEnabled = "UI/manipulate_button_pc_M",
			ManipulateFlashing = "UI/manipulate_button_flashing_pc_M",
			RechargeShimmer = "UI/ability_recharge_shimmer_M",
			PulseDisabled = "UI/pulse_button_charging_pc_M",
			PulseEnabled = "UI/pulse_button_pc_M",
			PulseFlashing = "UI/pulse_button_flashing_pc_M",
			ShieldDisabled = "UI/shield_button_charging_pc_M",
			ShieldEnabled = "UI/shield_button_pc_M",
			ActionBar = "UI/action_bar_pc_M"
		}
	end
	map(self.gfx, World.Load)
	
	self.sfx = {}
	
	self.sfx.Signaled = World.Load("Audio/armsignaled")
	self.sfx.AbilityUnlocked = World.Load("Audio/ability_unlocked")
	
	if (UI.mode == kGameUIMode_PC) then
		self.ActionBarSize = {720*UI.identityScale[1], 75*UI.identityScale[1]} -- maintain aspect
		self.ActionBarRect = {(UI.screenWidth-self.ActionBarSize[1])/2, UI.screenHeight-self.ActionBarSize[2],self.ActionBarSize[1], self.ActionBarSize[2]}
		
		self.widgets.ActionBar = UI:CreateWidget(
		"MatWidget", 
		{
			rect=self.ActionBarRect, 
			material=self.gfx.ActionBar
		}
		)
		
		self.widgets.Root:AddChild(self.widgets.ActionBar)
		self.widgets.ActionBar:SetVisible(false)
		self.actionBarVisible = false
	end
	
	self.widgets.Arm = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Arm, {0, 0}),
		{pressed = self.gfx.ArmPressed, enabled = self.gfx.Arm},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:ArmPressed() end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.Manipulate = UIPushButton:Create(
		UI:MaterialSize(self.gfx.ManipulateEnabled, {0, 0}),
		{ -- we go to disabled state when manipulate gets activated
			enabled = self.gfx.ManipulateEnabled,
			disabled = self.gfx.ManipulateDisabled,
			pressed = self.gfx.ManipulateDisabled
		},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:ManipulatePressed() end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.ManipulateCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.ManipulateEnabled})
	self.widgets.Root:AddChild(self.widgets.ManipulateCharging)
	self.widgets.ManipulateCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.ManipulateCharging:SetVisible(false)
	
	self.widgets.ManipulateShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.ManipulateShimmer:SetVisible(false)
	
	self.widgets.Manipulate.flashing = false
	
	self.widgets.Shield = UIPushButton:Create(
		UI:MaterialSize(self.gfx.ShieldEnabled, {0, 0}),
		{ -- we go to disabled state when shield gets activated
			enabled = self.gfx.ShieldEnabled,
			disabled = self.gfx.ShieldDisabled,
			pressed = self.gfx.ShieldDisabled
		},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:ShieldPressed() end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.ShieldCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.ShieldEnabled})
	self.widgets.Root:AddChild(self.widgets.ShieldCharging)
	self.widgets.ShieldCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.ShieldCharging:SetVisible(false)
	
	self.widgets.ShieldShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.ShieldShimmer:SetVisible(false)
	
	self.widgets.Pulse = UIPushButton:Create(
		UI:MaterialSize(self.gfx.PulseEnabled, {0, 0}),
		{ -- we go to disabled state when pulse is fired
			enabled = self.gfx.PulseEnabled,
			disabled = self.gfx.PulseDisabled,
			pressed = self.gfx.PulseDisabled
		},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:PulsePressed() end},
		nil,
		self.widgets.Root
	)

--[[
    self.widgets.MemoryGame = UIPushButton:Create(
        UI:MaterialSize(self.gfx.PulseEnabled, {0, 0}),
        { -- we go to disabled state when pulse is fired
            enabled = self.gfx.PulseEnabled,
            disabled = self.gfx.PulseDisabled,
            pressed = self.gfx.PulseDisabled
        },
        nil,
        {pressed=function (widget) HUD:MemoryGamePressed(widget) end},
        nil,
        self.widgets.Root
    )
]]--

	self.widgets.PulseCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.PulseEnabled})
	self.widgets.Root:AddChild(self.widgets.PulseCharging)
	self.widgets.PulseCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.PulseCharging:SetVisible(false)
	
	self.widgets.PulseShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.PulseShimmer:SetVisible(false)
	
	self.widgets.Pulse.flashing = false
	
	self.visible = true
	self.armEnabled = false
	self.manipulateEnabled = false
	self.shieldEnabled = false
	self.pulseEnabled = false
	
	self.widgets.Arm.class:SetEnabled(self.widgets.Arm, false)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, false)
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, false)
	
	if (UI.mode == kGameUIMode_PC) then
		local r = { self.ActionBarRect[1]+393*UI.identityScale[1], self.ActionBarRect[2]+20*UI.identityScale[1], 113*UI.identityScale[1], 50*UI.identityScale[1]}
		self.widgets.Arm:SetRect(r)
		self.widgets.Arm:BlendTo({1,1,1,0}, 0)
		self.widgets.Manipulate:BlendTo({1,1,1,0}, 0)
		self.widgets.Shield:BlendTo({1,1,1,0}, 0)
		self.widgets.Pulse:BlendTo({1,1,1,0}, 0)
		
		self.widgets.ManipulateLabelBkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=UI.gfx.KeyLabelBackground})
		self.widgets.Root:AddChild(self.widgets.ManipulateLabelBkg)
		self.widgets.ManipulateLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
		self.widgets.Root:AddChild(self.widgets.ManipulateLabel)
		
		self.widgets.ShieldLabelBkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material==UI.gfx.KeyLabelBackground})
		self.widgets.Root:AddChild(self.widgets.ShieldLabelBkg)
		self.widgets.ShieldLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
		self.widgets.Root:AddChild(self.widgets.ShieldLabel)
		
		self.widgets.PulseLabelBkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=UI.gfx.KeyLabelBackground})
		self.widgets.Root:AddChild(self.widgets.PulseLabelBkg)
		self.widgets.PulseLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=UI.typefaces.ActionBar})
		self.widgets.Root:AddChild(self.widgets.PulseLabel)
		
		self.widgets.ManipulateLabel:BlendTo({1,1,1,0}, 0)
		self.widgets.ManipulateLabelBkg:BlendTo({1,1,1,0}, 0)
		self.widgets.ShieldLabel:BlendTo({1,1,1,0}, 0)
		self.widgets.ShieldLabelBkg:BlendTo({1,1,1,0}, 0)
		self.widgets.PulseLabel:BlendTo({1,1,1,0}, 0)
		self.widgets.PulseLabelBkg:BlendTo({1,1,1,0}, 0)
	end
	
	
	self.widgets.Root:AddChild(self.widgets.ManipulateShimmer)
	self.widgets.Root:AddChild(self.widgets.ShieldShimmer)
	self.widgets.Root:AddChild(self.widgets.PulseShimmer)
end

function HUD.ArmPressed(self)
	COutLine(kC_Debug, "Arm Pressed!")
	if (Game.entity.manipulate or Game.entity.pulse) then
		return
	end
	World.playerPawn:EnterArm()
end

function HUD.ManipulatePressed(self)
	COutLine(kC_Debug, "Manipulate Pressed!")
	if (Game.entity.pulse) then
		return
	end
	Game.entity:BeginManipulate()
end

function HUD.ShieldPressed(self)
	COutLine(kC_Debug, "Shield Pressed!")
	if (World.playerPawn.shieldActive) then
		HUD:ExpireShield()
		return
	end
	
	HUD:BeginShield()
end

function HUD.PulsePressed(self)
	COutLine(kC_Debug, "Pulse Pressed!")
	if (Game.entity.manipulate) then
		return
	end
	Game.entity:BeginPulse()
end

function HUD.BeginShield(self, gameTime)

	if (gameTime == nil) then
		World.playerPawn:BeginShield()
	end
	
	if (gameTime == nil) then
		gameTime = GameDB.realTime
	end
	
	-- start ticking down the time.
	self.shieldStartTime = gameTime
	
	local gfx = {
		enabled = self.gfx.ShieldDisabled
	}
	
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.ShieldCharging:SetVisible(true)
	self.widgets.ShieldCharging:FillCircleTo(-1, 0)
		
	local f = function ()
		HUD:ExpireShield()
	end
	
	self.shieldExpiryTimer = World.globalTimers:Add(f, PlayerSkills:MaxShieldTime() - (GameDB.realTime - gameTime))
	
	f = function ()
		local dd = GameDB.realTime - HUD.shieldStartTime
		self.widgets.ShieldCharging:FillCircleTo(-(1 - (dd / PlayerSkills:MaxShieldTime())), 0)
	end
	
	self.shieldTimer = World.globalTimers:Add(f, 0, true)
	
end

-- djr, not sure if this is right or will work
function HUD.MemoryGamePressed(self, widget)
    COutLine(kC_Debug, "Reflex Game Pressed!")
    ReflexGame:DebugStart();
--    COutLine(kC_Debug, "Memory Game Pressed!")
--    MemoryGame:DebugStart();
end

function HUD.RechargePulse(self)
	self.pulseRechargeTime = PlayerSkills:PulseRechargeTime()
	self.pulseStart = GameDB.realTime
	HUD:InternalRechargePulse()
end

function HUD.InternalRechargePulse(self)
	-- pulse was fired
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, false)
		
	self.widgets.PulseCharging:SetVisible(true)
	self.widgets.PulseCharging:FillCircleTo(0, 0)
		
	local f = function ()
		self.pulseStart = nil
		if (not HUD.enabled) then
			return
		end
		if (self.pulseTimer) then
			self.pulseTimer:Clean()
			self.pulseTimer = nil
		end
		self.widgets.PulseCharging:SetVisible(false)
		self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, true)
		HUD:Shimmer(self.widgets.PulseShimmer)
	end
	
	World.globalTimers:Add(f, self.pulseRechargeTime - (GameDB.realTime - self.pulseStart))
	
	f = function ()
		local dd = GameDB.realTime - self.pulseStart
		self.widgets.PulseCharging:FillCircleTo(dd / self.pulseRechargeTime, 0)
	end
	
	self.pulseTimer = World.globalTimers:Add(f, 0, true)
end

function HUD.Layout(self)

	self.widgets.Arm:SetVisible(PlayerSkills:ArmUnlocked())
	
	if (UI.mode == kGameUIMode_Mobile) then
		HUD:LayoutMobile()
	else
		HUD:LayoutPC()
	end
end

function HUD.LayoutMobile(self)
	local y = 42
	if (PlayerSkills:ManipulateUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth, y)
		self.widgets.ManipulateCharging:SetRect(r)
		self.widgets.ManipulateShimmer:SetRect(r)
		y = y + r[4] + (24 * UI.identityScale[2])
		self.widgets.Manipulate:SetVisible(true)
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.ManipulateShimmer:SetVisible(false)
	else
		self.widgets.Manipulate:SetVisible(false)
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.ManipulateShimmer:SetVisible(false)
	end
	if (PlayerSkills:ShieldUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth, y)
		self.widgets.ShieldCharging:SetRect(r)
		self.widgets.ShieldShimmer:SetRect(r)
		y = y + r[4] + (24 * UI.identityScale[2])
		self.widgets.Shield:SetVisible(true)
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.ShieldShimmer:SetVisible(false)
	else
		self.widgets.Shield:SetVisible(false)
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.ShieldShimmer:SetVisible(false)
	end
	if (PlayerSkills:PulseUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth, y)
		self.widgets.PulseCharging:SetRect(r)
		self.widgets.PulseShimmer:SetRect(r)
		self.widgets.Pulse:SetVisible(true)
		self.widgets.PulseCharging:SetVisible(false)
		self.widgets.PulseShimmer:SetVisible(false)
	else
		self.widgets.Pulse:SetVisible(false)
		self.widgets.PulseCharging:SetVisible(false)
		self.widgets.PulseShimmer:SetVisible(false)
	end
end

function HUD.LayoutPC(self)
	local r = {0, 0, 48*UI.identityScale[1], 48*UI.identityScale[1]}
	
	r[1] = self.ActionBarRect[1]+219*UI.identityScale[1]
	r[2] = self.ActionBarRect[2]+20*UI.identityScale[1]
	self.widgets.Manipulate:SetRect(r)
	self.widgets.ManipulateCharging:SetRect(r)
	self.widgets.ManipulateShimmer:SetRect(ExpandRect(r, 32*UI.identityScale[1], 32*UI.identityScale[1]))
	
	UI:SetLabelText(self.widgets.ManipulateLabel, PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_Manipulate]))
	local unused, d = UI:RAlignLabel(
		self.widgets.ManipulateLabel, 
		self.ActionBarRect[1]+266*UI.identityScale[1],
		self.ActionBarRect[2]+20*UI.identityScale[1]
	)
	self.widgets.ManipulateLabelBkg:SetRect({ 
		self.ActionBarRect[1]+266*UI.identityScale[1]-(d[3]-d[1])-2,
		self.ActionBarRect[2]+20*UI.identityScale[1]-2,
		d[3]-d[1]+4,
		d[4]-d[2]+4
	})
	
	if (PlayerSkills:ManipulateUnlocked()) then
		self.widgets.Manipulate:SetVisible(true)
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.ManipulateShimmer:SetVisible(false)
		self.widgets.ManipulateLabel:SetVisible(true)
		self.widgets.ManipulateLabelBkg:SetVisible(true)
	else
		self.widgets.Manipulate:SetVisible(false)
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.ManipulateShimmer:SetVisible(false)
		self.widgets.ManipulateLabel:SetVisible(false)
		self.widgets.ManipulateLabelBkg:SetVisible(false)
	end
	
	r[1] = self.ActionBarRect[1]+277*UI.identityScale[1]
	r[2] = self.ActionBarRect[2]+20*UI.identityScale[1]
	self.widgets.Shield:SetRect(r)
	self.widgets.ShieldCharging:SetRect(r)
	self.widgets.ShieldShimmer:SetRect(ExpandRect(r, 32*UI.identityScale[1], 32*UI.identityScale[1]))
	
	UI:SetLabelText(self.widgets.ShieldLabel, PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_Shield]))
	unused, d = UI:RAlignLabel(
		self.widgets.ShieldLabel, 
		self.ActionBarRect[1]+324*UI.identityScale[1],
		self.ActionBarRect[2]+20*UI.identityScale[1]
	)
	self.widgets.ShieldLabelBkg:SetRect({ 
		self.ActionBarRect[1]+324*UI.identityScale[1]-(d[3]-d[1])-2,
		self.ActionBarRect[2]+20*UI.identityScale[1]-2,
		d[3]-d[1]+4,
		d[4]-d[2]+4
	})
	
	if (PlayerSkills:ShieldUnlocked()) then
		self.widgets.Shield:SetVisible(true)
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.ShieldShimmer:SetVisible(false)
		self.widgets.ShieldLabel:SetVisible(true)
		self.widgets.ShieldLabelBkg:SetVisible(true)
	else
		self.widgets.Shield:SetVisible(false)
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.ShieldShimmer:SetVisible(false)
		self.widgets.ShieldLabel:SetVisible(false)
		self.widgets.ShieldLabel:SetVisible(false)
	end

	r[1] = self.ActionBarRect[1]+335*UI.identityScale[1]
	r[2] = self.ActionBarRect[2]+20*UI.identityScale[1]
	self.widgets.Pulse:SetRect(r)
	self.widgets.PulseCharging:SetRect(r)
	self.widgets.PulseShimmer:SetRect(ExpandRect(r, 32*UI.identityScale[1], 32*UI.identityScale[1]))
	
	UI:SetLabelText(self.widgets.PulseLabel, PhysicalKeyName(Abducted.KeyBindings.ActionToKey[kAction_Pulse]))
	unused, d = UI:RAlignLabel(
		self.widgets.PulseLabel, 
		self.ActionBarRect[1]+382*UI.identityScale[1],
		self.ActionBarRect[2]+20*UI.identityScale[1]
	)
	self.widgets.PulseLabelBkg:SetRect({ 
		self.ActionBarRect[1]+382*UI.identityScale[1]-(d[3]-d[1])-2,
		self.ActionBarRect[2]+20*UI.identityScale[1]-2,
		d[3]-d[1]+4,
		d[4]-d[2]+4
	})
	
	if (PlayerSkills:PulseUnlocked()) then
		self.widgets.Pulse:SetVisible(true)
		self.widgets.PulseCharging:SetVisible(false)
		self.widgets.PulseShimmer:SetVisible(false)
		self.widgets.PulseLabel:SetVisible(true)
		self.widgets.PulseLabelBkg:SetVisible(true)
	else
		self.widgets.Pulse:SetVisible(false)
		self.widgets.PulseCharging:SetVisible(false)
		self.widgets.PulseShimmer:SetVisible(false)
		self.widgets.PulseLabel:SetVisible(false)
		self.widgets.PulseLabelBkg:SetVisible(false)
	end
	
end

function HUD.AnimateUnlock(self, items)

	self.sfx.AbilityUnlocked:Play(kSoundChannel_UI, 0)
	
	if (UI.mode == kGameUIMode_Mobile) then
		HUD:AnimateUnlockMobile(items)
	else
		HUD:AnimateUnlockPC(items)
	end

end

function HUD.AnimateUnlockMobile(self, items)
	if (PlayerSkills:ArmUnlocked()) then
		if (items.arm) then
			local r = self.widgets.Arm:Rect()
			self.widgets.Arm:SetRect({r[1]-r[3], r[2], r[3], r[4]})
			self.widgets.Arm:MoveTo({r[1], r[2]}, {0.3, 0})
			self.armEnabled = true
			self.widgets.Arm.class:SetEnabled(self.widgets.Arm, true)
			self.widgets.Arm:SetVisible(true)
		end
	end

	local y = 42
	if (PlayerSkills:ManipulateUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth, y)
		self.widgets.ManipulateCharging:SetRect(r)
		self.widgets.ManipulateShimmer:SetRect(r)
		y = y + r[4] + (24 * UI.identityScale[2])
		self.widgets.Manipulate:SetVisible(true)
		if (items.manipulate) then
			self.widgets.Manipulate:ScaleTo({0,0}, {0,0})
			self.widgets.Manipulate:ScaleTo({1,1}, {0.3, 0.3})
			if (self.manipulateEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.ManipulateShimmer)
				end
				World.gameTimers:Add(f, 0.3)
			end
		end
	end
	
	if (PlayerSkills:ShieldUnlocked()) then
		if (items.shield) then
			local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth, y)
			self.widgets.ShieldCharging:SetRect(r)
			self.widgets.ShieldShimmer:SetRect(r)
			y = y + r[4] + (24 * UI.identityScale[2])
			self.widgets.Shield:ScaleTo({0,0}, {0,0})
			self.widgets.Shield:ScaleTo({1,1}, {0.3, 0.3})
			self.widgets.Shield:SetVisible(true)
			if (self.shieldEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.ShieldShimmer)
				end
				World.gameTimers:Add(f, 0.3)
			end
		else
			local oldRect = self.widgets.Shield:Rect()
			local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth, y)
			self.widgets.Shield:SetRect(oldRect)
			self.widgets.Shield:MoveTo({r[1], r[2]}, {0.3, 0.3})
			self.widgets.ShieldCharging:MoveTo({r[1], r[2]}, {0.3, 0.3})
			self.widgets.ShieldShimmer:MoveTo({r[1], r[2]}, {0.3, 0.3})
			y = y + r[4] + (24 * UI.identityScale[2])
		end
	end
	
	if (PlayerSkills:PulseUnlocked()) then
		if (items.pulse) then
			local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth, y)
			self.widgets.PulseCharging:SetRect(r)
			self.widgets.PulseShimmer:SetRect(r)
			self.widgets.Pulse:ScaleTo({0,0}, {0,0})
			self.widgets.Pulse:ScaleTo({1,1}, {0.3, 0.3})
			self.widgets.Pulse:SetVisible(true)
			if (self.pulseEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.PulseShimmer)
				end
				World.gameTimers:Add(f, 0.3)
			end
		else
			local oldRect = self.widgets.Pulse:Rect()
			local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth, y)
			self.widgets.Pulse:SetRect(oldRect)
			self.widgets.Pulse:MoveTo({r[1], r[2]}, {0.3, 0.3})
			self.widgets.PulseCharging:MoveTo({r[1], r[2]}, {0.3, 0.3})
			self.widgets.PulseShimmer:MoveTo({r[1], r[2]}, {0.3, 0.3})
		end
	end
end

function HUD.AnimateUnlockPC(self, items)

	local actionBarVisible = self.actionBarVisible
	
	if (PlayerSkills:ArmUnlocked()) then
		if (items.arm) then
			self.armEnabled = true
			self.widgets.Arm.class:SetEnabled(self.widgets.Arm, true)
			self.widgets.Arm:SetVisible(true)
			self:AnimatePCActionBarItems({self.widgets.Arm})
			actionBarVisible = true
		end
	end

	if (PlayerSkills:ManipulateUnlocked()) then
		self.widgets.Manipulate:SetVisible(true)
		self.widgets.ManipulateLabel:SetVisible(true)
		self.widgets.ManipulateLabelBkg:SetVisible(true)
		if (items.manipulate) then
			self:AnimatePCActionBarItems({
				self.widgets.Manipulate, 
				self.widgets.ManipulateLabel, 
				self.widgets.ManipulateLabelBkg
			})
			actionBarVisible = true
			if (self.manipulateEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.ManipulateShimmer)
				end
				World.gameTimers:Add(f, 0.3)
			end
		end
	end
	
	if (PlayerSkills:ShieldUnlocked()) then
		self.widgets.Shield:SetVisible(true)
		self.widgets.ShieldLabel:SetVisible(true)
		self.widgets.ShieldLabelBkg:SetVisible(true)
		if (items.shield) then
			self:AnimatePCActionBarItems({
				self.widgets.Shield, 
				self.widgets.ShieldLabel, 
				self.widgets.ShieldLabelBkg
			})
			actionBarVisible = true
			if (self.shieldEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.ShieldShimmer)
				end
				World.gameTimers:Add(f, 0.3)
			end
		end
	end
	
	if (PlayerSkills:PulseUnlocked()) then
		self.widgets.Pulse:SetVisible(true)
		self.widgets.PulseLabel:SetVisible(true)
		self.widgets.PulseLabelBkg:SetVisible(true)
		if (items.pulse) then
			self:AnimatePCActionBarItems({
				self.widgets.Pulse, 
				self.widgets.PulseLabel, 
				self.widgets.PulseLabelBkg
			})
			actionBarVisible = true
			if (self.pulseEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.PulseShimmer)
				end
				World.gameTimers:Add(f, 0.3)
			end
		end
	end
	
	if ((self.actionBarVisible ~= actionBarVisible) and actionBarVisible) then
		self.widgets.ActionBar:SetVisible(true)
		self:AnimatePCActionBarItems({self.widgets.ActionBar})
		self.actionBarVisible = true
	end
end

function HUD.AnimatePCActionBarItems(self, widgets)

	if (self.actionBarVisible) then
		for k,v in pairs(widgets) do
			v:BlendTo({1,1,1,1}, 0.3)
		end
		return
	end

	-- widget is translated off bottom of the screen?
	for k,v in pairs(widgets) do
		local r = v:Rect()
		local x = {r[1], r[2], r[3], r[4]}
		x[2] = x[2] + self.ActionBarRect[4]
		v:SetRect(x)
		v:MoveTo({r[1], r[2]}, {0, 0.3})
		v:BlendTo({1,1,1,1}, 0)
	end
	
end

function HUD.Think(self)
	if (HUD.enabled) then
		self:UpdateManipulateButton()
		self:UpdatePulseButton()
	end
end

function HUD.UpdateManipulateButton(self)
	local flashing = Game.entity.manipulate
	if (flashing ~= self.widgets.Manipulate.flashing) then
		self.widgets.Manipulate.flashing = flashing
		local gfx = {}
		if (flashing) then
			gfx.enabled = self.gfx.ManipulateFlashing
		else
			gfx.enabled = self.gfx.ManipulateEnabled
			gfx.disabled = self.gfx.ManipulateDisabled
			gfx.pressed = self.gfx.ManipulateDisabled
		end
		
		self.widgets.Manipulate.class:ChangeGfx(self.widgets.Manipulate, gfx)
	end
end

function HUD.UpdatePulseButton(self)
	local flashing = Game.entity.pulse
	if (flashing ~= self.widgets.Pulse.flashing) then
		self.widgets.Pulse.flashing = flashing
		local gfx = {}
		if (flashing) then
			gfx.enabled = self.gfx.PulseFlashing
		else
			gfx.enabled = self.gfx.PulseEnabled
			gfx.disabled = self.gfx.PulseDisabled
			gfx.pressed = self.gfx.PulseDisabled
		end
		
		self.widgets.Pulse.class:ChangeGfx(self.widgets.Pulse, gfx)
	end
end

function HUD.RechargeManipulate(self)

	self.manipulateRechargeTime = PlayerSkills:ManipulateRechargeTime()
	self.manipulateStart = GameDB.realTime
	
	HUD:InternalRechargeManipulate()
	
end

function HUD.InternalRechargeManipulate(self)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	
	self.widgets.ManipulateCharging:SetVisible(true)
	self.widgets.ManipulateCharging:FillCircleTo(0, 0)
		
	local f = function ()
		self.manipulateStart = nil
		if (not HUD.enabled) then
			return
		end
		if (self.manipulateTimer) then
			self.manipulateTimer:Clean()
			self.manipulateTimer = nil
		end
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, true)
		HUD:Shimmer(self.widgets.ManipulateShimmer)
	end
	
	World.globalTimers:Add(f, self.manipulateRechargeTime - (GameDB.realTime - self.manipulateStart))
	
	f = function ()
		local dd = GameDB.realTime - self.manipulateStart
		self.widgets.ManipulateCharging:FillCircleTo(dd / self.manipulateRechargeTime, 0)
	end
	
	self.manipulateTimer = World.globalTimers:Add(f, 0, true)
end

function HUD.ExpireShield(self)

	if (self.shieldTimer) then
		self.shieldTimer:Clean()
		self.shieldTimer = nil
	end
	
	if (self.shieldExpiryTimer) then
		self.shieldExpiryTimer:Clean()
		self.shieldExpiryTimer = nil
	end

	local timeUsed = Min(GameDB.realTime - self.shieldStartTime, PlayerSkills:MaxShieldTime())
		
	self.shieldRechargeTime = PlayerSkills:ShieldRechargeTime(timeUsed)
	self.shieldStartTime = GameDB.realTime
	self.shieldRechargeFrac = (PlayerSkills:MaxShieldTime() - timeUsed) / PlayerSkills:MaxShieldTime()
	
	HUD:RechargeShield()
	
	World.playerPawn:EndShield()
	
end

function HUD.RechargeShield(self, instant)

	if (instant) then
		if (World.playerPawn.shieldActive) then
			World.playerPawn:EndShield()
		end
		
		if (self.shieldTimer) then
			self.shieldTimer:Clean()
			self.shieldTimer = nil
		end
		
		if (self.shieldExpiryTimer) then
			self.shieldExpiryTimer:Clean()
			self.shieldExpiryTimer = nil
		end
		
		self.shieldStartTime = nil
		
		if (not HUD.enabled) then
			return
		end
		
		self.widgets.ShieldCharging:SetVisible(false)
				
		local gfx = {}
		gfx.enabled = self.gfx.ShieldEnabled
		gfx.disabled = self.gfx.ShieldDisabled
		gfx.pressed = self.gfx.ShieldDisabled
		
		self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
		self.widgets.Shield.class:SetEnabled(self.widgets.Shield, self.shieldEnabled)
		return
	end

	local gfx = {
		enabled = self.gfx.ShieldEnabled,
		disabled = self.gfx.ShieldDisabled
	}
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, false)
	self.widgets.ShieldCharging:SetVisible(true)
	
	local f = function ()
		self.shieldStartTime = nil
	
		if (self.shieldTimer) then
			self.shieldTimer:Clean()
			self.shieldTimer = nil
		end
		
		if (self.shieldExpiryTimer) then
			self.shieldExpiryTimer:Clean()
			self.shieldExpiryTimer = nil
		end
		
		if (not HUD.enabled) then
			return
		end
		
		self.widgets.ShieldCharging:SetVisible(false)
				
		local gfx = {}
		gfx.enabled = self.gfx.ShieldEnabled
		gfx.disabled = self.gfx.ShieldDisabled
		gfx.pressed = self.gfx.ShieldDisabled
		
		self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
		self.widgets.Shield.class:SetEnabled(self.widgets.Shield, self.shieldEnabled)
	
		HUD:Shimmer(self.widgets.ShieldShimmer)
	end
	
	self.shieldExpiryTimer = World.globalTimers:Add(f, self.shieldRechargeTime)
	
	f = function ()
		local dd = GameDB.realTime - self.shieldStartTime
		dd = dd / self.shieldRechargeTime
		dd = self.shieldRechargeFrac + (dd * (1 - self.shieldRechargeFrac))
		self.widgets.ShieldCharging:FillCircleTo(-dd, 0)
	end
	
	self.shieldTimer = World.globalTimers:Add(f, 0, true)
	
end

function HUD.Shimmer(self, widget)
	widget:SetVisible(true)
	widget:BlendTo({0,0,0,0}, 0)
	widget:BlendTo({1,1,1,1}, 0.2)
	
	local f = function()
		widget:BlendTo({0,0,0,0}, 0.2)
	end
	
	World.globalTimers:Add(f, 0.2)
end

function HUD.PlayerDied(self)
	HUD:Disable()
end

function HUD.EnableAll(self)
	HUD:Enable({"arm", "manipulate", "shield", "pulse"})
end

function HUD.Enable(self, items)

	local state = {
		arm = false,
		manipulate = false,
		shield = false,
		pulse = false
	}
	
	for k,v in pairs(items) do
		state[v] = true
	end
	
	HUD:EnableArm(state.arm)
	HUD:EnableManipulate(state.manipulate)
	HUD:EnableShield(state.shield)
	HUD:EnablePulse(state.pulse)

end

function HUD.EnableArm(self, enable)
	if (self.armEnabled == enable) then
		return
	end
	
	self.armEnabled = enable
	self.widgets.Arm.class:SetEnabled(self.widgets.Arm, enable)
end

function HUD.SignalArm(self, signal)
	
	local gfx = {}
	
	if (signal) then
		gfx.pressed = self.gfx.ArmPressed
		gfx.enabled = self.gfx.ArmSignaled
		self.sfx.Signaled:Play(kSoundChannel_UI, 0)
	else
		gfx.pressed = self.gfx.ArmPressed
		gfx.enabled = self.gfx.Arm
	end
	
	self.widgets.Arm.class:ChangeGfx(self.widgets.Arm, gfx)

end

function HUD.EnableManipulate(self, enable)
	if (self.manipulateEnabled == enable) then
		return
	end
	
	self.manipulateEnabled = enable
	
	if (self.manipulateTimer) then
		self.manipulateTimer:Clean()
		self.manipulateTimer = nil
	end
	
	local gfx = {}
	
	gfx.enabled = self.gfx.ManipulateEnabled
	gfx.disabled = self.gfx.ManipulateDisabled
	gfx.pressed = self.gfx.ManipulateDisabled
	
	self.widgets.ManipulateCharging:SetVisible(false)
	self.widgets.Manipulate.class:ChangeGfx(self.widgets.Manipulate, gfx)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, enable)
end

function HUD.EnableShield(self, enable)
	if (self.shieldEnabled == enable) then
		return
	end
	
	self.shieldEnabled = enable
	
	if (self.shieldStartTime and (not World.playerPawn.dead)) then
		if (not enabled) then
			if (World.playerPawn.shieldActive) then
				HUD:ExpireShield()
			end
		end
		return
	end
		
	local gfx = {}
	gfx.enabled = self.gfx.ShieldEnabled
	gfx.disabled = self.gfx.ShieldDisabled
	gfx.pressed = self.gfx.ShieldDisabled
	
	self.widgets.ShieldCharging:SetVisible(false)
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, enable)
end

function HUD.EnablePulse(self, enable)
	if (self.pulseEnabled == enable) then
		return
	end
	
	self.pulseEnabled = enable
	
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
	
	local gfx = {}
	
	gfx.enabled = self.gfx.PulseEnabled
	gfx.disabled = self.gfx.PulseDisabled
	gfx.pressed = self.gfx.PulseDisabled
	
	self.widgets.PulseCharging:SetVisible(false)
	self.widgets.Pulse.class:ChangeGfx(self.widgets.Pulse, gfx)
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, enable)
end

function HUD.Disable(self)
-- disable the HUD
	HUD.enabled = false

	HUD:EnableArm(false)
	HUD:EnableManipulate(false)
	HUD:EnableShield(false)
	HUD:EnablePulse(false)
	
end

function HUD.HandleAction(self, action)
	if (action == kAction_Arm) then
		if (HUD.enabled and HUD.visible and self.armEnabled and self.widgets.Arm.enabled) then
			self:ArmPressed()
			if (self.widgets.Arm.sfx.pressed) then
				self.widgets.Arm.sfx.pressed:Play(kSoundChannel_UI, 0)
			end
		end
		return true
	end
	
	if (action == kAction_Manipulate) then
		if (HUD.enabled and HUD.visible and self.manipulateEnabled and self.widgets.Manipulate.enabled) then
			self:ManipulatePressed()
			if (self.widgets.Manipulate.sfx.pressed) then
				self.widgets.Manipulate.sfx.pressed:Play(kSoundChannel_UI, 0)
			end
		end
		return true
	end
	
	if (action == kAction_Shield) then
		if (HUD.enabled and HUD.visible and self.shieldEnabled and self.widgets.Shield.enabled) then
			self:ShieldPressed()
			if (self.widgets.Shield.sfx.pressed) then
				self.widgets.Shield.sfx.pressed:Play(kSoundChannel_UI, 0)
			end
		end
		return true
	end
	
	if (action == kAction_Pulse) then
		if (HUD.enabled and HUD.visible and self.pulseEnabled and self.widgets.Pulse.enabled) then
			self:PulsePressed()
			if (self.widgets.Pulse.sfx.pressed) then
				self.widgets.Pulse.sfx.pressed:Play(kSoundChannel_UI, 0)
			end
		end
		return true
	end
	
	return false
end

function HUD.SaveManipulateState(self)

	Persistence.WriteBool(SaveGame, "HUDManipulateEnabled", self.manipulateEnabled)
	
	if (self.manipulateStart) then
		Persistence.WriteString(SaveGame, "HUDManipulateState", "charging")
		Persistence.WriteNumber(SaveGame, "HUDManipulateTime", self.manipulateStart)
		Persistence.WriteNumber(SaveGame, "HUDManipulateRechargeTime", self.manipulateRechargeTime)
	else
		Persistence.DeleteKey(SaveGame, "HUDManipulateState")
	end
	
end

function HUD.LoadManipulateState(self)

	self.manipulateEnabled = Persistence.ReadBool(SaveGame, "HUDManipulateEnabled", false)
	
	local gfx = {}
	gfx.enabled = self.gfx.ManipulateEnabled
	gfx.disabled = self.gfx.ManipulateDisabled
	gfx.pressed = self.gfx.ManipulateDisabled
	
	self.widgets.ManipulateCharging:SetVisible(false)
	self.widgets.Manipulate.class:ChangeGfx(self.widgets.Manipulate, gfx)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, self.manipulateEnabled)
	
	local state = Persistence.ReadString(SaveGame, "HUDManipulateState")
	if (state == "charging") then
		self.manipulateStart = Persistence.ReadNumber(SaveGame, "HUDManipulateTime")
		self.manipulateRechargeTime = Persistence.ReadNumber(SaveGame, "HUDManipulateRechargeTime")
		HUD:InternalRechargeManipulate()
	else
		self.manipulateStart = nil
	end

end

function HUD.SaveShieldState(self)

	Persistence.WriteBool(SaveGame, "HUDShieldEnabled", self.shieldEnabled)

	if (World.playerPawn.shieldActive) then
		Persistence.WriteString(SaveGame, "HUDShieldState", "on")
		Persistence.WriteNumber(SaveGame, "HUDShieldTime", self.shieldStartTime)
	elseif (self.shieldStartTime) then
		Persistence.WriteString(SaveGame, "HUDShieldState", "charging")
		Persistence.WriteNumber(SaveGame, "HUDShieldRechargeTime", self.shieldRechargeTime)
		Persistence.WriteNumber(SaveGame, "HUDShieldTime", self.shieldStartTime)
		Persistence.WriteNumber(SaveGame, "HUDShieldRechargeFrac", self.shieldRechargeFrac)
	else
		Persistence.WriteString(SaveGame, "HUDShieldState", "off")
	end
	
end

function HUD.LoadShieldState(self)

	self.shieldEnabled = Persistence.ReadBool(SaveGame, "HUDShieldEnabled", false)
	
	if (self.shieldTimer) then
		self.shieldTimer:Clean()
		self.shieldTimer = nil
	end
	
	if (self.shieldExpiryTimer) then
		self.shieldExpiryTimer:Clean()
		self.shieldExpiryTimer = nil
	end
	
	local gfx = {}
	gfx.enabled = self.gfx.ShieldEnabled
	gfx.disabled = self.gfx.ShieldDisabled
	gfx.pressed = self.gfx.ShieldDisabled
	
	self.widgets.ShieldCharging:SetVisible(false)
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, self.shieldEnabled)

	local state = Persistence.ReadString(SaveGame, "HUDShieldState", "off")
	
	if (state == "on") then
		local gameTime = Persistence.ReadNumber(SaveGame, "HUDShieldTime")
		HUD:BeginShield(gameTime)
	elseif (state == "charging") then
		self.shieldRechargeTime = Persistence.ReadNumber(SaveGame, "HUDShieldRechargeTime")
		self.shieldStartTime = Persistence.ReadNumber(SaveGame, "HUDShieldTime")
		self.shieldRechargeFrac = Persistence.ReadNumber(SaveGame, "HUDShieldRechargeFrac")
		HUD:RechargeShield()
	end

end

function HUD.SavePulseState(self)

	Persistence.WriteBool(SaveGame, "HUDPulseEnabled", self.pulseEnabled)
	
	if (self.pulseStart) then
		Persistence.WriteString(SaveGame, "HUDPulseState", "charging")
		Persistence.WriteNumber(SaveGame, "HUDPulseTime", self.pulseStart)
		Persistence.WriteNumber(SaveGame, "HUDPulseRechargeTime", self.pulseRechargeTime)
	else
		Persistence.DeleteKey(SaveGame, "HUDPulseState")
	end
	
end

function HUD.LoadPulseState(self)

	self.pulseEnabled = Persistence.ReadBool(SaveGame, "HUDPulseEnabled", false)
	
	local gfx = {}
	gfx.enabled = self.gfx.PulseEnabled
	gfx.disabled = self.gfx.PulseDisabled
	gfx.pressed = self.gfx.PulseDisabled
	
	self.widgets.PulseCharging:SetVisible(false)
	self.widgets.Pulse.class:ChangeGfx(self.widgets.Pulse, gfx)
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, self.pulseEnabled)
	
	local state = Persistence.ReadString(SaveGame, "HUDPulseState")
	if (state == "charging") then
		self.pulseStart = Persistence.ReadNumber(SaveGame, "HUDPulseTime")
		self.pulseRechargeTime = Persistence.ReadNumber(SaveGame, "HUDPulseRechargeTime")
		HUD:InternalRechargePulse()
	else
		self.pulseStart = nil
	end

end

function HUD.SaveState(self)
--	Persistence.WriteBool(SaveGame, "HUDVisible", self.visible)
	HUD:SaveManipulateState()
	HUD:SaveShieldState()
	HUD:SavePulseState()
end

function HUD.LoadState(self)
	HUD.enabled = true
--	HUD.visible = Persistence.ReadBool(SaveGame, "HUDVisible", true)
	HUD:SetVisible(true)
	HUD:Layout()
	HUD:LoadManipulateState()
	HUD:LoadShieldState()
	HUD:LoadPulseState()
end
