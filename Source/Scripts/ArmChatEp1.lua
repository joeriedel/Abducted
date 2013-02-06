-- ArmChatEp1.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.Chats = {}

--[[---------------------------------------------------------------------------
	Episode 1 chat converastion questions and answers
-----------------------------------------------------------------------------]]

Arm.Chats.WhatIsThisPlace = {
	prompt = {{"ARM_CHAT_WHAT_IS_THIS_PLACE"}},
	reply = {{"ARM_CHAT_THIS_IS_THE_SHIP"}}
}

Arm.Chats.WhatDoYouMeanShip = {
	prompt = {{"ARM_CHAT_WHAT_DO_YOU_MEAN_SHIP"}},
	reply = {{"ARM_CHAT_NOT_A_HUMAN_SHIP"}}
}

Arm.Chats.WhereAreWe = {
	prompt = {{"ARM_CHAT_WHERE_ARE_WE"}},
	reply = {{"ARM_CHAT_NEAR_DEBRI"}}
}

Arm.Chats.WhyAreWeNearDebri = {
	prompt = {{"ARM_CHAT_WHY_ARE_WE_NEAR_DEBRI"}},
	reply = {{"ARM_CHAT_I_DONT_KNOW_BUT_SOMEONE"}}
}

Arm.Chats.SomeoneWho = {
	prompt = {{"ARM_CHAT_SOMEONE_WHO"}},
	reply = {{"ARM_CHAT_I_DONT_KNOW"}}
}

Arm.Chats.CMonTellMe = {
	prompt = {{"ARM_CHAT_CMON_TELL_ME"}},
	reply = {{"ARM_CHAT_I_WISH_I_COULD_TELL_YOU"}}
}

Arm.Chats.IDontLikeSecrets = {
	prompt = {{"ARM_CHAT_I_DONT_LIKE_SECRETS"}},
	reply = {{"ARM_CHAT_I_THINK_WE_ARE_DONE"}},
	action = "lock"
}

Arm.Chats.HowPowerful = {
	prompt = {{"ARM_CHAT_HOW_POWERFUL"}}
}

Arm.Chats.WhoAmI = {
	prompt = {{"ARM_CHAT_WHO_AM_I"}}
}

Arm.Chats.WhoAreYou = {
	prompt = {{"ARM_CHAT_WHO_ARE_YOU"}}
}

Arm.Chats.WhatCanYouDo = {
	prompt = {{"ARM_CHAT_WHAT_CAN_YOU_DO"}}
}

Arm.Chats.HowFarFromEarth = {
	prompt = {{"ARM_CHAT_HOW_FAR_FROM_EARTH"}}
}

Arm.Chats.HowBigIsThisShip = {
	prompt = {{"ARM_CHAT_HOW_BIG_IS_THIS_SHIP"}}
}

Arm.Chats.WhoMadeIt = {
	prompt = {{"ARM_CHAT_WHO_MADE_IT"}}
}

Arm.Chats.HowOldIsIt = {
	prompt = {{"ARM_CHAT_HOW_OLD_IS_IT"}}
}

Arm.Chats.IsThisAShip = {
	prompt = {{"ARM_CHAT_IS_THIS_A_SHIP"}}
}

Arm.Chats.HowLongWasIAsleep = {
	prompt = {{"ARM_CHAT_HOW_LONG_WAS_I_ASLEEP"}}
}

Arm.Chats.AmIAlone = {
	prompt = {{"ARM_CHAT_AM_I_ALONE"}}
}

Arm.Chats.HowDidIGetHere = {
	prompt = {{"ARM_CHAT_HOW_DID_I_GET_HERE"}}
}

--[[---------------------------------------------------------------------------
	Link questions and answers together into a dialog after they are declared
-----------------------------------------------------------------------------]]

Arm.Chats.WhatIsThisPlace.choices = {
	{ Arm.Chats.WhatDoYouMeanShip },
	{ Arm.Chats.WhereAreWe }
}

Arm.Chats.WhatDoYouMeanShip.choices = {
	{ Arm.Chats.WhoMadeIt },
	{ Arm.Chats.WhereAreWe },
	{ Arm.Chats.HowOldIsIt }
}

Arm.Chats.WhereAreWe.choices = {
	{ Arm.Chats.WhyAreWeNearDebri },
	{ Arm.Chats.HowFarFromEarth }
}

Arm.Chats.WhyAreWeNearDebri.choices = {
	{ Arm.Chats.SomeoneWho },
	{ Arm.Chats.HowBigIsThisShip },
	{ Arm.Chats.WhoMadeIt }
}

Arm.Chats.SomeoneWho.choices = {
	{ Arm.Chats.CMonTellMe },
	{ Arm.Chats.HowBigIsThisShip },
	{ Arm.Chats.HowOldIsIt }
}

Arm.Chats.CMonTellMe.choices = {
	{ Arm.Chats.IDontLikeSecrets },
	{ Arm.Chats.HowPowerful }
}

--[[---------------------------------------------------------------------------
Root conversations, these are displayed as "Change Conversation" choices
-----------------------------------------------------------------------------]]

Arm.Chats.Roots = {}
Arm.Chats.Roots.Genesis1 = {
	reply = {{"ARM_CHAT_DEFAULT_PROMPT"}},
	choices = {
		{Arm.Chats.WhoAmI},
		{Arm.Chats.WhatIsThisPlace},
		{Arm.Chats.WhoAreYou}
	}
}

Arm.Chats.Roots.Genesis2 = {
	reply = {{"ARM_CHAT_DEFAULT_PROMPT"}},
	choices = {
		{Arm.Chats.WhoAreYou},
		{Arm.Chats.IsThisAShip},
		{Arm.Chats.WhatCanYouDo}
	}
}

Arm.Chats.Roots.Genesis3 = {
	reply = {{"ARM_CHAT_DEFAULT_PROMPT"}},
	choices = {
		{Arm.Chats.IsThisAShip},
		{Arm.Chats.HowLongWasIAsleep},
		{Arm.Chats.AmIAlone}
	}
}

Arm.Chats.Roots.Genesis4 = {
	reply = {{"ARM_CHAT_DEFAULT_PROMPT"}},
	choices = {
		{Arm.Chats.HowDidIGetHere},
		{Arm.Chats.WhoAreYou},
		{Arm.Chats.WhereAreWe}
	}
}

--[[---------------------------------------------------------------------------
	Load available conversations
-----------------------------------------------------------------------------]]

Arm.Chats.Available = {}

function Arm.LoadCommonChats(self)
	Arm.Chats.Available.Genesis1 = Arm.Chats.Roots.Genesis1
	Arm.Chats.Available.Genesis2 = Arm.Chats.Roots.Genesis2
	Arm.Chats.Available.Genesis3 = Arm.Chats.Roots.Genesis3
	Arm.Chats.Available.Genesis4 = Arm.Chats.Roots.Genesis4
end

function Arm.LoadEp1Crushed(self)
	Arm:LoadCommonChats()
end

function Arm.LoadEp1Eaten(self)
	Arm:LoadCommonChats()
end

function Arm.LoadEp1Falling(self)
	Arm:LoadCommonChats()
end
