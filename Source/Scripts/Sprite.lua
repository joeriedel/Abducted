-- Sprite.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Sprite = Entity:New()

function Sprite.Spawn(self)

	COutLine(kC_Debug, "Sprite:Spawn")
	Entity.Spawn(self)
	
	local size = Vec2ForString(self.keys.size, "64 64")
	
	self:SetMins({-size[1]*0.5, -size[1]*0.5, -size[2]*0.5})
	self:SetMaxs({ size[1]*0.5,  size[1]*0.5,  size[2]*0.5})
	
	self.sprite = World.CreateSpriteBatch(1, 1)
	self.sprite.material = World.Load(StringForString(self.keys.material, "FX/shieldsprite1_M"))
	self.sprite.dm = self:AttachDrawModel(self.sprite, self.sprite.material)
	self.sprite.dm:SetBounds(self:Mins(), self:Maxs())
	self.sprite.sprite = self.sprite.dm:AllocateSprite()
	self.sprite.dm:SetSpriteData(
		self.sprite.sprite,
		{
			pos = {0,0,0},
			size = {size[1], size[2]},
			rgba = Color4ForString(self.keys.color, "255 255 255 255"),
			rot = NumberForString(self.keys.rotation, 0)
		}
	)
	
	self.visible = BoolForString(self.keys.visible, true)
	if (not self.visible) then
		self.sprite.dm:BlendTo({0,0,0,0}, 0)
	end
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link()
	
end

function Sprite.OnEvent(self, cmd, args)

	COutLineEvent("Sprite", self.keys.targetname, cmd, args)
	
	if (cmd == "fadein") then
		if (not self.visible) then
			if (not args) then
				error("Sprite.OnEvent(fadein) requires argument [time]")
			end
			self:BlendTo({1,1,1,1}, NumberForString(args, 0))
			self.visible = true
		end
	elseif (cmd == "fadeout") then
		if (self.visible) then
			if (not args) then
				error("Sprite.OnEvent(fadeout) requires argument [time]")
			end
			self:BlendTo({0,0,0,0}, NumberForString(args, 0))
			self.visible = false
		end
	end
	
end

info_sprite = Sprite