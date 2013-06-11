-- Input.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function World.InputEventFilter(e)
	if (cv_r_fly:Get()) then
		return false
	end
	return Game.entity:InputEventFilter(e)
end

function World.InputGestureFilter(g)
	return Game.entity:InputGestureFilter(g)
end

function World.OnInputEvent(e)
	
	if (FlyCam:HandleInput(e)) then
		return true
	end
	
	if Input.IsTouchEvent(e) or (e.type == kI_KeyDown) then
		if (Game.entity:OnInputEvent(e)) then
			return true
		end
	end
	
	return false

end

function World.OnInputGesture(g)

	if (Game.entity:OnInputGesture(g)) then
		return true
	end
	
	return false

end

Input = Class:New()

function Input.Spawn(self)

--	self.ManipulateUpKey = Persistence.ReadNumber(Globals, "inputMapping/manup", 

end

function Input.IsTouchBegin(e)
	return (e.type == kI_TouchBegin) or ((e.type == kI_MouseDown) and (e.data[3] == kMouseButton_LMask))
end

function Input.IsTouchEnd(e, touch)
	return ((touch == nil) or (e.touch == touch)) and 
		((e.type == kI_TouchEnd) or (e.type == kI_TouchCancelled) or 
		 ((e.type == kI_MouseUp) and (e.data[3] == kMouseButton_LMask)))
end

function Input.IsTouchMove(e, touch)
	return  (e.touch == touch) and ((e.type == kI_TouchMoved) or ((e.type == kI_MouseMove) and (e.data[3] == kMouseButton_LMask)))
end

function Input.IsTouchEvent(e)
	return I_IsTouch(e) or I_IsMouse(e)
end

