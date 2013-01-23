-- Input.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function World.OnInputEvent(e)

	if Input.IsTouchEvent(e) or (e.type == kI_MouseDown or e.type == kI_MouseUp) or (e.type == kI_KeyDown) then
		if (Game.OnInputEvent) then
			if (Game.OnInputEvent(e)) then
				return true
			end
		end
	end
	
	return false

end

function World.OnInputGesture(g)

	if (Game.OnInputGesture) then
		if (Game.OnInputGesture(e)) then
			return true
		end
	end
	
	return false

end

Input = {}
function Input.IsTouchBegin(e)
	return (e.type == kI_TouchBegin)
end

function Input.IsTouchEnd(e, touch)
	return ((touch == nil) or (e.touch == touch)) and 
		((e.type == kI_TouchEnd) or (e.type == kI_TouchCancelled))
end

function Input.IsTouchMove(e, touch)
	return  (e.touch == touch) and (e.type == kI_TouchMoved)
end

function Input.IsTouchEvent(e)
	return (e.type == kI_TouchBegin) or (e.type == kI_TouchEnd) or (e.type == kI_TouchMoved) or (e.type == kI_TouchStationary) or (e.type == kI_TouchCancelled)	
end

