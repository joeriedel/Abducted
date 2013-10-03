/*! \file Shader.c
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup renderer
*/

// Uniform/Varying/Attributes mean what they mean in GLSL

BEGIN_UNIFORMS
#if defined(SHADER_MV) || defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD) || defined(SHADER_EYE_VERTEX)
	GLSL(uniform) FLOAT4X4 UDECL(mv);
#endif
#if defined(SHADER_PRJ) || defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD)
#if defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD)
	GLSL(uniform) FLOAT4X4 UDECL(prj);
#else
	GLSL(uniform) PFLOAT4X4 UDECL(prj); // fog hack
#endif
#endif
#if defined(_GLES) && !(defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD))
	GLSL(uniform) FLOAT4X4 UDECL(mvp);
#endif
#if defined(SHADER_INVERSE_MV) || ((defined(SKIN_SPRITE)||defined(SKIN_BILLBOARD)) && (defined(SHADER_POSITION) || defined(LIGHTS) || defined(GEN_REFLECT) || defined(GEN_PROJECT)))
	// sprite skin may need worldspace vertex positions for lighting
	// or texture projection, or reflection gen.
	GLSL(uniform) FLOAT4X4 UDECL(imv);
#endif
#if defined(SHADER_INVERSE_MVP)
	GLSL(uniform) FLOAT4X4 UDELC(imvp);
#endif
#if defined(SHADER_INVERSE_PRJ)
	GLSL(uniform) FLOAT4X4 UDECL(iprj);
#endif
#if defined(SHADER_COLOR) || defined(SHADER_VERTEX_COLOR)
	GLSL(uniform) PFLOAT4 UDECL(color) HLSL(:COLOR0);
#endif
#if defined(SHADER_SPECULAR_COLORS)
	GLSL(uniform) HALF4 UDECL(scolor) HLSL(:COLOR1);
#endif
#if defined(GENREFLECT) || (defined(LIGHTS) && (defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)))
	GLSL(uniform) FLOAT3 UDECL(eye) HLSL(:C0);
#endif
#if defined(GENPROJECT)
	GLSL(uniform) FLOAT4X4 UDECL(tcPrj);
#endif
#if defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD)
	GLSL(uniform) FLOAT2 UDECL(spriteVerts)[4];
#endif
#if defined(PFX_VARS)
	GLSL(uniform) HALF2 UDECL(pfxVars);
#endif
	UTXREGS
	UTCMODREGS
	ULIGHTREGS
END_UNIFORMS

#if defined(VERTEX)
BEGIN_ATTRIBUTES
// -> Per vertex attributes (used only by VertexShader)
	GLSL(attribute) FLOAT4 IN(position) HLSL(:POSITION);
#if defined(SHADER_VERTEX_COLOR)
	GLSL(attribute) FIXED4 IN(vertexColor) HLSL(:POSITION1);
#endif
#if defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD)
	GLSL(attribute) FLOAT4 IN(spriteSkin) HLSL(:POSITION2);
#endif
	IN_TCREGS
	IN_NMREGS
	IN_TANREGS
END_ATTRIBUTES
#endif

BEGIN_VARYING
// -> FShader
#if defined(SHADER_POSITION)
	GLSL(varying) FLOAT4 OUT(position) HLSL(:POSITION);
#endif
#if defined(SHADER_VERTEX_COLOR)
	GLSL(varying) FIXED4 OUT(vertexColor) HLSL(:POSITION1);
#endif
#if defined(SHADER_EYE_VERTEX)
	GLSL(varying) PFLOAT4 OUT(eyeVertex) HLSL(:POSITION3);
#endif
#if defined(PFX_VARS)
	GLSL(varying) HALF2 OUT(pfxShift)[4];
#endif
	OUT_TCREGS
	OUT_NMREGS
	OUT_LIGHTREGS
END_VARYING

#if defined(FRAGMENT)

HALF DistanceAttn(
	HALF4 lightVertex // w contains radius
) {
	FLOAT3 _lightVertex = lightVertex.xyz; // optimizer converts length to dot(x, x) which overflows mediump
	HALF e = length(_lightVertex);
	e = max(HALF(0.0), lightVertex.w - e) / lightVertex.w;
	return e;
}

HALF4 DiffuseLightPixel(
	HALF4 lightVertex,
	HALF3 lightVec,
	HALF4 lightDfColor, // w contains brightness
	HALF3 normal,
	HALF4 diffuseColor
) {
	HALF e = DistanceAttn(lightVertex);
	e = e * lightDfColor.w;
	HALF r = max(HALF(0.0), dot(lightVec, normal));
	return HALF4((lightDfColor.xyz * diffuseColor.xyz) * (r * e), diffuseColor.w);
}

HALF4 DiffuseSpecularLightPixel(
	HALF4 lightVertex, // w contains radius
	HALF3 lightVec,  
	HALF3 lightHalf,
	HALF4 lightDfColor, // w contains brightness
	HALF3 lightSpColor, // w contains power
	HALF3 normal, 
	HALF4 diffuseColor,
	HALF4 specularColor,
	HALF specularExponent
) {
	HALF e = DistanceAttn(lightVertex);
	e = e * lightDfColor.w;
	HALF r = max(HALF(0.0), dot(lightVec, normal));
	HALF3 d = (lightDfColor.xyz * diffuseColor.xyz);
	HALF rs = max(HALF(0.0), dot(lightHalf, normal));
	HALF3 s = (lightSpColor * specularColor.xyz) * pow(rs, specularExponent);
	return HALF4((d+s) * r * e, diffuseColor.w);
}

#endif

#include <fragment>

MAIN
{
	DECL_R

#if defined(VERTEX)
	float PI = 3.14159265358979323846264;
	float HALF_PI = 3.14159265358979323846264/2.0;
	
#if defined(TANGENT_FRAME) || defined(NUM_SHADER_BITANGENTS)
	HALF3 v_bitan0 = IN(tan0).w * cross(IN(nm0), IN(tan0).xyz);
#endif

#if defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD)
	int idx = int(IN(spriteSkin).w);
	FLOAT2 sprite_vertex = UNIFORM(spriteVerts)[idx];
	FLOAT2 sincos = FLOAT2(sin(IN(spriteSkin).z), cos(IN(spriteSkin).z));
	FLOAT4 rotate = sprite_vertex.xxyy * sincos.xyxy;
	sprite_vertex.x = rotate.y - rotate.z;
	sprite_vertex.y = rotate.w + rotate.x;
	sprite_vertex *= IN(spriteSkin).xy;
	FLOAT4 sprite_skin = mul(UNIFORM(mv), IN(position));
#if defined(SKIN_BILLBOARD)
	// only rotate around world Z, not pure eye-space
	FLOAT3 worldEyeZ = mul(FLOAT3X3(UNIFORM(mv)), FLOAT3(0,0,1));
	sprite_skin.x += sprite_vertex.x;
	sprite_skin.xyz += worldEyeZ*sprite_vertex.y;
#else
	sprite_skin.xy += sprite_vertex.xy;
#endif
#if defined(SHADER_POSITION) || defined(LIGHTS) || defined(GEN_REFLECT) || defined(GEN_PROJECT)
	FLOAT4 vertex = mul(UNIFORM(imv), sprite_skin);
#endif
#else
	FLOAT4 vertex = IN(position);
#endif

#if defined(GENREFLECT) || (defined(LIGHTS) && (defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)))
	FLOAT3 v_eyevec = UNIFORM(eye) - vertex.xyz;
	HALF3 vn_eyevec = normalize(v_eyevec);
#endif

#if defined(GENREFLECT)
	HALF4 genReflectTC = HALF4(reflect(IN(nm0), vn_eyevec).xy, 0.0, 1.0);
#endif

#if defined(GENPROJECT)
	HALF4 genProjectTC = mul(UNIFORM(tcPrj), vertex);
#endif

	TCGEN
	TCMOD

#if defined(LIGHTS)
#if defined(SHADER_LIGHT_VEC) || defined(SHADER_LIGHT_VERTEXPOS) || defined(SHADER_LIGHT_TANVEC) || defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)
#if (SHADER_LIGHT_POS >= 4) || (SHADER_LIGHT_VERTEXPOS >= 4) || (SHADER_LIGHT_VEC >= 4) || (SHADER_LIGHT_HALFVEC >= 4) || (SHADER_LIGHT_TANVEC >= 4) || (SHADER_LIGHT_TANHALFVEC >= 4)
	FLOAT3 v_light3_dir = UNIFORM(light3_pos).xyz - vertex.xyz;
#endif
#if (SHADER_LIGHT_POS >= 3) || (SHADER_LIGHT_VERTEXPOS >= 3) || (SHADER_LIGHT_VEC >= 3) || (SHADER_LIGHT_HALFVEC >= 3) || (SHADER_LIGHT_TANVEC >= 3) || (SHADER_LIGHT_TANHALFVEC >= 3)
	FLOAT3 v_light2_dir = UNIFORM(light2_pos).xyz - vertex.xyz;
#endif
#if (SHADER_LIGHT_POS >= 2) || (SHADER_LIGHT_VERTEXPOS >= 2) || (SHADER_LIGHT_VEC >= 2) || (SHADER_LIGHT_HALFVEC >= 2) || (SHADER_LIGHT_TANVEC >= 2) || (SHADER_LIGHT_TANHALFVEC >= 2)
	FLOAT3 v_light1_dir = UNIFORM(light1_pos).xyz - vertex.xyz;
#endif
	FLOAT3 v_light0_dir = UNIFORM(light0_pos).xyz - vertex.xyz;
#endif
#if defined(SHADER_LIGHT_VEC) || defined(SHADER_LIGHT_TANVEC) || defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)
#if (SHADER_LIGHT_VEC >= 4) || (SHADER_LIGHT_TANVEC >= 4) || (SHADER_LIGHT_HALFVEC >= 4) || (SHADER_LIGHT_TANHALFVEC >= 4)
	HALF3 v_light3_vec = normalize(v_light3_dir);
#endif
#if (SHADER_LIGHT_VEC >= 3) || (SHADER_LIGHT_TANVEC >= 3) || (SHADER_LIGHT_HALFVEC >= 3) || (SHADER_LIGHT_TANHALFVEC >= 3)
	HALF3 v_light2_vec = normalize(v_light2_dir);
#endif
#if (SHADER_LIGHT_VEC >= 2) || (SHADER_LIGHT_TANVEC >= 2) || (SHADER_LIGHT_HALFVEC >= 2) || (SHADER_LIGHT_TANHALFVEC >= 2)
	HALF3 v_light1_vec = normalize(v_light1_dir);
#endif
	HALF3 v_light0_vec = normalize(v_light0_dir);
#endif
#if defined(SHADER_LIGHT_VEC)
#if (SHADER_LIGHT_VEC >= 4)
	OUT(light3_vec) = v_light3_vec;
#endif
#if (SHADER_LIGHT_VEC >= 3)
	OUT(light2_vec) = v_light2_vec;
#endif
#if (SHADER_LIGHT_VEC >= 2)
	OUT(light1_vec) = v_light1_vec;
#endif
	OUT(light0_vec) = v_light0_vec;
#endif
#if defined(SHADER_LIGHT_VERTEXPOS)
#if (SHADER_LIGHT_VERTEXPOS >= 4)
	OUT(light3_vpos).xyz = v_light3_dir;
	OUT(light3_vpos).w = HALF(UNIFORM(light3_pos).w);
#endif
#if (SHADER_LIGHT_VERTEXPOS >= 3)
	OUT(light2_vpos).xyz = v_light2_dir;
	OUT(light2_vpos).w = HALF(UNIFORM(light2_pos).w);
#endif
#if (SHADER_LIGHT_VERTEXPOS >= 2)
	OUT(light1_vpos).xyz = v_light1_dir;
	OUT(light1_vpos).w = HALF(UNIFORM(light1_pos).w);
#endif
	OUT(light0_vpos).xyz = v_light0_dir;
	OUT(light0_vpos).w = HALF(UNIFORM(light0_pos).w);
#endif
#if defined(SHADER_LIGHT_TANVEC)
// NOTE use of light_3_tanvec, see Common.c comments.
#if (SHADER_LIGHT_TANVEC >= 4)
	OUT(light_3_tanvec).x = dot(v_light3_vec, IN(tan0).xyz);
	OUT(light_3_tanvec).y = dot(v_light3_vec, v_bitan0);
	OUT(light_3_tanvec).z = dot(v_light3_vec, IN(nm0));
#endif
#if (SHADER_LIGHT_TANVEC >= 3)
	OUT(light2_tanvec).x = dot(v_light2_vec, IN(tan0).xyz);
	OUT(light2_tanvec).y = dot(v_light2_vec, v_bitan0);
	OUT(light2_tanvec).z = dot(v_light2_vec, IN(nm0));
#endif
#if (SHADER_LIGHT_TANVEC >= 2)
	OUT(light1_tanvec).x = dot(v_light1_vec, IN(tan0).xyz);
	OUT(light1_tanvec).y = dot(v_light1_vec, v_bitan0);
	OUT(light1_tanvec).z = dot(v_light1_vec, IN(nm0));
#endif
	OUT(light0_tanvec).x = dot(v_light0_vec, IN(tan0).xyz);
	OUT(light0_tanvec).y = dot(v_light0_vec, v_bitan0);
	OUT(light0_tanvec).z = dot(v_light0_vec, IN(nm0));
#endif
#if defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)
#if (SHADER_LIGHT_HALFVEC >= 4) || (SHADER_LIGHT_TANHALFVEC >= 4)
	HALF3 v_light3_halfvec = normalize((v_light3_vec + vn_eyevec) * 0.5f);
#endif
#if (SHADER_LIGHT_HALFVEC >= 3) || (SHADER_LIGHT_TANHALFVEC >= 3)
	HALF3 v_light2_halfvec = normalize((v_light2_vec + vn_eyevec) * 0.5f);
#endif
#if (SHADER_LIGHT_HALFVEC >= 2) || (SHADER_LIGHT_TANHALFVEC >= 2)
	HALF3 v_light1_halfvec = normalize((v_light1_vec + vn_eyevec) * 0.5f);
#endif
	HALF3 v_light0_halfvec = normalize((v_light0_vec + vn_eyevec) * 0.5f);
#if defined(SHADER_LIGHT_HALFVEC)
#if (SHADER_LIGHT_HALFVEC >= 4)
	OUT(light3_halfvec) = v_light3_halfvec;
#endif
#if (SHADER_LIGHT_HALFVEC >= 3)
	OUT(light2_halfvec) = v_light2_halfvec;
#endif
#if (SHADER_LIGHT_HALFVEC >= 2)
	OUT(light1_halfvec) = v_light1_halfvec;
#endif
	OUT(light0_halfvec) = v_light0_halfvec;
#endif
#if defined(SHADER_LIGHT_TANHALFVEC)
#if (SHADER_LIGHT_TANHALFVEC >= 4)
	OUT(light3_tanhalfvec).x = dot(v_light3_halfvec, IN(tan0).xyz);
	OUT(light3_tanhalfvec).y = dot(v_light3_halfvec, v_bitan0);
	OUT(light3_tanhalfvec).z = dot(v_light3_halfvec, IN(nm0));
#endif
#if (SHADER_LIGHT_TANHALFVEC >= 3)
	OUT(light2_tanhalfvec).x = dot(v_light2_halfvec, IN(tan0).xyz);
	OUT(light2_tanhalfvec).y = dot(v_light2_halfvec, v_bitan0);
	OUT(light2_tanhalfvec).z = dot(v_light2_halfvec, IN(nm0));
#endif
#if (SHADER_LIGHT_TANHALFVEC >= 2)
	OUT(light1_tanhalfvec).x = dot(v_light1_halfvec, IN(tan0).xyz);
	OUT(light1_tanhalfvec).y = dot(v_light1_halfvec, v_bitan0);
	OUT(light1_tanhalfvec).z = dot(v_light1_halfvec, IN(nm0));
#endif
	OUT(light0_tanhalfvec).x = dot(v_light0_halfvec, IN(tan0).xyz);
	OUT(light0_tanhalfvec).y = dot(v_light0_halfvec, v_bitan0);
	OUT(light0_tanhalfvec).z = dot(v_light0_halfvec, IN(nm0));
#endif
#endif
#endif

#include <vertex>

#if defined(SHADER_VERTEX_COLOR)
	OUT(vertexColor) = IN(vertexColor) * UNIFORM(color);
#endif
#if defined(SHADER_POSITION)
	OUT(position) = vertex;
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
#if defined(SKIN_SPRITE) || defined(SKIN_BILLBOARD)
	gl_Position = mul(UNIFORM(prj), sprite_skin);
#if defined(SHADER_EYE_VERTEX)
	OUT(eyeVertex) = sprite_skin;
#endif
#else
	gl_Position = mul(MVP, vertex);
#if defined(SHADER_EYE_VERTEX)
	OUT(eyeVertex) = mul(UNIFORM(mv), vertex);
#endif
#endif
#if defined(PFX_VARS) && (TEXCOORDS>=1)
	// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

	HALF2 pfxShiftOne = HALF2(1.3846153846 * UNIFORM(pfxVars)[0], 1.3846153846 * UNIFORM(pfxVars)[1]);
	HALF2 pfxShiftTwo = HALF2(3.2307692308 * UNIFORM(pfxVars)[0], 3.2307692308 * UNIFORM(pfxVars)[1]);
	OUT(pfxShift)[0] = OUT(TEXCOORD0).xy - pfxShiftOne;
	OUT(pfxShift)[1] = OUT(TEXCOORD0).xy + pfxShiftOne;
	OUT(pfxShift)[2] = OUT(TEXCOORD0).xy - pfxShiftTwo;
	OUT(pfxShift)[3] = OUT(TEXCOORD0).xy + pfxShiftTwo;
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
