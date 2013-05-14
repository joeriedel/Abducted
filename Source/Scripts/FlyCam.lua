-- FlyCam.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

FlyCam = {}
FlyCam.MoveSpeed = 1000
FlyCam.RotateSpeed = 0.2

function FlyCam.Think(self, dt)
	if (self.pos) then
		if (not cv_r_fly:Get()) then
			self.pos = nil
			return false
		end
	else
		if (not cv_r_fly:Get()) then
			return false
		end
		self.pos = World.CameraPos()
		self.fwd = {1,0,0}
		self.left = {0,1,0}
		self.angles = {0,0,0}
		self.mfwd = 0
		self.mback = 0
		self.mleft = 0
		self.mright = 0
	end
	
	local m = {0,0,0}
	
	local fwd = self.mfwd - self.mback
	
	if (fwd ~= 0) then
		m = VecScale(self.fwd, self.MoveSpeed * dt * fwd)
	end
	
	local strafe = self.mleft - self.mright
	
	if (strafe ~= 0) then
		local z = VecScale(self.left, self.MoveSpeed * dt * strafe)
		m = VecAdd(m, z)
	end
	
	self.pos = VecAdd(self.pos, m)
	
	World.SetCamera(self.pos, self.angles, 75)
	
end

function FlyCam.HandleInput(self, e)

	if (not self.pos) then
		return false
	end
	
	if ((e.type == kI_KeyDown) or (e.type == kI_KeyUp)) then
		return self:HandleKey(e)
	elseif ((e.type == kI_MouseDown) or (e.type == kI_MouseUp) or (e.type == kI_MouseMove)) then
		return self:HandleMouse(e)
	end
	
	return false
	
end

function FlyCam.HandleKey(self, e)
	if (e.type == kI_KeyUp) then
		if (e.data[1] == kKeyCode_W) then
			self.mfwd = 0
			return true
		elseif (e.data[1] == kKeyCode_S) then
			self.mback = 0
			return true
		elseif (e.data[1] == kKeyCode_A) then
			self.mleft = 0
			return true
		elseif (e.data[1] == kKeyCode_D) then
			self.mright = 0
			return true
		end
	else
		if (e.data[1] == kKeyCode_W) then
			self.mfwd = 1
			return true
		elseif (e.data[1] == kKeyCode_S) then
			self.mback = 1
			return true
		elseif (e.data[1] == kKeyCode_A) then
			self.mleft = 1
			return true
		elseif (e.data[1] == kKeyCode_D) then
			self.mright = 1
			return true
		end
	end
	
	return false
end

function FlyCam.HandleMouse(self, e)
	local d = {0, 0}
	
	if (e.type == kI_MouseDown) then
		if (e.data[3] == kMouseButton_RMask) then
			self.mp = {e.data[1], e.data[2]}
		end
	elseif (e.type == kI_MouseMove) then
		if (e.data[3] == kMouseButton_RMask) then
			if (self.mp) then
				d = {e.data[1]-self.mp[1], e.data[2]-self.mp[2]}
				self.mp = {e.data[1], e.data[2]}
			end
		end
	else
		self.mp = nil
	end
	
	self.angles[3] = self.angles[3] - (d[1] * self.RotateSpeed)
	self.angles[2] = self.angles[2] + (d[2] * self.RotateSpeed)
	self.angles = WrapAngles(self.angles)
	
	self.fwd = ForwardVecFromAngles(self.angles)
	self.left = RotateVec({0,1,0}, self.angles)
end

