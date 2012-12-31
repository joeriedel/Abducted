/*! \file HLSL.c
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup renderer
*/

#define _HLSL
#define HLSL(_x) _x
#define GLSL(_x)

#if defined(VERTEX)
#define VOID_GLOBALS uniform Uniforms U, Attributes I
#define GLOBALS , VOID_GLOBALS
#else
#define VOID_GLOBALS uniform Uniforms U, Varying I
#define GLOBALS , VOID_GLOBALS
#endif

#define P_GLOBALS U, I
#define UNIFORM(_x) U.##_x
#define UDECL(_x) _x
#define BEGIN_UNIFORMS struct Uniforms {
#define END_UNIFORMS };

struct FReturn
{
	float4 color : COLOR;
#if defined(DEPTH)
	float depth : DEPTH;
#endif
};

#define gl_FrontColor U.color
#define gl_FragColor R.color
#define gl_FragDepth R.depth
#define gl_Position R.position
#define gl_ModelViewProjectionMatrix state.matrix.mvp

#if defined(VERTEX)
#define MAIN Varying main(uniform Uniforms U, Attributes I)
#define DECL_R Varying R;
#else
#define FReturn main(uniform Uniforms U, Varying I)
#define DECL_R FReturn R;
#endif

#define BEGIN_ATTRIBUTES struct Attributes {
#define END_ATTRIBUTES };

#define BEGIN_VARYING struct Varying {
#define END_VARYING };

#define IN(_v) I._v
#define OUT(_v) R._v
#define SEL(_hlsl, _glsl) _hlsl

#define RETURN return R;

#define FLOAT float
#define FLOAT2 float2
#define FLOAT3 float3
#define FLOAT4 float4
#define FLOAT3X3 float3x3
#define FLOAT4X4 float4x4

#define HALF half
#define HALF2 half2
#define HALF3 half3
#define HALF4 half4
#define HALF3X3 half3x3
#define HALF4X4 half4x4

#define FIXED fixed
#define FIXED2 fixed2
#define FIXED3 fixed3
#define FIXED4 fixed4
#define FIXED3X3 fixed3x3
#define FIXED4X4 fixed4x4
