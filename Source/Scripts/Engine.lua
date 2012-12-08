-- Engine.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
		Engine Enums and Types
---------------------------------------------------------------------------]]--

-- COut/COutLine
kC_Debug =  0
kC_Info = 1
kC_Warn = 2
kC_Error = 3
kC_ErrMsgBox = 4

-- Platform Types
kPlatMac = 0
kPlatWin = 1
kPlatIPad = 2
kPlatIPhone = 3
kPlatXBox360 = 4
kPlatPS3 = 5

-- MoveTypes
kMT_None = 0
kMT_Fly = 1
kMT_Spline = 2

-- SolidTypes
kST_None = 0
kST_BBox = 1
kST_Brush = 2
kST_All = 0xff

-- Physics Flags
kPF_OnGround = 1
kPF_AutoFace = 2
kPF_Friction = 4
kPF_SeekAngles = 8
kPF_ResetSpline = 16
kPF_LoopSpline = 32
kPF_SplineEvents = 64
kPF_SplineBank = 128
kPF_FlipSplineBank = 256

-- Tick flags
kTF_PreTick = 1
kTF_PostTick = 2
kTF_PostPhysics = 4

kTickNone = 0
kTickNext = 1
kTickPop = 2

-- Asset Types
kAT_Texture = 0
kAT_Material = 1
kAT_Map = 2
kAT_SkModel = 3
kAT_SkAnimSet = 4
kAT_SkAnimStates = 5
kAT_Font = 6
kAT_Typeface = 7

-- Motion Types (for ViewModels / Ska Animation)
kSka_MT_None = 0
kSka_MT_Relative = 1
kSka_MT_Absolute = 2

-- Input
kI_KeyDown = 0
kI_KeyUp = 1
kI_MouseDown = 2
kI_MouseUp = 3
kI_MouseMove = 4
kI_MouseWheel = 5
kI_TouchBegin = 6
kI_TouchMoved = 7
kI_TouchStationary = 8
kI_TouchEnd = 9
kI_TouchCancelled = 10

-- Mouse Buttons
kMouseButton_LMask = 1
kMouseButton_MMask = 2
kMouseButton_RMask = 4

-- Sound Channels
kSoundChannel_First = 0
kSoundChannel_UI = SC_First
kSoundChannel_Ambient = 1
kSoundChannel_FX = 2
kSoundChannel_Music = 3
kSoundChannel_Max = 4

-- Unload Disposition
kUD_None = 0
kUD_Slot = 1
kUD_All  = 2

-- Cloud File Status
kCFS_Ready = 0
kCFS_Downloading = 1
kCFS_Error = 2

-- Cinematic Flags
kCF_AnimateCamera = 1
kCF_CanPlayForever = 2
kCF_Loop = 4

-- Material Timing Modes
kMat_TM_Absolute = 0
kMat_TM_Relative = 1

-- String table languages
kLangId_EN = 0
kLangId_FR = 1
kLangId_IT = 2
kLangId_GR = 3
kLangId_SP = 4
kLangId_RU = 5
kLangId_JP = 6
kLangId_CH = 7

-- Input Helpers

function I_IsMouse(event)
	return event.type == I_MouseDown or event.type == I_MouseUp or event.type == I_MouseMove
end

function I_IsMouseButton(event)
	return event.type == I_MouseDown or event.type == I_MouseUp
end

function I_IsTouch(event)
	return event.type >= I_TouchBegin and event.type <= I_TouchCancelled
end

--[[

InputEvent = {

	touch, -- Touch ID
	type,  -- I_*
	data[3],
	time -- in ms
}

--]]

-- Gestures

kIG_Null = 1
kIG_LClick = 2
kIG_RClick = 4
kIG_DoubleClick = 8
kIG_Tap = IG_LClick
kIG_DoubleTap = IG_DoubleClick
kIG_Line = 16
kIG_Circle = 32
kIG_Pinch = 64
kIGPhase_Begin = 0
kIGPhase_Move = 1
kIGPhase_End = 2

--[[

InputGesture = {

	id, -- IG_*
	phase, -- IGPhase*
	mins[2],
	maxs[2],
	origin[2],
	args[3], -- float args
	time -- in ms	

}

--]]

-- Pause Bits
kPauseGame = 1
kPauseUI = 2
kPauseCinematics = 4
kPauseAll = bit.bor(kPauseGame, kPauseUI, kPauseCinematics)

-- Widgets
kHA_Left = 0
kHA_Right = 1
kHA_Center = 2
kVA_Top = 0
kVA_Bottom = 1
kVA_Center = 2


