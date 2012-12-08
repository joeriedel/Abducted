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
	
	COutLine(C_Debug, "GameNetwork.Initialize(available=%s)", tostring(available))
	
	return available
	
end

function GameNetwork.Available()

	return Persistence.ReadBool(Session, "gnAvailable", false)

end

function GameNetwork.OnLocalPlayerAuthenticated(authenticated)

	local changed = false
	local wasAuthenticated = Persistence.ReadBool(Session, "gnAuthenticated", false)
	if (wasAuthenticated ~= authenticated) then
		COutLine(C_Debug, "GameNetwork.OnLocalPlayerAuthenticated(%s)", tostring(authenticated))
		Persistence.WriteBool(Session, "gnAuthenticated", authenticated)
		Session:Save()
	end
	
	if (World.game and World.game.OnLocalPlayerAuthenticated) then
		World.game:OnLocalPlayerAuthenticated(authenticated, changed)
	end

end

function GameNetwork.LocalPlayerIsAuthenticated()

	return Persistence.ReadBool(Session, "gnAuthenticated", false)

end

function GameNetwork.OnShowLeaderboard(show)

	COutLine(C_Debug, "GameNetwork.OnShowLeaderboard(%s)", tostring(show))
	
	if (World.game and World.game.OnShowLeaderboard) then
		World.game:OnShowLeaderboard(show)
	end

end

function GameNetwork.OnShowAchievements(show)

	COutLine(C_Debug, "GameNetwork.OnShowAchievements(%s)", tostring(show))
	
	if (World.game and World.game.OnShowAchievements) then
		World.game:OnShowAchievements(show)
	end

end


