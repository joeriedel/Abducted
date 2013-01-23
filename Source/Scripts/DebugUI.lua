-- DebugUI.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

DebugUI = Class:New()
DebugUI.Enabled = System.BuildConfig() ~= "ship"
DebugUI.Pos = {4, 4}
DebugUI.LineSpace = 4

function DebugUI.Spawn(self)

	if (not DebugUI.Enabled) then
		return
	end
	
	self.gfx = {}
	self.gfx.Typeface = World.Load("Sys/Debug_TF")
	
	local w,h = self.gfx.Typeface:Size()

	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_Debug)
		
	self.widgets.rspeeds = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
	self.widgets.root:AddChild(self.widgets.rspeeds);
	
	local pos = { DebugUI.Pos[1], DebugUI.Pos[2] }
	self.widgets.fps = UI:CreateWidget("TextLabel", {rect={pos[1], pos[2], 8, 8}, typeface=self.gfx.Typeface})
	self.widgets.rspeeds:AddChild(self.widgets.fps)
	pos[2] = pos[2] + h + DebugUI.LineSpace
	
	self.widgets.portals = UI:CreateWidget("TextLabel", {rect={pos[1], pos[2], 8, 8}, typeface=self.gfx.Typeface})
	self.widgets.rspeeds:AddChild(self.widgets.portals)
	pos[2] = pos[2] + h + DebugUI.LineSpace
	
	self.widgets.models = UI:CreateWidget("TextLabel", {rect={pos[1], pos[2], 8, 8}, typeface=self.gfx.Typeface})
	self.widgets.rspeeds:AddChild(self.widgets.models)
	pos[2] = pos[2] + h + DebugUI.LineSpace
	
	self.widgets.misc = UI:CreateWidget("TextLabel", {rect={pos[1], pos[2], 8, 8}, typeface=self.gfx.Typeface})
	self.widgets.rspeeds:AddChild(self.widgets.misc)
	pos[2] = pos[2] + h + DebugUI.LineSpace
	
	self.widgets.rspeeds:SetVisible(false)
end

function DebugUI.Think(self)
	if (not DebugUI.Enabled) then
		return
	end
	if (not cv_r_speeds:Get()) then
		self.widgets.rspeeds:SetVisible(false)
		return
	end
	
	local counters = World.DrawCounters()
	self.widgets.fps:SetText(string.format("%.3f fps", counters.fps))
	UI:SizeLabel(self.widgets.fps)
	
	self.widgets.portals:SetText(string.format("%d Area(s), %d/%d portal(s)", counters.drawnAreas, counters.drawnPortals, counters.testedPortals))
	UI:SizeLabel(self.widgets.portals)
	
	self.widgets.models:SetText(
		string.format("%d/%d World, %d/%d/%d Entities", 
			counters.drawnWorldModels, counters.testedWorldModels, 
			counters.drawnEntityModels, counters.testedEntityModels, counters.drawnEntities
		)
	)
	UI:SizeLabel(self.widgets.models)
	
	self.widgets.misc:SetText(string.format("%d Tri(s), %d Batch(s), %d Material(s)", 
		counters.numTris, counters.numBatches, counters.numMaterials
	))
	UI:SizeLabel(self.widgets.misc)
		
	self.widgets.rspeeds:SetVisible(true)
	
end