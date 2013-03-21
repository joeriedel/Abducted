// AbductedApp.cpp
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Abducted/LICENSE for licensing terms.

#include "AbductedApp.h"
#include <Engine/Engine.h>
#include <Runtime/Tokenizer.h>
#include <Runtime/File.h>
#include "Game/Entities/G_Exports.h"
#include <Main/GL/GLContext.h>
#if defined(RAD_OPT_GL) && defined(RAD_TARGET_GOLDEN)
#include <Engine/Renderer/GL/RGLBackend.h>
#endif
#if defined(RAD_TARGET_GOLDEN) && defined(RAD_OPT_PC)
#include <Engine/Persistence.h>
#endif

#if defined(RAD_OPT_IOS_DEVICE) && defined(RAD_TARGET_GOLDEN)
#define VALIDATE_NEON_SKIN
#endif

#if defined(VALIDATE_NEON_SKIN)
#include <Runtime/Base/SIMD.h>
#include <Engine/Renderer/SkMesh.h>
#include <Engine/Packages/Packages.h>
const SIMDDriver *SIMD_ref_bind();
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

	file::PakFileRef pakFile = engine->sys->files->OpenPakFile("manifest.pak");
	if (!pakFile) {
		COut(C_Error) << "Unable to open manifest.pak!" << std::endl;
		return false;
	}
	engine->sys->files->AddPakFile(pakFile);
	
	pakFile = engine->sys->files->OpenPakFile("pak0.pak");
	
	if (!pakFile) {
		COut(C_Error) << "Unable to open pak0.pak!" << std::endl;
		return false;
	}
	
	engine->sys->files->AddPakFile(pakFile);
	engine->sys->files->globalMask = file::kFileMask_PakFiles;
	
#endif
	return true;
}

bool AbductedApp::InitWindow() {
#if defined(RAD_OPT_GL) && defined(RAD_TARGET_GOLDEN)
#if defined(RAD_OPT_PC)
	Persistence::Ref settings = Persistence::Load("settings.prefs");
	
	r::VidMode mode;

	mode.bpp = 32;
	mode.hz  = 0;

	const char *vidString = "1280x720";

	vidString = settings->StringForKey("vidMode", vidString);
	mode.fullscreen = settings->BoolForKey("fullscreen", false);
	
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

#else

	// Embedded device

	r::VidMode mode = *primaryDisplay->curVidMode.get();

#endif

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
	file::MMFileInputBufferRef fileBuf = engine->sys->files->OpenInputBuffer("@r:/autoexec.txt", ZEngine);
	if (!fileBuf) {
		COut(C_Error) << "ERROR: autoexec.txt is missing." << std::endl;
		return false;
	}
	Tokenizer script;
	script.Bind(fileBuf);
	
	String token;
	if (script.GetToken(token, Tokenizer::kTokenMode_SameLine)) {
		m_game = Game::New();
		const r::VidMode *vidMode = activeDisplay->curVidMode;
		m_game->SetViewport(0, 0, vidMode->w, vidMode->h);
		if (!(m_game->LoadEntry() && m_game->LoadMapSeq(token.c_str, 0, world::kUD_Slot, true))) {
			m_game.reset();
			return false;
		}
	}
	
// big hack for NEON skin testing
// NOTE: none of our test data has 2 UV (tangent) channels in it, those are still done blind with no
// verification as to correctness.

#if defined(VALIDATE_NEON_SKIN)
	pkg::Asset::Ref asset = engine->sys->packages->Resolve("Characters/Eve", pkg::Z_Engine);
	RAD_VERIFY_MSG(asset, "Unable to load NEON skin test data!");
	RAD_VERIFY(asset->type == asset::AT_SkModel);
	
	int r = asset->Process(xtime::TimeSlice::Infinite, pkg::P_Load|pkg::P_FastPath);
	RAD_VERIFY_MSG(r == pkg::SR_Success, "Unable to load NEON skin test data!");
	
	r::SkMesh::Ref skModel(r::SkMesh::New(asset));
	
	ska::AnimState::Map::const_iterator it = skModel->states->find(CStr("walk"));
	RAD_VERIFY(it != skModel->states->end());
	
	ska::AnimationVariantsSource::Ref animSource = ska::AnimationVariantsSource::New(
		it->second,
		*skModel->ska.get(),
		ska::Notify::Ref()
	);

	ska::Controller::Ref target = boost::static_pointer_cast<ska::Controller>(animSource);
	
	ska::BlendToController::Ref blendTo = ska::BlendToController::New(*skModel->ska.get(), ska::Notify::Ref());
	skModel->ska->root = boost::static_pointer_cast<ska::Controller>(blendTo);
	blendTo->Activate();

	boost::static_pointer_cast<ska::BlendToController>(skModel->ska->root.get())->BlendTo(target);
	
	skModel->ska->Tick(1.f, -1.f, true, true, Mat4::Identity);
	
	for (int i = 0; i < skModel->numMeshes; ++i) {
		const ska::DMesh *m = skModel->DMesh(i);
		float *refFloats = (float*)safe_zone_malloc(ZEngine, ska::DMesh::kNumVertexFloats * m->totalVerts * sizeof(float), 0, SIMDDriver::kAlignment);
		float *simdFloats = (float*)safe_zone_malloc(ZEngine, ska::DMesh::kNumVertexFloats * m->totalVerts * sizeof(float), 0, SIMDDriver::kAlignment);
		
		skModel->SkinToBuffer(SIMD, i, simdFloats);

		// Compare the reference implementation with optimized path:
		static const SIMDDriver *SIMD_ref = SIMD_ref_bind();
		
		skModel->SkinToBuffer(SIMD_ref, i, refFloats);
		const float *src = refFloats;
		const float *cmp = simdFloats;
		for (int z = 0; z < (int)m->totalVerts; ++z) {
			float d[4];

			for (int j = 0; j < 3; ++j)
				d[j] = math::Abs(src[j]-cmp[j]);

			RAD_VERIFY_MSG((d[0] < 0.1f && d[1] < 0.1f && d[2] < 0.1f), "Bad SIMD vertex");
			
			src += 4;
			cmp += 4;

			for (int j = 0; j < 3; ++j)
				d[j] = math::Abs(src[j]-cmp[j]);

			RAD_VERIFY_MSG((d[0] < 0.1f && d[1] < 0.1f && d[2] < 0.1f), "Bad SIMD normal");

			src += 4;
			cmp += 4;

			for (int j = 0; j < 4; ++j) {
				d[j] = math::Abs(src[j]-cmp[j]);
			}

			RAD_VERIFY_MSG((d[0] < 0.1f && d[1] < 0.1f && d[2] < 0.1f && d[3] == 0.f), "Bad SIMD tangent");

			src += 4;
			cmp += 4;
		}
		
		zone_free(refFloats);
		zone_free(simdFloats);
	}
	
#endif

// time-test for SIMD
#if defined(RAD_OPT_TOOLS) || (defined(RAD_OPT_IOS_DEVICE) && !defined(RAD_OPT_SHIP))
//	SIMDSkinTest(COut(C_Info));
#endif

	return true;
}
#endif

void AbductedApp::OnTick(float dt) {
#if defined(RAD_TARGET_GOLDEN) || defined(RAD_OPT_IOS)
	if (m_game) {
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
	return "00000000000000000000";
#else
	return "00000000000000000000";
#endif
}

#if defined(RAD_OPT_IOS)
int AbductedApp::DoLauncher() {
	return 0;
}
#endif


