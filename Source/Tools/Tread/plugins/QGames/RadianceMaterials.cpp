// RadianceMaterials.cpp
// Copyright (c) 2012 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#include "stdafx.h"
extern "C" {
#include "lua-5.1.5/Lua/lua.h"
#include "lua-5.1.5/Lua/lauxlib.h"
}
#include <map>

namespace {

const char *sz_luaAddHelper = "function add(x) if (x.type == \"Texture\") then add_texture(x.name, x.Source.File) elseif ((x.type == \"Material\") and (x.Texture1 ~= nil) and (x.Texture1.Source.Texture ~= \"\")) then add_material(x.name, x.Texture1.Source.Texture) end end";
#define PKG_THIS "@pkg"

struct MaterialInfo {
	typedef std::map<CString, MaterialInfo> Map;
	CString name;
	CString texture;
	CString pkg;
};

class TextureShader : public CShader {
public:
	TextureShader(
		const char *name,
		const Image_t &image
	) : m_name(name), m_image(image), m_tex(0) {
		m_bpp = (image.DataType == IMAGE888) ? 3 : 4;
	}

	~TextureShader() {
		Free();
	}

	struct RTex_s *Load() {
		if (m_tex)
			return m_tex;
		m_tex = R_glCreateTexture(
			m_name,
			(int)m_image.Width,
			(int)m_image.Height,
			1,
			m_bpp,
			GL_TexUpload,
			0,
			0
		);

		m_tex->user_data[0] = this;
		return m_tex;
	}

	void Purge() {}
	bool Pickable() { return true; }
	const char *Name() { return m_name; }
	const char *DisplayName() { return m_name; }

	void Dimensions(int *w, int *h) {
		*w = (int)m_image.Width;
		*h = (int)m_image.Height;
	}

	int MemorySize() {
		return m_image.Width*m_image.Height*m_bpp;
	}

private:

	void Free() {
		if (m_image.ImagePtr) {
			DeallocAPointer(m_image.ImagePtr);
			m_image.ImagePtr = 0;
		}
	}

	static bool GL_TexUpload(RTex_t *tex) {
		TextureShader *self = reinterpret_cast<TextureShader*>(tex->user_data[0]);

		R_glUploadTexture(
			tex,
			(int)self->m_image.Width,
			(int)self->m_image.Height,
			1,
			GL_UNSIGNED_BYTE,
			(self->m_bpp == 3) ? GL_RGB : GL_RGBA,
			_upf_wrap|_upf_filter|_upf_mipmap,
			self->m_image.ImagePtr,
			0
		);

		self->Free();
		return true;
	}

	CString m_name;
	Image_t m_image;
	RTex_t *m_tex;
	int m_bpp;
};

class RadiancePackage {
public:

	bool Load(
		const char *name, 
		const char *szPath
	) {
		m_name = name;

		lua_State *L = luaL_newstate();

		lua_pushcfunction(L, lua_AddTexture);
		lua_setfield(L, LUA_GLOBALSINDEX, "add_texture");
		lua_pushcfunction(L, lua_AddMaterial);
		lua_setfield(L, LUA_GLOBALSINDEX, "add_material");

		lua_pushlightuserdata(L, this);
		lua_setfield(L, LUA_GLOBALSINDEX, PKG_THIS);

		if (luaL_dostring(L, sz_luaAddHelper)) {
			Sys_printf("ERROR: initializing lua, '%s'\n", lua_tostring(L, -1));
			lua_close(L);
			return false;
		}

		if (luaL_dofile(L, szPath)) {
			Sys_printf("ERROR: loading package '%s' -> '%s'\n", name, lua_tostring(L, -1));
			return false;
		}

		lua_close(L);

		return true;
	}

	CShader *ShaderForTexture(const char *baseDirectory, const char *materialName, const char *textureName) {
		CString name(textureName);
		Texture::Map::iterator it = m_textures.find(name);
		if (it == m_textures.end())
			return 0;

		const Texture &texture = it->second;
		
		CString ext;
		for (int i = 0; i < texture.image.GetLength(); ++i) {
			if (texture.image[i] == '.') {
				ext = texture.image.Right(texture.image.GetLength() - i - 1);
				break;
			}
		}

		Image_t src;
		void *data;
		int size;

		if(!(data=Load(CString(baseDirectory) + "/" + texture.image, &size))) {
			Sys_printf("ERROR: reading file '%s'\n", texture.image);
			return 0;
		}

		ext.MakeLower();
		if (ext == "tga") {
			if (ImageParseTGA(&src, (const Byte*)data)) {
				free(data);
				Sys_printf("ERROR: '%s' is not a targa file.\n", texture.image);
				return 0;
			}
		} else if (ext == "bmp") {
			if (ImageParseBMP(&src, (const Byte*)data)) {
				free(data);
				Sys_printf("ERROR: '%s' is not a bmp file.\n", texture.image);
				return 0;
			}
		} else {
			free(data);
			Sys_printf("ERROR: '%s' is not a supported image format.\n", texture.image);
			return 0;
		}

		// too big?
		if ((src.Width > 1024) || (src.Height > 1024)) {
			int w, h;

			if (src.Width > src.Height) {
				float x = 1024.f / src.Width;
				w = src.Width * x;
				h = src.Height * x;
			} else {
				float x = 1024.f / src.Height;
				w = src.Width * x;
				h = src.Height * x;
			}

			w = (w<1) ? 1 : w;
			h = (h<1) ? 1 : h;


			// gluScale image has overrun problems, so allocate some extra space

			Image_t dst;
			ImageInit(&dst, w, h + 8, src.DataType);
			dst.Height = h;

			if (src.DataType == IMAGE888) {
				ImageStore888(&dst, &src);
			} else {
				ImageStore8888(&dst, &src);
			}

			std::swap(src, dst);
			ImageDestroy(&dst);

		}

		TextureShader *shader = new TextureShader(
			materialName,
			src
		);

		return shader;
	}

	const MaterialInfo::Map &Materials() const {
		return m_materials;
	}

private:

	struct Texture {
		typedef std::map<CString, Texture> Map;
		CString name;
		CString image;
	};

	static int lua_AddTexture(lua_State *L) {
		lua_getfield(L, LUA_GLOBALSINDEX, PKG_THIS);
		RadiancePackage *self = reinterpret_cast<RadiancePackage*>(lua_touserdata(L, -1));
		
		Texture t;
		t.name = luaL_checkstring(L, 1);
		t.image = luaL_checkstring(L, 2);

		self->m_textures.insert(Texture::Map::value_type(t.name, t));
		return 0;
	}

	static int lua_AddMaterial(lua_State *L) {
		lua_getfield(L, LUA_GLOBALSINDEX, PKG_THIS);
		RadiancePackage *self = reinterpret_cast<RadiancePackage*>(lua_touserdata(L, -1));

		MaterialInfo m;

		m.name = self->m_name + "/" + luaL_checkstring(L, 1);

		char pkgName[1024];
		char texName[1024];

		const char *sz = luaL_checkstring(L, 2);
		for (int i = 0; sz[i]; ++i) {
			if (sz[i] == '/') {
				strncpy(pkgName, sz, i);
				pkgName[i] = 0;
				
				int len = strlen(sz);
				strncpy(texName, &sz[i+1], len-i-1);
				texName[len-i-1] = 0;
				break;
			}
		}

		m.pkg = pkgName;
		m.texture = texName;

		self->m_materials.insert(MaterialInfo::Map::value_type(m.name, m));
		return 0;
	}

	static void *Load(const char *path, int *size) {
		FILE *fp = fopen(path, "rb");
		if (!fp)
			return 0;
		fseek(fp, 0, SEEK_END);
		*size = (int)ftell(fp);
		fseek(fp, 0, SEEK_SET);

		void *data = malloc(*size);
		fread(data, 1, *size, fp);
		fclose(fp);

		return data;
	}

	Texture::Map m_textures;
	MaterialInfo::Map m_materials;
	CString m_name;
};

typedef std::map<CString, RadiancePackage*> PackageMap;
typedef std::map<CString, CShader*> ShaderMap;

}

void LoadRadianceMaterials(const char *baseDirectory, CLinkedList<CShader> &shaders) {

	Sys_printf("Loading Radiance Packages...\n");

	CString path(baseDirectory);
	path += "/Packages/*.pkg";

	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA(path, &fd);

	if (h == 0) {
		Sys_printf("ERROR: no packages found @ '%s'.", (LPCTSTR)path);
		return;
	}

	PackageMap packages;

	do {
		if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
			
			CString pkgPath(baseDirectory);
			pkgPath += "/Packages/";
			pkgPath += fd.cFileName;

			char szBuff[1024];
			strcpy(szBuff, fd.cFileName);

			int len = strlen(szBuff);
			for (int i = len-1; i >= 0; --i) {
				if (szBuff[i] == '.') {
					szBuff[i] = 0;
					break;
				}
			}

			Sys_printf("%s\n", szBuff);

			RadiancePackage *pkg = new RadiancePackage();
			if (pkg->Load(szBuff, pkgPath)) {
				packages.insert(PackageMap::value_type(CString(szBuff), pkg));
			} else {
				delete pkg;
			}
		}
	} while(FindNextFile(h, &fd));

	FindClose(h);

	// create shaders for all materials.
	for (PackageMap::iterator it = packages.begin(); it != packages.end(); ++it) {
		RadiancePackage &package = *it->second;
		for (MaterialInfo::Map::const_iterator it = package.Materials().begin(); it != package.Materials().end(); ++it) {
			const MaterialInfo &material = it->second;

			PackageMap::iterator texPkgIt = packages.find(material.pkg);
			if (texPkgIt != packages.end()) {
				RadiancePackage &texPkg = *texPkgIt->second;

				CShader *shader = texPkg.ShaderForTexture(baseDirectory, material.name, material.texture);
				if (shader) {
					Sys_printf("Loaded: %s\n", material.name);
					shaders.AddItem(shader);
				} else {
					Sys_printf("ERROR: loading '%s'\n", material.name);
				}
			}

		}
	}

	// cleanup
	for (PackageMap::iterator it = packages.begin(); it != packages.end(); ++it) {
		delete it->second;
	}
}
