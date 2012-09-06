// AbductedLauncher.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Crow/LICENSE for licensing terms.

#pragma once
#include <QtGui/QMainWindow>
#include <QtGui/QDialog>
#include <QtCore/QRect>
#include <Engine/Packages/PackagesDef.h>
#include <Engine/Sound/SoundDef.h>
#include <Engine/Persistence.h>
#include <Runtime/Time.h>

class QPixmap;
class QTimerEvent;
class QComboBox;
class QCheckBox;
class QPushButton;
class QVBoxLayout;

class AbductedLauncher : public QMainWindow {
	Q_OBJECT
public:

	AbductedLauncher();
	~AbductedLauncher();

	bool Run();
	
	RAD_DECLARE_READONLY_PROPERTY(AbductedLauncher, playClicked, bool);

protected:

	virtual void timerEvent(QTimerEvent*);
	virtual QSize sizeHint() const;

private slots:

	void Play();
	void GotoFacebook();
	void BuySoundtrack();
	void GraphicsSettings();

private:
	
	RAD_DECLARE_GET(playClicked, bool) {
		return m_playClicked;
	}

	void LoadSettings();
	void Center(const QRect &rect);
	void PlayMusic();
	void CreateButtons(QVBoxLayout *layout);
	int SelectBackground();

	xtime::SecondsTimer<> m_timer;
	SoundRef m_music;
	SoundContextRef m_soundContext;
	Persistence::Ref m_settings;
	Persistence::Ref m_defaults;
	QPushButton *m_play;
	bool m_playClicked;
};

///////////////////////////////////////////////////////////////////////////////

class AbductedGraphicsSettings : public QDialog {
	Q_OBJECT
public:
	AbductedGraphicsSettings(
		const Persistence::Ref &settings,
		const Persistence::Ref &defaults,
		QWidget *parent = 0
	);

	virtual void done(int r);

private slots:

	void ResetToDefaults();
	void UpdateEnabledState();

private:

	int m_defaultResolution;
	bool m_defaultFullscreen;

	int m_savedResolution;
	bool m_savedFullscreen;

	Persistence::Ref m_settings;
	QComboBox *m_resolutions;
	QCheckBox *m_fullscreen;
	QPushButton *m_ok;
	QPushButton *m_reset;
};
