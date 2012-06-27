// AbductedApp.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Abducted/LICENSE for licensing terms.

#pragma once

#include <Engine/App.h>

#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
#include <Engine/Game/Game.h>
#endif

class AbductedApp : public App
{
public:
	AbductedApp();
	virtual ~AbductedApp();

	virtual bool PreInit();
	virtual bool Initialize();
	virtual void Finalize();
	virtual void NotifyBackground(bool background);
	virtual void PostInputEvent(const InputEvent &e);

#if defined(RAD_OPT_PC)
	virtual void CloseWindow();
	virtual void FocusChange(bool focus);
#endif

protected:

	virtual void OnTick(float dt);

private:

	virtual RAD_DECLARE_GET(title, const char*);
	virtual RAD_DECLARE_GET(company, const char*);
	virtual RAD_DECLARE_GET(website, const char*);
	virtual RAD_DECLARE_GET(allowMultipleInstances, bool);
	virtual RAD_DECLARE_GET(flurryAPIKey, const char*);

#if defined(RAD_OPT_PC_TOOLS)
	virtual RAD_DECLARE_GET(wantEditor, bool);
	virtual void SwitchEditorMode(bool editor);
	virtual void EditorStart();
#endif

#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	virtual RAD_DECLARE_GET(game, Game*) { return m_game.get(); }
	bool RunAutoExec();
	Game::Ref m_game;
#endif

	bool m_background;
};
