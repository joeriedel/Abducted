/*! \file GLSL.c
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup renderer
*/

#define _GLSL
#define GLSL(_x) _x
#define HLSL(_x)
#define float4x4 mat4x4
#define float3x3 mat3x3
#define mul(_a, _b) ((_a) * (_b))
#define lerp mix
#define sat(_x) clamp((_x), 0.0, 1.0)
#define tex2D(_a, _b) texture2D(_a, _b)
#define texCUBE(_a, _b) textureCube(_a, _b)
#define MAIN void main()
#define DECL_R
#define RETURN
#define OUT(_v) out_##_v
#if defined(VERTEX)
#define IN(_v) in_##_v
#else
#define IN(_v) out_##_v
#endif
#define BEGIN_ATTRIBUTES
#define END_ATTRIBUTES
#define BEGIN_VARYING
#define END_VARYING
#define SEL(_hlsl, _glsl) _glsl
#define P_GLOBALS
#define GLOBALS
#define VOID_GLOBALS
#define UNIFORM(_x) U_##_x
#define UDECL(_x) UNIFORM(_x)
#define BEGIN_UNIFORMS
#define END_UNIFORMS

#if defined(_GLES)
#define GLES(_x) _x
#define MVP U_mvp
#else
#define GLES(_x)
#define highp
#define mediump
#define lowp
#define MVP gl_ModelViewProjectionMatrix
#endif

#define FLOAT highp float
#define FLOAT2 highp vec2
#define FLOAT3 highp vec3
#define FLOAT4 highp vec4
#define FLOAT3X3 highp mat3
#define FLOAT4X4 highp mat4

#define HALF mediump float
#define HALF2 mediump vec2
#define HALF3 mediump vec3
#define HALF4 mediump vec4
#define HALF3X3 mediump mat3
#define HALF4X4 mediump mat4

#define FIXED lowp float
#define FIXED2 lowp vec2
#define FIXED3 lowp vec3
#define FIXED4 lowp vec4
#define FIXED3X3 lowp mat3
#define FIXED4X4 lowp mat4
