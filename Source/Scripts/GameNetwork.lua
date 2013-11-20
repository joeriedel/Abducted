-- GameNetwork.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- Native:
-- GameNetwork.Create()
-- GameNetwork.AuthenticateLocalPlayer()
-- GameNetwork.SendScore("leaderboardId", score)
-- GameNetwork.LogEvent("eventName", {optional table}, timed)
-- GameNetwork.EndTimedEvent("eventName", {optional table})
-- GameNetwork.LogError("error", "message")
-- GameNetwork.SessionReportOnAppClose()
-- GameNetwork.SetSessionReportOnAppClose(bool)
-- GameNetwork.SessionReportOnAppPause()
-- GameNetwork.SetSessionReportOnAppPause(bool)
-- GameNetwork.ShowLeaderboard(leaderboardId)
-- GameNetwork.ShowAchievements()

function GameNetwork.Initialize()

	local created = Persistence.ReadBool(Session, "gnCreated", false)
	if created then
		return GameNetwork.Available()
	end
	
	local available = GameNetwork.Create() -- native
	Persistence.WriteBool(Session, "gnCreated", true)
	Persistence.WriteBool(Session, "gnAvailable", available)
	Session:Save()
	
	COutLine(kC_Debug, "GameNetwork.Initialize(available=%s)", tostring(available))
	
	return available
	
end

function GameNetwork.Available()

	return Persistence.ReadBool(Session, "gnAvailable", false)

end

function GameNetwork.OnLocalPlayerAuthenticated(authenticated)

	local changed = false
	local wasAuthenticated = Persistence.ReadBool(Session, "gnAuthenticated", false)
	if (wasAuthenticated ~= authenticated) then
		COutLine(kC_Debug, "GameNetwork.OnLocalPlayerAuthenticated(%s)", tostring(authenticated))
		Persistence.WriteBool(Session, "gnAuthenticated", authenticated)
		Session:Save()
	end
	
	if (Game.entity and Game.entity.OnLocalPlayerAuthenticated) then
		Game.entity:OnLocalPlayerAuthenticated(authenticated, changed)
	end

end

function GameNetwork.LocalPlayerIsAuthenticated()

	return Persistence.ReadBool(Session, "gnAuthenticated", false)

end

function GameNetwork.OnShowLeaderboard(show)

	COutLine(kC_Debug, "GameNetwork.OnShowLeaderboard(%s)", tostring(show))
	
	if (Game.entity and Game.entity.OnShowLeaderboard) then
		Game.entity:OnShowLeaderboard(show)
	end

end

function GameNetwork.OnShowAchievements(show)

	COutLine(kC_Debug, "GameNetwork.OnShowAchievements(%s)", tostring(show))
	
	if (Game.entity and Game.entity.OnShowAchievements) then
		Game.entity:OnShowAchievements(show)
	end

end


