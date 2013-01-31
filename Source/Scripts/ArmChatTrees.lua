-- ArmChatTrees.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

function Arm.LoadChats()



end

Arm.Chats = {}

Arm.Chats.WhatIsThisPlace = {}
Arm.Chats.WhatIsThisPlace.Choices = {
	WhatIsThisPlace = {
		prompt = "ARM_CHAT_WHAT_IS_THIS_PLACE",
		reply = {
			{
				probability = 1, 
				text = "ARM_CHAT_NOT_A_HUMAN_SHIP",
				replySpeed = { 1, 2 }
			}
		},
		choices = {}
	},
	WhereAreWe = {
		prompt = "ARM_CHAT_WHERE_ARE_WE",
		choices = {}
	}
	
}