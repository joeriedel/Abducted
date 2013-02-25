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
kPlatPC = 0
kPlatIPad = 1
kPlatIPhone = 2
kPlatXBox360 = 3
kPlatPS3 = 4

-- MoveTypes
kMoveType_None = 0
kMoveType_Fly = 1
kMoveType_Spline = 2
kMoveType_Floor = 3
kMoveType_Ska = 4

-- SolidTypes
kSolidType_None = 0
kSolidType_BBox = 1
kSolidType_All = 0xff

-- OccupantTypes
kOccupantType_None = 0
kOccupantType_BBox = 1
kOccupantType_Volume = 2

-- ClassBits
kEntityClass_Any = 0

-- BSP contents
kContentsFlag_Areaportal = 1
kContentsFlag_Solid = 2
kContentsFlag_Fog = 4
kContentsFlag_Water = 8
kContentsFlag_Clip = 16
kContentsFlag_Detail = 32
kContentsFlag_Floor = 64

-- Physics Flags
kPhysicsFlag_OnGround = 1
kPhysicsFlag_AutoFace = 2
kPhysicsFlag_Friction = 4
kPhysicsFlag_SeekAngles = 8
kPhysicsFlag_ResetSpline = 16
kPhysicsFlag_LoopSpline = 32
kPhysicsFlag_SplineEvents = 64
kPhysicsFlag_SplineBank = 128
kPhysicsFlag_FlipSplineBank = 256

-- ska::Notify mask flags
kSkaNotifyMaskFlag_Tags = 1
kSkaNotifyMaskFlag_EndFrame = 2
kSkaNotifyMaskFlag_Finished = 4
kSkaNotifyMaskFlag_All = 7
kSkaNotifyMaskFlag_None = 0

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

-- Waypoints

kWaypointState_Enabled = 1

-- Floors

kFloorState_Enabled = 1

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
kSoundChannel_UI = 0
kSoundChannel_Ambient = 1
kSoundChannel_FX = 2
kSoundChannel_Music = 3
kSoundChannel_Max = 4

-- Unload Disposition
kUnloadDisposition_None = 0
kUnloadDisposition_Slot = 1
kUnloadDisposition_All  = 2

-- Cloud File Status
kCloudFileStatus_Ready = 0
kCloudFileStatus_Downloading = 1
kCloudFileStatus_Error = 2

-- Cinematic Flags
kCinematicFlag_AnimateCamera = 1
kCinematicFlag_CanPlayForever = 2
kCinematicFlag_Loop = 4

-- Material Timing Modes
kMaterialTimingMode_Absolute = 0
kMaterialTimingMode_TM_Relative = 1

-- Material Widget Draw Modes
kMaterialWidgetDrawMode_Rect = 0
kMaterialWidgetDrawMode_Circle = 1

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
	return event.type == kI_MouseDown or event.type == kI_MouseUp or event.type == kI_MouseMove
end

function I_IsMouseButton(event)
	return event.type == kI_MouseDown or event.type == kI_MouseUp
end

function I_IsTouch(event)
	return event.type >= kI_TouchBegin and event.type <= kI_TouchCancelled
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
kHorizontalAlign_Left = 0
kHorizontalAlign_Right = 1
kHorizontalAlign_Center = 2
kVerticalAlign_Top = 0
kVerticalAlign_Bottom = 1
kVerticalAlign_Center = 2

kPositionMode_Relative = 1
kPositoinMode_Absolute = 2

