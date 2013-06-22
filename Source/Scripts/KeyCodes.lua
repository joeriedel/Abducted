-- KeyCodes.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[
NOTES: Not all the keys here are physical keys, this list contains physical keys and translated keys
(i.e. keys that are generated via modifiers). Only physical keys are contained in the string table for
UI display of the key-name. The OS generates the actual unicode text for the key when pressed (i.e WM_CHAR)
]]--

kKeyCode_None        = 0
kKeyCode_Backspace   = 8
kKeyCode_Tab         = 9
kKeyCode_Clear       = 12
kKeyCode_Return      = 13
kKeyCode_Pause       = 19
kKeyCode_Escape      = 27
kKeyCode_Space       = 32
kKeyCode_Exclaim     = 33
kKeyCode_QuoteDbl    = 34
kKeyCode_Hash        = 35
kKeyCode_DollarSign  = 36
kKeyCode_Percent     = 37
kKeyCode_Ampersand   = 38
kKeyCode_SingleQuote = 39
kKeyCode_ParenLeft   = 40
kKeyCode_ParenRight  = 41
kKeyCode_Asterisk    = 42
kKeyCode_Plus        = 43
kKeyCode_Comma       = 44
kKeyCode_Minus       = 45
kKeyCode_Period      = 46
kKeyCode_Slash		 = 47
kKeyCode_0			 = 48
kKeyCode_1			 = 49
kKeyCode_2			 = 50
kKeyCode_3			 = 51
kKeyCode_4			 = 52
kKeyCode_5			 = 53
kKeyCode_6			 = 54
kKeyCode_7			 = 55
kKeyCode_8			 = 56
kKeyCode_9			 = 57
kKeyCode_Colon		 = 58
kKeyCode_Semicolon   = 59
kKeyCode_Less        = 60
kKeyCode_Equals      = 61
kKeyCode_Greater     = 62
kKeyCode_Question    = 63
kKeyCode_At          = 64

kKeyCode_BracketLeft = 91
kKeyCode_BackSlash = 92
kKeyCode_BracketRight = 93
kKeyCode_Caret      = 94
kKeyCode_Underscore = 95
kKeyCode_BackQuote  = 96

kKeyCode_A           = 97
kKeyCode_B           = 98
kKeyCode_C           = 99
kKeyCode_D           = 100
kKeyCode_E           = 101
kKeyCode_F           = 102
kKeyCode_G           = 103
kKeyCode_H           = 104
kKeyCode_I           = 105
kKeyCode_J           = 106
kKeyCode_K           = 107
kKeyCode_L           = 108
kKeyCode_M           = 109
kKeyCode_N           = 110
kKeyCode_O           = 111
kKeyCode_P           = 112
kKeyCode_Q           = 113
kKeyCode_R           = 114
kKeyCode_S           = 115
kKeyCode_T           = 116
kKeyCode_U           = 117
kKeyCode_V           = 118
kKeyCode_W           = 119
kKeyCode_X           = 120
kKeyCode_Y           = 121
kKeyCode_Z           = 122

kKeyCode_BraceLeft  = 123
kKeyCode_Bar        = 124
kKeyCode_BraceRight = 125
kKeyCode_Tilde      = 126
kKeyCode_Delete		= 127

kKeyCode_KP0        = 256
kKeyCode_KP1        = 257
kKeyCode_KP2        = 258
kKeyCode_KP3        = 259
kKeyCode_KP4        = 260
kKeyCode_KP5        = 261
kKeyCode_KP6        = 262
kKeyCode_KP7        = 263
kKeyCode_KP8        = 264
kKeyCode_KP9        = 265
kKeyCode_KP_Period  = 266
kKeyCode_KP_Divide  = 267
kKeyCode_KP_Multiply = 268
kKeyCode_KP_Minus   = 269
kKeyCode_KP_Plus    = 270
kKeyCode_KP_Enter   = 271
kKeyCode_KP_Equals  = 272

kKeyCode_Up         = 273
kKeyCode_Down       = 274
kKeyCode_Right      = 275
kKeyCode_Left       = 276
kKeyCode_Insert     = 277
kKeyCode_Home       = 278
kKeyCode_End        = 279
kKeyCode_PageUp     = 280
kKeyCode_PageDown   = 281

kKeyCode_F1         = 282
kKeyCode_F2         = 283
kKeyCode_F3         = 284
kKeyCode_F4         = 285
kKeyCode_F5         = 286
kKeyCode_F6         = 287
kKeyCode_F7         = 288
kKeyCode_F8         = 289
kKeyCode_F9         = 290
kKeyCode_F10        = 291
kKeyCode_F11        = 292
kKeyCode_F12        = 293
kKeyCode_F13        = 294
kKeyCode_F14        = 295
kKeyCode_F15        = 296

kKeyCode_NumLock    = 300
kKeyCode_CapsLock   = 301
kKeyCode_ScrollLock = 302
kKeyCode_RShift     = 303
kKeyCode_LShift     = 304
kKeyCode_RCtrl      = 305
kKeyCode_LCtrl      = 306
kKeyCode_RAlt       = 307
kKeyCode_LAlt       = 308
kKeyCode_LCommand   = 311
kKeyCode_RCommand   = 312
	
kKeyCode_Help       = 315
kKeyCode_PrintScreen = 316
kKeyCode_SysReq     = 317
kKeyCode_Break      = 318
kKeyCode_Menu       = 319
kKeyCode_Max 		= 320

PhysicalKeyToStringIdTable = {
	[kKeyCode_Backspace] = "VKEY_BACKSPACE",
	[kKeyCode_Tab] = "VKEY_TAB",
	[kKeyCode_Clear] = "VKEY_CLEAR",
	[kKeyCode_Return] = "VKEY_RETURN",
	[kKeyCode_Pause] = "VKEY_PAUSE",
	[kKeyCode_Escape] = "VKEY_ESCAPE",
	[kKeyCode_Space] = "VKEY_SPACE",
	[kKeyCode_Period] = "VKEY_PERIOD",
	[kKeyCode_Slash] = "VKEY_FORWARDSLASH",
	[kKeyCode_Semicolon] = "VKEY_SEMICOLON",
	[kKeyCode_BracketLeft] = "VKEY_BRACKETLEFT",
	[kKeyCode_BackSlash] = "VKEY_BACKSLASH",
	[kKeyCode_BracketRight] = "VKEY_BRACKETRIGHT",
	[kKeyCode_BackQuote] = "VKEY_BACKQUOTE",
	[kKeyCode_0] = "VKEY_0",
	[kKeyCode_1] = "VKEY_1",
	[kKeyCode_2] = "VKEY_2",
	[kKeyCode_3] = "VKEY_3",
	[kKeyCode_4] = "VKEY_4",
	[kKeyCode_5] = "VKEY_5",
	[kKeyCode_6] = "VKEY_6",
	[kKeyCode_7] = "VKEY_7",
	[kKeyCode_8] = "VKEY_8",
	[kKeyCode_9] = "VKEY_9",
	[kKeyCode_A] = "VKEY_A",
	[kKeyCode_B] = "VKEY_B",
	[kKeyCode_C] = "VKEY_C",
	[kKeyCode_D] = "VKEY_D",
	[kKeyCode_E] = "VKEY_E",
	[kKeyCode_F] = "VKEY_F",
	[kKeyCode_G] = "VKEY_G",
	[kKeyCode_H] = "VKEY_H",
	[kKeyCode_I] = "VKEY_I",
	[kKeyCode_J] = "VKEY_J",
	[kKeyCode_K] = "VKEY_K",
	[kKeyCode_L] = "VKEY_L",
	[kKeyCode_M] = "VKEY_M",
	[kKeyCode_N] = "VKEY_N",
	[kKeyCode_O] = "VKEY_O",
	[kKeyCode_P] = "VKEY_P",
	[kKeyCode_Q] = "VKEY_Q",
	[kKeyCode_R] = "VKEY_R",
	[kKeyCode_S] = "VKEY_S",
	[kKeyCode_T] = "VKEY_T",
	[kKeyCode_U] = "VKEY_U",
	[kKeyCode_V] = "VKEY_V",
	[kKeyCode_W] = "VKEY_W",
	[kKeyCode_X] = "VKEY_X",
	[kKeyCode_Y] = "VKEY_Y",
	[kKeyCode_Z] = "VKEY_Z",
	[kKeyCode_Delete] = "VKEY_DELETE",
	[kKeyCode_KP0] = "VKEY_KP0",
	[kKeyCode_KP1] = "VKEY_KP1",
	[kKeyCode_KP2] = "VKEY_KP2",
	[kKeyCode_KP3] = "VKEY_KP3",
	[kKeyCode_KP3] = "VKEY_KP4",
	[kKeyCode_KP4] = "VKEY_KP5",
	[kKeyCode_KP5] = "VKEY_KP6",
	[kKeyCode_KP6] = "VKEY_KP7",
	[kKeyCode_KP7] = "VKEY_KP8",
	[kKeyCode_KP8] = "VKEY_KP9",
	[kKeyCode_KP_Period] = "VKEY_KP_PERIOD",
	[kKeyCode_KP_Divide] = "VKEY_KP_SLASH",
	[kKeyCode_KP_Multiply] = "VKEY_KP_MULTIPLY",
	[kKeyCode_KP_Plus] = "VKEY_KP_PLUS",
	[kKeyCode_KP_Enter] = "VKEY_KP_ENTER",
	[kKeyCode_KP_Equals] = "VKEY_KP_EQUALS",
	[kKeyCode_Up] = "VKEY_UP",
	[kKeyCode_Down] = "VKEY_DOWN",
	[kKeyCode_Right] = "VKEY_RIGHT",
	[kKeyCode_Left] = "VKEY_LEFT",
	[kKeyCode_Insert] = "VKEY_INSERT",
	[kKeyCode_Home] = "VKEY_HOME",
	[kKeyCode_End] = "VKEY_END",
	[kKeyCode_PageUp] = "VKEY_PAGEUP",
	[kKeyCode_PageDown] = "VKEY_PAGEDOWN",
	[kKeyCode_F1] = "VKEY_F1",
	[kKeyCode_F2] = "VKEY_F2",
	[kKeyCode_F3] = "VKEY_F3",
	[kKeyCode_F4] = "VKEY_F4",
	[kKeyCode_F5] = "VKEY_F5",
	[kKeyCode_F6] = "VKEY_F6",
	[kKeyCode_F7] = "VKEY_F7",
	[kKeyCode_F8] = "VKEY_F8",
	[kKeyCode_F9] = "VKEY_F9",
	[kKeyCode_F10] = "VKEY_F10",
	[kKeyCode_F11] = "VKEY_F11",
	[kKeyCode_F12] = "VKEY_F12",
	[kKeyCode_F13] = "VKEY_F13",
	[kKeyCode_F14] = "VKEY_F14",
	[kKeyCode_F15] = "VKEY_F15",
	[kKeyCode_NumLock] = "VKEY_NUMLOCK",
	[kKeyCode_CapsLock] = "VKEY_CAPSLOCK",
	[kKeyCode_ScrollLock] = "VKEY_SCROLLLOCK",
	[kKeyCode_RShift] = "VKEY_RSHIFT",
	[kKeyCode_LShift] = "VKEY_LSHIFT",
	[kKeyCode_RCtrl] = "VKEY_RCTRL",
	[kKeyCode_LCtrl] = "VKEY_LCTRL",
	[kKeyCode_RAlt] = "VKEY_RALT",
	[kKeyCode_LAlt] = "VKEY_LATL",
	[kKeyCode_PrintScreen] = "VKEY_PRINTSCREEN"
}

-- Returns the "name" of a physical key
function PhysicalKeyName(key)

	local id = PhysicalKeyToStringIdTable[key]
	if (id == nil) then
		return "<!!!NULL!!!>"
	end

	if (StringTable.Language == kLangId_EN) then
		return StringTable.Get(id)
	end
	
	local s = StringTable.Get(id, nil, true, nil)
	if ((s == nil) or (s == "")) then
		return StringTable.Get(id, nil, false, nil, kLangId_EN) -- try english
	end
	
	return s
end
