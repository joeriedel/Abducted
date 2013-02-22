/*! \file Shader.c
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup renderer
*/

// Uniform/Varying/Attributes mean what they mean in GLSL

BEGIN_UNIFORMS
#if defined(SKIN_SPRITE)
	GLSL(uniform) FLOAT4X4 UDECL(mv);
	GLSL(uniform) FLOAT4X4 UDECL(pr);
#elif defined(_GLES)
	GLSL(uniform) FLOAT4X4 UDECL(mvp);
#endif
#if defined(_GLES) || defined(_HLSL)
	GLSL(uniform) FIXED4 UDECL(color) HLSL(:COLOR0);
#endif
#if defined(GENREFLECT)
	GLSL(uniform) FLOAT3 UDECL(eye) HLSL(:C0);
#endif
#if defined(SKIN_SPRITE)
	GLSL(uniform) FLOAT2 UDECL(spriteVerts)[4];
#endif
	UTXREGS
	UTCMODREGS
	ULIGHTREGS
END_UNIFORMS

#if defined(VERTEX)
BEGIN_ATTRIBUTES
// -> Per vertex attributes (used only by VertexShader)
	GLSL(attribute) FLOAT4 IN(position) HLSL(:POSITION);
#if defined(VERTEX_COLOR)
	GLSL(attribute) FIXED4 IN(vertexColor) HLSL(:POSITION1);
#endif
#if defined(SKIN_SPRITE)
	GLSL(attribute) FLOAT4 IN(spriteSkin) HLSL(:POSITION2);
#endif
	IN_TCREGS
	IN_NMREGS
	IN_TANREGS
END_ATTRIBUTES
#endif

BEGIN_VARYING
// -> FShader
#if defined(SHADER_VERTEX)
	GLSL(varying) FLOAT4 OUT(position) HLSL(:POSITON);
#endif
#if defined(VERTEX_COLOR)
	GLSL(varying) FIXED4 OUT(vertexColor) HLSL(:POSITION1);
#endif
#if defined(_GLES)
	varying FIXED4 frag_color;
#endif
	OUT_TCREGS
	OUT_NMREGS
	OUT_TANREGS
	OUT_BITANREGS
	OUT_LIGHTREGS
END_VARYING

#include <fragment>

MAIN
{
	DECL_R

#if defined(VERTEX)
	float PI = 3.14159265358979323846264;
	float HALF_PI = 3.14159265358979323846264/2.0;
	
#if defined(TANGENT_FRAME) || defined(NUM_SHADER_BITANGENTS)
	FLOAT3 v_bitan0 = IN(tan0).w * cross(IN(nm0), IN(tan0).xyz);
#endif

#if defined(GENREFLECT) || (defined(LIGHTS) && defined(SHADER_LIGHT_HALFDIR))
	FLOAT3 v_eyevec = normalize(UNIFORM(eye) - IN(position).xyz);
#endif

#if defined(GENREFLECT)
	FLOAT3 genReflectTC_ = reflect(IN(nm0), v_eyevec);
	FLOAT4 genReflectTC = FLOAT4(genReflectTC_.x, genReflectTC_.y, genReflectTC_.z, 1.f);
#endif

	TCGEN
	TCMOD

#if defined(LIGHTS)
#if defined(SHADER_LIGHT_DIR) || defined(SHADER_LIGHT_HALFDIR)
	FLOAT3 v_light0_dir = normalize(UNIFORM(light0_pos) - IN(position).xyz);
#endif
#if defined(SHADER_LIGHT_DIR)
	// should i pack tangent frame in a matrix?
	OUT(light0_dir).x = dot(v_light0_dir, IN(tangent).xyz);
	OUT(light0_dir).y = dot(v_light0_dir, v_bitan0);
	OUT(light0_dir).z = dot(v_light0_dir, IN(nm0));
#endif
#if defined(SHADER_LIGHT_HALFDIR)
	FLOAT3 v_light0_halfdir = (v_light0_dir + v_eyevec) * 0.5f;
	OUT(light0_halfdir).x = dot(v_light0_halfdir, IN(tan0).xyz);
	OUT(light0_halfdir).y = dot(v_light0_halfdir, v_bitan0);
	OUT(light0_halfdir).z = dot(v_light0_halfdir, IN(nm0));
#endif
#endif

#if defined(VERTEX_COLOR)
	OUT(vertexColor) = IN(vertexColor);
#endif

#if defined(_GLES)
	frag_color = DCOLOR0;
#elif defined(_GLSL)
	gl_FrontColor = DCOLOR0;
#endif
#include <vertex>
#if defined(SHADER_POSITION)
	OUT(position) = IN(position);
#endif
#if defined(NUM_SHADER_NORMALS)
	OUT(nm0) = IN(nm0);
#endif
#if defined(NUM_SHADER_TANGENTS)
	OUT(tan0) = IN(tan0);
#endif
#if defined(NUM_SHADER_BITANGENTS)
	OUT(bitan0) = v_bitan0;
#endif
#if defined(SKIN_SPRITE)
	int idx = int(IN(spriteSkin).w);
	FLOAT2 sprite_vertex = UNIFORM(spriteVerts)[idx];
	FLOAT2 sincos = FLOAT2(sin(IN(spriteSkin).z), cos(IN(spriteSkin).z));
	FLOAT4 rotate = sprite_vertex.xxyy * sincos.xyxy;
	sprite_vertex.x = rotate.y - rotate.z;
	sprite_vertex.y = rotate.w + rotate.x;
	sprite_vertex *= IN(spriteSkin).xy;
	FLOAT4 mvpos = mul(UNIFORM(mv), IN(position));
	mvpos.xy += sprite_vertex.xy;
	gl_Position = mul(UNIFORM(pr), mvpos);
#else
	gl_Position = mul(MVP, IN(position));
#endif
#else // !defined(VERTEX)

	M m = _Material(P_GLOBALS);
	gl_FragColor = FOG(m.color);

#if defined(DEPTH)
	gl_FragDepth = m.depth;
#endif

#endif // defined(VERTEX)
	
	RETURN
}
