/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin
   Copyright (C) 2018-2019, Advanced Micro Devices, Inc.
   Copyright (C) 2019, Dave Love, University of Manchester

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name(s) of the copyright holder(s) nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#if 0
  // Used only during standalone testing of ARM support.
  #include "bli_system.h"
  #include "bli_type_defs.h"
  #include "bli_cpuid.h"
  #undef __x86_64__
  #undef _M_X64
  #undef __i386
  #undef _M_IX86
  #define __arm__
#endif

#ifdef BLIS_CONFIGURETIME_CPUID
  #define BLIS_INLINE static
  #define BLIS_EXPORT_BLIS
  #include "bli_system.h"
  #include "bli_type_defs.h"
  #include "bli_cpuid.h"
  #include "bli_arch.h"
#else
  #include "blis.h"
#endif
static char* find_string_in( char* target, char* buffer, size_t buf_len, char* filepath );

// -----------------------------------------------------------------------------

// Allow selecting the micro-architecture via the environment,
// similarly to OpenBLAS, which uses OPENBLAS_CORETYPE.
// The environment should be ignored when configuring.
static arch_t bli_env_check( void )
{
#ifndef BLIS_CONFIGURETIME_CPUID
	char *envval = getenv( "BLIS_CORETYPE" );
	for ( arch_t i = 0; i < BLIS_NUM_ARCHS; i++ )
	{
		if ( envval )
			{
			if ( 0 == strcmp( envval, bli_arch_string (i) ) )
				return i;
			else
				return BLIS_ARCH_GENERIC;
			}
	}
#endif
	return -1;
}

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)

#include "cpuid.h"

arch_t bli_cpuid_query_id( void )
{
	uint32_t vendor, family, model, features;
	arch_t envval = bli_env_check();

// We need to check the environment first for all relevant
// possibilities, though the structure of this is awkward, with
// condtionals below.
#ifdef BLIS_CONFIG_SKX
		if ( BLIS_ARCH_SKX == envval )
			return BLIS_ARCH_SKX;
#endif
#ifdef BLIS_CONFIG_KNL
		if ( BLIS_ARCH_KNL == envval )
			return BLIS_ARCH_KNL;
#endif
#ifdef BLIS_CONFIG_HASWELL
		if ( BLIS_ARCH_HASWELL == envval )
			return BLIS_ARCH_HASWELL;
#endif
#ifdef BLIS_CONFIG_SANDYBRIDGE
		if ( BLIS_ARCH_SANDYBRIDGE == envval )
			return BLIS_ARCH_SANDYBRIDGE;
#endif
#ifdef BLIS_CONFIG_PENRYN
		if ( BLIS_ARCH_PENRYN == envval )
			return BLIS_ARCH_PENRYN;
#endif
#ifdef BLIS_CONFIG_ZEN
		if ( BLIS_ARCH_ZEN == envval )
			return BLIS_ARCH_ZEN;
#endif
#ifdef BLIS_CONFIG_EXCAVATOR
		if ( BLIS_ARCH_EXCAVATOR == envval )
			return BLIS_ARCH_EXCAVATOR;
#endif
#ifdef BLIS_CONFIG_STEAMROLLER
		if ( BLIS_ARCH_STEAMROLLER == envval )
			return BLIS_ARCH_STEAMROLLER;
#endif
#ifdef BLIS_CONFIG_PILEDRIVER
		if ( BLIS_ARCH_PILEDRIVER == envval )
			return BLIS_ARCH_PILEDRIVER;
#endif
#ifdef BLIS_CONFIG_BULLDOZER
		if ( BLIS_ARCH_BULLDOZER == envval )
			return BLIS_ARCH_BULLDOZER;
#endif
		if ( BLIS_ARCH_GENERIC == envval )
			return BLIS_ARCH_GENERIC;

	// Call the CPUID instruction and parse its results into a family id,
	// model id, and a feature bit field. The return value encodes the
	// vendor.
	vendor = bli_cpuid_query( &family, &model, &features );

#if 0
	printf( "vendor   = %s\n", vendor==1 ? "AMD": "INTEL" );
	printf("family    = %x\n", family );
	printf( "model    = %x\n", model );
	
	printf( "features = %x\n", features );
#endif

	if ( vendor == VENDOR_INTEL )
	{
		// Check for each Intel configuration that is enabled, check for that
		// microarchitecture. We check from most recent to most dated.
#ifdef BLIS_CONFIG_SKX
		if ( bli_cpuid_is_skx( family, model, features ) )
			return BLIS_ARCH_SKX;
#endif
#ifdef BLIS_CONFIG_KNL
		if ( bli_cpuid_is_knl( family, model, features ) )
			return BLIS_ARCH_KNL;
#endif
#ifdef BLIS_CONFIG_HASWELL
		if ( bli_cpuid_is_haswell( family, model, features ) )
			return BLIS_ARCH_HASWELL;
#endif
#ifdef BLIS_CONFIG_SANDYBRIDGE
		if ( bli_cpuid_is_sandybridge( family, model, features ) )
			return BLIS_ARCH_SANDYBRIDGE;
#endif
#ifdef BLIS_CONFIG_PENRYN
		if ( bli_cpuid_is_penryn( family, model, features ) )
			return BLIS_ARCH_PENRYN;
#endif
		// If none of the other sub-configurations were detected, return
		// the 'generic' arch_t id value.
		return BLIS_ARCH_GENERIC;
	}
	else if ( vendor == VENDOR_AMD )
	{

		// Check for each AMD configuration that is enabled, check for that
		// microarchitecture. We check from most recent to most dated.
#ifdef BLIS_CONFIG_ZEN2
		if ( bli_cpuid_is_zen2( family, model, features ) )
			return BLIS_ARCH_ZEN2;
#endif
#ifdef BLIS_CONFIG_ZEN
		if ( bli_cpuid_is_zen( family, model, features ) )
			return BLIS_ARCH_ZEN;
#endif
#ifdef BLIS_CONFIG_EXCAVATOR
		if ( bli_cpuid_is_excavator( family, model, features ) )
			return BLIS_ARCH_EXCAVATOR;
#endif
#ifdef BLIS_CONFIG_STEAMROLLER
		if ( bli_cpuid_is_steamroller( family, model, features ) )
			return BLIS_ARCH_STEAMROLLER;
#endif
#ifdef BLIS_CONFIG_PILEDRIVER
		if ( bli_cpuid_is_piledriver( family, model, features ) )
			return BLIS_ARCH_PILEDRIVER;
#endif
#ifdef BLIS_CONFIG_BULLDOZER
		if ( bli_cpuid_is_bulldozer( family, model, features ) )
			return BLIS_ARCH_BULLDOZER;
#endif
		// If none of the other sub-configurations were detected, return
		// the 'generic' arch_t id value.
		return BLIS_ARCH_GENERIC;
	}
	else if ( vendor == VENDOR_UNKNOWN )
	{
		return BLIS_ARCH_GENERIC;
	}

	return BLIS_ARCH_GENERIC;
}

// -----------------------------------------------------------------------------

bool bli_cpuid_is_skx
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX      |
	                          FEATURE_FMA3     |
	                          FEATURE_AVX2     |
	                          FEATURE_AVX512F  |
	                          FEATURE_AVX512DQ |
	                          FEATURE_AVX512BW |
	                          FEATURE_AVX512VL ;


	int nvpu = vpu_count();

	if ( bli_cpuid_has_features( features, expected ) )
	{
		switch ( nvpu )
		{
		case 1:
			bli_arch_log( "Hardware has 1 FMA unit; using 'haswell' (not 'skx') sub-config.\n" );
			return FALSE;
		case 2:
			bli_arch_log( "Hardware has 2 FMA units; using 'skx' sub-config.\n" );
			return TRUE;
		default:
			bli_arch_log( "Number of FMA units unknown; using 'haswell' (not 'skx') config.\n" );
			return FALSE;
		}
	}
	else
		return FALSE;

	return TRUE;
}

bool bli_cpuid_is_knl
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX     |
	                          FEATURE_FMA3    |
	                          FEATURE_AVX2    |
	                          FEATURE_AVX512F |
	                          FEATURE_AVX512PF;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_haswell
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX  |
	                          FEATURE_FMA3 |
	                          FEATURE_AVX2;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_sandybridge
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_penryn
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_SSE3 |
	                          FEATURE_SSSE3;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	return TRUE;
}

// -----------------------------------------------------------------------------

bool bli_cpuid_is_zen2
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX  |
	                          FEATURE_FMA3 |
	                          FEATURE_AVX2;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	// All Zen2 cores have a family of 0x17.
	if ( family != 0x17 ) return FALSE;

	// Finally, check for specific models:
	// - 0x30-0xff (THIS NEEDS UPDATING)
	const bool is_arch
	=
	( 0x30 <= model && model <= 0xff );

	if ( !is_arch ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_zen
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX  |
	                          FEATURE_FMA3 |
	                          FEATURE_AVX2;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	// All Zen cores have a family of 0x17.
	if ( family != 0x17 ) return FALSE;

	// Finally, check for specific models:
	// - 0x00-0xff (THIS NEEDS UPDATING)
	const bool is_arch
	=
	( 0x00 <= model && model <= 0xff );

	if ( !is_arch ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_excavator
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX  |
	                          FEATURE_FMA3 |
	                          FEATURE_AVX2;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	// All Excavator cores have a family of 0x15.
	if ( family != 0x15 ) return FALSE;

	// Finally, check for specific models:
	// - 0x60-0x7f
	const bool is_arch
	=
	( 0x60 <= model && model <= 0x7f );

	if ( !is_arch ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_steamroller
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX  |
	                          FEATURE_FMA3 |
	                          FEATURE_FMA4;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	// All Steamroller cores have a family of 0x15.
	if ( family != 0x15 ) return FALSE;

	// Finally, check for specific models:
	// - 0x30-0x3f
	const bool is_arch
	=
	( 0x30 <= model && model <= 0x3f );

	if ( !is_arch ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_piledriver
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX  |
	                          FEATURE_FMA3 |
	                          FEATURE_FMA4;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	// All Piledriver cores have a family of 0x15.
	if ( family != 0x15 ) return FALSE;

	// Finally, check for specific models:
	// - 0x02
	// - 0x10-0x1f
	const bool is_arch
	=
	model == 0x02 || ( 0x10 <= model && model <= 0x1f );

	if ( !is_arch ) return FALSE;

	return TRUE;
}

bool bli_cpuid_is_bulldozer
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_AVX |
	                          FEATURE_FMA4;

	if ( !bli_cpuid_has_features( features, expected ) ) return FALSE;

	// All Bulldozer cores have a family of 0x15.
	if ( family != 0x15 ) return FALSE;

	// Finally, check for specific models:
	// - 0x00
	// - 0x01
	const bool is_arch
	=
	( model == 0x00 || model == 0x01 );

	if ( !is_arch ) return FALSE;

	return TRUE;
}

#elif defined(__aarch64__) || defined(__arm__) || defined(_M_ARM)

arch_t bli_cpuid_query_id( void )
{
	uint32_t vendor, model, part, features;
	arch_t envval = bli_env_check();

#ifdef BLIS_CONFIG_THUNDERX2
			if ( BLIS_ARCH_THUNDERX2 == envval )
				return BLIS_ARCH_THUNDERX2;
#endif
#ifdef BLIS_CONFIG_CORTEXA57
			if ( BLIS_ARCH_CORTEXA57 == envval )
				return BLIS_ARCH_CORTEXA57;
#endif
#ifdef BLIS_CONFIG_CORTEXA53
		if ( BLIS_ARCH_CORTEXA53 == envval )
			return BLIS_ARCH_CORTEXA53;
#endif
#ifdef BLIS_CONFIG_CORTEXA15
			if ( BLIS_ARCH_CORTEXA15 == envval )
				return BLIS_ARCH_CORTEXA15;
#endif
#ifdef BLIS_CONFIG_CORTEXA9
			if ( BLIS_ARCH_CORTEXA9 == envval )
				return BLIS_ARCH_CORTEXA9;
#endif
			if ( BLIS_ARCH_GENERIC == envval )
				return BLIS_ARCH_GENERIC;

	vendor = bli_cpuid_query( &model, &part, &features );

#if 0
	printf( "vendor   = %u\n", vendor );
	printf( "model    = %u\n", model );
	printf( "part     = 0x%x\n", part );
	printf( "features = %u\n", features );
#endif



	if ( vendor == VENDOR_ARM )
	{
		if ( model == MODEL_ARMV8 )
		{
			// Check for each ARMv8 configuration that is enabled, check for that
			// microarchitecture. We check from most recent to most dated.
#ifdef BLIS_CONFIG_A64FX
			if ( bli_cpuid_is_a64fx( model, part, features ) )
				return BLIS_ARCH_A64FX;
#endif
#ifdef BLIS_CONFIG_THUNDERX2
			if ( bli_cpuid_is_thunderx2( model, part, features ) )
				return BLIS_ARCH_THUNDERX2;
#endif
#ifdef BLIS_CONFIG_CORTEXA57
			if ( bli_cpuid_is_cortexa57( model, part, features ) )
				return BLIS_ARCH_CORTEXA57;
#endif
#ifdef BLIS_CONFIG_CORTEXA53
		if ( bli_cpuid_is_cortexa53( model, part, features ) )
			return BLIS_ARCH_CORTEXA53;
#endif
			// If none of the other sub-configurations were detected, return
			// the 'generic' arch_t id value.
			return BLIS_ARCH_GENERIC;
		}
		else if ( model == MODEL_ARMV7 )
		{
			// Check for each ARMv7 configuration that is enabled, check for that
			// microarchitecture. We check from most recent to most dated.
#ifdef BLIS_CONFIG_CORTEXA15
			if ( bli_cpuid_is_cortexa15( model, part, features ) )
				return BLIS_ARCH_CORTEXA15;
#endif
#ifdef BLIS_CONFIG_CORTEXA9
			if ( bli_cpuid_is_cortexa9( model, part, features ) )
				return BLIS_ARCH_CORTEXA9;
#endif
			// If none of the other sub-configurations were detected, return
			// the 'generic' arch_t id value.
			return BLIS_ARCH_GENERIC;
		}
	}
	else if ( vendor == VENDOR_UNKNOWN )
	{
		return BLIS_ARCH_GENERIC;
	}

	return BLIS_ARCH_GENERIC;
}

bool bli_cpuid_is_a64fx
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	return model == BLIS_ARCH_A64FX;
}

bool bli_cpuid_is_thunderx2
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	return model == BLIS_ARCH_THUNDERX2;
}

bool bli_cpuid_is_cortexa57
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	return model == BLIS_ARCH_CORTEXA57;
}

bool bli_cpuid_is_cortexa53
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	return model == BLIS_ARCH_CORTEXA53;
}

bool bli_cpuid_is_cortexa15
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_NEON;

	return bli_cpuid_has_features( features, expected ) && model == 0xc0f;
}

bool bli_cpuid_is_cortexa9
     (
       uint32_t family,
       uint32_t model,
       uint32_t features
     )
{
	// Check for expected CPU features.
	const uint32_t expected = FEATURE_NEON;

	return bli_cpuid_has_features( features, expected ) && model == 0xc09;
}

#endif

// -----------------------------------------------------------------------------

//
// This section of the file was based off of cpuid.cxx from TBLIS [1].
//
// [1] https://github.com/devinamatthews/tblis
//

/*

   Copyright (C) 2017, The University of Texas at Austin
   Copyright (C) 2017, Devin Matthews
   Copyright (C) 2018 - 2019, Advanced Micro Devices, Inc.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name(s) of the copyright holder(s) nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)

enum
{
                                      // input register(s)     output register
	FEATURE_MASK_SSE3     = (1u<< 0), // cpuid[eax=1]         :ecx[0]
	FEATURE_MASK_SSSE3    = (1u<< 9), // cpuid[eax=1]         :ecx[9]
	FEATURE_MASK_SSE41    = (1u<<19), // cpuid[eax=1]         :ecx[19]
	FEATURE_MASK_SSE42    = (1u<<20), // cpuid[eax=1]         :ecx[20]
	FEATURE_MASK_AVX      = (1u<<28), // cpuid[eax=1]         :ecx[28]
	FEATURE_MASK_AVX2     = (1u<< 5), // cpuid[eax=7,ecx=0]   :ebx[5]
	FEATURE_MASK_FMA3     = (1u<<12), // cpuid[eax=1]         :ecx[12]
	FEATURE_MASK_FMA4     = (1u<<16), // cpuid[eax=0x80000001]:ecx[16]
	FEATURE_MASK_AVX512F  = (1u<<16), // cpuid[eax=7,ecx=0]   :ebx[16]
	FEATURE_MASK_AVX512DQ = (1u<<17), // cpuid[eax=7,ecx=0]   :ebx[17]
	FEATURE_MASK_AVX512PF = (1u<<26), // cpuid[eax=7,ecx=0]   :ebx[26]
	FEATURE_MASK_AVX512ER = (1u<<27), // cpuid[eax=7,ecx=0]   :ebx[27]
	FEATURE_MASK_AVX512CD = (1u<<28), // cpuid[eax=7,ecx=0]   :ebx[28]
	FEATURE_MASK_AVX512BW = (1u<<30), // cpuid[eax=7,ecx=0]   :ebx[30]
	FEATURE_MASK_AVX512VL = (1u<<31), // cpuid[eax=7,ecx=0]   :ebx[31]
	FEATURE_MASK_XGETBV   = (1u<<26)|
                            (1u<<27), // cpuid[eax=1]         :ecx[27:26]
	XGETBV_MASK_XMM       = 0x02u,    // xcr0[1]
	XGETBV_MASK_YMM       = 0x04u,    // xcr0[2]
	XGETBV_MASK_ZMM       = 0xe0u     // xcr0[7:5]
};


uint32_t bli_cpuid_query
     (
       uint32_t* family,
       uint32_t* model,
       uint32_t* features
     )
{
	uint32_t eax, ebx, ecx, edx;

	uint32_t old_model  = 0;
	uint32_t old_family = 0;
	uint32_t ext_model  = 0;
	uint32_t ext_family = 0;

	*family   = 0;
	*model    = 0;
	*features = 0;

	//fprintf( stderr, "checking cpuid\n" );

	uint32_t cpuid_max     = __get_cpuid_max( 0,           0 );
	uint32_t cpuid_max_ext = __get_cpuid_max( 0x80000000u, 0 );

	//fprintf( stderr, "max cpuid leaf: %d\n", cpuid_max );
	//fprintf( stderr, "max extended cpuid leaf: %08x\n", cpuid_max_ext );

	if ( cpuid_max < 1 ) return VENDOR_UNKNOWN;

	// The fourth '0' serves as the NULL-terminator for the vendor string.
	uint32_t vendor_string[4] = { 0, 0, 0, 0 };

	// This is actually a macro that modifies the last four operands,
	// hence why they are not passed by address.
	__cpuid( 0, eax, vendor_string[0],
	                 vendor_string[2],
	                 vendor_string[1] );

	// Check extended feature bits for post-AVX2 features.
	if ( cpuid_max >= 7 )
	{
		// This is actually a macro that modifies the last four operands,
		// hence why they are not passed by address.
		__cpuid_count( 7, 0, eax, ebx, ecx, edx );

		//fprintf( stderr, "cpuid leaf 7:\n" );
		//print_binary( eax );
		//print_binary( ebx );
		//print_binary( ecx );
		//print_binary( edx );

		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX2     ) ) *features |= FEATURE_AVX2;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512F  ) ) *features |= FEATURE_AVX512F;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512DQ ) ) *features |= FEATURE_AVX512DQ;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512PF ) ) *features |= FEATURE_AVX512PF;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512ER ) ) *features |= FEATURE_AVX512ER;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512CD ) ) *features |= FEATURE_AVX512CD;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512BW ) ) *features |= FEATURE_AVX512BW;
		if ( bli_cpuid_has_features( ebx, FEATURE_MASK_AVX512VL ) ) *features |= FEATURE_AVX512VL;
	}

	// Check extended processor info / features bits for AMD-specific features.
	if ( cpuid_max_ext >= 0x80000001u )
	{
		// This is actually a macro that modifies the last four operands,
		// hence why they are not passed by address.
		__cpuid( 0x80000001u, eax, ebx, ecx, edx );

		//fprintf(stderr, "extended cpuid leaf 0x80000001:\n");
		//print_binary(eax);
		//print_binary(ebx);
		//print_binary(ecx);
		//print_binary(edx);

		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_FMA4 ) ) *features |= FEATURE_FMA4;
	}

	// Unconditionally check processor info / features bits.
	{
		// This is actually a macro that modifies the last four operands,
		// hence why they are not passed by address.
		__cpuid( 1, eax, ebx, ecx, edx );

		//fprintf(stderr, "cpuid leaf 1:\n");
		//print_binary(eax);
		//print_binary(ebx);
		//print_binary(ecx);
		//print_binary(edx);

		/*
		   cpuid(eax=1): eax[27:0]

			3: 0 - Stepping
			7: 4 - Model
		   11: 8 - Family
		   13:12 - Processor Type
		   19:16 - Extended Model
		   27:20 - Extended Family

		   Intel and AMD have suggested applications to display the family of a
		   CPU as the sum of the "Family" and the "Extended Family" fields shown
		   above, and the model as the sum of the "Model" and the 4-bit
		   left-shifted "Extended Model" fields. If "Family" is different than
		   6 or 15, only the "Family" and "Model" fields should be used while the
		   "Extended Family" and "Extended Model" bits are reserved. If "Family"
		   is set to 15, then "Extended Family" and the 4-bit left-shifted
		   "Extended Model" should be added to the respective base values, and if
		   "Family" is set to 6, then only the 4-bit left-shifted "Extended Model"
		   should be added to "Model".
		*/

		old_model  = ( eax >>  4 ) & ( 0xF  ); // bits 7:4
		old_family = ( eax >>  8 ) & ( 0xF  ); // bits 11:8

		ext_model  = ( eax >> 16 ) & ( 0xF  ); // bits 19:16
		ext_family = ( eax >> 20 ) & ( 0xFF ); // bits 27:20

		// Set the display model and family values based on the original family
		// value. See explanation above.
		if      ( old_family == 6 )
		{
			*model  = ( ext_model << 4 ) + old_model;
			*family =                      old_family;
		}
		else if ( old_family == 15 )
		{
			*model  = ( ext_model << 4 ) + old_model;
			*family = ( ext_family     ) + old_family;
		}
		else
		{
			*model  =                      old_model;
			*family =                      old_family;
		}

		// Check for SSE, AVX, and FMA3 features.
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_SSE3  ) ) *features |= FEATURE_SSE3;
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_SSSE3 ) ) *features |= FEATURE_SSSE3;
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_SSE41 ) ) *features |= FEATURE_SSE41;
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_SSE42 ) ) *features |= FEATURE_SSE42;
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_AVX   ) ) *features |= FEATURE_AVX;
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_FMA3  ) ) *features |= FEATURE_FMA3;

		// Check whether the hardware supports xsave/xrestor/xsetbv/xgetbv AND 
		// support for these is enabled by the OS. If so, then we proceed with
		// checking that various register-state saving features are available.
		if ( bli_cpuid_has_features( ecx, FEATURE_MASK_XGETBV ) )
		{
			uint32_t xcr = 0;

			// Call xgetbv to get xcr0 (the extended control register) copied
			// to [edx:eax]. This encodes whether software supports various
			// register state-saving features.
			__asm__ __volatile__
			(
				".byte 0x0F, 0x01, 0xD0"
				: "=a" (eax),
				  "=d" (edx)
				: "c"  (xcr)
				: "cc"
			);

			//fprintf(stderr, "xcr0:\n");
			//print_binary(eax);
			//print_binary(edx);

			//fprintf(stderr, "xgetbv: xmm: %d\n", bli_cpuid_has_features(eax, XGETBV_MASK_XMM));
			//fprintf(stderr, "xgetbv: ymm: %d\n", bli_cpuid_has_features(eax, XGETBV_MASK_XMM|
			//                                                XGETBV_MASK_YMM));
			//fprintf(stderr, "xgetbv: zmm: %d\n", bli_cpuid_has_features(eax, XGETBV_MASK_XMM|
			//                                                XGETBV_MASK_YMM|
			//                                                XGETBV_MASK_ZMM));

			// The OS can manage the state of 512-bit zmm (AVX-512) registers
			// only if the xcr[7:5] bits are set. If they are not set, then
			// clear all feature bits related to AVX-512. 
			if ( !bli_cpuid_has_features( eax, XGETBV_MASK_XMM |
				                               XGETBV_MASK_YMM |
				                               XGETBV_MASK_ZMM ) )
			{
				*features &= ~( FEATURE_AVX512F  |
				                FEATURE_AVX512DQ |
				                FEATURE_AVX512PF |
				                FEATURE_AVX512ER |
				                FEATURE_AVX512CD |
				                FEATURE_AVX512BW |
				                FEATURE_AVX512VL );
			}

			// The OS can manage the state of 256-bit ymm (AVX) registers
			// only if the xcr[2] bit is set. If it is not set, then
			// clear all feature bits related to AVX. 
			if ( !bli_cpuid_has_features( eax, XGETBV_MASK_XMM |
				                               XGETBV_MASK_YMM ) )
			{
				*features &= ~( FEATURE_AVX  |
				                FEATURE_AVX2 |
				                FEATURE_FMA3 |
				                FEATURE_FMA4 );
			}

			// The OS can manage the state of 128-bit xmm (SSE) registers
			// only if the xcr[1] bit is set. If it is not set, then
			// clear all feature bits related to SSE (which means the
			// entire bitfield is clear). 
			if ( !bli_cpuid_has_features( eax, XGETBV_MASK_XMM ) )
			{
				*features = 0;
			}
		}
		else
		{
			// If the hardware does not support xsave/xrestor/xsetbv/xgetbv,
			// OR these features are not enabled by the OS, then we clear
			// the bitfield, because it means that not even xmm support is
			// present.

			//fprintf(stderr, "xgetbv: no\n");
			features = 0;
		}
	}

	//fprintf(stderr, "vendor: %12s\n", vendor_string);
	//fprintf(stderr, "family: %d\n", family);
	//fprintf(stderr, "model: %d\n", model);
	//fprintf(stderr, "sse3: %d\n", bli_cpuid_has_features(features, FEATURE_SSE3));
	//fprintf(stderr, "ssse3: %d\n", bli_cpuid_has_features(features, FEATURE_SSSE3));
	//fprintf(stderr, "sse4.1: %d\n", bli_cpuid_has_features(features, FEATURE_SSE41));
	//fprintf(stderr, "sse4.2: %d\n", bli_cpuid_has_features(features, FEATURE_SSE42));
	//fprintf(stderr, "avx: %d\n", bli_cpuid_has_features(features, FEATURE_AVX));
	//fprintf(stderr, "avx2: %d\n", bli_cpuid_has_features(features, FEATURE_AVX2));
	//fprintf(stderr, "fma3: %d\n", bli_cpuid_has_features(features, FEATURE_FMA3));
	//fprintf(stderr, "fma4: %d\n", bli_cpuid_has_features(features, FEATURE_FMA4));
	//fprintf(stderr, "avx512f: %d\n", bli_cpuid_has_features(features, FEATURE_AVX512F));
	//fprintf(stderr, "avx512pf: %d\n", bli_cpuid_has_features(features, FEATURE_AVX512PF));
	//fprintf(stderr, "avx512dq: %d\n", bli_cpuid_has_features(features, FEATURE_AVX512DQ));

	// Check the vendor string and return a value to indicate Intel or AMD.
	if      ( strcmp( ( char* )vendor_string, "AuthenticAMD" ) == 0 )
		return VENDOR_AMD;
	else if ( strcmp( ( char* )vendor_string, "GenuineIntel" ) == 0 )
		return VENDOR_INTEL;
	else
		return VENDOR_UNKNOWN;
}

void get_cpu_name( char *cpu_name )
{
	uint32_t eax, ebx, ecx, edx;

	__cpuid( 0x80000002u, eax, ebx, ecx, edx );
	//printf("%x %x %x %x\n", eax, ebx, ecx, edx);

	*( uint32_t* )&cpu_name[0 + 0] = eax;
	*( uint32_t* )&cpu_name[0 + 4] = ebx;
	*( uint32_t* )&cpu_name[0 + 8] = ecx;
	*( uint32_t* )&cpu_name[0 +12] = edx;

	__cpuid( 0x80000003u, eax, ebx, ecx, edx );
	//printf("%x %x %x %x\n", eax, ebx, ecx, edx);

	*( uint32_t* )&cpu_name[16+ 0] = eax;
	*( uint32_t* )&cpu_name[16+ 4] = ebx;
	*( uint32_t* )&cpu_name[16+ 8] = ecx;
	*( uint32_t* )&cpu_name[16+12] = edx;

	__cpuid( 0x80000004u, eax, ebx, ecx, edx );
	//printf("%x %x %x %x\n", eax, ebx, ecx, edx);

	*( uint32_t* )&cpu_name[32+ 0] = eax;
	*( uint32_t* )&cpu_name[32+ 4] = ebx;
	*( uint32_t* )&cpu_name[32+ 8] = ecx;
	*( uint32_t* )&cpu_name[32+12] = edx;
}

// Return the number of FMA units _assuming avx512 is supported_.
// This needs updating for new processor types, sigh.
// See https://ark.intel.com/content/www/us/en/ark.html#@Processors
// and also https://github.com/jeffhammond/vpu-count
int vpu_count( void )
{
	char  cpu_name[48] = {};
	char* loc;
	char  model_num[5];
	int   sku;

	get_cpu_name( cpu_name );

	if ( strstr( cpu_name, "Intel(R) Xeon(R)" ) != NULL )
	{
		if (( loc = strstr( cpu_name, "Platinum" ) ))
			return 2;
		if ( loc == NULL )
			loc = strstr( cpu_name, "Gold" ); // 1 or 2, tested below
		if ( loc == NULL )
			if (( loc = strstr( cpu_name, "Silver" ) ))
				return 1;
		if ( loc == NULL )
			if (( loc = strstr( cpu_name, "Bronze" ) ))
				return 1;
		if ( loc == NULL )
			loc = strstr( cpu_name, "W" );
		if ( loc == NULL )
			if (( loc = strstr( cpu_name, "D" ) ))
				// Fixme:  May be wrong
				// <https://github.com/jeffhammond/vpu-count/issues/3#issuecomment-542044651>
				return 1;
		if ( loc == NULL )
			return -1;

		// We may have W-nnnn rather than, say, Gold nnnn
		if ( 'W' == *loc && '-' == *(loc+1) )
			loc++;
		else
			loc = strstr( loc+1, " " );
		if ( loc == NULL )
			return -1;

		strncpy( model_num, loc+1, 4 );
		model_num[4] = '\0'; // Things like i9-10900X matched above

		sku = atoi( model_num );

		// These were derived from ARK listings as of 2019-10-09, but
		// may not be complete, especially as the ARK Skylake listing
		// seems to be limited.
		if      ( 8199 >= sku && sku >= 8100 ) return 2;
		else if ( 6199 >= sku && sku >= 6100 ) return 2;
		else if (                sku == 5122 ) return 2;
		else if ( 6299 >= sku && sku >= 6200 ) return 2; // Cascade Lake Gold
		else if ( 5299 >= sku && sku >= 5200 ) return 1; // Cascade Lake Gold
		else if ( 5199 >= sku && sku >= 5100 ) return 1;
		else if ( 4199 >= sku && sku >= 4100 ) return 1;
		else if ( 3199 >= sku && sku >= 3100 ) return 1;
		else if ( 3299 >= sku && sku >= 3200 ) return 2; // Cascade Lake W
		else if ( 2299 >= sku && sku >= 2200 ) return 2; // Cascade Lake W
		else if ( 2199 >= sku && sku >= 2120 ) return 2;
		else if ( 2102 == sku || sku == 2104 ) return 2; // Gold exceptions
		else if ( 2119 >= sku && sku >= 2100 ) return 1;
		else return -1;
	}
	else if ( strstr( cpu_name, "Intel(R) Core(TM)" ) != NULL )
		return 2; // All i7/i9 with avx512?
	else
	{
		return -1;
	}
}

#elif defined(__aarch64__)

#if __linux__
// This is adapted from OpenBLAS.  See
// https://www.kernel.org/doc/html/latest/arm64/cpu-feature-registers.html
// for the mechanism, but not the magic numbers.

// Fixme:  Could these be missing in older Linux?
#include <asm/hwcap.h>
#include <sys/auxv.h>

#ifndef HWCAP_CPUID
#define HWCAP_CPUID (1 << 11)
#endif
/* From https://www.kernel.org/doc/html/latest/arm64/sve.html and the
   aarch64 hwcap.h */
#ifndef HWCAP_SVE
#define HWCAP_SVE (1 << 22)
#endif
/* Maybe also for AT_HWCAP2
#define HWCAP2_SVE2(1 << 1)
et al
) */

static uint32_t get_coretype(void) {
	int implementer, part, midr_el1;

// Fixme:  Presumably we should be checking the vector length as the
// imlementation isn't length-agnostic.
// #ifdef BLI_ARCH_ARMSVE          /* invented -- fixme */
// 	if (!(getauxval(AT_HWCAP) & HWCAP_SVE)) {
// 		/* Assuming no support for specific models */
// 		return BLI_ARCH_ARMSVE;
// 	}
// #endif

	if (!(getauxval(AT_HWCAP) & HWCAP_CPUID)) {
		// Fixme:  We could try reading /sys and /proc here, as below.
		// Find out if that could work when the HWCAP test fails.
		// https://github.com/xianyi/OpenBLAS/issues/2715 says HWCAP_CPUID
		// is a Linux 4.11 feature and that sys/.../midr_el1 is a 4.7
		// feature.  Should caution somewhere about binding to big cores
		// in big-little CPUs.
		return 0;
	}
	// Also available from
	// /sys/devices/system/cpu/cpu0/regs/identification/midr_el1
	// and split out in /proc/cpuinfo (with a tab before the colon):
	// CPU part	: 0x0a1
	__asm("mrs %0, MIDR_EL1" : "=r" (midr_el1));
	/*
	 * MIDR_EL1
	 *
	 * 31		   24 23	 20 19			16 15		   4 3		  0
	 * -----------------------------------------------------------------
	 * | Implementer | Variant | Architecture | Part Number | Revision |
	 * -----------------------------------------------------------------
	 */
	implementer = (midr_el1 >> 24) & 0xFF;
	part		= (midr_el1 >> 4)  & 0xFFF;
	// From Linux arch/arm64/include/asm/cputype.h
	// ARM_CPU_IMP_ARM 0x41
	// ARM_CPU_IMP_APM 0x50
	// ARM_CPU_IMP_CAVIUM 0x43
	// ARM_CPU_IMP_BRCM 0x42
	// ARM_CPU_IMP_QCOM 0x51
	// ARM_CPU_IMP_NVIDIA 0x4E
	// ARM_CPU_IMP_FUJITSU 0x46
	// ARM_CPU_IMP_HISI 0x48
	//
	// ARM_CPU_PART_AEM_V8 0xD0F
	// ARM_CPU_PART_FOUNDATION 0xD00
	// ARM_CPU_PART_CORTEX_A57 0xD07
	// ARM_CPU_PART_CORTEX_A72 0xD08
	// ARM_CPU_PART_CORTEX_A53 0xD03
	// ARM_CPU_PART_CORTEX_A73 0xD09
	// ARM_CPU_PART_CORTEX_A75 0xD0A
	// ARM_CPU_PART_CORTEX_A35 0xD04
	// ARM_CPU_PART_CORTEX_A55 0xD05
	// ARM_CPU_PART_CORTEX_A76 0xD0B
	// ARM_CPU_PART_NEOVERSE_N1 0xD0C
	//
	// APM_CPU_PART_POTENZA 0x000
	//
	// CAVIUM_CPU_PART_THUNDERX 0x0A1
	// CAVIUM_CPU_PART_THUNDERX_81XX 0x0A2
	// CAVIUM_CPU_PART_THUNDERX_83XX 0x0A3
	// CAVIUM_CPU_PART_THUNDERX2 0x0AF
	// CAVIUM_CPU_PART_THUNDERX3 0x0B8  // taken from OpenBLAS
	//
	// BRCM_CPU_PART_BRAHMA_B53 0x100 
	// BRCM_CPU_PART_VULCAN 0x516
	//
	// QCOM_CPU_PART_FALKOR_V1 0x800
	// QCOM_CPU_PART_FALKOR 0xC00
	// QCOM_CPU_PART_KRYO 0x200
        // QCOM_CPU_PART_KRYO_3XX_SILVER 0x803
        // QCOM_CPU_PART_KRYO_4XX_GOLD 0x804
        // QCOM_CPU_PART_KRYO_4XX_SILVER 0x805
	//
	// NVIDIA_CPU_PART_DENVER 0x003
	// NVIDIA_CPU_PART_CARMEL 0x004
	//
	// FUJITSU_CPU_PART_A64FX 0x001
	//
	// HISI_CPU_PART_TSV110 0xD01

	// Fixme:  After merging the vpu_count branch we could report the
	// part here with bli_dolog.
	switch(implementer)
	{
		case 0x41:		// ARM
			switch (part)
			{
				case 0xd07: // Cortex A57
					return BLIS_ARCH_CORTEXA57;
				case 0xd03: // Cortex A53
					return BLIS_ARCH_CORTEXA53;
			}
			break;
		case 0x42:		// Broadcom
			switch (part)
			{
				case 0x516: // Vulcan
					return BLIS_ARCH_THUNDERX2;
			}
			break;
		case 0x43:		// Cavium
			switch (part)
			{
				case 0x0af: // ThunderX2
				case 0x0b8: // ThunderX3
					return BLIS_ARCH_THUNDERX2;
			}
			break;
		case 0x46:      // Fujitsu
			switch (part)
			{
				case 0x001: // A64FX
					return BLIS_ARCH_A64FX;
			}
			break;
        // Fixme: OpenBLAS 
	}
	// Fixme: OpenBLAS uses a57 basic config for others, but says
	// -mtune will speed them up
	// <https://github.com/xianyi/OpenBLAS/commit/310ea55f29f16771438386fb2f1f140e2fd7e397>
	// It has Neoverse-N1 as -march=armv8.2-a -mtune=neoverse-n1 (gcc
	// 9+, else v8-a a72) plus some TX2 level 1 and 2 bits; as gravitin2, l1d 64k, l2 1024, l3 32M, Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm lrcpc dcpop asimddp ssbs
	return BLIS_ARCH_CORTEXA57;
}
#endif

uint32_t bli_cpuid_query
     (
       uint32_t* model,
       uint32_t* part,
       uint32_t* features
     )
{
	*model	  = MODEL_ARMV8;
	*part     = 0;
	*features = 0;
#if __linux__
	*part	  = get_coretype();
#endif

	return VENDOR_ARM;
}

#elif defined(__arm__) || defined(_M_ARM)

/* 
   I can't easily find documentation to do this as for aarch64, though
   it presumably could be unearthed from Linux code.  However, on
   Linux 5.2 (and Androids's 3.4), /proc/cpuinfo has this sort of
   thing, used below:

   CPU implementer	: 0x41
   CPU architecture: 7
   CPU variant	: 0x3
   CPU part	: 0xc09

   The complication for family selection is that Neon is optional for
   CortexA9, for instance.  That's tested in bli_cpuid_is_cortexa9.

   When reading /proc/cpuinfo, we should check the entry corresponding
   to the core we're actually running on, in case the system is
   heterogeneous (big.little).

   arch/arm/include/asm/cputype.h has:

   ARM_CPU_IMP_ARM			0x41
   ARM_CPU_IMP_BRCM		0x42
   ARM_CPU_IMP_DEC			0x44
   ARM_CPU_IMP_INTEL		0x69

   ARM implemented processors 
   ARM_CPU_PART_ARM1136		0x4100b360
   ARM_CPU_PART_ARM1156		0x4100b560
   ARM_CPU_PART_ARM1176		0x4100b760
   ARM_CPU_PART_ARM11MPCORE	0x4100b020
   ARM_CPU_PART_CORTEX_A8		0x4100c080
   ARM_CPU_PART_CORTEX_A9		0x4100c090
   ARM_CPU_PART_CORTEX_A5		0x4100c050
   ARM_CPU_PART_CORTEX_A7		0x4100c070
   ARM_CPU_PART_CORTEX_A12		0x4100c0d0
   ARM_CPU_PART_CORTEX_A17		0x4100c0e0
   ARM_CPU_PART_CORTEX_A15		0x4100c0f0
   ARM_CPU_PART_CORTEX_A53		0x4100d030
   ARM_CPU_PART_CORTEX_A57		0x4100d070
   ARM_CPU_PART_CORTEX_A72		0x4100d080
   ARM_CPU_PART_CORTEX_A73		0x4100d090
   ARM_CPU_PART_CORTEX_A75		0x4100d0a0
   ARM_CPU_PART_MASK		0xff00fff0

   Broadcom implemented processors 
   ARM_CPU_PART_BRAHMA_B15		0x420000f0
   ARM_CPU_PART_BRAHMA_B53		0x42001000

   DEC implemented cores 
   ARM_CPU_PART_SA1100		0x4400a110

   Intel implemented cores 
   ARM_CPU_PART_SA1110		0x6900b110
   ARM_CPU_REV_SA1110_A0		0
   ARM_CPU_REV_SA1110_B0		4
   ARM_CPU_REV_SA1110_B1		5
   ARM_CPU_REV_SA1110_B2		6
   ARM_CPU_REV_SA1110_B4		8

   ARM_CPU_XSCALE_ARCH_MASK	0xe000
   ARM_CPU_XSCALE_ARCH_V1		0x2000
   ARM_CPU_XSCALE_ARCH_V2		0x4000
   ARM_CPU_XSCALE_ARCH_V3		0x6000

   Qualcomm implemented cores
   ARM_CPU_PART_SCORPION		0x510002d0

   The list must be pretty incomplete.  A phone example in Android:
   "Qualcomm MSM 8974 HAMMERHEAD" (AKA Snapdragon 800):
   implementor: 0x51; variant: 0x2;  part: 0x06f

   ----

   Although it looks as if you should be able to do something similar
   to POWER, instead of reading /proc, like:

   #include <sys/auxv.h>
   #include <linux/auxvec.h>  // for AT_PLATFORM
   char * platform = NULL;
   platform = (char*) getauxval (AT_PLATFORM);

   but that just yields "v7l" on cortexa9.

 */

#define TEMP_BUFFER_SIZE 200

uint32_t bli_cpuid_query
     (
       uint32_t* model,
       uint32_t* part,
       uint32_t* features
     )
{
	*model    = MODEL_UNKNOWN;
    *part     = 0;
	*features = 0;

	char* pci_str = "/proc/cpuinfo";

	char  proc_str[ TEMP_BUFFER_SIZE ];
	char  ptno_str[ TEMP_BUFFER_SIZE ];
	char  feat_str[ TEMP_BUFFER_SIZE ];
	char* r_val;

	//printf( "bli_cpuid_query(): beginning search\n" );

	// Search /proc/cpuinfo for the 'Processor' entry.
	r_val = find_string_in( "Processor", proc_str, TEMP_BUFFER_SIZE, pci_str );
	if ( r_val == NULL ) return VENDOR_ARM;

	// Search /proc/cpuinfo for the 'CPU part' entry.
	r_val = find_string_in( "CPU part",  ptno_str, TEMP_BUFFER_SIZE, pci_str );
	if ( r_val == NULL ) return VENDOR_ARM;

	// Search /proc/cpuinfo for the 'Features' entry.
	r_val = find_string_in( "Features",  feat_str, TEMP_BUFFER_SIZE, pci_str );
	if ( r_val == NULL ) return VENDOR_ARM;

#if 0
	printf( "bli_cpuid_query(): full processor string: %s\n", proc_str );
	printf( "bli_cpuid_query(): full part num  string: %s\n", ptno_str );
	printf( "bli_cpuid_query(): full features  string: %s\n", feat_str );
#endif

	// Parse the feature string to check for SIMD features.
	if ( strstr( feat_str, "neon"  ) != NULL ||
	     strstr( feat_str, "asimd" ) != NULL )
		*features |= FEATURE_NEON;

	//printf( "bli_cpuid_query(): features var: %u\n", *features );

	// Parse the processor string to uncover the model.
	if      ( strstr( proc_str, "ARMv7"   ) != NULL )
		*model = MODEL_ARMV7;
	else if ( strstr( proc_str, "AArch64" ) != NULL ||
              strstr( proc_str, "ARMv8"   ) )
		*model = MODEL_ARMV8;

	//printf( "bli_cpuid_query(): model: %u\n", *model );

	// Parse the part number string.
	r_val = strstr( ptno_str, "0x" );
    if ( r_val != NULL)
    {
	    *part = strtol( r_val, NULL, 16 );
    }
	//printf( "bli_cpuid_query(): part#: %x\n", *part );

	return VENDOR_ARM;
}

// https://sourceforge.net/p/predef/wiki/Architectures/ lists macros
#elif __PPC64__ /* gcc */ || __ppc64__ /* should be gcc, but failed */ || \
      _ARCH_PPC64 /* xlc */
// NB POWER7 isn't actually used.  (ppc64le Linux is only supported on
// POWER8+.  Is BLIS is supposed to support big-endian?)

#if !__linux__
uint32_t bli_cpuid_query
     (
       uint32_t* model,
       uint32_t* part,
       uint32_t* features
     )
{
	*model	  = 0;			// Not used
	*features = 0;			// Not used
	*part = BLIS_ARCH_GENERIC;
	return 0;
}
#else  // __linux__
#include <sys/auxv.h>

uint32_t bli_cpuid_query
     (
       uint32_t* model,
       uint32_t* part,
       uint32_t* features
     )
{
	*model	  = 0;			// Not used
	*features = 0;			// Not used
	*part = BLIS_ARCH_GENERIC;

#if 0
// The easy GCC (and recent clang?) version
#  if __GNUC__ >= 6
	if ( __builtin_cpu_is( "power9" ) )
		*part = BLIS_ARCH_POWER9;
	else
#  endif
		if ( __builtin_cpu_is( "power8" ) )
		*part = BLIS_ARCH_POWER8;
	else if ( __builtin_cpu_is( "power7" ) )
		*part = BLIS_ARCH_POWER7;
#else
// See https://developer.ibm.com/tutorials/optimized-libraries-for-linux-on-power/
	char * platform = (char*) getauxval (AT_PLATFORM);
        if ( !platform ) return 0; // Fixme: can it be null?
	if ( strcmp( platform, "power8" ) == 0 )
		*part = BLIS_ARCH_POWER8;
	else if ( strcmp( platform, "power9" ) == 0 )
		*part = BLIS_ARCH_POWER9;
	else if ( strcmp( platform, "power7" ) == 0 )
		*part = BLIS_ARCH_POWER7;
#endif
	return 0;
}
#endif

arch_t bli_cpuid_query_id( void )
{
	uint32_t model, part, features;
#if defined BLIS_CONFIG_POWER9 || defined BLIS_CONFIG_POWER8
	arch_t envval = bli_env_check();
#endif

#ifdef BLIS_CONFIG_POWER9
	if ( BLIS_ARCH_POWER9 == envval )
		return BLIS_ARCH_POWER9;
#endif
#ifdef BLIS_CONFIG_POWER8
	if ( BLIS_ARCH_POWER8 == envval )
		return BLIS_ARCH_POWER8;
#endif
	if ( BLIS_ARCH_GENERIC == envval )
		return BLIS_ARCH_GENERIC;

	(void) bli_cpuid_query( &model, &part, &features );

// NB.  Must use #ifdef, not #if (for configure)
#ifdef BLIS_CONFIG_POWER9
	if ( part == BLIS_ARCH_POWER9 )
		return BLIS_ARCH_POWER9;
#endif
#ifdef BLIS_CONFIG_POWER8
	if ( part == BLIS_ARCH_POWER8 )
		return BLIS_ARCH_POWER8;
#endif
	return BLIS_ARCH_GENERIC;
}

#elif __s390x__	 /* gcc */ || __zarch__ /* clang */ || __SYSC_ZARCH__ /* Systems/C */

// Z13 introduced SIMD, so we should compile the generic kernel for
// that if possible.  z14 vector introduced vector float.  (Fedora
// appears to target Z12, so s390x must cover non-SIMD Z.)

// Fixme:  See if default block sizes etc. are appropriate; 32x128b
// SIMD registers <https://www.ibm.com/downloads/cas/WVPALM0N>.

/*
Example cpuinfo (from Fedora build system):

CPU info:
Architecture:        s390x
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Big Endian
CPU(s):              2
On-line CPU(s) list: 0,1
Thread(s) per core:  1
Core(s) per socket:  1
Socket(s) per book:  1
Book(s) per drawer:  1
Drawer(s):           2
NUMA node(s):        1
Vendor ID:           IBM/S390
Machine type:        2964
CPU dynamic MHz:     5000
CPU static MHz:      5000
BogoMIPS:            3033.00
Hypervisor:          z/VM 6.4.0
Hypervisor vendor:   IBM
Virtualization type: full
Dispatching mode:    horizontal
L1d cache:           128K
L1i cache:           96K
L2d cache:           2048K
L2i cache:           2048K
L3 cache:            65536K
L4 cache:            491520K
NUMA node0 CPU(s):   0,1
Flags:               esan3 zarch stfle msa ldisp eimm dfp edat etf3eh highgprs te vx sie

And one from a shell, that's completely different.  As with aarch64,
we could probably have multiple CPU types, but we presumably only have
ine instruction set.

vendor_id       : IBM/S390
# processors    : 4
bogomips per cpu: 3241.00
max thread id   : 0
features: esan3 zarch stfle msa ldisp eimm dfp edat etf3eh highgprs te vx vxd vxe gs vxe2 vxp sort dflt sie 
facilities      : 0 1 2 3 4 6 7 8 9 10 12 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 30 31 32 33 34 35 36 37 38 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 57 58 59 60 61 73 74 75 76 77 80 81 82 128 129 130 131 133 134 135 146 147 148 150 151 152 155 156 168
cache0          : level=1 type=Data scope=Private size=128K line_size=256 associativity=8
cache1          : level=1 type=Instruction scope=Private size=128K line_size=256 associativity=8
cache2          : level=2 type=Data scope=Private size=4096K line_size=256 associativity=8
cache3          : level=2 type=Instruction scope=Private size=4096K line_size=256 associativity=8
cache4          : level=3 type=Unified scope=Shared size=262144K line_size=256 associativity=32
cache5          : level=4 type=Unified scope=Shared size=983040K line_size=256 associativity=60
processor 0: version = FF,  identification = 200000,  machine = 8561
processor 1: version = FF,  identification = 200001,  machine = 8561
processor 2: version = FF,  identification = 200002,  machine = 8561
processor 3: version = FF,  identification = 200003,  machine = 8561

cpu number      : 0
physical id     : 0
core id         : 0
book id         : 0
drawer id       : 0
dedicated       : 0
address         : 0
siblings        : 1
cpu cores       : 1
version         : FF
identification  : 200000
machine         : 8561
cpu MHz dynamic : 5200
cpu MHz static  : 5200

...
*/

#if !__linux__
uint32_t bli_cpuid_query
     (
       uint32_t* model,
       uint32_t* part,
       uint32_t* features
     )
{

	*model = MODEL_Z900;
        return VENDOR_IBM;
}
#else  // __linux__
#include <sys/auxv.h>
uint32_t bli_cpuid_query
     (
       uint32_t* model,
       uint32_t* part,
       uint32_t* features
     )
{

	*model = MODEL_Z900;
#if __GLIBC__ && __GLIBC_PREREQ(2, 16)
// Prefer not reading /proc (following OpenBLAS)
	{
		unsigned long hwcap = getauxval( AT_HWCAP );
		if (( hwcap & HWCAP_S390_VX ) && ( hwcap & HWCAP_S390_VXE ))
			*model = MODEL_Z14;	// DP SIMD
		else if ( hwcap & HWCAP_S390_VX )
			*model = MODEL_Z13;	// SP SIMD
		// Else no SIMD
	}
#else
	char  mstr[ 200 ];
	char *r_val;
	// Given the cpuinfo examples above, presumably we should look at
	// features/flags rather than the machine id.
  	r_val = find_string_in( "facilities", mstr, sizeof mstr,
							"/proc/cpuinfo" );
	if ( r_val == NULL )
	{
		r_val = find_string_in( "flags", mstr, sizeof mstr,
								"/proc/cpuinfo" );
		if ( r_val == NULL ) return VENDOR_IBM;
	}
	r_val = strstr( r_val, "vx" ); // DP SIMD
	if ( r_val == NULL ) return VENDOR_IBM;
	*model = MODEL_Z13;
	r_val = strstr( r_val, "vxe" ); // SP SIMD
	if ( r_val != NULL ) *model = MODEL_Z14;
#endif
	return VENDOR_IBM;
}
#endif  // __linux__

arch_t bli_cpuid_query_id( void )
{
	uint32_t model, part, features;
#if defined BLIS_CONFIG_Z14 || defined BLIS_CONFIG_Z13
	arch_t envval = bli_env_check();
#endif
#ifdef BLIS_CONFIG_Z14
	if ( BLIS_ARCH_Z14 == envval )
		return BLIS_ARCH_Z14;
#endif
#ifdef BLIS_CONFIG_Z13
	if ( BLIS_ARCH_Z13 == envval )
		return BLIS_ARCH_Z13;
#endif
	if ( BLIS_ARCH_GENERIC == envval )
		return BLIS_ARCH_GENERIC;

	(void) bli_cpuid_query( &model, &part, &features );

// NB.  Must use #ifdef, not #if (for configure)
#ifdef BLIS_CONFIG_Z14
	if ( model == MODEL_Z14 )
		return BLIS_ARCH_Z14;
#endif
#ifdef BLIS_CONFIG_Z13
	if ( model == MODEL_Z13 )
		return BLIS_ARCH_Z13;
#endif
	return BLIS_ARCH_GENERIC;
}

#endif

#if __s390x__ || __zarch__ || __SYSC_ZARCH__ || __arm__ || _M_ARM
static char* find_string_in( char* target, char* buffer, size_t buf_len, char* filepath )
{
	// This function searches for the first line of the file located at
	// 'filepath' that contains the string 'target' and then copies that
	// line (actually, the substring of the line starting with 'target')
	// to 'buffer', which is 'buf_len' bytes long.

	char* r_val = NULL;

	// Allocate a temporary local buffer equal to the size of buffer.
	char* buf_local = malloc( buf_len * sizeof( char ) );

	// Open the file stream.
	FILE* stream = fopen( filepath, "r" );

	// Repeatedly read in a line from the stream, storing the contents of
	// the stream into buf_local.
	while ( !feof( stream ) )
	{
		// Read in the current line, up to buf_len-1 bytes.
		r_val = fgets( buf_local, buf_len-1, stream );

		//printf( "read line: %s", buf_local );

		// fgets() returns the pointer specified by the first argument (in
		// this case, buf_local) on success and NULL on error.
		if ( r_val == NULL ) break;

		// Since fgets() was successful, we can search for the target string
		// within the current line, as captured in buf_local.
		r_val = strstr( buf_local, target );

		// If the target string was found in buf_local, we save it to buffer.
		if ( r_val != NULL )
		{
			//printf( "  found match to '%s'\n", target );

			// Copy the string read by fgets() to the caller's buffer.
			strncpy( buffer, buf_local, buf_len );

			// Make sure that we have a terminating null character by the
			// end of the buffer.
			if ( buf_len > 0 ) buffer[ buf_len - 1 ] = '\0';

			// Leave the loop since we found the target string.
			break;
		}
	}

	// Close the file stream.
	fclose( stream );

	// Free the temporary local buffer.
	free( buf_local );

	// Return r_val so the caller knows if we failed.
	return r_val;
}
#endif

