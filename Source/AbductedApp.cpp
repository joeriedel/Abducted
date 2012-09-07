// AbductedApp.cpp
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Abducted/LICENSE for licensing terms.

#include "AbductedApp.h"
#include <Engine/Engine.h>
#include <Runtime/Tokenizer.h>
#include <Runtime/File.h>
#include "Game/Entities/G_Exports.h"
#if defined(RAD_OPT_GL) && defined(RAD_TARGET_GOLDEN) && defined(RAD_OPT_PC)
#include <Engine/Renderer/PC/RGLBackend.h>
#endif
#if defined(RAD_TARGET_GOLDEN) && defined(RAD_OPT_PC)
#include <Engine/Persistence.h>
#endif

App *App::New(int argc, const char **argv) { 
	return new AbductedApp(argc, argv); 
}

AbductedApp::AbductedApp(int argc, const char **argv) : App(argc, argv), m_background(false) {
	spawn::G_Exports();
}

AbductedApp::~AbductedApp() {
}

bool AbductedApp::PreInit() {
	COut(C_Info) << title.get() << " built on " << __DATE__ << " @ " << __TIME__ << std::endl;
	COut(C_Info) << "PreInit..." << std::endl;
	if (!App::PreInit()) 
		return false;
#if defined(RAD_TARGET_GOLDEN)
	file::HFile file;

	int media = file::AllMedia;
	int r = engine->sys->files->OpenFile(
		"pak0.pak",
		media,
		file,
		file::HIONotify()
	);

	if (r < file::Success) {
		COut(C_Error) << "Unable to open pak0.pak!" << std::endl;
		return false;
	}

	file::HPakFile pak = engine->sys->paks->MountPakFile(file, "pak0.pak");
	if (!pak) {
		COut(C_Error) << "Unable to mount pak0.pak!" << std::endl;
		return false;
	}

	engine->sys->files->AddPakFile(pak);

	// disable all file access except through the pak file.
	engine->sys->files->enabledMedia = file::Paks;
#endif
	return true;
}

bool AbductedApp::InitWindow() {
#if defined(RAD_OPT_GL) && defined(RAD_TARGET_GOLDEN) && defined(RAD_OPT_PC)
	Persistence::Ref settings = Persistence::Load("settings.prefs");
	
	r::VidMode mode;

	mode.bpp = 32;
	mode.hz  = 0;

	const char *vidString = "1280x720";

	vidString = settings->keys->StringForKey("vidMode", vidString);
	mode.fullscreen = settings->keys->BoolForKey("fullscreen", false);
	
	sscanf(vidString, "%dx%d", &mode.w, &mode.h);

	// try to pick a video mode that is the same aspect ratio as their desktop resolution.
	RAD_ASSERT(primaryDisplay.get());

	DisplayDevice::MatchDisposition matchOptions = DisplayDevice::kMatchDisposition_Upsize|
		DisplayDevice::kMatchDisposition_Any;

	if (!primaryDisplay->MatchVidMode(
		mode,
		matchOptions
	)) {
		// can we find anything?
		COut(C_Error) << "ERROR: Unable to find resolution " << mode.w << "x" << std::endl;
		return false;
	}

	if (!BindDisplayDevice(primaryDisplay, mode)) {
		COut(C_Error) << "ERROR: failed to set video mode " << mode.w << "x" << mode.h << std::endl;
		return false;
	}

	GLPixelFormat glpf;
	glpf.red = 8;
	glpf.green = 8;
	glpf.blue = 8;
	glpf.alpha = 8;
	glpf.depth = 24;
	glpf.stencil = 8;
	glpf.mSamples = 0;
	glpf.doubleBuffer = true;
	
	// Create an openGL context and bind it to the rendering backend.
	NativeDeviceContext::Ref glContext = CreateOpenGLContext(glpf);
	if (!glContext)
		return false;

	r::HRBackend rb = engine->sys->r.Cast<r::IRBackend>();

	r::HContext ctx = rb->CreateContext(glContext);
	if (!ctx)
		return false;

	// Assign the context
	engine->sys->r->ctx = ctx;
#endif

	return true;
}

bool AbductedApp::Initialize() {
	COut(C_Info) << "Initializing..." << std::endl;
	if (!App::Initialize()) 
		return false;
	return true;
}

bool AbductedApp::Run() {
#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	return RunAutoExec();
#else
	return true;
#endif
}

#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
bool AbductedApp::RunAutoExec() {
	char nativePath[file::MaxFilePathLen+1];
	file::ExpandToNativePath("9:/autoexec.txt", nativePath, file::MaxFilePathLen+1);
	FILE *fp = fopen(nativePath, "rb");
	if (!fp) {
		COut(C_Error) << "ERROR: autoexec.txt is missing." << std::endl;
		return false;
	}
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	void *data = safe_zone_malloc(ZEngine, size);
	if (fread(data, 1, size, fp) != size)
	{
		fclose(fp);
		zone_free(data);
		return false;
	}
	fclose(fp);

	Tokenizer script;
	script.InitParsing((const char *)data, (int)size);
	zone_free(data);

	String token;
	if (script.GetToken(token))
	{
		m_game = Game::New();
		const r::VidMode *vidMode = activeDisplay->curVidMode;
		m_game->SetViewport(0, 0, vidMode->w, vidMode->h);
		if (!(m_game->LoadEntry() && m_game->LoadMapSeq(token.c_str, 0, world::UD_Slot, true)))
		{
			m_game.reset();
			return false;
		}
	}

	return true;
}
#endif

void AbductedApp::OnTick(float dt) {
#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	if (m_game)
	{
		const r::VidMode *vidMode = activeDisplay->curVidMode;
		m_game->SetViewport(0, 0, vidMode->w, vidMode->h);
		m_game->Tick(dt);
		engine->sys->r->SwapBuffers();

#if !defined(RAD_OPT_IOS)
		if (m_game->quit)
			App::Get()->exit = true;
#endif
	}
#endif
}

void AbductedApp::NotifyBackground(bool background) {
#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	if (m_game && (m_background != background))
	{
		if (background)
			m_game->NotifySaveState();
		else
			m_game->NotifyRestoreState();

		m_background = background;
	}
#endif
}

void AbductedApp::PostInputEvent(const InputEvent &e) {
#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	if (m_game)
		m_game->PostInputEvent(e);
#endif
}

const char *AbductedApp::RAD_IMPLEMENT_GET(title)
{
	return "Abducted";
}

const char *AbductedApp::RAD_IMPLEMENT_GET(company)
{
	return "Sunside";
}

const char *AbductedApp::RAD_IMPLEMENT_GET(website)
{
	return "www.sunsidegames.com";
}

void AbductedApp::Finalize() {
#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	m_game.reset();
#endif
	App::Finalize();
}

const char *AbductedApp::RAD_IMPLEMENT_GET(flurryAPIKey) {
#if defined(RAD_OPT_SHIP)
	return "NSUEK83QJRBVXRFDND2T";
#else
	return "69GLZJN9GT5XN5IYQYXC";
#endif
}


