-- ViewController.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ViewController = Class:New()

ViewController.TM_Distance = 0
ViewController.TM_Look = 1

function ViewController.Spawn(self)
	World.viewController = self
	World.SetViewController(self)
end

function ViewController.OnLevelStart(self)
	COutLine(kC_Debug, "ViewController:OnLevelStart")
	self:Defaults()
end

function ViewController.OnEvent(self, cmd, args)
	if (cmd == "shake") then
		self:HandleShakeCmd(cmd, args)
		return true
	elseif (cmd == "camera") then
		self:HandleCameraCmd(args)
		return true
	end
	
	return false;
end

function ViewController.HandleCameraCmd(self, args)
	local x = Tokenize(args)
	local lx = #x
	if (lx < 2) then
		error("Camera command must contain camera name and distance!")
	end
		
	local camera = x[1]
	local distance = tonumber(x[2])
	local strict = not FindArrayElement(x, "strict=false")
	local useFOV = not FindArrayElement(x, "fov=false")
	local forceBehind = not FindArrayElement(x, "behind=false")
	local angles = {0, 180, 180}
	
	local specifiedAngles, idx = FindArrayElement(x, "angles=")
	if (specifiedAngles) then
		angles = Vec3ForString(x[idx+1])
	end
	
	local loose = FindArrayElement(x, "follow=loose")
	
	if (loose) then
		self:DoLooseCamera(camera, distance, strict, forceBehind, useFOV, angles)
	else
		self:DoTightCamera(camera, distance, strict, forceBehind, useFOV, angles)
	end
	
end

function ViewController.DoTightCamera(self, camera, distance, strict, forceBehind, useFOV, angles)
	
	if (not forceBehind) then
		forceBehind = -1
	else
		forceBehind = 1 -- take 1 second to switch sides
	end
	
	self:SetRailMode(
		camera,
		distance,
		strict,
		0.8,
		0.8,
		forceBehind,
		useFOV,
		angles
	)
	
end

function ViewController.DoLooseCamera(self, camera, distance, strict, forceBehind, useFOV, angles)
	if (not forceBehind) then
		forceBehind = -1
	else
		forceBehind = 1 -- take 1 second to switch sides
	end
	
	self:SetRailMode(
		camera,
		distance,
		strict,
		1.2,
		1.2,
		forceBehind,
		useFOV,
		angles
	)
	
end

function ViewController.HandleShakeCmd(self, cmd, args)
	self:SetCameraSway(
		0.25,
		1,
		0.25,
		-4,
		8,
		0.1,
		0.5,
		{1, 1}
	)
end
	
function ViewController.Defaults(self)
	
	COutLine(kC_Debug, "ViewController:Defaults")
	
--[[	
	self:SetTargetMode(
		self.TM_Distance, -- Specify the distance from the target
		0, -- in time
		0, -- out time
		-1, -- hold time (-1 for infinite)
		200, -- minimum  units
		200, -- maximum  units
		0, -- take X seconds to lerp between those distances
		-1, -- this is the "lag", a quasi number between 0 and 1 that controls how "loosly" the distance is tracked (<= 0 means no lag)
		{ 0, 10, -3 }, -- minAngles (X, Y, Z) NOTE: Y pitches up over the object, Z rotates around it
		{ 0, 10,  3 }, -- maxAngles
		60, -- Take X seconds to lerp between them
		-1, -- angle lag (<= 0 means no lag)
		false -- use pitch angle of target
	)
]]
	self:SetTargetMode(
		self.TM_Look, -- Specify the target look-pos
		0, -- in time
		0, -- out time
		-1, -- hold time (-1 for infinite)
		30, -- minimum  units
		50, -- maximum  units
		10, -- take X seconds to lerp between those distances
		-1, -- this is the "lag", a quasi number between 0 and 1 that controls how "loosly" the distance is tracked (<= 0 means no lag)
		{ 0, -10, -5 }, -- minAngles (X, Y, Z) NOTE: Y pitches up over the object, Z rotates around it
		{ 0, 10,  5 }, -- maxAngles
		10, -- Take X seconds to lerp between them
		-1, -- angle lag (<= 0 means no lag)
		false -- use pitch angle of target
	)
	
	self:SetCameraSway(
		0, -- in time
		0, -- out time
		-1, -- hold time (-1 for infinite)
		0, -- min sway
		10,  -- max sway
		10,   -- take 5 seconds to go from min/max
		19,  -- take 10 seconds to go around the circle
		{1, 1} -- scaling factors for x/y
	)
	
	-- This animates the FOV.
	-- It takes a base value, and a plus/minus factor
	-- If you set a Base Value other than zero the camera simply lerps between those fov's
	-- If you specify zero for the base, then the FOV is controlled by the distance parameters
	self:SetCameraFOV(
		0, -- in time
		0, -- out time
		-1, -- hold time (-1 for infinite)
		0, -- Base value (zero means distance controlled)
		0, -- Plus
		0, -- Minus
		0, -- Animate time
		90, -- minFOV for distance
		90, -- maxFOV for distance
		0, -- min distance == minFOV +Plus or -Minus 
		0  -- max distance == maxFOV +Plus or -Minus
	)
end

view_controller = ViewController
