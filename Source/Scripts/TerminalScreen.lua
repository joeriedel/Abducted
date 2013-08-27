-- TerminalScreen.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TerminalScreen = Entity:New()
TerminalScreen.MaxTouchDistancePct = 1/8
TerminalScreen.TouchPosShift = {70,0,120}
TerminalScreen.ButtonPosShift = {1.5/10, 1/10}
TerminalScreen.ButtonSize = {1.5/15, 1.5/15}
TerminalScreen.PopupEntity = nil
TerminalScreen.Objects = {}
TerminalScreen.Widgets = {}
TerminalScreen.Skip = false
TerminalScreen.DT = 0
TerminalScreen.Active = nil

function TerminalScreen.Spawn(self)
	Entity.Spawn(self)
	MakeAnimatable(self)
	
	 if (BoolForString(self.keys.cast_shadows, false)) then
		self:SetLightingFlags(kObjectLightingFlag_CastShadows)
	end
	
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)
	
	self.model = LoadModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	
	local scale = Vec3ForString(self.keys.scale, {1,1,1})
	self.model.dm:ScaleTo(scale, 0)

	self:SetMins({-200*scale[1], -200*scale[2], 0*scale[3]})
	self:SetMaxs({ 200*scale[1],  200*scale[2], 282*scale[3]})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self.model.glow1 = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.glow1)
	self.model.glow1:ScaleTo(scale, 0)
	self.model.glow1:SetBounds(self:Mins(), self:Maxs())
	self.model.glow1:BlendTo({1,1,1,0}, 0)
	
	self.model.glow2 = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.glow2)
	self.model.glow2:ScaleTo(scale, 0)
	self.model.glow2:SetBounds(self:Mins(), self:Maxs())
	self.model.glow2:BlendTo({1,1,1,0}, 0)
	
	self.glyphPos = VecAdd(self:WorldPos(), RotateVecZ(TerminalScreen.TouchPosShift, self:Angles().pos[3] - 90))
	
	self.alienSkin1 = World.Load("Shared/alienskin1_M")
	self.terminalFlash1 = World.Load("Objects/terminalglow1_M")
	self.terminalFlash2 = World.Load("Objects/terminalglow2_M")
	
	self.model.glow1:ReplaceMaterial(self.alienSkin1, self.terminalFlash1)
	self.model.glow2:ReplaceMaterial(self.alienSkin1, self.terminalFlash2)
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()
	
	self.enabled = BoolForString(self.keys.enabled, false)
	self.activateRadius = NumberForString(self.keys.activate_radius, 200)
	self.hackDifficulty = NumberForString(self.keys.hack_difficulty, 1)
	self.solveGlyphs = {}
	self.hackActions = self.keys.hack_success_actions
	self.solveActions = self.keys.solve_success_actions
	self.downgraded = false
	self.failcount = 0
	self.success = false
	
	if (self.keys.success_actions) then
		if (self.hackActions) then
			self.hackActions = self.hackActions..";"..self.keys.success_actions
		else
			self.hackActions = self.keys.success_actions
		end
		
		if (self.solveActions) then
			self.solveActions = self.solveActions..";"..self.keys.success_actions
		else
			self.solveActions = self.keys.success_actions
		end
	end
	
	self.active = false
	self.popup = false
	self.size = "small"
	self.state = "none"
	
	self:PlayAnim("idle", self.model)
	
	table.insert(TerminalScreen.Objects, self)
	
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

function TerminalScreen.RegisterGlyph(self, glyphNum)

	table.insert(self.solveGlyphs, glyphNum)

end

function TerminalScreen.CheckProximity(self, playerPos)
	if (not self.enabled) then
		return false
	end
	
	local dd = VecSub(playerPos, self:WorldPos())
	dd = VecMag(dd)
	
	return (dd <= self.activateRadius)
end

function TerminalScreen.CheckActive(self, playerPos)

	local activate = self:CheckProximity(playerPos)
	self:Activate(activate)

end

function TerminalScreen.Activate(self, activate)

	if (self.active == activate) then
		if (activate) then
			self:UpdateActivated()
		end
		return
	end
	
	self.active = activate

	if (self.active) then
		self:BeginActivated()
	else
		self:EndActivated()
	end
end

function TerminalScreen.BeginActivated(self)

	if (self.size == "big") then
		if (not self.success) then
			self:PopupUI()
		end
	else
		self.activateTime = Game.time
	end

end

function TerminalScreen.EndActivated(self)
	if (self.popup) then
		TerminalScreen.CancelUI()
		Arm:ClearContext()
		TerminalScreen.Signaled = nil
	end
	
	self.model.glow1:BlendTo({1,1,1,0}, 0.1)
	self.model.glow2:BlendTo({1,1,1,0}, 0.1)
	
	if (self.size == "small") then
		self:PlayAnim("idle", self.model)
		self.state = "none"
	end
	
end

function TerminalScreen.UpdateActivated(self)

	if (self.size == "big") then
		return
	end
	
	local dt = Game.time - self.activateTime
	
	if ((self.state == "none") and (dt > 2)) then
		self.state = "wiggle"
		self:PlayAnim("wiggle1", self.model)
	elseif ((self.state == "wiggle") and (dt > 5)) then
		self.state = "flash1"
		self.model.glow1:BlendTo({1,1,1,1}, 0.1)
	elseif ((self.state == "flash1") and (dt > 7)) then
		self.state = "flash2"
		self.model.glow2:BlendTo({1,1,1,1}, 0.1)
		self.model.glow1:BlendTo({1,1,1,0}, 0.1)
	elseif ((self.state == "flash2") and (dt > 8)) then
		self.state = "grow"
		self.model.glow2:BlendTo({1,1,1,0}, 2)
		self.size = "big"
		local f = function()
			if (self.active) then
				self:PopupUI()
			end
			self:PlayAnim("grown", self.model)
		end
		self:PlayAnim("grow", self.model).Seq(f)
	end
end

function TerminalScreen.OnEvent(self, cmd, args)
	COutLineEvent("TerminalScreen", self.keys.targetname, cmd, args)
	
	if (cmd == "enable") then
		self.enabled = true
		return true
	elseif (cmd == "disable") then
		self.enabled = false
		self:Activate(false)
	end

end

function TerminalScreen.PopupUI(self)
	local f = function()
		TerminalScreen.ExecutePopup(self)
	end
	TerminalScreen.CancelUI(f) -- cancel any active UI
end

function TerminalScreen.ExecutePopup(self)
	TerminalScreen.PopupEntity = self
	self.popup = true
	TerminalScreen.Widgets.Glyph:ScaleTo({1, 1}, {0.2,0.2})
	self:CheckSignalDowngrade()
end

function TerminalScreen.CancelUI(callback)
	if (TerminalScreen.PopupEntity) then
		if (GameDB:LoadingSaveGame()) then
			TerminalScreen.Widgets.Glyph:ScaleTo({0, 0}, {0, 0})
			TerminalScreen.PopupEntity = nil
		else
			Abducted.entity.eatInput = true
			TerminalScreen.Widgets.Glyph:ScaleTo({0, 0}, {0.2,0.2})
			local f = function ()
				Abducted.entity.eatInput = false
				if (TerminalScreen.PopupEntity) then
					TerminalScreen.PopupEntity.popup = false
					TerminalScreen.PopupEntity = nil
				end
				if (callback) then
					callback()
				end
			end
			World.gameTimers:Add(f, 0.2)
		end
	else
		if (callback) then
			callback()
		end
	end
end

function TerminalScreen.UpdateUI()
	if (World.paused) then
		return
	end
	if (TerminalScreen.PopupEntity) then
		local p, r = World.Project(TerminalScreen.PopupEntity.glyphPos)
		p = UI:MapToUI(p)
		
		r = TerminalScreen.Widgets.Glyph:Rect()
		r[1] = p[1] - r[3]/2
		r[2] = p[2] - r[4]/2
		BoundRect(r, TerminalScreen.ScreenBounds)
		TerminalScreen.Widgets.Glyph:SetRect(r)
	end
end

function TerminalScreen.CheckActivate(dt)

	TerminalScreen.DT = TerminalScreen.DT + dt
	if (TerminalScreen.DT < 0.25) then
		return
	end
	
	TerminalScreen.DT = TerminalScreen.DT - 0.25

	local playerPos = World.playerPawn:WorldPos()
	
	for k,v in pairs(TerminalScreen.Objects) do
		v:CheckActive(playerPos)
	end

end

function TerminalScreen.CheckSignalDowngrade(self)
	if ((self.failcount >= 2) and (not self.downgraded) and (self.hackDifficulty > 1)) then
		TerminalScreen.Signaled = self
		Arm:SignalContext(
			"TerminalDowngrade",
			function ()
				self:ClearContextChat()
			end
		)
	end
end

function TerminalScreen.Downgrade()
	local self = TerminalScreen.Signaled
	if (self.hackDifficulty > 1) then
		self.hackDifficulty = self.hackDifficulty - 1
		self.downgraded = true
	end
end

function TerminalScreen.ClearContextChat(self)
	self.failcount = 0
end

function TerminalScreen.GlyphPressed()
	local target = TerminalScreen.PopupEntity
	World.playerPawn:Stop()
	TerminalScreen.Active = target
	local f = function()
		World.playerPawn:EnterTerminal(target)
	end
	TerminalScreen.CancelUI(f)
end

function TerminalScreen.SolvePressed()
	local f = function()
		World.playerPawn:EnterSolveGame(TerminalScreen.Active)
	end
	TerminalScreen.HideUI(f)
end

function TerminalScreen.HackPressed()
	local f = function()
		World.playerPawn:EnterHackGame(TerminalScreen.Active)
	end
	TerminalScreen.HideUI(f)
end

function TerminalScreen.ReturnPressed()
	local f = function()
		World.playerPawn:LeaveTerminal()
	end
	TerminalScreen.HideUI(f)
end

function TerminalScreen.ShowUI(self)

	assert(#self.solveGlyphs > 0)

	TerminalScreen.SetButtonDifficulty(TerminalScreen.Widgets.Solve, #self.solveGlyphs)
	TerminalScreen.SetButtonDifficulty(TerminalScreen.Widgets.Hack, self.hackDifficulty)
	
	TerminalScreen.Widgets.Solve:BlendTo({1,1,1,1}, 0.2)
	TerminalScreen.Widgets.Hack:BlendTo({1,1,1,1}, 0.2)
	TerminalScreen.Widgets.Return:BlendTo({1,1,1,1}, 0.2)
	
end

function TerminalScreen.HideUI(callback)

	TerminalScreen.Widgets.Solve:BlendTo({1,1,1,0}, 0.2)
	TerminalScreen.Widgets.Hack:BlendTo({1,1,1,0}, 0.2)
	TerminalScreen.Widgets.Return:BlendTo({1,1,1,0}, 0.2)
			
	if (callback) then
		World.gameTimers:Add(callback, 0.2)
	end

end

function TerminalScreen.DoHackGame(self)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
		
	local entity = TerminalScreen.Active
		
	if (TerminalScreen.Skip) then
		TerminalScreen.GameComplete(entity, "hack", "w")
	else
		local f = function ()
			UI:BlendTo({0,0,0,0}, 0.3)
			ReflexGame:InitGame(self.hackDifficulty)
			ReflexGame:ShowBoard(true)
			
			local f = function(result)
				if (result ~= "w") then
					-- failed
					entity.failcount = entity.failcount + 1
				end
				TerminalScreen.GameComplete(entity, "hack", result)
			end
				
			ReflexGame:StartGame(self.hackActions, f)
		end
		
		World.globalTimers:Add(f, 0.3)
	end
end

function TerminalScreen.DoSolveGame(self)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
		
	local entity = TerminalScreen.Active
		
	if (true) then
		TerminalScreen.GameComplete(entity, "solve", "w")
	else
		local f = function ()
			UI:BlendTo({0,0,0,0}, 0.3)
			ReflexGame:InitGame(#self.solveGlpyhs)
			ReflexGame:ShowBoard(true)
			
			local f = function ()
				local f = function(result)
					TerminalScreen.GameComplete(entity, "solve", result)
				end
				Abducted.entity.eatInput = false
				ReflexGame:StartGame(self.solveActions, f)
			end
			
			World.globalTimers:Add(f, 0.3)
		end
		
		World.globalTimers:Add(f, 0.3)
	end
end

function TerminalScreen.GameComplete(self, mode, result)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	local f = function()
		UI:BlendTo({0,0,0,0}, 0.3)
        ReflexGame:ShowBoard(false)
        ReflexGame:ResetGame()
        MemoryGame:ShowBoard(false)
        MemoryGame:ResetGame()
		collectgarbage()
	end
	World.globalTimers:Add(f, 0.3)
	
	if (mode == "hack") then
		World.playerPawn:LeaveHackGame(self, result == "w")
	else
		World.playerPawn:LeaveSolveGame(self, result == "w")
	end
end

function TerminalScreen.PostHackEvents(self, won)

	if (won) then
		if (self.keys.hack_success) then
			World.PostEvent(self.keys.hack_success)
		end
	else
		if (self.keys.hack_fail) then
			World.PostEvent(self.keys.hack_fail)
		end
	end
		
	if (won) then
		if (self.keys.success) then
			World.PostEvent(self.keys.success)
		end
	else
		if (self.keys.fail) then
			World.PostEvent(self.keys.fail)
		end
	end

end

function TerminalScreen.PostSolveEvents(self, won)

	if (won) then
		if (self.keys.solve_success) then
			World.PostEvent(self.keys.solve_success)
		end
	else
		if (self.keys.solve_fail) then
			World.PostEvent(self.keys.solve_fail)
		end
	end

	if (won) then
		if (self.keys.success) then
			World.PostEvent(self.keys.success)
		end
	else
		if (self.keys.fail) then
			World.PostEvent(self.keys.fail)
		end
	end

end

function TerminalScreen.StaticInit()

	local typeface = World.Load("UI/TerminalScreenButtons_TF")
	
	TerminalScreen.gfx = {}
	TerminalScreen.gfx.Difficulty = {
		World.Load("UI/puzzle_level1_M"),
		World.Load("UI/puzzle_level2_M"),
		World.Load("UI/puzzle_level3_M"),
		World.Load("UI/puzzle_level4_M")
	}
	TerminalScreen.gfx.Button = World.Load("UI/puzzle_button_background1_M")
	TerminalScreen.gfx.ButtonPressed = World.Load("UI/puzzle_button_background_pressed1_M")
	
	-- Create buttons
	local rect = {
		0,
		0,
		TerminalScreen.ButtonSize[1] * UI.identityScale[1] * UI.screenWidth,
		TerminalScreen.ButtonSize[1] * UI.identityScale[1] * UI.screenWidth -- make it square
	}
	
	local w = UIPushButton:Create(
		rect,
		{
			enabled = UI.gfx.AnimatedGlpyh,
			pressed = UI.gfx.AnimatedGlpyhPressed
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed = TerminalScreen.GlyphPressed
		},
		nil,
		UI.widgets.interactive.Root
	)
	
	w:SetHAlign(kHorizontalAlign_Center)
	w:SetVAlign(kVerticalAlign_Center)
	w:ScaleTo({0,0}, {0,0})
	TerminalScreen.Widgets.Glyph = w
		
	local bounds = 0.025
	
	TerminalScreen.ScreenBounds = {
		bounds * UI.screenWidth,
		bounds * UI.screenHeight,
		UI.screenWidth - (bounds * UI.screenWidth * 2),
		UI.screenHeight - (bounds * UI.screenHeight * 2)
	}
	
	local solvePos = nil
	local hackPos = nil
	
	if (UI.systemScreen.aspect == "16x9") then
		solvePos = {
			677,
			206
		}
		hackPos = {
			587,
			499
		}
	elseif (UI.systemScreen.aspect == "4x3") then
		solvePos = {
			543,
			278
		}
		hackPos = {
			474,
			495
		}
	else
		solvePos = {
			523,
			216
		}
		hackPos = {
			447,
			428
		}
	end
	
	TerminalScreen.Widgets.Solve = TerminalScreen.CreateButton(
		solvePos,
		"TERMINAL_SOLVE",
		typeface,
		{ pressed = TerminalScreen.gfx.ButtonPressed, enabled = TerminalScreen.gfx.Button },
		TerminalScreen.SolvePressed
	)
	
	TerminalScreen.Widgets.Hack = TerminalScreen.CreateButton(
		hackPos,
		"TERMINAL_HACK",
		typeface,
		{ pressed = TerminalScreen.gfx.ButtonPressed, enabled = TerminalScreen.gfx.Button },
		TerminalScreen.HackPressed
	)
	
	TerminalScreen.Widgets.Return = TerminalScreen.CreateReturnArrow(TerminalScreen.ReturnPressed)
	
end

function TerminalScreen.CreateButton(center, text, typeface, gfx, handler)

	local bkgSize = { 315*0.8*UI.identityScale[1], 183*0.8*UI.identityScale[2] }
	local inset = { 53*0.8*UI.identityScale[1], 47*0.8*UI.identityScale[2] }
	local iconSize = (32*0.8*UI.identityScale[1])+8
	
	local content = {bkgSize[1]-(inset[1]*2), bkgSize[2]-(inset[2]*2)-iconSize}
	
	text = StringTable.Get(text)

	local w = UIPushButton:Create(
		{0, 0, 8, 8},
		{
			enabled = gfx.enabled,
			pressed = gfx.pressed
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed=handler
		},
		{
			highlight = {on={0,0,0,0}, off = {0,0,0,0}, overbright = {1,1,1,1}, time = 0.1, overbrightTime = 0.1},
			label = {typeface=typeface}
		},
		UI.widgets.world.Root
	)
	
	UI:SetLabelText(w.label, text)
	local r, d = UI:SizeLabelToContents(w.label)
	
	d[3] = d[3] - d[1]
	d[4] = d[4] - d[2]
	
	-- expand button rect around text
	r[1] = 0
	r[2] = 0
	
	local z = d[3]/content[1]
	bkgSize[1] = bkgSize[1] * z
	inset[1] = inset[1] * z
		
	r[3] = bkgSize[1]
	
	z = d[4]/content[2]
	bkgSize[2] = bkgSize[2] * z
	inset[2] = inset[2] * z
		
	r[4] = bkgSize[2]
		
	w:SetRect(r)
	UI:MoveWidgetByCenter(w, center[1], center[2])
	UI:MoveLabelNoPadd(w.label, nil, inset[2])
	UI:HCenterLabel(w.label, r)
	
	local icon = UI:CreateWidget("MatWidget", {rect={0,0,8,8}})
	icon:SetBlendWithParent(true)
	w.icon = icon
	w.iconPos = { r[3] / 2, r[4] - inset[2] - 4 }
	w:AddChild(icon)
	
	w:BlendTo({1,1,1,0}, 0)
	return w
end

function TerminalScreen.CreateReturnArrow(handler)

	local w = UIPushButton:Create(
		{0, 0, 125*UI.identityScale[1], 62.5*UI.identityScale[2]},
		{
			enabled = World.Load("UI/return_arrow_M"),
			pressed = World.Load("UI/return_arrow_pressed_M")
		},
		{
			pressed = UI.sfx.Command
		},
		{
			pressed=handler
		},
		{
			highlight = {on={0,0,0,0}, off = {0,0,0,0}, overbright = {1,1,1,1}, time = 0.1, overbrightTime = 0.1},
		},
		UI.widgets.interactive.Root
	)
	w:BlendTo({1,1,1,0}, 0)
	
	return w

end

function TerminalScreen.SetButtonDifficulty(button, difficulty)
	
	difficulty = Clamp(difficulty, 1, 4)
	
	local material = TerminalScreen.gfx.Difficulty[difficulty]
	local d = material:Dimensions()
	d[1] = d[1] * 0.8
	d[2] = d[2] * 0.8
	
	button.icon:SetMaterial(material)
	button.icon:SetRect({0,0,d[1],d[2]})
	UI:MoveWidgetByCenter(button.icon, button.iconPos[1], button.iconPos[2])
	
end

function TerminalScreen.SaveState(self)
	local state = {
		size = self.size,
		downgraded = tostring(self.downgraded),
		failcount = tostring(self.failcount),
		success = tostring(self.success),
		enabled = tostring(self.enabled)
	}
		
	return state
end

function TerminalScreen.LoadState(self, state)

	self.activated = false
	self.popup = false
	self.size = state.size
	self.downgraded = state.downgraded == "true"
	self.failcount = tonumber(self.failcount)
	self.success = state.success == "true"
	self.enabled = state.enabled == "true"
	self.state = "none"
	
	if (self.size == "big") then
		self.model:BlendImmediate("grown")
	else
		self.model:BlendImmediate("idle")
	end

end


info_terminal = TerminalScreen
