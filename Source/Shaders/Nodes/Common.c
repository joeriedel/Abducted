/*! \file Common.c
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup renderer
*/

///////////////////////////////////////////////////////////////////////////////

#if defined(TEXTURES)
#if TEXTURES==6
	#define UTXREGS \
		GLSL(uniform lowp) T0TYPE UDECL(t0) HLSL(:TEXUNIT0); \
		GLSL(uniform lowp) T1TYPE UDECL(t1) HLSL(:TEXUNIT1); \
		GLSL(uniform lowp) T2TYPE UDECL(t2) HLSL(:TEXUNIT2); \
		GLSL(uniform lowp) T3TYPE UDECL(t3) HLSL(:TEXUNIT3); \
		GLSL(uniform lowp) T4TYPE UDECL(t4) HLSL(:TEXUNIT4); \
		GLSL(uniform lowp) T5TYPE UDECL(t5) HLSL(:TEXUNIT5);
#elif TEXTURES==5
	#define UTXREGS \
		GLSL(uniform lowp) T0TYPE UDECL(t0) HLSL(:TEXUNIT0); \
		GLSL(uniform lowp) T1TYPE UDECL(t1) HLSL(:TEXUNIT1); \
		GLSL(uniform lowp) T2TYPE UDECL(t2) HLSL(:TEXUNIT2); \
		GLSL(uniform lowp) T3TYPE UDECL(t3) HLSL(:TEXUNIT3); \
		GLSL(uniform lowp) T4TYPE UDECL(t4) HLSL(:TEXUNIT4);
#elif TEXTURES==4
	#define UTXREGS \
		GLSL(uniform lowp) T0TYPE UDECL(t0) HLSL(:TEXUNIT0); \
		GLSL(uniform lowp) T1TYPE UDECL(t1) HLSL(:TEXUNIT1); \
		GLSL(uniform lowp) T2TYPE UDECL(t2) HLSL(:TEXUNIT2); \
		GLSL(uniform lowp) T3TYPE UDECL(t3) HLSL(:TEXUNIT3);
#elif TEXTURES==3
	#define UTXREGS \
		GLSL(uniform lowp) T0TYPE UDECL(t0) HLSL(:TEXUNIT0); \
		GLSL(uniform lowp) T1TYPE UDECL(t1) HLSL(:TEXUNIT1); \
		GLSL(uniform lowp) T2TYPE UDECL(t2) HLSL(:TEXUNIT2);
#elif TEXTURES==2
	#define UTXREGS \
		GLSL(uniform lowp) T0TYPE UDECL(t0) HLSL(:TEXUNIT0); \
		GLSL(uniform lowp) T1TYPE UDECL(t1) HLSL(:TEXUNIT1);
#elif TEXTURES==1
	#define UTXREGS \
		GLSL(uniform lowp) T0TYPE UDECL(t0) HLSL(:TEXUNIT0);
#endif
#else
#define UTXREGS
#endif // TEXTURES

///////////////////////////////////////////////////////////////////////////////

FLOAT4 TCScale(HALF4 wave, FLOAT4 tc) {
	return ((tc.xyxy-HALF4(0.5, 0.5, 0.0, 0.0))*wave.xyxy) + HALF4(0.5, 0.5, 0, 0);
}

FLOAT4 TCRotate(HALF4 sincos, FLOAT4 tc) {
	HALF4 t = (tc.xyxy-HALF4(0.5, 0.5, 0.5, 0.5))*sincos;
	return HALF4(t.z-t.y, t.w+t.x, tc.z, tc.w) + HALF4(0.5, 0.5, 0.0, 0.0);
}

FLOAT4 TCShift(FLOAT4 wave, FLOAT4 tc) {
	return tc+wave.xyxy;
}

FLOAT4 TCScroll(FLOAT4 wave, FLOAT4 tc) {
	return tc+wave.xyxy+wave.zwzw;
}

FLOAT4 TCTurb(FLOAT4 turb, FLOAT4 turb2, FLOAT4 tc, FLOAT PI) {
	return tc + sin(tc.yxwz*PI*turb2.xyxy+turb.xyxy) * turb.zwzw;
}

#if defined(TEXCOORDS)

// shader animation support

#if defined(TCINPUTS)
	#if TCINPUTS==6
		#define IN_TCREGS \
			GLSL(attribute) FLOAT4 SEL(tc0, IN(tc0)) HLSL(:TEXCOORD0); \
			GLSL(attribute) FLOAT4 SEL(tc1, IN(tc1)) HLSL(:TEXCOORD1); \
			GLSL(attribute) FLOAT4 SEL(tc2, IN(tc2)) HLSL(:TEXCOORD2); \
			GLSL(attribute) FLOAT4 SEL(tc3, IN(tc3)) HLSL(:TEXCOORD3); \
			GLSL(attribute) FLOAT4 SEL(tc4, IN(tc4)) HLSL(:TEXCOORD4); \
			GLSL(attribute) FLOAT4 SEL(tc5, IN(tc5)) HLSL(:TEXCOORD5);
	#elif TCINPUTS==5
		#define IN_TCREGS \
			GLSL(attribute) FLOAT4 SEL(tc0, IN(tc0)) HLSL(:TEXCOORD0); \
			GLSL(attribute) FLOAT4 SEL(tc1, IN(tc1)) HLSL(:TEXCOORD1); \
			GLSL(attribute) FLOAT4 SEL(tc2, IN(tc2)) HLSL(:TEXCOORD2); \
			GLSL(attribute) FLOAT4 SEL(tc3, IN(tc3)) HLSL(:TEXCOORD3); \
			GLSL(attribute) FLOAT4 SEL(tc4, IN(tc4)) HLSL(:TEXCOORD4);
	#elif TCINPUTS==4
		#define IN_TCREGS \
			GLSL(attribute) FLOAT4 SEL(tc0, IN(tc0)) HLSL(:TEXCOORD0); \
			GLSL(attribute) FLOAT4 SEL(tc1, IN(tc1)) HLSL(:TEXCOORD1); \
			GLSL(attribute) FLOAT4 SEL(tc2, IN(tc2)) HLSL(:TEXCOORD2); \
			GLSL(attribute) FLOAT4 SEL(tc3, IN(tc3)) HLSL(:TEXCOORD3);
	#elif TCINPUTS==3
		#define IN_TCREGS \
			GLSL(attribute) FLOAT4 SEL(tc0, IN(tc0)) HLSL(:TEXCOORD0); \
			GLSL(attribute) FLOAT4 SEL(tc1, IN(tc1)) HLSL(:TEXCOORD1); \
			GLSL(attribute) FLOAT4 SEL(tc2, IN(tc2)) HLSL(:TEXCOORD2);
	#elif TCINPUTS==2
		#define IN_TCREGS \
			GLSL(attribute) FLOAT4 SEL(tc0, IN(tc0)) HLSL(:TEXCOORD0); \
			GLSL(attribute) FLOAT4 SEL(tc1, IN(tc1)) HLSL(:TEXCOORD1);
	#else
		#define IN_TCREGS \
			GLSL(attribute) FLOAT4 SEL(tc0, IN(tc0)) HLSL(:TEXCOORD0);
	#endif // TCINPUTS
#else
	#define IN_TCREGS
#endif

#if TEXCOORDS==6
	#define OUT_TCREGS \
		GLSL(varying) FLOAT4 SEL(tc0, OUT(tc0)) HLSL(:TEXCOORD0); \
		GLSL(varying) FLOAT4 SEL(tc1, OUT(tc1)) HLSL(:TEXCOORD1); \
		GLSL(varying) FLOAT4 SEL(tc2, OUT(tc2)) HLSL(:TEXCOORD2); \
		GLSL(varying) FLOAT4 SEL(tc3, OUT(tc3)) HLSL(:TEXCOORD3); \
		GLSL(varying) FLOAT4 SEL(tc4, OUT(tc4)) HLSL(:TEXCOORD4); \
		GLSL(varying) FLOAT4 SEL(tc5, OUT(tc5)) HLSL(:TEXCOORD5);
	#define UTCMODREGS \
		GLSL(uniform) FLOAT4 UDECL(tcmod0)[6] HLSL(:C8); \
		GLSL(uniform) FLOAT4 UDECL(tcmod1)[6] HLSL(:C14); \
		GLSL(uniform) FLOAT4 UDECL(tcmod2)[6] HLSL(:C20); \
		GLSL(uniform) FLOAT4 UDECL(tcmod3)[6] HLSL(:C26); \
		GLSL(uniform) FLOAT4 UDECL(tcmod4)[6] HLSL(:C32); \
		GLSL(uniform) FLOAT4 UDECL(tcmod5)[6] HLSL(:C38);
#elif TEXCOORDS==5
	#define OUT_TCREGS \
		GLSL(varying) FLOAT4 SEL(tc0, OUT(tc0)) HLSL(:TEXCOORD0); \
		GLSL(varying) FLOAT4 SEL(tc1, OUT(tc1)) HLSL(:TEXCOORD1); \
		GLSL(varying) FLOAT4 SEL(tc2, OUT(tc2)) HLSL(:TEXCOORD2); \
		GLSL(varying) FLOAT4 SEL(tc3, OUT(tc3)) HLSL(:TEXCOORD3); \
		GLSL(varying) FLOAT4 SEL(tc4, OUT(tc4)) HLSL(:TEXCOORD4);
	#define UTCMODREGS \
		GLSL(uniform) FLOAT4 UDECL(tcmod0)[6] HLSL(:C8); \
		GLSL(uniform) FLOAT4 UDECL(tcmod1)[6] HLSL(:C14); \
		GLSL(uniform) FLOAT4 UDECL(tcmod2)[6] HLSL(:C20); \
		GLSL(uniform) FLOAT4 UDECL(tcmod3)[6] HLSL(:C26); \
		GLSL(uniform) FLOAT4 UDECL(tcmod4)[6] HLSL(:C32);
#elif TEXCOORDS==4
	#define OUT_TCREGS \
		GLSL(varying) FLOAT4 SEL(tc0, OUT(tc0)) HLSL(:TEXCOORD0); \
		GLSL(varying) FLOAT4 SEL(tc1, OUT(tc1)) HLSL(:TEXCOORD1); \
		GLSL(varying) FLOAT4 SEL(tc2, OUT(tc2)) HLSL(:TEXCOORD2); \
		GLSL(varying) FLOAT4 SEL(tc3, OUT(tc3)) HLSL(:TEXCOORD3);
	#define UTCMODREGS \
		GLSL(uniform) FLOAT4 UDECL(tcmod0)[6] HLSL(:C8); \
		GLSL(uniform) FLOAT4 UDECL(tcmod1)[6] HLSL(:C14); \
		GLSL(uniform) FLOAT4 UDECL(tcmod2)[6] HLSL(:C20); \
		GLSL(uniform) FLOAT4 UDECL(tcmod3)[6] HLSL(:C26);
#elif TEXCOORDS==3
	#define OUT_TCREGS \
		GLSL(varying) FLOAT4 SEL(tc0, OUT(tc0)) HLSL(:TEXCOORD0); \
		GLSL(varying) FLOAT4 SEL(tc1, OUT(tc1)) HLSL(:TEXCOORD1); \
		GLSL(varying) FLOAT4 SEL(tc2, OUT(tc2)) HLSL(:TEXCOORD2);
	#define UTCMODREGS \
		GLSL(uniform) FLOAT4 UDECL(tcmod0)[6] HLSL(:C8); \
		GLSL(uniform) FLOAT4 UDECL(tcmod1)[6] HLSL(:C14); \
		GLSL(uniform) FLOAT4 UDECL(tcmod2)[6] HLSL(:C20);
#elif TEXCOORDS==2
	#define OUT_TCREGS \
		GLSL(varying) FLOAT4 SEL(tc0, OUT(tc0)) HLSL(:TEXCOORD0); \
		GLSL(varying) FLOAT4 SEL(tc1, OUT(tc1)) HLSL(:TEXCOORD1);
	#define UTCMODREGS \
		GLSL(uniform) FLOAT4 UDECL(tcmod0)[6] HLSL(:C8); \
		GLSL(uniform) FLOAT4 UDECL(tcmod1)[6] HLSL(:C14);
#else
	#define OUT_TCREGS \
		GLSL(varying) FLOAT4 SEL(tc0, OUT(tc0)) HLSL(:TEXCOORD0);
	#define UTCMODREGS \
		GLSL(uniform) FLOAT4 UDECL(tcmod0)[6] HLSL(:C8);
#endif // TEXCOORDS

///////////////////////////////////////////////////////////////////////////////
// texture coordinate waveform animation

#if defined(TEXCOORD0_GENREFLECT)
	#define TEXCOORD0_GEN \
		OUT(tc0) = genReflectTC;
#else
	#define TEXCOORD0_GEN \
		OUT(tc0) = IN(TEXCOORD0);
#endif

#if defined(TEXCOORD0_ROTATE)
	#define TEXCOORD0_MODROTATE \
		OUT(tc0) = TCRotate(UNIFORM(tcmod0)[0], OUT(tc0));
#else
	#define TEXCOORD0_MODROTATE
#endif

#if defined(TEXCOORD0_TURB)
	#define TEXCOORD0_MODTURB \
		OUT(tc0) = TCTurb(UNIFORM(tcmod0)[1], UNIFORM(tcmod0)[5], OUT(tc0), PI);
#else
	#define TEXCOORD0_MODTURB
#endif

#if defined(TEXCOORD0_SCALE)
	#define TEXCOORD0_MODSCALE \
		OUT(tc0) = TCScale(UNIFORM(tcmod0)[2], OUT(tc0));
#else
	#define TEXCOORD0_MODSCALE
#endif

#if defined(TEXCOORD0_SHIFT)
	#define TEXCOORD0_MODSHIFT \
		OUT(tc0) = TCShift(UNIFORM(tcmod0)[3], OUT(tc0));
#else
	#define TEXCOORD0_MODSHIFT
#endif

#if defined(TEXCOORD0_SCROLL)
	#define TEXCOORD0_MODSCROLL \
		OUT(tc0) = TCScroll(UNIFORM(tcmod0)[4], OUT(tc0));
#else
	#define TEXCOORD0_MODSCROLL
#endif

#define TEXCOORD0_MOD \
	TEXCOORD0_MODROTATE \
	TEXCOORD0_MODSCALE \
	TEXCOORD0_MODTURB \
	TEXCOORD0_MODSHIFT \
	TEXCOORD0_MODSCROLL

#if TEXCOORDS>1

	#if defined(TEXCOORD1_GENREFLECT)
		#define TEXCOORD1_GEN \
			OUT(tc1) = genReflectTC;
	#else
		#define TEXCOORD1_GEN \
			OUT(tc1) = IN(TEXCOORD1);
	#endif

	#if defined(TEXCOORD1_ROTATE)
		#define TEXCOORD1_MODROTATE \
			OUT(tc1) = TCRotate(UNIFORM(tcmod1)[0], OUT(tc1));
	#else
		#define TEXCOORD1_MODROTATE
	#endif

	#if defined(TEXCOORD1_TURB)
		#define TEXCOORD1_MODTURB \
			OUT(tc1) = TCTurb(UNIFORM(tcmod1)[1], UNIFORM(tcmod1)[5], OUT(tc1), PI);
	#else
		#define TEXCOORD1_MODTURB
	#endif

	#if defined(TEXCOORD1_SCALE)
		#define TEXCOORD1_MODSCALE \
			OUT(tc1) = TCScale(UNIFORM(tcmod1)[2], OUT(tc1));
	#else
		#define TEXCOORD1_MODSCALE
	#endif

	#if defined(TEXCOORD1_SHIFT)
		#define TEXCOORD1_MODSHIFT \
			OUT(tc1) = TCShift(UNIFORM(tcmod1)[3], OUT(tc1));
	#else
		#define TEXCOORD1_MODSHIFT
	#endif

	#if defined(TEXCOORD1_SCROLL)
		#define TEXCOORD1_MODSCROLL \
			OUT(tc1) = TCScroll(UNIFORM(tcmod1)[4], OUT(tc1));
	#else
		#define TEXCOORD1_MODSCROLL
	#endif

	#define TEXCOORD1_MOD \
		TEXCOORD1_MODROTATE \
		TEXCOORD1_MODSCALE \
		TEXCOORD1_MODTURB \
		TEXCOORD1_MODSHIFT \
		TEXCOORD1_MODSCROLL

#if TEXCOORDS>2

	#if defined(TEXCOORD2_GENREFLECT)
		#define TEXCOORD2_GEN \
			OUT(tc2) = genReflectTC;
	#else
		#define TEXCOORD2_GEN \
			OUT(tc2) = IN(TEXCOORD2);
	#endif

	#if defined(TEXCOORD2_ROTATE)
		#define TEXCOORD2_MODROTATE \
			OUT(tc2) = TCRotate(UNIFORM(tcmod2)[0], OUT(tc2));
	#else
		#define TEXCOORD2_MODROTATE
	#endif

	#if defined(TEXCOORD2_TURB)
		#define TEXCOORD2_MODTURB \
			OUT(tc2) = TCTurb(UNIFORM(tcmod2)[1], UNIFORM(tcmod2)[5], OUT(tc2), PI);
	#else
		#define TEXCOORD2_MODTURB
	#endif

	#if defined(TEXCOORD2_SCALE)
		#define TEXCOORD2_MODSCALE \
			OUT(tc2) = TCScale(UNIFORM(tcmod2)[2], OUT(tc2));
	#else
		#define TEXCOORD2_MODSCALE
	#endif

	#if defined(TEXCOORD2_SHIFT)
		#define TEXCOORD2_MODSHIFT \
			OUT(tc2) = TCShift(UNIFORM(tcmod2)[3], OUT(tc2));
	#else
		#define TEXCOORD2_MODSHIFT
	#endif

	#if defined(TEXCOORD2_SCROLL)
		#define TEXCOORD2_MODSCROLL \
			OUT(tc2) = TCScroll(UNIFORM(tcmod2)[4], OUT(tc2));
	#else
		#define TEXCOORD2_MODSCROLL
	#endif

	#define TEXCOORD2_MOD \
		TEXCOORD2_MODROTATE \
		TEXCOORD2_MODSCALE \
		TEXCOORD2_MODTURB \
		TEXCOORD2_MODSHIFT \
		TEXCOORD2_MODSCROLL

#if TEXCOORDS>3

	#if defined(TEXCOORD3_GENREFLECT)
		#define TEXCOORD3_GEN \
			OUT(tc3) = genReflectTC;
	#else
		#define TEXCOORD3_GEN \
			OUT(tc3) = IN(TEXCOORD3);
	#endif

	#if defined(TEXCOORD3_ROTATE)
		#define TEXCOORD3_MODROTATE \
			OUT(tc3) = TCRotate(UNIFORM(tcmod3)[0], OUT(tc3));
	#else
		#define TEXCOORD3_MODROTATE
	#endif

	#if defined(TEXCOORD3_TURB)
		#define TEXCOORD3_MODTURB \
			OUT(tc3) = TCTurb(UNIFORM(tcmod3)[1], UNIFORM(tcmod3)[5], OUT(tc3), PI);
	#else
		#define TEXCOORD3_MODTURB
	#endif

	#if defined(TEXCOORD3_SCALE)
		#define TEXCOORD3_MODSCALE \
			OUT(tc3) = TCScale(UNIFORM(tcmod3)[2], OUT(tc3));
	#else
		#define TEXCOORD3_MODSCALE
	#endif

	#if defined(TEXCOORD3_SHIFT)
		#define TEXCOORD3_MODSHIFT \
			OUT(tc3) = TCShift(UNIFORM(tcmod3)[3], OUT(tc3));
	#else
		#define TEXCOORD3_MODSHIFT
	#endif

	#if defined(TEXCOORD3_SCROLL)
		#define TEXCOORD3_MODSCROLL \
			OUT(tc3) = TCScroll(UNIFORM(tcmod3)[4], OUT(tc3));
	#else
		#define TEXCOORD3_MODSCROLL
	#endif

	#define TEXCOORD3_MOD \
		TEXCOORD3_MODROTATE \
		TEXCOORD3_MODSCALE \
		TEXCOORD3_MODTURB \
		TEXCOORD3_MODSHIFT \
		TEXCOORD3_MODSCROLL

#if TEXCOORDS>4

	#if defined(TEXCOORD4_GENREFLECT)
		#define TEXCOORD4_GEN \
			OUT(tc4) = genReflectTC;
	#else
		#define TEXCOORD4_GEN \
			OUT(tc4) = IN(TEXCOORD4);
	#endif

	#if defined(TEXCOORD4_ROTATE)
		#define TEXCOORD4_MODROTATE \
			OUT(tc4) = TCRotate(UNIFORM(tcmod4[0]), OUT(tc4));
	#else
		#define TEXCOORD4_MODROTATE
	#endif

	#if defined(TEXCOORD4_TURB)
		#define TEXCOORD4_MODTURB \
			OUT(tc4) = TCTurb(UNIFORM(tcmod4)[1], UNIFORM(tcmod4)[5], OUT(tc4), PI);
	#else
		#define TEXCOORD4_MODTURB
	#endif

	#if defined(TEXCOORD4_SCALE)
		#define TEXCOORD4_MODSCALE \
			OUT(tc4) = TCScale(UNIFORM(tcmod4)[2], OUT(tc4));
	#else
		#define TEXCOORD4_MODSCALE
	#endif

	#if defined(TEXCOORD4_SHIFT)
		#define TEXCOORD4_MODSHIFT \
			OUT(tc4) = TCShift(UNIFORM(tcmod4)[3], OUT(tc4));
	#else
		#define TEXCOORD4_MODSHIFT
	#endif

	#if defined(TEXCOORD4_SCROLL)
		#define TEXCOORD4_MODSCROLL \
			OUT(tc4) = TCScroll(UNIFORM(tcmod4)[4], OUT(tc4));
	#else
		#define TEXCOORD4_MODSCROLL
	#endif

	#define TEXCOORD4_MOD \
		TEXCOORD4_MODROTATE \
		TEXCOORD4_MODSCALE \
		TEXCOORD4_MODTURB \
		TEXCOORD4_MODSHIFT \
		TEXCOORD4_MODSCROLL

#if TEXCOORDS>5

	#if defined(TEXCOORD5_GENREFLECT)
		#define TEXCOORD5_GEN \
			OUT(tc5) = genReflectTC;
	#else
		#define TEXCOORD5_GEN \
			OUT(tc5) = IN(TEXCOORD5);
	#endif

	#if defined(TEXCOORD5_ROTATE)
		#define TEXCOORD5_MODROTATE \
			OUT(tc5) = TCRotate(UNIFORM(tcmod5)[0], OUT(tc5));
	#else
		#define TEXCOORD5_MODROTATE
	#endif

	#if defined(TEXCOORD5_TURB)
		#define TEXCOORD5_MODTURB \
			OUT(tc5) = TCTurb(UNIFORM(tcmod5)[1], UNIFORM(tcmod5)[5], OUT(tc5), PI);
	#else
		#define TEXCOORD5_MODTURB
	#endif

	#if defined(TEXCOORD5_SCALE)
		#define TEXCOORD5_MODSCALE \
			OUT(tc5) = TCScale(UNIFORM(tcmod5)[2], OUT(tc5));
	#else
		#define TEXCOORD5_MODSCALE
	#endif

	#if defined(TEXCOORD5_SHIFT)
		#define TEXCOORD5_MODSHIFT \
			OUT(tc5) = TCShift(UNIFORM(tcmod5)[3], OUT(tc5));
	#else
		#define TEXCOORD5_MODSHIFT
	#endif

	#if defined(TEXCOORD5_SCROLL)
		#define TEXCOORD5_MODSCROLL \
			OUT(tc5) = TCScroll(UNIFORM(tcmod5)[4], OUT(tc5));
	#else
		#define TEXCOORD5_MODSCROLL
	#endif

	#define TEXCOORD5_MOD \
		TEXCOORD5_MODROTATE \
		TEXCOORD5_MODSCALE \
		TEXCOORD5_MODTURB \
		TEXCOORD5_MODSHIFT \
		TEXCOORD5_MODSCROLL

#else // TEXCOORDS<=5

	#define TCMOD \
		TEXCOORD0_MOD \
		TEXCOORD1_MOD \
		TEXCOORD2_MOD \
		TEXCOORD3_MOD \
		TEXCOORD4_MOD

	#define TCGEN \
		TEXCOORD0_GEN \
		TEXCOORD1_GEN \
		TEXCOORD2_GEN \
		TEXCOORD3_GEN \
		TEXCOORD4_GEN

#endif // TEXCOORDS>5

#else // TEXCOORD<=4

	#define TCMOD \
		TEXCOORD0_MOD \
		TEXCOORD1_MOD \
		TEXCOORD2_MOD \
		TEXCOORD3_MOD

	#define TCGEN \
		TEXCOORD0_GEN \
		TEXCOORD1_GEN \
		TEXCOORD2_GEN \
		TEXCOORD3_GEN

#endif // TEXCOORDS>4
#else // TEXCOORD<=3

#define TCMOD \
	TEXCOORD0_MOD \
	TEXCOORD1_MOD \
	TEXCOORD2_MOD
#define TCGEN \
	TEXCOORD0_GEN \
	TEXCOORD1_GEN \
	TEXCOORD2_GEN

#endif // TEXCOORDS>3
#else // TEXCOORDS<=2

#define TCMOD \
	TEXCOORD0_MOD \
	TEXCOORD1_MOD
#define TCGEN \
	TEXCOORD0_GEN \
	TEXCOORD1_GEN

#endif // TEXCOORDS>2
#else // TEXCOOORDS==1

#define TCMOD TEXCOORD0_MOD
#define TCGEN TEXCOORD0_GEN

#endif // TEXCOORDS>1
#else // !defined(TEXCOORDS)

#define UTCMODREGS
#define TCMOD
#define TCGEN
#define IN_TCREGS
#define OUT_TCREGS

#endif // TEXCOORDS

///////////////////////////////////////////////////////////////////////////////
 // 1 Normal channel supported

#if defined(NORMALS)
	#define IN_NMREGS \
		GLSL(attribute) FIXED3 SEL(nm0, IN(nm0)) HLSL(:NORMAL);

	#if defined(NUM_SHADER_NORMALS)
		#define OUT_NMREGS \
			GLSL(varying) FIXED3 SEL(nm0, OUT(nm0)) HLSL(:NORMAL);
	#else // !defined(NUM_SHADER_NORMALS)
		#define OUT_NMREGS
	#endif
#else // !defined(NORMALS)
#define IN_NMREGS
#define OUT_NMREGS
#endif // defined(NORMALS)

///////////////////////////////////////////////////////////////////////////////
// 1 Tangent channel

#if defined(TANGENTS)
	#define IN_TANREGS \
		GLSL(attribute) FIXED4 SEL(tan0, IN(tan0)) HLSL(:TANGENT);
	#if defined(NUM_SHADER_TANGENTS)
		#define OUT_TANREGS \
			GLSL(varying) FIXED4 SEL(tan0, OUT(tan0)) HLSL(:TANGENT);
	#else // !defined(NUM_SHADER_TANGENTS)
		#define OUT_TANREGS
	#endif
#else // !defined(TANGENTS)
#define IN_TANREGS
#define OUT_TANREGS
#endif // defined(TANGENTS)

///////////////////////////////////////////////////////////////////////////////

#if defined(BITANGENTS)
	#if defined(NUM_SHADER_BITANGENTS)
		#define OUT_BITANREGS \
			GLSL(varying) FIXED3 SEL(bitan0, OUT(bitan0)) HLSL(:BITANGENT);
	#else // !defined(NUM_SHADER_BITANGENTS)
		#define OUT_BITANREGS
	#endif
#else // !defined(BITANGENTS)
#define OUT_BITANREGS
#endif // defined(BITANGENTS)

///////////////////////////////////////////////////////////////////////////////

#if defined(LIGHTS)

#if defined(SHADER_LIGHT_POS) || defined(SHADER_LIGHT_VEC) || defined(SHADER_LIGHT_HALFVEC) || defined(SHADER_LIGHT_TANVEC) || defined(SHADER_LIGHT_TANHALFVEC)
#if (SHADER_LIGHT_POS == 4) || (SHADER_LIGHT_VEC == 4) || (SHADER_LIGHT_HALFVEC == 4) || (SHADER_LIGHT_TANVEC == 4) || (SHADER_LIGHT_TANHALFVEC == 4)
	#define ULIGHTPOSREGS \
		GLSL(uniform) FLOAT4 UDECL(light0_pos);
		GLSL(uniform) FLOAT4 UDECL(light1_pos);
		GLSL(uniform) FLOAT4 UDECL(light2_pos);
		GLSL(uniform) FLOAT4 UDECL(light3_pos);
#elif (SHADER_LIGHT_POS == 3) || (SHADER_LIGHT_VEC == 3) || (SHADER_LIGHT_HALFVEC == 3) || (SHADER_LIGHT_TANVEC == 3) || (SHADER_LIGHT_TANHALFVEC == 3)
	#define ULIGHTPOSREGS \
		GLSL(uniform) FLOAT4 UDECL(light0_pos);
		GLSL(uniform) FLOAT4 UDECL(light1_pos);
		GLSL(uniform) FLOAT4 UDECL(light2_pos);
#elif (SHADER_LIGHT_POS == 2) || (SHADER_LIGHT_VEC == 2) || (SHADER_LIGHT_HALFVEC == 2) || (SHADER_LIGHT_TANVEC == 2) || (SHADER_LIGHT_TANHALFVEC == 2)
	#define ULIGHTPOSREGS \
		GLSL(uniform) FLOAT4 UDECL(light0_pos);
		GLSL(uniform) FLOAT4 UDECL(light1_pos);
#elif (SHADER_LIGHT_POS == 1) || (SHADER_LIGHT_VEC == 1) || (SHADER_LIGHT_HALFVEC == 1) || (SHADER_LIGHT_TANVEC == 1) || (SHADER_LIGHT_TANHALFVEC == 1)
	#define ULIGHTPOSREGS \
		GLSL(uniform) FLOAT4 UDECL(light0_pos);
#endif
#else
#define ULIGHTPOSREGS
#endif

#if defined(SHADER_LIGHT_DIFFUSE_COLOR)
#if SHADER_LIGHT_DIFFUSE_COLOR == 4
	#define ULIGHTDFCOLORREGS \
		GLSL(uniform) HALF4 UDECL(light0_diffuseColor);
		GLSL(uniform) HALF4 UDECL(light1_diffuseColor);
		GLSL(uniform) HALF4 UDECL(light2_diffuseColor);
		GLSL(uniform) HALF4 UDECL(light3_diffuseColor);
#elif SHADER_LIGHT_DIFFUSE_COLOR == 3
	#define ULIGHTDFCOLORREGS \
		GLSL(uniform) HALF4 UDECL(light0_diffuseColor);
		GLSL(uniform) HALF4 UDECL(light1_diffuseColor);
		GLSL(uniform) HALF4 UDECL(light2_diffuseColor);
#elif SHADER_LIGHT_DIFFUSE_COLOR == 2
	#define ULIGHTDFCOLORREGS \
		GLSL(uniform) HALF4 UDECL(light0_diffuseColor);
		GLSL(uniform) HALF4 UDECL(light1_diffuseColor);
#elif SHADER_LIGHT_DIFFUSE_COLOR == 1
	#define ULIGHTDFCOLORREGS \
		GLSL(uniform) HALF4 UDECL(light0_diffuseColor);
#endif
#else
#define ULIGHTDFCOLORREGS
#endif

#if defined(SHADER_LIGHT_SPECULAR_COLOR)
#if SHADER_LIGHT_SPECULAR_COLOR == 4
	#define ULIGHTSPCOLORREGS \
		GLSL(uniform) HALF3 UDECL(light0_specularColor);
		GLSL(uniform) HALF3 UDECL(light1_specularColor);
		GLSL(uniform) HALF3 UDECL(light2_specularColor);
		GLSL(uniform) HALF3 UDECL(light3_specularColor);
#elif SHADER_LIGHT_SPECULAR_COLOR == 3
	#define ULIGHTSPCOLORREGS \
		GLSL(uniform) HALF3 UDECL(light0_specularColor);
		GLSL(uniform) HALF3 UDECL(light1_specularColor);
		GLSL(uniform) HALF3 UDECL(light2_specularColor);
#elif SHADER_LIGHT_SPECULAR_COLOR == 2
	#define ULIGHTSPCOLORREGS \
		GLSL(uniform) HALF3 UDECL(light0_specularColor);
		GLSL(uniform) HALF3 UDECL(light1_specularColor);
#elif SHADER_LIGHT_SPECULAR_COLOR == 1
	#define ULIGHTSPCOLORREGS \
		GLSL(uniform) HALF3 UDECL(light0_specularColor);
#endif
#else
#define ULIGHTSPCOLORREGS
#endif
	
#if defined(SHADER_LIGHT_VEC)
#if SHADER_LIGHT_VEC == 4
	#define OUT_LIGHTVEC \
		GLSL(varying) FIXED3 SEL(light0_vec, OUT(light0_vec));
		GLSL(varying) FIXED3 SEL(light1_vec, OUT(light1_vec));
		GLSL(varying) FIXED3 SEL(light2_vec, OUT(light2_vec));
		GLSL(varying) FIXED3 SEL(light3_vec, OUT(light3_vec));
#elif SHADER_LIGHT_VEC == 3
	#define OUT_LIGHTVEC \
		GLSL(varying) FIXED3 SEL(light0_vec, OUT(light0_vec));
		GLSL(varying) FIXED3 SEL(light1_vec, OUT(light1_vec));
		GLSL(varying) FIXED3 SEL(light2_vec, OUT(light2_vec));
#elif SHADER_LIGHT_VEC == 2
	#define OUT_LIGHTVEC \
		GLSL(varying) FIXED3 SEL(light0_vec, OUT(light0_vec));
		GLSL(varying) FIXED3 SEL(light1_vec, OUT(light1_vec));
#elif SHADER_LIGHT_VEC == 1
	#define OUT_LIGHTVEC \
		GLSL(varying) FIXED3 SEL(light0_vec, OUT(light0_vec));
#endif
#else
#define OUT_LIGHTVEC
#endif

#if defined(SHADER_LIGHT_HALFVEC)
#if SHADER_LIGHT_HALFVEC == 4
	#define OUT_LIGHTHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_halfvec, OUT(light0_halfvec));
		GLSL(varying) FIXED3 SEL(light1_halfvec, OUT(light1_halfvec));
		GLSL(varying) FIXED3 SEL(light2_halfvec, OUT(light2_halfvec));
		GLSL(varying) FIXED3 SEL(light3_halfvec, OUT(light3_halfvec));
#elif SHADER_LIGHT_HALFVEC == 3
	#define OUT_LIGHTHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_halfvec, OUT(light0_halfvec));
		GLSL(varying) FIXED3 SEL(light1_halfvec, OUT(light1_halfvec));
		GLSL(varying) FIXED3 SEL(light2_halfvec, OUT(light2_halfvec));
#elif SHADER_LIGHT_HALFVEC == 2
	#define OUT_LIGHTHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_halfvec, OUT(light0_halfvec));
		GLSL(varying) FIXED3 SEL(light1_halfvec, OUT(light1_halfvec));
#elif SHADER_LIGHT_HALFVEC == 1
	#define OUT_LIGHTHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_halfvec, OUT(light0_halfvec));
#endif
#else
#define OUT_LIGHTHALFVEC
#endif

#if defined(SHADER_LIGHT_TANVEC)
#if SHADER_LIGHT_TANVEC == 4
	#define OUT_LIGHTTANVEC \
		GLSL(varying) FIXED3 SEL(light0_tanvec, OUT(light0_tanvec));
		GLSL(varying) FIXED3 SEL(light1_tanvec, OUT(light1_tanvec));
		GLSL(varying) FIXED3 SEL(light2_tanvec, OUT(light2_tanvec));
		GLSL(varying) FIXED3 SEL(light3_tanvec, OUT(light3_tanvec));
#elif SHADER_LIGHT_TANVEC == 3
	#define OUT_LIGHTTANVEC \
		GLSL(varying) FIXED3 SEL(light0_tanvec, OUT(light0_tanvec));
		GLSL(varying) FIXED3 SEL(light1_tanvec, OUT(light1_tanvec));
		GLSL(varying) FIXED3 SEL(light2_tanvec, OUT(light2_tanvec));
#elif SHADER_LIGHT_TANVEC == 2
	#define OUT_LIGHTTANVEC \
		GLSL(varying) FIXED3 SEL(light0_tanvec, OUT(light0_tanvec));
		GLSL(varying) FIXED3 SEL(light1_tanvec, OUT(light1_tanvec));
#elif SHADER_LIGHT_TANVEC == 1
	#define OUT_LIGHTTANVEC \
		GLSL(varying) FIXED3 SEL(light0_tanvec, OUT(light0_tanvec));
#endif
#else
#define OUT_LIGHTTANVEC
#endif

#if defined(SHADER_LIGHT_TANHALFVEC)
#if SHADER_LIGHT_TANHALFVEC == 4
	#define OUT_LIGHTTANHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_tanhalfvec, OUT(light0_tanhalfvec));
		GLSL(varying) FIXED3 SEL(light1_tanhalfvec, OUT(light1_tanhalfvec));
		GLSL(varying) FIXED3 SEL(light2_tanhalfvec, OUT(light2_tanhalfvec));
		GLSL(varying) FIXED3 SEL(light3_tanhalfvec, OUT(light3_tanhalfvec));
#elif SHADER_LIGHT_TANHALFVEC == 3
	#define OUT_LIGHTTANHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_tanhalfvec, OUT(light0_tanhalfvec));
		GLSL(varying) FIXED3 SEL(light1_tanhalfvec, OUT(light1_tanhalfvec));
		GLSL(varying) FIXED3 SEL(light2_tanhalfvec, OUT(light2_tanhalfvec));
#elif SHADER_LIGHT_TANHALFVEC == 2
	#define OUT_LIGHTTANHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_tanhalfvec, OUT(light0_tanhalfvec));
		GLSL(varying) FIXED3 SEL(light1_tanhalfvec, OUT(light1_tanhalfvec));
#elif SHADER_LIGHT_TANHALFVEC == 1
	#define OUT_LIGHTTANHALFVEC \
		GLSL(varying) FIXED3 SEL(light0_tanhalfvec, OUT(light0_tanhalfvec));
#endif
#else
#define OUT_LIGHTTANHALFVEC
#endif

#define OUT_LIGHTREGS \
	OUT_LIGHTVEC \
	OUT_LIGHTTANVEC \
	OUT_LIGHTHALFVEC \
	OUT_LIGHTTANHALFVEC

#define ULIGHTREGS \
	ULIGHTPOSREGS \
	ULIGHTDFCOLORREGS \
	ULIGHTSPCOLORREGS

#else
#define ULIGHTREGS
#define OUT_LIGHTREGS
#endif // defined(LIGHTS)

#if defined(SHADER_LIGHT_DIR) || defined(SHADER_LIGHT_HALFDIR)
	#define TANGENT_FRAME
#endif

#undef FOG
#define FOG(_x) _x

struct M {
	FIXED4 color;
#if defined(DEPTH)
	FLOAT depth;
#endif
};
