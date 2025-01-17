// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include "base/basictypes.h"
#include "Globals.h"
#include "math/lin/matrix4x4.h"

#include <map>

#include "GPU/Common/ShaderCommon.h"
#include "GPU/Common/ShaderId.h"
#include "GPU/GLES/VertexShaderGenerator.h"
#include "GPU/GLES/GeometryShaderGenerator.h"
#include "GPU/GLES/FragmentShaderGenerator.h"

// Virtual Reality stuff
enum ViewportType {
	VIEW_FULLSCREEN = 0,
	VIEW_LETTERBOXED,
	VIEW_HUD_ELEMENT,
	VIEW_SKYBOX,
	VIEW_PLAYER_1,
	VIEW_PLAYER_2,
	VIEW_PLAYER_3,
	VIEW_PLAYER_4,
	VIEW_OFFSCREEN,
	VIEW_RENDER_TO_TEXTURE,
};
extern enum ViewportType g_viewport_type, g_old_viewport_type;
//void ScaleRequestedToRendered(EFBRectangle *requested, EFBRectangle *rendered);
//extern EFBRectangle g_final_screen_region, g_requested_viewport, g_rendered_viewport;
extern bool debug_newScene;
extern bool m_layer_on_top;


class Shader;

// Pre-fetched attrs and uniforms
enum {
	ATTR_POSITION = 0,
	ATTR_TEXCOORD = 1,
	ATTR_NORMAL = 2,
	ATTR_W1 = 3,
	ATTR_W2 = 4,
	ATTR_COLOR0 = 5,
	ATTR_COLOR1 = 6,

	ATTR_COUNT,
};

class LinkedShader {
public:
	LinkedShader(Shader *vs, Shader *gs, Shader *fs, u32 vertType, bool useHWTransform, LinkedShader *previous, bool isClear);
	~LinkedShader();

	u32 use(u32 vertType, LinkedShader *previous, bool isClear);
	void stop();
	u32 UpdateUniforms(u32 vertType, bool isClear);
	Matrix4x4 SetProjectionConstants(float input_proj_matrix[], bool shouldLog, bool isThrough);
	void CheckOrientationConstants();

	Shader *vs_, *gs_;
	// Set to false if the VS failed, happens on Mali-400 a lot for complex shaders.
	bool useHWTransform_;

	uint32_t program;
	u32 availableUniforms;
	u32 dirtyUniforms;

	// Present attributes in the shader.
	int attrMask;  // 1 << ATTR_ ... or-ed together.

	int u_stencilReplaceValue;
	int u_tex;
	int u_proj;
	int u_proj_through;
	int u_StereoParams;
	int u_texenv;
	int u_view;
	int u_texmtx;
	int u_world;
	int u_depthRange;   // x,y = viewport xscale/xcenter. z,w=clipping minz/maxz (?)

#ifdef USE_BONE_ARRAY
	int u_bone;  // array, size is numBones
#else
	int u_bone[8];
#endif
	int numBones;

	// Shader blending.
	int u_fbotex;
	int u_blendFixA;
	int u_blendFixB;
	int u_fbotexSize;

	// Fragment processing inputs
	int u_alphacolorref;
	int u_alphacolormask;
	int u_testtex;
	int u_fogcolor;
	int u_fogcoef;

	// Texturing
	int u_uvscaleoffset;
	int u_texclamp;
	int u_texclampoff;

	// Lighting
	int u_ambient;
	int u_matambientalpha;
	int u_matdiffuse;
	int u_matspecular;
	int u_matemissive;
	int u_lightpos[4];
	int u_lightdir[4];
	int u_lightatt[4];  // attenuation
	int u_lightangle[4]; // spotlight cone angle (cosine)
	int u_lightspotCoef[4]; // spotlight dropoff
	int u_lightdiffuse[4];  // each light consist of vec4[3]
	int u_lightspecular[4];  // attenuation
	int u_lightambient[4];  // attenuation
};

enum {
	DIRTY_PROJMATRIX = (1 << 0),
	DIRTY_PROJTHROUGHMATRIX = (1 << 1),
	DIRTY_FOGCOLOR = (1 << 2),
	DIRTY_FOGCOEF = (1 << 3),
	DIRTY_TEXENV = (1 << 4),
	DIRTY_ALPHACOLORREF = (1 << 5),

	// 1 << 6 is free! Wait, not anymore...
	DIRTY_STENCILREPLACEVALUE = (1 << 6),

	DIRTY_ALPHACOLORMASK = (1 << 7),
	DIRTY_LIGHT0 = (1 << 8),
	DIRTY_LIGHT1 = (1 << 9),
	DIRTY_LIGHT2 = (1 << 10),
	DIRTY_LIGHT3 = (1 << 11),

	DIRTY_MATDIFFUSE = (1 << 12),
	DIRTY_MATSPECULAR = (1 << 13),
	DIRTY_MATEMISSIVE = (1 << 14),
	DIRTY_AMBIENT = (1 << 15),
	DIRTY_MATAMBIENTALPHA = (1 << 16),

	DIRTY_SHADERBLEND = (1 << 17),  // Used only for in-shader blending.

	DIRTY_UVSCALEOFFSET = (1 << 18),  // this will be dirtied ALL THE TIME... maybe we'll need to do "last value with this shader compares"

	// Texclamp is fairly rare so let's share it's bit with DIRTY_DEPTHRANGE.
	DIRTY_TEXCLAMP = (1 << 19),
	DIRTY_DEPTHRANGE = (1 << 19),

	DIRTY_WORLDMATRIX = (1 << 21),
	DIRTY_VIEWMATRIX = (1 << 22),  // Maybe we'll fold this into projmatrix eventually
	DIRTY_TEXMATRIX = (1 << 23),
	DIRTY_BONEMATRIX0 = (1 << 24),
	DIRTY_BONEMATRIX1 = (1 << 25),
	DIRTY_BONEMATRIX2 = (1 << 26),
	DIRTY_BONEMATRIX3 = (1 << 27),
	DIRTY_BONEMATRIX4 = (1 << 28),
	DIRTY_BONEMATRIX5 = (1 << 29),
	DIRTY_BONEMATRIX6 = (1 << 30),
	DIRTY_BONEMATRIX7 = (1 << 31),

	DIRTY_ALL = 0xFFFFFFFF
};

// Real public interface

class Shader {
public:
	Shader(const char *code, uint32_t glShaderType, bool useHWTransform, const ShaderID &shaderID);
	~Shader();
	uint32_t shader;

	bool Failed() const { return failed_; }
	bool UseHWTransform() const { return useHWTransform_; }
	const ShaderID &ID() const { return id_; }

	std::string GetShaderString(DebugShaderStringType type) const;

private:
	std::string source_;
	ShaderID id_;
	bool failed_;
	bool useHWTransform_;
	bool isFragment_;
};

class ShaderManager {
public:
	ShaderManager();
	~ShaderManager();

	void ClearCache(bool deleteThem);  // TODO: deleteThem currently not respected

	// This is the old ApplyShader split into two parts, because of annoying information dependencies.
	// If you call ApplyVertexShader, you MUST call ApplyFragmentShader soon afterwards.
	Shader *ApplyVertexShader(int prim, u32 vertType);
	Shader *ApplyGeometryShader(int prim, u32 vertType);
	LinkedShader *ApplyFragmentShader(Shader *vs, Shader *gs, int prim, u32 vertType, bool isClear);

	void DirtyShader();
	void DirtyUniform(u32 what) {
		globalDirty_ |= what;
	}
	void DirtyLastShader();  // disables vertex arrays

	int NumVertexShaders() const { return (int)vsCache_.size(); }
	int NumGeometryShaders() const { return (int)gsCache_.size(); }
	int NumFragmentShaders() const { return (int)fsCache_.size(); }
	int NumPrograms() const { return (int)linkedShaderCache_.size(); }

	std::vector<std::string> DebugGetShaderIDs(DebugShaderType type);
	std::string DebugGetShaderString(std::string id, DebugShaderType type, DebugShaderStringType stringType);

private:
	void Clear();
	static bool DebugAreShadersCompatibleForLinking(Shader *vs, Shader *gs, Shader *fs);

	struct LinkedShaderCacheEntry {
		LinkedShaderCacheEntry(Shader *vs_, Shader *gs_, Shader *fs_, LinkedShader *ls_)
			: vs(vs_), gs(gs_), fs(fs_), ls(ls_) { }

		Shader *vs;
		Shader *gs;
		Shader *fs;
		LinkedShader *ls;
	};
	typedef std::vector<LinkedShaderCacheEntry> LinkedShaderCache;

	LinkedShaderCache linkedShaderCache_;

	bool lastVShaderSame_, lastGShaderSame_;

	ShaderID lastFSID_;
	ShaderID lastGSID_;
	ShaderID lastVSID_;

	LinkedShader *lastShader_;
	u32 globalDirty_;
	u32 shaderSwitchDirty_;
	char *codeBuffer_;

	typedef std::map<ShaderID, Shader *> FSCache;
	FSCache fsCache_;

	typedef std::map<ShaderID, Shader *> GSCache;
	GSCache gsCache_;

	typedef std::map<ShaderID, Shader *> VSCache;
	VSCache vsCache_;
};
