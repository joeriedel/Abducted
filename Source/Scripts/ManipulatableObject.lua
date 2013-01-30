-- ManipulatableObject.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ManipulatableObject = Entity:New()
ManipulatableObject.Objects = LL_New()
ManipulatableObject.MaxManipulateDistancePct = 1/5 -- max distance to target center

function ManipulatableObject.Spawn(self)
	COutLine(kC_Debug, "Manipulatable:Spawn(%s)", StringForString(self.keys.model, "<NULL>"))
	
	self.model = LoadSkModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.vision = self.model.dm:CreateInstance()
	self:AttachDrawModel(self.model.vision) -- manipulate vision model
	self.model.vision:BlendTo({1,1,1,0}, 0)
	
	MakeAnimatable(self)
	self:SetOccupantType(kOccupantType_BBox)
	
	local angle = NumberForString(self.keys.angle, 0)
	
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	self:SetAngles(angleVertex)
	
	self.skillRequired = NumberForString(self.keys.required_skill, 0)
	self.manipulateWindow = NumberForString(self.keys.manipulate_window, 5)
	
	self.enabled = false
	self.swipePos = {0, 0, 0}
	
	self:LoadAndSwapManipulateMaterials()
	self:LoadSounds()
	self:Dormant()
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

function ManipulatableObject.OnEvent(self, cmd, args)
	if (cmd == "activate") then
		self:Awaken()
	end
end

function ManipulatableObject.Dormant(self)
	self:PlayAnim("dormant", self.model)
	if (self.sounds.Dormant) then
		self.sounds.Dormant:Play(kSoundChannel_FX, 0)
	end
end

function ManipulatableObject.Awaken(self)
	self:PlayAnim("awaken", self.model).Seq(ManipulatableObject.Idle)
	if (self.sounds.Dormant) then
		self.sounds.Dormant:FadeVolume(0, 1)
	end
	if (self.sounds.Awaken) then
		self.sounds.Awaken:Play(kSoundChannel_FX, 0)
	end
end

function ManipulatableObject.Idle(self)
	self:PlayAnim("idle", self.model)
	if (self.sounds.Idle) then
		self.sounds.Idle:Play(kSoundChannel_FX, 0)
	end
	
	if (not self.enabled) then
		self.enabled = true
		self.listItem = LL_Append(ManipulatableObject.Objects, {entity=self})
		if (Game.entity.manipulate) then -- show ourselves
			ManipulatableObject.NotifyManipulate(true)
		end
	end
end

function ManipulatableObject.NotifyManipulate(enabled)

	local time = 0.5
	local hidden = {1,1,1,0}
	
	if (enabled) then
		time = 0.15
	end
	
	local f = function (x)
		local rgba
		if (enabled) then
			rgba = x.entity:SelectColor()
		else
			rgba = hidden
		end
		
		x.entity.model.vision:BlendTo(rgba, time)
	end
	
	LL_Iterate(ManipulatableObject.Objects, f)

end

function ManipulatableObject.SelectColor(self)
		
	if (self.skillRequired > PlayerSkills.Manipulate+1) then
		return {1,0,0,1}
	elseif (self.skillRequired <= PlayerSkills.Manipulate) then
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
		local x = lineSegDist[1]
		lineSegDist[1] = lineSegDist[2]
		lineSegDist[2] = x
		
		x = lineStart
		lineStart = lineEnd
		lineEnd = lineStart
	end

	local cameraFwd = World.CameraFwd()
	local cameraPos = World.CameraPos()

	local best
	local bestWorldDist
	local x = LL_Head(ManipulatableObject.Objects)
	
	while (x) do
	
		local world = x.entity:WorldPos()
		local screen,r = World.Project(world)
		local dx, dy, dd
		
		if (r) then
			
			-- must be on screen
			if ((screen[1] >= 0) and (screen[1] < UI.systemScreen.width) and
			    (screen[2] >= 1) and (screen[2] < UI.systemScreen.height)) then
				
				local keep = false
				
				-- closer than another matching object?
				local worldDist = VecDot(world, cameraFwd)
				if ((bestWorldDist == nil) or (worldDist < bestWorldDist)) then
					-- capsule distance
					
					local segPos = g.args[1]*screen[1] + g.args[2]*screen[2]
					if (segPos < lineSegDist[1]) then
						-- distance to endpoints check:
						dx = screen[1] - lineStart[1]
						dy = screen[2] - lineStart[2]
						dd = math.sqrt(dx*dx + dy*dy)
						if (dd <= kMaxDist) then
							keep = true
						end
					elseif (segPos > lineSegDist[2]) then
						dx = screen[1] - lineEnd[1]
						dy = screen[2] - lineEnd[2]
						dd = math.sqrt(dx*dx + dy*dy)
						if (dd <= kMaxDist) then
							keep = true
						end
					else
						-- orthogonal distance check
						dd = (normal[1]*screen[1]) + (normal[2]*screen[2])
						dd = math.abs(dd - lineDist)
						
						if (dd <= kMaxDist) then
							keep = true
						end
					end
				end
				
				if (keep) then
					bestWorldDist = worldDist
					best = x.entity
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

function ManipulatableObject.ManipulateLeftRight(target, g)

	-- may need to flip direction if we are facing towards the object
	local targetFwd = ForwardVecFromAngles(target:WorldAngles())
	local playerFwd = ForwardVecFromAngles(World.playerPawn:WorldAngles())
	local cameraFwd = World.CameraFwd()
		
	local objDir = ManipulatableObject.GetObserverOrientedLeftRight(targetFwd, cameraFwd, g)
	local playerDir = ManipulatableObject.GetObserverOrientedLeftRight(playerFwd, cameraFwd, g)
	
	return target:Manipulate(objDir, playerDir)

end

function ManipulatableObject.GetObserverOrientedLeftRight(targetFwd, observerFwd, g)
	local flip = false
	
	if (VecDot(observerFwd, targetFwd) < 0) then
		flip = true -- facing eachother
	end
	
	local sign = g.args[1]
	
	if (flip) then
		sign = -sign
	end
	
	local dir
	
	if (sign >= 0) then
		dir = "right"
	else
		dir = "left"
	end
	
	return dir
end

function ManipulatableObject.ManipulateUpDown(target, g)
	local dir
	if (g.args[1] >= 0) then
		dir = "down"
	else
		dir = "up"
	end
	
	return target:Manipulate(dir, dir)
end

function ManipulatableObject.Manipulate(self, objDir, playerDir)

	local state = "manipulate_"..objDir
	local idle  = "manipulate_"..objDir.."_idle"
	local ret   = "manipulate_"..objDir.."_return"
	
	if (not self.model:HasState(state)) then
		return false -- this model has no manipulate in that direction
	end
	
	self.manipulate = objDir
	self:PlayAnim(state, self.model).Seq(idle)
	
	-- no longer manipulatable
	if (self.listItem) then
		self.model.vision:BlendTo({1,1,1,0}, 0.5)
		LL_Remove(ManipulatableObject.Objects, self.listItem)
		self.listItem = nil
	end
	
	-- how long do we sit here?
	if (self.skillRequired > PlayerSkills.Manipulate) then
		local f = function ()
			self.manipulate = nil
			self:PlayAnim(ret, self.model).Seq("idle")
			-- manipulatable again
			self.listItem = LL_Append(ManipulatableObject.Objects, {entity=self})
		end
		World.gameTimers:Add(f, self.manipulateWindow, true)
	end
	
	-- tell the player they moved us
	World.playerPawn:ManipulateDir(playerDir)
	
	return true

end

--[[---------------------------------------------------------------------------
	Tentacles
-----------------------------------------------------------------------------]]

Tentacle = ManipulatableObject:New()

function Tentacle.Spawn(self)
	ManipulatableObject.Spawn(self)
	
	self:SetMins({-24, -24, -48+64})
	self:SetMaxs({ 24,  24,  48+64})
	self.model.dm:SetScale({0.4, 0.4, 0.4})
	self.model.dm:SetAngles({0, -90, 180})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self.model.vision:SetScale({0.4, 0.4, 0.4})
	self.model.vision:SetAngles({0, -90, 180})
--	self.model.vision:SetPos({0, 100, 0})
	self.model.vision:SetBounds(self:Mins(), self:Maxs())
	
	self:Link() -- kMoveType_None
end

info_tentacle = Tentacle