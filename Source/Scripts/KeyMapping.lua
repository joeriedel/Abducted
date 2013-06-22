-- KeyMapping.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

kAction_ManipulateLeft = "ManLeft"
kAction_ManipulateRight = "ManRight"
kAction_ManipulateUp = "ManUp"
kAction_ManipulateDown = "ManDown"
kAction_Shield = "Shield"
kAction_Pulse = "Pulse"
kAction_Manipulate = "Manipulate"
kAction_Arm = "Arm"
kAction_Stop = "Stop"
kAction_Select1 = "Select1"
kAction_Select2 = "Select2"
kAction_Select3 = "Select3"
kAction_Select4 = "Select4"
kAction_Select5 = "Select5"

DefaultActionKeys = {
	[kAction_ManipulateUp] = kKeyCode_W,
	[kAction_ManipulateDown] = kKeyCode_S,
	[kAction_ManipulateLeft] = kKeyCode_A,
	[kAction_ManipulateRight] = kKeyCode_D,
	[kAction_Manipulate] = kKeyCode_Z,
	[kAction_Arm] = kKeyCode_Tab,
	[kAction_Shield] = kKeyCode_X,
	[kAction_Pulse] = kKeyCode_C,
	[kAction_Stop] = kKeyCode_Space,
	[kAction_Select1] = kKeyCode_1,
	[kAction_Select2] = kKeyCode_2,
	[kAction_Select3] = kKeyCode_3,
	[kAction_Select4] = kKeyCode_4,
	[kAction_Select5] = kKeyCode_5
}

function LoadKeyBindings()
	local bindings = {
		ActionToKey = {},
		KeyToAction = {}
	}
	for k,v in pairs(DefaultActionKeys) do
		local key = Persistence.ReadNumber(Globals, "KeyBindings", v, k)
		bindings.ActionToKey[k] = key
		bindings.KeyToAction[key] = k
	end
	
	return bindings
end

function SaveKeyBindings(bindings)
	for k,v in pairs(bindings.ActionToKey) do
		Persistence.WriteNumber(Globals, "KeyBindings", v, k)	
	end
end