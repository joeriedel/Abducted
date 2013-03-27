-- HUD.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

HUD = Class:New()

function HUD.Spawn(self)
	
	HUD:Load()
	HUD:Layout()
	HUD.enabled = true
	
	World.globalTimers:Add(
		function () HUD:Think() end,
		0
	)
end

function HUD.SetVisible(self, visible)
	self.widgets.Root:SetVisible(visible)
end

function HUD.Load(self)
	self.widgets = {}
	self.widgets.Root = UI:CreateRoot(UI.kLayer_HUD)
	
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
	
	map(self.gfx, World.Load)
	
	self.sfx = {}
	
	self.sfx.Signaled = World.Load("Audio/armsignaled")
	self.sfx.AbilityUnlocked = World.Load("Audio/ability_unlocked")
	
	self.widgets.Arm = UIPushButton:Create(
		UI:MaterialSize(self.gfx.Arm, {0, 0}),
		{pressed = self.gfx.ArmPressed, enabled = self.gfx.Arm},
		nil,
		{pressed=function (widget) HUD:ArmPressed(widget) end},
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
		nil,
		{pressed=function (widget) HUD:ManipulatePressed(widget) end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.ManipulateCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.ManipulateEnabled})
	self.widgets.Root:AddChild(self.widgets.ManipulateCharging)
	self.widgets.ManipulateCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.ManipulateCharging:SetVisible(false)
	
	self.widgets.ManipulateShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.Root:AddChild(self.widgets.ManipulateShimmer)
	self.widgets.ManipulateShimmer:SetVisible(false)
	
	self.widgets.Manipulate.flashing = false
	
	self.widgets.Shield = UIPushButton:Create(
		UI:MaterialSize(self.gfx.ShieldEnabled, {0, 0}),
		{ -- we go to disabled state when shield gets activated
			enabled = self.gfx.ShieldEnabled,
			disabled = self.gfx.ShieldDisabled,
			pressed = self.gfx.ShieldDisabled
		},
		nil,
		{pressed=function (widget) HUD:ShieldPressed(widget) end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.ShieldCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.ShieldEnabled})
	self.widgets.Root:AddChild(self.widgets.ShieldCharging)
	self.widgets.ShieldCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.ShieldCharging:SetVisible(false)
	
	self.widgets.ShieldShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.Root:AddChild(self.widgets.ShieldShimmer)
	self.widgets.ShieldShimmer:SetVisible(false)
	
	self.widgets.Pulse = UIPushButton:Create(
		UI:MaterialSize(self.gfx.PulseEnabled, {0, 0}),
		{ -- we go to disabled state when pulse is fired
			enabled = self.gfx.PulseEnabled,
			disabled = self.gfx.PulseDisabled,
			pressed = self.gfx.PulseDisabled
		},
		nil,
		{pressed=function (widget) HUD:PulsePressed(widget) end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.PulseCharging = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.PulseEnabled})
	self.widgets.Root:AddChild(self.widgets.PulseCharging)
	self.widgets.PulseCharging:SetDrawMode(kMaterialWidgetDrawMode_Circle)
	self.widgets.PulseCharging:SetVisible(false)
	
	self.widgets.PulseShimmer = UI:CreateWidget("MatWidget", {rect={0, 0, 8, 8}, material=self.gfx.RechargeShimmer})
	self.widgets.Root:AddChild(self.widgets.PulseShimmer)
	self.widgets.PulseShimmer:SetVisible(false)
	
	self.widgets.Pulse.flashing = false
	
	self.armEnabled = false
	self.manipulateEnabled = false
	self.shieldEnabled = false
	self.pulseEnabled = false
	
	self.widgets.Arm.class:SetEnabled(self.widgets.Arm, false)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, false)
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, false)
end

function HUD.ArmPressed(self, widget)
	COutLine(kC_Debug, "Arm Pressed!")
	if (Game.entity.manipulate or Game.entity.pulse) then
		return
	end
	Arm:Start("chat")
end

function HUD.ManipulatePressed(self, widget)
	if (Game.entity.pulse) then
		return
	end
	Game.entity:BeginManipulate()
end

function HUD.ShieldPressed(self, widget)
	if (World.playerPawn.shieldActive) then
		if (self.shieldExpiryTimer) then
			self.shieldExpiryTimer:Clean()
			self.shieldExpiryTimer = nil
		end
		HUD:ExpireShield()
		return
	end
	
	World.playerPawn:BeginShield()
	
	-- start ticking down the time.
	self.shieldStartTime = Game.time
	
	local gfx = {
		enabled = self.gfx.ShieldDisabled
	}
	
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.ShieldCharging:SetVisible(true)
	self.widgets.ShieldCharging:FillCircleTo(-1, 0)
		
	local f = function ()
		HUD:ExpireShield()
	end
	
	self.shieldExpiryTimer = World.gameTimers:Add(f, PlayerSkills.MaxShieldTime)
	
	f = function ()
		local dd = Game.time - HUD.shieldStartTime
		self.widgets.ShieldCharging:FillCircleTo(-(1 - (dd / PlayerSkills.MaxShieldTime)), 0)
	end
	
	self.shieldTimer = World.gameTimers:Add(f, 0)
	
end

function HUD.PulsePressed(self, widget)
	COutLine(kC_Debug, "Pulse Pressed!")
	if (Game.entity.manipulate) then
		return
	end
	Game.entity:BeginPulse()
end

function HUD.RechargePulse(self)
	-- pulse was fired
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, false)
		
	local rechargeTime = PlayerSkills:PulseRechargeTime()
	
	self.widgets.PulseCharging:SetVisible(true)
	self.widgets.PulseCharging:FillCircleTo(0, 0)
		
	local f = function ()
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
	
	World.gameTimers:Add(f, rechargeTime, true)
	
	self.pulseStart = Game.time
	
	f = function ()
		local dd = Game.time - self.pulseStart
		self.widgets.PulseCharging:FillCircleTo(dd / rechargeTime, 0)
	end
	
	self.pulseTimer = World.gameTimers:Add(f, 0)
end

function HUD.Layout(self)

	if (not PlayerSkills:ArmUnlocked()) then
		self.widgets.Arm:SetVisible(false)
	end

	local y = 42
	if (PlayerSkills:ManipulateUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth, y)
		self.widgets.ManipulateCharging:SetRect(r)
		self.widgets.ManipulateShimmer:SetRect(r)
		y = y + r[4] + (24 * UI.identityScale[2])
	else
		self.widgets.Manipulate:SetVisible(false)
	end
	if (PlayerSkills:ShieldUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth, y)
		self.widgets.ShieldCharging:SetRect(r)
		self.widgets.ShieldShimmer:SetRect(r)
		y = y + r[4] + (24 * UI.identityScale[2])
	else
		self.widgets.Shield:SetVisible(false)
	end
	if (PlayerSkills:PulseUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth, y)
		self.widgets.PulseCharging:SetRect(r)
		self.widgets.PulseShimmer:SetRect(r)
	else
		self.widgets.Pulse:SetVisible(false)
	end
end

function HUD.AnimateUnlock(self, items)

	self.sfx.AbilityUnlocked:Play(kSoundChannel_UI, 0)

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
		local unlock = items.manipulate
		local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth, y)
		self.widgets.ManipulateCharging:SetRect(r)
		self.widgets.ManipulateShimmer:SetRect(r)
		y = y + r[4] + (24 * UI.identityScale[2])
		self.widgets.Manipulate:SetVisible(true)
		if (unlock) then
			self.widgets.Manipulate:ScaleTo({0,0}, {0,0})
			self.widgets.Manipulate:ScaleTo({1,1}, {0.3, 0.3})
			if (self.manipulateEnabled) then
				local f = function()
					HUD:Shimmer(self.widgets.ManipulateShimmer)
				end
				World.gameTimers:Add(f, 0.3, true)
			end
		end
	end
	
	if (PlayerSkills:ShieldUnlocked()) then
		local unlock = items.shield
		if (unlock) then
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
				World.gameTimers:Add(f, 0.3, true)
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
		local unlock = items.pulse
		if (unlock) then
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
				World.gameTimers:Add(f, 0.3, true)
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
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
		
	local rechargeTime = PlayerSkills:ManipulateRechargeTime()
	
	self.widgets.ManipulateCharging:SetVisible(true)
	self.widgets.ManipulateCharging:FillCircleTo(0, 0)
		
	local f = function ()
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
	
	World.gameTimers:Add(f, rechargeTime, true)
	
	self.manipulateStart = Game.time
	
	f = function ()
		local dd = Game.time - self.manipulateStart
		self.widgets.ManipulateCharging:FillCircleTo(dd / rechargeTime, 0)
	end
	
	self.manipulateTimer = World.gameTimers:Add(f, 0)
end

function HUD.ExpireShield(self)

	if (self.shieldTimer) then
		self.shieldTimer:Clean()
		self.shieldTimer = nil
	end

	local timeUsed = Min(Game.time - self.shieldStartTime, PlayerSkills.MaxShieldTime)
	local gfx = {
		enabled = self.gfx.ShieldEnabled,
		disabled = self.gfx.ShieldDisabled
	}
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, false)
	
	self.shieldRechargeTime = PlayerSkills:ShieldRechargeTime(timeUsed)
	self.shieldStartTime = Game.time
	self.shieldRechargeFrac = (PlayerSkills.MaxShieldTime - timeUsed) / PlayerSkills.MaxShieldTime
	
	local f = function ()
		if (not HUD.enabled) then
			return
		end
		if (self.shieldTimer) then
			self.shieldTimer:Clean()
			self.shieldTimer = nil
		end
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.Shield.class:SetEnabled(self.widgets.Shield, true)
		HUD:Shimmer(self.widgets.ShieldShimmer)
	end
	
	World.gameTimers:Add(f, self.shieldRechargeTime, true)
	
	f = function ()
		local dd = Game.time - self.shieldStartTime
		dd = dd / self.shieldRechargeTime
		dd = self.shieldRechargeFrac + (dd * (1 - self.shieldRechargeFrac))
		self.widgets.ShieldCharging:FillCircleTo(-dd, 0)
	end
	
	self.shieldTimer = World.gameTimers:Add(f, 0)
	
	World.playerPawn:EndShield()
	
end

function HUD.Shimmer(self, widget)
	widget:SetVisible(true)
	widget:BlendTo({0,0,0,0}, 0)
	widget:BlendTo({1,1,1,1}, 0.2)
	
	local f = function()
		widget:BlendTo({0,0,0,0}, 0.2)
	end
	
	World.gameTimers:Add(f, 0.2, true)
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
	
	if (self.shieldExpiryTimer) then
		self.shieldExpiryTimer:Clean()
		self.shieldExpiryTimer = nil
	end
	if (self.shieldTimer) then
		self.shieldTimer:Clean()
		self.shieldTimer = nil
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

