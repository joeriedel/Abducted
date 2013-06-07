-- ManipulatableObjectUI.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

ManipulatableObjectUI = Class:New()

function ManipulatableObjectUI.Spawn(self)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
end

function ManipulatableObjectUI.Notify(self, enabled)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
	self.nextKey = 0
	
	if (enabled) then
		self.objects = {}
	else
		self.objects = nil
	end
	
end

function ManipulatableObjectUI.NotifyObject(self, object, enabled)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
end

function ManipulatableObjectUI.HandleInputEvent(self, e)
	if (UI.mode == kGameUIMode_Mobile) then
		return false
	end
	
end

function ManipulatableObjectUI.UpdateUI(self, key)
	if (UI.mode == kGameUIMode_Mobile) then
		return
	end
	
end

function ManipulatableObjectUI.MakeControlPanel(self, key)

end
