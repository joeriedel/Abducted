-- Metadata.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Metadata = Entity:New()
Metadata.kSize = 45
Metadata.List = LL_New()
Metadata.kSmackDistance = 60

function Metadata.Spawn(self)

	if ((self.keys.terminal == nil) or
		(self.keys.terminal == "")) then
		error("Metadata glyph has no terminal!")
	end
	
	self.glyph = NumberForString(self.keys.glyph_num, 1)
	if ((self.glyph < 1) or (self.glyph > 27)) then
		error("Metadata glyph is out of range (1-27 is the valid range))")
	end
	
	self:SetMins({-Metadata.kSize/2, -Metadata.kSize/2, -Metadata.kSize/2})
	self:SetMaxs({Metadata.kSize/2, Metadata.kSize/2, Metadata.kSize/2})
	
	self.model = World.Load("Objects/alien_metadata1")
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	self.explodeSound = World.LoadSound("Audio/AFX_ShieldImpactLight")
	self:AttachSound(self.explodeSound)
	
	self.sprite = World.CreateSpriteBatch(1, 1)
	self.sprite.material = World.Load(string.format("FX/glyph%02d_M", self.glyph))
	self.sprite.dm = self:AttachDrawModel(self.sprite, self.sprite.material)
	self.sprite.dm:SetBounds(self:Mins(), self:Maxs())
	self.sprite.sprite = self.sprite.dm:AllocateSprite()
	self.sprite.dm:SetSpriteData(
		self.sprite.sprite,
		{
			pos = {0, 0, 0},
			size = {Metadata.kSize, Metadata.kSize},
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.sprite.dm:Skin()
	self.sprite.dm:SetVisible(false)
	
	self.explodeSprite = World.CreateSpriteBatch(1, 1)
	self.explodeSprite.material = World.Load("FX/terminalflash1_M")
	self.explodeSprite.dm = self:AttachDrawModel(self.explodeSprite, self.explodeSprite.material)
	self.explodeSprite.dm:SetBounds(self:Mins(), self:Maxs())
	self.explodeSprite.sprite = self.explodeSprite.dm:AllocateSprite()
	self.explodeSprite.dm:SetSpriteData(
		self.explodeSprite.sprite,
		{
			pos = {0, 0, 0},
			size = {Metadata.kSize*4, Metadata.kSize*4},
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.explodeSprite.dm:Skin()
	self.explodeSprite.dm:BlendTo({1,1,1,0}, 0)
	
	self.sparks = World.Load("FX/sparks1")
	self.sparks:SetMaxParticles(50)
	self.sparks.dm = self:AttachDrawModel(self.sparks)
	self.sparks.dm:SetBounds(self:Mins(), self:Maxs())
	
	self.pos = self:WorldPos()
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()
	
	self.opened = false
	self.listItem = LL_Append(Metadata.List, {x=self})
	
	self.think = Metadata.CheckProximity
	self:SetNextThink(1/4)
	
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

function Metadata.PostSpawn(self)

	local terminal = World.FindEntityTargets(self.keys.terminal)
	if (terminal) then
		terminal = terminal[1]
		if (terminal.RegisterGlyph) then
			terminal:RegisterGlyph(self.glyph)
		end
	end

end

function Metadata.CheckProximity(self)

	local v = VecSub(self.pos, VecAdd(World.playerPawn:WorldPos(), World.playerPawn:CameraShift()))
	local d = VecMag(v)

	if (d <= Metadata.kSmackDistance) then
		local f = function()
			self:Explode()
		end
		if (World.playerPawn:SmackMetadata(f)) then
			self.think = nil -- we are gonna smack, don't do this again while waiting for it
		end
	end
end

function Metadata.CheckPulseTarget(self, x, y)
	local p, r = World.Project(self.pos)
	if (not r) then
		return false
	end
	
	p = UI:MapToUI(p)
	
	local dx = p[1]-x
	local dy = p[2]-y
	local dd = math.sqrt(dx*dx+dy*dy)
	if (dd <= UI.screenDiagonal*Discovery.TouchDistance) then
		return true
	end
	
	return false
end

function Metadata.CheckPulseTargets(mx, my)
	local x = LL_Head(Metadata.List)
	while (x) do
		if(x.x:CheckPulseTarget(mx, my)) then
			return x.x
		end
		x = LL_Next(x)
	end
	
	return nil
end

function Metadata.Explode(self)
	self.think = nil
	LL_Remove(Metadata.List, self.listItem)
	self.listItem = nil
	self.explodeSound:Play(kSoundChannel_FX, 0)
	self.explodeSprite.dm:BlendTo({1,1,1,0}, 0.1)
	self.explodeSprite.dm:BlendTo({1,1,1,1}, 0.1)
	self.explodeSprite.dm:SetVisible(true)
	self.sparks:Spawn(50)
	local f = function()
		self.explodeSprite.dm:BlendTo({1,1,1,0}, 0.2)
		self.sprite.dm:SetVisible(true)
		self.model.dm:SetVisible(false)
	end
	
	World.globalTimers:Add(f, 0.1)
	
	HUD:Print(nil, "METADATA_MESSAGE")
	EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", "EVENT_LOG_OPENED_METADATA_GLYPH")
	PlayerSkills:AwardSkillPoints(25)
end

function Metadata.SaveState(self)
	local state = {
		opened = tostring(self.opened)
	}
		
	return state
end

function Metadata.LoadState(self, state)

	self.opened = state.opened == "true"
	
	self.think = nil
	self.listItem = nil
	
	self.explodeSprite.dm:SetVisible(false)
	
	if (self.opened) then
		self.model.dm:SetVisible(false)
		self.sprite.dm:SetVisible(true)
	else
		self.think = Metadata.CheckProximity
		self:SetNextThink(1/4)
		self.model.dm:SetVisible(true)
		self.sprite.dm:SetVisible(false)
		self.listItem = LL_Append(Metadata.List, {x=self})
	end

end

function Metadata.ResetForCheckpoint()
	Metadata.List = LL_New()
end

info_metadata = Metadata
