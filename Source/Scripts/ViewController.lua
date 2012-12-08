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

function ViewController.LevelStart(self)
	COutLine(kC_Debug, "ViewController:LevelStart")
	self:Defaults()
end
	
function ViewController.Defaults(self)
	
	COutLine(kC_Debug, "ViewController:Defaults")
	
	--self:SetCamera(VecSub(self:Target():WorldPos(), {50, 0, 0}), VecZero())
	
	self:SetTargetMode(
		self.TM_Distance, -- Specify the distance from the target
		0, -- in time
		0, -- out time
		-1, -- hold time (-1 for infinite)
		1, -- minimum  units
		1, -- maximum  units
		0, -- take X seconds to lerp between those distances
		-1, -- this is the "lag", a quasi number between 0 and 1 that controls how "loosly" the distance is tracked (<= 0 means no lag)
		{ 0, -10, -140 }, -- minAngles (X, Y, Z) NOTE: Y pitches up over the object, Z rotates around it
		{ 0, -10, -160 }, -- maxAngles
		60, -- Take X seconds to lerp between them
		-1, -- angle lag (<= 0 means no lag)
		false -- use pitch angle of target
	)
	
	self:SetCameraSway(
		0, -- in time
		0, -- out time
		-1, -- hold time (-1 for infinite)
		0, -- min sway
		25,  -- max sway
		10,   -- take 5 seconds to go from min/max
		19,  -- take 10 seconds to go around the circle
		{1, 1} -- scaling factors for x/y
	)
	
	-- This animates the FOV.
	-- It takes a base value, and a plus/minus factor
	-- If you set a Base Value other than zero the camera simply lerps between those fov's
	-- If you specify zero for the base, then the FOV is controlled by the distance parameters
	self:SetCameraFov(
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
