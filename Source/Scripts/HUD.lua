-- HUD.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

HUD = Class:New()
HUD.kArmActivityFlashDuration = 0.5

function HUD.Spawn(self)
	
	HUD:Load()
	HUD:Layout()
	HUD.enabled = true
	HUD.visible = true
	
	HUD:CreatePrinter()
		
	World.globalTimers:Add(
		function () HUD:Think() end,
		0,
		true
	)
end

function HUD.CreatePrinter(self)

	local typeface = World.Load("UI/HUD_TF")
	
	if (UI.mode == kGameUIMode_Mobile) then
		local rect = self.widgets.Arm:Rect()
		
		HUD.printerRect = {
			0,
			8,
			UI.screenWidth * 0.5,
			UI.screenHeight
		}
		
		HUD.printerRect[1] = (UI.screenWidth - HUD.printerRect[3])/2
	else
		HUD.printerRect = {
			0,
			0,
			UI.screenWidth * 0.5,
			UI.screenHeight
		}
		
		HUD.printerRect[1] = UI.screenWidth - HUD.printerRect[3]
	end
	
	HUD.printer = TextPrinter:New(typeface, HUD.printerRect, 5, 50, nil, UI.widgets.hudprint.Root)

end

function HUD.Print(self, icon, text, callback, useStringTable)

	if (UI.mode == kGameUIMode_Mobile) then
		self.printer:PrintHCentered(
			icon,
			text,
			0.5,
			3,
			3,
			1,
			callback,
			useStringTable
		)
	else
		self.printer:PrintRightAligned(
			icon,
			text,
			1,
			3,
			3,
			1,
			callback,
			useStringTable
		)
	end
	
end

function HUD.SetVisible(self, visible)
	self.visible = visible
	self.widgets.Root:SetVisible(visible)
	UI.widgets.discoveries.Root:SetVisible(visible)
	UI.widgets.hudprint.Root:SetVisible(visible)
end

function HUD.Load(self)
	self.widgets = {}
	self.widgets.Root = UI:CreateRoot(UI.kLayer_HUD)
	
	HUD.mobileInset = 28 * UI.identityScale[1]
	HUD.mobileButtonSpace = 16 * UI.identityScale[2]
	HUD.mobileButtonY = 35 * UI.identityScale[2]
	
	HUD.pulseMode = "Normal"
	
	if (UI.mode == kGameUIMode_Mobile) then
		self.gfx = {
			Arm = "UI/arm_button_M",
			ArmPressed = "UI/arm_button_pressed_M",
			ManipulateDisabled = "UI/manipulate_button_charging_M",
			ManipulateEnabled = "UI/manipulate_button_M",
			ManipulateFlashing = "UI/manipulate_button_flashing_M",
			RechargeShimmer = "UI/ability_recharge_shimmer_M",
			PulseDisabled = "UI/pulse_button_charging_M",
			PulseEnabled = "UI/pulse_button_M",
			PulseFlashing = "UI/pulse_button_flashing_M",
			ShieldDisabled = "UI/shield_button_charging_M",
			ShieldEnabled = "UI/shield_button_M",
			PowerBubble = "UI/power_bubble_M",
			PowerBubbleDisabled = "UI/power_bubble_disabled_M",
			DropMine = "UI/dropmine_M",
			DropMineDisabled = "UI/dropmine_disabled_M",
			RapidPulse = "UI/rapid_pulse_M",
			RapidPulseFlashing = "UI/rapid_pulse_flashing_M",
			RapidPulseDisabled = "UI/rapid_pulse_disabled_M",
			TNTBox = "UI/tntbox_M",
			TNTBoxDisabled = "UI/tntbox_disabled_M"
		}
	else
		self.gfx = {
			Arm = "UI/arm_button_pc_M",
			ArmPressed = "UI/arm_button_pressed_pc_M",
			ManipulateDisabled = "UI/manipulate_button_charging_pc_M",
			ManipulateEnabled = "UI/manipulate_button_pc_M",
			ManipulateFlashing = "UI/manipulate_button_flashing_pc_M",
			RechargeShimmer = "UI/ability_recharge_shimmer_M",
			PulseDisabled = "UI/pulse_button_charging_pc_M",
			PulseEnabled = "UI/pulse_button_pc_M",
			PulseFlashing = "UI/pulse_button_flashing_pc_M",
			ShieldDisabled = "UI/shield_button_charging_pc_M",
			ShieldEnabled = "UI/shield_button_pc_M",
			ActionBar = "UI/action_bar_pc_M",
			PowerBubble = "UI/power_bubble_pc_M",
			PowerBubbleDisabled = "UI/power_bubble_disabled_pc_M",
			DropMine = "UI/dropmine_pc_M",
			DropMineDisabled = "UI/dropmine_disabled_pc_M",
			RapidPulse = "UI/rapid_pulse_pc_M",
			RapidPulseFlashing = "UI/rapid_pulse_flashing_pc_M",
			RapidPulseDisabled = "UI/rapid_pulse_disabled_pc_M",
			TNTBox = "UI/tntbox_pc_M",
			TNTBoxDisabled = "UI/tntbox_disabled_pc_M"
		}
	end
	
	self.gfx.ArmActivity = "UI/arm_activity_M"
	self.gfx.ArmSignaled = "UI/arm_signaled_M"
	
	map(self.gfx, World.Load)
	
	self.pulseFlashing = self.gfx.PulseFlashing
	self.pulseGfx = {
		enabled = self.gfx.PulseEnabled,
		disabled = self.gfx.PulseDisabled,
		pressed = self.gfx.PulseDisabled
	}
	
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
	
	self.widgets.ArmActivity = UI:CreateWidget("MatWidget", {rect={0,0,8,8,}, material=self.gfx.ArmActivity})
	self.widgets.ArmActivity:SetVisible(false)
	self.widgets.Arm:AddChild(self.widgets.ArmActivity)
	
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
	
	local size = UI:MaterialSize(self.gfx.PowerBubble)
	
	self.widgets.PowerBubble = UIPushButton:Create(
		{0, 0, size[3], size[4]},
		{ -- we go to disabled state when pulse is fired
			enabled = self.gfx.PowerBubble,
			disabled = self.gfx.PowerBubbleDisabled,
			pressed = self.gfx.PowerBubbleDisabled
		},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:PowerBubblePressed() end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.PowerBubble:SetVisible(false)
	
	self.widgets.DropMine = UIPushButton:Create(
		{0, 0, size[3], size[4]},
		{ -- we go to disabled state when pulse is fired
			enabled = self.gfx.DropMine,
			disabled = self.gfx.DropMineDisabled,
			pressed = self.gfx.DropMineDisabled
		},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:DropMinePressed() end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.DropMine:SetVisible(false)
	
	self.widgets.RapidPulse = UIPushButton:Create(
		{0, 0, size[3], size[4]},
		{ -- we go to disabled state when pulse is fired
			enabled = self.gfx.RapidPulse,
			disabled = self.gfx.RapidPulseDisabled,
			pressed = self.gfx.RapidPulseDisabled
		},
		{ pressed = UI.sfx.Command },
		{pressed=function (widget) HUD:RapidPulsePressed() end},
		nil,
		self.widgets.Root
	)
	
	self.widgets.RapidPulse:SetVisible(false)
	
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
		
		local activityRect = self.widgets.Arm:Rect()
		activityRect[1] = 0
		activityRect[2] = 0
		activityRect[3] = activityRect[3] * 0.4
		activityRect[4] = activityRect[3]
		activityRect = CenterChildRectInRect(activityRect, self.widgets.Arm:Rect())
		self.widgets.ArmActivity:SetRect(activityRect)
	
	else
		local activityRect = self.widgets.Arm:Rect()
		activityRect[1] = 0
		activityRect[2] = 0
		activityRect[3] = activityRect[3] * 0.35
		activityRect[4] = activityRect[3]
		activityRect = CenterRectInRect(activityRect, {46,30,185,105})
		self.widgets.ArmActivity:SetRect(activityRect)
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

function HUD.PowerBubblePressed(self)
	COutLine(kC_Debug, "Power Bubble Pressed!")
	Game.entity:PowerBubble()
end

function HUD.DropMinePressed(self)
	COutLine(kC_Debug, "Drop Mine Pressed!")
	if ((self.pulseMode == "Mines") or Game.entity.pulse) then -- sanity check
		Game.entity:DropMine()
	end
end

function HUD.RapidPulsePressed(self)
	COutLine(kC_Debug, "Rapid Pulse Pressed!")
	Game.entity:RapidPulse()
end

function HUD.BeginShield(self, gameTime)

	if (gameTime == nil) then
		World.playerPawn:BeginShield()
		gameTime = GameDB.realTime
	end
	
	HUD:RefreshAvailableActions()
	
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

function HUD.ShieldPenalty(self)
	if (World.playerPawn.shieldActive) then
		local penalty = PlayerSkills:ShieldMultiActionTimePenalty()
		if (penalty > 0) then
			local maxTime = PlayerSkills:MaxShieldTime()
			local elapsed = GameDB.realTime - HUD.shieldStartTime
			if ((elapsed+penalty) >= maxTime) then
				self:ExpireShield(maxTime)
			else
				HUD.shieldStartTime = HUD.shieldStartTime - penalty -- just like if we started earlier
				local f = function ()
					HUD:ExpireShield()
				end
				
				self.shieldExpiryTimer:Clean()
				self.shieldExpiryTimer = World.globalTimers:Add(f, PlayerSkills:MaxShieldTime() - (elapsed+penalty))
			end
		end
	end
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
	self.pulseStartTime = GameDB.realTime
	HUD:InternalRechargePulse()
end

function HUD.InternalRechargePulse(self)
	-- pulse was fired
	self.widgets.Pulse.class:SetEnabled(self.widgets.Pulse, false)
	self.pulseEnabled = false
		
	self.widgets.PulseCharging:SetVisible(true)
	self.widgets.PulseCharging:FillCircleTo(0, 0)
		
	local f = function ()
		self.pulseStartTime = nil
		if (self.pulseTimer) then
			self.pulseTimer:Clean()
			self.pulseTimer = nil
		end
		if (not HUD.enabled) then
			return
		end
		self.widgets.PulseCharging:SetVisible(false)
		HUD:RefreshAvailableActions()
		HUD:Shimmer(self.widgets.PulseShimmer)
		
		if (self.pulseMode == "Mines") then
			local maxMines = PlayerSkills:MaxMines()
			local numMines = World.playerPawn:NumMines()
			if (numMines < maxMines) then
				self.widgets.DropMine.class:SetEnabled(self.widgets.DropMine, true)
			end
		end
	end
	
	World.globalTimers:Add(f, self.pulseRechargeTime - (GameDB.realTime - self.pulseStartTime))
	
	f = function ()
		local dd = GameDB.realTime - self.pulseStartTime
		self.widgets.PulseCharging:FillCircleTo(dd / self.pulseRechargeTime, 0)
	end
	
	self.pulseTimer = World.globalTimers:Add(f, 0, true)
end

function HUD.ShowPulseModes(self, show)

	if (UI.mode == kGameUIMode_Mobile) then
		HUD:ShowPulseModesMobile(show)
	end

end

function HUD.NotifyZeroMines(self)
	if (self.pulseMode == "Mines") then
		HUD:SwitchPulseMode("Normal")
		HUD:ShowPulseModes(false)
		self.widgets.DropMine.class:SetEnabled(self.widgets.DropMine, true)
	end
end

function HUD.SwitchPulseMode(self, mode)

	if (self.pulseMode == mode) then
		return
	end
	
	self.pulseMode = mode
	
	if (mode == "Mines") then
	
		self.pulseGfx = {
			enabled = self.gfx.TNTBox,
			disabled = self.gfx.TNTBoxDisabled,
			pressed = self.gfx.TNTBoxDisabled
		}
	
		-- Main pulse button becomes a tnt detonate box
		self.widgets.Pulse.class:ChangeGfx(
			self.widgets.Pulse,
			self.pulseGfx
		)
		
		self.widgets.PulseCharging:SetMaterial(self.gfx.TNTBox)
		
	elseif (mode == "Rapid") then
	
		-- swap pulse with rapid pulse
		
		self.pulseGfx = {
			enabled = self.gfx.RapidPulse,
			disabled = self.gfx.RapidPulseDisabled,
			pressed = self.gfx.RapidPulseDisabled
		}
			
		self.widgets.Pulse.class:ChangeGfx(
			self.widgets.Pulse,
			self.pulseGfx			
		)
		
		self.widgets.PulseCharging:SetMaterial(self.gfx.RapidPulse)
		
		self.widgets.RapidPulse.class:ChangeGfx(
			self.widgets.RapidPulse,
			{
				enabled = self.gfx.PulseEnabled,
				disabled = self.gfx.PulseDisabled,
				pressed = self.gfx.PulseDisabled
			}
		)
		
		self.pulseFlashing = self.gfx.RapidPulseFlashing
	
	else
	
		self.pulseFlashing = self.gfx.PulseFlashing
		
		self.pulseGfx = {
			enabled = self.gfx.PulseEnabled,
			disabled = self.gfx.PulseDisabled,
			pressed = self.gfx.PulseDisabled
		}
	
		self.widgets.Pulse.class:ChangeGfx(
			self.widgets.Pulse,
			self.pulseGfx
		)
		
		self.widgets.PulseCharging:SetMaterial(self.gfx.PulseEnabled)
		
		self.widgets.RapidPulse.class:ChangeGfx(
			self.widgets.RapidPulse,
			{
				enabled = self.gfx.RapidPulse,
				disabled = self.gfx.RapidPulseDisabled,
				pressed = self.gfx.RapidPulseDisabled
			}
		)
		
	end

end

function HUD.DetonateMines(self)
	World.playerPawn:DetonateMines()
end

function HUD.ShowPulseModesMobile(self, show)

	local r = self.widgets.Pulse:Rect()
	local pulseIconCenter = {r[1] + r[3]/2, r[2] + r[4]/2}
	local space = 8 * UI.identityScale[1]
	local buttonRect = self.widgets.PowerBubble:Rect()
	local startY = r[2] + (r[4]-buttonRect[4])/2
	local blendTime = 0.2
	local rollOutTime = 0.2	
	
	-- pyramid setup
	local kButtonPositions = {
		{ -- 1 button
			{
				r[1] - buttonRect[3],
				startY
			}
		},
		{ -- 2 buttons
			{
				r[1] - buttonRect[3],
				startY
			},
			{
				pulseIconCenter[1] - buttonRect[3],
				startY + buttonRect[4] + space
			}
		},
		{ -- 3 buttons
			{
				r[1] - buttonRect[3],
				startY
			},
			{
				pulseIconCenter[1] - buttonRect[3],
				startY + buttonRect[4]
			},
			{
				pulseIconCenter[1] - buttonRect[3]/2,
				startY + (buttonRect[4]+space)*2
			}
		}
	}
	
	local kNumButtons = 0
	
	if (PlayerSkills.PowerBubble > 0) then
		kNumButtons = kNumButtons + 1
	end
	
	if (PlayerSkills.Mines > 0) then
		kNumButtons = kNumButtons + 1
	end
	
	if (PlayerSkills.TriggerHappy > 0) then
		kNumButtons = kNumButtons + 1
	end
	
	local buttonIdx = 1
		
	if (PlayerSkills.PowerBubble > 0) then
		
		local z = self.widgets.PowerBubble:Rect()
		z[1] = pulseIconCenter[1] - (z[3]/2)
		z[2] = pulseIconCenter[2] - (z[4]/2)
			
		if (show) then
			self.widgets.PowerBubble:SetVisible(true)
			self.widgets.PowerBubble:BlendTo({1,1,1,0}, 0)
			self.widgets.PowerBubble:BlendTo({1,1,1,1}, blendTime)
									
			self.widgets.PowerBubble:MoveTo(z, {0,0})
			self.widgets.PowerBubble:MoveTo(kButtonPositions[kNumButtons][buttonIdx], {rollOutTime, rollOutTime})
			
			buttonIdx = buttonIdx + 1
		else
			self.widgets.PowerBubble:MoveTo(z, {rollOutTime, rollOutTime})
			self.widgets.PowerBubble:BlendTo({1,1,1,0}, blendTime)
		end
	end
	
	if (PlayerSkills.Mines > 0) then
	
		if (HUD.pulseMode == "Mines") then
			-- don't touch the mines button in mines mode
			buttonIdx = buttonIdx + 1
		else
		
			local z = self.widgets.DropMine:Rect()
			z[1] = pulseIconCenter[1] - (z[3]/2)
			z[2] = pulseIconCenter[2] - (z[4]/2)
				
			if (show) then
				self.widgets.DropMine:SetVisible(true)
				self.widgets.DropMine:BlendTo({1,1,1,0}, 0)
				self.widgets.DropMine:BlendTo({1,1,1,1}, blendTime)
										
				self.widgets.DropMine:MoveTo(z, {0,0})
				self.widgets.DropMine:MoveTo(kButtonPositions[kNumButtons][buttonIdx], {rollOutTime, rollOutTime})
				
				buttonIdx = buttonIdx + 1
			else
				self.widgets.DropMine:MoveTo(z, {rollOutTime, rollOutTime})
				self.widgets.DropMine:BlendTo({1,1,1,0}, blendTime)
			end
		end
	end
	
	if (PlayerSkills.TriggerHappy > 0) then
		
		local z = self.widgets.RapidPulse:Rect()
		z[1] = pulseIconCenter[1] - (z[3]/2)
		z[2] = pulseIconCenter[2] - (z[4]/2)
			
		if (show) then
			self.widgets.RapidPulse:SetVisible(true)
			self.widgets.RapidPulse:BlendTo({1,1,1,0}, 0)
			self.widgets.RapidPulse:BlendTo({1,1,1,1}, blendTime)
									
			self.widgets.RapidPulse:MoveTo(z, {0,0})
			self.widgets.RapidPulse:MoveTo(kButtonPositions[kNumButtons][buttonIdx], {rollOutTime, rollOutTime})
			
			buttonIdx = buttonIdx + 1
		else
			self.widgets.RapidPulse:MoveTo(z, {rollOutTime, rollOutTime})
			self.widgets.RapidPulse:BlendTo({1,1,1,0}, blendTime)
		end
	end

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
	local y = HUD.mobileButtonY
	if (PlayerSkills:ManipulateUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth - HUD.mobileInset, y)
		self.widgets.ManipulateCharging:SetRect(r)
		self.widgets.ManipulateShimmer:SetRect(r)
		y = y + r[4] + HUD.mobileButtonSpace
		self.widgets.Manipulate:SetVisible(true)
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.ManipulateShimmer:SetVisible(false)
	else
		self.widgets.Manipulate:SetVisible(false)
		self.widgets.ManipulateCharging:SetVisible(false)
		self.widgets.ManipulateShimmer:SetVisible(false)
	end
	if (PlayerSkills:ShieldUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth - HUD.mobileInset, y)
		self.widgets.ShieldCharging:SetRect(r)
		self.widgets.ShieldShimmer:SetRect(r)
		y = y + r[4] + HUD.mobileButtonSpace
		self.widgets.Shield:SetVisible(true)
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.ShieldShimmer:SetVisible(false)
	else
		self.widgets.Shield:SetVisible(false)
		self.widgets.ShieldCharging:SetVisible(false)
		self.widgets.ShieldShimmer:SetVisible(false)
	end
	if (PlayerSkills:PulseUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth - HUD.mobileInset, y)
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

	local y = HUD.mobileButtonY
	if (PlayerSkills:ManipulateUnlocked()) then
		local r = UI:RAlignWidget(self.widgets.Manipulate, UI.screenWidth - HUD.mobileInset, y)
		self.widgets.ManipulateCharging:SetRect(r)
		self.widgets.ManipulateShimmer:SetRect(r)
		y = y + r[4] + HUD.mobileButtonSpace
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
			local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth - HUD.mobileInset, y)
			self.widgets.ShieldCharging:SetRect(r)
			self.widgets.ShieldShimmer:SetRect(r)
			y = y + r[4] + HUD.mobileButtonSpace
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
			local r = UI:RAlignWidget(self.widgets.Shield, UI.screenWidth - HUD.mobileInset, y)
			self.widgets.Shield:SetRect(oldRect)
			self.widgets.Shield:MoveTo({r[1], r[2]}, {0.3, 0.3})
			self.widgets.ShieldCharging:MoveTo({r[1], r[2]}, {0.3, 0.3})
			self.widgets.ShieldShimmer:MoveTo({r[1], r[2]}, {0.3, 0.3})
			y = y + r[4] + HUD.mobileButtonSpace
		end
	end
	
	if (PlayerSkills:PulseUnlocked()) then
		if (items.pulse) then
			local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth - HUD.mobileInset, y)
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
			local r = UI:RAlignWidget(self.widgets.Pulse, UI.screenWidth - HUD.mobileInset, y)
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
		local gfx = nil
		if (flashing) then
			gfx = {enabled = self.pulseFlashing}
		else
			gfx = self.pulseGfx
		end
		
		self.widgets.Pulse.class:ChangeGfx(self.widgets.Pulse, gfx)
	end
end

function HUD.RechargeManipulate(self)

	self.manipulateRechargeTime = PlayerSkills:ManipulateRechargeTime()
	self.manipulateStartTime = GameDB.realTime
	
	HUD:InternalRechargeManipulate()
	
end

function HUD.InternalRechargeManipulate(self)
	self.widgets.Manipulate.class:SetEnabled(self.widgets.Manipulate, false)
	self.manipulateEnabled = false
	
	self.widgets.ManipulateCharging:SetVisible(true)
	self.widgets.ManipulateCharging:FillCircleTo(0, 0)
		
	local f = function ()
		self.manipulateStartTime = nil
		if (not HUD.enabled) then
			return
		end
		if (self.manipulateTimer) then
			self.manipulateTimer:Clean()
			self.manipulateTimer = nil
		end
		self.widgets.ManipulateCharging:SetVisible(false)
		HUD:RefreshAvailableActions()
		HUD:Shimmer(self.widgets.ManipulateShimmer)
	end
	
	World.globalTimers:Add(f, self.manipulateRechargeTime - (GameDB.realTime - self.manipulateStartTime))
	
	f = function ()
		local dd = GameDB.realTime - self.manipulateStartTime
		self.widgets.ManipulateCharging:FillCircleTo(dd / self.manipulateRechargeTime, 0)
	end
	
	self.manipulateTimer = World.globalTimers:Add(f, 0, true)
end

function HUD.ExpireShield(self, timeUsed)

	if (self.shieldTimer) then
		self.shieldTimer:Clean()
		self.shieldTimer = nil
	end
	
	if (self.shieldExpiryTimer) then
		self.shieldExpiryTimer:Clean()
		self.shieldExpiryTimer = nil
	end
	
	if (timeUsed == nil) then
		timeUsed = Min(GameDB.realTime - self.shieldStartTime, PlayerSkills:MaxShieldTime())
	end
	
	self.shieldRechargeTime = PlayerSkills:ShieldRechargeTime(timeUsed)
	self.shieldStartTime = GameDB.realTime
	self.shieldRechargeFrac = (PlayerSkills:MaxShieldTime() - timeUsed) / PlayerSkills:MaxShieldTime()
	
	HUD:RechargeShield()
	
	World.playerPawn:EndShield()
	HUD:RefreshAvailableActions()
end

function HUD.RechargeShield(self, instant)

	if (instant) then
		self.shieldCharging = false
		self.widgets.PowerBubble.class:SetEnabled(self.widgets.PowerBubble, true)
		
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
		HUD:RefreshAvailableActions()
		return
	end

	local gfx = {
		enabled = self.gfx.ShieldEnabled,
		disabled = self.gfx.ShieldDisabled
	}
	
	self.widgets.Shield.class:ChangeGfx(self.widgets.Shield, gfx)
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, false)
	self.shieldEnabled = false
	self.shieldCharging = true
	self.widgets.ShieldCharging:SetVisible(true)
	self.widgets.PowerBubble.class:SetEnabled(self.widgets.PowerBubble, false)
	
	local f = function ()
		self.shieldStartTime = nil
		self.shieldCharging = false
		self.widgets.PowerBubble.class:SetEnabled(self.widgets.PowerBubble, true)
		
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
		HUD:RefreshAvailableActions()
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

function HUD.ArmActivity(self)

	if (self.armSignaled) then
		return
	end

	if (self.armActivityTimer) then
		self.armActivityTimer:Clean()
	end
	
	self.widgets.ArmActivity:SetMaterial(self.gfx.ArmActivity)
	self.widgets.ArmActivity:SetVisible(true)
	
	local f = function()
		self.widgets.ArmActivity:SetVisible(false)
	end
	
	World.globalTimers:Add(f, HUD.kArmActivityFlashDuration)

end

function HUD.PlayerDied(self)
	HUD:Disable()
end

function HUD.EnableAll(self)
	HUD:Enable({"arm", "manipulate", "shield", "pulse"})
end

function HUD.Enable(self, items)

	self.itemState = {
		arm = false,
		manipulate = false,
		shield = false,
		pulse = false
	}
	
	for k,v in pairs(items) do
		self.itemState[v] = true
	end
	
	HUD:RefreshAvailableActions()
end

function HUD.RefreshAvailableActions(self)

	if (not HUD.enabled) then
		return
	end
		
	if (Game.entity.pulse) then
		HUD:EnableArm(false)
		HUD:EnableManipulate(false)
		if (PlayerSkills.MultiShield >= 3) then
			if (self.shieldStartTime == nil) then
				HUD:EnableShield(self.itemState.shield)
			end
		else
			HUD:EnableShield(false)
		end
	elseif (Game.entity.manipulate) then
		HUD:EnableArm(false)
		HUD:EnablePulse(false)
		if (PlayerSkills.MultiShield >= 1) then
			if (self.shieldStartTime == nil) then
				HUD:EnableShield(self.itemState.shield)
			end
		else
			HUD:EnableShield(false)
		end
	elseif (World.playerPawn.shieldActive) then
		HUD:EnableArm(false)
		if (PlayerSkills.MultiShield >= 3) then
			if (self.pulseStartTime == nil) then
				HUD:EnablePulse(self.itemState.pulse)
			end
		else
			HUD:EnablePulse(false)
		end
		if (PlayerSkills.MultiShield >= 1) then
			if (self.manipulateStartTime == nil) then
				HUD:EnableManipulate(self.itemState.manipulate)
			end
		else
			HUD:EnableManipulate(false)
		end
	else
		HUD:EnableArm(self.itemState.arm)
		if (self.shieldStartTime == nil) then
			HUD:EnableShield(self.itemState.shield)
		end
		if (self.manipulateStartTime == nil) then
			HUD:EnableManipulate(self.itemState.manipulate)
		end
		if (self.pulseStartTime == nil) then
			HUD:EnablePulse(self.itemState.pulse)
		end
	end
	
end

function HUD.EnableArm(self, enable)
	if (self.armEnabled == enable) then
		return
	end
	
	self.armEnabled = enable
	self.widgets.Arm.class:SetEnabled(self.widgets.Arm, enable)
end

function HUD.SignalArm(self, signal)

	self.armSignaled = signal
	
	if (signal) then
		self.widgets.ArmActivity:SetMaterial(self.gfx.ArmSignaled)
		self.widgets.ArmActivity:SetVisible(true)
		self.sfx.Signaled:Play(kSoundChannel_UI, 0)
	else
		self.widgets.ArmActivity:SetVisible(false)
	end

end

function HUD.EnableManipulate(self, enable)
	if (HUD.enabled and (self.manipulateEnabled == enable)) then
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
	if (HUD.enabled and (self.shieldEnabled == enable)) then
		return
	end
	
	self.shieldEnabled = enable
	
	if (self.shieldStartTime and (not World.playerPawn.dead) and HUD.enabled) then
		return
	end
	
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
	self.widgets.Shield.class:SetEnabled(self.widgets.Shield, enable)
end

function HUD.EnablePulse(self, enable)
	if (HUD.enabled and (self.pulseEnabled == enable)) then
		return
	end
	
	self.pulseEnabled = enable
	
	if (self.pulseTimer) then
		self.pulseTimer:Clean()
		self.pulseTimer = nil
	end
	
	local gfx = self.pulseGfx
	
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
	
	if (self.manipulateStartTime) then
		Persistence.WriteString(SaveGame, "HUDManipulateState", "charging")
		Persistence.WriteNumber(SaveGame, "HUDManipulateTime", self.manipulateStartTime)
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
		self.manipulateStartTime = Persistence.ReadNumber(SaveGame, "HUDManipulateTime")
		self.manipulateRechargeTime = Persistence.ReadNumber(SaveGame, "HUDManipulateRechargeTime")
		HUD:InternalRechargeManipulate()
	else
		self.manipulateStartTime = nil
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

	self.shieldCharging = false
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
	
	if (self.pulseStartTime) then
		Persistence.WriteString(SaveGame, "HUDPulseState", "charging")
		Persistence.WriteNumber(SaveGame, "HUDPulseTime", self.pulseStartTime)
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
		self.pulseStartTime = Persistence.ReadNumber(SaveGame, "HUDPulseTime")
		self.pulseRechargeTime = Persistence.ReadNumber(SaveGame, "HUDPulseRechargeTime")
		HUD:InternalRechargePulse()
	else
		self.pulseStartTime = nil
	end

end

function HUD.SaveState(self)
--	Persistence.WriteBool(SaveGame, "HUDVisible", self.visible)
	HUD:SaveManipulateState()
	HUD:SaveShieldState()
	HUD:SavePulseState()
end

function HUD.LoadState(self)
	self.enabled = true
	self.armActivityTimer = nil
	self.armSignaled = false
	self.printer:Clear()
	self.printer.timers = World.globalTimers
	self.widgets.PowerBubble:SetVisible(false)
	self.widgets.DropMine:SetVisible(false)
	self.widgets.RapidPulse:SetVisible(false)
	self.widgets.ArmActivity:SetVisible(false)
	self.widgets.PowerBubble.class:SetEnabled(self.widgets.PowerBubble, true)
	self.widgets.DropMine.class:SetEnabled(self.widgets.DropMine, true)
	self.widgets.RapidPulse.class:SetEnabled(self.widgets.RapidPulse, true)
	HUD:SwitchPulseMode("Normal")
--	HUD.visible = Persistence.ReadBool(SaveGame, "HUDVisible", true)
	HUD:SetVisible(true)
	HUD:Layout()
	HUD:LoadManipulateState()
	HUD:LoadShieldState()
	HUD:LoadPulseState()
	HUD:RefreshAvailableActions()
end
