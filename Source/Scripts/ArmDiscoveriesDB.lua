-- ArmDiscoveriesDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms


--[[ 

Some quick documentation on how these should be setup

These discoveries can be unlocked in a variety of ways so in order to customize the
event log, which is essentially a first-person narration of events, context of how things
are discovered are important.

In a discovery the following items MUST exist:
	picture, title, text, index
	
	picture: the picture to display for the discovery in the DB
	title: the name of the object to be displayed
	text: the text description of the discovered object.
	index: the numerical order that the discovery should appear in.
	
	
The following items are OPTIONAL:

	chat, discoveryPopupText, mysteryTitle, mysteryText
	mysteryChat, mysteryLogText, logText

	chat: 	the name of the conversation topic to start if the user selects
			"talk about this". If this is ommitted then the DB entry will not
			have a link to the arm and "talk about this" will not be appended 
			to the title.
			
	discoveryPopupText: used by a discovery entity in the floating popup window.
	
	mysteryTitle: if this is present then when an item is discovered in the world via
				a discovery entity it will be "locked" and only the mystery text items
				will be used until the users talks to the arm about it.
				
				If this is present the following items are required:
				
					mysteryText, mysteryChat, mysteryLogText
					
				mysteryText: the text description used in the discovery popup, and the DB
				mysteryLogText: the text inserted into the event log when the item is first
								discovered.
				mysteryChat: the chat topic to activate when the user "talks" about it in the
								DB. This topic typically will have a "discover" action somewhere
								in it to fully unlock the discovery.
			
	logText: log text is an array with several optional members:
		logText = {
			all = "STRING",
			arm = "STRING",
			terminal = "STRING",
			world = "STRING"
		}
		
		Depending on the context of how the user discovers this item, the appropriate
		string table entry will be inserted into the event log.
		
		all: 	used if text for the specific context wasn't found, i.e. this is the default text
				to use. If the discovery is only discoverable from one place OR the flavor texts
				don't make sense then just use an "all" text version.
				
		arm:	if a conversation tree contains a discover action then the item will be discovered
				in the context of the "arm", and this text will be used in the event log.
				
				"The Arm told me about a substance called Computronium".
				
		terminal: if a terminal puzzles reward actions contain a discover action then the item will
				be discovered in the context of a "terminal", and this text will be used in the
				event log.
				
				"I downloaded information from a terminal about a substance called Computronium. 
				 Maybe the Arm knows more about it."
				
		world:	if the object is discovered using an discovery entity in the world, or otherwise
				discovered from a script event, then the context will be "world" and this text
				will be used in the event log.
				
				"I found this strange looking substance while I was wandering the ship. The Arm told me it is
				 called Computronium. Maybe the Arm knows more about it."

]]
Arm.Discoveries = {

	--[[MysteryExample = {
		picture = "UI/discovery_tentacles_M",
		title = "ARM_TOPIC_TENTACLES",
		text = "ARM_DISCOVERY_TENTACLES",
		mysteryTitle = "ARM_DISCOVERY_MYSTERY_TITLE",
		mysteryText = "ARM_DISCOVERY_MYSTERY_TEXT",
		mysteryChat = "Tentacles",
		mysteryLogText = "ARM_DISCOVERY_MYSTERY_LOG",
		logText = {
			arm = "ARM_DISCOVERY_TENTACLES_LOG_ARM"
		},
		chat = "ARM_TOPIC_TENTACLES"
	},]]
	
	Pulse = {
		picture = "UI/discovery_pulse_M",
		title = "ARM_DISCOVERY_PULSE_TITLE",
		text = "ARM_DISCOVERY_PULSE",
		logText = {
			all = "ARM_DISCOVERY_PULSE_LOG"
		}
	},
	
	Manipulate = {
		picture = "UI/discovery_manipulate_M",
		title = "ARM_DISCOVERY_MANIPULATE_TITLE",
		text = "ARM_DISCOVERY_MANIPULATE",
		logText = {
			all = "ARM_DISCOVERY_MANIPULATE_LOG"
		}
	},
	
	Shield = {
		picture = "UI/discovery_shield_M",
		title = "ARM_DISCOVERY_SHIELD_TITLE",
		text = "ARM_DISCOVERY_SHIELD",
		logText = {
			all = "ARM_DISCOVERY_SHIELD_LOG"
		}
	},
	
	Acheron = {
		picture = "UI/discovery_acheron_M",
		title = "ARM_DISCOVERY_ACHERON_TITLE",
		text = "ARM_DISCOVERY_ACHERON",
		logText = {
			all = "ARM_DISCOVERY_ACHERON_LOG"
		}
	},
	
	Exodus = {
		picture = "UI/discovery_exodus_M",
		title = "ARM_DISCOVERY_EXODUS_TITLE",
		text = "ARM_DISCOVERY_EXODUS",
		logText = {
			all = "ARM_DISCOVERY_EXODUS_LOG"
		}
	},
	
	Morta = {
		picture = "UI/discovery_morta_M",
		title = "ARM_DISCOVERY_MORTA_TITLE",
		text = "ARM_DISCOVERY_MORTA",
		logText = {
			all = "ARM_DISCOVERY_MORTA_LOG"
		}
	},
	
	TheGhost = {
		picture = "UI/discovery_ghost_M",
		title = "ARM_DISCOVERY_GHOST_TITLE",
		text = "ARM_DISCOVERY_GHOST",
		logText = {
			all = "ARM_DISCOVERY_GHOST_LOG"
		}
	},
		
	BlockCreature = {
		picture = "UI/discovery_block_creature_M",
		title = "ARM_DISCOVERY_BLOCK_CREATURE_TITLE",
		text = "ARM_DISCOVERY_BLOCK_CREATURE",
		logText = {
			all = "ARM_DISCOVERY_BLOCK_CREATURE_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_BLOCK_CREATURE_POPUP"
	},
	
	BugHoles = {
		picture = "UI/discovery_bug_holes_M",
		title = "ARM_DISCOVERY_BUG_HOLES_TITLE",
		text = "ARM_DISCOVERY_BUG_HOLES",
		logText = {
			all = "ARM_DISCOVERY_BUG_HOLES_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_BUG_HOLES_POPUP"
	},
	
	Bugs = {
		picture = "UI/discovery_bugs_M",
		title = "ARM_DISCOVERY_BUGS_TITLE",
		text = "ARM_DISCOVERY_BUGS",
		logText = {
			all = "ARM_DISCOVERY_BUGS_LOG"
		},
		chat = "Bugs"
	},
	
	ComputerLog = {
		picture = "UI/discovery_computer_log_M",
		title = "ARM_DISCOVERY_COMPUTER_LOG_TITLE",
		text = "ARM_DISCOVERY_COMPUTER_LOG",
		logText = {
			all = "ARM_DISCOVERY_COMPUTER_LOG_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_COMPUTER_LOG_POPUP"
	},
	
	CrystalineStructure = {
		picture = "UI/discovery_crystaline_structure_M",
		title = "ARM_DISCOVERY_CRYSTALINE_STRUCTURE_TITLE",
		text = "ARM_DISCOVERY_CRYSTALINE_STRUCTURE",
		logText = {
			all = "ARM_DISCOVERY_CRYSTALINE_STRUCTURE_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_CRYSTALINE_STRUCTURE_POPUP"
	},
	
	DarkCorner = {
		picture = "UI/discovery_dark_corner_M",
		title = "ARM_DISCOVERY_DARK_CORNER_TITLE",
		text = "ARM_DISCOVERY_DARK_CORNER",
		logText = {
			all = "ARM_DISCOVERY_DARK_CORNER_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_DARK_CORNER_POPUP"
	},
	
	DeepSpace = {
		picture = "UI/discovery_deep_space_M",
		title = "ARM_DISCOVERY_DEEPSPACE_TITLE",
		text = "ARM_DISCOVERY_DEEPSPACE",
		logText = {
			all = "ARM_DISCOVERY_DEEPSPACE_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_DEEPSPACE_POPUP"
	},
	
	Dreamscape = {
		picture = "UI/discovery_dreamscape_M",
		title = "ARM_DISCOVERY_DREAMSCAPE_TITLE",
		text = "ARM_DISCOVERY_DREAMSCAPE",
		logText = {
			all = "ARM_DISCOVERY_DREAMSCAPE_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_DREAMSCAPE_POPUP"
	},
	
	DreamscapeMatrix = {
		picture = "UI/discovery_dreamscape_matrix_M",
		title = "ARM_DISCOVERY_DREAMSCAPE_MATRIX_TITLE",
		text = "ARM_DISCOVERY_DREAMSCAPE_MATRIX",
		logText = {
			all = "ARM_DISCOVERY_DDREAMSCAPE_MATRIX_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_DREAMSCAPE_MATRIX_POPUP"
	},
	
	DreamscapePortal = {
		picture = "UI/discovery_dreamscape_portal_M",
		title = "ARM_DISCOVERY_DREAMSCAPE_PORTAL_TITLE",
		text = "ARM_DISCOVERY_DREAMSCAPE_PORTAL",
		logText = {
			all = "ARM_DISCOVERY_DREAMSCAPE_PORTAL_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_DREAMSCAPE_PORTAL_POPUP"
	},
	
	FlowingSkin = {
		picture = "UI/discovery_flowing_skin_M",
		title = "ARM_DISCOVERY_FLOWING_SKIN_TITLE",
		text = "ARM_DISCOVERY_FLOWING_SKIN",
		logText = {
			all = "ARM_DISCOVERY_FLOWING_SKIN_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_FLOWING_SKIN_POPUP"
	},
	
	GlowingPolyps = {
		picture = "UI/discovery_glowing_polyps_M",
		title = "ARM_DISCOVERY_GLOWING_POLYPS_TITLE",
		text = "ARM_DISCOVERY_GLOWING_POLYPS",
		logText = {
			all = "ARM_DISCOVERY_GLOWING_POLYPS_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_GLOWING_POLYPS_POPUP"
	},
	
	Glyphs = {
		picture = "UI/discovery_glyphs_M",
		title = "ARM_DISCOVERY_GLYPHS_TITLE",
		text = "ARM_DISCOVERY_GLYPHS",
		logText = {
			all = "ARM_DISCOVERY_GLYPHS_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_GLYPHS_POPUP"
	},
	
	HumanShipDoor = {
		picture = "UI/discovery_human_ship_door_M",
		title = "ARM_DISCOVERY_HUMAN_SHIP_DOOR_TITLE",
		text = "ARM_DISCOVERY_HUMAN_SHIP_DOOR",
		logText = {
			all = "ARM_DISCOVERY_HUMAN_SHIP_DOOR_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_HUMAN_SHIP_DOOR_POPUP"
	},
	
	HumanShipTop1 = {
		picture = "UI/discovery_human_ship_top1_M",
		title = "ARM_DISCOVERY_HUMAN_SHIP_TOP1_TITLE",
		text = "ARM_DISCOVERY_HUMAN_SHIP_TOP1",
		logText = {
			all = "ARM_DISCOVERY_HUMAN_SHIP_TOP1_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_HUMAN_SHIP_TOP1_POPUP"
	},
	
	HumanShipTop2 = {
		picture = "UI/discovery_human_ship_top2_M",
		title = "ARM_DISCOVERY_HUMAN_SHIP_TOP2_TITLE",
		text = "ARM_DISCOVERY_HUMAN_SHIP_TOP2",
		logText = {
			all = "ARM_DISCOVERY_HUMAN_SHIP_TOP2_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_HUMAN_SHIP_TOP2_POPUP"
	},
	
	HumanShipTop3 = {
		picture = "UI/discovery_human_ship_top3_M",
		title = "ARM_DISCOVERY_HUMAN_SHIP_TOP3_TITLE",
		text = "ARM_DISCOVERY_HUMAN_SHIP_TOP3",
		logText = {
			all = "ARM_DISCOVERY_HUMAN_SHIP_TOP3_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_HUMAN_SHIP_TOP3_POPUP"
	},
	
	HumanShipTop4 = {
		picture = "UI/discovery_human_ship_top4_M",
		title = "ARM_DISCOVERY_HUMAN_SHIP_TOP4_TITLE",
		text = "ARM_DISCOVERY_HUMAN_SHIP_TOP4",
		logText = {
			all = "ARM_DISCOVERY_HUMAN_SHIP_TOP4_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_HUMAN_SHIP_TOP4_POPUP"
	},
	
	JumpDown = {
		picture = "UI/discovery_jump_down_M",
		title = "ARM_DISCOVERY_JUMP_DOWN_TITLE",
		text = "ARM_DISCOVERY_JUMP_DOWN",
		logText = {
			all = "ARM_DISCOVERY_JUMP_DOWN_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_JUMP_DOWN_POPUP"
	},
	
	ManipulateTentacle = {
		picture = "UI/discovery_manipulate_tentacle_M",
		title = "ARM_DISCOVERY_MANIPULATE_TENTACLE_TITLE",
		text = "ARM_DISCOVERY_MANIPULATE_TENTACLE",
		logText = {
			all = "ARM_DISCOVERY_MANIPULATE_TENTACLE_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_MANIPULATE_TENTACLE_POPUP"
	},
	
	ManipulatePylon = {
		picture = "UI/discovery_manipulate_pylon_M",
		title = "ARM_DISCOVERY_MANIPULATE_PYLON_TITLE",
		text = "ARM_DISCOVERY_MANIPULATE_PYLON",
		logText = {
			all = "ARM_DISCOVERY_MANIPULATE_PYLON_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_MANIPULATE_PYLON_POPUP"
	},
	
	MedicalDiscovery = {
		picture = "UI/discovery_medical_discovery_M",
		title = "ARM_DISCOVERY_MEDICAL_DISCOVERY_TITLE",
		text = "ARM_DISCOVERY_MEDICAL_DISCOVERY",
		logText = {
			all = "ARM_DISCOVERY_MEDICAL_DISCOVERY_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_MEDICAL_DISCOVERY_POPUP"
	},
	
	Overgrowth = {
		picture = "UI/discovery_overgrowth_M",
		title = "ARM_DISCOVERY_OVERGROWTH_TITLE",
		text = "ARM_DISCOVERY_OVERGROWTH",
		logText = {
			all = "ARM_DISCOVERY_OVERGROWTH_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_OVERGROWTH_POPUP"
	},
	
	PlacesToClimb = {
		picture = "UI/discovery_places_to_climb_M",
		title = "ARM_DISCOVERY_PLACES_TO_CLIMB_TITLE",
		text = "ARM_DISCOVERY_PLACES_TO_CLIMB",
		logText = {
			all = "ARM_DISCOVERY_PLACES_TO_CLIMB_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_PLACES_TO_CLIMB_POPUP"
	},
	
	Pod = {
		picture = "UI/discovery_pod_M",
		title = "ARM_DISCOVERY_POD_TITLE",
		text = "ARM_DISCOVERY_POD",
		logText = {
			all = "ARM_DISCOVERY_POD_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_POD_POPUP"
	},
	
	Polyps = {
		picture = "UI/discovery_polyps_M",
		title = "ARM_DISCOVERY_POLYPS_TITLE",
		text = "ARM_DISCOVERY_POLYPS",
		logText = {
			all = "ARM_DISCOVERY_POLYPS_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_POLYPS_POPUP"
	},
	
	PylonElevator = {
		picture = "UI/discovery_pylon_elevator_M",
		title = "ARM_DISCOVERY_PYLON_ELEVATOR_TITLE",
		text = "ARM_DISCOVERY_PYLON_ELEVATOR",
		logText = {
			all = "ARM_DISCOVERY_PYLON_ELEVATOR_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_PYLON_ELEVATOR_POPUP"
	},
	
	Pylons = {
		picture = "UI/discovery_pylons_M",
		title = "ARM_DISCOVERY_PYLONS_TITLE",
		text = "ARM_DISCOVERY_PYLONS",
		logText = {
			all = "ARM_DISCOVERY_PYLONS_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_PYLONS"
	},
	
	Rain = {
		picture = "UI/discovery_rain_M",
		title = "ARM_DISCOVERY_RAIN_TITLE",
		text = "ARM_DISCOVERY_RAIN",
		logText = {
			all = "ARM_DISCOVERY_RAIN"
		},
		discoveryPopupText = "ARM_DISCOVERY_RAIN_POPUP"
	},
	
	Skin = {
		picture = "UI/discovery_skin_M",
		title = "ARM_DISCOVERY_SKIN_TITLE",
		text = "ARM_DISCOVERY_SKIN",
		logText = {
			all = "ARM_DISCOVERY_SKIN"
		},
		discoveryPopupText = "ARM_DISCOVERY_SKIN_POPUP",
		chat = "Skin"
	},
	
	--[[SleepingTentacle = {
		picture = "UI/discovery_sleeping_tentacle_M",
		title = "ARM_DISCOVERY_SLEEPING_TENTACLE_TITLE",
		text = "ARM_DISCOVERY_SLEEPING_TENTACLE",
		logText = {
			all = "ARM_DISCOVERY_SLEEPING_TENTACLE"
		},
		discoveryPopupText = "ARM_DISCOVERY_SLEEPING_TENTACLE_POPUP"
	},]]
	
	Tentacles = {
		picture = "UI/discovery_tentacles_M",
		title = "ARM_DISCOVERY_TENTACLES_TITLE",
		text = "ARM_DISCOVERY_TENTACLES",
		logText = {
			all = "ARM_DISCOVERY_TENTACLES_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_TENTACLES_POPUP",
		chat = "Tentacles"
	},
	
	RogueTentacles = {
		picture = "UI/discovery_rogue_tentacles_M",
		title = "ARM_DISCOVERY_ROGUE_TENTACLES_TITLE",
		text = "ARM_DISCOVERY_ROGUE_TENTACLES",
		logText = {
			all = "ARM_DISCOVERY_ROGUE_TENTACLES_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_ROGUE_TENTACLES_POPUP",
		chat = "RogueTentacles"
	},
	
	Terminals = {
		picture = "UI/discovery_terminals_M",
		title = "ARM_DISCOVERY_TERMINALS_TITLE",
		text = "ARM_DISCOVERY_TERMINALS",
		logText = {
			all = "ARM_DISCOVERY_TERMINALS_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_TERMINALS_POPUP",
		chat = "Terminals",
		mysteryTitle = "ARM_DISCOVERY_MYSTERY_TITLE",
		mysteryText = "ARM_DISCOVERY_MYSTERY_TEXT",
		mysteryChat = "TerminalsMystery",
		mysteryLogText = "ARM_DISCOVERY_MYSTERY_LOG",
		events = {
			"crushed_terminal1discovery2_controller trigger"
		}
	},
	
	TheMasters = {
		picture = "UI/discovery_the_masters_M",
		title = "ARM_DISCOVERY_THE_MASTERS_TITLE",
		text = "ARM_DISCOVERY_THE_MASTERS",
		logText = {
			all = "ARM_DISCOVERY_THE_MASTERS_LOG"
		},
		chat = "Masters"
	},
	
	TheMaw = {
		picture = "UI/discovery_the_maw_M",
		title = "ARM_DISCOVERY_THE_MAW_TITLE",
		text = "ARM_DISCOVERY_THE_MAW",
		logText = {
			all = "ARM_DISCOVERY_THE_MAW_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_THE_MAW_POPUP"
	},
	
	TheRoom = {
		picture = "UI/discovery_the_room_M",
		title = "ARM_DISCOVERY_THE_ROOM_TITLE",
		text = "ARM_DISCOVERY_THE_ROOM",
		logText = {
			all = "ARM_DISCOVERY_THE_ROOM_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_THE_ROOM_POPUP"
	},
	
	TheShip = {
		picture = "UI/discovery_the_ship_M",
		title = "ARM_DISCOVERY_THE_SHIP_TITLE",
		text = "ARM_DISCOVERY_THE_SHIP",
		logText = {
			all = "ARM_DISCOVERY_THE_SHIP_LOG"
		}
	},
	
	WildGrowth = {
		picture = "UI/discovery_wild_growth_M",
		title = "ARM_DISCOVERY_WILD_GROWTH_TITLE",
		text = "ARM_DISCOVERY_WILD_GROWTH",
		logText = {
			all = "ARM_DISCOVERY_WILD_GROWTH_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_WILD_GROWTH_POPUP"
	},
	
	WindowLight = {
		picture = "UI/discovery_window_light_M",
		title = "ARM_DISCOVERY_WINDOW_LIGHT_TITLE",
		text = "ARM_DISCOVERY_WINDOW_LIGHT",
		logText = {
			all = "ARM_DISCOVERY_WINDOW_LIGHT_LOG"
		},
		discoveryPopupText = "ARM_DISCOVERY_WINDOW_LIGHT_POPUP"
	}
}

Arm.DiscoveriesDBInset = { 16, 16 }

function Arm.SpawnDiscoveriesDB(self)

	self.discoveriesDBWorkspace = {
		self.dbWorkspace[1] + (Arm.DiscoveriesDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[2] + (Arm.DiscoveriesDBInset[2] * UI.identityScale[2]),
		self.dbWorkspace[3] - (Arm.DiscoveriesDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[4] - (Arm.DiscoveriesDBInset[2] * UI.identityScale[2])
	}
	
	self.discoveriesDBWorkspaceSize = {
		0,
		0,
		self.discoveriesDBWorkspace[3],
		self.discoveriesDBWorkspace[4]
	}
	
	-- big scrolling list of text
	self.widgets.db.DiscoveriesRoot = UI:CreateWidget("Widget", {rect=self.discoveriesDBWorkspace})
	self.widgets.db.Root:AddChild(self.widgets.db.DiscoveriesRoot)
	self.widgets.db.DiscoveriesRoot:SetBlendWithParent(true)
	
	self.discoveriesDBArea = {
		0,
		0,
		self.discoveriesDBWorkspaceSize[3] - (4 * UI.identityScale[1]),
		self.discoveriesDBWorkspaceSize[4]
	}
	
	if (UI.mode == kGameUIMode_Mobile) then
		local rect = UI:MaterialSize(self.gfx.DiscoveriesScrollBar)
		local h = self.discoveriesDBWorkspaceSize[4]
		local scale = h / rect[4]
		rect[4] = h
		rect[3] = rect[3] * scale
		rect[1] = self.discoveriesDBWorkspaceSize[3] - rect[3]
		rect[2] = 0
		
		self.widgets.db.DiscoveriesScrollBar = UIPushButton:Create(
			rect,
			{
				pressed = self.gfx.DiscoveriesScrollBarPressed,
				enabled = self.gfx.DiscoveriesScrollBar
			},
			{
				pressed = UI.sfx.Command
			},
			{
				pressed = function (w, e) Arm:DiscoveryMobileScrollPressed(e) end
			},
			{
				pressedWhenDown = true
			},
			self.widgets.db.DiscoveriesRoot
		)
		
		self.widgets.db.DiscoveriesScrollBar:SetBlendWithParent(true)
		self.widgets.db.DiscoveriesScrollBar.height = rect[4]
		
		self.discoveriesDBArea[3] = self.discoveriesDBArea[3] - rect[3]
	end
	
	self.widgets.db.Discoveries = UI:CreateWidget("VListWidget", {rect=self.discoveriesDBArea})
	self.widgets.db.DiscoveriesRoot:AddChild(self.widgets.db.Discoveries)
	self.widgets.db.Discoveries:SetBlendWithParent(true)
	
	if (UI.mode == kGameUIMode_PC) then
		UI:CreateVListWidgetScrollBar(
			self.widgets.db.Discoveries,
			24,
			24,
			8
		)
		self.discoveriesDBArea[3] = self.discoveriesDBArea[3] - 24
	end
	
	self.widgets.db.Discoveries:SetClipRect(self.discoveriesDBArea)
	self.widgets.db.Discoveries:SetEndStops({0, self.discoveriesDBArea[4]*0.1})
	
	self.discoveryTime = -1
	
	local index = 100
	local sorted = {}
	
	for i=0,25 do
	
		local marker = {
			string = string.char(65+i),
			index = index
		}
		
		table.insert(sorted, marker)
		
		local marker = {
			string = string.char(97+i),
			index = index
		}
		
		table.insert(sorted, marker)
	
		index = index + 100
	end
	
	for k,v in pairs(Arm.Discoveries) do
	
		v.picture = World.Load(v.picture)
		v.pictureSize = UI:MaterialSize(v.picture)
		v.pictureSize[3] = v.pictureSize[3] * UI.identityScale[1]
		v.pictureSize[4] = v.pictureSize[4] * UI.identityScale[2]
		
		local marker = {
			string = StringTable.Get(v.title),
			dbItem = v
		}
		
		table.insert(sorted, marker)
	end
	
	local sortfunc = function(a, b)
		return System.UTF8Compare(a.string, b.string) < 0
	end
	
	table.sort(sorted, sortfunc)
	
	local index = 1
	
	for k,v in pairs(sorted) do
	
		if (v.dbItem) then
			v.dbItem.index = index
			index = index + 1
		else
			-- marker hit, set sort
			index = v.index
		end
	
	end
	
end

function Arm.DiscoveryMobileScrollPressed(self, e)
	local frac = e.data[2] / self.widgets.db.DiscoveriesScrollBar.height
	local index = math.floor(Clamp(frac, 0, 1) * 2600)
	
	-- find closest matching item
	local topic = nil
	local best = 999999
	
	for k,v in pairs(self.discoveryList) do
	
		local dd = math.abs(v.index - index)
		if (dd < best) then
			best = dd
			topic = v
		end
	
	end
	
	
	if (topic) then
		self.widgets.db.Discoveries:ScrollTo({0, topic.y}, 0.4)
	end
	
end

function Arm.EnterDiscoveriesDB(self, enter, callback, time)
	if (time == nil) then	
		time = 0
	end
	
	if (enter) then
		Arm:LoadDiscoveries()
		Arm:ScrollToRequestedItem()
		self.widgets.db.DiscoveriesRoot:BlendTo({1,1,1,1}, time)
	else
		self.widgets.db.DiscoveriesRoot:BlendTo({1,1,1,0}, time)
	end
	
	if (callback and (time > 0)) then
		self.dbTimer = World.globalTimers:Add(callback, time)
	elseif (callback) then
		callback()
	end
end

function Arm.ClearDiscoveries(self)
	self.discoveryTime = -1
	if (self.discoveryList) then
		for k,v in pairs(self.discoveryList) do
			if (v.button) then
				v.button:Unmap() -- mark gc
				if (v.button.label) then
					v.button.label:Unmap()
				end
				if (v.button.highlight) then	
					v.button.highlight:Unmap()
				end
			end
		end
		self.discoveryList = nil
	end
	self.widgets.db.Discoveries:Clear()
end

function Arm.DiscoveryPressed(self, discovery)

	if (discovery.unlocked or (discovery.mysteryTitle == nil)) then
		self.requestedTopic = discovery.chat
	else
		self.requestedTopic = discovery.mysteryChat
	end
	
	self.topic = nil
	Arm:TalkPressed()
end

function Arm.LayoutDiscovery(self, discovery, section, state)

	-- discoveries have a title, a picture, and a description
	local y = 0
	
	local f = function()
		Arm:DiscoveryPressed(discovery)
	end
	
	local rect = {
		0,
		y+state.y,
		self.discoveriesDBArea[3],
		state.titleAdvance
	}
	
	local unlocked = discovery.unlocked
	
	if (discovery.mysteryTitle == nil) then
		unlocked = true
	end

	local w
	local title = nil

	if (unlocked) then
		title = StringTable.Get(discovery.title)
	else
		title = StringTable.Get(discovery.mysteryTitle)
	end
	
	local chat = nil
	
	if (unlocked or (discovery.mysteryChat == nil)) then
		chat = discovery.chat
	else
		chat = discovery.mysteryChat
	end
	
	if (chat) then
		title = title.." "..StringTable.Get("ARM_DISCOVERY_TALK_ABOUT_THIS")
	end
	local underlineY
	
	if (chat) then
		w = UI:CreateStylePushButton(
			rect,
			f,
			{
				background = false,
				highlight = { 
					on = {0.75, 0.75, 0.75, 1}, 
					time = 0.2 
				}, 
				typeface = state.titleTypeface,
				pressed = state.sound
			}
		)
		
		self.widgets.db.Discoveries:AddItem(w)
		
		local sw,sh = UI:StringDimensions(state.titleTypeface, title)
		
		if (sw > self.discoveriesDBArea[3]) then
			rect = UI:LineWrapCenterLJustifyText(w.label, self.discoveriesDBArea[3], true, 0, title)
		else
			UI:SetLabelText(w.label, title)
			rect = UI:SizeLabelToContents(w.label)
		end
		
		-- expand the button around the label
		local buttonRect = {
			0,
			y+state.y,
			rect[3] + 16*UI.identityScale[1],
			rect[4] + 16*UI.identityScale[2]
		}
			
		w:SetRect(buttonRect)
		w:SetBlendWithParent(true)
		w.highlight:SetRect({0, 0, buttonRect[3], buttonRect[4]})
		rect = CenterChildRectInRect(rect, buttonRect)
		w.label:SetRect(rect)
		
		-- put an underline under the title
		underlineY = y + rect[2] + rect[4]-- + state.titleUnderline
		section.button = w
	else
		-- just make a plain text label
		w = UI:CreateWidget("TextLabel", {rect=rect, typeface=state.titleTypeface})
		rect = UI:LineWrapCenterLJustifyText(w, self.discoveriesDBArea[3], true, 0, title)
		self.widgets.db.Discoveries:AddItem(w)
		w:SetBlendWithParent(true)
		w:Unmap() -- mark gc
			
		-- put an underline under the title
		underlineY = y + rect[4]-- + state.titleUnderline
		
	end
	
	section.y = y+state.y

	local underlineWidth = self.discoveriesDBArea[3] - discovery.pictureSize[3] - (12*UI.identityScale[1])
	
	-- put in the picture
	
	y = underlineY + state.titleSpace
	
	rect = {
		self.discoveriesDBArea[3] - (4*UI.identityScale[1]) - discovery.pictureSize[3],
		y + state.y,
		discovery.pictureSize[3],
		discovery.pictureSize[4]
	}
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material=discovery.picture})
	w:SetBlendWithParent(true)
	self.widgets.db.Discoveries:AddItem(w)
	w:Unmap() -- mark gc
	
	-- margins
	local textX = 8 * UI.identityScale[1]
	local textMargin = self.discoveriesDBArea[3] - (2*textX)
	
	local pictureMaxY = rect[4] + (16 * UI.identityScale[2])
	local pictureMargin = rect[1] - (12*UI.identityScale[1]) - textX
	
	-- insert text
	
	local text = nil
	if (unlocked or (discovery.mysteryText == nil)) then
		text = discovery.text
	else
		text = discovery.mysteryText
	end
	
	local text = StringTable.Get(text)
	text = string.split(text, "\n")

	local modelStrings = {}
	local textY = 0
	
	for k,v in pairs(text) do

		if (v and (v ~= "")) then
			local a = nil
			local b = v
		
			-- do we have to insert text lines that fit inside this picture area?
			while (textY <= pictureMaxY) do

				a,b = UI:SplitStringAtSize(state.typeface, b, pictureMargin)
					
				local string = {
					x = 0,
					y = textY,
					text = a,
					scaleX = UI.fontScale[1],
					scaleY = UI.fontScale[2]
				}
				
				table.insert(modelStrings, string)
				textY = textY + state.advance
					
				if (b == nil) then
					break
				end
			
			end
			
			if (b ~= nil) then
				local lines = UI:WordWrap(state.typeface, b, textMargin)
				
				for k,v in pairs(lines) do
				
					local string = {
						x = 0,
						y = textY,
						text = v,
						scaleX = UI.fontScale[1],
						scaleY = UI.fontScale[2]
					}
					
					table.insert(modelStrings, string)
					textY = textY + state.advance
				
				end
				
			end
		end
		
		textY = textY + state.advance
	
	end
	
	textY = Max(textY, pictureMaxY)
	
	w = UI:CreateWidget("TextLabel", {rect={textX, y+state.y, 8, 8}, typeface=state.typeface})
	w:SetBlendWithParent(true)
	w:SetText(modelStrings)
	UI:SizeLabelToContents(w)
	self.widgets.db.Discoveries:AddItem(w)
	w:Unmap() -- mark gc
	
	-- underline last
	
	w = UI:CreateWidget("MatWidget", {rect={0,underlineY+state.y, underlineWidth, 7}, material=self.gfx.LineBorder4})
	w:SetBlendWithParent(true)
	self.widgets.db.Discoveries:AddItem(w)
	w:Unmap() -- mark gc
	
	state.y = state.y + y + textY + state.discoverySpace
	
end

function Arm.ScrollToRequestedItem(self)
	local requestedTopic = self.requestedDBTopic
	self.requestedDBTopic = nil
	
	if (requestedTopic) then
		local topic = Arm.Discoveries[requestedTopic]
		if (topic) then
			-- it's in our list scroll to it
			self.widgets.db.Discoveries:ScrollTo({0, topic.y}, 0.4)
		end
	end

end

function Arm.LoadDiscoveries(self)

	if (self.discoveryTime >= GameDB.discoveryTime) then
	
		for k,v in pairs(self.discoveryList) do
			if (v.button) then
				v.button.class:ResetHighlight(v.button, true)
			end
		end
	
		return
	end
	
	Arm:ClearDiscoveries()
	
	self.discoveryTime = GameDB.discoveryTime
	self.discoveryList = {}
	
	local unlocked = {}
	
	for k,v in pairs(Arm.Discoveries) do
		if (GameDB:CheckDiscovery(k)) then
			table.insert(self.discoveryList, v)
			v.unlocked = GameDB:CheckDiscoveryUnlocked(k)
		end
	end
	
	local sorter = function(a, b)
		return a.index < b.index
	end
	
	table.sort(self.discoveryList, sorter)
	
	local state = {
		y = 0,
		titleSpace = 8*UI.identityScale[2],
		advance = UI:FontAdvanceSize(self.typefaces.Chat),
		titleTypeface = self.typefaces.LogArmAsk,
		typeface = self.typefaces.Chat,
		sound = self.sfx.Button,
		discoverySpace = 32 * UI.identityScale[2]
	}
	
	for k,v in pairs(self.discoveryList) do
		self:LayoutDiscovery(v, v, state)
	end
	
	self.widgets.db.Discoveries:RecalcLayout()

end
