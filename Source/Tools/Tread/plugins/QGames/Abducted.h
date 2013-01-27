// Abducted.h
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#pragma once

#include "Quake.h"

///////////////////////////////////////////////////////////////////////////////
// AbductedGame
///////////////////////////////////////////////////////////////////////////////

class AbductedGame : public CQuakeGame
{
public:

	AbductedGame() : CQuakeGame(CQBrush::TYPE_Q3) {}

	virtual const char *Name();

	virtual void GetExportFile(const char *filename, char *buff, int buffSize);
	virtual CPluginFileExport *NativeMapExporter();
	virtual CPluginFileExport *FileExporter(int i);

	virtual const char *PakType(int i);
	virtual CTextureFactory *CreatePakFile();
	virtual CQuakeCompiler *CreateCompiler(const char *filename, QuakeToolsList &tools, CTreadDoc *doc, bool runMap);

	virtual CQuakeUserData *CreateUserData(CTreadDoc *doc);
	virtual void InitializeToolsList(QuakeToolsList &tools);
	virtual void GetLeakFileName(CTreadDoc *doc, char *buff, int buffSize);

	virtual CObjectCreator *ObjectCreator(int i);

protected:

	virtual void RunMapCompile(const char *mappath, QuakeToolsList &tools, CTreadDoc *doc, bool runGame);

};

///////////////////////////////////////////////////////////////////////////////
// AbductedUserData
///////////////////////////////////////////////////////////////////////////////

class AbductedUserData : public CQuakeUserData
{
public:
	AbductedUserData(CTreadDoc *doc, AbductedGame *game) : CQuakeUserData(doc, game), m_WaypointMenuCreated(false) {}
	
	CWaypoint::ContextMenu m_WaypointMenu;
	
	void MakeBrushMenu(CTreadDoc *doc);
	void MakeWaypointMenu(CTreadDoc *doc);



protected:

	bool m_WaypointMenuCreated;
};

///////////////////////////////////////////////////////////////////////////////
// AbductedMap
///////////////////////////////////////////////////////////////////////////////

class AbductedMap : public CQuakeMap
{
public:

	virtual const char *Type() { return "Abducted Map"; }
	virtual const char *Extension() { return "map"; }
	virtual const char *PluginGame() { return "Abducted"; }

protected:

	virtual bool ExportTextures(const char *filename, CTreadDoc *doc) { return false; }
};


