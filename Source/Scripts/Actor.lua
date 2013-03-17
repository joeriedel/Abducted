-- Actor.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Actor
-----------------------------------------------------------------------------]]

Actor = Entity:New()

function Actor.Spawn(self)

    COutLine(kC_Debug, "Actor:Spawn")
    Entity.Spawn(self)
    
    if (self.keys.model) then
        self.model = LoadModel(self.keys.model)
        self.model.dm = self:AttachDrawModel(self.model)
        if (self.model.SetRootController and self.keys.idle) then
            self.model:BlendToState(self.keys.idle)
        end 
        if (self.keys.scale) then
			self.model.dm:ScaleTo(Vec3ForString(self.keys.scale), 0)
        end
        if (self.keys.angles) then
			self:SetRotation(Vec3ForString(self.keys.angles))
		end
        self.visible = BoolForString(self.keys.visible, true)
        self.model.dm:SetVisible(self.visible)
        
        self:SetMins(Vec3ForString(self.keys.mins), {-32, -32, -32})
		self:SetMaxs(Vec3ForString(self.keys.maxs), { 32,  32,  32})
        self.model.dm:SetBounds(self:Mins(), self:Maxs())
        self:SetOccupantType(kOccupantType_BBox)
		self:Link()
    end

end

function Actor.OnEvent(self, cmd, args)
	COutLineEvent("Actor", cmd, args)
	
	if (cmd == "show") then
		self.visible = false
		if (self.model) then
			self.model.dm:SetVisible(false)
		end
		return true
	elseif (cmd == "hide") then
		self.visible = false
		if (self.model) then
			self.model.dm:SetVisible(false)
		end
		return true
	elseif (cmd == "play") then
		if (self.model.BlendToState) then
			self.model:BlendToState(args)
		end
		return true
	end
	
	return false
end

info_actor = Actor

