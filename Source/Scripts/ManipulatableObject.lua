-- ManipulatableObject.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ManipulatableObject = Entity:New()
ManipulatableObject.Objects = LL_New()
ManipulatableObject.MaxManipulateDistancePct = 1/5 -- max distance to target center
ManipulatableObject.AmbiguousManipulateDistancePct = ManipulatableObject.MaxManipulateDistancePct * 0.25

function ManipulatableObject.Spawn(self)
	COutLine(kC_Debug, "Manipulatable:Spawn(%s)", StringForString(self.keys.model, "<NULL>"))
	Entity.Spawn(self)
	
	self:SetClassBits(kEntityClass_Monster)
	 if (BoolForString(self.keys.cast_shadows, false)) then
		self:SetLightingFlags(kObjectLightingFlag_CastShadows)
	end
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)
	
	-- setup auto-face smoothing
	local angleVertex = self:Angles()
	angleVertex.mass = 5
	angleVertex.drag[1] = 7
	angleVertex.drag[2] = 7
	angleVertex.friction = 0.5
	self:SetAngles(angleVertex)
	
	local spring = self:AngleSpring()
	spring.elasticity = 200
	self:SetAngleSpring(spring)
	
	self.model = LoadSkModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.vision = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.vision) -- manipulate vision model
	self.model.vision:BlendTo({1,1,1,0}, 0)
	self:SetFacing(NumberForString(self.keys.angle, 0))
	
	local scale = Vec3ForString(self.keys.scale, {1, 1, 1})
	self.model.dm:ScaleTo(scale, 0)
	self.model.vision:ScaleTo(scale, 0)
	
	local mins = Vec3ForString(self.keys.mins, {-24, -24, 0})
	local maxs = Vec3ForString(self.keys.maxs, { 24,  24, 128})
	
	if (BoolForString(self.keys.autoscalebounds, false)) then
		mins = VecMul(mins, scale)
		maxs = VecMul(maxs, scale)
	end
	
	self:SetMins( mins)
	self:SetMaxs(maxs)
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	self.model.vision:SetBounds(self:Mins(), self:Maxs())
	self.manipulateShift = Vec3ForString(self.keys.manipulate_shift, {0,0,0})
	self.shakeCamera = StringForString(self.keys.shake_camera, "false")
	
	MakeAnimatable(self)
	self:SetOccupantType(kOccupantType_BBox)
	
	if (self.keys.damage_bone) then
		local boneIdx = self.model.dm:FindBone(self.keys.damage_bone)
		if (boneIdx < 0) then
			error(string.format("ManipulatableObject: bone named %s not found", self.keys.damage_bone))
		end
		local size = Vec3ForString(self.keys.damage_bone_size, {16, 16, 16})
		size = VecScale(size, 0.5)
		self:SetTouchBone(self.model.dm, boneIdx)
		self:SetTouchBoneBounds(VecNeg(size), size)
		self:SetTouchClassBits(bit.bor(kEntityClass_Player, kEntityClass_Monster))
		self.canDamage = true
	end
	
	self.keepAttacking = BoolForString(self.keys.keep_attacking, false)
	self.cameraFocus = BoolForString(self.keys.camera_focus, false)
	self.autoFace = BoolForString(self.keys.auto_face, false)
	
	self.skillRequired = NumberForString(self.keys.required_skill, 0)
	self.manipulateWindow = NumberForString(self.keys.manipulate_window, 5)
	self.manipulateDamage = BoolForString(self.keys.manipulate_damage, false)
	
	self.enabled = false
	self.canAttack = false
	self.enableManipulateShimmer = false
	self.didManipulateShimmer = false
	self.swipePos = {0, 0, 0}
	
	self:Show(BoolForString(self.keys.visible, true))
	
	if (not self.left) then
		self.left = {0, 1, 0}
	end
	
	if (not self.lookScale) then
		self.lookScale = 20
	end
		
	self:LoadAndSwapManipulateMaterials()
	self:LoadSounds()
	
	if (BoolForString(self.keys.enabled, false)) then
		self:Idle()
	else
		self:Dormant()
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

function ManipulatableObject.PostSpawn(self)

	if (self.keys.manipulate_target) then
		local x = World.FindEntityTargets(self.keys.manipulate_target)
		if (x) then
			self.manipulateTarget = x[1]
		end
	end
	
	if (self.manipulateTarget == nil) then
		self.manipulateTarget = self
	end
	
	if (self.keys.manipulate_damage_base_name) then
		if (self.manipulateDamage) then
			self.manipulateBrushes = {
				up = World.FindEntityTargets(self.keys.manipulate_damage_base_name.."_up"),
				down = World.FindEntityTargets(self.keys.manipulate_damage_base_name.."_down"),
				left = World.FindEntityTargets(self.keys.manipulate_damage_base_name.."_left"),
				right = World.FindEntityTargets(self.keys.manipulate_damage_base_name.."_right")
			}
			for k,v in pairs(self.manipulateBrushes) do
				local x = self.manipulateBrushes[k]
				if (x) then
					self.manipulateBrushes[k] = x[1]
				end
			end
		end
	end

end

function ManipulatableObject.ManipulateShimmer(self)
	self.enableManipulateShimmer = true
end

function ManipulatableObject.DoManipulateShimmer(self)
	if (not self.enableManipulateShimmer) then
		self.didManipulateShimmer = true
	end
	
	if (self.didManipulateShimmer) then
		return
	end
	
	self.didManipulateShimmer = true
	self.model.vision:BlendTo(self:SelectColor(), 0.5)
	local f = function()
		self.model.vision:BlendTo({1,1,1,0}, 0.5)
	end
	self.manipulateShimmerTimer = World.gameTimers:Add(f, 0.5)
end

function ManipulatableObject.AddToManipulateList(self)
	if (PlayerSkills:ManipulateUnlocked()) then
		if (self.listItem == nil) then
			self.listItem = LL_Append(ManipulatableObject.Objects, {entity=self})
		end
		self:DoManipulateShimmer()
	end
end

function ManipulatableObject.RemoveFromManipulateList(self)
	if (self.listItem) then
		LL_Remove(ManipulatableObject.Objects, self.listItem)
		self.listItem = nil
	end
end

function ManipulatableObject.LoadAndSwapManipulateMaterials(self)

	local materials = self.model.vision:MaterialList()
	if (materials == nil) then
		return
	end
	
	for k,v in pairs(materials) do
		local x = v:Name().."_manipulate"
		local z = World.Load(x)
		self.model.vision:ReplaceMaterial(v, z)
	end

end

function ManipulatableObject.LoadSounds(self)

	self.sounds = {}
	
	if (self.keys.dormant_sound) then
		self.sounds.Dormant = World.LoadSound(self.keys.dormant_sound)
		self.sounds.Dormant:SetLoop(true)
	end
	
	if (self.keys.sleep_sound) then
		self.sounds.Sleep = World.LoadSound(self.keys.sleep_sound)
	end
	
	if (self.keys.awaken_sound) then
		self.sounds.Awaken = World.LoadSound(self.keys.awaken_sound)
	end
	
	if (self.keys.idle_sound) then
		self.sounds.Idle = World.LoadSound(self.keys.idle_sound)
		self.sounds.Idle:SetLoop(true)
	end
	
	if (self.keys.attack_sound) then
		self.sounds.Attack = World.LoadSound(self.keys.attack_sound)
	end
	
	if (self.keys.hit_sound) then
		self.sounds.Hit = World.LoadSound(self.keys.hit_sound)
	end
	
	if (self.keys.manipulate_sound) then
		self.sounds.Manipulate = World.LoadSound(self.keys.manipulate_sound)
		if (self.keys.loop_manipulate_sound) then
			self.sounds.Manipulate:SetLoop(true)
		end
	end
	
	if (self.keys.manipulate_endstop_sound) then
		self.sounds.ManipulateEnd = World.LoadSound(self.keys.manipulate_endstop_sound)
	end
	
	if (self.keys.reset_sound) then
		if (self.keys.reset_sound == self.keys.manipulate_sound) then
			-- don't make 2 emitters
			self.sounds.Reset = self.sounds.Manipulate
		else
			self.sounds.Reset = World.LoadSound(self.keys.reset_sound)
			if (self.keys.loop_manipulate_sound) then
				self.sounds.Reset:SetLoop(true)
			end
		end
	end
	
	if (self.keys.death_sound) then
		self.sounds.Death = World.LoadSound(self.keys.death_sound)
	end
	
	local maxDistance = NumberForString(self.keys.sound_max_distance, "400")
	
	for k,v in pairs(self.sounds) do
		v:SetMaxDistance(maxDistance)
		v:SetRefDistance(maxDistance/2)
		self:AttachSound(v)
	end

end

function ManipulatableObject.Show(self, show)
	self.visible = show
	self.model.dm:SetVisible(show)
	self.model.vision:SetVisible(show)
end

function ManipulatableObject.OnEvent(self, cmd, args)
	COutLineEvent("ManipulatableObject", self.keys.targetname, cmd, args)
	
	if (cmd == "activate") then
		self:ManipulateShimmer()
		self:Awaken()
		return true
	elseif(cmd == "deactivate") then
		self:Sleep()
		return true
	elseif (cmd == "attack") then
		self.attackArgs = args
		self:Attack()
		return true
	elseif (cmd == "idle") then
		self:Idle()
		return true
	elseif (cmd == "idle_if_active") then
		if (self.enabled) then
			self:Idle()
		end
		return true
	elseif (cmd == "show") then
		self:Show(true)
		return true
	elseif (cmd == "hide") then
		self:Show(false)
		return true
	elseif (cmd == "manipulate") then
		local x = Tokenize(args)
		if ((x[1] == "left") or (x[1] == "right") or (x[1] == "up") or (x[1] == "down")) then
			self:Manipulate(x[1], nil, x[2] ~= "permanent")
		else
			self:CustomManipulate(x[1])
		end
		return true
	elseif (cmd == "play") then
		self:CustomAnimation(args)
		return true
	end
	
	return false
	
end

function ManipulatableObject.OnDamage(self, targets)

	local killMsg = nil
	
	if (self.killedMessages) then
		killMsg = self.killedMessages[IntRand(1, #self.killedMessages)]
	end

	for k,v in pairs(targets) do
		if ((not v.dead) and v.Kill) then
			v:Kill(self, killMsg)
		end
	end

end

function ManipulatableObject.Dormant(self)
	COutLine(kC_Debug, "ManipulatableObject.Dormant")
	
	self.awake = false
	
	self.think = nil
	
	if (BoolForString(self.keys.idle_while_dormant, false)) then
		self:PlayAnim("idle", self.model)
		if (self.sounds.Idle) then
			self.sounds.Idle:Play(kSoundChannel_FX, 0)
		end
	else
		self:PlayAnim("dormant", self.model)
		if (self.sounds.Dormant) then
			self.sounds.Dormant:Play(kSoundChannel_FX, 0)
		end
		if (self.sounds.Idle) then
			self.sounds.Idle:FadeOutAndStop(1)
		end
	end
end

function ManipulatableObject.Dormant2(self)
	COutLine(kC_Debug, "ManipulatableObject.Dormant2")
	
	self.awake = false
	
	self.think = nil
	self.didManipulateShimmer = false -- let us trigger it again
	
	if (BoolForString(self.keys.idle_while_dormant, false)) then
		self:PlayAnim("idle", self.model)
		if (self.sounds.Idle) then
			self.sounds.Idle:Play(kSoundChannel_FX, 0)
		end
	else
		if (self:PlayAnim("dormant2", self.model) == nil) then
			self:PlayAnim("dormant", self.model)
		end
		if (self.sounds.Dormant) then
			self.sounds.Dormant:Play(kSoundChannel_FX, 0)
		end
		if (self.sounds.Idle) then
			self.sounds.Idle:FadeOutAndStop(1)
		end
	end
end

function ManipulatableObject.Sleep(self)
	COutLine(kC_Debug, "ManipulatableObject.Sleep")
	
	self.think = nil
	
	if (not self.awake) then
		return
	end
	
	self.awake = false
	self.enabled = false
	self.canAttack = false
	
	self:RemoveFromManipulateList()
	
	self:SetAutoFace(nil)
	World.viewController:RemoveLookTarget(self.manipulateTarget)
	
	local blend = self:PlayAnim("sleep", self.model)
	if (self.sounds.Sleep) then
		self.sounds.Sleep:Play(kSoundChannel_FX, 0)
	end
	if (self.sounds.Idle) then
		self.sounds.Idle:FadeOutAndStop(1)
	end
		
	if (blend) then
		blend.Seq(ManipulatableObject.Dormant2)
	else
		self:Dormant2()
	end
end

function ManipulatableObject.Awaken(self)
	COutLine(kC_Debug, "ManipulatableObject.Awaken")
	self.think = nil
	self.awake = true
	self.canAttack = true
	
	if (self.autoFace) then
		self:SetAutoFace(World.playerPawn)
	end
	if (self.cameraFocus) then
		local fov = NumberForString(self.keys.camera_focus_fov, 10)
		if (fov <= 0) then
			fov = nil
		end
		World.viewController:AddLookTarget(self.manipulateTarget, self.manipulateShift, fov)
	end
	
	if (not BoolForString(self.keys.idle_while_dormant, false)) then
		local blend = self:PlayAnim("awaken", self.model)
		if (self.sounds.Dormant) then
			self.sounds.Dormant:FadeOutAndStop(1)
		end
		if (self.sounds.Idle) then
			self.sounds.Idle:FadeOutAndStop(1)
		end
		if (blend) then
			blend.Seq(ManipulatableObject.Idle)
			if (self.sounds.Awaken) then
				self.sounds.Awaken:Play(kSoundChannel_FX, 0)
			end
		else
			self:Idle()
		end
	else
		self:Idle()
	end
end

function ManipulatableObject.Idle(self)
	COutLine(kC_Debug, "ManipulatableObject.Idle")
	self.think = nil
	self.awake = true
	self.canAttack = true
	self:PlayAnim("idle", self.model, false)
	self:EnableTouch(false)
	
	if (self.sounds.Idle) then
		self.sounds.Idle:Play(kSoundChannel_FX, 0)
	end
	if (self.sounds.Dormant) then
		self.sounds.Dormant:FadeOutAndStop(1)
	end
	
	if (self.autoFace) then
		self:SetAutoFace(World.playerPawn)
	end
	
	self:AddToManipulateList()
		
	if (not self.enabled) then
		self.enabled = true
		if (Game.entity.manipulate) then -- show ourselves
			ManipulatableObject.NotifyManipulate(true)
		end
	end
end

function ManipulatableObject.Attack(self)
	if (not (self.canAttack and self.canDamage)) then
		COutLine(kC_Debug, "ManipulatableObject.Attack(ignored): cannot attack right now.")
		return
	end
	
	COutLine(kC_Debug, "ManipulatableObject.Attack")
		
	self.think = nil
	self.hitPlayer = false
	self.canAttack = false
	self.nextAttackTime = nil
	local args = Tokenize(self.attackArgs)
	
	local blend = self:PlayAnim(args[1], self.model)
	if (blend) then
		blend.Seq(ManipulatableObject.AttackFinish)
		if (self.sounds.Attack) then
			self.sounds.Attack:Play(kSoundChannel_FX, 0)
		end
		self:EnableTouch(true)
		
		if (self.keys.on_attack_begin) then
			World.PostEvent(self.keys.on_attack_begin)
		end
		
		local min = nil
		local max = nil
		
		if (args[2]) then
			min = tonumber(args[2])
		end
		
		if (args[3]) then
			max = tonumber(args[3])
		end
		
		if (min) then
			if (max) then
				self.nextAttackTime = {min, max}
			else
				self.nextAttackTime = {min, min}
			end
		end
		
	end
end

function ManipulatableObject.AttackFinish(self)
	COutLine(kC_Debug, "ManipulatableObject.AttackFinish")
	
	self:Idle()
		
	if (self.keys.on_attack_end) then
		World.PostEvent(self.keys.on_attack_end)
	end
	
	if (self.hitPlayer and (not self.keepAttacking)) then
		self.nextAttackTime = nil
	end
	
	if (self.nextAttackTime) then
		self.think = ManipulatableObject.Attack
		local t = FloatRand(self.nextAttackTime[1], self.nextAttackTime[2])
		COutLine(kC_Debug, "ManipulatableObject: attacking again in %f seconds", t)
		self:SetNextThink(t)
		self.nextAttackTime = nil
	end
end

function ManipulatableObject.NotifyManipulate(enabled)

	local time = 0.5
	local hidden = {1,1,1,0}
	
	if (enabled) then
		time = 0.15
	end
	
	ManipulatableObjectUI:Notify(enabled)
	
	local f = function (x)
		local rgba
		if (enabled) then
			rgba = x.entity:SelectColor()
		else
			rgba = hidden
		end
		
		x.entity.model.vision:BlendTo(rgba, time)
		
		if (x.entity.skillRequired <= (PlayerSkills:ManipulateSkillLevel()+1)) then
			ManipulatableObjectUI:NotifyObject(x.entity, enabled, time)
		end
		
		if (x.entity.manipulateShimmerTimer) then
			x.entity.manipulateShimmerTimer:Clean()
			x.entity.manipulateShimmerTimer = nil
		end
	end
	
	LL_Do(ManipulatableObject.Objects, f)
	
	ManipulatableObjectUI:NotifySingle()

end

function ManipulatableObject.SelectColor(self)
		
	if (self.skillRequired > PlayerSkills:ManipulateSkillLevel()+1) then
		return {1,0,0,1}
	elseif (self.skillRequired <= PlayerSkills:ManipulateSkillLevel()) then
		return {0,1,0,1}
	end
	
	return {1,1,0,1}
end

function ManipulatableObject.FindSwipeTarget(g)

	-- figure out line normal
	assert(g.id == kIG_Line)
	
	local kMaxDist = UI.systemScreen.diagonal * ManipulatableObject.MaxManipulateDistancePct
		
	local normal = { -g.args[2], g.args[1] } -- orthogonal to line
	local lineDist = (normal[1]*g.start[1]) + (normal[2]*g.start[2])
	local lineSegDist = {
		g.args[1]*g.start[1] + g.args[2]*g.start[2],
		g.args[1]*g.finish[1] + g.args[2]*g.finish[2]
	}
	
	local lineStart = g.start
	local lineEnd = g.finish
	
	if (lineSegDist[1] > lineSegDist[2]) then
		lineSegDist[1], lineSegDist[2] = lineSegDist[2], lineSegDist[1]
		lineStart, lineEnd = lineEnd, lineStart
	end

	local cameraFwd = World.CameraFwd()
	local cameraPos = World.CameraPos()
	
	-- select the object who's line is closes to the manipulate point
	-- if any object falls in the ambiguous "area" (i.e. aren't that close)
	-- then sort these by distance to the camera

	local best = nil
	local bestDist = nil
	local bestWorldDist = nil
	local bestScreenPos = nil
	
	local x = LL_Head(ManipulatableObject.Objects)
	
	while (x) do
	
		local targetname = x.entity.keys.targetname
		
		if (x.entity.visible and (x.entity.skillRequired <= (PlayerSkills:ManipulateSkillLevel()+1))) then
			local world = VecAdd(x.entity.manipulateShift, x.entity.manipulateTarget:WorldPos())
			local screen,r = World.Project(world)
			local dx, dy, dd
			
			if (r) then
				
				-- must be on screen
				if ((screen[1] >= 0) and (screen[1] < UI.systemScreen.width) and
					(screen[2] >= 0) and (screen[2] < UI.systemScreen.height)) then
					
					
					-- capsule distance
					local segPos = g.args[1]*screen[1] + g.args[2]*screen[2]
					if (segPos < lineSegDist[1]) then
						-- distance to endpoints check:
						dx = screen[1] - lineStart[1]
						dy = screen[2] - lineStart[2]
						dd = math.sqrt(dx*dx+dy*dy)
					elseif (segPos > lineSegDist[2]) then
						dx = screen[1] - lineEnd[1]
						dy = screen[2] - lineEnd[2]
						dd = math.sqrt(dx*dx+dy*dy)
					else
						-- orthogonal distance check
						dd = (normal[1]*screen[1]) + (normal[2]*screen[2])
						dd = math.abs(dd - lineDist)
					end
					
					if (dd <= kMaxDist) then
						local worldDist = VecDot(world, cameraFwd)
					
						-- candidate for selection
						
						if (best) then
							dx = screen[1] - bestScreenPos[1]
							dy = screen[2] - bestScreenPos[2]
							local z = math.sqrt(dx*dx+dy+dy)
							if (z <= ManipulatableObject.AmbiguousManipulateDistancePct) then
								-- these points are close together test Z from the camera
								-- instead of screenspace
								if (worldDist < bestWorldDist) then
									best = x.entity
									bestWorldDist = worldDist
									bestDist = dd
									bestScreenPos = screen
								end
							elseif (dd < bestDist) then
								best = x.entity
								bestWorldDist = worldDist
								bestDist = dd
								bestScreenPos = screen
							end
						else
							best = x.entity
							bestWorldDist = worldDist
							bestDist = dd
							bestScreenPos = screen
						end
					end
				end
			end
		end
		
		x = LL_Next(x)
	
	end
	
	return best

end

function ManipulatableObject.ManipulateGesture(g)

	local target = ManipulatableObject.FindSwipeTarget(g)
	
	if (target == nil) then
		return nil
	end
	
	-- figure out which direction to manipulate the target
	local leftRight = target.model:HasState("manipulate_left") or target.model:HasState("manipulate_right")
	local upDown = target.model:HasState("manipulate_up") or target.model:HasState("manipulate_down")
	if (leftRight and upDown) then
		-- ambiguous, select using direction swiped
		if ((g.args[1] > 0.707) or (g.args[1] < -0.707)) then
			return ManipulatableObject.ManipulateLeftRight(target, g)
		else
			return ManipulatableObject.ManipulateUpDown(target, g)
		end
	elseif (leftRight) then
		return ManipulatableObject.ManipulateLeftRight(target, g)
	elseif (upDown) then
		return ManipulatableObject.ManipulateUpDown(target, g)
	end
	
	return false

end

function ManipulatableObject.CanManipulateDir(self, dir)
	return self.model:HasState("manipulate_"..dir)
end

function ManipulatableObject.ManipulateLeftRight(target, g)

	-- may need to flip direction if we are facing towards the object
	local targetFwd = ForwardVecFromAngles(target:WorldAngles())
	local playerFwd = ForwardVecFromAngles(World.playerPawn:WorldAngles())
	local cameraFwd = World.CameraFwd()
		
	local objDir = ManipulatableObject.GetObserverOrientedLeftRight(targetFwd, cameraFwd, g)
	local playerDir = ManipulatableObject.GetObserverOrientedLeftRight(playerFwd, cameraFwd, g)
	
	return target:Manipulate(objDir, playerDir)

end

function ManipulatableObject.GetObserverOrientedLeftRight(targetFwd, observerFwd, g, dir)
	local flip = false
	
	if (VecDot(observerFwd, targetFwd) < 0) then
		flip = true -- facing eachother
		
		if (dir == "left") then
			return "right"
		elseif (dir) then
			return "left"
		end
	end
	
	if (dir) then
		return dir
	end
	
	local sign = g.args[1]
	
	if (flip) then
		sign = -sign
	end
	
	local d
	
	if (sign >= 0) then
		d = "right"
	else
		d = "left"
	end
	
	return d
end

function ManipulatableObject.DoManipulateCommand(target, dir)

	if ((dir == "up") or (dir == "down")) then
		return target:Manipulate(dir, dir)
	end

	local targetFwd = ForwardVecFromAngles(target:WorldAngles())
	local playerFwd = ForwardVecFromAngles(World.playerPawn:WorldAngles())
	local cameraFwd = World.CameraFwd()
	
	local objDir = ManipulatableObject.GetObserverOrientedLeftRight(targetFwd, cameraFwd, nil, dir)
	local playerDir = ManipulatableObject.GetObserverOrientedLeftRight(playerFwd, cameraFwd, nil, dir)
	
	return target:Manipulate(objDir, playerDir)
end

function ManipulatableObject.ManipulateUpDown(target, g)
	local dir
	if (g.args[2] >= 0) then
		dir = "down"
	else
		dir = "up"
	end
	
	return target:Manipulate(dir, dir)
end

function ManipulatableObject.CustomAnimation(self, customAnim) 
		
	if (not self.model:HasState(customAnim)) then
		return false -- this model has no manipulate in that direction
	end
	
	local f = function()
		if (self.awake) then
			self:Idle()
		else
			self:Dormant()
		end
	end
	
	if (self.sounds.Manipulate) then
		self.sounds.Manipulate:FadeOutAndStop(0.1)
	end
	if (self.sounds.Idle) then
		self.sounds.Idle:Play(kSoundChannel_FX, 0)
	end
	
	self.manipulate = nil
	self.canAttack = true
	self.enabled = false
	local blend = self:PlayAnim(customAnim, self.model).Seq(f)
	if (blend) then
		blend.OnTag = function(self, tag)
			World.PostEvent(tag)
		end
	end
	self:SetAutoFace(nil)
	self:EnableTouch(false)
		
	-- no longer manipulatable
	self.model.vision:BlendTo({1,1,1,0}, 0.5)
	self:RemoveFromManipulateList()
	
	return true
end

function ManipulatableObject.CustomManipulate(self, customAnim) 
		
	if (not self.model:HasState(customAnim)) then
		return false -- this model has no manipulate in that direction
	end
	
	local f = function()
		if (self.shakeCamera == "EndOf") then
			self:ShakeCamera()
		end
		if (self.sounds.Manipulate) then
			self.sounds.Manipulate:FadeOutAndStop(0.1)
		end
		
		self:EnableTouch(false)
		
		-- manipulatable again
		self:AddToManipulateList()
		self:Idle()
	end
	
	if (self.shakeCamera == "StartOf") then
		self:ShakeCamera()
	end
	
	if (self.sounds.Manipulate) then
		self.sounds.Manipulate:FadeVolume(1, 0)
		self.sounds.Manipulate:Play(kSoundChannel_FX, 0)
	end
	
	self.manipulate = nil
	self.canAttack = false
	self.enabled = false
	self:PlayAnim(customAnim, self.model).Seq(f)
	self:SetAutoFace(nil)
	
	self:EnableTouch(self.canDamage)
		
	-- no longer manipulatable
	self.model.vision:BlendTo({1,1,1,0}, 0.5)
	self:RemoveFromManipulateList()
	
	return true
end

function ManipulatableObject.Manipulate(self, objDir, playerDir, canReset)

	local state = "manipulate_"..objDir
	local idle  = "manipulate_"..objDir.."_idle"
	local ret   = "manipulate_"..objDir.."_return"
	
	if (not self.model:HasState(state)) then
		return false -- this model has no manipulate in that direction
	end
	
	local f = function()
		if (self.shakeCamera == "EndOf") then
			self:ShakeCamera()
		end
		if (self.sounds.ManipulateEnd) then
			if (self.sounds.Manipulate) then
				self.sounds.Manipulate:Stop()
			end
			self.sounds.ManipulateEnd:Play(kSoundChannel_FX, 0)
		elseif (self.sounds.Manipulate) then
			self.sounds.Manipulate:FadeOutAndStop(0.1)
		end
		
		self:DoManipulateDamage(objDir)
		self:EnableTouch(false)
	end
	
	if (self.shakeCamera == "StartOf") then
		self:ShakeCamera()
	end
	
	if (self.sounds.Manipulate) then
		self.sounds.Manipulate:FadeVolume(1, 0)
		self.sounds.Manipulate:Play(kSoundChannel_FX, 0)
	end
	
	self.manipulate = objDir
	self.canAttack = false
	self.enabled = false
	self:PlayAnim(state, self.model).Seq(f).Seq(idle)
	self:SetAutoFace(nil)
	
	self:EnableTouch(self.canDamage)
		
	-- no longer manipulatable
	self.model.vision:BlendTo({1,1,1,0}, 0.5)
	self:RemoveFromManipulateList()
	
	-- how long do we sit here?
--	local alwaysReset = StringForString(self.keys.reset, "auto")=="always"
	local neverReset = StringForString(self.keys.reset, "auto")=="never"
	
	if (((canReset == nil) and (not neverReset) and (self.skillRequired > PlayerSkills:ManipulateSkillLevel())) or (canReset and (not neverReset))) then
		local f = function ()
			COutLine(kC_Debug, "Manipulatable.Reset")
			self.manipulate = nil
			
			if (self.keys.on_reset) then
				World.PostEvent(self.keys.on_reset)
			end
			
			local blend = self:PlayAnim(ret, self.model)
			if (blend) then
				if (self.sounds.Reset) then
					self.sounds.Reset:FadeVolume(1, 0)
					self.sounds.Reset:Play(kSoundChannel_FX, 0)
				end
				
				local f = function()
					COutLine(kC_Debug, "Manipulatable.PostReset")
					if (self.keys.post_reset) then
						World.PostEvent(self.keys.post_reset)
					end
					if (self.sounds.Reset) then
						if (self.sounds.ManipulateEnd) then
							if (self.sounds.Reset) then
								self.sounds.Reset:Stop()
							end
							self.sounds.ManipulateEnd:Play(kSoundChannel_FX, 0)
						elseif (self.sounds.Reset) then
							self.sounds.Reset:FadeOutAndStop(0.1)
						end
					end
				end
				blend.Seq(f).Seq(ManipulatableObject.Idle)
			else
				COutLine(kC_Debug, "Manipulatable.PostReset")
				if (self.keys.post_reset) then
					World.PostEvent(self.keys.post_reset)
				end
				self:Idle()
			end
			
			-- manipulatable again
			self:AddToManipulateList()
		end
		World.gameTimers:Add(f, self.manipulateWindow)
	else
		-- object is not going to reset
		self.saveManipulateDir = objDir
		World.viewController:RemoveLookTarget(self.manipulateTarget)
		
		if (self.keys.on_permanent_manipulated) then
			World.PostEvent(self.keys.on_permanent_manipulated)
		end
	end
	
	-- tell the player they moved us
	if (playerDir) then -- this indicates it was done by the player and not a scripted event
		World.playerPawn:ManipulateDir(playerDir)
		self:LookInDir(objDir)
		if (self.keys.on_manipulated) then
			World.PostEvent(self.keys.on_manipulated)
		end
		
		Abducted.entity:EndManipulate()
		HUD:RechargeManipulate()
	end
	
	return true

end

function ManipulatableObject.ShakeCamera(self)
	World.viewController:SetCameraSway(
		0.1,
		0.1,
		0.1,
		-4,
		4,
		0.1,
		0.25,
		{1, 1}
	)
end

function ManipulatableObject.LookInDir(self, dir)
	local vec
	if (dir == "up") then
		vec = {0, 0, 1}
	elseif (dir == "down") then
		vec = {0, 0, -1}
	elseif (dir == "left") then
		vec = VecCopy(self.left)
	else
		vec = VecNeg(self.left)
	end
	
	local q = QuatFromAngles(self:Angles().pos)
	QuatRotateVec(q, vec)
	
	vec = VecScale(vec, self.lookScale)
	
	local cameraTarget = World.playerPawn:CameraPos()
	local target = VecSub(self.manipulateTarget:WorldPos(), cameraTarget)
	target[2] = 0
	target = VecAdd(cameraTarget, target) -- our position, equal to camera height
	target = VecAdd(target, vec)
	
	World.viewController:BlendToLookTarget(
		target, -- target position to "look" at
		0.4, -- in time
		1, -- out time
		0, -- hold time
		1,  -- max weight (how much towards the target we look, 1 = all the way)
		1, -- smooth factor for in time, these serve to "tighten" or "loosen" the motion
		1 -- smooth factor for out time
	)
	
end

function ManipulatableObject.DoManipulateDamage(self, dir)
	if (self.manipulateBrushes) then
		local t = self.manipulateBrushes[dir]
		if (t) then
			local ents = t:GetTouching()
			if (ents) then
				for k,v in pairs(ents) do
					if (v.Kill) then
						v:Kill(self)
					end
				end
			end
		end
	end
end

function ManipulatableObject.SaveState(self)

	local state = {
		awake = tostring(self.awake),
		visible = tostring(self.visible),
		enableManipulateShimmer = tostring(self.enableManipulateShimmer),
		didManipulateShimmer = tostring(self.didManipulateShimmer)
	}
	
	if (self.saveManipulateDir) then
		state.manipulate = self.saveManipulateDir
	end

	return state
end

function ManipulatableObject.LoadState(self, state)

	self:Show(state.visible == "true")
	self.think = nil
	self.model.vision:BlendTo({1,1,1,0}, 0)
	
	if (self.manipulateShimmerTimer) then
		self.manipulateShimmerTimer:Clean()
		self.manipulateShimmerTimer = nil
	end
	
	self.enableManipulateShimmer = state.enableManipulateShimmer == "true"
	self.didManipulateShimmer = state.didManipulateShimmer == "true"
	
	if (state.awake == "true") then
		self.awake = true
		if (state.manipulate) then
			self.canAttack = false
			self.enabled = false
			self.saveManipulateDir = state.manipulate
			-- no longer manipulatable
			self:RemoveFromManipulateList()
			self.model:BlendImmediate("manipulate_"..state.manipulate.."_idle")
			if (self.sounds.Idle) then
				self.sounds.Idle:Stop()
			end
			if (self.sounds.Dormant) then
				self.sounds.Dormant:Stop()
			end
			if (self.sounds.Sleep) then
				self.sounds.Sleep:Stop()
			end
			if (self.sounds.Manipulate) then
				self.sounds.Manipulate:FadeVolume(1, 0)
				self.sounds.Manipulate:Play(kSoundChannel_FX, 0)
			end
		else
			self.enabled = true
			self.canAttack = true
			self:PlayAnim("idle", self.model)
			self:AddToManipulateList()
			if (self.cameraFocus) then
				local fov = NumberForString(self.keys.camera_focus_fov, 10)
				if (fov <= 0) then
					fov = nil
				end
				World.viewController:AddLookTarget(self.manipulateTarget, self.manipulateShift, fov)
			end
			if (self.sounds.Idle) then
				self.sounds.Idle:Play()
			end
			if (self.sounds.Dormant) then
				self.sounds.Dormant:Stop()
			end
			if (self.sounds.Sleep) then
				self.sounds.Sleep:Stop()
			end
			if (self.sounds.Manipulate) then
				self.sounds.Manipulate:Stop()
			end
		end
	else
		self.awake = false
		self.enabled = false
		self.canAttack = false
		self:RemoveFromManipulateList()
		self.model:BlendImmediate("dormant")
		
		if (self.sounds.Idle) then
			self.sounds.Idle:Stop()
		end
		if (self.sounds.Dormant) then
			self.sounds.Dormant:Play()
		end
		if (self.sounds.Sleep) then
			self.sounds.Sleep:Stop()
		end
		if (self.sounds.Manipulate) then
			self.sounds.Manipulate:Stop()
		end
	end

end

StdManipulatable = ManipulatableObject:New()

function StdManipulatable.Spawn(self)
	ManipulatableObject.Spawn(self)
	self:Link() -- kMoveType_None
end

Tentacle = StdManipulatable:New()

function Tentacle.Spawn(self)
	StdManipulatable.Spawn(self)
	self.killedMessages = { "TENTACLE_KILLED_MESSAGE1" }
end

info_tentacle = Tentacle

Pylon = StdManipulatable:New()

function Pylon.Spawn(self)
	StdManipulatable.Spawn(self)
	self.killedMessages = { "PYLON_KILLED_MESSAGE1" }
end

info_pylon = Pylon
info_custom_manipulatable = StdManipulatable
