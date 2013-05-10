/*! \file Shader.c
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup renderer
*/

// Uniform/Varying/Attributes mean what they mean in GLSL

#if defined(_GLSL) && defined(VERTEX)
invariant gl_Position;
#endif

BEGIN_UNIFORMS
#if defined(SKIN_SPRITE)
	GLSL(uniform) FLOAT4X4 UDECL(mv);
	GLSL(uniform) FLOAT4X4 UDECL(pr);
#elif defined(_GLES)
	GLSL(uniform) FLOAT4X4 UDECL(mvp);
#endif
#if defined(SHADER_COLOR) || defined(SHADER_VERTEX_COLOR)
	GLSL(uniform) PRECISION_COLOR_TYPE UDECL(color) HLSL(:COLOR0);
#endif
#if defined(SHADER_SPECULAR_COLORS)
	GLSL(uniform) HALF4 UDECL(scolor) HLSL(:COLOR1);
#endif
#if defined(GENREFLECT) || (defined(LIGHTS) && (defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)))
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
#if defined(SHADER_VERTEX_COLOR)
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
#if defined(SHADER_POSITION)
	GLSL(varying) FLOAT4 OUT(position) HLSL(:POSITION);
#endif
#if defined(SHADER_VERTEX_COLOR)
	GLSL(varying) FIXED4 OUT(vertexColor) HLSL(:POSITION1);
#endif
	OUT_TCREGS
	OUT_NMREGS
	OUT_LIGHTREGS
END_VARYING

#if defined(FRAGMENT)

HALF4 DiffuseLightPixel(
	HALF4 lightVertex,
	HALF3 lightVec,
	HALF4 lightDfColor, // w contains brightness
	HALF3 normal,
	HALF4 diffuseColor
) {
	HALF e = length(lightVertex.xyz);
	e = max(HALF(0.0), lightVertex.w - e) / lightVertex.w;
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
	HALF e = length(lightVertex.xyz);
	e = max(HALF(0.0), lightVertex.w - e) / lightVertex.w;
	e = e * lightDfColor.w;
	HALF r = max(HALF(0.0), dot(lightVec, normal));
	HALF3 d = (lightDfColor.xyz * diffuseColor.xyz) * r;
	r = max(HALF(0.0), dot(lightHalf, normal));
	HALF3 s = (lightSpColor * specularColor.xyz) * pow(r, specularExponent);
	return HALF4((d+s) * e, diffuseColor.w);
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
	FIXED3 v_bitan0 = IN(tan0).w * cross(IN(nm0), IN(tan0).xyz);
#endif

#if defined(GENREFLECT) || (defined(LIGHTS) && (defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)))
	FIXED3 vn_eyevec = normalize(UNIFORM(eye) - IN(position).xyz);
#endif

#if defined(GENREFLECT)
	HALF3 genReflectTC_ = reflect(HALF3(IN(nm0)), HALF3(vn_eyevec));
	HALF4 genReflectTC = HALF4(genReflectTC_.x, genReflectTC_.y, genReflectTC_.z, 1.f);
#endif

	TCGEN
	TCMOD

#if defined(LIGHTS)
#if defined(SHADER_LIGHT_VEC) || defined(SHADER_LIGHTVERTEXPOS) || defined(SHADER_LIGHT_TANVEC) || defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)
	HALF3 v_light0_dir = UNIFORM(light0_pos).xyz - IN(position).xyz;
#endif
#if defined(SHADER_LIGHT_VEC) || defined(SHADER_LIGHT_TANVEC) || defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)
	HALF3 v_light0_vec = normalize(v_light0_dir);
#endif
#if defined(SHADER_LIGHT_VEC)
	OUT(light0_vec) = v_light0_vec;
#endif
#if defined(SHADER_LIGHT_VERTEXPOS)
	OUT(light0_vpos).xyz = -v_light0_dir;
	OUT(light0_vpos).w = HALF(UNIFORM(light0_pos).w);
#endif
#if defined(SHADER_LIGHT_TANVEC)
	OUT(light0_tanvec).x = dot(v_light0_vec, IN(tan0).xyz);
	OUT(light0_tanvec).y = dot(v_light0_vec, v_bitan0);
	OUT(light0_tanvec).z = dot(v_light0_vec, IN(nm0));
#endif
#if defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANHALFVEC)
	HALF3 v_light0_halfvec = (v_light0_vec + vn_eyevec) * 0.5f;
#if defined(SHADER_LIGHT_HALFVEC)
	OUT(light0_halfvec) = v_light0_halfvec;
#endif
#if defined(SHADER_LIGHT_TANHALFVEC)
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
	OUT(position) = IN(position);
#endif
#if defined(NUM_SHADER_NORMALS)
	OUT(nm0) = IN(nm0);
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
