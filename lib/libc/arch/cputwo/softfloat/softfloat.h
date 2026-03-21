/*	$NetBSD$	*/

/* This is a derivative work. */

/*
===============================================================================

This C header file is part of the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2a.

Written by John R. Hauser.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.

===============================================================================
*/

/*
-------------------------------------------------------------------------------
No extended or quadruple precision on CPUTwo (32-bit, no FPU).
-------------------------------------------------------------------------------
*/
/* #define FLOATX80 */
/* #define FLOAT128 */

#include <machine/ieeefp.h>

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point types.
-------------------------------------------------------------------------------
*/
typedef unsigned int float32;
typedef unsigned long long float64;

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point underflow tininess-detection mode.
-------------------------------------------------------------------------------
*/
#ifndef SOFTFLOAT_FOR_GCC
extern int float_detect_tininess;
#endif
enum {
    float_tininess_after_rounding  = 0,
    float_tininess_before_rounding = 1
};

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point rounding mode.
-------------------------------------------------------------------------------
*/
extern fp_rnd float_rounding_mode;
#define float_round_nearest_even	FP_RN
#define float_round_to_zero		FP_RZ
#define float_round_down		FP_RM
#define float_round_up			FP_RP

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point exception flags.
-------------------------------------------------------------------------------
*/
extern fp_except float_exception_flags;
extern fp_except float_exception_mask;
enum {
    float_flag_inexact   = FP_X_IMP,
    float_flag_underflow = FP_X_UFL,
    float_flag_overflow  = FP_X_OFL,
    float_flag_divbyzero = FP_X_DZ,
    float_flag_invalid   = FP_X_INV
};

void float_raise( fp_except );

float32 int32_to_float32( int32 );
float32 uint32_to_float32( uint32 );
float64 int32_to_float64( int32 );
float64 uint32_to_float64( uint32 );
#ifndef SOFTFLOAT_FOR_GCC
float32 int64_to_float32( long long );
float64 int64_to_float64( long long );
#endif

int float32_to_int32( float32 );
int float32_to_int32_round_to_zero( float32 );
#if defined(SOFTFLOAT_FOR_GCC) && defined(SOFTFLOAT_NEED_FIXUNS)
unsigned int float32_to_uint32_round_to_zero( float32 );
#endif
#ifndef SOFTFLOAT_FOR_GCC
long long float32_to_int64( float32 );
long long float32_to_int64_round_to_zero( float32 );
#endif
float64 float32_to_float64( float32 );

float32 float32_round_to_int( float32 );
float32 float32_add( float32, float32 );
float32 float32_sub( float32, float32 );
float32 float32_mul( float32, float32 );
float32 float32_div( float32, float32 );
float32 float32_rem( float32, float32 );
float32 float32_sqrt( float32 );
int float32_eq( float32, float32 );
int float32_le( float32, float32 );
int float32_lt( float32, float32 );
int float32_eq_signaling( float32, float32 );
int float32_le_quiet( float32, float32 );
int float32_lt_quiet( float32, float32 );
#ifndef SOFTFLOAT_FOR_GCC
int float32_is_signaling_nan( float32 );
#endif

int float64_to_int32( float64 );
int float64_to_int32_round_to_zero( float64 );
#if defined(SOFTFLOAT_FOR_GCC) && defined(SOFTFLOAT_NEED_FIXUNS)
unsigned int float64_to_uint32_round_to_zero( float64 );
#endif
#ifndef SOFTFLOAT_FOR_GCC
long long float64_to_int64( float64 );
long long float64_to_int64_round_to_zero( float64 );
#endif
float32 float64_to_float32( float64 );

float64 float64_round_to_int( float64 );
float64 float64_add( float64, float64 );
float64 float64_sub( float64, float64 );
float64 float64_mul( float64, float64 );
float64 float64_div( float64, float64 );
float64 float64_rem( float64, float64 );
float64 float64_sqrt( float64 );
int float64_eq( float64, float64 );
int float64_le( float64, float64 );
int float64_lt( float64, float64 );
int float64_eq_signaling( float64, float64 );
int float64_le_quiet( float64, float64 );
int float64_lt_quiet( float64, float64 );
#ifndef SOFTFLOAT_FOR_GCC
int float64_is_signaling_nan( float64 );
#endif
