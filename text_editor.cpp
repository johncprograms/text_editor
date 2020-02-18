void
LogUI( void* cstr ... );

#define FINDLEAKS   0

//
// comment text that i'll use for searchability later:
//     TODO: something left to do here.
//     PERF: pointing out a perf problem.
// we should resolve all the TODOs before considering any kind of general release.
// i'm okay with PERFs sticking around; we probably shouldn't resolve them until they become a measurable problem.
//





// ============================================================================
// WINDOWS JUNK


// for memory leak checking, put CrtShowMemleaks() first in every entry point.
#ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #define CrtShowMemleaks()   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF )
#else
  #define CrtShowMemleaks()    // nothing
#endif

#pragma comment( lib, "kernel32" )
#pragma comment( lib, "user32"   )
#pragma comment( lib, "gdi32"    )
//#pragma comment( lib, "winspool" )
//#pragma comment( lib, "comdlg32" )
//#pragma comment( lib, "advapi32" )
#pragma comment( lib, "shell32"  )
#pragma comment( lib, "shcore"   )
//#pragma comment( lib, "ole32"    )
//#pragma comment( lib, "oleaut32" )
//#pragma comment( lib, "uuid"     )
//#pragma comment( lib, "odbc32"   )
//#pragma comment( lib, "odbccp32" )
#pragma comment( lib, "winmm"    )


//#define _CRT_SECURE_NO_WARNINGS


// TODO: move these includes to crt_platform.h
// TODO: move platform specific headers out of this file!

//#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <stddef.h> // for idx_t, etc.
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h> // for isspace, etc.
#include <windows.h>
#include <ShellScalingApi.h>
#include <time.h>
#include <intrin.h>
#include <immintrin.h>
#include <process.h>
#include <dbghelp.h>

// TODO: more undefs

#undef GetProp
#undef min
#undef Min
#undef MIN
#undef max
#undef Max
#undef MAX
#undef clamp
#undef Clamp
#undef CLAMP
#undef abs
#undef Abs
#undef ABS
#undef swap
#undef Swap
#undef SWAP





// ============================================================================
// CORE TYPE DEFINITIONS

typedef int8_t     s8;
typedef int16_t    s16;
typedef int32_t    s32;
typedef int64_t    s64;

#define MIN_s8    Cast( s8, -0x80LL )
#define MAX_s8    Cast( s8,  0x7FLL )

#define MIN_s16   Cast( s16, -0x8000LL )
#define MAX_s16   Cast( s16,  0x7FFFLL )

#define MIN_s32   Cast( s32, -0x80000000LL )
#define MAX_s32   Cast( s32,  0x7FFFFFFFLL )

#define MIN_s64   Cast( s64, -0x8000000000000000LL )
#define MAX_s64   Cast( s64,  0x7FFFFFFFFFFFFFFFLL )


typedef uint8_t    u8;
typedef uint16_t   u16;
typedef uint32_t   u32;
typedef uint64_t   u64;

#define MAX_u8    Cast( u8, 0xFFU )
#define MAX_u16   Cast( u16, 0xFFFFU )
#define MAX_u32   Cast( u32, 0xFFFFFFFFU )
#define MAX_u64   Cast( u64, 0xFFFFFFFFFFFFFFFFULL )


typedef uintptr_t   idx_t;
typedef ptrdiff_t   sidx_t;

typedef float    f32;
typedef double   f64;

#define MIN_f32   ( -3.402823466e+38F )
#define MAX_f32   (  3.402823466e+38F )

#define MIN_f64   ( -1.7976931348623158e+308 )
#define MAX_f64   (  1.7976931348623158e+308 )

#ifdef _M_AMD64

  #define _SIZEOF_IDX_T   8
  #define MAX_idx         MAX_u64
  #define MAX_sidx        MAX_s64
  #define MIN_sidx        MIN_s64

  #define Round_idx_from_f32( x ) \
    Round_u64_from_f32( x )

  #define Round_idx_from_f64( x ) \
    Round_u64_from_f64( x )

  #define GetThreadIdFast() \
    ( *Cast( u32*, Cast( u8*, __readgsqword( 0x30 ) ) + 0x48 ) )

#elif _M_IX86

  #define _SIZEOF_IDX_T   4
  #define MAX_idx         MAX_u32
  #define MAX_sidx        MAX_s32
  #define MIN_sidx        MIN_s32

  #define Round_idx_from_f32( x ) \
    Round_u32_from_f32( x )

  #define Round_idx_from_f64( x ) \
    Round_u32_from_f64( x )

  #define GetThreadIdFast() \
    ( *Cast( u32*, Cast( u8*, __readfsdword( 0x18 ) ) + 0x24 ) )

#else
  #error Define _SIZEOF_IDX_T based on a predefined macro for your new platform.
#endif







// ============================================================================
// CORE LANGUAGE MACROS

#define elif \
  else if


#define Cast( type, var )   ( ( type )( var ) )

// for string literals, we need to cast to u8* because there's no compiler option to make literals unsigned!
#define Str( x )   Cast( u8*, x )


#define ML( a ) \
  ( a ).mem, ( a ).len

#define AL( a ) \
  ( a ), _countof( a )


// dear god, please save us from the infinitesimal wisdom of c++ macros.
// these are needed due to the order of evaluation of macros.
// so, never use ## directly unless you know what you're doing, use this NAMEJOIN instead!
// the _NAMEJOIN is intended to be private and shouldn't be used.
#define _NAMEJOIN( a, b ) a ## b
#define NAMEJOIN( a, b ) _NAMEJOIN( a, b )


#define ForceInl   __forceinline
#define ForceNoInl   __declspec( noinline )

#define NoInl   __declspec( noinline ) static
#define Inl   __forceinline static
//#define Inl   inline static


#define constant      static constexpr
#define constantold   static const


// NOTE: for aligning some set of structs of arbitrary size that you're packing contiguously into an array.
//   we need each field to have aligned loads/stores, which is done by forcing _SIZEOF_IDX_T alignment.
#define ALIGNTOIDX   __declspec( align( _SIZEOF_IDX_T ) )


#define Templ template< typename T >


#define Implies( p, q ) \
  ( !( p )  ||  ( q ) )

#define MIN( a, b )   ( ( ( a ) <= ( b ) )  ?  ( a )  :  ( b ) )
#define MAX( a, b )   ( ( ( a ) >  ( b ) )  ?  ( a )  :  ( b ) )

#define MIN3( a, b, c )   ( MIN( MIN( ( a ), ( b ) ), ( c ) ) )
#define MAX3( a, b, c )   ( MAX( MAX( ( a ), ( b ) ), ( c ) ) )

#define MIN4( a, b, c, d )   ( MIN( MIN( ( a ), ( b ) ), MIN( ( c ), ( d ) ) ) )
#define MAX4( a, b, c, d )   ( MAX( MAX( ( a ), ( b ) ), MAX( ( c ), ( d ) ) ) )

#define CLAMP( x, r0, r1 )   MAX( r0, MIN( r1, x ) )

#define ABS( a )   ( ( ( a ) < 0 )  ?  -( a )  :  ( a ) )

#define SWAP( type, a, b )   do { type tmp = a;  a = b;  b = tmp; } while( 0 )

#define PERMUTELEFT3( type, a, b, c )    do { type tmp = a;  a = b;  b = c;  c = tmp; } while( 0 )
#define PERMUTERIGHT3( type, a, b, c )   do { type tmp = a;  a = c;  c = b;  b = tmp; } while( 0 )

// intrinsics that aren't really intrinsic; need decls:
unsigned char _addcarry_u64( unsigned char c_in, unsigned __int64 a, unsigned __int64 b, unsigned __int64 * out );
unsigned __int64 _lzcnt_u64( unsigned __int64 a );







// ============================================================================
// ENUM MACROS

typedef u32 enum_t;

#define Enumc( name ) \
  enum class name : enum_t

#if 0
  // ugh, c++ doesn't let you do bitmask enums very easily.
  // instead of reusing enumc for this, we'll make our own thing.

  define ENUM_IS_BITMASK( type ) \
    Inl type operator|( type a, type b ) \
    { \
      return Cast( type, Cast( enum_t, a ) | Cast( enum_t, b ) ); \
    } \
    Inl type operator&( type a, type b ) \
    { \
      return Cast( type, Cast( enum_t, a ) & Cast( enum_t, b ) ); \
    } \
    Inl bool operator bool( type a ) \
    { \
      return Cast( enum_t, a ); \
    } \

#endif







// ============================================================================
// FOR LOOP MACROS

#define Fori( idxtype, idx, start, end ) \
  for( idxtype idx = start;  idx < end;  ++idx )

// we can declare things outside the for(), then we can use different types.
// for now, it's fine to use idxtype for the bool too.
#define ReverseFori( idxtype, idx, start, end ) \
  for( \
    idxtype __loop = start < end, idx = MAX( 1, end ) - 1; \
    __loop  &&  idx >= start; \
    idx == start  ?  __loop = 0  :  --idx )

#define Forinc( idxtype, idx, start, end, inc ) \
  for( idxtype idx = start;  idx < end;  idx += inc )

// ideally, we would add a T* val param, so you don't need to do: auto val = &elem->value;
// but, you can't declare two things of different types in a for-decl.
// so we're stuck with this for now.
#define ForList( elem, list ) \
  for( auto elem = ( list ).first;  elem;  elem = ( elem  ?  elem->next  :  0 ) )

#define ReverseForList( elem, list ) \
  for( auto elem = ( list ).last;  elem;  elem = ( elem  ?  elem->prev  :  0 ) )

#define ForNext( elem, firstelem ) \
  for( auto elem = firstelem;  elem;  elem = elem->next )

#define ReverseForPrev( elem, lastelem ) \
  for( auto elem = lastelem;  elem;  elem = elem->prev )

#define ForLen( idx, arr ) \
  Fori( idx_t, idx, 0, ( arr ).len )

#define ReverseForLen( idx, arr ) \
  ReverseFori( idx_t, idx, 0, ( arr ).len )

#define For( idx, start, end ) \
  Fori( idx_t, idx, start, end )

#define ReverseFor( idx, start, end ) \
  ReverseFori( idx_t, idx, start, end )

#define ForEach( elem, list ) \
  for( auto& elem : list )

#define Forever \
  for( ;; )








// ============================================================================
// MEMORY OPERATIONS

#if 1

  // not a macro, due to having an overload that takes lengths for src0 and src1.
  Inl bool
  MemEqual( void* src0, void* src1, idx_t nbytes )
  {
    return ( memcmp( src0, src1, nbytes ) == 0 );
  }

  // we can't have Memmove take args, since it can take macro args, which don't evaluate in proper order.
  #define Memmove                         memmove
  #define Memzero( dst, nbytes )          memset( dst, 0, nbytes )
  #define Typezero( ptr )                 memset( ptr, 0, sizeof( *ptr ) )


#else

  // PERF: try inlining some loops via chunking, to avoid the loop decr cost.
  Inl bool
  MemEqual( void* src0, void* src1, idx_t nbytes )
  {
    if( src0 == src1 ) {
      return 1;
    } else {
      idx_t* aReg = Cast( idx_t*, src0 );
      idx_t* bReg = Cast( idx_t*, src1 );
      idx_t nReg = nbytes / sizeof( idx_t );
      while( nReg-- ) {
        if( *aReg++ != *bReg++ ) {
          return 0;
        }
      }
      u8* a8 = Cast( u8*, aReg );
      u8* b8 = Cast( u8*, bReg );
      idx_t n8 = nbytes % sizeof( idx_t );
      while( n8-- ) {
        if( *a8++ != *b8++ ) {
          return 0;
        }
      }
      return 1;
    }
  }

  // PERF: try inlining some loops via chunking, to avoid the loop decr cost.
  Inl void
  _MemcpyL( void* dst, void* src, idx_t nbytes )
  {
    if( nbytes ) {
      idx_t* dstReg = Cast( idx_t*, dst );
      idx_t* srcReg = Cast( idx_t*, src );
      idx_t nReg = nbytes / sizeof( idx_t );
      while( nReg-- ) {
        *dstReg++ = *srcReg++;
      }
      u8* dst8 = Cast( u8*, dstReg );
      u8* src8 = Cast( u8*, srcReg );
      idx_t n8 = nbytes % sizeof( idx_t );
      while( n8-- ) {
        *dst8++ = *src8++;
      }
    }
  }

  // PERF: try inlining some loops via chunking, to avoid the loop decr cost.
  Inl void
  _MemcpyR( void* dst, void* src, idx_t nbytes )
  {
    if( nbytes ) {
      dst = Cast( u8*, dst ) + nbytes;
      src = Cast( u8*, src ) + nbytes;
      u8* dst8 = Cast( u8*, dst );
      u8* src8 = Cast( u8*, src );
      idx_t n8 = nbytes % sizeof( idx_t );
      while( n8-- ) {
        *--dst8 = *--src8;
      }
      idx_t* dstReg = Cast( idx_t*, dst8 );
      idx_t* srcReg = Cast( idx_t*, src8 );
      idx_t nReg = nbytes / sizeof( idx_t );
      while( nReg-- ) {
        *--dstReg = *--srcReg;
      }
    }
  }


  Inl void
  Memmove( void* dst, void* src, idx_t nbytes )
  {
    if( dst == src ) {
    } elif( ( src < dst )  &  ( Cast( u8*, src ) + nbytes > dst ) ) {
      _MemcpyR( dst, src, nbytes );
    } else {
      _MemcpyL( dst, src, nbytes );
    }
  }


  // PERF: try inlining some loops via chunking, to avoid the loop decr cost.
  Inl void
  Memzero( void* dst, idx_t nbytes )
  {
    if( nbytes ) {
      idx_t* dstReg = Cast( idx_t*, dst );
      idx_t zeroReg = 0;
      idx_t nReg = nbytes / sizeof( idx_t );
      while( nReg-- ) {
        *dstReg++ = zeroReg;
      }
      u8* dst8 = Cast( u8*, dstReg );
      u8 zero8 = 0;
      idx_t n8 = nbytes % sizeof( idx_t );
      while( n8-- ) {
        *dst8++ = zero8;
      }
    }
  }

#endif


Inl bool
MemEqual( void* src0, idx_t src0_len, void* src1, idx_t src1_len )
{
  if( src0_len == src1_len ) {
    return MemEqual( src0, src1, src1_len );
  } else {
    return 0;
  }
}


Inl void
MemReverse( void* src, idx_t nbytes )
{
  auto a = Cast( u8*, src );
  For( i, 0, nbytes / 2 ) {
    SWAP( u8, a[i], a[nbytes - i - 1] );
  }
}

Inl bool
MemScanIdx( idx_t* dst, void* src, idx_t src_len, void* key, idx_t elemsize )
{
  For( i, 0, src_len ) {
    bool equal = MemEqual( src, key, elemsize );
    if( equal ) {
      *dst = i;
      return 1;
    }
    src = Cast( u8*, src ) + elemsize;
  }
  return 0;
}

Inl bool
MemScanIdxRev( idx_t* dst, void* src, idx_t src_len, void* key, idx_t elemsize )
{
  void* iter;
  idx_t i = src_len;
  while( i ) {
    i -= 1;
    iter = Cast( u8*, src ) + i * elemsize;
    bool equal = MemEqual( iter, key, elemsize );
    if( equal ) {
      *dst = i;
      return 1;
    }
  }
  return 0;
}

Inl void*
MemScan( void* src, idx_t src_len, void* key, idx_t elemsize )
{
  if( src_len ) {
    idx_t idx;
    bool found = MemScanIdx( &idx, src, src_len, key, elemsize );
    if( found ) {
      void* res = Cast( u8*, src ) + idx * elemsize;
      return res;
    }
  }
  return 0;
}

Inl void*
MemScanRev( void* src, idx_t src_len, void* key, idx_t elemsize )
{
  if( src_len ) {
    idx_t idx;
    bool found = MemScanIdxRev( &idx, src, src_len, key, elemsize );
    if( found ) {
      void* res = Cast( u8*, src ) + idx * elemsize;
      return res;
    }
  }
  return 0;
}








// ============================================================================
// ASSERT MACROS

Inl void
LogAddIndent( s32 delta );
void
Log( void* cstr ... );
void
LogInline( void* cstr ... );

Inl bool CsFrom_u64( u8*, idx_t, idx_t*, u64, bool );


NoInl void
_WarningTriggered( char* break_if_false, char* file, u64 line, char* function )
{
  u8 tmp[512];
  u64 len = 0;
  u64 tmplen = strlen( file );
  Memmove( tmp + len, file, tmplen );  len += tmplen;
  Memmove( tmp + len, " | ", 3 );  len += 3;
  CsFrom_u64( tmp + len, _countof( tmp ) - len, &tmplen, line, 1 );  len += tmplen;
  Memmove( tmp + len, " | ", 3 );  len += 3;
  tmplen = strlen( function );
  Memmove( tmp + len, function, tmplen );  len += tmplen;
  Memmove( tmp + len, " | ", 3 );  len += 3;
  tmplen = strlen( break_if_false );
  Memmove( tmp + len, break_if_false, tmplen );  len += tmplen;
  tmp[len] = 0;

  Log( "WARNING: %s", tmp );

#ifdef _DEBUG
  __debugbreak();
#endif
}

NoInl void
_CrashTriggered( char* break_if_false, char* file, u64 line, char* function )
{
  u8 tmp[512];
  u64 len = 0;
  u64 tmplen = strlen( file );
  Memmove( tmp + len, file, tmplen );  len += tmplen;
  Memmove( tmp + len, " | ", 3 );  len += 3;
  CsFrom_u64( tmp + len, _countof( tmp ) - len, &tmplen, line, 1 );  len += tmplen;
  Memmove( tmp + len, " | ", 3 );  len += 3;
  tmplen = strlen( function );
  Memmove( tmp + len, function, tmplen );  len += tmplen;
  Memmove( tmp + len, " | ", 3 );  len += 3;
  tmplen = strlen( break_if_false );
  Memmove( tmp + len, break_if_false, tmplen );  len += tmplen;
  tmp[len] = 0;

  Log( "CRASH: %s", tmp );

  MessageBoxA( 0, Cast( LPCSTR, tmp ), "Crash!", 0 );

  __debugbreak();

  *(int*)0 = 0;
}

#define AssertWarn( break_if_false )   \
  do{ \
    if( !( break_if_false ) ) {  \
      _WarningTriggered( #break_if_false, __FILE__, __LINE__, __FUNCTION__ ); \
    } \
  } while( 0 ) \

#define AssertCrash( break_if_false )   \
  do{ \
    if( !( break_if_false ) ) {  \
      _CrashTriggered( #break_if_false, __FILE__, __LINE__, __FUNCTION__ ); \
    } \
  } while( 0 ) \


#define ImplementWarn()   AssertWarn( !"Implement!" );
#define ImplementCrash()     AssertCrash( !"Implement!" );

#define UnreachableWarn()     AssertWarn( !"Unreachable" );
#define UnreachableCrash()       AssertCrash( !"Unreachable" );


#define CompileAssert( error_if_false )    static_assert( error_if_false, # error_if_false )











// ============================================================================
// INTEGER MATH

Inl s32
Clamp_s32_from_s64( s64 x )
{
  if( x > Cast( s64, MAX_s32 ) ) {
    x = MAX_s32;
  } elif( x < Cast( s64, MIN_s32 ) ) {
    x = MIN_s32;
  }
  return Cast( s32, x );
}


Inl u32
RoundDownMultiple4( u32 x )
{
  u32 r = ( x & ~3 );
  return r;
}

Inl u32
RoundUpMultiple4( u32 x )
{
  u32 r = ( ( x + 3 ) & ~3 );
  return r;
}


Inl idx_t
RoundUpMultiple16( idx_t x )
{
  idx_t r = ( ( x + 15 ) & ~15 );
  return r;
}

Inl idx_t
RoundUpMultiple32( idx_t x )
{
  idx_t r = ( ( x + 31 ) & ~31 );
  return r;
}

Inl idx_t
RoundUpToMultipleOfN( idx_t x, idx_t n )
{
  auto rem = x % n;
  if( rem ) {
    x += ( n - rem );
  }
  return x;
}


Inl idx_t
RoundDownMultiplePow2( idx_t x, idx_t pow2 )
{
  idx_t mask = pow2 - 1;
  AssertCrash( ( pow2 & mask ) == 0 );
  idx_t r = ( x & ~mask );
  return r;
}
Inl idx_t
RoundUpMultiplePow2( idx_t x, idx_t pow2 )
{
  idx_t mask = pow2 - 1;
  AssertCrash( ( pow2 & mask ) == 0 );
  idx_t r = ( ( x + mask ) & ~mask );
  return r;
}


#define AllOnes( num_ones ) \
  ( ( 1 << num_ones ) - 1 )



//
// equivalent of:
//     Cast( u8, Round_u32_from_f32( Cast( f32, a ) * Cast( f32, b ) / 255.0f ) )
// for a,b in [0, 256)
//
#if 1

  #define Mul255ByPercentage255( a, b ) \
    Cast( u8, ( ( 2 * Cast( u16, ( a ) ) * Cast( u16, ( b ) ) ) / 255 + 1 ) >> 1 )

#else

  #define Mul255ByPercentage255( a, b ) \
    ( g_mulpcttable[a][b] )

#endif



Inl u64
Pack( u32 hi, u32 lo )
{
  return ( Cast( u64, hi ) << 32ULL ) | Cast( u64, lo );
}




// ============================================================================
// FLOAT MATH

#define f32_PI            ( 3.1415926535897932385f )
#define f32_PI_REC        ( 0.3183098861837906715f )
#define f32_2PI           ( 6.2831853071795864769f )
#define f32_2PI_REC       ( 0.1591549430918953358f )
#define f32_4PI           ( 12.566370614359172954f )
#define f32_4PI_REC       ( 0.0795774715459476679f )
#define f32_E             ( 2.7182818284590452354f )
#define f32_E_REC         ( 0.3678794411714423216f )
#define f32_RT2_REC       ( 0.7071067811865475244f )
#define f32_RAD_PER_DEG   ( f32_PI / 180.0f )
#define f32_DEG_PER_RAD   ( 180.0f / f32_PI )


#define f64_PI            ( 3.1415926535897932385 )
#define f64_PI_REC        ( 0.3183098861837906715 )
#define f64_2PI           ( 6.2831853071795864769 )
#define f64_2PI_REC       ( 0.1591549430918953358 )
#define f64_4PI           ( 12.566370614359172954 )
#define f64_4PI_REC       ( 0.0795774715459476679 )
#define f64_E             ( 2.7182818284590452354 )
#define f64_E_REC         ( 0.3678794411714423216 )
#define f64_RT2_REC       ( 0.7071067811865475244 )
#define f64_RAD_PER_DEG   ( f64_PI / 180.0 )
#define f64_DEG_PER_RAD   ( 180.0 / f64_PI )


Inl f32
rsqrt( f32 r )
{
  f32 half = 0.5f * r;
  f32 result = r;
  s32 bits = *Cast( s32*, &result );
  bits = 0x5F375A86 - ( bits >> 1 );
  result = *Cast( f32*, &bits );
  result = result * ( 1.5f - half * result * result );
  result = result * ( 1.5f - half * result * result );
  return result;
}

Inl f64
rsqrt( f64 r )
{
  f64 half = 0.5 * r;
  f64 result = r;
  s64 bits = *Cast( s64*, &result );
  bits = 0x5FE6EB50C7B537A9L - ( bits >> 1 );
  result = *Cast( f64*, &bits );
  result = result * ( 1.5 - half * result * result );
  result = result * ( 1.5 - half * result * result );
  return result;
}


Inl f32 qsqrt32( f32 r ) { return 1.0f / rsqrt( r ); }
Inl f64 qsqrt64( f64 r ) { return 1.0  / rsqrt( r ); }


// Note that we can't handcode cvttss2si for these! It converts to a signed int; we need extra logic to handle unsigned.
// Let the compiler generate these for us; the C++ spec requires what we need.
#define Truncate_s32_from_f32( x )   Cast( s32, x )
#define Truncate_s64_from_f64( x )   Cast( s64, x )
#define Truncate_s64_from_f32( x )   Cast( s64, x )
#define Truncate_u32_from_f32( x )   Cast( u32, x )
#define Truncate_u64_from_f64( x )   Cast( u64, x )
#define Truncate_u64_from_f32( x )   Cast( u64, x )

#if 0
  // these are still usable if we set the FP rounding mode correctly:
  //   check if these are faster than compiler-generated versions.
  #define Truncate_s32_from_f32( x )   _mm_cvttss_si32( _mm_set_ss( x ) )
  #define Truncate_s64_from_f64( x )   _mm_cvttsd_si64( _mm_set_sd( x ) )
  #define Truncate_s64_from_f32( x )   _mm_cvttss_si64( _mm_set_ss( x ) )

#endif

#define Round_s32_from_f32( x )   Truncate_s32_from_f32( x + 0.5f )
#define Round_s64_from_f64( x )   Truncate_s64_from_f64( x + 0.5  )
#define Round_s64_from_f32( x )   Truncate_s64_from_f32( x + 0.5f )
#define Round_u32_from_f32( x )   Truncate_u32_from_f32( x + 0.5f )
#define Round_u64_from_f64( x )   Truncate_u64_from_f64( x + 0.5  )
#define Round_u64_from_f32( x )   Truncate_u64_from_f32( x + 0.5f )


#define Sq( x )   ( ( x ) * ( x ) )
#define Sign( x )   ( ( ( x ) > 0 ) ? 1 : ( ( ( x ) < 0 ) ? -1 : 0 ) )

#define Floor32( x )   _mm_cvtss_f32( _mm_floor_ps( _mm_set_ss( x ) ) )
#define Floor64( x )   _mm_cvtsd_f64( _mm_floor_pd( _mm_set_sd( x ) ) )

#define Ceil32( x )   _mm_cvtss_f32( _mm_ceil_ps( _mm_set_ss( x ) ) )
#define Ceil64( x )   _mm_cvtsd_f64( _mm_ceil_pd( _mm_set_sd( x ) ) )

#define Truncate32( x )   _mm_cvtss_f32( _mm_round_ps( _mm_set_ss( x ), _MM_FROUND_TO_ZERO ) )
#define Truncate64( x )   _mm_cvtsd_f64( _mm_round_pd( _mm_set_sd( x ), _MM_FROUND_TO_ZERO ) )

#define Round32( x )   _mm_cvtss_f32( _mm_round_ps( _mm_set_ss( x ), _MM_FROUND_TO_NEAREST_INT ) )
#define Round64( x )   _mm_cvtsd_f64( _mm_round_pd( _mm_set_sd( x ), _MM_FROUND_TO_NEAREST_INT ) )

#define Sqrt32( x )   _mm_cvtss_f32( _mm_sqrt_ss( _mm_set_ss( x ) ) )
#define Sqrt64( x )   _mm_cvtsd_f64( _mm_sqrt_sd( _mm_set_sd( x ), _mm_set_sd( x ) ) )

#define Frac32( x )   ( x - Truncate32( x ) )
#define Frac64( x )   ( x - Truncate64( x ) )

#define Remainder32( x, d )   ( x - Truncate32( x / d ) * d )
#define Remainder64( x, d )   ( x - Truncate64( x / d ) * d )

#define Smoothstep32( x )   ( x * x * ( 3.0f - 2.0f * x ) )
#define Smoothstep64( x )   ( x * x * ( 3.0  - 2.0  * x ) )


// PERF: quicker versions of these math functions.

// i was tracing the assembly of the crt impls, to find how they compute this.
// the crt functions have function calls in them, which are terrifyingly slow.
//
// xmm0 = x
// xmm1 = 2.4801587301587298e-05
// xmm2 = 1 - x^2/2
// xmm3 = x^2

#define Cos32( x )   cosf( x )
#define Cos64( x )   cos( x )
Inl f32 Cos( f32 x ) { return Cos32( x ); }
Inl f64 Cos( f64 x ) { return Cos64( x ); }

#define Sin32( x )   sinf( x )
#define Sin64( x )   sin( x )
Inl f32 Sin( f32 x ) { return Sin32( x ); }
Inl f64 Sin( f64 x ) { return Sin64( x ); }

Inl f32 Tan32( f32 x ) { return tanf( x ); }
Inl f64 Tan64( f64 x ) { return tan( x ); }

Inl f32 Pow32( f32 x, f32 e ) { return powf( x, e ); }
Inl f64 Pow64( f64 x, f64 e ) { return pow( x, e ); }

Inl f32 Exp32( f32 x ) { return expf( x ); }
Inl f64 Exp64( f64 x ) { return exp( x ); }


Inl f32 lerp( f32 x0, f32 x1, f32 t ) { return t * x1 + ( 1.0f - t ) * x0; }
Inl f64 lerp( f64 x0, f64 x1, f64 t ) { return t * x1 + ( 1.0  - t ) * x0; }


Inl f64
Lerp_from_f64(
  f64 y0,
  f64 y1,
  f64 x,
  f64 x0,
  f64 x1 )
{
  return y0 + ( ( y1 - y0 ) / ( x1 - x0 ) ) * ( x - x0 );
}

Inl f32
Lerp_from_f32(
  f32 y0,
  f32 y1,
  f32 x,
  f32 x0,
  f32 x1 )
{
  return y0 + ( ( y1 - y0 ) / ( x1 - x0 ) ) * ( x - x0 );
}

Inl f32
Lerp_from_s32(
  f32 y0,
  f32 y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return y0 + ( ( y1 - y0 ) / ( x1 - x0 ) ) * ( x - x0 );
}

Inl f64
Lerp_from_s32(
  f64 y0,
  f64 y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return y0 + ( ( y1 - y0 ) / ( x1 - x0 ) ) * ( x - x0 );
}

Inl s32
Lerp_from_s32(
  s32 y0,
  s32 y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return Round_s32_from_f32( y0 + ( ( y1 - y0 ) / Cast( f32, x1 - x0 ) ) * ( x - x0 ) );
}

Inl s32
Lerp_from_f32(
  s32 y0,
  s32 y1,
  f32 x,
  f32 x0,
  f32 x1 )
{
  return Round_s32_from_f32( y0 + ( ( y1 - y0 ) / ( x1 - x0 ) ) * ( x - x0 ) );
}


Inl bool
PtInInterval( f32 x, f32 x0, f32 x1, f32 epsilon )
{
  bool r = ( x0 - epsilon <= x )  &&  ( x <= x1 + epsilon );
  return r;
}

Inl bool
IntervalsOverlap( f32 x0, f32 x1, f32 y0, f32 y1, f32 epsilon )
{
  bool r =
    PtInInterval( x0, y0, y1, epsilon )  ||
    PtInInterval( x1, y0, y1, epsilon )  ||
    PtInInterval( y0, x0, x1, epsilon )  ||
    PtInInterval( y1, x0, x1, epsilon );
  return r;
}


struct
kahan32_t
{
  f32 sum;
  f32 err;
};

struct
kahan64_t
{
  f64 sum;
  f64 err;
};

Inl void
Add( kahan32_t& kahan, f32 val )
{
  f32 val_corrected = val - kahan.err;
  f32 new_sum = kahan.sum + val_corrected;
  kahan.err = ( new_sum - kahan.sum ) - val_corrected;
  kahan.sum = new_sum;
}

Inl void
Add( kahan64_t& kahan, f64 val )
{
  f64 val_corrected = val - kahan.err;
  f64 new_sum = kahan.sum + val_corrected;
  kahan.err = ( new_sum - kahan.sum ) - val_corrected;
  kahan.sum = new_sum;
}

Inl void
Sub( kahan32_t& kahan, f32 val )
{
  Add( kahan, -val );
}

Inl void
Sub( kahan64_t& kahan, f64 val )
{
  Add( kahan, -val );
}


struct
inc_stats_t
{
  kahan32_t mean;
  idx_t count;
};

Inl void
Init( inc_stats_t& s )
{
  s.mean = {};
  s.count = 0;
}

Inl void
AddValue( inc_stats_t& s, f32 value )
{
  s.count += 1;
  Add( s.mean, ( value - s.mean.sum ) / Cast( f32, s.count ) );
}

Inl void
AddMean( inc_stats_t& s, f32 mean, idx_t count )
{
  AssertCrash( count );
  auto new_count = s.count + count;
  s.mean.sum *= Cast( f32, s.count ) / Cast( f32, new_count );
  s.mean.err = 0;
  mean *= Cast( f32, count ) / Cast( f32, new_count );
  Add( s.mean, mean );
  s.count = new_count;
}





// ============================================================================
// BASIC STRINGS

#define IsWhitespace( c )   ( ( ( c ) == ' ' ) || ( ( c ) == '\t' ) || ( ( c ) == '\r' ) || ( ( c ) == '\n' ) )
#define IsNumber( c )       ( ( '0' <= ( c ) )  &&  ( ( c ) <= '9' ) )
#define IsLower( c )        ( ( 'a' <= ( c ) )  &&  ( ( c ) <= 'z' ) )
#define IsUpper( c )        ( ( 'A' <= ( c ) )  &&  ( ( c ) <= 'Z' ) )
#define IsAlpha( c )        ( IsLower( c )  ||  IsUpper( c ) )
#define InWord( c )         ( IsAlpha( c )  ||  IsNumber( c )  ||  ( c ) == '_' )
#define AsNumber( c )       ( ( c ) - '0' )


Inl idx_t
StringHash( void* key, idx_t key_len )
{
  sidx_t h = 5381;
  s8* s = Cast( s8*, key );
  sidx_t c;
  For( i, 0, key_len ) {
    c = *s++;
    h = c + h * ( h << 5 );
  }
  idx_t r = Cast( idx_t, h );
  return r;
}





// ============================================================================
// ALLOCATION ENTRY POINTS

#define DEFAULT_ALIGN   16 // must be power of 2
#define DEFAULT_ALIGNMASK   ( DEFAULT_ALIGN - 1 )

#ifndef FINDLEAKS
#define FINDLEAKS 0
#endif

#if FINDLEAKS

  static u64 allocs[100000];
  idx_t allocs_len = 0;
  static u64 frees[100000];
  idx_t frees_len = 0;

  static constexpr idx_t c_extra = 64;
  static volatile u64 __counter = 0;
  Inl void*
  MemHeapAllocBytes( idx_t nbytes )
  {
    nbytes += c_extra;
    auto m = Cast( u8*, _aligned_malloc( nbytes, DEFAULT_ALIGN ) );
    u64 counter = ( InterlockedIncrement( &__counter ) - 1 ); // GetValueBeforeAtomicInc
    *Cast( u64*, m ) = counter;
    AssertCrash( allocs_len < _countof( allocs ) );
    allocs[ allocs_len++ ] = counter;

//    if( counter == 2301 ) __debugbreak();

    Log( "alloc # %llu, nbytes %llu", counter, nbytes );

    m += c_extra;
    return m;
  }

  Inl void*
  MemHeapReallocBytes( void* mem, idx_t oldlen, idx_t newlen )
  {
    auto m = Cast( u8*, mem );
    m -= c_extra;
    newlen += c_extra;
    AssertCrash( !( Cast( idx_t, m ) & DEFAULT_ALIGNMASK ) );
    auto mnew = Cast( u8*, _aligned_realloc( m, newlen, DEFAULT_ALIGN ) );
    mnew += c_extra;
    return mnew;
  }

  Inl void
  MemHeapFree( void* mem )
  {
    if( mem ) {
      auto m = Cast( u8*, mem );
      m -= c_extra;
      AssertCrash( frees_len < _countof( frees ) );
      frees[ frees_len++ ] = *Cast( u64*, m );
      AssertCrash( !( Cast( idx_t, m ) & DEFAULT_ALIGNMASK ) );
      _aligned_free( m );
    }
  }

#else

  Inl void*
  MemHeapAllocBytes( idx_t nbytes )
  {
    // NOTE: ~1 Gb heap allocations tended to cause a hang on my old laptop.
    // Anything remotely close to that size should be made by VirtualAlloc, not by the CRT heap!
    AssertCrash( nbytes <= 1ULL*1024*1024*1024 );

    void* mem = _aligned_malloc( nbytes, DEFAULT_ALIGN );
    return mem;
  }


  Inl void*
  MemHeapReallocBytes( void* mem, idx_t oldlen, idx_t newlen )
  {
    // NOTE: ~1 Gb heap allocations tended to cause a hang on my old laptop.
    // Anything remotely close to that size should be made by VirtualAlloc, not by the CRT heap!
    AssertCrash( newlen <= 1ULL*1024*1024*1024 );
    AssertCrash( !( Cast( idx_t, mem ) & DEFAULT_ALIGNMASK ) );

  #if 0
    void* memnew = _aligned_malloc( newlen, DEFAULT_ALIGN );
    Memmove( memnew, mem, MIN( oldlen, newlen ) );
    _aligned_free( mem );
  #else
    void* memnew = _aligned_realloc( mem, newlen, DEFAULT_ALIGN );
  #endif

    return memnew;
  }

  // TODO: zero mem after free for everyone!

  Inl void
  MemHeapFree( void* mem )
  {
    AssertCrash( !( Cast( idx_t, mem ) & DEFAULT_ALIGNMASK ) );
    _aligned_free( mem );
  }

#endif


#define MemHeapAlloc( type, num ) \
  Cast( type*, MemHeapAllocBytes( num * sizeof( type ) ) )

#define MemHeapRealloc( type, mem, oldnum, newnum ) \
  Cast( type*, MemHeapReallocBytes( mem, oldnum * sizeof( type ), newnum * sizeof( type ) ) )





Inl void
MemVirtualFree( void* mem )
{
  BOOL r = VirtualFree( mem, 0, MEM_RELEASE );
  AssertWarn( r );
}

Inl void*
MemVirtualAllocBytes( idx_t nbytes )
{
  //Log( "VA: %llu", nbytes );
  void* mem = VirtualAlloc( 0, nbytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
  return mem;
}

Inl void*
MemVirtualReallocBytes( void* mem, idx_t oldlen, idx_t newlen )
{
  void* memnew = MemVirtualAllocBytes( newlen );
  Memmove( memnew, mem, MIN( oldlen, newlen ) );
  MemVirtualFree( mem );
  return memnew;
}


#define MemVirtualAlloc( type, num ) \
  Cast( type*, MemVirtualAllocBytes( num * sizeof( type ) ) )

#define MemVirtualRealloc( type, mem, oldnum, newnum ) \
  Cast( type*, MemVirtualReallocBytes( mem, oldnum * sizeof( type ), newnum * sizeof( type ) ) )













#if 0

  #define Defer( code ) \
    auto NAMEJOIN( deferred_, __LINE__ ) = _MakeDefer( [&]() { code } )

  Templ struct
  _defer_t
  {
    T lambda;

    ForceInl _defer_t( T t ) :
      lambda( t )
    {
    }

    ForceInl ~_defer_t()
    {
      lambda();
    }
  };

  Templ ForceInl _defer_t<T>
  _MakeDefer( T t )
  {
    return _defer_t<T>( t );
  }

#endif



#if 0

  Templ struct
  ExitScope
  {
    T lambda;
    ExitScope(T t) :
      lambda( t )
    {
    }

    ~ExitScope()
    {
      lambda();
    }

    ExitScope( const ExitScope& );

  private:
    ExitScope& operator=( const ExitScope& );
  };

  struct
  ExitScopeHelp
  {
    Templ ForceInl
    ExitScope<T> operator+(T t)
    {
      return t;
    }
  };

  #define Defer \
    const auto& NAMEJOIN( defer__, __LINE__ ) = ExitScopeHelp() + [&]()

#endif



Templ
struct vec2
{
  T x;
  T y;
};

Templ
struct vec3
{
  T x;
  T y;
  T z;
};

Templ
struct vec4
{
  T x;
  T y;
  T z;
  T w;
};



Templ Inl void Set( vec4<T>* dst, T x, T y, T z, T w ) { dst->x = x;  dst->y = y;  dst->z = z;  dst->w = w; }
Templ Inl void Set( vec3<T>* dst, T x, T y, T z )      { dst->x = x;  dst->y = y;  dst->z = z; }
Templ Inl void Set( vec2<T>* dst, T x, T y )           { dst->x = x;  dst->y = y; }


Templ Inl void Copy( vec4<T>* dst, vec4<T>& src )  { dst->x = src.x;  dst->y = src.y;  dst->z = src.z;  dst->w = src.w; }
Templ Inl void Copy( vec3<T>* dst, vec3<T>& src )  { dst->x = src.x;  dst->y = src.y;  dst->z = src.z; }
Templ Inl void Copy( vec2<T>* dst, vec2<T>& src )  { dst->x = src.x;  dst->y = src.y; }


Templ Inl void Zero( vec4<T>* dst )  { dst->x = 0;  dst->y = 0;  dst->z = 0;  dst->w = 0; }
Templ Inl void Zero( vec3<T>* dst )  { dst->x = 0;  dst->y = 0;  dst->z = 0; }
Templ Inl void Zero( vec2<T>* dst )  { dst->x = 0;  dst->y = 0; }


Templ Inl void
QuatMul( vec4<T>* dst, vec4<T>& a, vec4<T>& b )
{
  dst->x = a.w * b.x + a.x * b.w - a.y * b.z + a.z * b.y;
  dst->y = a.w * b.y + a.x * b.z + a.y * b.w - a.z * b.x;
  dst->z = a.w * b.z - a.x * b.y + a.y * b.x + a.z * b.w;
  dst->w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
}


Templ Inl void Add( vec4<T>* dst, vec4<T>& a, vec4<T>& b )  { dst->x = a.x + b.x;  dst->y = a.y + b.y;  dst->z = a.z + b.z;  dst->w = a.w + b.w; }
Templ Inl void Add( vec3<T>* dst, vec3<T>& a, vec3<T>& b )  { dst->x = a.x + b.x;  dst->y = a.y + b.y;  dst->z = a.z + b.z; }
Templ Inl void Add( vec2<T>* dst, vec2<T>& a, vec2<T>& b )  { dst->x = a.x + b.x;  dst->y = a.y + b.y; }

Templ Inl void Sub( vec4<T>* dst, vec4<T>& a, vec4<T>& b )  { dst->x = a.x - b.x;  dst->y = a.y - b.y;  dst->z = a.z - b.z;  dst->w = a.w - b.w; }
Templ Inl void Sub( vec3<T>* dst, vec3<T>& a, vec3<T>& b )  { dst->x = a.x - b.x;  dst->y = a.y - b.y;  dst->z = a.z - b.z; }
Templ Inl void Sub( vec2<T>* dst, vec2<T>& a, vec2<T>& b )  { dst->x = a.x - b.x;  dst->y = a.y - b.y; }

Templ Inl void Mul( vec4<T>* dst, vec4<T>& a, vec4<T>& b )  { dst->x = a.x * b.x;  dst->y = a.y * b.y;  dst->z = a.z * b.z;  dst->w = a.w * b.w; }
Templ Inl void Mul( vec3<T>* dst, vec3<T>& a, vec3<T>& b )  { dst->x = a.x * b.x;  dst->y = a.y * b.y;  dst->z = a.z * b.z; }
Templ Inl void Mul( vec2<T>* dst, vec2<T>& a, vec2<T>& b )  { dst->x = a.x * b.x;  dst->y = a.y * b.y; }

Templ Inl void Div( vec4<T>* dst, vec4<T>& a, vec4<T>& b )  { dst->x = a.x / b.x;  dst->y = a.y / b.y;  dst->z = a.z / b.z;  dst->w = a.w / b.w; }
Templ Inl void Div( vec3<T>* dst, vec3<T>& a, vec3<T>& b )  { dst->x = a.x / b.x;  dst->y = a.y / b.y;  dst->z = a.z / b.z; }
Templ Inl void Div( vec2<T>* dst, vec2<T>& a, vec2<T>& b )  { dst->x = a.x / b.x;  dst->y = a.y / b.y; }


Templ Inl void Add( vec4<T>* dst, vec4<T>& a, T b )  { dst->x = a.x + b;  dst->y = a.y + b;  dst->z = a.z + b;  dst->w = a.w + b; }
Templ Inl void Add( vec3<T>* dst, vec3<T>& a, T b )  { dst->x = a.x + b;  dst->y = a.y + b;  dst->z = a.z + b; }
Templ Inl void Add( vec2<T>* dst, vec2<T>& a, T b )  { dst->x = a.x + b;  dst->y = a.y + b; }

Templ Inl void Sub( vec4<T>* dst, vec4<T>& a, T b )  { dst->x = a.x - b;  dst->y = a.y - b;  dst->z = a.z - b;  dst->w = a.w - b; }
Templ Inl void Sub( vec3<T>* dst, vec3<T>& a, T b )  { dst->x = a.x - b;  dst->y = a.y - b;  dst->z = a.z - b; }
Templ Inl void Sub( vec2<T>* dst, vec2<T>& a, T b )  { dst->x = a.x - b;  dst->y = a.y - b; }

Templ Inl void Mul( vec4<T>* dst, vec4<T>& a, T b )  { dst->x = a.x * b;  dst->y = a.y * b;  dst->z = a.z * b;  dst->w = a.w * b; }
Templ Inl void Mul( vec3<T>* dst, vec3<T>& a, T b )  { dst->x = a.x * b;  dst->y = a.y * b;  dst->z = a.z * b; }
Templ Inl void Mul( vec2<T>* dst, vec2<T>& a, T b )  { dst->x = a.x * b;  dst->y = a.y * b; }

Templ Inl void Div( vec4<T>* dst, vec4<T>& a, T b )  { dst->x = a.x / b;  dst->y = a.y / b;  dst->z = a.z / b;  dst->w = a.w / b; }
Templ Inl void Div( vec3<T>* dst, vec3<T>& a, T b )  { dst->x = a.x / b;  dst->y = a.y / b;  dst->z = a.z / b; }
Templ Inl void Div( vec2<T>* dst, vec2<T>& a, T b )  { dst->x = a.x / b;  dst->y = a.y / b; }


Templ Inl void Add( vec4<T>* dst, T a, vec4<T>& b )  { dst->x = a + b.x;  dst->y = a + b.y;  dst->z = a + b.z;  dst->w = a + b.w; }
Templ Inl void Add( vec3<T>* dst, T a, vec3<T>& b )  { dst->x = a + b.x;  dst->y = a + b.y;  dst->z = a + b.z; }
Templ Inl void Add( vec2<T>* dst, T a, vec2<T>& b )  { dst->x = a + b.x;  dst->y = a + b.y; }

Templ Inl void Sub( vec4<T>* dst, T a, vec4<T>& b )  { dst->x = a - b.x;  dst->y = a - b.y;  dst->z = a - b.z;  dst->w = a - b.w; }
Templ Inl void Sub( vec3<T>* dst, T a, vec3<T>& b )  { dst->x = a - b.x;  dst->y = a - b.y;  dst->z = a - b.z; }
Templ Inl void Sub( vec2<T>* dst, T a, vec2<T>& b )  { dst->x = a - b.x;  dst->y = a - b.y; }

Templ Inl void Mul( vec4<T>* dst, T a, vec4<T>& b )  { dst->x = a * b.x;  dst->y = a * b.y;  dst->z = a * b.z;  dst->w = a * b.w; }
Templ Inl void Mul( vec3<T>* dst, T a, vec3<T>& b )  { dst->x = a * b.x;  dst->y = a * b.y;  dst->z = a * b.z; }
Templ Inl void Mul( vec2<T>* dst, T a, vec2<T>& b )  { dst->x = a * b.x;  dst->y = a * b.y; }

Templ Inl void Div( vec4<T>* dst, T a, vec4<T>& b )  { dst->x = a / b.x;  dst->y = a / b.y;  dst->z = a / b.z;  dst->w = a / b.w; }
Templ Inl void Div( vec3<T>* dst, T a, vec3<T>& b )  { dst->x = a / b.x;  dst->y = a / b.y;  dst->z = a / b.z; }
Templ Inl void Div( vec2<T>* dst, T a, vec2<T>& b )  { dst->x = a / b.x;  dst->y = a / b.y; }


Templ Inl void Add( vec4<T>* dst, vec4<T>& a )  { Add( dst, *dst, a ); }
Templ Inl void Add( vec3<T>* dst, vec3<T>& a )  { Add( dst, *dst, a ); }
Templ Inl void Add( vec2<T>* dst, vec2<T>& a )  { Add( dst, *dst, a ); }

Templ Inl void Sub( vec4<T>* dst, vec4<T>& a )  { Sub( dst, *dst, a ); }
Templ Inl void Sub( vec3<T>* dst, vec3<T>& a )  { Sub( dst, *dst, a ); }
Templ Inl void Sub( vec2<T>* dst, vec2<T>& a )  { Sub( dst, *dst, a ); }

Templ Inl void Mul( vec4<T>* dst, vec4<T>& a )  { Mul( dst, *dst, a ); }
Templ Inl void Mul( vec3<T>* dst, vec3<T>& a )  { Mul( dst, *dst, a ); }
Templ Inl void Mul( vec2<T>* dst, vec2<T>& a )  { Mul( dst, *dst, a ); }

Templ Inl void Div( vec4<T>* dst, vec4<T>& a )  { Div( dst, *dst, a ); }
Templ Inl void Div( vec3<T>* dst, vec3<T>& a )  { Div( dst, *dst, a ); }
Templ Inl void Div( vec2<T>* dst, vec2<T>& a )  { Div( dst, *dst, a ); }


Templ Inl void Add( vec4<T>* dst, T a )  { Add( dst, *dst, a ); }
Templ Inl void Add( vec3<T>* dst, T a )  { Add( dst, *dst, a ); }
Templ Inl void Add( vec2<T>* dst, T a )  { Add( dst, *dst, a ); }

Templ Inl void Sub( vec4<T>* dst, T a )  { Sub( dst, *dst, a ); }
Templ Inl void Sub( vec3<T>* dst, T a )  { Sub( dst, *dst, a ); }
Templ Inl void Sub( vec2<T>* dst, T a )  { Sub( dst, *dst, a ); }

Templ Inl void Mul( vec4<T>* dst, T a )  { Mul( dst, *dst, a ); }
Templ Inl void Mul( vec3<T>* dst, T a )  { Mul( dst, *dst, a ); }
Templ Inl void Mul( vec2<T>* dst, T a )  { Mul( dst, *dst, a ); }

Templ Inl void Div( vec4<T>* dst, T a )  { Div( dst, *dst, a ); }
Templ Inl void Div( vec3<T>* dst, T a )  { Div( dst, *dst, a ); }
Templ Inl void Div( vec2<T>* dst, T a )  { Div( dst, *dst, a ); }


Templ Inl void Floor( vec4<T>* dst, vec4<T>& a )  { dst->x = Floor( a.x );  dst->y = Floor( a.y );  dst->z = Floor( a.z );  dst->w = Floor( a.w ); }
Templ Inl void Floor( vec3<T>* dst, vec3<T>& a )  { dst->x = Floor( a.x );  dst->y = Floor( a.y );  dst->z = Floor( a.z ); }
Templ Inl void Floor( vec2<T>* dst, vec2<T>& a )  { dst->x = Floor( a.x );  dst->y = Floor( a.y ); }

Templ Inl void Ceil( vec4<T>* dst, vec4<T>& a )  { dst->x = Ceil( a.x );  dst->y = Ceil( a.y );  dst->z = Ceil( a.z );  dst->w = Ceil( a.w ); }
Templ Inl void Ceil( vec3<T>* dst, vec3<T>& a )  { dst->x = Ceil( a.x );  dst->y = Ceil( a.y );  dst->z = Ceil( a.z ); }
Templ Inl void Ceil( vec2<T>* dst, vec2<T>& a )  { dst->x = Ceil( a.x );  dst->y = Ceil( a.y ); }

Templ Inl void Floor( vec4<T>* dst )  { Floor( dst, *dst ); }
Templ Inl void Floor( vec3<T>* dst )  { Floor( dst, *dst ); }
Templ Inl void Floor( vec2<T>* dst )  { Floor( dst, *dst ); }

Templ Inl void Ceil( vec4<T>* dst )  { Ceil( dst, *dst ); }
Templ Inl void Ceil( vec3<T>* dst )  { Ceil( dst, *dst ); }
Templ Inl void Ceil( vec2<T>* dst )  { Ceil( dst, *dst ); }


Templ Inl T Dot( vec4<T> a, vec4<T> b )  { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
Templ Inl T Dot( vec3<T> a, vec3<T> b )  { return a.x*b.x + a.y*b.y + a.z*b.z; }
Templ Inl T Dot( vec2<T> a, vec2<T> b )  { return a.x*b.x + a.y*b.y; }

Templ Inl T ProjFrac( vec4<T>& v, vec4<T>& axis )  { return Dot( v, axis ) * RecLength( axis ); }
Templ Inl T ProjFrac( vec3<T>& v, vec3<T>& axis )  { return Dot( v, axis ) * RecLength( axis ); }
Templ Inl T ProjFrac( vec2<T>& v, vec2<T>& axis )  { return Dot( v, axis ) * RecLength( axis ); }

Templ Inl T MaxElem( vec4<T> v ) { return MAX4( v.x, v.y, v.z, v.w ); }
Templ Inl T MaxElem( vec3<T> v ) { return MAX3( v.x, v.y, v.z ); }
Templ Inl T MaxElem( vec2<T> v ) { return MAX( v.x, v.y ); }

Templ Inl T MinElem( vec4<T> v ) { return MIN4( v.x, v.y, v.z, v.w ); }
Templ Inl T MinElem( vec3<T> v ) { return MIN3( v.x, v.y, v.z ); }
Templ Inl T MinElem( vec2<T> v ) { return MIN( v.x, v.y ); }

Templ Inl T CrossX( vec3<T>& a, vec3<T>& b )  { return a.y*b.z - a.z*b.y; }
Templ Inl T CrossY( vec3<T>& a, vec3<T>& b )  { return a.z*b.x - a.x*b.z; }
Templ Inl T CrossZ( vec3<T>& a, vec3<T>& b )  { return a.x*b.y - a.y*b.x; }
Templ Inl T CrossZ( vec2<T>& a, vec2<T>& b )  { return a.x*b.y - a.y*b.x; }

Templ Inl void
Cross( vec3<T>* dst, vec3<T>& a, vec3<T>& b )
{
  T cx = CrossX( a, b );
  T cy = CrossY( a, b );
  T cz = CrossZ( a, b );
  dst->x = cx;
  dst->y = cy;
  dst->z = cz;
}

Templ Inl void Cross( vec3<T>* dst, vec3<T>& a ) { Cross( dst, *dst, a ); }



Templ Inl void
Perp( vec2<T>* dst, vec2<T>& a )
{
  dst->x = -a.y;
  dst->y = a.x;
}



Templ Inl void Abs( vec4<T>* dst, vec4<T>& a )  { dst->x = ABS( a.x );  dst->y = ABS( a.y );  dst->z = ABS( a.z );  dst->w = ABS( a.w ); }
Templ Inl void Abs( vec3<T>* dst, vec3<T>& a )  { dst->x = ABS( a.x );  dst->y = ABS( a.y );  dst->z = ABS( a.z ); }
Templ Inl void Abs( vec2<T>* dst, vec2<T>& a )  { dst->x = ABS( a.x );  dst->y = ABS( a.y ); }

Templ Inl void Abs( vec4<T>* dst )  { Abs( dst, *dst ); }
Templ Inl void Abs( vec3<T>* dst )  { Abs( dst, *dst ); }
Templ Inl void Abs( vec2<T>* dst )  { Abs( dst, *dst ); }

Templ Inl T Squared( vec4<T> v )  { return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; }
Templ Inl T Squared( vec3<T> v )  { return v.x*v.x + v.y*v.y + v.z*v.z; }
Templ Inl T Squared( vec2<T> v )  { return v.x*v.x + v.y*v.y; }

Templ Inl T Length( vec4<T> v )  { return Sqrt32( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w ); }
Templ Inl T Length( vec3<T> v )  { return Sqrt32( v.x*v.x + v.y*v.y + v.z*v.z ); }
Templ Inl T Length( vec2<T> v )  { return Sqrt32( v.x*v.x + v.y*v.y ); }

Templ Inl T RecLength( vec4<T> v )  { return rsqrt( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w ); }
Templ Inl T RecLength( vec3<T> v )  { return rsqrt( v.x*v.x + v.y*v.y + v.z*v.z ); }
Templ Inl T RecLength( vec2<T> v )  { return rsqrt( v.x*v.x + v.y*v.y ); }

Templ Inl T ComponentSum( vec4<T> v )  { return v.x + v.y + v.z + v.w; }
Templ Inl T ComponentSum( vec3<T> v )  { return v.x + v.y + v.z; }
Templ Inl T ComponentSum( vec2<T> v )  { return v.x + v.y; }

Templ Inl void Normalize( vec4<T>* dst, vec4<T>& a )  { Mul( dst, a, RecLength( a ) ); }
Templ Inl void Normalize( vec3<T>* dst, vec3<T>& a )  { Mul( dst, a, RecLength( a ) ); }
Templ Inl void Normalize( vec2<T>* dst, vec2<T>& a )  { Mul( dst, a, RecLength( a ) ); }

Templ Inl void Normalize( vec4<T>* dst )  { Normalize( dst, *dst ); }
Templ Inl void Normalize( vec3<T>* dst )  { Normalize( dst, *dst ); }
Templ Inl void Normalize( vec2<T>* dst )  { Normalize( dst, *dst ); }

Templ Inl void AddMul( vec4<T>* dst, vec4<T>& v, vec4<T>& s, vec4<T>& t )  { dst->x = v.x + s.x*t.x;  dst->y = v.y + s.y*t.y;  dst->z = v.z + s.z*t.z;  dst->w = v.w + s.w*t.w; }
Templ Inl void AddMul( vec3<T>* dst, vec3<T>& v, vec3<T>& s, vec3<T>& t )  { dst->x = v.x + s.x*t.x;  dst->y = v.y + s.y*t.y;  dst->z = v.z + s.z*t.z; }
Templ Inl void AddMul( vec2<T>* dst, vec2<T>& v, vec2<T>& s, vec2<T>& t )  { dst->x = v.x + s.x*t.x;  dst->y = v.y + s.y*t.y; }

Templ Inl void AddMul( vec4<T>* dst, vec4<T>& v, T s, vec4<T>& t )  { dst->x = v.x + s*t.x;  dst->y = v.y + s*t.y;  dst->z = v.z + s*t.z;  dst->w = v.w + s*t.w; }
Templ Inl void AddMul( vec3<T>* dst, vec3<T>& v, T s, vec3<T>& t )  { dst->x = v.x + s*t.x;  dst->y = v.y + s*t.y;  dst->z = v.z + s*t.z; }
Templ Inl void AddMul( vec2<T>* dst, vec2<T>& v, T s, vec2<T>& t )  { dst->x = v.x + s*t.x;  dst->y = v.y + s*t.y; }

Templ Inl void AddMul( vec4<T>* dst, vec4<T>& v, vec4<T>& s, T t )  { dst->x = v.x + s.x*t;  dst->y = v.y + s.y*t;  dst->z = v.z + s.z*t;  dst->w = v.w + s.w*t; }
Templ Inl void AddMul( vec3<T>* dst, vec3<T>& v, vec3<T>& s, T t )  { dst->x = v.x + s.x*t;  dst->y = v.y + s.y*t;  dst->z = v.z + s.z*t; }
Templ Inl void AddMul( vec2<T>* dst, vec2<T>& v, vec2<T>& s, T t )  { dst->x = v.x + s.x*t;  dst->y = v.y + s.y*t; }


Templ Inl void AddMul( vec4<T>* dst, vec4<T>& s, vec4<T>& t )  { AddMul( dst, *dst, s, t ); }
Templ Inl void AddMul( vec3<T>* dst, vec3<T>& s, vec3<T>& t )  { AddMul( dst, *dst, s, t ); }
Templ Inl void AddMul( vec2<T>* dst, vec2<T>& s, vec2<T>& t )  { AddMul( dst, *dst, s, t ); }

Templ Inl void AddMul( vec4<T>* dst, T s, vec4<T>& t )  { AddMul( dst, *dst, s, t ); }
Templ Inl void AddMul( vec3<T>* dst, T s, vec3<T>& t )  { AddMul( dst, *dst, s, t ); }
Templ Inl void AddMul( vec2<T>* dst, T s, vec2<T>& t )  { AddMul( dst, *dst, s, t ); }

Templ Inl void AddMul( vec4<T>* dst, vec4<T>& s, T t )  { AddMul( dst, *dst, s, t ); }
Templ Inl void AddMul( vec3<T>* dst, vec3<T>& s, T t )  { AddMul( dst, *dst, s, t ); }
Templ Inl void AddMul( vec2<T>* dst, vec2<T>& s, T t )  { AddMul( dst, *dst, s, t ); }



Templ Inl vec4<T> _vec4( T a, T b, T c, T d ) { vec4<T> r = { a, b, c, d };  return r; }
Templ Inl vec4<T> _vec4( T a ) { return _vec4( a, a, a, a ); }
Templ Inl vec4<T> _vec4() { return _vec4( Cast( T, 0 ) ); }

Templ Inl vec3<T> _vec3( T a, T b, T c ) { vec3<T> r = { a, b, c };  return r; }
Templ Inl vec3<T> _vec3( T a ) { return _vec3( a, a, a ); }
Templ Inl vec3<T> _vec3() { return _vec3( Cast( T, 0 ) ); }

Templ Inl vec2<T> _vec2( T a, T b ) { vec2<T> r = { a, b };  return r; }
Templ Inl vec2<T> _vec2( T a ) { return _vec2( a, a ); }
Templ Inl vec2<T> _vec2() { return _vec2( Cast( T, 0 ) ); }



Templ Inl vec4<T>
QuatMul( vec4<T>& a, vec4<T>& b )
{
  return _vec4(
    a.w * b.x + a.x * b.w - a.y * b.z + a.z * b.y,
    a.w * b.y + a.x * b.z + a.y * b.w - a.z * b.x,
    a.w * b.z - a.x * b.y + a.y * b.x + a.z * b.w,
    a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}



Templ Inl vec4<T> operator+( vec4<T> a, vec4<T> b ) { return _vec4<T>( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w ); }
Templ Inl vec3<T> operator+( vec3<T> a, vec3<T> b ) { return _vec3<T>( a.x + b.x, a.y + b.y, a.z + b.z ); }
Templ Inl vec2<T> operator+( vec2<T> a, vec2<T> b ) { return _vec2<T>( a.x + b.x, a.y + b.y ); }
Templ Inl vec4<T> operator-( vec4<T> a, vec4<T> b ) { return _vec4<T>( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }
Templ Inl vec3<T> operator-( vec3<T> a, vec3<T> b ) { return _vec3<T>( a.x - b.x, a.y - b.y, a.z - b.z ); }
Templ Inl vec2<T> operator-( vec2<T> a, vec2<T> b ) { return _vec2<T>( a.x - b.x, a.y - b.y ); }
Templ Inl vec4<T> operator*( vec4<T> a, vec4<T> b ) { return _vec4<T>( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w ); }
Templ Inl vec3<T> operator*( vec3<T> a, vec3<T> b ) { return _vec3<T>( a.x * b.x, a.y * b.y, a.z * b.z ); }
Templ Inl vec2<T> operator*( vec2<T> a, vec2<T> b ) { return _vec2<T>( a.x * b.x, a.y * b.y ); }
Templ Inl vec4<T> operator/( vec4<T> a, vec4<T> b ) { return _vec4<T>( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w ); }
Templ Inl vec3<T> operator/( vec3<T> a, vec3<T> b ) { return _vec3<T>( a.x / b.x, a.y / b.y, a.z / b.z ); }
Templ Inl vec2<T> operator/( vec2<T> a, vec2<T> b ) { return _vec2<T>( a.x / b.x, a.y / b.y ); }


// NOTE: very easy to misuse these guys; accidentally add something to all dimensions.
#if 0
Templ Inl vec4<T> operator+( vec4<T> a, T b ) { return _vec4<T>( a.x + b, a.y + b, a.z + b, a.w + b ); }
Templ Inl vec3<T> operator+( vec3<T> a, T b ) { return _vec3<T>( a.x + b, a.y + b, a.z + b ); }
Templ Inl vec2<T> operator+( vec2<T> a, T b ) { return _vec2<T>( a.x + b, a.y + b ); }
Templ Inl vec4<T> operator-( vec4<T> a, T b ) { return _vec4<T>( a.x - b, a.y - b, a.z - b, a.w - b ); }
Templ Inl vec3<T> operator-( vec3<T> a, T b ) { return _vec3<T>( a.x - b, a.y - b, a.z - b ); }
Templ Inl vec2<T> operator-( vec2<T> a, T b ) { return _vec2<T>( a.x - b, a.y - b ); }
Templ Inl vec4<T> operator+( T a, vec4<T> b ) { return _vec4<T>( a + b.x, a + b.y, a + b.z, a + b.w ); }
Templ Inl vec3<T> operator+( T a, vec3<T> b ) { return _vec3<T>( a + b.x, a + b.y, a + b.z ); }
Templ Inl vec2<T> operator+( T a, vec2<T> b ) { return _vec2<T>( a + b.x, a + b.y ); }
Templ Inl vec4<T> operator-( T a, vec4<T> b ) { return _vec4<T>( a - b.x, a - b.y, a - b.z, a - b.w ); }
Templ Inl vec3<T> operator-( T a, vec3<T> b ) { return _vec3<T>( a - b.x, a - b.y, a - b.z ); }
Templ Inl vec2<T> operator-( T a, vec2<T> b ) { return _vec2<T>( a - b.x, a - b.y ); }
Templ Inl vec4<T>& operator+=( vec4<T>& a, T b ) { a = a + b;  return a; }
Templ Inl vec3<T>& operator+=( vec3<T>& a, T b ) { a = a + b;  return a; }
Templ Inl vec2<T>& operator+=( vec2<T>& a, T b ) { a = a + b;  return a; }
Templ Inl vec4<T>& operator-=( vec4<T>& a, T b ) { a = a - b;  return a; }
Templ Inl vec3<T>& operator-=( vec3<T>& a, T b ) { a = a - b;  return a; }
Templ Inl vec2<T>& operator-=( vec2<T>& a, T b ) { a = a - b;  return a; }
#endif


Templ Inl vec4<T> operator*( vec4<T> a, T b ) { return _vec4<T>( a.x * b, a.y * b, a.z * b, a.w * b ); }
Templ Inl vec3<T> operator*( vec3<T> a, T b ) { return _vec3<T>( a.x * b, a.y * b, a.z * b ); }
Templ Inl vec2<T> operator*( vec2<T> a, T b ) { return _vec2<T>( a.x * b, a.y * b ); }
Templ Inl vec4<T> operator/( vec4<T> a, T b ) { return _vec4<T>( a.x / b, a.y / b, a.z / b, a.w / b ); }
Templ Inl vec3<T> operator/( vec3<T> a, T b ) { return _vec3<T>( a.x / b, a.y / b, a.z / b ); }
Templ Inl vec2<T> operator/( vec2<T> a, T b ) { return _vec2<T>( a.x / b, a.y / b ); }
Templ Inl vec4<T> operator*( T a, vec4<T> b ) { return _vec4<T>( a * b.x, a * b.y, a * b.z, a * b.w ); }
Templ Inl vec3<T> operator*( T a, vec3<T> b ) { return _vec3<T>( a * b.x, a * b.y, a * b.z ); }
Templ Inl vec2<T> operator*( T a, vec2<T> b ) { return _vec2<T>( a * b.x, a * b.y ); }
Templ Inl vec4<T> operator/( T a, vec4<T> b ) { return _vec4<T>( a / b.x, a / b.y, a / b.z, a / b.w ); }
Templ Inl vec3<T> operator/( T a, vec3<T> b ) { return _vec3<T>( a / b.x, a / b.y, a / b.z ); }
Templ Inl vec2<T> operator/( T a, vec2<T> b ) { return _vec2<T>( a / b.x, a / b.y ); }
Templ Inl vec4<T>& operator*=( vec4<T>& a, T b ) { a = a * b;  return a; }
Templ Inl vec3<T>& operator*=( vec3<T>& a, T b ) { a = a * b;  return a; }
Templ Inl vec2<T>& operator*=( vec2<T>& a, T b ) { a = a * b;  return a; }
Templ Inl vec4<T>& operator/=( vec4<T>& a, T b ) { a = a / b;  return a; }
Templ Inl vec3<T>& operator/=( vec3<T>& a, T b ) { a = a / b;  return a; }
Templ Inl vec2<T>& operator/=( vec2<T>& a, T b ) { a = a / b;  return a; }


Templ Inl vec4<T>& operator+=( vec4<T>& a, vec4<T> b ) { a = a + b;  return a; }
Templ Inl vec3<T>& operator+=( vec3<T>& a, vec3<T> b ) { a = a + b;  return a; }
Templ Inl vec2<T>& operator+=( vec2<T>& a, vec2<T> b ) { a = a + b;  return a; }
Templ Inl vec4<T>& operator-=( vec4<T>& a, vec4<T> b ) { a = a - b;  return a; }
Templ Inl vec3<T>& operator-=( vec3<T>& a, vec3<T> b ) { a = a - b;  return a; }
Templ Inl vec2<T>& operator-=( vec2<T>& a, vec2<T> b ) { a = a - b;  return a; }
Templ Inl vec4<T>& operator*=( vec4<T>& a, vec4<T> b ) { a = a * b;  return a; }
Templ Inl vec3<T>& operator*=( vec3<T>& a, vec3<T> b ) { a = a * b;  return a; }
Templ Inl vec2<T>& operator*=( vec2<T>& a, vec2<T> b ) { a = a * b;  return a; }
Templ Inl vec4<T>& operator/=( vec4<T>& a, vec4<T> b ) { a = a / b;  return a; }
Templ Inl vec3<T>& operator/=( vec3<T>& a, vec3<T> b ) { a = a / b;  return a; }
Templ Inl vec2<T>& operator/=( vec2<T>& a, vec2<T> b ) { a = a / b;  return a; }


Templ Inl bool operator==( vec4<T>& a, vec4<T> b ) { return ( a.x == b.x )  &  ( a.y == b.y )  &  ( a.z == b.z )  &  ( a.w == b.w ); }
Templ Inl bool operator==( vec3<T>& a, vec3<T> b ) { return ( a.x == b.x )  &  ( a.y == b.y )  &  ( a.z == b.z ); }
Templ Inl bool operator==( vec2<T>& a, vec2<T> b ) { return ( a.x == b.x )  &  ( a.y == b.y ); }


Templ Inl vec4<T> operator-( vec4<T> a ) { return _vec4( -a.x, -a.y, -a.z, -a.w ); }
Templ Inl vec3<T> operator-( vec3<T> a ) { return _vec3( -a.x, -a.y, -a.z ); }
Templ Inl vec2<T> operator-( vec2<T> a ) { return _vec2( -a.x, -a.y ); }



Templ Inl vec4<T> Floor( vec4<T> a ) { return _vec4( Floor32( a.x ), Floor32( a.y ), Floor32( a.z ), Floor32( a.w ) ); }
Templ Inl vec3<T> Floor( vec3<T> a ) { return _vec3( Floor32( a.x ), Floor32( a.y ), Floor32( a.z ) ); }
Templ Inl vec2<T> Floor( vec2<T> a ) { return _vec2( Floor32( a.x ), Floor32( a.y ) ); }

Templ Inl vec4<T> Ceil( vec4<T> a ) { return _vec4( Ceil32( a.x ), Ceil32( a.y ), Ceil32( a.z ), Ceil32( a.w ) ); }
Templ Inl vec3<T> Ceil( vec3<T> a ) { return _vec3( Ceil32( a.x ), Ceil32( a.y ), Ceil32( a.z ) ); }
Templ Inl vec2<T> Ceil( vec2<T> a ) { return _vec2( Ceil32( a.x ), Ceil32( a.y ) ); }

Templ Inl vec4<T> Abs( vec4<T> a ) { return _vec4( ABS( a.x ), ABS( a.y ), ABS( a.z ), ABS( a.w ) ); }
Templ Inl vec3<T> Abs( vec3<T> a ) { return _vec3( ABS( a.x ), ABS( a.y ), ABS( a.z ) ); }
Templ Inl vec2<T> Abs( vec2<T> a ) { return _vec2( ABS( a.x ), ABS( a.y ) ); }

Templ Inl vec4<T> Max( vec4<T> a, vec4<T> b ) { return _vec4( MAX( a.x, b.x ), MAX( a.y, b.y ), MAX( a.z, b.z ), MAX( a.w, b.w ) ); }
Templ Inl vec3<T> Max( vec3<T> a, vec3<T> b ) { return _vec3( MAX( a.x, b.x ), MAX( a.y, b.y ), MAX( a.z, b.z ) ); }
Templ Inl vec2<T> Max( vec2<T> a, vec2<T> b ) { return _vec2( MAX( a.x, b.x ), MAX( a.y, b.y ) ); }

Templ Inl vec4<T> Min( vec4<T> a, vec4<T> b ) { return _vec4( MIN( a.x, b.x ), MIN( a.y, b.y ), MIN( a.z, b.z ), MIN( a.w, b.w ) ); }
Templ Inl vec3<T> Min( vec3<T> a, vec3<T> b ) { return _vec3( MIN( a.x, b.x ), MIN( a.y, b.y ), MIN( a.z, b.z ) ); }
Templ Inl vec2<T> Min( vec2<T> a, vec2<T> b ) { return _vec2( MIN( a.x, b.x ), MIN( a.y, b.y ) ); }


Templ Inl vec4<T> Clamp( vec4<T> a, T s, T t ) { return _vec4( CLAMP( a.x, s, t ), CLAMP( a.y, s, t ), CLAMP( a.z, s, t ), CLAMP( a.w, s, t ) ); }
Templ Inl vec3<T> Clamp( vec3<T> a, T s, T t ) { return _vec3( CLAMP( a.x, s, t ), CLAMP( a.y, s, t ), CLAMP( a.z, s, t ) ); }
Templ Inl vec2<T> Clamp( vec2<T> a, T s, T t ) { return _vec2( CLAMP( a.x, s, t ), CLAMP( a.y, s, t ) ); }

Templ Inl vec4<T> Clamp( vec4<T> a, vec4<T> s, vec4<T> t ) { return _vec4( CLAMP( a.x, s.x, t.x ), CLAMP( a.y, s.y, t.y ), CLAMP( a.z, s.z, t.z ), CLAMP( a.w, s.w, t.w ) ); }
Templ Inl vec3<T> Clamp( vec3<T> a, vec3<T> s, vec3<T> t ) { return _vec3( CLAMP( a.x, s.x, t.x ), CLAMP( a.y, s.y, t.y ), CLAMP( a.z, s.z, t.z ) ); }
Templ Inl vec2<T> Clamp( vec2<T> a, vec2<T> s, vec2<T> t ) { return _vec2( CLAMP( a.x, s.x, t.x ), CLAMP( a.y, s.y, t.y ) ); }

Templ Inl vec4<T> Normalize( vec4<T> a ) { return a * RecLength( a ); }
Templ Inl vec3<T> Normalize( vec3<T> a ) { return a * RecLength( a ); }
Templ Inl vec2<T> Normalize( vec2<T> a ) { return a * RecLength( a ); }




Templ Inl vec3<T>
Cross( vec3<T> a, vec3<T> b )
{
  return _vec3( CrossX( a, b ), CrossY( a, b ), CrossZ( a, b ) );
}


Templ Inl vec2<T>
Perp( vec2<T> a )
{
  return _vec2( -a.y, a.x );
}


// REFERENCE:
//   http://orbit.dtu.dk/files/57573287/onb_frisvad_jgt2012.pdf
//
Templ Inl void
OrthonormalBasisGivenNorm(
  vec3<T>& t,
  vec3<T>& b,
  vec3<T> n, // assumed unit length.
  T epsilon
  )
{
  if( n.z < -1 + epsilon ) {
    t = _vec3<T>( 0, -1, 0 );
    b = _vec3<T>( -1, 0, 0 );
  } else {
    T c = 1 / ( 1 + n.z );
    T d = -n.x * n.y * c;
    t = _vec3<T>( 1 - n.x * n.x * c, d, -n.x );
    b = _vec3<T>( d, 1 - n.y * n.y * c, -n.y );
  }
}



Inl vec4<f64>
Lerp_from_vec4f64(
  vec4<f64>& y0,
  vec4<f64>& y1,
  vec4<f64>& x,
  vec4<f64>& x0,
  vec4<f64>& x1 )
{
  return _vec4(
    Lerp_from_f64( y0.x, y1.x, x.x, x0.x, x1.x ),
    Lerp_from_f64( y0.y, y1.y, x.y, x0.y, x1.y ),
    Lerp_from_f64( y0.z, y1.z, x.z, x0.z, x1.z ),
    Lerp_from_f64( y0.w, y1.w, x.w, x0.w, x1.w )
  );
}

Inl vec4<f32>
Lerp_from_vec4f32(
  vec4<f32>& y0,
  vec4<f32>& y1,
  vec4<f32>& x,
  vec4<f32>& x0,
  vec4<f32>& x1 )
{
  return _vec4(
    Lerp_from_f32( y0.x, y1.x, x.x, x0.x, x1.x ),
    Lerp_from_f32( y0.y, y1.y, x.y, x0.y, x1.y ),
    Lerp_from_f32( y0.z, y1.z, x.z, x0.z, x1.z ),
    Lerp_from_f32( y0.w, y1.w, x.w, x0.w, x1.w )
  );
}



Inl vec4<f64>
Lerp_from_s32(
  vec4<f64>& y0,
  vec4<f64>& y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return _vec4(
    Lerp_from_s32( y0.x, y1.x, x, x0, x1 ),
    Lerp_from_s32( y0.y, y1.y, x, x0, x1 ),
    Lerp_from_s32( y0.z, y1.z, x, x0, x1 ),
    Lerp_from_s32( y0.w, y1.w, x, x0, x1 )
  );
}

Inl vec4<f32>
Lerp_from_s32(
  vec4<f32>& y0,
  vec4<f32>& y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return _vec4(
    Lerp_from_s32( y0.x, y1.x, x, x0, x1 ),
    Lerp_from_s32( y0.y, y1.y, x, x0, x1 ),
    Lerp_from_s32( y0.z, y1.z, x, x0, x1 ),
    Lerp_from_s32( y0.w, y1.w, x, x0, x1 )
  );
}



Inl vec2<f64>
Lerp_from_vec2f64(
  vec2<f64>& y0,
  vec2<f64>& y1,
  vec2<f64>& x,
  vec2<f64>& x0,
  vec2<f64>& x1 )
{
  return _vec2(
    Lerp_from_f64( y0.x, y1.x, x.x, x0.x, x1.x ),
    Lerp_from_f64( y0.y, y1.y, x.y, x0.y, x1.y )
  );
}

Inl vec2<f32>
Lerp_from_vec2f32(
  vec2<f32>& y0,
  vec2<f32>& y1,
  vec2<f32>& x,
  vec2<f32>& x0,
  vec2<f32>& x1 )
{
  return _vec2(
    Lerp_from_f32( y0.x, y1.x, x.x, x0.x, x1.x ),
    Lerp_from_f32( y0.y, y1.y, x.y, x0.y, x1.y )
  );
}



Inl vec2<f64>
Lerp_from_s32(
  vec2<f64>& y0,
  vec2<f64>& y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return _vec2(
    Lerp_from_s32( y0.x, y1.x, x, x0, x1 ),
    Lerp_from_s32( y0.y, y1.y, x, x0, x1 )
  );
}

Inl vec2<f32>
Lerp_from_s32(
  vec2<f32>& y0,
  vec2<f32>& y1,
  s32 x,
  s32 x0,
  s32 x1 )
{
  return _vec2(
    Lerp_from_s32( y0.x, y1.x, x, x0, x1 ),
    Lerp_from_s32( y0.y, y1.y, x, x0, x1 )
  );
}



Inl vec2<f64>
Lerp_from_f64(
  vec2<f64>& y0,
  vec2<f64>& y1,
  f64 x,
  f64 x0,
  f64 x1 )
{
  return _vec2(
    Lerp_from_f64( y0.x, y1.x, x, x0, x1 ),
    Lerp_from_f64( y0.y, y1.y, x, x0, x1 )
  );
}

Inl vec2<f32>
Lerp_from_f32(
  vec2<f32>& y0,
  vec2<f32>& y1,
  f32 x,
  f32 x0,
  f32 x1 )
{
  return _vec2(
    Lerp_from_f32( y0.x, y1.x, x, x0, x1 ),
    Lerp_from_f32( y0.y, y1.y, x, x0, x1 )
  );
}



Inl bool
PtInBox( vec2<f32>& p, vec2<f32>& p00, vec2<f32>& p11, f32 epsilon )
{
  bool r =
    PtInInterval( p.x, p00.x, p11.x, epsilon )  &&
    PtInInterval( p.y, p00.y, p11.y, epsilon );
  return r;
}

Inl f32
DistanceToBox( vec2<f32>& p, vec2<f32>& p00, vec2<f32>& p11 )
{
  f32 r = 0.0f;
  if( p.x <= p00.x  &&  p.y <= p00.y ) {
    r = Length( p - p00 );
  } elif( p.x <= p00.x  &&  p11.y <= p.y ) {
    r = Length( p - _vec2( p00.x, p11.y ) );
  } elif( p11.x <= p.x  &&  p.y <= p00.y ) {
    r = Length( p - _vec2( p11.x, p00.y ) );
  } elif( p11.x <= p.x  &&  p11.y <= p.y ) {
    r = Length( p - p11 );
  } elif( p.x <= p00.x ) {
    r = p00.x - p.x;
  } elif( p11.x <= p.x ) {
    r = p.x - p11.x;
  } elif( p.y <= p00.y ) {
    r = p00.y - p.y;
  } elif( p11.y <= p.y ) {
    r = p.y - p11.y;
  } else {
    // inside the box.
  }
  return r;
}


Inl bool
PtInTriangle(
  vec2<f32>& p,
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec2<f32>& p2,
  vec2<f32>& e0,
  vec2<f32>& e1,
  vec2<f32>& e2,
  f32 winding // either -1 or 1.
  )
{
  auto c0 = p - p0;
  auto c1 = p - p1;
  auto c2 = p - p2;
  auto f0 = winding * CrossZ( e0, c0 );
  auto f1 = winding * CrossZ( e1, c1 );
  auto f2 = winding * CrossZ( e2, c2 );
  bool r = ( f0 >= 0 )  &&  ( f1 >= 0 )  &&  ( f2 >= 0 );
  return r;
}




Templ
struct mat4x4r
{
  union
  {
    struct
    {
      vec4<T> row0;
      vec4<T> row1;
      vec4<T> row2;
      vec4<T> row3;
    };
    vec4<T> rows[4];
  };
};

Templ
struct mat4x4c
{
  union
  {
    vec4<T> cols[4];
    struct
    {
      vec4<T> col0;
      vec4<T> col1;
      vec4<T> col2;
      vec4<T> col3;
    };
  };
};


Templ
struct mat3x3r
{
  union
  {
    vec3<T> rows[3];
    struct
    {
      vec3<T> row0;
      vec3<T> row1;
      vec3<T> row2;
    };
  };
};

Templ
struct mat3x3c
{
  union
  {
    vec3<T> cols[3];
    struct
    {
      vec3<T> col0;
      vec3<T> col1;
      vec3<T> col2;
    };
  };
};


Templ
struct mat2x2r
{
  vec2<T> row0;
  vec2<T> row1;
};

Templ
struct mat2x2c
{
  vec2<T> col0;
  vec2<T> col1;
};




#define MAT4x4R( dst,  \
  a, b, c, d,  \
  e, f, g, h,  \
  i, j, k, l,  \
  m, n, o, p )  \
  do { dst->row0.x = a;  dst->row0.y = b;  dst->row0.z = c;  dst->row0.w = d; \
       dst->row1.x = e;  dst->row1.y = f;  dst->row1.z = g;  dst->row1.w = h; \
       dst->row2.x = i;  dst->row2.y = j;  dst->row2.z = k;  dst->row2.w = l; \
       dst->row3.x = m;  dst->row3.y = n;  dst->row3.z = o;  dst->row3.w = p; } while( 0 )

#define MAT4x4C( dst,  \
  a, b, c, d,  \
  e, f, g, h,  \
  i, j, k, l,  \
  m, n, o, p )  \
  do { dst->col0.x = a;  dst->col0.y = e;  dst->col0.z = i;  dst->col0.w = m; \
       dst->col1.x = b;  dst->col1.y = f;  dst->col1.z = j;  dst->col1.w = n; \
       dst->col2.x = c;  dst->col2.y = g;  dst->col2.z = k;  dst->col2.w = o; \
       dst->col3.x = d;  dst->col3.y = h;  dst->col3.z = l;  dst->col3.w = p; } while( 0 )



#define MAT3x3R( dst,  \
  a, b, c,  \
  d, e, f,  \
  g, h, i )  \
  do { dst->row0.x = a;  dst->row0.y = b;  dst->row0.z = c; \
       dst->row1.x = d;  dst->row1.y = e;  dst->row1.z = f; \
       dst->row2.x = g;  dst->row2.y = h;  dst->row2.z = i; } while( 0 )

#define MAT3x3C( dst,  \
  a, b, c,  \
  d, e, f,  \
  g, h, i )  \
  do { dst->col0.x = a;  dst->col0.y = d;  dst->col0.z = g; \
       dst->col1.x = b;  dst->col1.y = e;  dst->col1.z = h; \
       dst->col2.x = c;  dst->col2.y = f;  dst->col2.z = i; } while( 0 )



#define MAT2x2R( dst,  \
  a, b,  \
  c, d )  \
  do { dst->row0.x = a;  dst->row0.y = b; \
       dst->row1.x = c;  dst->row1.y = d; } while( 0 )

#define MAT2x2C( dst,  \
  a, b,  \
  c, d )  \
  do { dst->col0.x = a;  dst->col0.y = c; \
       dst->col1.x = b;  dst->col1.y = d; } while( 0 )





Templ Inl void
Copy( mat4x4r<T>* dst, mat4x4r<T>& src )
{
  Copy( &dst->row0, src.row0 );
  Copy( &dst->row1, src.row1 );
  Copy( &dst->row2, src.row2 );
  Copy( &dst->row3, src.row3 );
}
Templ Inl void
Copy( mat4x4c<T>* dst, mat4x4c<T>& src )
{
  Copy( &dst->col0, src.col0 );
  Copy( &dst->col1, src.col1 );
  Copy( &dst->col2, src.col2 );
  Copy( &dst->col3, src.col3 );
}
Templ Inl void
Copy( mat3x3r<T>* dst, mat3x3r<T>& src )
{
  Copy( &dst->row0, src.row0 );
  Copy( &dst->row1, src.row1 );
  Copy( &dst->row2, src.row2 );
}
Templ Inl void
Copy( mat3x3c<T>* dst, mat3x3c<T>& src )
{
  Copy( &dst->col0, src.col0 );
  Copy( &dst->col1, src.col1 );
  Copy( &dst->col2, src.col2 );
}
Templ Inl void
Copy( mat2x2r<T>* dst, mat2x2r<T>& src )
{
  Copy( &dst->row0, src.row0 );
  Copy( &dst->row1, src.row1 );
}
Templ Inl void
Copy( mat2x2c<T>* dst, mat2x2c<T>& src )
{
  Copy( &dst->col0, src.col0 );
  Copy( &dst->col1, src.col1 );
}



Templ Inl void
Copyr( mat4x4r<T>* dst, T* src )
{
  MAT4x4R( dst,
    src[0],  src[1],  src[2],  src[3],
    src[4],  src[5],  src[6],  src[7],
    src[8],  src[9],  src[10], src[11],
    src[12], src[13], src[14], src[15]
    );
}
Templ Inl void
Copyr( mat4x4c<T>* dst, T* src )
{
  MAT4x4C( dst,
    src[0],  src[1],  src[2],  src[3],
    src[4],  src[5],  src[6],  src[7],
    src[8],  src[9],  src[10], src[11],
    src[12], src[13], src[14], src[15]
    );
}
Templ Inl void
Copyr( mat3x3r<T>* dst, T* src )
{
  MAT3x3R( dst,
    src[0],  src[1],  src[2],
    src[3],  src[4],  src[5],
    src[6],  src[7],  src[8]
    );
}
Templ Inl void
Copyr( mat3x3c<T>* dst, T* src )
{
  MAT3x3C( dst,
    src[0],  src[1],  src[2],
    src[3],  src[4],  src[5],
    src[6],  src[7],  src[8]
    );
}
Templ Inl void
Copyr( mat2x2r<T>* dst, T* src )
{
  MAT2x2R( dst,
    src[0],  src[1],
    src[2],  src[3]
    );
}
Templ Inl void
Copyr( mat2x2c<T>* dst, T* src )
{
  MAT2x2C( dst,
    src[0],  src[1],
    src[2],  src[3]
    );
}



Templ Inl void
Copyc( mat4x4r<T>* dst, T* src )
{
  MAT4x4R( dst,
    src[0], src[4], src[8],  src[12],
    src[1], src[5], src[9],  src[13],
    src[2], src[6], src[10], src[14],
    src[3], src[7], src[11], src[15]
    );
}
Templ Inl void
Copyc( mat4x4c<T>* dst, T* src )
{
  MAT4x4C( dst,
    src[0], src[4], src[8],  src[12],
    src[1], src[5], src[9],  src[13],
    src[2], src[6], src[10], src[14],
    src[3], src[7], src[11], src[15]
    );
}
Templ Inl void
Copyc( mat3x3r<T>* dst, T* src )
{
  MAT3x3R( dst,
    src[0], src[3], src[6],
    src[1], src[4], src[7],
    src[2], src[5], src[8]
    );
}
Templ Inl void
Copyc( mat3x3c<T>* dst, T* src )
{
  MAT3x3C( dst,
    src[0], src[3], src[6],
    src[1], src[4], src[7],
    src[2], src[5], src[8]
    );
}
Templ Inl void
Copyc( mat2x2r<T>* dst, T* src )
{
  MAT2x2R( dst,
    src[0], src[2],
    src[1], src[3]
    );
}
Templ Inl void
Copyc( mat2x2c<T>* dst, T* src )
{
  MAT2x2C( dst,
    src[0], src[2],
    src[1], src[3]
    );
}



Templ Inl void
Copyr( T* dst, mat4x4r<T>& src )
{
  dst[0]  = src.row0.x;  dst[1]  = src.row0.y;  dst[2]  = src.row0.z;  dst[3]  = src.row0.w;
  dst[4]  = src.row1.x;  dst[5]  = src.row1.y;  dst[6]  = src.row1.z;  dst[7]  = src.row1.w;
  dst[8]  = src.row2.x;  dst[9]  = src.row2.y;  dst[10] = src.row2.z;  dst[11] = src.row2.w;
  dst[12] = src.row3.x;  dst[13] = src.row3.y;  dst[14] = src.row3.z;  dst[15] = src.row3.w;
}
Templ Inl void
Copyr( T* dst, mat4x4c<T>& src )
{
  dst[0]  = src.col0.x;  dst[1]  = src.col1.x;  dst[2]  = src.col2.x;  dst[3]  = src.col3.x;
  dst[4]  = src.col0.y;  dst[5]  = src.col1.y;  dst[6]  = src.col2.y;  dst[7]  = src.col3.y;
  dst[8]  = src.col0.z;  dst[9]  = src.col1.z;  dst[10] = src.col2.z;  dst[11] = src.col3.z;
  dst[12] = src.col0.w;  dst[13] = src.col1.w;  dst[14] = src.col2.w;  dst[15] = src.col3.w;
}
Templ Inl void
Copyr( T* dst, mat3x3r<T>& src )
{
  dst[0] = src.row0.x;  dst[1] = src.row0.y;  dst[2] = src.row0.z;
  dst[3] = src.row1.x;  dst[4] = src.row1.y;  dst[5] = src.row1.z;
  dst[6] = src.row2.x;  dst[7] = src.row2.y;  dst[8] = src.row2.z;
}
Templ Inl void
Copyr( T* dst, mat3x3c<T>& src )
{
  dst[0] = src.col0.x;  dst[1] = src.col1.x;  dst[2] = src.col2.x;
  dst[3] = src.col0.y;  dst[4] = src.col1.y;  dst[5] = src.col2.y;
  dst[6] = src.col0.z;  dst[7] = src.col1.z;  dst[8] = src.col2.z;
}
Templ Inl void
Copyr( T* dst, mat2x2r<T>& src )
{
  dst[0] = src.row0.x;  dst[1] = src.row0.y;
  dst[2] = src.row1.x;  dst[3] = src.row1.y;
}
Templ Inl void
Copyr( T* dst, mat2x2c<T>& src )
{
  dst[0] = src.col0.x;  dst[1] = src.col1.x;
  dst[2] = src.col0.y;  dst[3] = src.col1.y;
}



Templ Inl void
Copyc( T* dst, mat4x4r<T>& src )
{
  dst[0] = src.row0.x;  dst[4] = src.row0.y;  dst[8]  = src.row0.z;  dst[12] = src.row0.w;
  dst[1] = src.row1.x;  dst[5] = src.row1.y;  dst[9]  = src.row1.z;  dst[13] = src.row1.w;
  dst[2] = src.row2.x;  dst[6] = src.row2.y;  dst[10] = src.row2.z;  dst[14] = src.row2.w;
  dst[3] = src.row3.x;  dst[7] = src.row3.y;  dst[11] = src.row3.z;  dst[15] = src.row3.w;
}
Templ Inl void
Copyc( T* dst, mat4x4c<T>& src )
{
  dst[0]  = src.col0.x;  dst[1]  = src.col0.y;  dst[2]  = src.col0.z;  dst[3]  = src.col0.w;
  dst[4]  = src.col1.x;  dst[5]  = src.col1.y;  dst[6]  = src.col1.z;  dst[7]  = src.col1.w;
  dst[8]  = src.col2.x;  dst[9]  = src.col2.y;  dst[10] = src.col2.z;  dst[11] = src.col2.w;
  dst[12] = src.col3.x;  dst[13] = src.col3.y;  dst[14] = src.col3.z;  dst[15] = src.col3.w;
}
Templ Inl void
Copyc( T* dst, mat3x3r<T>& src )
{
  dst[0] = src.row0.x;  dst[3] = src.row0.y;  dst[6] = src.row0.z;
  dst[1] = src.row1.x;  dst[4] = src.row1.y;  dst[7] = src.row1.z;
  dst[2] = src.row2.x;  dst[5] = src.row2.y;  dst[8] = src.row2.z;
}
Templ Inl void
Copyc( T* dst, mat3x3c<T>& src )
{
  dst[0] = src.col0.x;  dst[1] = src.col0.y;  dst[2] = src.col0.z;
  dst[3] = src.col1.x;  dst[4] = src.col1.y;  dst[5] = src.col1.z;
  dst[6] = src.col2.x;  dst[7] = src.col2.y;  dst[8] = src.col2.z;
}
Templ Inl void
Copyc( T* dst, mat2x2r<T>& src )
{
  dst[0] = src.row0.x;  dst[2] = src.row0.y;
  dst[1] = src.row1.x;  dst[3] = src.row1.y;
}
Templ Inl void
Copyc( T* dst, mat2x2c<T>& src )
{
  dst[0] = src.col0.x;  dst[1] = src.col0.y;
  dst[2] = src.col1.x;  dst[3] = src.col1.y;
}



Templ Inl void
Zero( mat4x4r<T>* dst, mat4x4r<T>& src )
{
  Zero( &dst->row0, src.row0 );
  Zero( &dst->row1, src.row1 );
  Zero( &dst->row2, src.row2 );
  Zero( &dst->row3, src.row3 );
}
Templ Inl void
Zero( mat4x4c<T>* dst, mat4x4c<T>& src )
{
  Zero( &dst->col0, src.col0 );
  Zero( &dst->col1, src.col1 );
  Zero( &dst->col2, src.col2 );
  Zero( &dst->col3, src.col3 );
}
Templ Inl void
Zero( mat3x3r<T>* dst, mat3x3r<T>& src )
{
  Zero( &dst->row0, src.row0 );
  Zero( &dst->row2, src.row2 );
  Zero( &dst->row1, src.row1 );
}
Templ Inl void
Zero( mat3x3c<T>* dst, mat3x3c<T>& src )
{
  Zero( &dst->col0, src.col0 );
  Zero( &dst->col1, src.col1 );
  Zero( &dst->col2, src.col2 );
}
Templ Inl void
Zero( mat2x2r<T>* dst, mat2x2r<T>& src )
{
  Zero( &dst->row0, src.row0 );
  Zero( &dst->row1, src.row1 );
}
Templ Inl void
Zero( mat2x2c<T>* dst, mat2x2c<T>& src )
{
  Zero( &dst->col0, src.col0 );
  Zero( &dst->col1, src.col1 );
}



Templ Inl void
Add( mat4x4r<T>* dst, mat4x4r<T>& a, mat4x4r<T>& b )
{
  Add( &dst->row0, a.row0, b.row0 );
  Add( &dst->row1, a.row1, b.row1 );
  Add( &dst->row2, a.row2, b.row2 );
  Add( &dst->row3, a.row3, b.row3 );
}
Templ Inl void
Add( mat4x4c<T>* dst, mat4x4c<T>& a, mat4x4c<T>& b )
{
  Add( &dst->col0, a.col0, b.col0 );
  Add( &dst->col1, a.col1, b.col1 );
  Add( &dst->col2, a.col2, b.col2 );
  Add( &dst->col3, a.col3, b.col3 );
}
Templ Inl void
Add( mat3x3r<T>* dst, mat3x3r<T>& a, mat3x3r<T>& b )
{
  Add( &dst->row0, a.row0, b.row0 );
  Add( &dst->row1, a.row1, b.row1 );
  Add( &dst->row2, a.row2, b.row2 );
}
Templ Inl void
Add( mat3x3c<T>* dst, mat3x3c<T>& a, mat3x3c<T>& b )
{
  Add( &dst->col0, a.col0, b.col0 );
  Add( &dst->col1, a.col1, b.col1 );
  Add( &dst->col2, a.col2, b.col2 );
}
Templ Inl void
Add( mat2x2r<T>* dst, mat2x2r<T>& a, mat2x2r<T>& b )
{
  Add( &dst->row0, a.row0, b.row0 );
  Add( &dst->row1, a.row1, b.row1 );
}
Templ Inl void
Add( mat2x2c<T>* dst, mat2x2c<T>& a, mat2x2c<T>& b )
{
  Add( &dst->col0, a.col0, b.col0 );
  Add( &dst->col1, a.col1, b.col1 );
}



Templ Inl void
Sub( mat4x4r<T>* dst, mat4x4r<T>& a, mat4x4r<T>& b )
{
  Sub( &dst->row0, a.row0, b.row0 );
  Sub( &dst->row1, a.row1, b.row1 );
  Sub( &dst->row2, a.row2, b.row2 );
  Sub( &dst->row3, a.row3, b.row3 );
}
Templ Inl void
Sub( mat4x4c<T>* dst, mat4x4c<T>& a, mat4x4c<T>& b )
{
  Sub( &dst->col0, a.col0, b.col0 );
  Sub( &dst->col1, a.col1, b.col1 );
  Sub( &dst->col2, a.col2, b.col2 );
  Sub( &dst->col3, a.col3, b.col3 );
}
Templ Inl void
Sub( mat3x3r<T>* dst, mat3x3r<T>& a, mat3x3r<T>& b )
{
  Sub( &dst->row0, a.row0, b.row0 );
  Sub( &dst->row1, a.row1, b.row1 );
  Sub( &dst->row2, a.row2, b.row2 );
}
Templ Inl void
Sub( mat3x3c<T>* dst, mat3x3c<T>& a, mat3x3c<T>& b )
{
  Sub( &dst->col0, a.col0, b.col0 );
  Sub( &dst->col1, a.col1, b.col1 );
  Sub( &dst->col2, a.col2, b.col2 );
}
Templ Inl void
Sub( mat2x2r<T>* dst, mat2x2r<T>& a, mat2x2r<T>& b )
{
  Sub( &dst->row0, a.row0, b.row0 );
  Sub( &dst->row1, a.row1, b.row1 );
}
Templ Inl void
Sub( mat2x2c<T>* dst, mat2x2c<T>& a, mat2x2c<T>& b )
{
  Sub( &dst->col0, a.col0, b.col0 );
  Sub( &dst->col1, a.col1, b.col1 );
}



Templ Inl void
Mul( mat4x4r<T>* dst, mat4x4r<T>& a, mat4x4r<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &a ) );
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->row0.x = a.row0.x * b.row0.x + a.row0.y * b.row1.x + a.row0.z * b.row2.x + a.row0.w * b.row3.x;
  dst->row0.y = a.row0.x * b.row0.y + a.row0.y * b.row1.y + a.row0.z * b.row2.y + a.row0.w * b.row3.y;
  dst->row0.z = a.row0.x * b.row0.z + a.row0.y * b.row1.z + a.row0.z * b.row2.z + a.row0.w * b.row3.z;
  dst->row0.w = a.row0.x * b.row0.w + a.row0.y * b.row1.w + a.row0.z * b.row2.w + a.row0.w * b.row3.w;

  dst->row1.x = a.row1.x * b.row0.x + a.row1.y * b.row1.x + a.row1.z * b.row2.x + a.row1.w * b.row3.x;
  dst->row1.y = a.row1.x * b.row0.y + a.row1.y * b.row1.y + a.row1.z * b.row2.y + a.row1.w * b.row3.y;
  dst->row1.z = a.row1.x * b.row0.z + a.row1.y * b.row1.z + a.row1.z * b.row2.z + a.row1.w * b.row3.z;
  dst->row1.w = a.row1.x * b.row0.w + a.row1.y * b.row1.w + a.row1.z * b.row2.w + a.row1.w * b.row3.w;

  dst->row2.x = a.row2.x * b.row0.x + a.row2.y * b.row1.x + a.row2.z * b.row2.x + a.row2.w * b.row3.x;
  dst->row2.y = a.row2.x * b.row0.y + a.row2.y * b.row1.y + a.row2.z * b.row2.y + a.row2.w * b.row3.y;
  dst->row2.z = a.row2.x * b.row0.z + a.row2.y * b.row1.z + a.row2.z * b.row2.z + a.row2.w * b.row3.z;
  dst->row2.w = a.row2.x * b.row0.w + a.row2.y * b.row1.w + a.row2.z * b.row2.w + a.row2.w * b.row3.w;

  dst->row3.x = a.row3.x * b.row0.x + a.row3.y * b.row1.x + a.row3.z * b.row2.x + a.row3.w * b.row3.x;
  dst->row3.y = a.row3.x * b.row0.y + a.row3.y * b.row1.y + a.row3.z * b.row2.y + a.row3.w * b.row3.y;
  dst->row3.z = a.row3.x * b.row0.z + a.row3.y * b.row1.z + a.row3.z * b.row2.z + a.row3.w * b.row3.z;
  dst->row3.w = a.row3.x * b.row0.w + a.row3.y * b.row1.w + a.row3.z * b.row2.w + a.row3.w * b.row3.w;
}
Templ Inl void
Mul( mat4x4c<T>* dst, mat4x4c<T>& a, mat4x4c<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &a ) );
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->col0.x = a.col0.x * b.col0.x + a.col1.x * b.col0.y + a.col2.x * b.col0.z + a.col3.x * b.col0.w;
  dst->col0.y = a.col0.y * b.col0.x + a.col1.y * b.col0.y + a.col2.y * b.col0.z + a.col3.y * b.col0.w;
  dst->col0.z = a.col0.z * b.col0.x + a.col1.z * b.col0.y + a.col2.z * b.col0.z + a.col3.z * b.col0.w;
  dst->col0.w = a.col0.w * b.col0.x + a.col1.w * b.col0.y + a.col2.w * b.col0.z + a.col3.w * b.col0.w;

  dst->col1.x = a.col0.x * b.col1.x + a.col1.x * b.col1.y + a.col2.x * b.col1.z + a.col3.x * b.col1.w;
  dst->col1.y = a.col0.y * b.col1.x + a.col1.y * b.col1.y + a.col2.y * b.col1.z + a.col3.y * b.col1.w;
  dst->col1.z = a.col0.z * b.col1.x + a.col1.z * b.col1.y + a.col2.z * b.col1.z + a.col3.z * b.col1.w;
  dst->col1.w = a.col0.w * b.col1.x + a.col1.w * b.col1.y + a.col2.w * b.col1.z + a.col3.w * b.col1.w;

  dst->col2.x = a.col0.x * b.col2.x + a.col1.x * b.col2.y + a.col2.x * b.col2.z + a.col3.x * b.col2.w;
  dst->col2.y = a.col0.y * b.col2.x + a.col1.y * b.col2.y + a.col2.y * b.col2.z + a.col3.y * b.col2.w;
  dst->col2.z = a.col0.z * b.col2.x + a.col1.z * b.col2.y + a.col2.z * b.col2.z + a.col3.z * b.col2.w;
  dst->col2.w = a.col0.w * b.col2.x + a.col1.w * b.col2.y + a.col2.w * b.col2.z + a.col3.w * b.col2.w;

  dst->col3.x = a.col0.x * b.col3.x + a.col1.x * b.col3.y + a.col2.x * b.col3.z + a.col3.x * b.col3.w;
  dst->col3.y = a.col0.y * b.col3.x + a.col1.y * b.col3.y + a.col2.y * b.col3.z + a.col3.y * b.col3.w;
  dst->col3.z = a.col0.z * b.col3.x + a.col1.z * b.col3.y + a.col2.z * b.col3.z + a.col3.z * b.col3.w;
  dst->col3.w = a.col0.w * b.col3.x + a.col1.w * b.col3.y + a.col2.w * b.col3.z + a.col3.w * b.col3.w;
}
Templ Inl void
Mul( mat3x3r<T>* dst, mat3x3r<T>& a, mat3x3r<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &a ) );
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->row0.x = a.row0.x * b.row0.x + a.row0.y * b.row1.x + a.row0.z * b.row2.x;
  dst->row0.y = a.row0.x * b.row0.y + a.row0.y * b.row1.y + a.row0.z * b.row2.y;
  dst->row0.z = a.row0.x * b.row0.z + a.row0.y * b.row1.z + a.row0.z * b.row2.z;

  dst->row1.x = a.row1.x * b.row0.x + a.row1.y * b.row1.x + a.row1.z * b.row2.x;
  dst->row1.y = a.row1.x * b.row0.y + a.row1.y * b.row1.y + a.row1.z * b.row2.y;
  dst->row1.z = a.row1.x * b.row0.z + a.row1.y * b.row1.z + a.row1.z * b.row2.z;

  dst->row2.x = a.row2.x * b.row0.x + a.row2.y * b.row1.x + a.row2.z * b.row2.x;
  dst->row2.y = a.row2.x * b.row0.y + a.row2.y * b.row1.y + a.row2.z * b.row2.y;
  dst->row2.z = a.row2.x * b.row0.z + a.row2.y * b.row1.z + a.row2.z * b.row2.z;
}
Templ Inl void
Mul( mat3x3c<T>* dst, mat3x3c<T>& a, mat3x3c<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &a ) );
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->col0.x = a.col0.x * b.col0.x + a.col1.x * b.col0.y + a.col2.x * b.col0.z;
  dst->col0.y = a.col0.y * b.col0.x + a.col1.y * b.col0.y + a.col2.y * b.col0.z;
  dst->col0.z = a.col0.z * b.col0.x + a.col1.z * b.col0.y + a.col2.z * b.col0.z;

  dst->col1.x = a.col0.x * b.col1.x + a.col1.x * b.col1.y + a.col2.x * b.col1.z;
  dst->col1.y = a.col0.y * b.col1.x + a.col1.y * b.col1.y + a.col2.y * b.col1.z;
  dst->col1.z = a.col0.z * b.col1.x + a.col1.z * b.col1.y + a.col2.z * b.col1.z;

  dst->col2.x = a.col0.x * b.col2.x + a.col1.x * b.col2.y + a.col2.x * b.col2.z;
  dst->col2.y = a.col0.y * b.col2.x + a.col1.y * b.col2.y + a.col2.y * b.col2.z;
  dst->col2.z = a.col0.z * b.col2.x + a.col1.z * b.col2.y + a.col2.z * b.col2.z;
}
Templ Inl void
Mul( mat2x2r<T>* dst, mat2x2r<T>& a, mat2x2r<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &a ) );
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->row0.x = a.row0.x * b.row0.x + a.row0.y * b.row1.x;
  dst->row0.y = a.row0.x * b.row0.y + a.row0.y * b.row1.y;

  dst->row1.x = a.row1.x * b.row0.x + a.row1.y * b.row1.x;
  dst->row1.y = a.row1.x * b.row0.y + a.row1.y * b.row1.y;
}
Templ Inl void
Mul( mat2x2c<T>* dst, mat2x2c<T>& a, mat2x2c<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &a ) );
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->col0.x = a.col0.x * b.col0.x + a.col1.x * b.col0.y;
  dst->col0.y = a.col0.y * b.col0.x + a.col1.y * b.col0.y;

  dst->col1.x = a.col0.x * b.col1.x + a.col1.x * b.col1.y;
  dst->col1.y = a.col0.y * b.col1.x + a.col1.y * b.col1.y;
}



Templ Inl void
Mul( vec4<T>* dst, mat4x4r<T>& a, vec4<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->x = Dot( a.row0, b );
  dst->y = Dot( a.row1, b );
  dst->z = Dot( a.row2, b );
  dst->w = Dot( a.row3, b );
}
Templ Inl void
Mul( vec4<T>* dst, mat4x4c<T>& a, vec4<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->x = a.col0.x * b.x + a.col1.x * b.y + a.col2.x * b.z + a.col3.x * b.w;
  dst->y = a.col0.y * b.x + a.col1.y * b.y + a.col2.y * b.z + a.col3.y * b.w;
  dst->z = a.col0.z * b.x + a.col1.z * b.y + a.col2.z * b.z + a.col3.z * b.w;
  dst->w = a.col0.w * b.x + a.col1.w * b.y + a.col2.w * b.z + a.col3.w * b.w;
}
Templ Inl void
Mul( vec3<T>* dst, mat3x3r<T>& a, vec3<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->x = Dot( a.row0, b );
  dst->y = Dot( a.row1, b );
  dst->z = Dot( a.row2, b );
}
Templ Inl void
Mul( vec3<T>* dst, mat3x3c<T>& a, vec3<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->x = a.col0.x * b.x + a.col1.x * b.y + a.col2.x * b.z;
  dst->y = a.col0.y * b.x + a.col1.y * b.y + a.col2.y * b.z;
  dst->z = a.col0.z * b.x + a.col1.z * b.y + a.col2.z * b.z;
}
Templ Inl void
Mul( vec2<T>* dst, mat2x2r<T>& a, vec2<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->x = Dot( a.row0, b );
  dst->y = Dot( a.row1, b );
}
Templ Inl void
Mul( vec2<T>* dst, mat2x2c<T>& a, vec2<T>& b )
{
  AssertCrash( Cast( idx_t, dst ) != Cast( idx_t, &b ) );

  dst->x = a.col0.x * b.x + a.col1.x * b.y;
  dst->y = a.col0.y * b.x + a.col1.y * b.y;
}



Templ Inl void
Zero( mat4x4r<T>* dst )
{
  Zero( &dst->row0 );
  Zero( &dst->row1 );
  Zero( &dst->row2 );
  Zero( &dst->row3 );
}
Templ Inl void
Zero( mat4x4c<T>* dst )
{
  Zero( &dst->col0 );
  Zero( &dst->col1 );
  Zero( &dst->col2 );
  Zero( &dst->col3 );
}
Templ Inl void
Zero( mat3x3r<T>* dst )
{
  Zero( &dst->row0 );
  Zero( &dst->row1 );
  Zero( &dst->row2 );
}
Templ Inl void
Zero( mat3x3c<T>* dst )
{
  Zero( &dst->col0 );
  Zero( &dst->col1 );
  Zero( &dst->col2 );
}
Templ Inl void
Zero( mat2x2r<T>* dst )
{
  Zero( &dst->row0 );
  Zero( &dst->row1 );
}
Templ Inl void
Zero( mat2x2c<T>* dst )
{
  Zero( &dst->col0 );
  Zero( &dst->col1 );
}






Templ Inl void
Identity( mat4x4r<T>* dst )
{
  MAT4x4R( dst,
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
    );
}
Templ Inl void
Identity( mat4x4c<T>* dst )
{
  MAT4x4C( dst,
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
    );
}
Templ Inl void
Identity( mat3x3r<T>* dst )
{
  MAT3x3R( dst,
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
    );
}
Templ Inl void
Identity( mat3x3c<T>* dst )
{
  MAT3x3C( dst,
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
    );
}
Templ Inl void
Identity( mat2x2r<T>* dst )
{
  MAT2x2R( dst,
    1, 0,
    0, 1
    );
}
Templ Inl void
Identity( mat2x2c<T>* dst )
{
  MAT2x2C( dst,
    1, 0,
    0, 1
    );
}



Templ Inl void
_ElemSwap( T* a, T* b )
{
  T tmp = *a;  *a = *b;  *b = tmp;
}

Templ Inl void
Transpose( mat4x4r<T>* dst )
{
  _ElemSwap( &dst->row0.y, &dst->row1.x );
  _ElemSwap( &dst->row0.z, &dst->row2.x );
  _ElemSwap( &dst->row0.w, &dst->row3.x );
  _ElemSwap( &dst->row1.z, &dst->row2.y );
  _ElemSwap( &dst->row1.w, &dst->row3.y );
  _ElemSwap( &dst->row2.w, &dst->row3.z );
}
Templ Inl void
Transpose( mat4x4c<T>* dst )
{
  _ElemSwap( &dst->col0.y, &dst->col1.x );
  _ElemSwap( &dst->col0.z, &dst->col2.x );
  _ElemSwap( &dst->col0.w, &dst->col3.x );
  _ElemSwap( &dst->col1.z, &dst->col2.y );
  _ElemSwap( &dst->col1.w, &dst->col3.y );
  _ElemSwap( &dst->col2.w, &dst->col3.z );
}
Templ Inl void
Transpose( mat3x3r<T>* dst )
{
  _ElemSwap( &dst->row0.y, &dst->row1.x );
  _ElemSwap( &dst->row0.z, &dst->row2.x );
  _ElemSwap( &dst->row1.z, &dst->row2.y );
}
Templ Inl void
Transpose( mat3x3c<T>* dst )
{
  _ElemSwap( &dst->col0.y, &dst->col1.x );
  _ElemSwap( &dst->col0.z, &dst->col2.x );
  _ElemSwap( &dst->col1.z, &dst->col2.y );
}
Templ Inl void
Transpose( mat2x2r<T>* dst )
{
  _ElemSwap( &dst->row0.y, &dst->row1.x );
}
Templ Inl void
Transpose( mat2x2c<T>* dst )
{
  _ElemSwap( &dst->col0.y, &dst->col1.x );
}



Templ Inl void Transpose( mat4x4r<T>* dst, mat4x4r<T>& src ) { *dst = src;  Transpose( dst ); }
Templ Inl void Transpose( mat4x4c<T>* dst, mat4x4c<T>& src ) { *dst = src;  Transpose( dst ); }
Templ Inl void Transpose( mat3x3r<T>* dst, mat3x3r<T>& src ) { *dst = src;  Transpose( dst ); }
Templ Inl void Transpose( mat3x3c<T>* dst, mat3x3c<T>& src ) { *dst = src;  Transpose( dst ); }
Templ Inl void Transpose( mat2x2r<T>* dst, mat2x2r<T>& src ) { *dst = src;  Transpose( dst ); }
Templ Inl void Transpose( mat2x2c<T>* dst, mat2x2c<T>& src ) { *dst = src;  Transpose( dst ); }



Templ Inl void
Frustum( mat4x4r<T>* dst, T x0, T x1, T y0, T y1, T z0, T z1 )
{
  T rml_rec = 1 / ( x1 - x0 );
  T tmb_rec = 1 / ( y1 - y0 );
  T fmn_rec = 1 / ( z1 - z0 );

  MAT4x4R( dst,
    2 * z0 * rml_rec,  0,                 ( x1 + x0 ) * rml_rec,  0,
    0,                 2 * z0 * tmb_rec,  ( y1 + y0 ) * tmb_rec,  0,
    0,                 0,                 ( z0 + z1 ) * fmn_rec,  2 * z0 * z1 * fmn_rec,
    0,                 0,                 -1,                     0
    );
}
Templ Inl void
Frustum( mat4x4c<T>* dst, T x0, T x1, T y0, T y1, T z0, T z1 )
{
  T rml_rec = 1 / ( x1 - x0 );
  T tmb_rec = 1 / ( y1 - y0 );
  T fmn_rec = 1 / ( z1 - z0 );

  MAT4x4C( dst,
    2 * z0 * rml_rec,  0,                 ( x1 + x0 ) * rml_rec,  0,
    0,                 2 * z0 * tmb_rec,  ( y1 + y0 ) * tmb_rec,  0,
    0,                 0,                 ( z0 + z1 ) * fmn_rec,  2 * z0 * z1 * fmn_rec,
    0,                 0,                 -1,                     0
    );
}



Templ Inl void
Ortho( mat4x4r<T>* dst, T x0, T x1, T y0, T y1, T z0, T z1 )
{
  T rml_rec = 1 / ( x1 - x0 );
  T tmb_rec = 1 / ( y1 - y0 );
  T fmn_rec = 1 / ( z1 - z0 );

  MAT4x4R( dst,
    2 * rml_rec,  0,            0,            -( x1 + x0 ) * rml_rec,
    0,            2 * tmb_rec,  0,            -( y1 + y0 ) * tmb_rec,
    0,            0,            2 * fmn_rec,  -( z1 + z0 ) * fmn_rec,
    0,            0,            0,            1
    );
}
Templ Inl void
Ortho( mat4x4c<T>* dst, T x0, T x1, T y0, T y1, T z0, T z1 )
{
  T rml_rec = 1 / ( x1 - x0 );
  T tmb_rec = 1 / ( y1 - y0 );
  T fmn_rec = 1 / ( z1 - z0 );

  MAT4x4C( dst,
    2 * rml_rec,  0,            0,            -( x1 + x0 ) * rml_rec,
    0,            2 * tmb_rec,  0,            -( y1 + y0 ) * tmb_rec,
    0,            0,            2 * fmn_rec,  -( z1 + z0 ) * fmn_rec,
    0,            0,            0,            1
    );
}
Templ Inl void
Ortho( mat3x3r<T>* dst, T x0, T x1, T y0, T y1 )
{
  T rml_rec = 1 / ( x1 - x0 );
  T tmb_rec = 1 / ( y1 - y0 );

  MAT3x3R( dst,
    2 * rml_rec,  0,            -( x1 + x0 ) * rml_rec,
    0,            2 * tmb_rec,  -( y1 + y0 ) * tmb_rec,
    0,            0,            1
    );
}
Templ Inl void
Ortho( mat3x3c<T>* dst, T x0, T x1, T y0, T y1 )
{
  T rml_rec = 1 / ( x1 - x0 );
  T tmb_rec = 1 / ( y1 - y0 );

  MAT3x3C( dst,
    2 * rml_rec,  0,            -( x1 + x0 ) * rml_rec,
    0,            2 * tmb_rec,  -( y1 + y0 ) * tmb_rec,
    0,            0,            1
    );
}



Templ Inl void
Translate( mat4x4r<T>* dst, T x, T y, T z )
{
  MAT4x4R( dst,
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1
    );
}
Templ Inl void
Translate( mat4x4c<T>* dst, T x, T y, T z )
{
  MAT4x4C( dst,
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1
    );
}
Templ Inl void
Translate( mat3x3r<T>* dst, T x, T y )
{
  MAT3x3R( dst,
    1, 0, x,
    0, 1, y,
    0, 0, 1
    );
}
Templ Inl void
Translate( mat3x3c<T>* dst, T x, T y )
{
  MAT3x3C( dst,
    1, 0, x,
    0, 1, y,
    0, 0, 1
    );
}

Templ Inl void
Translate( mat4x4r<T>* dst, vec3<T>& xyz )
{
  MAT4x4R( dst,
    1, 0, 0, xyz.x,
    0, 1, 0, xyz.y,
    0, 0, 1, xyz.z,
    0, 0, 0, 1
    );
}
Templ Inl void
Translate( mat4x4c<T>* dst, vec3<T>& xyz )
{
  MAT4x4C( dst,
    1, 0, 0, xyz.x,
    0, 1, 0, xyz.y,
    0, 0, 1, xyz.z,
    0, 0, 0, 1
    );
}
Templ Inl void
Translate( mat3x3r<T>* dst, vec2<T>& xy )
{
  MAT3x3R( dst,
    1, 0, xy.x,
    0, 1, xy.y,
    0, 0, 1
    );
}
Templ Inl void
Translate( mat3x3c<T>* dst, vec2<T>& xy )
{
  MAT3x3C( dst,
    1, 0, xy.x,
    0, 1, xy.y,
    0, 0, 1
    );
}



Templ Inl void
Scale( mat4x4r<T>* dst, T x, T y, T z )
{
  MAT4x4R( dst,
    x, 0, 0, 0,
    0, y, 0, 0,
    0, 0, z, 0,
    0, 0, 0, 1
    );
}
Templ Inl void
Scale( mat4x4c<T>* dst, T x, T y, T z )
{
  MAT4x4C( dst,
    x, 0, 0, 0,
    0, y, 0, 0,
    0, 0, z, 0,
    0, 0, 0, 1
    );
}
Templ Inl void
Scale( mat3x3r<T>* dst, T x, T y, T z )
{
  MAT3x3R( dst,
    x, 0, 0,
    0, y, 0,
    0, 0, z
    );
}
Templ Inl void
Scale( mat3x3c<T>* dst, T x, T y, T z )
{
  MAT3x3C( dst,
    x, 0, 0,
    0, y, 0,
    0, 0, z
    );
}

Templ Inl void
Scale( mat4x4r<T>* dst, vec3<T>& xyz )
{
  MAT4x4R( dst,
    xyz.x, 0,     0,     0,
    0,     xyz.y, 0,     0,
    0,     0,     xyz.z, 0,
    0,     0,     0,     1
    );
}
Templ Inl void
Scale( mat4x4c<T>* dst, vec3<T>& xyz )
{
  MAT4x4C( dst,
    xyz.x, 0,     0,     0,
    0,     xyz.y, 0,     0,
    0,     0,     xyz.z, 0,
    0,     0,     0,     1
    );
}
Templ Inl void
Scale( mat3x3r<T>* dst, vec3<T>& xyz )
{
  MAT3x3R( dst,
    xyz.x, 0,     0,
    0,     xyz.y, 0,
    0,     0,     xyz.z
    );
}
Templ Inl void
Scale( mat3x3c<T>* dst, vec3<T>& xyz )
{
  MAT3x3C( dst,
    xyz.x, 0,     0,
    0,     xyz.y, 0,
    0,     0,     xyz.z
    );
}



Templ Inl void
Rotate( mat4x4r<T>* dst, vec3<T>& axis, T angle_radians )
{
  T xx = axis.x * axis.x;
  T xy = axis.x * axis.y;
  T xz = axis.x * axis.z;
  T yy = axis.y * axis.y;
  T yz = axis.y * axis.z;
  T zz = axis.z * axis.z;
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );
  T xs = axis.x * s;
  T ys = axis.y * s;
  T zs = axis.z * s;
  T mc = 1 - c;

  MAT4x4R( dst,
    xx * mc + c,   xy * mc - zs,  xz * mc + ys,  0,
    xy * mc + zs,  yy * mc + c,   yz * mc - xs,  0,
    xz * mc - ys,  yz * mc + xs,  zz * mc + c,   0,
    0,             0,             0,             1
    );
}
Templ Inl void
Rotate( mat4x4c<T>* dst, vec3<T>& axis, T angle_radians )
{
  T xx = axis.x * axis.x;
  T xy = axis.x * axis.y;
  T xz = axis.x * axis.z;
  T yy = axis.y * axis.y;
  T yz = axis.y * axis.z;
  T zz = axis.z * axis.z;
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );
  T xs = axis.x * s;
  T ys = axis.y * s;
  T zs = axis.z * s;
  T mc = 1 - c;

  MAT4x4C( dst,
    xx * mc + c,   xy * mc - zs,  xz * mc + ys,  0,
    xy * mc + zs,  yy * mc + c,   yz * mc - xs,  0,
    xz * mc - ys,  yz * mc + xs,  zz * mc + c,   0,
    0,             0,             0,             1
    );
}

Templ Inl void
Rotate( mat3x3r<T>* dst, vec3<T>& axis, T angle_radians )
{
  T xx = axis.x * axis.x;
  T xy = axis.x * axis.y;
  T xz = axis.x * axis.z;
  T yy = axis.y * axis.y;
  T yz = axis.y * axis.z;
  T zz = axis.z * axis.z;
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );
  T xs = axis.x * s;
  T ys = axis.y * s;
  T zs = axis.z * s;
  T mc = 1 - c;

  MAT3x3R( dst,
    xx * mc + c,   xy * mc - zs,  xz * mc + ys,
    xy * mc + zs,  yy * mc + c,   yz * mc - xs,
    xz * mc - ys,  yz * mc + xs,  zz * mc + c
    );
}
Templ Inl void
Rotate( mat3x3c<T>* dst, vec3<T>& axis, T angle_radians )
{
  T xx = axis.x * axis.x;
  T xy = axis.x * axis.y;
  T xz = axis.x * axis.z;
  T yy = axis.y * axis.y;
  T yz = axis.y * axis.z;
  T zz = axis.z * axis.z;
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );
  T xs = axis.x * s;
  T ys = axis.y * s;
  T zs = axis.z * s;
  T mc = 1 - c;

  MAT3x3C( dst,
    xx * mc + c,   xy * mc - zs,  xz * mc + ys,
    xy * mc + zs,  yy * mc + c,   yz * mc - xs,
    xz * mc - ys,  yz * mc + xs,  zz * mc + c
    );
}



Templ Inl void
Rotate( mat4x4r<T>* dst, vec4<T>& quaternion )
{
  T qxx = quaternion.x * quaternion.x;
  T qxy = quaternion.x * quaternion.y;
  T qxz = quaternion.x * quaternion.z;
  T qxw = quaternion.x * quaternion.w;
  T qyy = quaternion.y * quaternion.y;
  T qyz = quaternion.y * quaternion.z;
  T qyw = quaternion.y * quaternion.w;
  T qzz = quaternion.z * quaternion.z;
  T qzw = quaternion.z * quaternion.w;

  MAT4x4R( dst,
    1 - 2 * ( qyy + qzz ),  2 * ( qxy - qzw ),      2 * ( qxz + qyw ),      0,
    2 * ( qxy + qzw ),      1 - 2 * ( qxx + qzz ),  2 * ( qyz - qxw ),      0,
    2 * ( qxz - qyw ),      2 * ( qyz + qxw ),      1 - 2 * ( qxx + qyy ),  0,
    0,                      0,                      0,                      1
    );
}
Templ Inl void
Rotate( mat4x4c<T>* dst, vec4<T>& quaternion )
{
  T qxx = quaternion.x * quaternion.x;
  T qxy = quaternion.x * quaternion.y;
  T qxz = quaternion.x * quaternion.z;
  T qxw = quaternion.x * quaternion.w;
  T qyy = quaternion.y * quaternion.y;
  T qyz = quaternion.y * quaternion.z;
  T qyw = quaternion.y * quaternion.w;
  T qzz = quaternion.z * quaternion.z;
  T qzw = quaternion.z * quaternion.w;

  MAT4x4C( dst,
    1 - 2 * ( qyy + qzz ),  2 * ( qxy - qzw ),      2 * ( qxz + qyw ),      0,
    2 * ( qxy + qzw ),      1 - 2 * ( qxx + qzz ),  2 * ( qyz - qxw ),      0,
    2 * ( qxz - qyw ),      2 * ( qyz + qxw ),      1 - 2 * ( qxx + qyy ),  0,
    0,                      0,                      0,                      1
    );
}

Templ Inl void
Rotate( mat3x3r<T>* dst, vec4<T>& quaternion )
{
  T qxx = quaternion.x * quaternion.x;
  T qxy = quaternion.x * quaternion.y;
  T qxz = quaternion.x * quaternion.z;
  T qxw = quaternion.x * quaternion.w;
  T qyy = quaternion.y * quaternion.y;
  T qyz = quaternion.y * quaternion.z;
  T qyw = quaternion.y * quaternion.w;
  T qzz = quaternion.z * quaternion.z;
  T qzw = quaternion.z * quaternion.w;

  MAT3x3R( dst,
    1 - 2 * ( qyy + qzz ),  2 * ( qxy - qzw ),      2 * ( qxz + qyw ),
    2 * ( qxy + qzw ),      1 - 2 * ( qxx + qzz ),  2 * ( qyz - qxw ),
    2 * ( qxz - qyw ),      2 * ( qyz + qxw ),      1 - 2 * ( qxx + qyy )
    );
}
Templ Inl void
Rotate( mat3x3c<T>* dst, vec4<T>& quaternion )
{
  T qxx = quaternion.x * quaternion.x;
  T qxy = quaternion.x * quaternion.y;
  T qxz = quaternion.x * quaternion.z;
  T qxw = quaternion.x * quaternion.w;
  T qyy = quaternion.y * quaternion.y;
  T qyz = quaternion.y * quaternion.z;
  T qyw = quaternion.y * quaternion.w;
  T qzz = quaternion.z * quaternion.z;
  T qzw = quaternion.z * quaternion.w;

  MAT3x3C( dst,
    1 - 2 * ( qyy + qzz ),  2 * ( qxy - qzw ),      2 * ( qxz + qyw ),
    2 * ( qxy + qzw ),      1 - 2 * ( qxx + qzz ),  2 * ( qyz - qxw ),
    2 * ( qxz - qyw ),      2 * ( qyz + qxw ),      1 - 2 * ( qxx + qyy )
    );
}

Templ Inl void
RotateX( mat4x4r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT4x4R( dst,
    1,  0,  0,   0,
    0,  c,  -s,  0,
    0,  s,  c,   0,
    0,  0,  0,   1
    );
}
Templ Inl void
RotateX( mat4x4c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT4x4C( dst,
    1,  0,  0,   0,
    0,  c,  -s,  0,
    0,  s,  c,   0,
    0,  0,  0,   1
    );
}

Templ Inl void
RotateX( mat3x3r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3R( dst,
    1,  0,  0,
    0,  c,  -s,
    0,  s,  c
    );
}
Templ Inl void
RotateX( mat3x3c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3C( dst,
    1,  0,  0,
    0,  c,  -s,
    0,  s,  c
    );
}

Templ Inl void
RotateY( mat4x4r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT4x4R( dst,
    c,   0,  s,  0,
    0,   1,  0,  0,
    -s,  0,  c,  0,
    0,   0,  0,  1
    );
}
Templ Inl void
RotateY( mat4x4c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT4x4C( dst,
    c,   0,  s,  0,
    0,   1,  0,  0,
    -s,  0,  c,  0,
    0,   0,  0,  1
    );
}

Templ Inl void
RotateY( mat3x3r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3R( dst,
    c,   0,  s,
    0,   1,  0,
    -s,  0,  c
    );
}
Templ Inl void
RotateY( mat3x3c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3C( dst,
    c,   0,  s,
    0,   1,  0,
    -s,  0,  c
    );
}

Templ Inl void
RotateZ( mat4x4r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT4x4R( dst,
    c,  -s,  0,  0,
    s,  c,   0,  0,
    0,  0,   1,  0,
    0,  0,   0,  1
    );
}
Templ Inl void
RotateZ( mat4x4c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT4x4C( dst,
    c,  -s,  0,  0,
    s,  c,   0,  0,
    0,  0,   1,  0,
    0,  0,   0,  1
    );
}

Templ Inl void
RotateZ( mat3x3r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3R( dst,
    c,  -s,  0,
    s,  c,   0,
    0,  0,   1
    );
}
Templ Inl void
RotateZ( mat3x3c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3C( dst,
    c,  -s,  0,
    s,  c,   0,
    0,  0,   1
    );
}

Templ Inl void
Rotate( mat3x3r<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3R( dst,
    c,  -s,  0,
    s,  c,   0,
    0,  0,   1
    );
}
Templ Inl void
Rotate( mat3x3c<T>* dst, T angle_radians )
{
  T c = Cos( angle_radians );
  T s = Sin( angle_radians );

  MAT3x3C( dst,
    c,  -s,  0,
    s,  c,   0,
    0,  0,   1
    );
}



Templ Inl void
Cross( mat4x4r<T>* dst, vec3<T>& v )
{
  MAT4x4R( dst,
    0,     -v.z,  v.y,   0,
    v.z,   0,     -v.x,  0,
    -v.y,  v.x,   0,     0,
    0,     0,     0,     1
    );
}
Templ Inl void
Cross( mat4x4c<T>* dst, vec3<T>& v )
{
  MAT4x4C( dst,
    0,     -v.z,  v.y,   0,
    v.z,   0,     -v.x,  0,
    -v.y,  v.x,   0,     0,
    0,     0,     0,     1
    );
}
Templ Inl void
Cross( mat3x3r<T>* dst, vec3<T>& v )
{
  MAT3x3R( dst,
    0,     -v.z,  v.y,
    v.z,   0,     -v.x,
    -v.y,  v.x,   0
    );
}
Templ Inl void
Cross( mat3x3c<T>* dst, vec3<T>& v )
{
  MAT3x3C( dst,
    0,     -v.z,  v.y,
    v.z,   0,     -v.x,
    -v.y,  v.x,   0
    );
}



Templ void
Col( mat4x4r<T>& src, u32 idx, vec4<T>* dst )
{
  dst->x = src.row0[idx];
  dst->y = src.row1[idx];
  dst->z = src.row2[idx];
  dst->w = src.row3[idx];
}

Templ void
Row( mat4x4c<T>& src, u32 idx, vec4<T>* dst )
{
  dst->x = src.cols0[idx];
  dst->y = src.cols1[idx];
  dst->z = src.cols2[idx];
  dst->w = src.cols3[idx];
}

Templ void
Col( mat3x3r<T>& src, u32 idx, vec3<T>* dst )
{
  dst->x = src.row0[idx];
  dst->y = src.row1[idx];
  dst->z = src.row2[idx];
}

Templ void
Row( mat3x3c<T>& src, u32 idx, vec3<T>* dst )
{
  dst->x = src.cols0[idx];
  dst->y = src.cols1[idx];
  dst->z = src.cols2[idx];
}


Enumc( alloctype_t )
{
  no_alloc,
  crt_heap,
  virtualalloc,
};

// arbitrary-length contiguous string of memory.
struct
string_t
{
  u8* mem;
  idx_t len; // # of bytes mem can possibly hold.
  alloctype_t alloctype;
};


Inl void
Zero( string_t& dst )
{
  dst.mem = 0;
  dst.len = 0;
  dst.alloctype = alloctype_t::no_alloc;
}


constant idx_t c_virtualalloc_threshold = 100*1024*1024;

Inl void
Alloc( string_t& dst, idx_t nbytes )
{
  dst.len = nbytes;

  if( nbytes >= c_virtualalloc_threshold ) {
    dst.alloctype = alloctype_t::virtualalloc;
    dst.mem = MemVirtualAlloc( u8, nbytes );
  } else {
    dst.alloctype = alloctype_t::crt_heap;
    dst.mem = MemHeapAlloc( u8, nbytes );
  }
}

Inl void
Free( string_t& dst )
{
  if( dst.mem ) {
    switch( dst.alloctype ) {
      case alloctype_t::crt_heap:
        MemHeapFree( dst.mem );
        break;

      case alloctype_t::virtualalloc:
        MemVirtualFree( dst.mem );
        break;

      case alloctype_t::no_alloc:
        break;

      default: UnreachableCrash();
    }
  }
  Zero( dst );
}


Inl void
ExpandTo( string_t& dst, idx_t len_new )
{
  AssertCrash( len_new > dst.len );
  switch( dst.alloctype ) {
    case alloctype_t::crt_heap:
      dst.mem = MemHeapRealloc( u8, dst.mem, dst.len, len_new );
      break;

    case alloctype_t::virtualalloc:
      dst.mem = MemVirtualRealloc( u8, dst.mem, dst.len, len_new );
      break;

    default: UnreachableCrash();
  }
  dst.len = len_new;
}

Inl void
ShrinkTo( string_t& dst, idx_t len_new )
{
  AssertCrash( len_new < dst.len );
  AssertCrash( len_new > 0 );
  switch( dst.alloctype ) {
    case alloctype_t::crt_heap:
      dst.mem = MemHeapRealloc( u8, dst.mem, dst.len, len_new );
      break;

    case alloctype_t::virtualalloc:
      dst.mem = MemVirtualRealloc( u8, dst.mem, dst.len, len_new );
      break;

    default: UnreachableCrash();
  }
  dst.len = len_new;
}



Inl void
Reserve( string_t& dst, idx_t len_new )
{
  if( dst.len < len_new ) {
    idx_t m = 2;
    while( m * dst.len < len_new ) {
      m *= 2;
    }
    ExpandTo( dst, m * dst.len );
  }
}


Inl bool
PtrInsideMem( string_t& str, void* ptr )
{
  AssertCrash( str.mem );
  AssertCrash( str.len );

  void* start = str.mem;
  void* end = str.mem + str.len;
  bool inside = ( start <= ptr )  &&  ( ptr < end );
  return inside;
}

Inl bool
Equal( string_t& a, string_t& b )
{
  return MemEqual( ML( a ), ML( b ) );
}


// arbitrary-length contiguous string of memory.
// short-lived view on other memory.
struct
slice_t
{
  u8* mem;
  idx_t len; // # of bytes mem can possibly hold.
};

Inl slice_t
SliceFromString( string_t& a )
{
  slice_t r;
  r.mem = a.mem;
  r.len = a.len;
  return r;
}

Inl bool
EqualContents( slice_t& a, slice_t& b )
{
  return MemEqual( ML( a ), ML( b ) );
}


struct
plist_t
{
  u8* current_page;
  idx_t pagesize; // TODO: remove
  idx_t pagetop;
  idx_t max_aligned;
  idx_t userbytes;
};

struct
plistheader_t
{
  u8* prev;
  u8* next;
  idx_t pagesize;
};

Inl plistheader_t*
_GetHeader( u8* page )
{
  return Cast( plistheader_t*, page );
}



Inl void
Zero( plist_t& list )
{
  list.current_page = 0;
  list.pagesize = 0;
  list.pagetop = 0;
  list.max_aligned = 0;
  list.userbytes = 0;
}

Inl void
Init( plist_t& list, idx_t initial_size )
{
  Zero( list );
  list.pagesize = initial_size + sizeof( plistheader_t );
  list.current_page = MemHeapAlloc( u8, list.pagesize );
  list.pagetop = sizeof( plistheader_t );
  list.max_aligned = 1;
  auto header = _GetHeader( list.current_page );
  header->prev = 0;
  header->next = 0;
  header->pagesize = list.pagesize;
}

Inl void
Kill( plist_t& list )
{
  auto page = list.current_page;
  auto header = _GetHeader( list.current_page );
  AssertCrash( !header->next );
  auto prev = header->prev;
  MemHeapFree( page );
  while( prev ) {
    page = prev;
    header = _GetHeader( page );
    prev = header->prev;
    MemHeapFree( page );
  }
  Zero( list );
}

Inl void
Reset( plist_t& list )
{
  auto page = list.current_page;
  auto header = _GetHeader( list.current_page );
  AssertCrash( !header->next );
  while( header->prev ) {
    auto prev = header->prev;
    MemHeapFree( page );
    page = prev;
    header = _GetHeader( page );
  }
  header->next = 0;
  list.current_page = page;
  list.pagetop = sizeof( plistheader_t );
  list.max_aligned = 1;
  list.userbytes = 0;
}

#define AddPlist( type, list, count ) \
  Cast( type*, AddBackBytes( list, sizeof( type ), count * sizeof( type ) ) )

Inl u8*
AddBackBytes( plist_t& list, idx_t align, idx_t len )
{
  list.max_aligned = MAX( list.max_aligned, align );
  list.userbytes += len;

  // align pagetop
  auto rem = list.pagetop % align;
  if( rem ) {
    list.pagetop += ( align - rem );
  }

  // return pagetop if possible.
  if( ( list.pagetop <= list.pagesize )  &&  ( len <= list.pagesize - list.pagetop ) ) {
    auto r = list.current_page + list.pagetop;
    list.pagetop += len;
    return r;
  }

  // alloc a new page.
  auto pagesize = list.pagesize;
  AssertCrash( pagesize );
  // only expand x2 when we really have to.
  while( pagesize < len + sizeof( plistheader_t ) ) {
    pagesize *= 2;
  }
  auto page = MemHeapAlloc( u8, pagesize );
  auto oldpage = list.current_page;
  auto oldheader = _GetHeader( oldpage );
  oldheader->next = page;
  auto header = _GetHeader( page );
  header->prev = oldpage;
  header->next = 0;
  header->pagesize = pagesize;
  list.current_page = page;
  list.pagesize = pagesize;
  list.pagetop = sizeof( plistheader_t );

  // align pagetop
  rem = list.pagetop % align;
  if( rem ) {
    list.pagetop += ( align - rem );
  }

  // return pagetop.
  auto r = list.current_page + list.pagetop;
  list.pagetop += len;
  return r;
}







#if 0 // This doesn't work as-is, since each page doesn't maintain a 'top' count.

  // test code to exhibit the problem:
  {
    plist_t list;
    Init( list, 4 );

    auto r0 = AddPlist( u8, list, 6 );
    Memmove( r0, "123456", 6 );
    AssertCrash( MemEqual( r0, "123456", 6 ) );

    auto str = StringFromPlist( list );
    AssertCrash( MemEqual( ML( str ), "123456", 6 ) );
    Free( str );

    Kill( list );
  }

  Inl string_t
  StringFromPlist( plist_t& list )
  {
    // we probably don't want to make a contiguous buffer from a plist, unless we've only ever allocated u8s.
    AssertCrash( list.max_aligned == 1 );

    string_t r;
    Alloc( r, list.userbytes );

    auto page = list.current_page;
    auto header = _GetHeader( page );
    AssertCrash( !header->next );

    // walk back to the first page.
    while( header->prev ) {
      page = header->prev;
      header = _GetHeader( page );
    }

    // walk forw to the last page, copying mem along the way.
    idx_t count = 0;
    while( page ) {
      auto current_pagetop = header->next  ?  header->pagesize  :  list.pagetop;
      auto userbytes_in_page = current_pagetop - sizeof( plistheader_t );
      Memmove( r.mem + count, header + 1, userbytes_in_page );
      count += userbytes_in_page;

      page = header->next;
      header = _GetHeader( page );
    }
    AssertCrash( count == list.userbytes );
    return r;
  }

#endif



// meant to hold elements which are uniform in size.
Templ struct
array_t
{
  T* mem;
  idx_t capacity; // # of elements mem can possibly hold.
  idx_t len; // # of elements in mem.
};


Inl slice_t
SliceFromArray( array_t<u8>& array )
{
  slice_t r;
  r.mem = array.mem;
  r.len = array.len;
  return r;
}


Templ Inl void
Zero( array_t<T>& array )
{
  array.mem = 0;
  array.capacity = 0;
  array.len = 0;
}



Templ Inl void
Alloc( array_t<T>& array, idx_t nelems )
{
  AssertCrash( sizeof( T ) * nelems <= c_virtualalloc_threshold ); // You should use other data structures for large allocations!
  Zero( array );
  array.mem = MemHeapAlloc( T, nelems );
  array.capacity = nelems;
}


Templ Inl void
Free( array_t<T>& array )
{
  AssertCrash( array.len <= array.capacity );
  if( array.mem ) {
    MemHeapFree( array.mem );
  }
  Zero( array );
}



Templ Inl void
Reserve( array_t<T>& array, idx_t enforce_capacity )
{
  AssertCrash( array.capacity );
  AssertCrash( array.len <= array.capacity );
  AssertCrash( sizeof( T ) * enforce_capacity <= c_virtualalloc_threshold ); // You should use other data structures for large allocations!
  if( array.capacity < enforce_capacity ) {
    auto new_capacity = 2 * array.capacity;
    while( new_capacity < enforce_capacity ) {
      new_capacity *= 2;
    }
    array.mem = MemHeapRealloc( T, array.mem, array.capacity, new_capacity );
    array.capacity = new_capacity;
  }
}



Templ Inl void
Copy( array_t<T>& array, array_t<T>& src )
{
  AssertCrash( array.len <= array.capacity );
  Reserve( array, src.len );
  Memmove( array.mem, src.mem, src.len * sizeof( T ) );
  array.len = src.len;
}




Templ Inl T*
AddBack( array_t<T>& array, idx_t nelems = 1 )
{
  AssertCrash( array.len <= array.capacity );
  Reserve( array, array.len + nelems );
  auto r = array.mem + array.len;
  array.len += nelems;
  return r;
}


Templ Inl T*
AddAt( array_t<T>& array, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( array.len <= array.capacity );
  AssertCrash( idx <= array.len );
  Reserve( array, array.len + nelems );
  if( idx < array.len ) {
    auto nshift = array.len - idx;
    Memmove(
      array.mem + idx + nelems,
      array.mem + idx,
      sizeof( T ) * nshift
    );
  }
  auto r = array.mem + idx;
  array.len += nelems;
  return r;
}


Templ Inl void
RemBack( array_t<T>& array, idx_t nelems = 1 )
{
  AssertCrash( array.len <= array.capacity );
  AssertCrash( nelems <= array.len );
  array.len -= nelems;
}


Templ Inl void
RemAt( array_t<T>& array, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( array.len <= array.capacity );
  AssertCrash( array.len >= idx + nelems );
  if( idx + nelems < array.len ) {
    Memmove(
      array.mem + idx,
      array.mem + idx + nelems,
      sizeof( T ) * ( array.len - idx - nelems )
      );
  }
  array.len -= nelems;
}

// search { mem, len } for val.
// returns 'idx', the place where val would belong in sorted order. this is in the interval [ 0, len ].
//   '0' meaning val belongs before the element currently at index 0.
//   'len' meaning val belongs at the very end.
Templ Inl void
BinarySearch( T* mem, idx_t len, T val, idx_t* sorted_insert_idx )
{
  idx_t left = 0;
  auto middle = len / 2;
  auto right = len;
  Forever {
    if( left == right ) {
      *sorted_insert_idx = left;
      return;
    }
    auto mid = mem[middle];
    if( val < mid ) {
      // left stays put.
      right = middle;
      middle = left + ( right - left ) / 2;
    } elif( val > mid ) {
      left = middle;
      middle = left + ( right - left ) / 2;
      // right stays put.

      // out of bounds on right side.
      if( left == middle ) {
        *sorted_insert_idx = right;
        return;
      }
    } else {
      *sorted_insert_idx = middle;
      return;
    }
  }
}

Templ Inl bool
ArrayContains( T* mem, idx_t len, T* val )
{
  For( i, 0, len ) {
    if( *Cast( T*, val ) == *Cast( T*, mem + i ) ) {
      return 1;
    }
  }
  return 0;
}




#define TMPL   template<typename T, idx_t N>


TMPL struct
embeddedarray_t
{
  idx_t len; // # of elements in mem.
  T mem[N];
};


template<idx_t N> Inl slice_t
SliceFromArray( embeddedarray_t<u8, N>& array )
{
  slice_t r;
  r.mem = array.mem;
  r.len = array.len;
  return r;
}


TMPL Inl void
Zero( embeddedarray_t<T, N>& array )
{
  array.len = 0;
}


TMPL Inl idx_t
Capacity( embeddedarray_t<T, N>& array )
{
  return N;
}


TMPL Inl void
Copy( embeddedarray_t<T, N>& dst, embeddedarray_t<T, N>& src )
{
  AssertCrash( dst.len <= N );
  Memmove( dst.mem, src.mem, src.len * sizeof( T ) );
  dst.len = src.len;
}

TMPL Inl void
Clear( embeddedarray_t<T, N>& array )
{
  array.len = 0;
}


TMPL Inl T*
AddBack( embeddedarray_t<T, N>& array, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= N );
  auto r = array.mem + array.len;
  array.len += nelems;
  return r;
}


TMPL Inl T*
AddAt( embeddedarray_t<T, N>& array, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= N );
  AssertCrash( idx <= array.len );
  if( idx < array.len ) {
    auto nshift = array.len - idx;
    Memmove(
      array.mem + idx + nelems,
      array.mem + idx,
      sizeof( T ) * nshift
    );
  }
  auto r = array.mem + idx;
  array.len += nelems;
  return r;
}


TMPL Inl void
RemBack( embeddedarray_t<T, N>& array, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= N );
  AssertCrash( nelems <= array.len );
  array.len -= nelems;
}


TMPL Inl void
RemAt( embeddedarray_t<T, N>& array, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= N );
  AssertCrash( array.len >= idx + nelems );
  if( idx + nelems < array.len ) {
    Memmove(
      array.mem + idx,
      array.mem + idx + nelems,
      sizeof( T ) * ( array.len - idx - nelems )
      );
  }
  array.len -= nelems;
}

#undef TMPL



Templ struct
fixedarray_t
{
  idx_t len; // # of elements in mem.
  T* mem;
  idx_t capacity; // # of elements mem can possibly hold.
};



Templ Inl void
Zero( fixedarray_t<T>& array )
{
  array.len = 0;
  array.mem = 0;
  array.capacity = 0;
}



Templ Inl void
Alloc( fixedarray_t<T>& array, idx_t nelems )
{
  AssertCrash( sizeof( T ) * nelems <= c_virtualalloc_threshold ); // You should use other data structures for large allocations!
  Zero( array );
  array.mem = MemHeapAlloc( T, nelems );
  array.capacity = nelems;
}


Templ Inl void
Free( fixedarray_t<T>& array )
{
  AssertCrash( array.len <= array.capacity );
  if( array.mem ) {
    MemHeapFree( array.mem );
  }
  Zero( array );
}



// Intentionally no expansion/contraction here!


Templ Inl idx_t
Capacity( fixedarray_t<T>& array )
{
  return array.capacity;
}


Templ Inl T*
AddBack( fixedarray_t<T>& array, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= array.capacity );
  auto r = array.mem + array.len;
  array.len += nelems;
  return r;
}


Templ Inl T*
AddAt( fixedarray_t<T>& array, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= array.capacity );
  AssertCrash( idx <= array.len );
  if( idx < array.len ) {
    auto nshift = array.len - idx;
    Memmove(
      array.mem + idx + nelems,
      array.mem + idx,
      sizeof( T ) * nshift
    );
  }
  auto r = array.mem + idx;
  array.len += nelems;
  return r;
}


Templ Inl void
RemBack( fixedarray_t<T>& array, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= array.capacity );
  AssertCrash( nelems <= array.len );
  array.len -= nelems;
}


Templ Inl void
RemAt( fixedarray_t<T>& array, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( array.len + nelems <= array.capacity );
  AssertCrash( array.len >= idx + nelems );
  if( idx + nelems < array.len ) {
    Memmove(
      array.mem + idx,
      array.mem + idx + nelems,
      sizeof( T ) * ( array.len - idx - nelems )
      );
  }
  array.len -= nelems;
}



// NOTE: only cast to this; you shouldn't ever instantiate one of these.
Templ struct
pagearray_elem_t
{
  pagearray_elem_t<T>* prev;
  pagearray_elem_t<T>* next;

  T* mem; // points to memory immediately following this struct. important for proper alignment.

  // these are in units of pagearray_t.sizeof_elem
  // so, multiply by that number to get them in terms of bytes.
  idx_t capacity;
  idx_t len;
};

Templ struct
pagearray_t
{
  pagearray_elem_t<T>* first_page;
  pagearray_elem_t<T>* current_page;
  idx_t hdrbytes;
  idx_t totallen;
};



Templ Inl void
Zero( pagearray_t<T>& list )
{
  list.first_page = 0;
  list.current_page = 0;
  list.totallen = 0;
}

Templ Inl void
Init( pagearray_t<T>& list, idx_t nelems_capacity )
{
  AssertCrash( nelems_capacity );

  Zero( list );
  list.hdrbytes = RoundUpToMultipleOfN( sizeof( pagearray_elem_t<T> ), sizeof( T ) );

  auto page = Cast( pagearray_elem_t<T>*, MemHeapAlloc( u8, list.hdrbytes + nelems_capacity * sizeof( T ) ) );
  page->prev = 0;
  page->next = 0;
  page->mem = Cast( T*, Cast( u8*, page ) + list.hdrbytes );
  page->capacity = nelems_capacity;
  page->len = 0;

  list.current_page = page;
  list.first_page = list.current_page;
}

Templ Inl void
Kill( pagearray_t<T>& list )
{
  auto page = list.current_page;
  AssertCrash( !page->next );
  auto prev = page->prev;
  MemHeapFree( page );
  while( prev ) {
    page = prev;
    prev = page->prev;
    MemHeapFree( page );
  }
  Zero( list );
}

Templ Inl void
Reset( pagearray_t<T>& list )
{
  auto page = list.current_page;
  AssertCrash( !page->next );
  while( page->prev ) {
    auto prev = page->prev;
    MemHeapFree( page );
    page = prev;
  }
  page->next = 0;
  page->len = 0;
  list.current_page = page;
  list.totallen = 0;
}

Templ Inl T*
CurrentPage_AddBack( pagearray_t<T>& list, idx_t nelems )
{
  auto page = list.current_page;
  AssertCrash( page->len + nelems <= page->capacity );
  auto r = page->mem + page->len;
  page->len += nelems;
  list.totallen += nelems;
  return r;
}

Templ Inl bool
CurrentPage_HasRoomFor( pagearray_t<T>& list, idx_t nelems )
{
  auto page = list.current_page;
  auto r = page->len + nelems <= page->capacity;
  return r;
}

Templ Inl idx_t
CurrentPage_Capacity( pagearray_t<T>& list )
{
  auto page = list.current_page;
  return page->capacity;
}

Templ Inl void
AddNewPage( pagearray_t<T>& list, idx_t nelems_capacity )
{
  AssertCrash( nelems_capacity );

  auto page = Cast( pagearray_elem_t<T>*, MemHeapAlloc( u8, list.hdrbytes + nelems_capacity * sizeof( T ) ) );
  page->next = 0;
  page->mem = Cast( T*, Cast( u8*, page ) + list.hdrbytes );
  page->capacity = nelems_capacity;
  page->len = 0;

  page->prev = list.current_page;
  list.current_page->next = page;

  list.current_page = page;
}

Templ Inl T*
AddBack( pagearray_t<T>& list, idx_t nelems = 1 )
{
  if( !CurrentPage_HasRoomFor( list, nelems ) ) {
    AddNewPage( list, 2 * CurrentPage_Capacity( list ) );
  }
  auto elem = CurrentPage_AddBack( list, nelems );
  return elem;
}







Templ struct
pagerelativepos_t
{
  pagearray_elem_t<T>* page;
  idx_t idx;
};

Templ Inl pagerelativepos_t<T>
MakeIteratorAtLinearIndex( pagearray_t<T>& list, idx_t idx )
{
  AssertCrash( idx < list.totallen );
  ForNext( page, list.first_page ) {
    if( idx < page->len ) {
      pagerelativepos_t<T> pos;
      pos.page = page;
      pos.idx = idx;
      return pos;
    }
    idx -= page->len;
  }
  UnreachableCrash();
  return {};
}

Templ Inl T*
GetElemAtIterator( pagearray_t<T>& list, pagerelativepos_t<T> pos )
{
  auto elem = pos.page->mem + pos.idx;
  return elem;
}

Templ Inl pagerelativepos_t<T>
IteratorMoveR( pagearray_t<T>& list, pagerelativepos_t<T> pos, idx_t nelems = 1 )
{
  auto r = pos;
  r.idx += nelems;
  while( r.idx >= r.page->len ) {
    r.idx -= r.page->len;
    r.page = r.page->next;
    if( !r.page ) {
      // we allow pos to go one past the last element; same as integer indices.
      AssertCrash( !r.idx );
      break;
    }
  }
  return r;
}

Templ Inl T*
LookupElemByLinearIndex( pagearray_t<T>& list, idx_t idx )
{
  AssertCrash( idx < list.totallen );
  ForNext( page, list.first_page ) {
    if( idx < page->len ) {
      auto r = page->mem + idx;
      return r;
    }
    idx -= page->len;
  }
  UnreachableCrash();
  return 0;
}


Templ struct
listelem_t
{
  listelem_t<T>* prev;
  listelem_t<T>* next;
  T value;
};








Templ struct
list_t
{
  idx_t len; // number of elements currently in the list.
  listelem_t<T>* first;
  listelem_t<T>* last;
};


Templ Inl void
Zero( list_t<T>& list )
{
  list.len = 0;
  list.first = 0;
  list.last = 0;
}

Templ Inl void
_InitialInsert( list_t<T>& list, listelem_t<T>* elem )
{
  AssertCrash( !list.len );
  elem->prev = 0;
  elem->next = 0;
  list.first = elem;
  list.last = elem;
  list.len = 1;
}

Templ Inl void
InsertBefore( list_t<T>& list, listelem_t<T>* elem, listelem_t<T>* place )
{
  AssertCrash( elem );
  if( !list.len ) {
    AssertCrash( !place );
    _InitialInsert( list, elem );
  } else {
    AssertCrash( list.len );
    AssertCrash( place );
    auto prev = place->prev;
    auto next = place;
    elem->prev = prev;
    elem->next = next;
    if( prev ) {
      prev->next = elem;
    }
    next->prev = elem;
    if( list.first == place ) {
      list.first = elem;
    }
    list.len += 1;
  }
  AssertCrash( elem->prev != elem  &&  elem->next != elem );
}

Templ Inl void
InsertAfter( list_t<T>& list, listelem_t<T>* elem, listelem_t<T>* place )
{
  AssertCrash( elem );
  if( !list.len ) {
    AssertCrash( !place );
    _InitialInsert( list, elem );
  } else {
    AssertCrash( list.len );
    AssertCrash( place );
    auto prev = place;
    auto next = place->next;
    elem->prev = prev;
    elem->next = next;
    prev->next = elem;
    if( next ) {
      next->prev = elem;
    }
    if( list.last == place ) {
      list.last = elem;
    }
    list.len += 1;
  }
  AssertCrash( elem->prev != elem  &&  elem->next != elem );
}

Templ Inl void
InsertFirst( list_t<T>& list, listelem_t<T>* elem )
{
  InsertBefore( list, elem, list.first );
}

Templ Inl void
InsertLast( list_t<T>& list, listelem_t<T>* elem )
{
  InsertAfter( list, elem, list.last );
}

Templ Inl void
Rem( list_t<T>& list, listelem_t<T>* elem )
{
  AssertCrash( list.len );
  auto prev = elem->prev;
  auto next = elem->next;
  elem->next = 0;
  elem->prev = 0;
  if( prev ) {
    prev->next = next;
  }
  if( next ) {
    next->prev = prev;
  }
  if( list.first == elem ) {
    list.first = next;
  }
  if( list.last == elem ) {
    list.last = prev;
  }
  list.len -= 1;
}

Templ Inl void
RemFirst( list_t<T>& list )
{
  Rem( list, list.first );
}

Templ Inl void
RemLast( list_t<T>& list )
{
  Rem( list, list.last );
}



















Templ struct
listwalloc_t
{
  idx_t len; // number of elements currently in the list.
  listelem_t<T>* first;
  listelem_t<T>* last;
  plist_t* elems;
  embeddedarray_t<listelem_t<T>*, 128> free_elems; // indices into listwalloc_t.elems.
};


Templ Inl void
Init( listwalloc_t<T>& list, plist_t* elems )
{
  list.len = 0;
  list.first = 0;
  list.last = 0;
  list.elems = elems;
  list.free_elems.len = 0;
}

Templ Inl void
Kill( listwalloc_t<T>& list )
{
  list.len = 0;
  list.first = 0;
  list.last = 0;
  list.free_elems.len = 0;
}

Templ void
Clear( listwalloc_t<T>& list )
{
  list.len = 0;
  list.first = 0;
  list.last = 0;
  list.free_elems.len = 0;
}

Templ Inl listelem_t<T>*
_GetNewElem( listwalloc_t<T>& list )
{
  listelem_t<T>* elem;
  if( list.free_elems.len ) {
    elem = list.free_elems.mem[ list.free_elems.len - 1 ];
    RemBack( list.free_elems );
  } else {
    elem = AddPlist( listelem_t<T>, *list.elems, 1 );
  }
  AssertCrash( elem );
  return elem;
}

Templ Inl void
_InitialInsert( listwalloc_t<T>& list, listelem_t<T>* elem )
{
  AssertCrash( !list.len );
  elem->prev = 0;
  elem->next = 0;
  list.first = elem;
  list.last = elem;
  list.len = 1;
}

Templ Inl void
InsertBefore( listwalloc_t<T>& list, listelem_t<T>* elem, listelem_t<T>* place )
{
  AssertCrash( elem );
  if( !list.len ) {
    AssertCrash( !place );
    _InitialInsert( list, elem );
  } else {
    AssertCrash( list.len );
    AssertCrash( place );
    auto prev = place->prev;
    auto next = place;
    elem->prev = prev;
    elem->next = next;
    if( prev ) {
      prev->next = elem;
    }
    next->prev = elem;
    if( list.first == place ) {
      list.first = elem;
    }
    list.len += 1;
  }
  AssertCrash( elem->prev != elem  &&  elem->next != elem );
}

Templ Inl void
InsertAfter( listwalloc_t<T>& list, listelem_t<T>* elem, listelem_t<T>* place )
{
  AssertCrash( elem );
  if( !list.len ) {
    AssertCrash( !place );
    _InitialInsert( list, elem );
  } else {
    AssertCrash( list.len );
    AssertCrash( place );
    auto prev = place;
    auto next = place->next;
    elem->prev = prev;
    elem->next = next;
    prev->next = elem;
    if( next ) {
      next->prev = elem;
    }
    if( list.last == place ) {
      list.last = elem;
    }
    list.len += 1;
  }
  AssertCrash( elem->prev != elem  &&  elem->next != elem );
}

Templ Inl void
InsertFirst( listwalloc_t<T>& list, listelem_t<T>* elem )
{
  InsertBefore( list, elem, list.first );
}

Templ Inl void
InsertLast( listwalloc_t<T>& list, listelem_t<T>* elem )
{
  InsertAfter( list, elem, list.last );
}

Templ Inl listelem_t<T>*
AddBefore( listwalloc_t<T>& list, listelem_t<T>* place )
{
  auto elem = _GetNewElem( list );
  InsertBefore( list, elem, place );
  return elem;
}

Templ Inl listelem_t<T>*
AddAfter( listwalloc_t<T>& list, listelem_t<T>* place )
{
  auto elem = _GetNewElem( list );
  InsertAfter( list, elem, place );
  return elem;
}

Templ Inl listelem_t<T>*
AddFirst( listwalloc_t<T>& list )
{
  return AddBefore( list, list.first );
}

Templ Inl listelem_t<T>*
AddLast( listwalloc_t<T>& list )
{
  return AddAfter( list, list.last );
}

Templ Inl void
Rem( listwalloc_t<T>& list, listelem_t<T>* elem )
{
  AssertCrash( list.len );
  auto prev = elem->prev;
  auto next = elem->next;
  elem->next = 0;
  elem->prev = 0;
  if( prev ) {
    prev->next = next;
  }
  if( next ) {
    next->prev = prev;
  }
  if( list.first == elem ) {
    list.first = next;
  }
  if( list.last == elem ) {
    list.last = prev;
  }
  list.len -= 1;
}

Templ Inl void
RemFirst( listwalloc_t<T>& list )
{
  Rem( list, list.first );
}

Templ Inl void
RemLast( listwalloc_t<T>& list )
{
  Rem( list, list.last );
}

Templ Inl void
Reclaim( listwalloc_t<T>& list, listelem_t<T>* elem )
{
  if( list.free_elems.len < Capacity( list.free_elems ) ) {
    *AddBack( list.free_elems ) = elem;
  }
}




// meant to hold elements which are uniform in size, but the element size isn't known at compile-time.
struct
bytearray_t
{
  u8* mem;
  idx_t bytecapacity; // # of bytes mem can possibly hold.
  idx_t len; // # of elements in mem.
  idx_t byteelem; // # of bytes in one element.
};


Inl void
Zero( bytearray_t& bytearray )
{
  bytearray.mem = 0;
  bytearray.bytecapacity = 0;
  bytearray.len = 0;
  bytearray.byteelem = 0;
}


Inl idx_t
Capacity( bytearray_t& bytearray )
{
  idx_t capacity = bytearray.bytecapacity / bytearray.byteelem;
  return capacity;
}



Inl void
Alloc( bytearray_t& bytearray, idx_t nelems, idx_t elemsize )
{
  AssertCrash( nelems * elemsize <= 1ULL*1024*1024*1024 ); // You should use other data structures for large allocations!
  Zero( bytearray );
  bytearray.byteelem = elemsize;
  bytearray.bytecapacity = nelems * elemsize;
  bytearray.mem = MemHeapAlloc( u8, bytearray.bytecapacity );
  AssertCrash( ( bytearray.bytecapacity % bytearray.byteelem ) == 0 );
}

Inl void
Free( bytearray_t& bytearray )
{
  if( bytearray.mem ) {
    MemHeapFree( bytearray.mem );
  }
  Zero( bytearray );
}


#define ByteArrayElem( type, bytearray, idx ) \
  Cast( type*, bytearray.mem + bytearray.byteelem * idx )


Inl void
Clear( bytearray_t& bytearray )
{
  bytearray.len = 0;
}


Inl void
Reserve( bytearray_t& bytearray, idx_t enforce_capacity )
{
  AssertCrash( enforce_capacity * bytearray.byteelem <= 1ULL*1024*1024*1024 ); // You should use other data structures for large allocations!
  if( bytearray.bytecapacity < enforce_capacity ) {
    auto new_capacity = 2 * bytearray.bytecapacity;
    while( new_capacity < enforce_capacity ) {
      new_capacity *= 2;
    }
    bytearray.mem = MemHeapRealloc( u8, bytearray.mem, bytearray.bytecapacity, new_capacity );
    bytearray.bytecapacity = new_capacity;
  }
}


Inl void*
AddBack( bytearray_t& bytearray, idx_t nelems = 1 )
{
  Reserve( bytearray, bytearray.len + nelems );
  auto r = bytearray.mem + bytearray.byteelem * bytearray.len;
  bytearray.len += nelems;
  return r;
}



Inl void*
AddAt( bytearray_t& bytearray, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( idx <= bytearray.len );
  Reserve( bytearray, bytearray.len + nelems );
  if( idx < bytearray.len ) {
    auto nshift = bytearray.len - idx;
    Memmove(
      bytearray.mem + bytearray.byteelem * ( idx + nelems ),
      bytearray.mem + bytearray.byteelem * idx,
      bytearray.byteelem * nshift
      );
  }
  auto r = bytearray.mem + bytearray.byteelem * idx;
  bytearray.len += nelems;
  return r;
}


Inl void
RemBack( bytearray_t& bytearray, idx_t nelems = 1 )
{
  AssertCrash( nelems <= bytearray.len );
  bytearray.len -= nelems;
}


Inl void
RemAt( bytearray_t& bytearray, idx_t idx, idx_t nelems = 1 )
{
  AssertCrash( idx + nelems <= bytearray.len );
  if( idx + nelems < bytearray.len ) {
    Memmove(
      bytearray.mem + bytearray.byteelem * idx,
      bytearray.mem + bytearray.byteelem * ( idx + nelems ),
      bytearray.byteelem * ( bytearray.len - idx - nelems )
      );
  }
  bytearray.len -= nelems;
}




// TODO: add links between hashset elems so we can iterate nicely over all elems/assocs.

#define HASHSET_ELEM_EQUAL( name )   bool ( name )( void* elem0, void* elem1, idx_t elem_size )
typedef HASHSET_ELEM_EQUAL( *pfn_hashset_elem_equal_t );

#define HASHSET_ELEM_HASH( name )   idx_t ( name )( void* elem, idx_t elem_len )
typedef HASHSET_ELEM_HASH( *pfn_hashset_elem_hash_t );


struct
hashset_t
{
  bytearray_t elems;
  idx_t elem_data_len; // each hashed element has to be the same size, stored here.
  idx_t assoc_data_len; // each associated element has to be the same size, stored here.
  idx_t cardinality; // number of elems that have data currently.
  f32 expand_load_factor; // in ( 0, 1 ). will double capacity on Add when load factor goes beyond this threshold.
  pfn_hashset_elem_equal_t ElemEqual;
  pfn_hashset_elem_hash_t ElemHash;
};


// NOTE: only cast to this; you shouldn't ever instantiate one of these.
ALIGNTOIDX struct
hashset_elem_t
{
  idx_t hash_code;
  bool has_data; // NOTE: we rely on Memzero to set this to false in Init.
};

Inl void*
_GetElemData( hashset_t& set, hashset_elem_t* elem )
{
  void* r = Cast( u8*, elem ) + sizeof( hashset_elem_t );
  return r;
}

Inl void*
_GetAssocData( hashset_t& set, hashset_elem_t* elem )
{
  void* r = Cast( u8*, elem ) + sizeof( hashset_elem_t ) + set.elem_data_len;
  return r;
}


void
Init(
  hashset_t& set,
  idx_t capacity,
  idx_t elem_data_len,
  idx_t assoc_data_len,
  f32 expand_load_factor,
  pfn_hashset_elem_equal_t ElemEqual,
  pfn_hashset_elem_hash_t ElemHash
  )
{
  AssertCrash( ( expand_load_factor > 0 )  &  ( expand_load_factor <= 1 ) );

  idx_t elem_len = sizeof( hashset_elem_t ) + elem_data_len + assoc_data_len; // NOTE: change if hashset_elem_t ever changes.
  Alloc( set.elems, capacity, elem_len );

  Memzero( set.elems.mem, capacity * elem_len );
  set.elems.len = capacity;

  set.elem_data_len = elem_data_len;
  set.assoc_data_len = assoc_data_len;
  set.expand_load_factor = expand_load_factor;
  set.cardinality = 0;
  set.ElemEqual = ElemEqual;
  set.ElemHash = ElemHash;
}


void
Kill( hashset_t& set )
{
  Free( set.elems );
  set.elem_data_len = 0;
  set.assoc_data_len = 0;
  set.expand_load_factor = 0;
  set.cardinality = 0;
  set.ElemEqual = 0;
  set.ElemHash = 0;
}


void
Clear( hashset_t& set )
{
  set.cardinality = 0;
  ForLen( i, set.elems ) {
    auto& elem = *ByteArrayElem( hashset_elem_t, set.elems, i );
    elem.has_data = 0;
  }
}


static void
DoubleCapacity( hashset_t& old_set )
{
  hashset_t set;
  Init(
    set,
    2 * Capacity( old_set.elems ),
    old_set.elem_data_len,
    old_set.assoc_data_len,
    old_set.expand_load_factor,
    old_set.ElemEqual,
    old_set.ElemHash
    );

  ForLen( i, old_set.elems ) {
    auto old_elem = ByteArrayElem( hashset_elem_t, old_set.elems, i );
    if( old_elem->has_data ) {
      void* element = _GetElemData( old_set, old_elem );
      void* assoc = _GetAssocData( old_set, old_elem );

      idx_t hash_code = old_elem->hash_code;
      idx_t idx = hash_code % set.elems.len;
      idx_t nprobed = 0;
      while( nprobed < set.elems.len ) {

        auto elem = ByteArrayElem( hashset_elem_t, set.elems, idx );

        if( elem->has_data ) {
          if( elem->hash_code == hash_code ) {
            if( set.ElemEqual( _GetElemData( set, elem ), element, set.elem_data_len ) ) {
              UnreachableCrash();
            }
          }
          idx = ( idx + 1 ) % set.elems.len;
          nprobed += 1;

        } else {
          elem->hash_code = hash_code;
          Memmove( _GetElemData( set, elem ), element, set.elem_data_len );
          Memmove( _GetAssocData( set, elem ), assoc, set.assoc_data_len );
          elem->has_data = 1;
          set.cardinality += 1;
          break;
        }
      }
      AssertCrash( nprobed < set.elems.len );
    }
  }
  Kill( old_set );
  old_set = set;
}


void
Add(
  hashset_t& set,
  void* element,
  void* assoc,
  bool* already_there,
  void* assoc_already_there,
  bool overwrite_assoc_if_already_there
  )
{
  f32 load_factor = Cast( f32, set.cardinality + 1 ) / Cast( f32, set.elems.len );
  if( load_factor > set.expand_load_factor ) {
    DoubleCapacity( set );
  }

  idx_t hash_code = set.ElemHash( element, set.elem_data_len );
  idx_t idx = hash_code % set.elems.len;
  idx_t nprobed = 0;
  while( nprobed < set.elems.len ) {
    auto elem = ByteArrayElem( hashset_elem_t, set.elems, idx );
    if( elem->has_data ) {
      if( elem->hash_code == hash_code ) {
        if( set.ElemEqual( _GetElemData( set, elem ), element, set.elem_data_len ) ) {
          if( already_there ) {
            *already_there = 1;
          }
          if( assoc_already_there ) {
            Memmove( assoc_already_there, _GetAssocData( set, elem ), set.assoc_data_len );
          }
          if( overwrite_assoc_if_already_there ) {
            Memmove( _GetAssocData( set, elem ), assoc, set.assoc_data_len );
          }
          return;
        }
      }
      idx = ( idx + 1 ) % set.elems.len;
      nprobed += 1;

    } else {
      elem->hash_code = hash_code;
      elem->has_data = 1;
      set.cardinality += 1;
      Memmove( _GetElemData( set, elem ), element, set.elem_data_len );
      Memmove( _GetAssocData( set, elem ), assoc, set.assoc_data_len );
      if( already_there ) {
        *already_there = 0;
      }
      return;
    }
  }
  UnreachableCrash();
}


void
Lookup(
  hashset_t& set,
  void* element,
  bool* found,
  void* found_assoc
  )
{
  *found = 0;

  idx_t hash_code = set.ElemHash( element, set.elem_data_len );
  idx_t idx = hash_code % set.elems.len;

  idx_t nprobed = 0;
  while( nprobed < set.elems.len ) {
    auto elem = ByteArrayElem( hashset_elem_t, set.elems, idx );

    if( elem->has_data ) {
      if( hash_code == elem->hash_code ) {
        if( set.ElemEqual( _GetElemData( set, elem ), element, set.elem_data_len ) ) {
          *found = 1;
          if( found_assoc ) {
            Memmove( found_assoc, _GetAssocData( set, elem ), set.assoc_data_len );
          }
          break;
        }
      }
      idx = ( idx + 1 ) % set.elems.len;
      nprobed += 1;

    } else {
      break;
    }
  }
}


void
LookupRaw(
  hashset_t& set,
  void* element,
  bool* found,
  void** found_assoc
  )
{
  *found = 0;
  if( found_assoc ) {
    *found_assoc = 0;
  }

  idx_t hash_code = set.ElemHash( element, set.elem_data_len );
  idx_t idx = hash_code % set.elems.len;

  idx_t nprobed = 0;
  while( nprobed < set.elems.len ) {
    auto elem = ByteArrayElem( hashset_elem_t, set.elems, idx );

    if( elem->has_data ) {
      if( hash_code == elem->hash_code ) {
        if( set.ElemEqual( _GetElemData( set, elem ), element, set.elem_data_len ) ) {
          *found = 1;
          if( found_assoc ) {
            *found_assoc = _GetAssocData( set, elem );
          }
          break;
        }
      }
      idx = ( idx + 1 ) % set.elems.len;
      nprobed += 1;

    } else {
      break;
    }
  }
}


void
Remove(
  hashset_t& set,
  void* element,
  bool* found,
  void* found_assoc
  )
{
  *found = 0;

  idx_t hash_code = set.ElemHash( element, set.elem_data_len );
  idx_t idx = hash_code % set.elems.len;

  idx_t nprobed = 0;
  while( nprobed < set.elems.len ) {
    auto elem = ByteArrayElem( hashset_elem_t, set.elems, idx );

    if( elem->has_data ) {
      if( hash_code == elem->hash_code ) {
        if( set.ElemEqual( _GetElemData( set, elem ), element, set.elem_data_len ) ) {
          elem->has_data = 0;
          set.cardinality -= 1;
          *found = 1;
          if( found_assoc ) {
            Memmove( found_assoc, _GetAssocData( set, elem ), set.assoc_data_len );
          }
          break;
        }
      }
      idx = ( idx + 1 ) % set.elems.len;
      nprobed += 1;

    } else {
      break;
    }
  }
}










// ==========================================================================
//
// Default hash functions
//


Inl
HASHSET_ELEM_EQUAL( Equal_Memcmp )
{
  bool r = MemEqual( elem0, elem1, elem_size );
  return r;
}

Inl
HASHSET_ELEM_EQUAL( Equal_FirstU32 )
{
  AssertCrash( elem_size >= sizeof( u32 ) );
  auto& a = *Cast( u32*, elem0 );
  auto& b = *Cast( u32*, elem1 );
  bool r = ( a == b );
  return r;
}

Inl
HASHSET_ELEM_EQUAL( Equal_FirstU64 )
{
  AssertCrash( elem_size >= sizeof( u64 ) );
  auto& a = *Cast( u64*, elem0 );
  auto& b = *Cast( u64*, elem1 );
  bool r = ( a == b );
  return r;
}

Inl
HASHSET_ELEM_EQUAL( Equal_FirstIdx )
{
  AssertCrash( elem_size >= sizeof( idx_t ) );
  auto& a = *Cast( idx_t*, elem0 );
  auto& b = *Cast( idx_t*, elem1 );
  bool r = ( a == b );
  return r;
}

Inl
HASHSET_ELEM_EQUAL( Equal_SliceContents )
{
  AssertCrash( elem_size == sizeof( slice_t ) );
  auto a = Cast( slice_t*, elem0 );
  auto b = Cast( slice_t*, elem1 );
  bool r = MemEqual( a->mem, a->len, b->mem, b->len );
  return r;
}

Inl
HASHSET_ELEM_HASH( Hash_FirstU32 )
{
  AssertCrash( elem_len >= sizeof( u32 ) );
  auto a = *Cast( u32*, elem );
  idx_t r = 3 + 7 * a;
  return r;
}

Inl
HASHSET_ELEM_HASH( Hash_FirstU64 )
{
  AssertCrash( elem_len >= sizeof( u64 ) );
  auto a = *Cast( u64*, elem );
  idx_t r = Cast( idx_t, 3 + 7 * a );
  return r;
}

Inl
HASHSET_ELEM_HASH( Hash_FirstIdx )
{
  AssertCrash( elem_len >= sizeof( idx_t ) );
  idx_t a = *Cast( idx_t*, elem );
  idx_t r = 3 + 7 * a;
  return r;
}

Inl
HASHSET_ELEM_HASH( Hash_SliceContents )
{
  AssertCrash( elem_len == sizeof( slice_t ) );
  auto a = Cast( slice_t*, elem );
  idx_t r = StringHash( a->mem, a->len );
  return r;
}




// TODO: look at void funcs and see if we can return a idx_t for # u8s processed ?


Inl u8
CsToLower( u8 c )
{
  return IsUpper( c )  ?  ( c + 32 )  :  c;
}

Inl u8
CsToUpper( u8 c )
{
  return IsLower( c )  ?  ( c - 32 )  :  c;
}

Inl idx_t
CsLen( u8* src )
{
  idx_t len = 0;
  while( *src++ )
    ++len;
  return len;
}

Inl idx_t CsLen( const u8* src ) { return CsLen( Cast( u8*, src ) ); }

Inl idx_t
CsLen( u8* src_start, u8* src_end )
{
  sidx_t src_len = src_end - src_start;
  AssertCrash( src_len >= 0 );
  return Cast( idx_t, src_len );
}


Inl u8*
AllocCstr( u8* mem, idx_t len )
{
  auto r = MemHeapAlloc( u8, len + 1 );
  Memmove( r, mem, len );
  r[len] = 0;
  return r;
}

Inl u8*
AllocCstr( slice_t& str )
{
  return AllocCstr( ML( str ) );
}

Inl u8*
AllocCstr( string_t& str )
{
  return AllocCstr( ML( str ) );
}



Inl slice_t
SliceFromCStr( void* str )
{
  slice_t r;
  r.mem = Cast( u8*, str );
  r.len = CsLen( r.mem );
  return r;
}


Inl idx_t
CsCount( u8* src, idx_t src_len, u8 key )
{
  idx_t count = 0;
  For( i, 0, src_len ) {
    if( src[i] == key )
      ++count;
  }
  return count;
}


Inl bool
CsEquals( u8* a, idx_t a_len, u8* b, idx_t b_len, bool case_sens )
{
  if( b_len != a_len )
    return 0;
  if( case_sens ) {
    For( i, 0, a_len ) {
      if( a[i] != b[i] )
        return 0;
    }
  } else {
    For( i, 0, a_len ) {
      if( CsToLower( a[i] ) != CsToLower( b[i] ) )
        return 0;
    }
  }
  return 1;
}

Inl idx_t
CsCount( u8* src, idx_t src_len, u8* key, idx_t key_len )
{
  idx_t count = 0;
  if( src_len < key_len )
    return count;
  For( i, 0, src_len - key_len ) {
    if( CsEquals( &src[i], key_len, key, key_len, 1 ) )
      ++count;
  }
  return count;
}


Inl idx_t
CsReplace( u8* dst, idx_t dst_len, u8 key, u8 val )
{
  idx_t nreplaced = 0;
  For( i, 0, dst_len ) {
    if( dst[i] == key ) {
      dst[i] = val;
      ++nreplaced;
    }
  }
  return nreplaced;
}



Inl void
CsToLower( u8* dst, idx_t dst_len )
{
  For( i, 0, dst_len )
    dst[i] = CsToLower( dst[i] );
}

Inl void
CsToUpper( u8* dst, idx_t dst_len )
{
  For( i, 0, dst_len )
    dst[i] = CsToUpper( dst[i] );
}


Inl void
CsCopy( u8* dst, u8 src )
{
  dst[0] = src;
  dst[1] = 0;
}

Inl void
CsCopy( u8* dst, u8* src, idx_t src_len )
{
  For( i, 0, src_len )
    *dst++ = src[i];
  *dst = 0;
}

Inl idx_t
CsCopy( u8* dst, u8* src_start, u8* src_end )
{
  idx_t ncpy = 0;
  u8* c = src_start;
  while( c != src_end ) {
    *dst++ = *c++;
    ++ncpy;
  }
  *dst = 0;
  return ncpy;
}



Inl bool
CsIdxScanR( idx_t* dst, u8* src, idx_t src_len, idx_t idx, u8 key )
{
  For( i, idx, src_len ) {
    if( src[i] == key ) {
      *dst = i;
      return 1;
    }
  }
  return 0;
}

Inl bool
CsIdxScanR( idx_t* dst, u8* src, idx_t src_len, u8* key, idx_t key_len, bool case_sens, bool word_boundary )
{
  For( i, 0, src_len ) {
    if( CsEquals( src + i, MIN( src_len - i, key_len ), key, key_len, case_sens ) ) {
      bool found = 1;
      if( word_boundary ) {
        if( i > 0  &&  InWord( src[i - 1] ) ) {
          found = 0;
        }
        if( i + key_len < src_len  &&  InWord( src[i + key_len] ) ) {
          found = 0;
        }
      }
      if( found ) {
        *dst = i;
        return 1;
      }
    }
  }
  return 0;
}


Inl bool
CsIdxScanL( idx_t* dst, u8* src, idx_t src_len, idx_t idx, u8 key )
{
  idx_t i = idx + 1;
  while( i ) {
    --i;
    if( src[i] == key ) {
      *dst = i;
      return 1;
    }
  }
  return 0;
}

Inl bool
CsIdxScanL( idx_t* dst, u8* src, idx_t src_len, idx_t idx, u8* key, idx_t key_len, bool case_sens, bool word_boundary )
{
  ImplementCrash();
  return 0;
  //u8* find_rev = Rscan( src, src_len, key, key_len );
  //if( !find_rev )
  //  return 0;
  //ptrdiff_t diff_findrev_src = find_rev - src;
  //AssertCrash( diff_findrev_src >= 0 );
  //*dst = ( idx_t )diff_findrev_src;
  //return 1;
}

Inl u8*
CsScanR( u8* src, idx_t src_len, u8 key )
{
  For( i, 0, src_len ) {
    if( src[i] == key )
      return src + i;
  }
  return 0;
}

Inl u8*
CsScanR( u8* src, idx_t src_len, u8* key, idx_t key_len, bool case_sens, bool word_boundary )
{
  idx_t i;
  bool f = CsIdxScanR( &i, src, src_len, key, key_len, case_sens, word_boundary );
  return f  ?  src + i  :  0;
}


Inl u8*
CsScanL( u8* src, idx_t src_len, u8 key )
{
  idx_t i = src_len;
  while( i ) {
    --i;
    if( src[i] == key )
      return &src[i];
  }
  return 0;
}

Inl u8*
CsScanL( u8* src, idx_t src_len, u8* key, idx_t key_len )
{
  ImplementCrash();
  return 0;
  //u8* forw = Scan( src, src_len, key, key_len );
  //u8* last = 0;
  //while( forw ) {
  //  last = forw;
  //  forw = Scan( forw + 1, key );
  //}
  //return last;
}



Inl void
CsAddBack( u8* dst, idx_t dst_len, u8 src )
{
  CsCopy( &dst[dst_len], src );
}

Inl void
CsAddBack( u8* dst, idx_t dst_len, u8* src, idx_t src_len )
{
  CsCopy( &dst[dst_len], src, src_len );
}

Inl idx_t
CsAddBack( u8* dst, idx_t dst_len, u8* src_start, u8* src_end )
{
  return CsCopy( &dst[dst_len], src_start, src_end );
}



Inl void
CsAddFront( u8* dst, idx_t dst_len, u8 src )
{
  u8* d1 = &dst[ dst_len + 1 ];
  u8* d2 = &dst[ dst_len + 2 ];
  For( i, 0, dst_len + 1 )
    *--d2 = *--d1;
  *--d2 = src;
}

Inl void
CsAddFront( u8* dst, idx_t dst_len, u8* src, idx_t src_len )
{
  if( !src_len )
    return;
  u8* d1 = &dst[ dst_len + 1 ];
  u8* d2 = &dst[ dst_len + 1 + src_len ];
  For( i, 0, dst_len + 1 )
    *--d2 = *--d1;
  u8* c = &src[ src_len ];
  For( i, 0, src_len )
    *--d2 = *--c;
}

Inl idx_t
CsAddFront( u8* dst, idx_t dst_len, u8* src_start, u8* src_end )
{
  idx_t src_len = CsLen( src_start, src_end );
  if( !src_len )
    return 0;
  u8* d1 = &dst[ dst_len + 1 ];
  u8* d2 = &dst[ dst_len + 1 + src_len ];
  For( i, 0, dst_len + 1 )
    *--d2 = *--d1;
  u8* c = &src_start[ src_len ];
  For( i, 0, src_len )
    *--d2 = *--c;
  return src_len;
}


Inl void
CsAddAt( u8* dst, idx_t dst_len, idx_t idx, u8 src )
{
  AssertCrash( idx <= dst_len );
  u8* d1 = &dst[ dst_len + 1 ];
  u8* d2 = &dst[ dst_len + 2 ];
  idx_t nshift = dst_len - idx;
  For( i, 0, nshift + 1 )
    *--d2 = *--d1;
  *--d2 = src;
}

Inl void
CsAddAt( u8* dst, idx_t dst_len, idx_t idx, u8* src, idx_t src_len )
{
  ImplementCrash();
}


Inl void
CsRemBack( u8* dst, u8* src, idx_t src_len, idx_t nchars )
{
  AssertCrash( nchars <= src_len );
  if( dst ) {
    For( i, 0, nchars )
      dst[i] = src[ src_len - nchars + i ];
    dst[nchars] = 0;
  }
  src[ src_len - nchars ] = 0;
}


Inl idx_t
CsRemBackTo( u8* dst, u8* src, idx_t src_len, u8 delim )
{
  idx_t idx;
  if( !CsIdxScanL( &idx, src, src_len, src_len - 1, delim ) )
    idx = 0;
  idx_t nrem = src_len - idx;
  CsRemBack( dst, src, src_len, nrem );
  return nrem;
}

Inl void
CsRemBackTo( u8* dst, u8* src, idx_t src_len, u8* delim, idx_t delim_len )
{
  idx_t idx;
  if( !CsIdxScanL( &idx, src, src_len, src_len - 1, delim, delim_len, 1, 0 ) )
    idx = 0;
  idx_t nrem = src_len - idx;
  CsRemBack( dst, src, src_len, nrem );
}


Inl void
CsRemFront( u8* dst, u8* src, idx_t src_len, idx_t nchars )
{
  AssertCrash( nchars <= src_len );
  if( dst )
    CsCopy( dst, src, nchars );
  u8* c = &src[nchars];
  For( i, 0, src_len - nchars )
    *src++ = *c++;
  *src = 0;
}


Inl idx_t
CsRemFrontTo( u8* dst, u8* src, idx_t src_len, u8 delim )
{
  idx_t idx;
  if( !CsIdxScanR( &idx, src, src_len, 0, delim ) )
    idx = src_len;
  CsRemFront( dst, src, src_len, idx );
  return idx;
}

Inl void
CsRemFrontTo( u8* dst, u8* src, idx_t src_len, u8* delim, idx_t delim_len )
{
  idx_t idx;
  if( !CsIdxScanR( &idx, src, src_len, delim, delim_len, 1, 0 ) )
    idx = src_len;
  CsRemFront( dst, src, src_len, idx );
}


Inl void
CsRemAt( u8* dst, u8* src, idx_t src_len, idx_t idx, idx_t nchars )
{
  AssertCrash( idx < src_len );
  CsRemFront( dst, &src[idx], src_len - idx, nchars );
}



Inl idx_t
CsRemWhitespaceL( u8* dst, idx_t dst_len )
{
  idx_t nrem;
  for( nrem = 0;  nrem < dst_len;  ++nrem ) {
    if( !isspace( dst[nrem] ) )
      break;
  }
  if( !nrem )
    return 0;
  CsRemFront( 0, dst, dst_len, nrem );
  return nrem;
}

Inl idx_t
CsRemWhitespaceR( u8* dst, idx_t dst_len )
{
  idx_t nrem;
  for( nrem = 0;  nrem < dst_len;  ++nrem ) {
    if( !isspace( dst[ dst_len - nrem - 1 ] ) )
      break;
  }
  if( !nrem )
    return 0;
  CsRemBack( 0, dst, dst_len, nrem );
  return nrem;
}


Inl void
CsFrom_f64( u8* dst, idx_t dst_len, idx_t* dst_size, f64 src )
{
  *dst_size = sprintf_s( Cast( char*, dst ), dst_len, "%.14f", src );
}

Inl void
CsFrom_f32( u8* dst, idx_t dst_len, idx_t* dst_size, f32 src )
{
  *dst_size = sprintf_s( Cast( char*, dst ), dst_len, "%.7f", src );
}


Inl f64
CsTo_f64( u8* src, idx_t src_len )
{
  return Cast( f64, atof( Cast( char*, src ) ) );
}

Inl f32
CsTo_f32( u8* src, idx_t src_len )
{
  return Cast( f32, atof( Cast( char*, src ) ) );
}







#if 0 // TODO: use tls temp.
Inl void
Format( u8* dst, idx_t dstlen, u8* format, idx_t formatlen, ... )
{
  ImplementCrash();

  va_list args;
  va_start( args, formatlen );

  idx_t dstpos = 0;
  idx_t pos = 0;

  while( pos < formatlen ) {
    if( pos + 2 < formatlen ) {
      if( format[pos + 0] == '|'  &&  format[pos + 1] == '|' ) {
        if( dstpos < dstlen ) {
          dst[dstpos++] = '|';
          pos += 2;
        } else {
          break;
        }
      }
      if( format[pos + 0] == '|' ) {

      }
    }
  }

  va_end( args );
}
#endif

string_t
AllocString( void* cstr ... )
{
  string_t str;
  Alloc( str, MAX( 32768, 2 * CsLen( Str( cstr ) ) ) );

  va_list args;
  va_start( args, cstr );
  str.len = vsprintf_s( // TODO: stop using CRT version.
    Cast( char* const, str.mem ),
    str.len,
    Cast( const char* const, cstr ),
    args
    );
  va_end( args );

  return str;
}



Templ Inl bool
CsFromIntegerU(
  u8* dst,
  idx_t dst_len,
  idx_t* dst_size,
  T src,
  bool use_separator = 0,
  u8 separator = ',',
  u8 separator_count = 3,
  u8 radix = 10,
  u8* digitmap = Str( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" )
  )
{
  idx_t out_len = 0;
  u8 sep_count = 0;
  auto write = dst;
  if( !src ) {
    if( out_len + 1 > dst_len ) {
      return 0;
    }
    *write++ = digitmap[0];
    out_len += 1;
  } else {
    while( src ) {
      auto digit = Cast( u8, src % radix );
      src /= radix;
      if( use_separator ) {
        if( sep_count == separator_count ) {
          sep_count = 0;
          if( out_len + 1 > dst_len ) {
            return 0;
          }
          *write++ = separator;
          out_len += 1;
        }
        sep_count += 1;
      }
      if( out_len + 1 > dst_len ) {
        return 0;
      }
      *write++ = digitmap[digit];
      out_len += 1;
    }
    MemReverse( dst, out_len );
  }
  if( out_len + 1 > dst_len ) {
    return 0;
  }
  *write++ = 0;
  *dst_size = out_len;
  return 1;
}

Templ Inl bool
CsFromIntegerS(
  u8* dst,
  idx_t dst_len,
  idx_t* dst_size,
  T src,
  bool use_separator = 0,
  u8 separator = ',',
  u8 separator_count = 3,
  u8 radix = 10,
  u8* digitmap = Str( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" )
  )
{
  idx_t out_len = 0;
  u8 sep_count = 0;
  auto write = dst;
  if( !src ) {
    if( out_len + 1 > dst_len ) {
      return 0;
    }
    *write++ = digitmap[0];
    out_len += 1;
  } else {
    bool write_negative = ( src < 0 );
    while( src ) {
      auto digit = ABS( Cast( s8, src % radix ) );
      src /= radix;
      if( use_separator ) {
        if( sep_count == separator_count ) {
          sep_count = 0;
          if( out_len + 1 > dst_len ) {
            return 0;
          }
          *write++ = separator;
          out_len += 1;
        }
        sep_count += 1;
      }
      if( out_len + 1 > dst_len ) {
        return 0;
      }
      *write++ = digitmap[digit];
      out_len += 1;
    }
    if( write_negative ) {
      if( out_len + 1 > dst_len ) {
        return 0;
      }
      *write++ = '-';
      out_len += 1;
    }
    MemReverse( dst, out_len );
  }
  if( out_len + 1 > dst_len ) {
    return 0;
  }
  *write++ = 0;
  *dst_size = out_len;
  return 1;
}


Templ Inl T
CsToIntegerU(
  u8* src,
  idx_t src_len,
  u8 ignore = ',',
  u8 radix = 10,
  u8* digitmap = Str( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" )
  )
{
  T dst = 0;
  T digit_factor = 1;
  ReverseFor( i, 0, src_len ) {
    auto c = src[i];
    if( ignore  &&  c == ignore ) {
      continue;
    }
    // TODO: extended mapping.
    AssertCrash( '0' <= c  &&  c <= '9' );
    T digit = c - '0';
    dst += digit * digit_factor;
    digit_factor *= radix;
  }
  return dst;
}

Templ Inl T
CsToIntegerS(
  u8* src,
  idx_t src_len,
  u8 ignore = ',',
  u8 radix = 10,
  u8* digitmap = Str( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" )
  )
{
  T dst = 0;
  T digit_factor = 1;
  ReverseFor( i, 0, src_len ) {
    auto c = src[i];
    if( ignore  &&  c == ignore ) {
      continue;
    }
    if( !i  &&  c == '-' ) {
      dst *= -1;
      continue;
    }
    // TODO: extended mapping.
    AssertCrash( '0' <= c  &&  c <= '9' );
    T digit = c - '0';
    dst += digit * digit_factor;
    digit_factor *= radix;
  }
  return dst;
}

constant f32 g_f32_from_digit[] = {
  0.0f,
  1.0f,
  2.0f,
  3.0f,
  4.0f,
  5.0f,
  6.0f,
  7.0f,
  8.0f,
  9.0f,
};
#if 0
Inl bool
CsToFloat32(
  u8* src,
  idx_t src_len,
  f32& dst
  )
{
  dst = 0;
  if( !( src_len || IsNumber( *src ) || *src == '.' || *src == '-' || *src == '+' || *src == 'N' || *src == 'I' ) ) {
    return 0;
  }
  bool negative = ( *src == '-' );
  if( *src == '-' || *src == '+' ) {
    src += 1;
    src_len -= 1;
  }
  if( !( src_len || IsNumber( *src ) || *src == '.' || *src == 'N' || *src == 'I' ) ) {
    return 0;
  }
  if( CsEquals( src, src_len, Str( "NAN" ), 3, 1 ) ) {
    dst = ( negative  ?  -NAN  :  NAN );
    return 1;
  } elif( CsEquals( src, src_len, Str( "INF" ), 3, 1 ) ) {
    dst = ( negative  ?  -INFINITY  :  INFINITY );
    return 1;
  }
  if( !( IsNumber( *src ) || *src == '.' ) ) {
    return 0;
  }
  // TODO: only do one iteration.
  auto pos_exponent = src_len;
  For( i, 0, src_len ) {
    if( src[i] == 'e' || src[i] == 'E' ) {
      pos_exponent = i;
      break;
    }
  }
  f32 digit_factor = 1.0f; // TODO: ordered table, so we don't accumulate errors.
  auto pos_decimal = pos_exponent;
  For( i, 0, pos_exponent ) {
    if( src[i] == '.' ) {
      pos_decimal = i;
      break;
    }
    if( i ) {
      digit_factor *= 10.0f;
    }
  }
  kahan32_t d = {};
  For( i, 0, pos_exponent ) {
    auto half = src[i];
    if( half == '.' ) {
      continue;
    }
    if( !IsNumber( half ) ) {
      return 0;
    }
    auto digit = g_f32_from_digit[ AsNumber( half ) ];
    Add( d, digit * digit_factor );
    digit_factor *= 0.1f;
  }
  src += pos_exponent;
  src_len -= pos_exponent;
  if( src_len ) {
    if( !( *src == 'e' || *src == 'E' ) ) {
      return 0;
    }
    src += 1;
    src_len -= 1;
    if( !( src_len || IsNumber( *src ) || *src == '-' || *src == '+' ) ) {
      return 0;
    }
    bool exp_negative = ( *src == '-' );
    if( *src == '-' || *src == '+' ) {
      src += 1;
      src_len -= 1;
    }
    if( !( src_len || IsNumber( *src ) ) ) {
      return 0;
    }
    auto exp = g_f32_from_digit[ AsNumber( *src ) ];
    while( src_len ) {
      src += 1;
      src_len -= 1;
      if( src_len ) {
        if( !IsNumber( *src ) ) {
          return 0;
        }
        exp *= 10.0f;
        exp += g_f32_from_digit[ AsNumber( *src ) ];
      }
    }
    exp = ( exp_negative  ?  -exp  :  exp );
    d.sum *= Pow( 10.0f, exp );
  }
  dst = d.sum;
  return 1;
}
#endif
#if 0
-149  1.4012984643248170709237295832899e-45
-148  2.8025969286496341418474591665798e-45
-147  5.6051938572992682836949183331597e-45
-146
-145
-144
-143
-142
 -25
 -24  -8   5.9604644775390625
 -23  -7   1.1920928955078125
 -22  -7   2.384185791015625
 -21  -7   4.76837158203125
 -20  -7   9.5367431640625
 -19  -6   1.9073486328125
 -18  -6   3.814697265625
 -17  -6   7.62939453125
 -16  -5   1.52587890625
 -15  -5   3.0517578125
 -14  -5   6.103515625
 -13  -4   1.220703125
 -12  -4   2.44140625
 -11  -4   4.8828125
 -10  -4   9.765625
  -9  -3   1.953125
  -8  -3   3.90625
  -7  -3   7.8125
  -6  -2   1.5625
  -5  -2   3.125
  -4  -2   6.25
  -3  -1   1.25
  -2  -1   2.5
  -1  -1   5
   0   0   1
   1   0   2
   2   0   4
   3   0   8
   4   1   1.6
   5   1   3.2
   6   1   6.4
   7   2   1.28
   8   2   2.56
   9   2   5.12
  10   3   1.024
  11   3   2.048
  12   3   4.096
  13   3   8.192
  14   4   1.6384
  15   4   3.2768
  16   4   6.5536
  17   5   1.31072
  18   5   2.62144
  19   5   5.24288
  10   6   1.048576
  11   6   2.097152
  12   6   4.194304
  13   6   8.388608
  14   7   1.6777216
  15   7   3.3554432
  16   7   6.7108864
  17   8   1.34217728
  18   8   2.68435456
  19   8   5.36870912
  20   9   1.073741824
  21   9   2.147483648
  22   9   4.294967296
  23   9   8.589934592
  24  10   1.7179869184
  25  10   3.4359738368
  26  10   6.8719476736
  27  11   1.37438953472
  28  11   2.74877906944
  29  11   5.49755813888

64
63        1
61       11
57      111
49     1111
33    11111
1    111111
0   1000000


63
62        1
60       11
56      111
48     1111
32    11111
0    111111


62
61        1
59       11
55      111
47     1111
31    11111
15   101111
7    110111
3    111011
1    111101
0    111110

#endif


struct
num2_t
{
  array_t<u64> bins;
};

Inl void
Init( num2_t& num, idx_t size )
{
  Alloc( num.bins, size );
}

Inl void
Kill( num2_t& num )
{
  Free( num.bins );
}

Inl void
TrimLeadingZeros( num2_t& dst )
{
  idx_t leading_zeros = 0;
  ReverseFor( i, 0, dst.bins.len ) {
    if( dst.bins.mem[i] != 0 ) {
      break;
    }
    leading_zeros += 1;
  }
  dst.bins.len -= leading_zeros;
}

Inl bool
IsZero( num2_t& dst )
{
  return !dst.bins.len;
}

#if 0
Inl void
Add( num2_t& dst, u64 b )
{
  if( !dst.bins.len ) {
    *AddBack( dst.bins ) = b;
  } else {
    u8 carry = 0;
    idx_t bin = 0;
    do {
      if( bin >= dst.bins.len ) {
        *AddBack( dst.bins ) = 0;
      }
      carry = _addcarry_u64( carry, dst.bins.mem[bin], b, dst.bins.mem + bin );
      b = 0;
      bin += 1;
    } while( carry );
  }
}
#endif

// dst += 1 << index
Inl void
AddBit( num2_t& dst, u64 index )
{
  u64 bin = index / 64;
  u64 bit = index % 64;
  if( bin >= dst.bins.len ) {
    auto nbins = bin - dst.bins.len + 1;
    AssertCrash( nbins <= MAX_idx );
    Memzero( AddBack( dst.bins, Cast( idx_t, nbins ) ), Cast( idx_t, nbins ) * sizeof( u64 ) );
  }
  u8 carry = 0;
  u64 b = ( 1ULL << bit );
  do {
    if( bin >= dst.bins.len ) {
      *AddBack( dst.bins ) = 0;
    }
    carry = _addcarry_u64( carry, dst.bins.mem[bin], b, dst.bins.mem + bin );
    b = 0;
    bin += 1;
  } while( carry );
}

// dst[index] = value
Inl void
SetBit( num2_t& dst, u64 index, bool value )
{
  u64 bin = index / 64;
  u64 bit = index % 64;
  if( value ) {
    if( bin >= dst.bins.len ) {
      auto nbins = bin - dst.bins.len + 1;
      AssertCrash( nbins <= MAX_idx );
      Memzero( AddBack( dst.bins, Cast( idx_t, nbins ) ), Cast( idx_t, nbins ) * sizeof( u64 ) );
    }
    dst.bins.mem[bin] |= ( 1ULL << bit );
  } else {
    if( bin < dst.bins.len ) {
      dst.bins.mem[bin] &= ~( 1ULL << bit );
    }
  }
}

Inl bool
GetBit( num2_t& src, u64 index ) // TODO: not optimal
{
  u64 bin = index / 64;
  u64 bit = index % 64;
  if( bin >= src.bins.len ) {
    return 0;
  }
  return ( src.bins.mem[bin] >> bit ) & 1;
}

struct
num_t
{
  array_t<u8> digits; // index 0 is 10s place, index 1 is 100s place, etc.
  s64 exp;
  bool positive;
};

Inl void
Init( num_t& num, idx_t size )
{
  Alloc( num.digits, size );
  num.exp = 0;
  num.positive = 1;
}

Inl void
Kill( num_t& num )
{
  Free( num.digits );
}

Inl void
CopyNum( num_t& dst, num_t& src )
{
  Copy( dst.digits, src.digits );
  dst.exp = src.exp;
  dst.positive = src.positive;
}

Inl bool
IsZero( num_t& num )
{
  return ( !num.digits.len );
}

Inl void
TrimLeadingZeros( num_t& dst )
{
  idx_t leading_zeros = 0;
  ReverseFor( i, 0, dst.digits.len ) {
    if( dst.digits.mem[i] != 0 ) {
      break;
    }
    leading_zeros += 1;
  }
  AssertCrash( dst.digits.len >= leading_zeros );
  dst.digits.len -= leading_zeros;
}

Inl void
IfAllZerosSetToZero( num_t& dst )
{
  bool nonzero = 0;
  ForLen( i, dst.digits ) {
    if( dst.digits.mem[i] != 0 ) {
      nonzero = 1;
      break;
    }
  }
  if( !nonzero ) {
    dst.digits.len = 0;
    dst.positive = 1;
  }
}

Inl void
Add( num_t& a, num_t& b, num_t& dst )
{
  auto len_a = Cast( s64, a.digits.len );
  auto len_b = Cast( s64, b.digits.len );
  dst.digits.len = 0;
  s64 exp = MIN( a.exp, b.exp );
  s64 nplaces = MAX( len_a + a.exp, len_b + b.exp ) - exp;
  dst.exp = exp;
  s8 carry = 0;
  auto place_a = exp - a.exp;
  auto place_b = exp - b.exp;
  Fori( s64, i, 0, nplaces ) {
    s8 digit_a = ( 0 <= place_a && place_a < len_a )  ?  Cast( s8, a.digits.mem[ place_a ] )  :  0;
    s8 digit_b = ( 0 <= place_b && place_b < len_b )  ?  Cast( s8, b.digits.mem[ place_b ] )  :  0;
    place_a += 1;
    place_b += 1;
    s8 extra_carry = 0;
    if( a.positive  &&  b.positive ) {
    } elif( !a.positive  &&  !b.positive ) {
      digit_a = -digit_a;
      digit_b = -digit_b;
    } elif( !a.positive  &&  b.positive ) {
      digit_a = -digit_a;
      digit_a -= 10;
      extra_carry = 1;
    } else {
      digit_b = -digit_b;
      digit_a += 10;
      extra_carry = -1;
    }
//    digit_a = a.positive  ?  digit_a  :  -digit_a;
//    digit_b = b.positive  ?  digit_b  :  -digit_b;
//    if( ABS( digit_a ) < ABS( digit_b ) ) {
//      digit_a += 10;
//      extra_carry = -1;
//    }
    s8 sum = digit_a + digit_b + carry;
    s8 digit;
    if( sum < 0 ) {
      carry = ( sum <= -10 )  ?  -1  :  0;
      digit = ( sum <= -10 )  ?  sum + 10  :  sum;
    } else {
      carry = ( sum >= 10 )  ?  1  :  0;
      digit = ( sum >= 10 )  ?  sum - 10  :  sum;
    }
    carry += extra_carry;
    *AddBack( dst.digits ) = Cast( u8, ABS( digit ) );
    dst.positive = ( sum >= 0 );
  }
  if( carry ) {
    *AddBack( dst.digits ) = 1;
    dst.positive = ( carry >= 0 );
  } else {
    TrimLeadingZeros( dst );
  }
}

Inl void
Mul( num_t& a, u8 b, bool b_positive, num_t& dst )
{
  dst.digits.len = 0;
  switch( b ) {
    case 0: {
      dst.exp = 0;
    } break;
    case 1: {
      Copy( dst.digits, a.digits );
      dst.exp = a.exp;
    } break;
    default: {
      Reserve( dst.digits, a.digits.len + 1 );
      dst.exp = a.exp;
      u8 carry = 0;
      For( i, 0, a.digits.len ) {
        u8 prod = b * a.digits.mem[i] + carry;
        carry = prod / 10;
        u8 digit = prod % 10;
        *AddBack( dst.digits ) = digit;
      }
      if( carry ) {
        *AddBack( dst.digits ) = carry;
      } else {
        TrimLeadingZeros( dst );
      }
    } break;
  }
  dst.positive = ( a.positive == b_positive );
}

Inl void
Mul( num_t& a, num_t& b, num_t& tmp, num_t& tmp2, num_t& dst )
{
  dst.digits.len = 0;
  dst.exp = 0;
  For( i, 0, a.digits.len ) {
    Mul( b, a.digits.mem[i], a.positive, tmp ); // TODO: only 10 possible results in this loop; cache them!
    tmp.exp += i;
    CopyNum( tmp2, dst ); // TODO: add in place
    Add( tmp, tmp2, dst );
  }
  dst.exp += a.exp;
}

Inl bool
CsFromFloat32(
  u8* dst,
  idx_t dst_len,
  idx_t* dst_size,
  f32 src
  )
{
  auto srcu = *Cast( u32*, &src );
  auto sign = srcu >> 31u;
  u8 rawexpu = Cast( u8, srcu >> 23u );
  auto f = ( 1 << 23u ) | ( srcu & AllOnes( 23u ) );

  idx_t size = 0;

  if( sign ) {
    if( 1 > dst_len ) {
      return 0;
    }
    *dst++ = '-';
    dst_len -= 1;
    size += 1;
  }

  if( !rawexpu ) { // zero, denormals
    if( f ) { // denormals
      UnreachableCrash();
      return 0;
    } else { // zero
      if( 1 > dst_len ) {
        return 0;
      }
      *dst++ = '0';
      dst_len -= 1;
      size += 1;
    }
  } elif( rawexpu == AllOnes( 8 ) ) { // inf, nan
    if( 3 > dst_len ) {
      return 0;
    }
    auto str = f  ?  Str( "nan" )  :  Str( "inf" );
    Memmove( dst, str, 3 );
    dst += 3;
    dst_len -= 3;
    size += 3;
  } else {
    s8 rawexp = ( rawexpu >= 127 )  ?  Cast( s8, rawexpu - 127 )  :  Cast( s8, rawexpu ) - 127;

    num_t a, b, c, d, e;
    Init( a, 100 );
    Init( b, 100 );
    Init( c, 100 );
    Init( d, 100 );
    Init( e, 100 );

    num_t sum;
    Init( sum, 100 );
    Fori( s8, i, 0, 24 ) {
      if( f & ( 1 << i ) ) {
        auto exp = rawexp + i - 23;

        // TODO: cache powers of 2

        // a = 2 ^ exp
        a.exp = 0;
        a.positive = 1;
        a.digits.len = 0;
        *AddBack( a.digits ) = 1;

        if( exp > 0 ) {
          b.exp = 0;
          b.positive = 1;
          b.digits.len = 0;
          *AddBack( b.digits ) = 2;

        } elif( exp < 0 ) {
          b.exp = -1;
          b.positive = 1;
          b.digits.len = 0;
          *AddBack( b.digits ) = 5;
        }
        idx_t expu = ABS( exp );
        For( j, 0, expu ) {
          Mul( a, b, c, d, e ); // TODO: mul in place
          CopyNum( a, e );
        }

        // sum += 2 ^ exp
        CopyNum( c, sum ); // TODO: add in place
        Add( a, c, sum );
      }
    }

    // TODO: limit digits length.

    if( sum.digits.len > dst_len ) {
      Kill( a );
      Kill( b );
      Kill( c );
      Kill( d );
      Kill( e );
      Kill( sum );
      return 0;
    }
    ReverseFor( i, 0, sum.digits.len ) {
      *dst++ = '0' + sum.digits.mem[i];
    }
    dst_len -= sum.digits.len;
    size += sum.digits.len;

    if( 1 > dst_len ) {
      Kill( a );
      Kill( b );
      Kill( c );
      Kill( d );
      Kill( e );
      Kill( sum );
      return 0;
    }
    *dst++ = 'e';
    dst_len -= 1;
    size += 1;

    idx_t exp_size = 0;
    if( !CsFromIntegerS( dst, dst_len, &exp_size, sum.exp ) ) {
      Kill( a );
      Kill( b );
      Kill( c );
      Kill( d );
      Kill( e );
      Kill( sum );
      return 0;
    }
    dst += exp_size;
    dst_len -= exp_size;
    size += exp_size;

    *dst_size = size;

    Kill( a );
    Kill( b );
    Kill( c );
    Kill( d );
    Kill( e );
    Kill( sum );
  }

  if( 1 > dst_len ) {
    return 0;
  }
  *dst++ = 0;
  dst_len -= 1;
  size += 1;

  return 1;
}

Inl bool
CsToFloat32(
  u8* src,
  idx_t src_len,
  f32& dst
  )
{
  // TODO: full input verification.

  dst = 0;
  if( !( src_len || IsNumber( *src ) || *src == '.' || *src == '-' || *src == '+' || *src == 'n' || *src == 'i' ) ) {
    return 0;
  }
  bool negative = ( *src == '-' );
  if( *src == '-' || *src == '+' ) {
    src += 1;
    src_len -= 1;
  }
  if( !( src_len || IsNumber( *src ) || *src == '.' || *src == 'n' || *src == 'i' ) ) {
    return 0;
  }
  if( CsEquals( src, src_len, Str( "nan" ), 3, 1 ) ) {
    dst = ( negative  ?  -NAN  :  NAN );
    return 1;
  } elif( CsEquals( src, src_len, Str( "inf" ), 3, 1 ) ) {
    dst = ( negative  ?  -INFINITY  :  INFINITY );
    return 1;
  }
  if( !( IsNumber( *src ) || *src == '.' ) ) {
    return 0;
  }
  auto pos_decimal = src_len;
  auto pos_exponent = src_len;
  For( i, 0, src_len ) {
    if( src[i] == '.' ) {
      pos_decimal = i;
      continue;
    }
    if( src[i] == 'e' || src[i] == 'E' ) {
      pos_exponent = i;
      continue;
    }
  }
  s16 exp = 0;
  if( pos_exponent != src_len ) {
    exp = CsToIntegerS<s16>( src + pos_exponent + 1, src_len - pos_exponent - 1, 0 );
  }

  // TODO: exp bounds!

  num_t intnum;
  Init( intnum, 100 );
  num_t fracnum;
  Init( fracnum, 100 );

  s64 pos_abs = Cast( s64, pos_decimal ) + exp;
  if( exp >= 0 ) {
    // 01234567
    // 123.45e4
    // 1234500
    // pos_decimal = 3
    // exp = 4
    // pos_abs = 7

    // 012345678
    // 12.3456e1
    // 123.456
    // pos_decimal = 2
    // exp = 1
    // pos_abs = 3

    ReverseFori( s64, i, 0, pos_abs + 1 ) {
      if( i >= Cast( s64, pos_exponent ) ) {
        *AddBack( intnum.digits ) = 0;
      } elif( i == Cast( s64, pos_decimal ) ) {
        continue;
      } else {
        *AddBack( intnum.digits ) = AsNumber( src[i] );
      }
    }
    ReverseFori( s64, i, pos_abs + 1, Cast( s64, pos_exponent ) ) {
      *AddBack( fracnum.digits ) = AsNumber( src[i] );
    }
  } else {
    // 0123456789
    // 1234.56e-2
    // 12.3456
    // pos_decimal = 4
    // exp = -2
    // pos_abs = 2

    // 0123456789
    // 12.345e-4
    // 0.0012345
    // pos_decimal = 2
    // exp = -4
    // pos_abs = -2

    ReverseFori( s64, i, 0, pos_abs + 1 ) {
      *AddBack( intnum.digits ) = AsNumber( src[i] );
    }
    ReverseFori( s64, i, pos_abs, Cast( s64, pos_exponent ) ) {
      if( i < 0 ) {
        *AddBack( fracnum.digits ) = 0;
      } elif( i == Cast( s64, pos_decimal ) ) {
        continue;
      } else {
        *AddBack( fracnum.digits ) = AsNumber( src[i] );
      }
    }
  }
  TrimLeadingZeros( intnum );
  AssertCrash( intnum.exp == 0 ); // must be 0 for algorithm to work!
  fracnum.exp = -Cast( s64, fracnum.digits.len );
  TrimLeadingZeros( fracnum );

  num_t neg_pow2, half, two, tmp0, tmp1, tmp2, tmp3;
  Init( neg_pow2, 100 );
  Init( half, 100 );
  Init( two, 100 );
  Init( tmp0, 100 );
  Init( tmp1, 100 );
  Init( tmp2, 100 );
  Init( tmp3, 100 );
  *AddBack( neg_pow2.digits ) = 1;
  neg_pow2.positive = 0;
  *AddBack( half.digits ) = 5;
  half.exp = -1;
  *AddBack( two.digits ) = 2;
  num2_t r;
  Init( r, 10 );
  u64 bit = 0;
  bool exactly_pow2 = 0;

  if( !IsZero( intnum ) ) {
    Forever {
      // we'll keep doubling neg_pow2 until we hit or go past our intnum.
      // this lets us determine the highest-set bit.
      // TODO: something smarter / quicker.
      Add( intnum, neg_pow2, tmp0 );
      if( IsZero( tmp0 ) ) {
        // our intnum is exactly a power of 2, so set that bit and quit.
        SetBit( r, bit, 1 );
        exactly_pow2 = 1;
        break;
      }
      if( !tmp0.positive ) {
        // we've gone past our intnum, so rollback one iter.
        SetBit( r, bit - 1, 1 );
        CopyNum( intnum, tmp3 );
        break;
      }
      CopyNum( tmp3, tmp0 ); // save for rollback
      Mul( neg_pow2, two, tmp0, tmp1, tmp2 );
      CopyNum( neg_pow2, tmp2 );
      bit += 1;
    }
    // now we know the high bit, and we've subtracted that value from our intnum.
    // walk down from the high bit, subtracting values from our intnum and setting bits.
    // TODO: don't recompute powers of 2!
    while( !exactly_pow2 ) {
      bit -= 1;
      if( !bit ) {
        // early exit for final bit.
        SetBit( r, bit, 1 );
        exactly_pow2 = 1;
        break;
      }
      Mul( neg_pow2, half, tmp0, tmp1, tmp2 );
      Add( intnum, tmp2, tmp0 );
      CopyNum( neg_pow2, tmp2 );
      if( IsZero( tmp0 ) ) {
        // our intnum is exactly a power of 2, so set that bit and quit.
        SetBit( r, bit, 1 );
        exactly_pow2 = 1;
        break;
      }
      if( !tmp0.positive ) {
        // keep walking down.
        continue;
      }
      CopyNum( intnum, tmp0 );
      SetBit( r, bit, 1 );
    }
    TrimLeadingZeros( r );
  }

  // TODO: we set the 24th bit of 'frac', even though we mask it out when forming the f32.
  u8 expu = 0;
  u32 frac = 0;

  u64 intexp = 0;
  if( r.bins.len ) {
    u64 leadingbitindex = 63 - _lzcnt_u64( r.bins.mem[ r.bins.len - 1 ] );
    intexp = leadingbitindex + 64 * ( r.bins.len - 1 );
    For( i, 0, MIN( 23, intexp ) + 1 ) {
      frac |= ( GetBit( r, intexp - i ) << ( 23 - i ) );
    }
  }

  // digits after the decimal place!
  // TODO: round to nearest ULP, instead of truncating at 23 bits.
  u32 frac2 = 0;
  For( i, intexp + 1, 23 ) {
    Mul( fracnum, two, tmp0, tmp1, tmp2 );
    CopyNum( fracnum, tmp2 );

    // throw away the integral part of the number.
    bool fracbit = 0;
    bool quit = 0;
    if( fracnum.exp >= 0 ) {
      fracnum.digits.len = 0;
      fracnum.positive = 1;
      fracbit = 1;
      quit = 1;
    } else {
      // 12345e-3  ->  12.345
      Fori( s64, j, -fracnum.exp, Cast( s64, fracnum.digits.len ) ) {
        if( fracnum.digits.mem[j] != 0 ) {
          fracbit = 1;
          fracnum.digits.len = Cast( idx_t, -fracnum.exp );
        }
      }
    }

    frac2 |= fracbit << ( 23 - i );
    if( quit ) {
      break;
    }
  }

  // add integral part and fractional part.
  frac += frac2;

  if( frac != 0 ) {
    expu = Cast( u8, intexp + 127 );

    // TODO: this left shift is causing us to lose some precision.
    //
    while( !( frac & ( 1 << 23 ) ) ) {
      frac = frac << 1;
      expu -= 1;
    }
  }


  Kill( r );
  Kill( tmp3 );
  Kill( tmp2 );
  Kill( tmp1 );
  Kill( tmp0 );
  Kill( two );
  Kill( half );
  Kill( neg_pow2 );
  Kill( intnum );
  Kill( fracnum );

  auto& dstu = *Cast( u32*, &dst );
  dstu |= ( expu << 23u );
  dstu |= ( frac & AllOnes( 23u ) );
  dstu |= ( negative << 31u );

  return 1;
}




Inl bool
CsFrom_u64( u8* dst, idx_t dst_len, idx_t* dst_size, u64 src, bool thousands_sep = 0 )
{
  return CsFromIntegerU( dst, dst_len, dst_size, src, thousands_sep );
}

Inl bool
CsFrom_u32( u8* dst, idx_t dst_len, idx_t* dst_size, u32 src, bool thousands_sep = 0 )
{
  return CsFromIntegerU( dst, dst_len, dst_size, src, thousands_sep );
}

Inl bool
CsFrom_u16( u8* dst, idx_t dst_len, idx_t* dst_size, u16 src, bool thousands_sep = 0 )
{
  return CsFromIntegerU( dst, dst_len, dst_size, src, thousands_sep );
}

Inl bool
CsFrom_u8( u8* dst, idx_t dst_len, idx_t* dst_size, u8 src, bool thousands_sep = 0 )
{
  return CsFromIntegerU( dst, dst_len, dst_size, src, thousands_sep );
}



Inl bool
CsFrom_s64( u8* dst, idx_t dst_len, idx_t* dst_size, s64 src, bool thousands_sep = 0 )
{
  return CsFromIntegerS( dst, dst_len, dst_size, src, thousands_sep );
}

Inl bool
CsFrom_s32( u8* dst, idx_t dst_len, idx_t* dst_size, s32 src, bool thousands_sep = 0 )
{
  return CsFromIntegerS( dst, dst_len, dst_size, src, thousands_sep );
}

Inl bool
CsFrom_s16( u8* dst, idx_t dst_len, idx_t* dst_size, s16 src, bool thousands_sep = 0 )
{
  return CsFromIntegerS( dst, dst_len, dst_size, src, thousands_sep );
}

Inl bool
CsFrom_s8( u8* dst, idx_t dst_len, idx_t* dst_size, s8 src, bool thousands_sep = 0 )
{
  return CsFromIntegerS( dst, dst_len, dst_size, src, thousands_sep );
}






Inl u64
CsTo_u64( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerU<u64>( src, src_len, ',', radix );
}

Inl u32
CsTo_u32( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerU<u32>( src, src_len, ',', radix );
}

Inl u16
CsTo_u16( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerU<u16>( src, src_len, ',', radix );
}

Inl u8
CsTo_u8( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerU<u8>( src, src_len, ',', radix );
}



Inl s64
CsTo_s64( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerS<s64>( src, src_len, ',', radix );
}

Inl s32
CsTo_s32( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerS<s32>( src, src_len, ',', radix );
}

Inl s16
CsTo_s16( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerS<s16>( src, src_len, ',', radix );
}

Inl s8
CsTo_s8( u8* src, idx_t src_len, u8 radix = 10 )
{
  return CsToIntegerS<s8>( src, src_len, ',', radix );
}





#define CURSORMOVE( name, cond ) \
  Inl idx_t \
  NAMEJOIN( name, L )( u8* src, idx_t src_len, idx_t pos ) \
  { \
    while( pos ) { \
      pos -= 1; \
      if( cond( src[pos] ) ) { \
        pos += 1; \
        break; \
      } \
    } \
    return pos; \
  } \
  Inl idx_t \
  NAMEJOIN( name, R )( u8* src, idx_t src_len, idx_t pos ) \
  { \
    while( pos != src_len ) { \
      if( cond( src[pos] ) ) { \
        break; \
      } \
      pos += 1; \
    } \
    return pos; \
  } \

#define COND( c )   ( c == '\r' )  ||  ( c == '\n' )
CURSORMOVE( CursorStopAtNewline, COND );
#undef COND

#define COND( c )   ( c == ' '  )  ||  ( c == '\t' )
CURSORMOVE( CursorStopAtSpacetab, COND );
#undef COND

#define COND( c )   ( c != ' '  )  &&  ( c != '\t' )
CURSORMOVE( CursorSkipSpacetab, COND );
#undef COND

#undef CURSORMOVE


Inl idx_t
CursorCharL( u8* src, idx_t src_len, idx_t pos )
{
  if( !pos ) {
    return 0;
  } else {
    return pos - 1;
  }
}

Inl idx_t
CursorCharR( u8* src, idx_t src_len, idx_t pos )
{
  if( pos == src_len ) {
    return pos;
  } else {
    return pos + 1;
  }
}




static constexpr idx_t c_fspath_len = 384;

// Holds metadata for a file OR a directory in a filesystem.
typedef embeddedarray_t<u8, c_fspath_len> fsobj_t;

Inl fsobj_t
_fsobj( void* cstr )
{
  fsobj_t r;
  r.len = CsLen( Cast( u8*, cstr ) );
  Memmove( r.mem, cstr, r.len );
  return r;
}


Inl u64
_GetFileTime( WIN32_FIND_DATA& f )
{
  u64 time = Pack( f.ftLastWriteTime.dwHighDateTime, f.ftLastWriteTime.dwLowDateTime );
  return time;
}

Inl u64
_GetFileSize( WIN32_FIND_DATA& f )
{
  u64 size;
  size = Cast( u64, f.nFileSizeHigh ) << 32;
  size |= Cast( u64, f.nFileSizeLow );
  return size;
}

Inl bool
_IsRegularDir( WIN32_FIND_DATA& f )
{
  auto len = CsLen( Str( f.cFileName ) );
  bool r =
    ( f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
    !CsEquals( Str( f.cFileName ), len, Str( "." ), 1, 1 ) &&
    !CsEquals( Str( f.cFileName ), len, Str( ".." ), 2, 1 );
  return r;
}

Inl bool
_IsFile( WIN32_FIND_DATA& f )
{
  return !( f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
}



static bool
_ObjectExists( u8* name )
{
  WIN32_FIND_DATA f;
  HANDLE h = FindFirstFile( Cast( char*, name ), &f );
  if( INVALID_HANDLE_VALUE == h ) {
    return 0;
  }
  AssertWarn( FindClose( h ) );
  return 1;
}

static bool
_RecycleFsObj( u8* name, idx_t len )
{
  AssertCrash( name[len] == 0 );

  u8 fullpath[c_fspath_len + 2];
  u8* name_only = 0;
  DWORD res = GetFullPathName( Cast( char*, name ), sizeof( fullpath ), Cast( char*, fullpath ), Cast( char**, &name_only ) );
  if( !res ) {
    return 0;
  }

  idx_t fullpath_len = CsLen( fullpath );
  fullpath[fullpath_len + 0] = 0;
  fullpath[fullpath_len + 1] = 0; // double 0-term for SHFILEOPSTRUCT.

  SHFILEOPSTRUCT recycle = {};
  recycle.hwnd = 0;
  recycle.wFunc = FO_DELETE;
  recycle.pFrom = Cast( char*, fullpath );
  recycle.pTo = "\0\0";
  recycle.fFlags = FOF_ALLOWUNDO;
  recycle.lpszProgressTitle = "Sending fsobj to recycle bin.";

  int operr = SHFileOperation( &recycle );
  if( operr ) {
    return 0;
  }

  if( recycle.fAnyOperationsAborted ) {
    return 0;
  }

  return 1;
}



// utility to standardize dir/file pathnames.

void
_FixupDir( u8* dir, idx_t* dir_len )
{
  idx_t len = *dir_len;
  if( len ) {
    // leave leading backslashes in place, since windows requires network shares to start with two of them.
    idx_t start = 0;
    For( i, 0, len ) {
      if( dir[i] != '\\' ) {
        break;
      }
      ++start;
    }
    CsReplace( dir + start, len - start, '\\', '/' );
    if( dir[len - 1] == '/'  ||  dir[len - 1] == '\\' ) {
      dir[len - 1] = 0;
      len -= 1;
    }
  }
  *dir_len = len;
}

void
_FixupFile( u8* file, idx_t* file_len )
{
  CsReplace( file, *file_len, '\\', '/' );
}

Inl fsobj_t
_StandardFilename( u8* name, idx_t len )
{
  fsobj_t file;
  file.len = 0;
  Memmove( AddBack( file, len ), name, len );
  Memmove( AddBack( file ), "\0", 1 );
  RemBack( file );
  _FixupFile( file.mem, &file.len );
  return file;
}

Inl fsobj_t
_StandardDirname( u8* name, idx_t len )
{
  fsobj_t dir;
  dir.len = 0;
  Memmove( AddBack( dir, len ), name, len );
  Memmove( AddBack( dir ), "\0", 1 );
  RemBack( dir );
  _FixupDir( dir.mem, &dir.len );
  return dir;
}






void
FsGetCwd( u8* dst, idx_t dst_len, idx_t* cwd_len )
{
  AssertCrash( dst_len <= UINT_MAX );
  DWORD res = GetCurrentDirectory( Cast( DWORD, dst_len ), Cast( char*, dst ) );
  AssertWarn( res );
  *cwd_len = CsLen( dst );
  _FixupDir( dst, cwd_len );
}

void
FsGetCwd( fsobj_t& obj )
{
  FsGetCwd( obj.mem, Capacity( obj ), &obj.len );
}

void
FsSetCwd( u8* cwd, idx_t cwd_len )
{
  fsobj_t dir = _StandardDirname( cwd, cwd_len );
  BOOL res = SetCurrentDirectory( Cast( char*, dir.mem ) );
  AssertWarn( res );
}

void
FsSetCwd( fsobj_t& obj )
{
  FsSetCwd( ML( obj ) );
}





#define FS_ITERATOR( fnname )   \
  bool ( fnname )( \
    u8* name, \
    idx_t len, \
    bool file, \
    bool readonly, \
    u64 filesize, \
    void* misc \
    )

typedef FS_ITERATOR( *pfn_fsiterator_t );

bool
FsIterate(
  u8* path,
  idx_t path_len,
  bool recur,
  pfn_fsiterator_t FsIterator,
  void* misc
  )
{
  bool r = 1;
  array_t<fsobj_t> searchdirs;
  Alloc( searchdirs, 32 );
  *AddBack( searchdirs ) = _StandardDirname( path, path_len );
  while( searchdirs.len ) {
    fsobj_t searchdir = searchdirs.mem[ searchdirs.len - 1 ];
    RemBack( searchdirs );
    Memmove( AddBack( searchdir, 3 ), "/*\0", 3 );
    WIN32_FIND_DATA f;
    // PERF: try FindFirstFileEx, etc. there's params to help speed things up on Win7+
    HANDLE h = FindFirstFile( Cast( char*, searchdir.mem ), &f );
    RemBack( searchdir, 2 );
    if( h == INVALID_HANDLE_VALUE ) {
      continue;
    }
    bool more_files = 1;
    while( more_files ) {
      if( _IsFile( f ) ) {
        auto filename_len = CsLen( Cast( u8*, f.cFileName ) );
        Memmove( AddBack( searchdir, filename_len ), Cast( u8*, f.cFileName ), filename_len );
        bool readonly = f.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
        u64 filesize = Pack( f.nFileSizeHigh, f.nFileSizeLow );
        r &= FsIterator( ML( searchdir ), 1, readonly, filesize, misc );
        RemBack( searchdir, filename_len );
      } elif( _IsRegularDir( f ) ) {
        auto filename_len = CsLen( Cast( u8*, f.cFileName ) );
        Memmove( AddBack( searchdir, filename_len ), Cast( u8*, f.cFileName ), filename_len );
        r &= FsIterator( ML( searchdir ), 0, 0, 0, misc );
        if( recur ) {
          *AddBack( searchdirs ) = searchdir;
        }
        RemBack( searchdir, filename_len );
      }
      more_files = !!FindNextFile( h, &f );
    }
    FindClose( h );
  }
  Free( searchdirs );
  return r;
}








bool
DirExists( u8* name, idx_t len )
{
  fsobj_t dir = _StandardDirname( name, len );
  DWORD attribs = GetFileAttributes( Cast( char*, dir.mem ) );
  if( attribs == INVALID_FILE_ATTRIBUTES ) {
    return 0;
  }
  return !!( attribs & FILE_ATTRIBUTE_DIRECTORY );
}


bool
DirCreate( u8* name, idx_t len )
{
  fsobj_t dir = _StandardDirname( name, len );

  u8* slash = dir.mem;
  Forever {
    slash = CsScanR( slash, dir.len, '/' );
    if( slash ) {
      *slash = 0;
    }
    if( !DirExists( ML( dir ) ) ) {
      if( !CreateDirectory( Cast( char*, dir.mem ), 0 ) ) {
        return 0;
      }
    }
    if( slash ) {
      *slash = '/';
      slash += 1;
    } else {
      break;
    }
  }
  return 1;
}


bool
FileDelete( u8* name, idx_t len )
{
  fsobj_t file = _StandardFilename( name, len );
  if( !DeleteFile( Cast( char*, file.mem ) ) ) {
    return 0;
  }
  return 1;
}





Inl
FS_ITERATOR( _IterDirDeleteContents )
{
  bool r = 1;
  if( file ) {
    if( !FileDelete( name, len ) ) {
      r = 0;
    }
  } else {
    auto dirs = Cast( array_t<fsobj_t>*, misc );
    *AddBack( *dirs ) = _StandardDirname( name, len );
  }
  return r;
}

bool
DirDeleteContents( u8* name, idx_t len )
{
  fsobj_t dir = _StandardDirname( name, len );
  array_t<fsobj_t> dirs_to_delete;
  Alloc( dirs_to_delete, 32 );
  bool r = FsIterate( name, len, 1, _IterDirDeleteContents, Cast( void*, &dirs_to_delete ) );
  ReverseFor( i, 0, dirs_to_delete.len ) {
    auto rem = dirs_to_delete.mem + i;
    r &= !!RemoveDirectory( Cast( char*, rem->mem ) );
  }
  Free( dirs_to_delete );
  return r;
}



bool
DirDelete( u8* name, idx_t len )
{
  fsobj_t dir = _StandardDirname( name, len );
  if( !DirDeleteContents( name, len ) ) {
    return 0;
  }
  if( !RemoveDirectory( Cast( char*, dir.mem ) ) ) {
    return 0;
  }
  return 1;
}

bool
DirRecycle( u8* name, idx_t len )
{
  fsobj_t dir = _StandardDirname( name, len );
  return _RecycleFsObj( ML( dir ) );
}



bool
DirCopy( u8* dst, idx_t dst_len, u8* src, idx_t src_len )
{
  ImplementCrash();
  if( CsEquals( dst, dst_len, src, src_len, 0 ) ) {
    return 1;
  }

  //array_t<fsobj_t> files;
  //Alloc( files, 16 );
  //fsFindFiles( files, src );
  //u8 name [ c_fspath_len ];
  //for( idx_t i = 0;  i < size( files );  ++i ) {
  //  CsCopy( name, dst, CsLen( dst ) );
  //  Cstr::AddBack( name, '/' );
  //  if( files[i].PathRel[0] ) {
  //    Cstr::AddBack( name, files[i].PathRel );
  //    Cstr::AddBack( name, '/' );
  //  }
  //  Cstr::AddBack( name, files[i].NameOnly );
  //  Cstr::AddBack( name, '.' );
  //  Cstr::AddBack( name, files[i].Ext );
  //  if( !FileCopy( name, files[i].NameAbs ) )
  //    goto fsDirCopy_FAIL;
  //}
  //EditKill( files );
  //return 1;

  //fsDirCopy_FAIL:
  //EditKill( files );
  return 1;
}

bool
DirCopyOverwrite( u8* dst, idx_t dst_len, u8* src, idx_t src_len )
{
  ImplementCrash();
  if( CsEquals( dst, dst_len, src, src_len, 0 ) ) {
    return 1;
  }

  //array_t<fsobj_t> files;
  //Alloc( files, 16 );
  //fsFindFiles( files, src );
  //u8 name [ c_fspath_len ];
  //for( idx_t i = 0;  i < size( files );  ++i ) {
  //  CsCopy( name, dst, CsLen( dst ) );
  //  Cstr::AddBack( name, '/' );
  //  if( files[i].PathRel[0] ) {
  //    Cstr::AddBack( name, files[i].PathRel );
  //    Cstr::AddBack( name, '/' );
  //  }
  //  Cstr::AddBack( name, files[i].NameOnly );
  //  Cstr::AddBack( name, '.' );
  //  Cstr::AddBack( name, files[i].Ext );
  //  if( !FileCopyOverwrite( name, files[i].NameAbs ) )
  //    goto fsDirCopyOverwrite_FAIL;
  //}
  //EditKill( files );
  //return 1;

  //fsDirCopyOverwrite_FAIL:
  //EditKill( files );
  return 1;
}

bool
DirMove( u8* dst, idx_t dst_len, u8* src, idx_t src_len )
{
  fsobj_t srcdir = _StandardDirname( src, src_len );
  fsobj_t dstdir = _StandardDirname( dst, dst_len );

  if( _ObjectExists( dstdir.mem ) ) {
    return 0;
  }
  bool moved = !!MoveFile( Cast( char*, srcdir.mem ), Cast( char*, dstdir.mem ) );
  return moved;
}


struct
file_t
{
  fsobj_t obj;
  void* loaded; // stores file handle.
  u64 size;
  u64 time_create;
  u64 time_lastaccess;
  u64 time_lastwrite;
  bool readonly;
};



bool
_EnsureDstDirectory( fsobj_t& dstfile )
{
  u8* last_slash = CsScanL( ML( dstfile ), '/' );
  if( !last_slash ) {
    return 0;
  }
  fsobj_t dst_dir = {};
  CsCopy( dst_dir.mem, dstfile.mem, last_slash );
  dst_dir.len = CsLen( dst_dir.mem );
  if( !DirExists( ML( dst_dir ) ) ) {
    if( !DirCreate( ML( dst_dir ) ) ) {
      return 0;
    }
  }
  return 1;
}

#define _GetHandle( file ) \
  Cast( HANDLE, file.loaded )

#define _u64_from_FILETIME( ft ) \
  Pack( ft.dwHighDateTime, ft.dwLowDateTime )

Inl void
_PopulateMetadata( file_t& file )
{
  BY_HANDLE_FILE_INFORMATION fileinfo = {};
  AssertWarn( GetFileInformationByHandle( _GetHandle( file ), &fileinfo ) );
  file.size = Pack( fileinfo.nFileSizeHigh, fileinfo.nFileSizeLow );
  file.time_create     = _u64_from_FILETIME( fileinfo.ftCreationTime   );
  file.time_lastaccess = _u64_from_FILETIME( fileinfo.ftLastAccessTime );
  file.time_lastwrite  = _u64_from_FILETIME( fileinfo.ftLastWriteTime  );
  file.readonly = fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
}

void
_SetFilePtr( file_t& file, u64 file_offset )
{
  s32 res = SetFilePointer( _GetHandle( file ), 0, 0, FILE_BEGIN );
  AssertWarn( res != INVALID_SET_FILE_POINTER );
  auto nchunks = Cast( u64, file_offset / MAX_s64 );
  auto nrem    = Cast( u64, file_offset % MAX_s64 );
  Fori( u64, i, 0, nchunks ) {
    auto file_offset_lo = Cast( LONG, MAX_u32 );
    auto file_offset_hi = Cast( LONG, MAX_s32 );
    res = SetFilePointer( _GetHandle( file ), file_offset_lo, &file_offset_hi, FILE_CURRENT );
    AssertWarn( res != INVALID_SET_FILE_POINTER );
  }
  if( nrem ) {
    auto file_offset_lo = Cast( LONG, nrem & MAX_u32 );
    auto file_offset_hi = Cast( LONG, nrem >> 32ULL );
    res = SetFilePointer( _GetHandle( file ), file_offset_lo, file_offset_hi ? &file_offset_hi : 0, FILE_CURRENT );
    AssertWarn( res != INVALID_SET_FILE_POINTER );
  }
}


Enumc( fileop_t )
{
  R,
  W,
  RW,
};

static const bool g_hasWrite[] = {
  0,
  1,
  1,
};

#define _HasWrite( op ) \
  g_hasWrite[Cast( enum_t, op )]

static const DWORD g_accessBits[] = {
  GENERIC_READ,
  GENERIC_WRITE,
  GENERIC_READ | GENERIC_WRITE,
};

#define _GetAccessBits( access ) \
  g_accessBits[Cast( enum_t, access )]

static const DWORD g_shareBits[] = {
  FILE_SHARE_READ,
  FILE_SHARE_WRITE,
  FILE_SHARE_READ | FILE_SHARE_WRITE,
};

#define _GetShareBits( share ) \
  g_shareBits[Cast( enum_t, share )]

Enumc( fileopen_t )
{
  only_new,
  only_existing,
  always,
};

static const DWORD g_opentype[] = {
  CREATE_NEW,
  OPEN_EXISTING,
  OPEN_ALWAYS,
};

#define _GetOpenType( open ) \
  g_opentype[Cast( enum_t, open )]



file_t
FileOpen( u8* name, idx_t len, fileopen_t type, fileop_t access, fileop_t share )
{
  file_t file = {};
  file.obj = _StandardFilename( name, len );
  DWORD attribs = FILE_ATTRIBUTE_NORMAL;

  switch( type ) {

    case fileopen_t::only_new: {
      if( !_EnsureDstDirectory( file.obj ) ) {
        return file;
      }
    } break;

    case fileopen_t::only_existing: {
      attribs = GetFileAttributes( Cast( char*, file.obj.mem ) );
      if( _HasWrite( access ) && ( attribs & FILE_ATTRIBUTE_READONLY ) ) {
        attribs &= ~FILE_ATTRIBUTE_READONLY;
        if( !SetFileAttributes( Cast( char*, file.obj.mem ), attribs ) ) {
          return file;
        }
      }
    } break;

    case fileopen_t::always: {
      if( !_EnsureDstDirectory( file.obj ) ) {
        return file;
      }
    } break;
  }

  HANDLE h = CreateFile(
    Cast( char*, file.obj.mem ),
    _GetAccessBits( access ),
    _GetShareBits( share ),
    0,
    _GetOpenType( type ),
    attribs,
    0
    );

  if( h == INVALID_HANDLE_VALUE ) {
    return file;
  }
  file.loaded = Cast( void*, h );
  _PopulateMetadata( file );
  return file;
}


// TODO: probably return success bool
void
FileRead( file_t& file, u64 file_offset, u8* dst, u64 dst_len )
{
  _SetFilePtr( file, file_offset );
  u64 ntoread = MIN( file.size - file_offset, dst_len );
  u32 nchunks = Cast( u32, ntoread / MAX_u32 );
  u32 nrem    = Cast( u32, ntoread % MAX_u32 );
  Fori( u32, i, 0, nchunks ) {
    DWORD nread = 0;
    BOOL r = ReadFile(
      _GetHandle( file ),
      dst,
      MAX_u32,
      &nread,
      0
      );
    AssertWarn( r );
    AssertWarn( nread == MAX_u32 );
    dst += nread;
  }

  if( nrem ) {
    DWORD nread = 0;
    BOOL r = ReadFile(
      _GetHandle( file ),
      dst,
      nrem,
      &nread,
      0
      );
    AssertWarn( r );
    AssertWarn( nread == nrem );
    dst += nread;
  }

  // update file info.
  _PopulateMetadata( file );
}


void
FileWrite( file_t& file, u64 file_offset, u8* src, u64 src_len )
{
  _SetFilePtr( file, file_offset );
  u32 nchunks = Cast( u32, src_len / MAX_u32 );
  u32 nrem    = Cast( u32, src_len % MAX_u32 );
  Fori( u32, i, 0, nchunks ) {
    DWORD nwritten = 0;
    BOOL r = WriteFile(
      _GetHandle( file ),
      src,
      MAX_u32,
      &nwritten,
      0
      );
    AssertWarn( r );
    AssertWarn( nwritten == MAX_u32 );
    src += nwritten;
  }

  DWORD nwritten = 0;
  BOOL r = WriteFile(
    _GetHandle( file ),
    src,
    nrem,
    &nwritten,
    0
    );
  AssertWarn( r );
  AssertWarn( nwritten == nrem );
  src += nwritten;

  // update file info.
  _PopulateMetadata( file );
}

void
FileWriteAppend( file_t& file, u8* src, u64 src_len )
{
  FileWrite( file, file.size, src, src_len );
}


void
FileSetEOF( file_t& file, u64 file_offset )
{
  _SetFilePtr( file, file_offset );
  AssertWarn( SetEndOfFile( _GetHandle( file ) ) );
  _PopulateMetadata( file );
}
void
FileSetEOF( file_t& file )
{
  FileSetEOF( file, file.size );
}


u64
FileTimeLastWrite( u8* name, idx_t len )
{
  fsobj_t file = _StandardFilename( name, len );
  WIN32_FILE_ATTRIBUTE_DATA metadata;
  AssertWarn( GetFileAttributesEx( Cast( char*, file.mem ), GetFileExInfoStandard, &metadata ) );
  return Pack( metadata.ftLastWriteTime.dwHighDateTime, metadata.ftLastWriteTime.dwLowDateTime );
}

void
FileFree( file_t& file )
{
  if( file.loaded ) {
    AssertWarn( CloseHandle( _GetHandle( file ) ) );
  }
  file = {};
}




bool
FileRecycle( u8* name, idx_t len )
{
  fsobj_t file = _StandardFilename( name, len );
  return _RecycleFsObj( ML( file ) );
}



bool
FileExists( u8* name, idx_t len )
{
  fsobj_t file = _StandardFilename( name, len );
  WIN32_FIND_DATA f;
  HANDLE h = FindFirstFile( Cast( char*, file.mem ), &f );
  if( INVALID_HANDLE_VALUE == h ) {
    return 0;
  }
  bool r = _IsFile( f );
  AssertWarn( FindClose( h ) );
  return r;
}




bool
FileCopy( u8* dstname, idx_t dstname_len, u8* srcname, idx_t srcname_len )
{
  fsobj_t dst = _StandardFilename( dstname, dstname_len );
  fsobj_t src = _StandardFilename( srcname, srcname_len );
  if( !FileExists( ML( src ) ) ) {
    return 0;
  }
  if( !_EnsureDstDirectory( dst ) ) {
    return 0;
  }
  bool copied = !!CopyFile( Cast( char*, src.mem ), Cast( char*, dst.mem ), 1 /* FAIL if exists */ );
  return copied;
}

bool
FileCopyOverwrite( u8* dstname, idx_t dstname_len, u8* srcname, idx_t srcname_len )
{
  fsobj_t dst = _StandardFilename( dstname, dstname_len );
  fsobj_t src = _StandardFilename( srcname, srcname_len );
  if( !_EnsureDstDirectory( dst ) ) {
    return 0;
  }
  bool copied = !!CopyFile( Cast( char*, src.mem ), Cast( char*, dst.mem ), 0 /* OVR if exists */ );
  return copied;
}

bool
FileMove( u8* dstname, idx_t dstname_len, u8* srcname, idx_t srcname_len )
{
  fsobj_t dst = _StandardFilename( dstname, dstname_len );
  fsobj_t src = _StandardFilename( srcname, srcname_len );
  if( FileExists( ML( dst ) ) ) {
    return 0;
  }
  bool moved = !!MoveFile( Cast( char*, src.mem ), Cast( char*, dst.mem ) );
  return moved;
}



#if 0

u64
GetSize( file_t& file )
{
  u64 size = 0;
  AssertWarn( GetFileSizeEx( _GetHandle( file ), Cast( LARGE_INTEGER*, &size ) ) );
  return size;
}

bool
Size( u8* name, idx_t len, u64* size )
{
  fsobj_t file = _StandardFilename( name, len );

  WIN32_FILE_ATTRIBUTE_DATA metadata;
  bool res = !!GetFileAttributesEx( Cast( char*, file.mem ), GetFileExInfoStandard, &metadata );
  u64 r = 0;
  if( res ) {
    r = Cast( u64, metadata.nFileSizeHigh ) << 32ULL;
    r |= Cast( u64, metadata.nFileSizeLow );
  }
  *size = r;
  return res;
}

#endif


struct
filemapped_t
{
  u8* mapped_mem;
  idx_t len;
  void* m; // file mapping handle.
  void* f; // file handle.
};

filemapped_t
FileAllocReadOnlyExclusive( u8* filename, idx_t filename_len )
{
  fsobj_t file = _StandardFilename( filename, filename_len );

  filemapped_t ret = {};

  HANDLE f = CreateFile(
    Cast( char*, file.mem ),
    GENERIC_READ,
    FILE_SHARE_READ,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0
    );
  if( f == INVALID_HANDLE_VALUE ) {
    return ret;
  }

  u64 file_size;
  BOOL sized = GetFileSizeEx( f, Cast( LARGE_INTEGER*, &file_size ) );
  AssertWarn( sized );
  if( !file_size ) {
    // we can't memory-map an empty file.
    ret.f = Cast( void*, f );

  } else {
    HANDLE m = CreateFileMapping(
      f,
      0,
      PAGE_READONLY,
      0, 0, 0
      );
    if( !m ) {
      CloseHandle( f );
      return ret;
    }

    void* p = MapViewOfFile(
      m,
      FILE_MAP_READ,
      0, 0, 0
      );
    if( !p ) {
      CloseHandle( f );
      CloseHandle( m );
      return ret;
    }

    ret.mapped_mem = Cast( u8*, p );

    AssertCrash( file_size < MAX_idx );
    ret.len = Cast( idx_t, file_size );
    ret.m = Cast( void*, m );
    ret.f = Cast( void*, f );
  }

  return ret;
}

void
FileFree( filemapped_t& file )
{
  if( file.len ) {
    AssertWarn( UnmapViewOfFile( file.mapped_mem ) );
    CloseHandle( Cast( HANDLE, file.m ) );
    CloseHandle( Cast( HANDLE, file.f ) );
  }

  file = {};
}


// TODO: handle 0 size.
// TODO: callers should probably stream into chunks, rather than one contiguous string_t.
string_t
FileAlloc( file_t& file )
{
  AssertCrash( file.size <= MAX_idx );
  idx_t ntoread = Cast( idx_t, file.size );

  string_t r;
  Alloc( r, ntoread );
  FileRead( file, 0, ML( r ) );
  return r;
}



// TODO: redo hotloading.

// effectively an FileReadAll() call, but with additional timestamp caching so
//   that you'll only go to disk if the timestamps are different.
// 'dst' is always cleared.
// copies all of the bytes into 'dst' from the 'filename' file.
// if the file is new or has changed, 'dst' will contain updated file
//   contents, and returns 1.
// if the file has NOT changed, returns 1.
// if the file doesn't exist or the read fails, returns 0.
//bool
//FileHotload( u8* filename, idx_t filename_len, slice_t* dst );



#if 0

Inl void
_FindFiles( array_t<fsobj_t>& dst, u8* dir_base, idx_t dir_base_len )
{
  WIN32_FIND_DATA f;
  HANDLE h;

  fsobj_t file = {};
  CsCopy( file.mem, dir_base, dir_base_len );
  CsAddBack( file.mem, '/' );
  u8* file_name_rel = file.mem + CsLen( file.mem );

  fsobj_t search = {};
  CsCopy( search.mem, dir_base, dir_base_len );
  CsAddBack( search.mem, Str( "/*" ), 2 );
  h = FindFirstFile( Cast( char*, search.mem ), &f );
  if( h == INVALID_HANDLE_VALUE ) {
    return;
  }

  Forever {
    if( _IsFile( f ) ) {
      CsCopy( file_name_rel, Cast( u8*, f.cFileName ) );
      file.len = CsLen( file.mem );
      *AddBack( dst ) = file;
    }
    if( !FindNextFile( h, &f ) ) {
      break;
    }
  }
  FindClose( h );
}


Inl void
_FindDirs( array_t<fsobj_t>& dst, u8* dir_base, idx_t dir_base_len )
{
  WIN32_FIND_DATA f;
  HANDLE h;

  fsobj_t dir = {};
  CsCopy( dir.mem, dir_base, dir_base_len );
  CsAddBack( dir.mem, '/' );
  u8* dir_name_rel = dir.mem + CsLen( dir.mem );

  fsobj_t search = {};
  CsCopy( search.mem, dir_base, dir_base_len );
  CsAddBack( search.mem, Str( "/*" ), 2 );
  h = FindFirstFile( Cast( char*, search.mem ), &f );
  if( h == INVALID_HANDLE_VALUE ) {
    return;
  }

  Forever {
    if( _IsRegularDir( f ) ) {
      CsCopy( dir_name_rel, Cast( u8*, f.cFileName ) );
      dir.len = CsLen( dir.mem );
      *AddBack( dst ) = dir;
    }
    if( !FindNextFile( h, &f ) ) {
      break;
    }
  }
  FindClose( h );
}





Inl void
_FindFilesRecur( array_t<fsobj_t>& dst, u8* dir_base, idx_t dir_base_len )
{
  WIN32_FIND_DATA f;
  HANDLE h;

  array_t<fsobj_t> stack_subdirs;
  Alloc( stack_subdirs, 64 );

  fsobj_t search = {};
  CsCopy( search.mem, dir_base, dir_base_len );
  CsAddBack( search.mem, '/' );
  u8* search_rel = search.mem + CsLen( search.mem );

  fsobj_t file = {};
  CsCopy( file.mem, dir_base, dir_base_len );
  CsAddBack( file.mem, '/' );
  u8* file_name_rel_base = file.mem + CsLen( file.mem );

  fsobj_t subdir = {};
  subdir.mem[0] = 0;
  *AddBack( stack_subdirs ) = subdir;

  while( stack_subdirs.len ) {
    subdir = stack_subdirs.mem[stack_subdirs.len - 1];
    RemBack( stack_subdirs );
    idx_t subdir_len = CsLen( subdir.mem );

    u8* file_name_rel = file_name_rel_base; // relative to subdir.
    if( subdir.mem[0] ) {
      CsCopy( file_name_rel, subdir.mem, subdir_len );
      file_name_rel += subdir_len;
      *file_name_rel++ = '/';

      CsCopy( search_rel, subdir.mem, subdir_len );
      CsAddBack( search_rel, '/' );
      CsAddBack( search_rel, '*' );
    } else {
      CsCopy( search_rel, '*' );
    }
    h = FindFirstFile( Cast( char*, search.mem ), &f );
    if( h == INVALID_HANDLE_VALUE )
      continue;

    Forever {
      if( _IsFile( f ) ) {
        CsCopy( file_name_rel, Cast( u8*, f.cFileName ) );
        file.len = CsLen( file.mem );
        *AddBack( dst ) = file;

      } elif( _IsRegularDir( f ) ) {
        u8* subdir_end = subdir.mem + subdir_len;
        if( subdir.mem[0] ) {
          CsCopy( subdir_end, '/' );
          CsAddBack( subdir_end, Cast( u8*, f.cFileName ) );
        } else {
          CsCopy( subdir_end, Cast( u8*, f.cFileName ) );
        }
        *AddBack( stack_subdirs ) = subdir;
        subdir_end[0] = 0; // reset subdir.
      }
      if( !FindNextFile( h, &f ) )
        break;
    }
    FindClose( h );
  }
  Free( stack_subdirs );
}


Inl void
_FindDirsRecur( array_t<fsobj_t>& dst, u8* dir_base, idx_t dir_base_len )
{
  WIN32_FIND_DATA f;
  HANDLE h;

  array_t<fsobj_t> stack_subdirs;
  Alloc( stack_subdirs, 64 );

  fsobj_t search = {};
  CsCopy( search.mem, dir_base, dir_base_len );
  CsAddBack( search.mem, '/' );
  u8* search_rel = search.mem + CsLen( search.mem );

  fsobj_t dir = {};
  CsCopy( dir.mem, dir_base, dir_base_len );
  CsAddBack( dir.mem, '/' );
  u8* dir_name_rel_base = dir.mem + CsLen( dir.mem );

  fsobj_t subdir;
  subdir.len = 0;
  *AddBack( stack_subdirs ) = subdir;

  while( stack_subdirs.len ) {
    subdir = stack_subdirs.mem[stack_subdirs.len - 1];
    RemBack( stack_subdirs );

    u8* dir_name_rel = dir_name_rel_base; // relative to subdir.
    if( subdir.mem[0] ) {
      Memmove( dir_name_rel, subdir.mem, subdir.len );
      dir_name_rel += subdir.len;
      *dir_name_rel++ = '/';

      CsCopy( search_rel, subdir.mem, subdir.len );
      CsAddBack( search_rel, '/' );
      CsAddBack( search_rel, '*' );
    } else {
      CsCopy( search_rel, '*' );
    }
    h = FindFirstFile( Cast( char*, search.mem ), &f );
    if( h == INVALID_HANDLE_VALUE ) {
      continue;
    }

    Forever {
      if( _IsRegularDir( f ) ) {
        CsCopy( dir_name_rel, Cast( u8*, f.cFileName ) );
        dir.len = CsLen( dir.mem );
        *AddBack( dst ) = dir;

        u8* subdir_end = subdir.mem + subdir_len;
        if( subdir.mem[0] ) {
          CsCopy( subdir_end, '/' );
          CsAddBack( subdir_end, Cast( u8*, f.cFileName ) );
        } else {
          CsCopy( subdir_end, Cast( u8*, f.cFileName ) );
        }
        *AddBack( stack_subdirs ) = subdir;
        subdir_end[0] = 0; // reset subdir.
      }
      if( !FindNextFile( h, &f ) ) {
        break;
      }
    }
    FindClose( h );
  }
  Free( stack_subdirs );
}

void
FsFindFiles( array_t<fsobj_t>& dst, u8* dir_base, idx_t dir_base_len, bool recur )
{
  if( recur ) {
    _FindFilesRecur( dst, dir_base, dir_base_len );
  } else {
    _FindFiles( dst, dir_base, dir_base_len );
  }
}

void
FsFindDirs( array_t<fsobj_t>& dst, u8* dir_base, idx_t dir_base_len, bool recur )
{
  if( recur ) {
    _FindDirsRecur( dst, dir_base, dir_base_len );
  } else {
    _FindDirs( dst, dir_base, dir_base_len );
  }
}

#endif









struct
findfiles_t
{
  array_t<slice_t>* spans;
  plist_t* plist;
};

Inl
FS_ITERATOR( _IterFindFiles )
{
  auto dst = Cast( findfiles_t*, misc );
  if( file ) {
    auto span = AddBack( *dst->spans );
    span->mem = AddPlist( u8, *dst->plist, len );
    span->len = len;
    Memmove( span->mem, name, len );
  }
  return 1;
}

Inl void
FsFindFiles(
  array_t<slice_t>& dst,
  plist_t& dstmem,
  u8* path,
  idx_t path_len,
  bool recur
  )
{
  findfiles_t find;
  find.spans = &dst;
  find.plist = &dstmem;
  FsIterate( path, path_len, recur, _IterFindFiles, &find );
}

Inl
FS_ITERATOR( _IterFindDirs )
{
  auto dst = Cast( findfiles_t*, misc );
  if( !file ) {
    auto span = AddBack( *dst->spans );
    span->mem = AddPlist( u8, *dst->plist, len );
    span->len = len;
    Memmove( span->mem, name, len );
  }
  return 1;
}

Inl void
FsFindDirs(
  array_t<slice_t>& dst,
  plist_t& dstmem,
  u8* path,
  idx_t path_len,
  bool recur
  )
{
  findfiles_t find;
  find.spans = &dst;
  find.plist = &dstmem;
  FsIterate( path, path_len, recur, _IterFindDirs, &find );
}



struct
dir_or_file_t
{
  slice_t name;
  u64 filesize;
  bool is_file; // else dir
  bool readonly;
};

struct
finddirsandfiles_t
{
  array_t<dir_or_file_t>* spans;
  plist_t* plist;
};

Inl
FS_ITERATOR( _IterFindDirsAndFiles )
{
  auto dst = Cast( finddirsandfiles_t*, misc );
  auto dir_or_file = AddBack( *dst->spans );
  dir_or_file->name.mem = AddPlist( u8, *dst->plist, len );
  dir_or_file->name.len = len;
  Memmove( dir_or_file->name.mem, name, len );
  dir_or_file->filesize = filesize;
  dir_or_file->is_file = file;
  dir_or_file->readonly = readonly;
  return 1;
}

Inl void
FsFindDirsAndFiles(
  array_t<dir_or_file_t>& dst,
  plist_t& dstmem,
  u8* path,
  idx_t path_len,
  bool recur
  )
{
  finddirsandfiles_t find;
  find.spans = &dst;
  find.plist = &dstmem;
  FsIterate( path, path_len, recur, _IterFindDirsAndFiles, &find );
}



static f32 g_sec_per_tsc32;
static f64 g_sec_per_tsc64;

static f32 g_sec_per_qpc32;
static f64 g_sec_per_qpc64;

static bool g_has_sleep_prec_period_min;
static u32 g_sleep_prec_period_min;



// TODO: define struct types around cycle_t and clock_t so we can't mix up the apis.
//   already had one bug like that.

Inl u64
TimeTSC()
{
  return __rdtsc();
}


Inl void
TimeSleep( u32 milliseconds )
{
  Sleep( milliseconds ); // arg, winAPI! let me have a nano TimeSleep!
}


void
TimeInit()
{
  u64 t0 = __rdtsc();

  u64 qpc_per_sec;
  QueryPerformanceFrequency( Cast( LARGE_INTEGER*, &qpc_per_sec ) );
  g_sec_per_qpc32 = 1.0f / qpc_per_sec;
  g_sec_per_qpc64 = 1.0 / qpc_per_sec;

  bool init_sec_per_tsc = 0;

  Log( "CPUID" );
  LogAddIndent( +1 );

  s32 tmp[4];
  __cpuid( tmp, 0 );
  auto max_id = tmp[0];
  __cpuid( tmp, 1 << 31 );
  auto max_extid = tmp[0];

  array_t<s32> info, extinfo;
  Alloc( info, 256 );
  Alloc( extinfo, 256 );
  Fori( s32, i, 0, max_id + 1 ) {
    __cpuidex( AddBack( info, 4 ), i, 0 );
  }
  tmp[0] = info.mem[1];
  tmp[1] = info.mem[3];
  tmp[2] = info.mem[2];
  auto vendor = Cast( u8*, tmp );
  Log( "vendor: %s", vendor );
  Fori( s32, i, 1 << 31, max_extid + 1 ) {
    __cpuidex( AddBack( extinfo, 4 ), i, 0 );
  }
  if( extinfo.len > 16 ) {
    auto brand = SliceFromCStr( extinfo.mem + 8 );
    Log( "brand: %s", brand.mem );
    auto space = CsScanL( ML( brand ), ' ' );
    auto decimal = CsScanL( ML( brand ), '.' );
    auto ghz = CsScanL( ML( brand ), 'G' );
    if( space  &&  decimal  &&  ghz ) {
      auto integer = CsToIntegerU<u32>( space + 1, decimal - ( space + 1 ) );
      auto frac = CsToIntegerU<u32>( decimal + 1, ghz - ( decimal + 1 ) );
      AssertWarn( 10 <= frac  &&  frac <= 99 );
      auto tsc_per_sec = Cast( f64, integer ) * 1e9 + Cast( f64, frac ) * 1e7;
      g_sec_per_tsc64 = 1.0  / tsc_per_sec;
      g_sec_per_tsc32 = 1.0f / Cast( f32, tsc_per_sec );
      init_sec_per_tsc = 1;
    }
  }

  if( info.len > 4 ) {
    Log( "stepping: %X", info.mem[4] & AllOnes( 4 ) );
    auto model = ( info.mem[4] >> 4 ) & AllOnes( 4 );
    auto family = ( info.mem[4] >> 8 ) & AllOnes( 4 );
    auto extmodel = ( info.mem[4] >> 16 ) & AllOnes( 4 );
    auto extfamily = ( info.mem[4] >> 20 ) & AllOnes( 8 );
    Log( "model: 0x%X", ( family == 0x6 || family == 0xF )  ?  ( extmodel << 4 ) + model  :  model );
    Log( "family: 0x%X", ( family == 0xF )  ?  ( extfamily + family )  :  family );
    Log( "processor type: 0x%X", ( info.mem[4] >> 12 ) & AllOnes( 2 ) );

    Log( "x87 FPU: %X", ( info.mem[7] >> 0 ) & 1 );
    Log( "TSC: %X", ( info.mem[7] >> 4 ) & 1 );
    Log( "CMOV: %X", ( info.mem[7] >> 15 ) & 1 );
    Log( "MMX: %X", ( info.mem[7] >> 23 ) & 1 );
    Log( "SSE: %X", ( info.mem[7] >> 25 ) & 1 );
    Log( "SSE2: %X", ( info.mem[7] >> 26 ) & 1 );

    Log( "SSE3: %X", ( info.mem[6] >> 0 ) & 1 );
    Log( "SSSE3: %X", ( info.mem[6] >> 9 ) & 1 );
    Log( "FMA: %X", ( info.mem[6] >> 12 ) & 1 );
    Log( "SSE4.1: %X", ( info.mem[6] >> 19 ) & 1 );
    Log( "SSE4.2: %X", ( info.mem[6] >> 20 ) & 1 );
    Log( "MOVBE: %X", ( info.mem[6] >> 22 ) & 1 );
    Log( "POPCNT: %X", ( info.mem[6] >> 23 ) & 1 );
    Log( "AES: %X", ( info.mem[6] >> 25 ) & 1 );
    Log( "AVX: %X", ( info.mem[6] >> 28 ) & 1 );
    Log( "F16C: %X", ( info.mem[6] >> 29 ) & 1 );
    Log( "RDRAND: %X", ( info.mem[6] >> 30 ) & 1 );
  }
  if( info.len > 28 ) {
    Log( "SGX: %X", ( info.mem[29] >> 2 ) & 1 );
    Log( "AVX2: %X", ( info.mem[29] >> 5 ) & 1 );
    Log( "AVX512F: %X", ( info.mem[29] >> 16 ) & 1 );
    Log( "RDSEED: %X", ( info.mem[29] >> 18 ) & 1 );
    Log( "AVX512PF: %X", ( info.mem[29] >> 26 ) & 1 );
    Log( "AVX512ER: %X", ( info.mem[29] >> 27 ) & 1 );
    Log( "AVX512CD: %X", ( info.mem[29] >> 28 ) & 1 );
    Log( "SHA: %X", ( info.mem[29] >> 29 ) & 1 );
  }
  if( info.len > 40 ) {
    auto version = info.mem[40] & AllOnes( 8 );
    Log( "general-purpose perf counters per logical processor: %u", ( info.mem[40] >> 8 ) & AllOnes( 8 ) );
    Log( "general-purpose perf counter bit width: %u", ( info.mem[40] >> 16 ) & AllOnes( 8 ) );
    Log( "core cycle event: %X", !( ( info.mem[41] >> 0 ) & 1 ) );
    Log( "instruction retired event: %X", !( ( info.mem[41] >> 1 ) & 1 ) );
    Log( "reference cycles event: %X", !( ( info.mem[41] >> 2 ) & 1 ) );
    Log( "last-level cache reference event: %X", !( ( info.mem[41] >> 3 ) & 1 ) );
    Log( "last-level cache miss event: %X", !( ( info.mem[41] >> 4 ) & 1 ) );
    Log( "branch instruction retired event: %X", !( ( info.mem[41] >> 5 ) & 1 ) );
    Log( "branch mispredict retired event: %X", !( ( info.mem[41] >> 6 ) & 1 ) );
    if( version > 1 ) {
      Log( "fixed-function perf counters: %u", info.mem[43] & AllOnes( 5 ) );
      Log( "fixed-function perf counter bit width: %u", ( info.mem[43] >> 5 ) & AllOnes( 7 ) );
    }
  }
  if( info.len > 84 ) {
    auto crystal_freq = Cast( u32, info.mem[86] );
    auto tsc_over_crystal_numer = Cast( u32, info.mem[85] );
    auto tsc_over_crystal_denom = Cast( u32, info.mem[84] );
    auto tsc_over_crystal = Cast( f64, tsc_over_crystal_numer ) / Cast( f64, tsc_over_crystal_denom );
    auto tsc_freq = Cast( f64, crystal_freq ) * tsc_over_crystal;
    if( crystal_freq ) {
      Log( "core crystal clock frequency ( Hz ): %u", crystal_freq );
    } else {
      Log( "core crystal clock frequency ( Hz ): UNKNOWN" );
    }
    if( tsc_over_crystal_numer ) {
      Log(
        "tsc frequency over core crystal clock frequency: ( %u / %u ) = %f",
        tsc_over_crystal_numer,
        tsc_over_crystal_denom,
        tsc_over_crystal
        );
    } else {
      Log( "tsc frequency over core crystal clock frequency: UNKNOWN" );
    }
    if( crystal_freq && tsc_over_crystal_numer ) {
      Log( "tsc frequency ( Hz ): %f", tsc_freq );
    } else {
      Log( "tsc frequency ( Hz ): UNKNOWN" );
    }
  }
  if( info.len > 88 ) {
    auto base_freq = info.mem[88] & AllOnes( 16 );
    auto max_freq = info.mem[89] & AllOnes( 16 );
    auto bus_freq = info.mem[90] & AllOnes( 16 );
    Log( "base frequency ( MHz ): %u", base_freq );
    Log( "max frequency ( MHz ): %u", max_freq );
    Log( "bus frequency ( MHz ): %u", bus_freq );

    // NOTE: we can't rely solely on this, since not all processors with TSC have this info.
    if( !init_sec_per_tsc ) {
      init_sec_per_tsc = 1;
      g_sec_per_tsc32 = 1.0f / Cast( f32, 1000000 * Cast( u64, base_freq ) );
      g_sec_per_tsc64 = 1.0f / Cast( f64, 1000000 * Cast( u64, base_freq ) );
    }
  }
  if( extinfo.len > 4 ) {
    Log( "LZCNT: %X", ( extinfo.mem[6] >> 5 ) & 1 );
    Log( "execute disable bit: %X", ( extinfo.mem[7] >> 20 ) & 1 );
    Log( "1GB pages: %X", ( extinfo.mem[7] >> 26 ) & 1 );
    Log( "RDTSCP: %X", ( extinfo.mem[7] >> 27 ) & 1 );
  }
  if( extinfo.len > 24 ) {
    Log( "cache line size ( B ): %u", ( extinfo.mem[26] >> 0 ) & AllOnes( 8 ) );
    Log( "L2 cache size per core ( KB ): %u", ( extinfo.mem[26] >> 16 ) & AllOnes( 16 ) );
    auto l2_assoc = ( extinfo.mem[26] >> 12 ) & AllOnes( 3 );
    switch( l2_assoc ) {
      case 0x0: Log( "L2 cache associativity: disabled" ); break;
      case 0x1: Log( "L2 cache associativity: direct-mapped" ); break;
      case 0x2: Log( "L2 cache associativity: 2-way" ); break;
      case 0x4: Log( "L2 cache associativity: 4-way" ); break;
      case 0x6: Log( "L2 cache associativity: 8-way" ); break;
      case 0x8: Log( "L2 cache associativity: 16-way" ); break;
      case 0xF: Log( "L2 cache associativity: fully-associative" ); break;
      default:  Log( "L2 cache associativity: UNKNOWN" ); break;
    }
  }
  if( extinfo.len > 28 ) {
    Log( "invariant TSC: %X", ( extinfo.mem[31] >> 8 ) & 1 );
  }
  if( extinfo.len > 32 ) {
    Log( "physical address bits: %u", ( extinfo.mem[32] >> 0 ) & AllOnes( 8 ) );
    Log( "linear address bits: %u", ( extinfo.mem[32] >> 8 ) & AllOnes( 8 ) );
  }
  Free( info );
  Free( extinfo );

  // NOTE: this is a last-chance measure.
  //   we're just stalling 0.5 seconds on startup :(
  //   waitiable timer is much more accurate timing mechanism than TimeSleep, so we could reduce that timing.
  if( !init_sec_per_tsc ) {
    init_sec_per_tsc = 1;

    Log( "WARNING: Falling back to slow sec_per_tsc calculation on startup" );

    u64 tsc_dticks;
    u64 qpc_period;
    u64 qpc_start;
    u64 tsc_start = TimeTSC();
    QueryPerformanceCounter( Cast( LARGE_INTEGER*, &qpc_start ) );
    Forever {
      TimeSleep( 1 );

      u64 qpc_cur;
      QueryPerformanceCounter( Cast( LARGE_INTEGER*, &qpc_cur ) );
      u64 qpc_dticks = qpc_cur - qpc_start;
      if( qpc_dticks > ( qpc_per_sec / 2 ) ) {
        tsc_dticks = TimeTSC() - tsc_start;
        qpc_period = qpc_dticks;
        break;
      }
    }
    g_sec_per_tsc32 = Cast( f32, qpc_period ) / ( Cast( f32, tsc_dticks ) * Cast( f32, qpc_per_sec ) );
    g_sec_per_tsc64 = Cast( f64, qpc_period ) / ( Cast( f64, tsc_dticks ) * Cast( f64, qpc_per_sec ) );
  }

  Log( "large page size: %llu", GetLargePageMinimum() );

  TIMECAPS time_caps;
  MMRESULT res = timeGetDevCaps( &time_caps, sizeof( TIMECAPS ) );
  g_has_sleep_prec_period_min = ( res == MMSYSERR_NOERROR );
  if( g_has_sleep_prec_period_min ) {
    g_sleep_prec_period_min = time_caps.wPeriodMin;
    AssertWarn( timeBeginPeriod( g_sleep_prec_period_min ) == TIMERR_NOERROR );
  }

  u64 t1 = __rdtsc();
  Log( "TimeInit: %f", ( t1 - t0 ) * g_sec_per_tsc64 );

  LogAddIndent( -1 );
  Log( "" );
}


void
TimeKill()
{
  if( g_has_sleep_prec_period_min ) {
    AssertWarn( timeEndPeriod( g_sleep_prec_period_min ) == TIMERR_NOERROR );
  }
}


void
TimeDate( u8* dst, idx_t dst_len, idx_t* written_size )
{
  time_t time_raw;
  struct tm time_data;

  time( &time_raw );
  localtime_s( &time_data, &time_raw );
  dst[0] = 0;

  u8* date_str = Str( "%y.%m.%d.%H.%M.%S" ); // Has 19 chars, not counting nul-terminator.
  idx_t date_str_len = ( 19 + 1 ); // +1 for nul-terminator.
  idx_t written = strftime(
    Cast( char*, dst ),
    MIN( dst_len, date_str_len ),
    Cast( char*, date_str ),
    &time_data
    );
  *written_size = written;
}


// NOTE: don't use for wall-clock time, because RDTSC supposedly can't accomplish that.
Inl f32
TimeSecFromTSC32( u64 delta )
{
  return Cast( f32, delta ) * g_sec_per_tsc32;
}

Inl f64
TimeSecFromTSC64( u64 delta )
{
  return Cast( f64, delta ) * g_sec_per_tsc64;
}




// wall-clock time that can be converted to seconds via SecFromClocksX.
Inl u64
TimeClock()
{
  u64 qpc;
  QueryPerformanceCounter( Cast( LARGE_INTEGER*, &qpc ) );
  return qpc;
}


Inl f32
TimeSecFromClocks32( u64 delta )
{
  return Cast( f32, delta ) * g_sec_per_qpc32;
}

Inl f64
TimeSecFromClocks64( u64 delta )
{
  return Cast( f64, delta ) * g_sec_per_qpc64;
}





static volatile s32 g_tls_handle = 0;

struct
tls_t
{
  plist_t temp;
};

void
Init( tls_t* tls )
{
  Init( tls->temp, 32768 );
}

void
Kill( tls_t* tls )
{
  Kill( tls->temp );
}

void
ThreadInit()
{
  AssertWarn( g_tls_handle != TLS_OUT_OF_INDEXES );
  auto tls = MemHeapAlloc( tls_t, 1 );
  Init( tls );
  AssertWarn( TlsSetValue( g_tls_handle, tls ) );
}

void
ThreadKill()
{
  AssertWarn( g_tls_handle != TLS_OUT_OF_INDEXES );
  auto tls = Cast( tls_t*, TlsGetValue( g_tls_handle ) );
  AssertWarn( tls );
  Kill( tls );
  MemHeapFree( tls );
}

Inl tls_t*
GetTls()
{
  auto tls = Cast( tls_t*, TlsGetValue( g_tls_handle ) );
  AssertWarn( tls );
  return tls;
}


// TODO: log failures of Execute.

s32
Execute( slice_t command, pagearray_t<u8>& output, bool show_window )
{
  s32 r = 0;

  HANDLE child_stdout_r = 0;
  HANDLE child_stdout_w = 0;

  SECURITY_ATTRIBUTES security;
  security.nLength = sizeof( SECURITY_ATTRIBUTES );
  security.bInheritHandle = TRUE;
  security.lpSecurityDescriptor = 0;

  BOOL res;
  res = CreatePipe( &child_stdout_r, &child_stdout_w, &security, 0 );
  AssertWarn( res );
  res = SetHandleInformation( child_stdout_r, HANDLE_FLAG_INHERIT, 0 );
  AssertWarn( res );

  u8 com[32768]; // TODO: too big for stack.
  if( command.len + 1 >= _countof( com ) ) {
    auto str = SliceFromCStr( "input command is too long!\r\n" );
    Memmove( AddBack( output, str.len ), ML( str ) );
    CloseHandle( child_stdout_r );
    CloseHandle( child_stdout_w );
    return -1;
  }
  CsCopy( com, ML( command ) );

  PROCESS_INFORMATION process = { 0 };
  STARTUPINFO startup = { 0 };
  startup.cb = sizeof( STARTUPINFO );
  startup.hStdError = child_stdout_w;
  startup.hStdOutput = child_stdout_w;
  startup.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
  startup.wShowWindow = show_window  ?  SW_SHOW  :  SW_HIDE;
  startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

  BOOL created = CreateProcess(
    0,
    Cast( char*, com ),
    0,
    0,
    TRUE,
    0,
    0,
    0,
    &startup,
    &process
    );

  if( !created ) {
    auto str = SliceFromCStr( "can't find command executable!\r\n" );
    Memmove( AddBack( output, str.len ), ML( str ) );
    r = -2;
  }

  // Close the parent attachment to the write half of the pipe.
  CloseHandle( child_stdout_w );

  // wait for exit.
  if( created ) {
    DWORD waited = WaitForSingleObject( process.hProcess, INFINITE );
    AssertWarn( waited == WAIT_OBJECT_0 );
  }

  // pull out the process's output:
  if( created ) {
    Forever {
      u8 pipebuf[4096];
      DWORD nread;
      BOOL read_res = ReadFile( child_stdout_r, pipebuf, _countof( pipebuf ), &nread, 0 );
      if( !read_res || !nread ) {
        break;
      }
      Memmove( AddBack( output, nread ), pipebuf, nread );
    }
  }
  CloseHandle( child_stdout_r );

  // get the process's exit code.
  if( created ) {
    DWORD exit = 0;
    BOOL got_exit = GetExitCodeProcess( process.hProcess, &exit );
    if( !got_exit ) {
      auto str = SliceFromCStr( "failed to get process exit code!\r\n" );
      Memmove( AddBack( output, str.len ), ML( str ) );
      r = -3;
    } elif( exit == STILL_ACTIVE ) {
      auto str = SliceFromCStr( "process still open after waiting for exit!\r\n" );
      Memmove( AddBack( output, str.len ), ML( str ) );
      AssertWarn( !"process still open after waiting for exit!" );
      r = -4;
    } else {
      r = exit;
    }
  }

  // clean up the process
  if( created ) {
    CloseHandle( process.hProcess );
    CloseHandle( process.hThread );
  }
  return r;
}






Inl void
PinThreadToOneCore()
{
  DWORD_PTR process_mask, system_mask;
  CompileAssert( sizeof( DWORD_PTR ) == sizeof( sidx_t ) );
  AssertWarn( GetProcessAffinityMask( GetCurrentProcess(), &process_mask, &system_mask ) );
  AssertWarn( process_mask );
  bool found = 0;
  ReverseFor( i, 0, 64 ) {
    if( ( process_mask >> i ) & 1ULL ) {
      if( !found ) {
        found = 1;
      } else {
        process_mask &= ~( 1ULL << i );
      }
    }
  }
  AssertWarn( process_mask );
  AssertWarn( !( process_mask & ( process_mask - 1 ) ) ); // pow of 2

  u64 prev_thread_mask = SetThreadAffinityMask( GetCurrentThread(), process_mask );
  AssertWarn( prev_thread_mask );
}


#define CAS( dst, compare, exchange ) \
  ( compare == InterlockedCompareExchange( dst, exchange, compare ) )


#define GetValueBeforeAtomicInc( dst ) \
  ( InterlockedIncrement( dst ) - 1 )

#define GetValueAfterAtomicInc( dst ) \
  ( InterlockedIncrement( dst ) )


typedef volatile idx_t lock_t;

Inl void
Lock( lock_t* lock )
{
  while( !CAS( lock, 0, 1 ) ) {
    _mm_pause();
  }
}

Inl void
Unlock( lock_t* lock )
{
  *lock = 0;
  _ReadWriteBarrier();
}






// Can't do this the normal way, since function pointer syntax is stupid.
typedef u32 ( __stdcall *pfn_threadproc_t )( void* misc );





// multi-reader, multi-writer circular queue, fixed elemsize.
Templ struct
queue_mrmw_t
{
  T* mem;
  volatile idx_t pos_rd;
  volatile idx_t pos_wr;
  volatile idx_t pos_wr_advance;
  idx_t capacity;
};

Templ Inl void
Zero( queue_mrmw_t<T>& queue )
{
  queue.mem = 0;
  queue.pos_rd = 0;
  queue.pos_wr = 0;
  queue.pos_wr_advance = 0;
  queue.capacity = 0;
}

Templ Inl void
Alloc( queue_mrmw_t<T>& queue, idx_t size )
{
  Zero( queue );
  queue.mem = MemHeapAlloc( T, size );
  queue.capacity = size;
}

Templ Inl void
Free( queue_mrmw_t<T>& queue )
{
  MemHeapFree( queue.mem  );
  Zero( queue );
}

Templ Inl void
EnqueueM( queue_mrmw_t<T>& queue, T* src, bool* success )
{
  Forever {
    idx_t local_wr = queue.pos_wr;
    idx_t local_wr_advance = queue.pos_wr_advance;
    if( local_wr_advance != local_wr ) {
      continue;
    }
    idx_t new_wr = ( local_wr + 1 ) % queue.capacity;
    idx_t new_wr_advance = ( local_wr_advance + 1 ) % queue.capacity;
    if( new_wr == queue.pos_rd ) {
      *success = 0;
      return;
    }
    if( CAS( &queue.pos_wr_advance, local_wr_advance, new_wr_advance ) ) {
      queue.mem[new_wr] = *src;
      _ReadWriteBarrier();
      AssertCrash( queue.pos_wr == local_wr );
      queue.pos_wr = new_wr;
      break;
    }
  }
  *success = 1;
}

Templ Inl void
DequeueM( queue_mrmw_t<T>& queue, T* dst, bool* success )
{
  Forever {
    idx_t local_rd = queue.pos_rd;
    if( local_rd == queue.pos_wr ) {
      *success = 0;
      return;
    }
    idx_t new_rd = ( local_rd + 1 ) % queue.capacity;
    *dst = queue.mem[new_rd];
    if( CAS( &queue.pos_rd, local_rd, new_rd ) ) {
      break;
    }
  }
  *success = 1;
}




// multi-reader, single-writer circular queue, fixed elemsize.
Templ struct
queue_mrsw_t
{
  T* mem;
  volatile idx_t pos_rd;
  volatile idx_t pos_wr;
  idx_t capacity;
};

Templ Inl void
Zero( queue_mrsw_t<T>& queue )
{
  queue.mem = 0;
  queue.pos_rd = 0;
  queue.pos_wr = 0;
  queue.capacity = 0;
}

Templ Inl void
Alloc( queue_mrsw_t<T>& queue, idx_t size )
{
  Zero( queue );
  queue.mem = MemHeapAlloc( T, size );
  queue.capacity = size;
}

Templ Inl void
Free( queue_mrsw_t<T>& queue )
{
  MemHeapFree( queue.mem );
  Zero( queue );
}

Templ Inl void
EnqueueS( queue_mrsw_t<T>& queue, T* src, bool* success )
{
  idx_t local_wr = queue.pos_wr;
  local_wr = ( local_wr + 1 ) % queue.capacity;
  if( local_wr == queue.pos_rd ) {
    *success = 0;
    return;
  }
  queue.mem[local_wr] = *src;
  _ReadWriteBarrier();
  queue.pos_wr = local_wr;
  *success = 1;
}

Templ Inl void
DequeueM( queue_mrsw_t<T>& queue, T* dst, bool* success )
{
  Forever {
    idx_t local_rd = queue.pos_rd;
    if( local_rd == queue.pos_wr ) {
      *success = 0;
      return;
    }
    idx_t new_rd = ( local_rd + 1 ) % queue.capacity;
    *dst = queue.mem[new_rd];
    if( CAS( &queue.pos_rd, local_rd, new_rd ) ) {
      break;
    }
  }
  *success = 1;
}




// single-reader, multi-writer circular queue, fixed elemsize.
Templ struct
queue_srmw_t
{
  T* mem;
  volatile idx_t pos_rd;
  volatile idx_t pos_wr;
  volatile idx_t pos_wr_advance;
  idx_t capacity;
};

Templ Inl void
Zero( queue_srmw_t<T>& queue )
{
  queue.mem = 0;
  queue.pos_rd = 0;
  queue.pos_wr = 0;
  queue.pos_wr_advance = 0;
  queue.capacity = 0;
}

Templ Inl void
Alloc( queue_srmw_t<T>& queue, idx_t size )
{
  Zero( queue );
  queue.mem = MemHeapAlloc( T, size );
  queue.capacity = size;
}

Templ Inl void
Free( queue_srmw_t<T>& queue )
{
  MemHeapFree( queue.mem );
  Zero( queue );
}

Templ Inl void
EnqueueM( queue_srmw_t<T>& queue, T* src, bool* success )
{
  Forever {
    idx_t local_wr = queue.pos_wr;
    idx_t local_wr_advance = queue.pos_wr_advance;
    if( local_wr_advance != local_wr ) {
      continue;
    }
    idx_t new_wr = ( local_wr + 1 ) % queue.capacity;
    idx_t new_wr_advance = ( local_wr_advance + 1 ) % queue.capacity;
    if( new_wr == queue.pos_rd ) {
      *success = 0;
      return;
    }
    if( CAS( &queue.pos_wr_advance, local_wr_advance, new_wr_advance ) ) {
      queue.mem[new_wr] = *src;
      _ReadWriteBarrier();
      AssertCrash( queue.pos_wr == local_wr );
      queue.pos_wr = new_wr;
      break;
    }
  }
  *success = 1;
}

Templ Inl void
DequeueS( queue_srmw_t<T>& queue, T* dst, bool* success )
{
  idx_t local_rd = queue.pos_rd;
  if( local_rd == queue.pos_wr ) {
    *success = 0;
    return;
  }
  local_rd = ( local_rd + 1 ) % queue.capacity;
  *dst = queue.mem[local_rd];
  _ReadWriteBarrier();
  queue.pos_rd = local_rd;
  *success = 1;
}




// single-reader, single-writer circular queue, fixed elemsize.
Templ struct
queue_srsw_t
{
  T* mem;
  volatile idx_t pos_rd;
  volatile idx_t pos_wr;
  idx_t capacity;
};

Templ Inl void
Zero( queue_srsw_t<T>& queue )
{
  queue.mem = 0;
  queue.pos_rd = 0;
  queue.pos_wr = 0;
  queue.capacity = 0;
}

Templ Inl void
Alloc( queue_srsw_t<T>& queue, idx_t size )
{
  Zero( queue );
  queue.mem = MemHeapAlloc( T, size );
  queue.capacity = size;
}

Templ Inl void
Free( queue_srsw_t<T>& queue )
{
  MemHeapFree( queue.mem );
  Zero( queue );
}

Templ Inl void
EnqueueS( queue_srsw_t<T>& queue, T* src, bool* success )
{
  idx_t local_wr = queue.pos_wr;
  local_wr = ( local_wr + 1 ) % queue.capacity;
  if( local_wr == queue.pos_rd ) {
    *success = 0;
    return;
  }
  queue.mem[local_wr] = *src;
  _ReadWriteBarrier();
  queue.pos_wr = local_wr;
  *success = 1;
}

Templ Inl void
DequeueS( queue_srsw_t<T>& queue, T* dst, bool* success )
{
  idx_t local_rd = queue.pos_rd;
  if( local_rd == queue.pos_wr ) {
    *success = 0;
    return;
  }
  local_rd = ( local_rd + 1 ) % queue.capacity;
  *dst = queue.mem[local_rd];
  _ReadWriteBarrier();
  queue.pos_rd = local_rd;
  *success = 1;
}



struct
asyncqueue_entry_t;

#define __AsyncTask( name )    void ( name )( asyncqueue_entry_t* asyncqueue_entry )
typedef __AsyncTask( *pfn_asynctask_t );

#define __MainTaskCompleted( name )    void ( name )( void* maincontext, void* asynccontext, bool* target_valid )
typedef __MainTaskCompleted( *pfn_maintaskcompleted_t );

struct
asyncqueue_entry_t
{
  pfn_asynctask_t FnAsyncTask;
  pfn_maintaskcompleted_t FnMainTaskCompleted;
  void* asynccontext;
  void* maincontext;
};

struct
maincompletedqueue_entry_t
{
  pfn_maintaskcompleted_t FnMainTaskCompleted;
  void* maincontext;
  void* asynccontext;
};



struct
taskthread_t
{
  HANDLE wake;
  queue_srsw_t<asyncqueue_entry_t> input;


  u8 cache_line_padding_to_avoid_thrashing[64]; // last thing, since this type is packed into an array_t
};

Inl void
Kill( taskthread_t& t )
{
  Free( t.input );
}



struct
mainthread_t
{
  queue_srmw_t<maincompletedqueue_entry_t> maincompletedqueue;
  volatile HANDLE wake_asynctaskscompleted;
  volatile bool signal_quit;

  array_t<HANDLE> taskthread_handles; // TODO: fixedarray_t
  array_t<taskthread_t> taskthreads; // TODO: fixedarray_t
};

static mainthread_t g_mainthread = {};



Inl void
PushMainTaskCompleted( asyncqueue_entry_t* ae )
{
  // pass down the asynccontext, so FnMainTaskCompleted can find results.
  maincompletedqueue_entry_t me;
  me.FnMainTaskCompleted = ae->FnMainTaskCompleted;
  me.maincontext = ae->maincontext;
  me.asynccontext = ae->asynccontext;

  bool success = 0;
  while( !success ) {
    EnqueueM( g_mainthread.maincompletedqueue, &me, &success );
  }

  auto r = SetEvent( g_mainthread.wake_asynctaskscompleted );
  AssertCrash( r );
}

u32 __stdcall
TaskThread( void* misc )
{
  ThreadInit();

// Only supported on Win10+.
//  HRESULT hr = SetThreadDescription( GetCurrentThread(), L"TaskThread" );
//  AssertWarn( SUCCEEDED( hr ) );

  auto taskthread = Cast( taskthread_t*, misc );

  while( !g_mainthread.signal_quit ) {
    DWORD wait = WaitForSingleObject( taskthread->wake, INFINITE );
    if( wait == WAIT_OBJECT_0 ) {
      while( !g_mainthread.signal_quit ) {

        bool success;
        asyncqueue_entry_t ae;
        DequeueS( taskthread->input, &ae, &success );
        if( !success ) {
          // this is the only reader on this queue, so we know the queue is empty here.
          break;
        }

        ae.FnAsyncTask( &ae );
      }
    } else {
      Log( "Task thread failed WaitForSingleObject with %d", wait );
    }
  }

  ThreadKill();
  return 0;
}


Inl void
PushAsyncTask( idx_t taskthreadidx, asyncqueue_entry_t entry )
{
  AssertCrash( g_mainthread.taskthreads.len );
  taskthreadidx = taskthreadidx % g_mainthread.taskthreads.len;
  auto t = g_mainthread.taskthreads.mem + taskthreadidx;

  // TODO: handle t-in-use

  bool success = 0;
  while( !success ) {
    EnqueueS( t->input, &entry, &success );
  }

  auto r = SetEvent( t->wake );
  AssertCrash( r );
}


void
MainThreadInit()
{
  g_tls_handle = TlsAlloc();
  _ReadWriteBarrier();
  AssertWarn( g_tls_handle != TLS_OUT_OF_INDEXES );

  ThreadInit();

  idx_t num_threads = 7; // TODO: cpu info

  g_mainthread.signal_quit = 0;

  g_mainthread.wake_asynctaskscompleted = CreateEvent( 0, 0, 0, 0 );
  AssertCrash( g_mainthread.wake_asynctaskscompleted );

  constant idx_t c_perthread_queuesize = 256;

  Alloc( g_mainthread.maincompletedqueue, num_threads * c_perthread_queuesize );

  Alloc( g_mainthread.taskthread_handles, num_threads );
  Alloc( g_mainthread.taskthreads, num_threads );

  For( i, 0, num_threads ) {
    auto t = AddBack( g_mainthread.taskthreads );
    t->wake = CreateEvent( 0, 0, 0, 0 );
    AssertCrash( t->wake );
    Alloc( t->input, c_perthread_queuesize );
    auto handle = AddBack( g_mainthread.taskthread_handles );
    _ReadWriteBarrier();
    *handle = Cast( HANDLE, _beginthreadex( 0, 0, TaskThread, t, 0, 0 ) );
    AssertCrash( *handle );
  }
}

void
SignalQuitAndWaitForTaskThreads()
{
  g_mainthread.signal_quit = 1;
  _ReadWriteBarrier();

  ForLen( i, g_mainthread.taskthreads ) {
    auto taskthread = g_mainthread.taskthreads.mem + i;
    SetEvent( taskthread->wake );
  }

  // TODO: should we use Msg- version of this, to keep flushing our win messages ?
  DWORD waitres = WaitForMultipleObjects(
    Cast( DWORD, g_mainthread.taskthread_handles.len ),
    g_mainthread.taskthread_handles.mem,
    1,
    INFINITE
    );
  AssertCrash( waitres != WAIT_FAILED );
  AssertCrash( waitres != WAIT_TIMEOUT );
}

void
MainThreadKill()
{
  Free( g_mainthread.taskthread_handles );

  ForLen( i, g_mainthread.taskthreads ) {
    auto taskthread = g_mainthread.taskthreads.mem + i;
    Kill( *taskthread );
  }
  Free( g_mainthread.taskthreads );

  Free( g_mainthread.maincompletedqueue );

  ThreadKill();

  TlsFree( g_tls_handle );
  g_tls_handle = 0;
}



#define LOGGER_ENABLED   1

#ifndef LOGGER_ENABLED
#error make a choice for LOGGER_ENABLED; either 0 or 1.
#endif

#if !LOGGER_ENABLED

Inl void
LoggerInit()
{
}

Inl void
LoggerKill()
{
}

Inl void
LogAddIndent( s32 delta )
{
}

Inl void
LogUseConsole( bool x )
{
}

void
Log( void* cstr ... )
{
}

void
LogInline( void* cstr ... )
{
}

#else // LOGGER_ENABLED


struct
log_t
{
  embeddedarray_t<u8, 65536> buffer;
  file_t file;
  s32 indent;
  bool initialized;
  bool console_override;
};


static log_t g_log = {};

// TODO: convert to lock_t/Lock/Unlock
volatile static u64 g_log_inuse = 0;

Inl log_t*
AcquireLog()
{
  AssertCrash( g_log.initialized ); // Using the logger before it's initialized by the main thread!
  while( !CAS( &g_log_inuse, 0, 1 ) )
  {
    _mm_pause();
  }
  auto log = &g_log;
  AssertCrash( log->initialized ); // Using the logger before it's initialized by the main thread!
  return log;
}

Inl void
ReleaseLog( log_t*& log )
{
  AssertCrash( log->initialized ); // Using the logger before it's initialized by the main thread!
  _ReadWriteBarrier();
  g_log_inuse = 0;
  log = 0;
}

Inl void
LoggerInit()
{
  AssertCrash( CAS( &g_log_inuse, 0, 1 ) ); // Some other thread is using the logger before it's initialized!
  auto log = &g_log;

  fsobj_t filename;
#if 0
  FsGetCwd( filename );
#endif

  // TODO: OS abstraction.
  filename.len = GetModuleFileName( 0, Cast( LPSTR, filename.mem ), Cast( DWORD, Capacity( filename ) ) );
  filename = _StandardFilename( ML( filename ) );

  if( filename.len ) {
    auto last_slash = CsScanL( ML( filename ), '/' );
    AssertCrash( last_slash );
    filename.len = ( last_slash - filename.mem );
    filename.len += 1; // include slash.

    embeddedarray_t<u8, 64> tmp;
    TimeDate( tmp.mem, Capacity( tmp ), &tmp.len );
    AssertWarn( tmp.len );

    Memmove( AddBack( filename, 8 ), "te_logs/", 8 );
    Memmove( AddBack( filename, tmp.len ), ML( tmp ) );
    Memmove( AddBack( filename, 4 ), ".log", 4 );

    log->file = FileOpen( ML( filename ), fileopen_t::always, fileop_t::W, fileop_t::RW );

    if( log->file.loaded ) {
      FileSetEOF( log->file, 0 );
    } else {
      printf( "WARNING: failed to open log file!\n" );
      printf( "\tRedirecting log output to stdout...\n" );
    }
  } else {
    printf( "WARNING: failed to open log file!\n" );
    printf( "\tRedirecting log output to stdout...\n" );
  }

  log->initialized = 1;
  log->console_override = 0;

  _ReadWriteBarrier();
  g_log_inuse = 0;
}

Inl void
LoggerKill()
{
  AssertCrash( CAS( &g_log_inuse, 0, 1 ) ); // Some other thread is using the logger while we're destroying it!
  auto log = &g_log;

  FileFree( log->file );
  log->initialized = 0;

  _ReadWriteBarrier();
  g_log_inuse = 0;
}


Inl void
LogAddIndent( s32 delta )
{
  auto log = AcquireLog();
  log->indent += delta;
  ReleaseLog( log );
}

Inl void
LogUseConsole( bool x )
{
  auto log = AcquireLog();
  log->console_override = x;
  ReleaseLog( log );
}


void
Log( void* cstr ... )
{
  auto log = AcquireLog();

  va_list args;
  va_start( args, cstr );
  if( log->file.loaded  &&  !log->console_override ) {
    For( i, 0, log->indent ) {
      Memmove( AddBack( log->buffer ), "\t", 1 );
    }
    log->buffer.len += vsprintf_s(
      Cast( char* const, log->buffer.mem + log->buffer.len ),
      MAX( Capacity( log->buffer ), log->buffer.len ) - log->buffer.len,
      Cast( const char* const, cstr ),
      args
      );
    Memmove( AddBack( log->buffer ), "\r\n", 2 );
    FileWriteAppend( log->file, ML( log->buffer ) );
    FileSetEOF( log->file );
    log->buffer.len = 0;
  } else {
    For( i, 0, log->indent ) {
      printf( "\t" );
    }
    vprintf( Cast( const char* const, cstr ), args );
    printf( "\r\n" );
  }
  va_end( args );

  ReleaseLog( log );
}


void
LogInline( void* cstr ... )
{
  auto log = AcquireLog();

  va_list args;
  va_start( args, cstr );
  if( log->file.loaded  &&  !log->console_override ) {
    log->buffer.len += vsprintf_s(
      Cast( char* const, log->buffer.mem + log->buffer.len ),
      MAX( Capacity( log->buffer ), log->buffer.len ) - log->buffer.len,
      Cast( const char* const, cstr ),
      args
      );
    FileWriteAppend( log->file, ML( log->buffer ) );
    FileSetEOF( log->file );
    log->buffer.len = 0;
  } else {
    vprintf( Cast( const char* const, cstr ), args );
  }
  va_end( args );

  ReleaseLog( log );
}

#endif // LOGGER_ENABLED



#define PROF_ENABLED   0
#define PROF_ENABLED_AT_LAUNCH   0


#ifndef PROF_ENABLED
#error make a choice for PROF_ENABLED; either 0 or 1.
#endif

#ifndef PROF_ENABLED_AT_LAUNCH
#error make a choice for PROF_ENABLED_AT_LAUNCH; either 0 or 1.
#endif



#if !PROF_ENABLED

#define ProfInit() // nothing

#define ProfFunc() // nothing

#define Prof( zone_label ) // nothing

#define ProfClose( zone_label ) // nothing

#define ProfFinish() // nothing

#define ProfOutputCmdline() // nothing

#define ProfKill() // nothing


#else // PROF_ENABLED


#define PROF_ZONE_CAPACITY          128 // maximum number of zones you'll register with Prof(_ )
#define PROF_ZONE_NAME_MAX          128 // maximum string length Prof(_ ) will store and use.

#define PROF_STORE_MAX   1ULL*1024*1024*1024




#define ProfFunc() \
  prof_zone_t NAMEJOIN( prof_zone_, __LINE__ ) ( Str( __FUNCTION__ ), Str( __FILE__ ), __LINE__, __COUNTER__ )

#define Prof( name ) \
  prof_zone_t NAMEJOIN( prof_zone_, name ) ( Str( # name ), Str( __FILE__ ), __LINE__, __COUNTER__ )

#define ProfClose( name ) \
  NAMEJOIN( prof_zone_, name ).Close()



struct
prof_elem_t
{
  u64 time_elapsed;
  u8* name;
  u8* file;
  u32 line;
  u32 tid;
  u32 id;
};

volatile prof_elem_t* g_prof_buffer;
volatile idx_t g_prof_buffer_len;
volatile idx_t g_prof_pos;
volatile idx_t g_saved_prof_pos;
volatile bool g_prof_enabled;

Inl void
ProfDisable()
{
  if( g_prof_enabled ) {
    g_prof_enabled = 0;
    g_saved_prof_pos = g_prof_pos;
    g_prof_pos = g_prof_buffer_len;
  }
}

Inl void
ProfEnable()
{
  if( !g_prof_enabled ) {
    g_prof_enabled = 1;
    g_prof_pos = g_saved_prof_pos;
  }
}





#define AddProfRecord( name, time_start, file, line, id ) \
  do { \
    if( g_prof_pos < g_prof_buffer_len ) { /* avoids overflow, as long as g_prof_buffer_len isn't close to the bounds. */ \
      idx_t pos = GetValueBeforeAtomicInc( &g_prof_pos ); \
      if( pos < g_prof_buffer_len ) { \
        g_prof_buffer[pos].time_elapsed = TimeTSC() - ( time_start ); \
        g_prof_buffer[pos].name = ( name ); \
        g_prof_buffer[pos].file = file; \
        g_prof_buffer[pos].line = line; \
        g_prof_buffer[pos].tid = GetThreadIdFast(); \
        g_prof_buffer[pos].id = id; \
      } \
    } \
  } while( 0 )


struct
prof_zone_t
{
  u64 time_start;
  u8* name;
  u8* file;
  u32 line;
  u32 id;
  bool open;

  ForceInl
  prof_zone_t( u8* zonename, u8* filename, u32 lineno, u32 zoneid )
  {
    time_start = TimeTSC();
    name = zonename;
    file = filename;
    line = lineno;
    id = zoneid;
    open = 1;
  }

  ForceInl
  ~prof_zone_t()
  {
    if( open ) {
      AddProfRecord( name, time_start, file, line, id );
    }
  }

  ForceInl void
  Close()
  {
    AddProfRecord( name, time_start, file, line, id );
    open = 0;
  }
};


struct
prof_zone_data_t
{
  u8* name;
  slice_t file;
  u32 line;
  u32 tid;
  u32 id;
  u64 n_invocs;
  f64 time_total;
  f64 time_total_err;
  f64 time_mean;
  f64 time_variance;
};


// TODO: fixedarray_t ?
u32 g_n_zones = {}; // num of zones profiled.
prof_zone_data_t g_zones[PROF_ZONE_CAPACITY]; // profiling data per-zone.




Inl void
ProfZero()
{
  g_prof_buffer = 0;
  g_prof_buffer_len = 0;
  g_prof_pos = 0;
  g_saved_prof_pos = 0;

  g_n_zones = {};
  For( i, 0, PROF_ZONE_CAPACITY ) {
    g_zones[i] = {};
  }
}


Inl void
ProfInit()
{
  ProfZero();

  g_prof_buffer_len = PROF_STORE_MAX / sizeof( prof_elem_t );
  g_prof_buffer = MemVirtualAlloc( prof_elem_t, g_prof_buffer_len );
  AssertCrash( g_prof_buffer );

  #if PROF_ENABLED_AT_LAUNCH
    ProfEnable();
  #else
    ProfDisable();
  #endif
}

Inl void
ProfKill()
{
  MemVirtualFree( Cast( void*, g_prof_buffer ) );
  ProfZero();
}


Inl s32
CompareZoneData( const void* a, const void* b )
{
  auto& zone0 = *Cast( prof_zone_data_t*, a );
  auto& zone1 = *Cast( prof_zone_data_t*, b );
  s32 r = ( zone0.time_total <= zone1.time_total )  ?  1  :  -1;
  return r;
}

struct
zoneid_t
{
  u32 id;
  u32 tid;
};

HASHSET_ELEM_EQUAL( EqualNames )
{
  AssertCrash( elem_size == sizeof( u8* ) );
  bool res = !strcmp( *Cast( const char**, elem0 ), *Cast( const char**, elem1 ) );
  return res;
}

HASHSET_ELEM_HASH( ZoneLabelHash )
{
  u8* label = *Cast( u8**, elem );
  idx_t hash = 0;
  For( i, 0, elem_len ) {
    hash += label[i] * ( i + 1 );
  }
  hash += elem_len;
  return hash;
}

void
ProfFinish()
{
  // Since we disable profiling by messing with g_prof_pos, we have to reset it here for things to function.
  ProfEnable();

  // TODO: hashset iteration so I don't have to keep an extra array in sync.
  array_t<zoneid_t> set;
  Alloc( set, 256 );

  auto nelems = MIN( g_prof_pos, g_prof_buffer_len );

  hashset_t map;
  Init(
    map,
    1024,
    sizeof( zoneid_t ),
    sizeof( prof_zone_data_t ),
    0.75f,
    Equal_FirstU64,
    Hash_FirstU64
  );
  CompileAssert( sizeof( zoneid_t ) == sizeof( u64 ) ); // we use u64 hash functions, so zoneid_t better be 64bits.

  For( i, 0, nelems ) {
    auto& elem = Cast( prof_elem_t&, g_prof_buffer[i] );

    bool found;
    prof_zone_data_t* rawzone;
    zoneid_t zoneid = { elem.id, elem.tid };
    LookupRaw( map, &zoneid, &found, Cast( void**, &rawzone ) );

    if( !found ) {
      f64 time_elapsed = TimeSecFromTSC64( elem.time_elapsed );

      prof_zone_data_t newzone;
      newzone.n_invocs = 1;
      newzone.time_mean = time_elapsed;
      newzone.time_total = time_elapsed;
      newzone.time_total_err = 0;
      newzone.time_variance = 0;
      newzone.name = elem.name;
      newzone.file = SliceFromCStr( elem.file );
      newzone.line = elem.line;
      newzone.tid = elem.tid;
      newzone.id = elem.id;

      bool already_there;
      Add( map, &zoneid, &newzone, &already_there, 0, 0 );
      AssertCrash( !already_there );

      *AddBack( set ) = zoneid;

    } else {
      prof_zone_data_t& zone = *rawzone;

      // calculate the all-important value: time elapsed since start.
      f64 time_elapsed = TimeSecFromTSC64( elem.time_elapsed );

      // calculate incrementally the total time ( using kahan summation )
      f64 time_elapsed_cor = time_elapsed - zone.time_total_err; // TODO: convert to kahan64_t
      f64 time_total = zone.time_total + time_elapsed_cor;
      zone.time_total_err = ( time_total - zone.time_total ) - time_elapsed_cor;
      zone.time_total = time_total;

      // calculate incrementally the mean.
      u64 prev_n = zone.n_invocs;
      f64 rec_n = 1.0 / ( prev_n + 1 );
      f64 prev_mu = zone.time_mean;
      f64 prev_diff = time_elapsed - prev_mu;
      f64 mu = prev_mu + rec_n * prev_diff;
      zone.time_mean = mu;

      // calculate incrementally the variance.
      f64 prev_sn = prev_n * zone.time_variance;
      zone.time_variance = rec_n * ( prev_sn + prev_diff * ( time_elapsed - mu ) );

      zone.n_invocs += 1;
    }
  }

  ForLen( i, set ) {
    auto& zoneid = set.mem[i];

    bool found;
    prof_zone_data_t zone;
    Lookup( map, &zoneid, &found, &zone );
    AssertCrash( found );

    AssertCrash( g_n_zones < PROF_ZONE_CAPACITY );
    g_zones[g_n_zones++] = zone;
  }

  Kill( map );
  Free( set );

  // sort g_zones however we want:
  qsort(
    g_zones,
    g_n_zones,
    sizeof( prof_zone_data_t ),
    CompareZoneData
    );
}



#define FUNCTION_W   24
#define FUNCTION_WS "24"

#define FILE_W   15
#define FILE_WS "15"

void
ProfOutputCmdline()
{
  if( !g_n_zones ) {
    return;
  }
//  LogUseConsole( 1 );
  Log( "PROFILE" );
  LogAddIndent( +1 );
  Log( "Clocks / S: %llu", Cast( u64, 1 / g_sec_per_tsc64 ) );
  Log( "Clocks / MS: %llu", Cast( u64, 1 / ( 1000 * g_sec_per_tsc64 ) ) );
  LogInline( "%5s   ",             "ID" );
  LogInline( "%7s   ",             "THREAD" );
  LogInline( "%-" FILE_WS "s",     "FILE" );
  LogInline( "%7s   ",             "LINE" );
  LogInline( "%-" FUNCTION_WS "s", "FUNCTION" );
  LogInline( "%12s   ",            "INVOCS" );
  LogInline( "%-12s",              "TIME(S)" );
  LogInline( "%-12s",              "MEAN(MS)" );
  LogInline( "%-12s",              "SDEV(MS)" );
  LogInline( "%-12s",              "WIDTH" );
  Log( "" );
  embeddedarray_t<u8, 4096> tmp;
  For( i, 0, g_n_zones ) {
    auto& zone = g_zones[i];
    if( zone.n_invocs ) {
      tmp.len = 0;

      LogInline( "%5u   ", zone.id );
      LogInline( "%7u   ", zone.tid );

      u8* filename = CsScanL( ML( zone.file ), '\\' );
      if( !filename )
        filename = CsScanL( ML( zone.file ), '/' );
      if( !filename )
        filename = zone.file.mem - 1;
      filename += 1; // skip slash
      auto tmpsize = MIN( CsLen( filename ), FILE_W );
      Clear( tmp );
      Memmove( AddBack( tmp, tmpsize ), filename, tmpsize );
      *AddBack( tmp ) = 0;
      LogInline( "%-" FILE_WS "s", tmp.mem );
      RemBack( tmp );

      LogInline( "%7u   ", zone.line );

      tmpsize = MIN( CsLen( zone.name ), FUNCTION_W );
      Clear( tmp );
      Memmove( AddBack( tmp, tmpsize ), zone.name, tmpsize );
      *AddBack( tmp ) = 0;
      LogInline( "%-" FUNCTION_WS "s", tmp.mem );

      LogInline( "%12llu   ", zone.n_invocs );
      LogInline( "%-12.4g", zone.time_total );
      LogInline( "%-12.4g", 1000 * zone.time_mean );
      LogInline( "%-12.4g", 1000 * Sqrt64( zone.time_variance ) );
      if( zone.time_mean != 0.0 ) {
        LogInline( "%-12.4g", Sqrt64( zone.time_variance ) / zone.time_mean );
      }
      Log( "" );
    }
  }
  LogAddIndent( -1 );
  Log( "" );
//  LogUseConsole( 0 );
}

#undef FUNCTION_W
#undef FUNCTION_WS
#undef FILE_W
#undef FILE_WS




#endif // PROF_ENABLED





// xorshift RNG.
//
struct rng_xorshift32_t
{
  u32 x;
};


// Linear congruential RNG.
//   Xn+1 = ( a * Xn + b ) mod m.
//
struct rng_lcg_t
{
  u64 m;
  u64 a;
  u64 b;
  u64 x;
  f64 unit_norm64;
  f32 unit_norm32;
};


// Mersenne Twister RNG.
//
struct rng_mt_t
{
  u32 mt [ 624 ];
  u32 idx;
};



Inl void
Init( rng_xorshift32_t& rng, u64 seed )
{
  rng.x = Cast( u32, seed );
}

Inl u32
Rand32( rng_xorshift32_t& rng )
{
  rng.x ^= rng.x << 13;
  rng.x ^= rng.x >> 17;
  rng.x ^= rng.x << 5;
  return rng.x;
}

Inl u64
Rand64( rng_xorshift32_t& rng )
{
  u64 r = ( Cast( u64, Rand32( rng ) ) << 32ULL ) | Rand32( rng );
  return r;
}

#define RAND_XORSHIFT32_UNIT_NORM32   ( 1.0f / 0xFFFFFFFFU )
#define RAND_XORSHIFT32_UNIT_NORM64   ( 1.0 / 0xFFFFFFFFU )

Inl f32
Zeta32( rng_xorshift32_t& rng )
{
  return Rand32( rng ) * RAND_XORSHIFT32_UNIT_NORM32;
}

Inl f64
Zeta64( rng_xorshift32_t& rng )
{
  return Rand32( rng ) * RAND_XORSHIFT32_UNIT_NORM64;
}



Inl void
Init( rng_lcg_t& rng, u64 seed )
{
  rng.m = 0x0000000100000000ULL; // modulus, 2^32.
  rng.a = 0x000000005851F42DULL; // in( 0, m )
  rng.b = 0x0000000014057B7DULL; // in [ 0, m )
  rng.x = 0x00000000FFFFFFFFULL & ( rng.m - seed ); // in [ 0, m )
  rng.unit_norm64 = 1.0 / ( rng.m - 1.0 );
  rng.unit_norm32 = 1.0f / ( rng.m - 1.0f );
}


Inl u32
Rand32( rng_lcg_t& rng )
{
  rng.x = 0x00000000FFFFFFFFULL & ( rng.a * rng.x + rng.b );
  return Cast( u32, rng.x );
}

Inl u64
Rand64( rng_lcg_t& rng )
{
  u64 r;
  u32* dst32 = Cast( u32*, &r );
  rng.x = 0x00000000FFFFFFFFULL & ( rng.a * rng.x + rng.b );
  *dst32++ = Cast( u32, rng.x );
  rng.x = 0x00000000FFFFFFFFULL & ( rng.a * rng.x + rng.b );
  *dst32++ = Cast( u32, rng.x );
  return r;
}

Inl f32
Zeta32( rng_lcg_t& rng )
{
  rng.x = 0x00000000FFFFFFFFULL & ( rng.a * rng.x + rng.b );
  return rng.x * rng.unit_norm32;
}

Inl f64
Zeta64( rng_lcg_t& rng )
{
  rng.x = 0x00000000FFFFFFFFULL & ( rng.a * rng.x + rng.b );
  return rng.x * rng.unit_norm64;
}



Inl void
Rand( rng_lcg_t& rng, u32* dst, u32 n )
{
  while( n-- ) {
    *dst++ = Rand32( rng );
  }
}

Inl void
Rand( rng_lcg_t& rng, u64* dst, u32 n )
{
  while( n-- ) {
    *dst++ = Rand64( rng );
  }
}


Inl void
Zeta( rng_lcg_t& rng, f32* dst, u32 n )
{
  while( n-- ) {
    *dst++ = Zeta32( rng );
  }
}

Inl void
Zeta( rng_lcg_t& rng, f64* dst, u32 n )
{
  while( n-- ) {
    *dst++ = Zeta64( rng );
  }
}






Inl void
Init( rng_mt_t& rng, u64 seed )
{
  rng.idx = 0;
  rng.mt[0] = Cast( u32, seed );
  Fori( u32, i, 1, 624 ) {
    rng.mt[i] = 0x6C078965U * ( rng.mt[i-1] ^ ( rng.mt[i-1] >> 30 ) ) + i;
  }
}


Inl u32
ComputeRand( rng_mt_t& rng )
{
  if( rng.idx == 0 ) {
    Fori( u32, i, 0, 624 ) {
      u32 y = ( rng.mt[i] & 0x80000000U ) + ( rng.mt[ (i+1) % 624 ] & 0x7FFFFFFFU );
      rng.mt[i] = rng.mt[ (i+397) % 624 ] ^ ( y >> 1 );
      if( y & 1 ) {
        rng.mt[i] ^= 0x9908B0DFU;
      }
    }
  }
  u32 z = rng.mt[rng.idx];
  rng.idx = ( rng.idx + 1 ) % 624;
  z ^= ( z >> 11 );
  z ^= ( ( z << 7 ) & 0x9D2C5680U );
  z ^= ( ( z << 15 ) & 0xEFC60000U );
  z ^= ( z >> 18 );
  return z;
}



Inl u32
Rand32( rng_mt_t& rng )
{
  return ComputeRand( rng );
}

Inl u64
Rand64( rng_mt_t& rng )
{
  u64 r;
  u32* dst32 = Cast( u32*, &r );
  *dst32++ = ComputeRand( rng );
  *dst32++ = ComputeRand( rng );
  return r;
}


#define RAND_MT_UNIT_NORM32   ( 1.0f / 0xFFFFFFFFU )
#define RAND_MT_UNIT_NORM64   ( 1.0 / 0xFFFFFFFFU )

Inl f32
Zeta32( rng_mt_t& rng )
{
  return ComputeRand( rng ) * RAND_MT_UNIT_NORM32;
}

Inl f64
Zeta64( rng_mt_t& rng )
{
  return ComputeRand( rng ) * RAND_MT_UNIT_NORM64;
}




Inl void
Rand( rng_mt_t& rng, u32* dst, u32 n )
{
  while( n-- ) {
    *dst++ = ComputeRand( rng );
  }
}

Inl void
Rand( rng_mt_t& rng, u64* dst, u32 n )
{
  u32* dst32 = Cast( u32*, dst );
  while( n-- ) {
    *dst32++ = ComputeRand( rng );
    *dst32++ = ComputeRand( rng );
  }
}


Inl void
Zeta( rng_mt_t& rng, f32* dst, u32 n )
{
  while( n-- ) {
    *dst++ = ComputeRand( rng ) * RAND_MT_UNIT_NORM32;
  }
}

Inl void
Zeta( rng_mt_t& rng, f64* dst, u32 n )
{
  while( n-- ) {
    *dst++ = ComputeRand( rng ) * RAND_MT_UNIT_NORM64;
  }
}




#define __OnMainKill( name ) \
  void ( name )( void* user )

typedef __OnMainKill( *pfn_onmainkill_t );

struct
onmainkill_entry_t
{
  pfn_onmainkill_t fn;
  void* user;
};

static array_t<onmainkill_entry_t> g_onmainkill_entries = {};

Inl void
RegisterOnMainKill(
  pfn_onmainkill_t fn,
  void* user
  )
{
  onmainkill_entry_t entry;
  entry.fn = fn;
  entry.user = user;
  *AddBack( g_onmainkill_entries ) = entry;
}



typedef BOOL ( WINAPI *pfn_minidumpwritedump_t )(
  HANDLE hProcess,
  DWORD dwPid,
  HANDLE hFile,
  MINIDUMP_TYPE DumpType,
  PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
  PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
  PMINIDUMP_CALLBACK_INFORMATION CallbackParam
  );

void
WriteDump( _EXCEPTION_POINTERS* exc_ptrs )
{
  HMODULE dbghelp_dll = LoadLibraryA( "dbghelp.dll" );
  pfn_minidumpwritedump_t pfn = Cast( pfn_minidumpwritedump_t, GetProcAddress( dbghelp_dll, "MiniDumpWriteDump" ) );

  fsobj_t filename;

  filename.len = GetModuleFileName( 0, Cast( LPSTR, filename.mem ), Cast( DWORD, Capacity( filename ) ) );
  filename = _StandardFilename( ML( filename ) );

  if( filename.len ) {
    auto last_slash = CsScanL( ML( filename ), '/' );
    AssertCrash( last_slash );
    filename.len = ( last_slash - filename.mem );
    filename.len += 1; // include slash.

    embeddedarray_t<u8, 64> tmp;
    TimeDate( tmp.mem, Capacity( tmp ), &tmp.len );
    AssertWarn( tmp.len );

    Memmove( AddBack( filename, 8 ), "te_logs/", 8 );
    Memmove( AddBack( filename, tmp.len ), ML( tmp ) );
    Memmove( AddBack( filename, 4 ), ".dmp", 4 );

    *AddBack( filename ) = 0;

    HANDLE hFile = CreateFileA(
      Cast( LPCSTR, filename.mem ),
      GENERIC_WRITE,
      FILE_SHARE_WRITE,
      0,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      0
      );

    _MINIDUMP_EXCEPTION_INFORMATION exc_info;
    exc_info.ThreadId = GetCurrentThreadId();
    exc_info.ExceptionPointers = exc_ptrs;
    exc_info.ClientPointers = 0; // TODO: should this be 1 ?

    pfn(
      GetCurrentProcess(),
      GetCurrentProcessId(),
      hFile,
      Cast( MINIDUMP_TYPE, MiniDumpWithFullMemory | MiniDumpIgnoreInaccessibleMemory | MiniDumpWithThreadInfo ),
      &exc_info,
      0,
      0
      );

    CloseHandle( hFile );
  }
}

LONG WINAPI
UnhandledSEHWriteDump( _EXCEPTION_POINTERS* exc_ptrs )
{
  WriteDump( exc_ptrs );
//  return EXCEPTION_CONTINUE_SEARCH;
  return EXCEPTION_EXECUTE_HANDLER;
}


void __cdecl
HandleCRTPureCall()
{
  s32 crazy_vftable_empty_function_invoked = 0;
  AssertCrash( crazy_vftable_empty_function_invoked );
}


void HandleCRTInvalidParameter(
  const wchar_t* expression,
  const wchar_t* function,
  const wchar_t* file,
  u32 line,
  idx_t reserved
  )
{
  s32 crt_function_given_invalid_param = 0;
  AssertCrash( crt_function_given_invalid_param );
}






Inl void
MainInit()
{
  CrtShowMemleaks();

  SetUnhandledExceptionFilter( UnhandledSEHWriteDump );
  _set_purecall_handler( HandleCRTPureCall );
  _set_invalid_parameter_handler( HandleCRTInvalidParameter );

  ProfInit();
  LoggerInit();
  MainThreadInit();
  TimeInit();

  Alloc( g_onmainkill_entries, 16 );

#if DBG
  AllocConsole();
  FILE* tmp;
  freopen_s( &tmp, "CONOUT$", "wb", stdout );
  freopen_s( &tmp, "CONOUT$", "wb", stderr );
#endif
}

Inl void
MainKill()
{
  ReverseForLen( i, g_onmainkill_entries ) {
    auto entry = g_onmainkill_entries.mem + i;
    entry->fn( entry->user );
  }
  Free( g_onmainkill_entries );

  ProfFinish();
  ProfOutputCmdline();
  ProfKill();
  LoggerKill();
  TimeKill();
  MainThreadKill();

#if FINDLEAKS
  if( allocs_len != frees_len ) {
    hashset_t h;
    Init(
      h,
      10000,
      8,
      0,
      0.9f,
      Equal_FirstU64,
      Hash_FirstU64
      );

    For( i, 0, frees_len ) {
      Add( h, frees + i, 0, 0, 0, 0 );
    }
    For( i, 0, allocs_len ) {
      bool found = 0;
      LookupRaw( h, allocs + i, &found, 0 );
      if( !found ) {
        u64 alloc = allocs[i];
        printf( "Leaked allocation # %llu\n", alloc );
        __debugbreak();
      }
    }
    Kill( h );
  }
#endif
}




#define OPENGL_INSTEAD_OF_SOFTWARE       0
#define GLW_RAWINPUT_KEYBOARD            0
#define GLW_RAWINPUT_KEYBOARD_NOHOTKEY   1

// COMPILE-TIME OPTIONS

#ifndef OPENGL_INSTEAD_OF_SOFTWARE
#error make a choice for OPENGL_INSTEAD_OF_SOFTWARE; either 0 or 1.
#endif

#ifndef GLW_RAWINPUT_KEYBOARD
#error make a choice for GLW_RAWINPUT_KEYBOARD; either 0 or 1.
#endif

#ifndef GLW_RAWINPUT_KEYBOARD_NOHOTKEY
#error make a choice for GLW_RAWINPUT_KEYBOARD_NOHOTKEY; either 0 or 1.
#endif





#if OPENGL_INSTEAD_OF_SOFTWARE

  #include <gl/gl.h>
  #include "glw_glext.h"
  #include "glw_wglext.h"

  #pragma comment( lib, "opengl32" )

  #define glVerify() \
    do { \
      u32 err = glGetError(); \
      AssertWarn( !err ); \
    } while( 0 )

#endif


#include "glw_font_stb_truetype.h"





// ==============================================================================
//
// WINDOWING / INPUT INTERFACE:
//

Enumc( glwmousebtn_t )
{
  none = 0,
  l,
  r,
  m,
  b4,
  b5,

  count
};


struct
glwkeylocks_t
{
  bool caps;
  bool num;
  bool scroll;
};



#define KEYVALMAP( _x ) \
  _x( none             ,   0 , 0               ) \
  _x( backspace        ,   1 , VK_BACK         ) \
  _x( tab              ,   2 , VK_TAB          ) \
  _x( clear            ,   3 , VK_CLEAR        ) \
  _x( enter            ,   4 , VK_RETURN       ) \
  _x( shift            ,   5 , VK_SHIFT        ) \
  _x( ctrl             ,   6 , VK_CONTROL      ) \
  _x( alt              ,   7 , VK_MENU         ) \
  _x( pause            ,   8 , VK_PAUSE        ) \
  _x( capslock         ,   9 , VK_CAPITAL      ) \
  _x( esc              ,  10 , VK_ESCAPE       ) \
  _x( space            ,  11 , VK_SPACE        ) \
  _x( page_u           ,  12 , VK_PRIOR        ) \
  _x( page_d           ,  13 , VK_NEXT         ) \
  _x( end              ,  14 , VK_END          ) \
  _x( home             ,  15 , VK_HOME         ) \
  _x( arrow_l          ,  16 , VK_LEFT         ) \
  _x( arrow_u          ,  17 , VK_UP           ) \
  _x( arrow_r          ,  18 , VK_RIGHT        ) \
  _x( arrow_d          ,  19 , VK_DOWN         ) \
  _x( select           ,  20 , VK_SELECT       ) \
  _x( print            ,  21 , VK_PRINT        ) \
  _x( printscrn        ,  22 , VK_SNAPSHOT     ) \
  _x( insert           ,  23 , VK_INSERT       ) \
  _x( del              ,  24 , VK_DELETE       ) \
  _x( help             ,  25 , VK_HELP         ) \
  _x( numlock          ,  26 , VK_NUMLOCK      ) \
  _x( scrolllock       ,  27 , VK_SCROLL       ) \
  _x( shift_l          ,  28 , VK_LSHIFT       ) \
  _x( shift_r          ,  29 , VK_RSHIFT       ) \
  _x( ctrl_l           ,  30 , VK_LCONTROL     ) \
  _x( ctrl_r           ,  31 , VK_RCONTROL     ) \
  _x( alt_l            ,  32 , VK_LMENU        ) \
  _x( alt_r            ,  33 , VK_RMENU        ) \
  _x( volume_mute      ,  34 , VK_VOLUME_MUTE  ) \
  _x( volume_u         ,  35 , VK_VOLUME_UP    ) \
  _x( volume_d         ,  36 , VK_VOLUME_DOWN  ) \
  _x( semicolon        ,  37 , VK_OEM_1        ) \
  _x( equals           ,  38 , VK_OEM_PLUS     ) \
  _x( minus            ,  39 , VK_OEM_MINUS    ) \
  _x( comma            ,  40 , VK_OEM_COMMA    ) \
  _x( period           ,  41 , VK_OEM_PERIOD   ) \
  _x( slash_forw       ,  42 , VK_OEM_2        ) \
  _x( slash_back       ,  43 , VK_OEM_5        ) \
  _x( tilde            ,  44 , VK_OEM_3        ) \
  _x( brace_l          ,  45 , VK_OEM_4        ) \
  _x( brace_r          ,  46 , VK_OEM_6        ) \
  _x( quote            ,  47 , VK_OEM_7        ) \
  _x( num_0            ,  48 , 0x30            ) \
  _x( num_1            ,  49 , 0x31            ) \
  _x( num_2            ,  50 , 0x32            ) \
  _x( num_3            ,  51 , 0x33            ) \
  _x( num_4            ,  52 , 0x34            ) \
  _x( num_5            ,  53 , 0x35            ) \
  _x( num_6            ,  54 , 0x36            ) \
  _x( num_7            ,  55 , 0x37            ) \
  _x( num_8            ,  56 , 0x38            ) \
  _x( num_9            ,  57 , 0x39            ) \
  _x( a                ,  58 , 0x41            ) \
  _x( b                ,  59 , 0x42            ) \
  _x( c                ,  60 , 0x43            ) \
  _x( d                ,  61 , 0x44            ) \
  _x( e                ,  62 , 0x45            ) \
  _x( f                ,  63 , 0x46            ) \
  _x( g                ,  64 , 0x47            ) \
  _x( h                ,  65 , 0x48            ) \
  _x( i                ,  66 , 0x49            ) \
  _x( j                ,  67 , 0x4A            ) \
  _x( k                ,  68 , 0x4B            ) \
  _x( l                ,  69 , 0x4C            ) \
  _x( m                ,  70 , 0x4D            ) \
  _x( n                ,  71 , 0x4E            ) \
  _x( o                ,  72 , 0x4F            ) \
  _x( p                ,  73 , 0x50            ) \
  _x( q                ,  74 , 0x51            ) \
  _x( r                ,  75 , 0x52            ) \
  _x( s                ,  76 , 0x53            ) \
  _x( t                ,  77 , 0x54            ) \
  _x( u                ,  78 , 0x55            ) \
  _x( v                ,  79 , 0x56            ) \
  _x( w                ,  80 , 0x57            ) \
  _x( x                ,  81 , 0x58            ) \
  _x( y                ,  82 , 0x59            ) \
  _x( z                ,  83 , 0x5A            ) \
  _x( numpad_0         ,  84 , VK_NUMPAD0      ) \
  _x( numpad_1         ,  85 , VK_NUMPAD1      ) \
  _x( numpad_2         ,  86 , VK_NUMPAD2      ) \
  _x( numpad_3         ,  87 , VK_NUMPAD3      ) \
  _x( numpad_4         ,  88 , VK_NUMPAD4      ) \
  _x( numpad_5         ,  89 , VK_NUMPAD5      ) \
  _x( numpad_6         ,  90 , VK_NUMPAD6      ) \
  _x( numpad_7         ,  91 , VK_NUMPAD7      ) \
  _x( numpad_8         ,  92 , VK_NUMPAD8      ) \
  _x( numpad_9         ,  93 , VK_NUMPAD9      ) \
  _x( numpad_mul       ,  94 , VK_MULTIPLY     ) \
  _x( numpad_add       ,  95 , VK_ADD          ) \
  _x( numpad_sub       ,  96 , VK_SUBTRACT     ) \
  _x( numpad_decimal   ,  97 , VK_DECIMAL      ) \
  _x( numpad_div       ,  98 , VK_DIVIDE       ) \
  _x( fn_1             ,  99 , VK_F1           ) \
  _x( fn_2             , 100 , VK_F2           ) \
  _x( fn_3             , 101 , VK_F3           ) \
  _x( fn_4             , 102 , VK_F4           ) \
  _x( fn_5             , 103 , VK_F5           ) \
  _x( fn_6             , 104 , VK_F6           ) \
  _x( fn_7             , 105 , VK_F7           ) \
  _x( fn_8             , 106 , VK_F8           ) \
  _x( fn_9             , 107 , VK_F9           ) \
  _x( fn_10            , 108 , VK_F10          ) \
  _x( fn_11            , 109 , VK_F11          ) \
  _x( fn_12            , 110 , VK_F12          ) \
  _x( count            , 111 , 0               ) \

Enumc( glwkey_t )
{
  #define ENTRY( name, value, win_value )   name = value,
  KEYVALMAP( ENTRY )
  #undef ENTRY
};


#if OPENGL_INSTEAD_OF_SOFTWARE

  #define D_GLPROC( pfunc_type, func )   pfunc_type func = {};

  D_GLPROC( PFNWGLGETEXTENSIONSSTRINGARBPROC , wglGetExtensionsStringARB  )
  D_GLPROC( PFNWGLSWAPINTERVALEXTPROC        , wglSwapIntervalEXT         )
  D_GLPROC( PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB )

  D_GLPROC( PFNGLGENFRAMEBUFFERSPROC         , glGenFramebuffers          )
  D_GLPROC( PFNGLDELETEFRAMEBUFFERSPROC      , glDeleteFramebuffers       )
  D_GLPROC( PFNGLBINDFRAMEBUFFERPROC         , glBindFramebuffer          )
  D_GLPROC( PFNGLGENRENDERBUFFERSPROC        , glGenRenderbuffers         )
  D_GLPROC( PFNGLDELETERENDERBUFFERSPROC     , glDeleteRenderbuffers      )
  D_GLPROC( PFNGLBINDRENDERBUFFERPROC        , glBindRenderbuffer         )
  D_GLPROC( PFNGLRENDERBUFFERSTORAGEPROC     , glRenderbufferStorage      )
  D_GLPROC( PFNGLFRAMEBUFFERRENDERBUFFERPROC , glFramebufferRenderbuffer  )
  D_GLPROC( PFNGLFRAMEBUFFERTEXTURE2DPROC    , glFramebufferTexture2D     )
  D_GLPROC( PFNGLDRAWBUFFERSPROC             , glDrawBuffers              )
  D_GLPROC( PFNGLCHECKFRAMEBUFFERSTATUSPROC  , glCheckFramebufferStatus   )
  D_GLPROC( PFNGLCREATEPROGRAMPROC           , glCreateProgram            )
  D_GLPROC( PFNGLCREATESHADERPROC            , glCreateShader             )
  D_GLPROC( PFNGLGETUNIFORMLOCATIONPROC      , glGetUniformLocation       )
  D_GLPROC( PFNGLSHADERSOURCEPROC            , glShaderSource             )
  D_GLPROC( PFNGLCOMPILESHADERPROC           , glCompileShader            )
  D_GLPROC( PFNGLATTACHSHADERPROC            , glAttachShader             )
  D_GLPROC( PFNGLLINKPROGRAMPROC             , glLinkProgram              )
  D_GLPROC( PFNGLUSEPROGRAMPROC              , glUseProgram               )
  D_GLPROC( PFNGLVALIDATEPROGRAMPROC         , glValidateProgram          )
  D_GLPROC( PFNGLUNIFORMMATRIX4FVPROC        , glUniformMatrix4fv         )
  D_GLPROC( PFNGLUNIFORM1IPROC               , glUniform1i                )
  D_GLPROC( PFNGLUNIFORM4FVPROC              , glUniform4fv               )
  D_GLPROC( PFNGLGETPROGRAMIVPROC            , glGetProgramiv             )
  D_GLPROC( PFNGLGETPROGRAMINFOLOGPROC       , glGetProgramInfoLog        )
  D_GLPROC( PFNGLGENBUFFERSPROC              , glGenBuffers               )
  D_GLPROC( PFNGLBINDBUFFERPROC              , glBindBuffer               )
  D_GLPROC( PFNGLBUFFERDATAPROC              , glBufferData               )
  D_GLPROC( PFNGLENABLEVERTEXATTRIBARRAYPROC , glEnableVertexAttribArray  )
  D_GLPROC( PFNGLVERTEXATTRIBPOINTERPROC     , glVertexAttribPointer      )
  D_GLPROC( PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray )
  D_GLPROC( PFNGLDELETEBUFFERSPROC           , glDeleteBuffers            )
  D_GLPROC( PFNGLGETATTRIBLOCATIONPROC       , glGetAttribLocation        )
  D_GLPROC( PFNGLGENVERTEXARRAYSPROC         , glGenVertexArrays          )
  D_GLPROC( PFNGLBINDVERTEXARRAYPROC         , glBindVertexArray          )
  D_GLPROC( PFNGLDELETEVERTEXARRAYSPROC      , glDeleteVertexArrays       )
  D_GLPROC( PFNGLACTIVETEXTUREPROC           , glActiveTexture            )
  D_GLPROC( PFNGLGENERATEMIPMAPPROC          , glGenerateMipmap           )
  D_GLPROC( PFNGLDELETESHADERPROC            , glDeleteShader             )
  D_GLPROC( PFNGLDELETEPROGRAMPROC           , glDeleteProgram            )
  D_GLPROC( PFNGLGETSHADERIVPROC             , glGetShaderiv              )
  D_GLPROC( PFNGLGETSHADERINFOLOGPROC        , glGetShaderInfoLog         )

  #undef D_GLPROC

  #define L_GLPROC( pfunc_type, func ) \
    do { \
      func = Cast( pfunc_type, wglGetProcAddress( # func ) ); \
    } while( 0 );


  void
  LoadWGLFunctions( HDC dc )
  {
    ProfFunc();
    // load wgl functions from a dummy context.
    HGLRC dummy_context = wglCreateContext( dc );
    AssertWarn( dummy_context );
    AssertWarn( wglMakeCurrent( dc, dummy_context ) );

    L_GLPROC( PFNWGLGETEXTENSIONSSTRINGARBPROC , wglGetExtensionsStringARB  )
    L_GLPROC( PFNWGLSWAPINTERVALEXTPROC        , wglSwapIntervalEXT         )
    L_GLPROC( PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB )

    AssertWarn( wglMakeCurrent( 0, 0 ) );
    AssertWarn( wglDeleteContext( dummy_context ) );
  }

  void
  LoadOpenGLFunctions( HDC dc )
  {
    ProfFunc();
  // load gl functions after we've created the right wgl context
  L_GLPROC( PFNGLGENFRAMEBUFFERSPROC         , glGenFramebuffers          )
  L_GLPROC( PFNGLDELETEFRAMEBUFFERSPROC      , glDeleteFramebuffers       )
  L_GLPROC( PFNGLBINDFRAMEBUFFERPROC         , glBindFramebuffer          )
  L_GLPROC( PFNGLGENRENDERBUFFERSPROC        , glGenRenderbuffers         )
  L_GLPROC( PFNGLDELETERENDERBUFFERSPROC     , glDeleteRenderbuffers      )
  L_GLPROC( PFNGLBINDRENDERBUFFERPROC        , glBindRenderbuffer         )
  L_GLPROC( PFNGLRENDERBUFFERSTORAGEPROC     , glRenderbufferStorage      )
  L_GLPROC( PFNGLFRAMEBUFFERRENDERBUFFERPROC , glFramebufferRenderbuffer  )
  L_GLPROC( PFNGLFRAMEBUFFERTEXTURE2DPROC    , glFramebufferTexture2D     )
  L_GLPROC( PFNGLDRAWBUFFERSPROC             , glDrawBuffers              )
  L_GLPROC( PFNGLCHECKFRAMEBUFFERSTATUSPROC  , glCheckFramebufferStatus   )
  L_GLPROC( PFNGLCREATEPROGRAMPROC           , glCreateProgram            )
  L_GLPROC( PFNGLCREATESHADERPROC            , glCreateShader             )
  L_GLPROC( PFNGLGETUNIFORMLOCATIONPROC      , glGetUniformLocation       )
  L_GLPROC( PFNGLSHADERSOURCEPROC            , glShaderSource             )
  L_GLPROC( PFNGLCOMPILESHADERPROC           , glCompileShader            )
  L_GLPROC( PFNGLATTACHSHADERPROC            , glAttachShader             )
  L_GLPROC( PFNGLLINKPROGRAMPROC             , glLinkProgram              )
  L_GLPROC( PFNGLUSEPROGRAMPROC              , glUseProgram               )
  L_GLPROC( PFNGLVALIDATEPROGRAMPROC         , glValidateProgram          )
  L_GLPROC( PFNGLUNIFORMMATRIX4FVPROC        , glUniformMatrix4fv         )
  L_GLPROC( PFNGLUNIFORM1IPROC               , glUniform1i                )
  L_GLPROC( PFNGLUNIFORM4FVPROC              , glUniform4fv               )
  L_GLPROC( PFNGLGETPROGRAMIVPROC            , glGetProgramiv             )
  L_GLPROC( PFNGLGETPROGRAMINFOLOGPROC       , glGetProgramInfoLog        )
  L_GLPROC( PFNGLGENBUFFERSPROC              , glGenBuffers               )
  L_GLPROC( PFNGLBINDBUFFERPROC              , glBindBuffer               )
  L_GLPROC( PFNGLBUFFERDATAPROC              , glBufferData               )
  L_GLPROC( PFNGLENABLEVERTEXATTRIBARRAYPROC , glEnableVertexAttribArray  )
  L_GLPROC( PFNGLVERTEXATTRIBPOINTERPROC     , glVertexAttribPointer      )
  L_GLPROC( PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray )
  L_GLPROC( PFNGLDELETEBUFFERSPROC           , glDeleteBuffers            )
  L_GLPROC( PFNGLGETATTRIBLOCATIONPROC       , glGetAttribLocation        )
  L_GLPROC( PFNGLGENVERTEXARRAYSPROC         , glGenVertexArrays          )
  L_GLPROC( PFNGLBINDVERTEXARRAYPROC         , glBindVertexArray          )
  L_GLPROC( PFNGLDELETEVERTEXARRAYSPROC      , glDeleteVertexArrays       )
  L_GLPROC( PFNGLACTIVETEXTUREPROC           , glActiveTexture            )
  L_GLPROC( PFNGLGENERATEMIPMAPPROC          , glGenerateMipmap           )
  L_GLPROC( PFNGLDELETESHADERPROC            , glDeleteShader             )
  L_GLPROC( PFNGLDELETEPROGRAMPROC           , glDeleteProgram            )
  L_GLPROC( PFNGLGETSHADERIVPROC             , glGetShaderiv              )
  L_GLPROC( PFNGLGETSHADERINFOLOGPROC        , glGetShaderInfoLog         )
  }

  #undef L_GLPROC

#endif // OPENGL_INSTEAD_OF_SOFTWARE








#if OPENGL_INSTEAD_OF_SOFTWARE

  struct
  glwshader_t
  {
    u32 program;
    u32 vs;
    u32 fs;
  };

  Inl void
  _VerifyShader( u32 shader, char* prefix )
  {
    s32 loglen = 0;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &loglen );  glVerify();
    if( loglen ) {
      auto log = MemHeapAlloc( char, loglen );
      s32 nwrite = 0;
      glGetShaderInfoLog( shader, loglen, &nwrite, log );  glVerify();
      Log( "%s%s", prefix, log );
      MemHeapFree( log );
      UnreachableCrash();
    }
  }

  Inl void
  _VerifyProgram( u32 program, char* prefix )
  {
    s32 loglen = 0;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &loglen );  glVerify();
    if( loglen ) {
      auto log = MemHeapAlloc( char, loglen );
      s32 nwrite = 0;
      glGetProgramInfoLog( program, loglen, &nwrite, log );  glVerify();
      Log( "%s%s", prefix, log );
      MemHeapFree( log );
      UnreachableCrash();
    }
  }

  glwshader_t
  GlwLoadShader( char* vs, char* fs )
  {
    ProfFunc();
    glwshader_t shader;

    shader.vs = glCreateShader( GL_VERTEX_SHADER );  glVerify();
    shader.fs = glCreateShader( GL_FRAGMENT_SHADER );  glVerify();
    s32 vs_len = Cast( s32, CsLen( Cast( u8*, vs ) ) );
    s32 fs_len = Cast( s32, CsLen( Cast( u8*, fs ) ) );
    glShaderSource( shader.vs, 1, &vs, &vs_len );  glVerify();
    glShaderSource( shader.fs, 1, &fs, &fs_len );  glVerify();
    glCompileShader( shader.vs );  glVerify();
    glCompileShader( shader.fs );  glVerify();
    _VerifyShader( shader.vs, "Vertex shader: " );
    _VerifyShader( shader.fs, "Fragment shader: " );

    shader.program = glCreateProgram();  glVerify();
    glAttachShader( shader.program, shader.vs );  glVerify();
    glAttachShader( shader.program, shader.fs );  glVerify();
    glLinkProgram( shader.program );  glVerify();
    _VerifyProgram( shader.program, "Shader program: " );

    glUseProgram( shader.program );  glVerify();
    glValidateProgram( shader.program );  glVerify();

    return shader;
  }

  void
  GlwUnloadShader( glwshader_t& shader )
  {
    glDeleteShader( shader.vs );  glVerify();
    glDeleteShader( shader.fs );  glVerify();
    glDeleteProgram( shader.program );  glVerify();
  }





  struct
  glwtarget_t
  {
    u32 id;
    u32 color;
    u32 depth;
  };

  void
  Init( glwtarget_t& target )
  {
    glGenFramebuffers( 1, &target.id );  glVerify();
  }

  void
  Resize( glwtarget_t& target, u32 x, u32 y )
  {
    glBindFramebuffer( GL_FRAMEBUFFER, target.id );  glVerify();

    glDeleteTextures( 1, &target.color );
    glGenTextures( 1, &target.color );  glVerify();
    glBindTexture( GL_TEXTURE_2D, target.color );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glBindTexture( GL_TEXTURE_2D, 0 );  glVerify();

    glDeleteRenderbuffers( 1, &target.depth );
    glGenRenderbuffers( 1, &target.depth );  glVerify();
    glBindRenderbuffer( GL_RENDERBUFFER, target.depth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, x, y );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, target.depth );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );  glVerify();

    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.color, 0 );
    u32 drawing_buffers[] = {
      GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers( _countof( drawing_buffers ), drawing_buffers );  glVerify();

    glReadBuffer( GL_NONE );  glVerify();

    u32 res = glCheckFramebufferStatus( GL_FRAMEBUFFER );  glVerify();
    AssertWarn( res == GL_FRAMEBUFFER_COMPLETE );
  }

  void
  Kill( glwtarget_t& target )
  {
    glDeleteTextures( 1, &target.color );
    glDeleteRenderbuffers( 1, &target.depth );
    glDeleteFramebuffers( 1, &target.id );  glVerify();
  }


  // special case shaders:

  struct
  shader_color_t
  {
    glwshader_t core;
    s32 loc_ndc_from_client;
    s32 loc_color;
    s32 attribloc_pos;
  };

  void
  Init( shader_color_t& shader )
  {
    shader.core = GlwLoadShader(
      R"STRING(
        #version 330 core
        layout( location = 0 ) in vec3 vertex_pos;
        uniform mat4 ndc_from_client;
        void main() {
          gl_Position = ndc_from_client * vec4( vertex_pos, 1 );
        }
      )STRING",

      R"STRING(
        #version 330 core
        layout( location = 0 ) out vec4 pixel;
        uniform vec4 color;
        void main() {
          pixel = color;
        }
      )STRING"
      );

    // uniforms
    shader.loc_ndc_from_client = glGetUniformLocation( shader.core.program, "ndc_from_client" );  glVerify();
    shader.loc_color = glGetUniformLocation( shader.core.program, "color" );  glVerify();

    // attribs
    shader.attribloc_pos = glGetAttribLocation( shader.core.program, "vertex_pos" );  glVerify();
  }

  void
  Kill( shader_color_t& shader )
  {
    GlwUnloadShader( shader.core );
    shader.loc_ndc_from_client = -1;
    shader.loc_color = -1;
    shader.attribloc_pos = -1;
  }



  struct
  shader_tex_t
  {
    glwshader_t core;
    s32 loc_ndc_from_client;
    s32 loc_tex_sampler;
    s32 loc_color;
    s32 attribloc_pos;
    s32 attribloc_tc;
  };

  void
  Init( shader_tex_t& shader )
  {
    shader.core = GlwLoadShader(
      R"STRING(
        #version 330 core
        layout( location = 0 ) in vec3 vertex_pos;
        layout( location = 1 ) in vec2 vertex_tc;
        out vec2 tc;
        uniform mat4 ndc_from_client;
        void main() {
          gl_Position = ndc_from_client * vec4( vertex_pos, 1 );
          tc = vertex_tc;
        }
      )STRING",

      R"STRING(
        #version 330 core
        in vec2 tc;
        layout( location = 0 ) out vec4 pixel;
        uniform sampler2D tex_sampler;
        uniform vec4 color;
        void main() {
          vec4 texel = texture( tex_sampler, tc );
          pixel = color * texel;
        }
      )STRING"
      );

    // uniforms
    shader.loc_ndc_from_client = glGetUniformLocation( shader.core.program, "ndc_from_client" );  glVerify();
    shader.loc_tex_sampler = glGetUniformLocation( shader.core.program, "tex_sampler" );  glVerify();
    shader.loc_color = glGetUniformLocation( shader.core.program, "color" );  glVerify();

    // attribs
    shader.attribloc_pos = glGetAttribLocation( shader.core.program, "vertex_pos" );  glVerify();
    shader.attribloc_tc = glGetAttribLocation( shader.core.program, "vertex_tc" );  glVerify();
  }

  void
  Kill( shader_tex_t& shader )
  {
    GlwUnloadShader( shader.core );
    shader.loc_ndc_from_client = -1;
    shader.loc_tex_sampler = -1;
    shader.loc_color = -1;
    shader.attribloc_pos = -1;
    shader.attribloc_tc = -1;
  }

#endif // OPENGL_INSTEAD_OF_SOFTWARE








Enumc( glwcursortype_t )
{
  arrow,
  ibeam,
  hand,
  wait,
};



Enumc( glwcallbacktype_t )
{
  keyevent,
  mouseevent,
  windowevent,
  render,
};

struct
glwcallback_t
{
  glwcallbacktype_t type;
  void* misc;
  void* fn;
};



struct
glwclient_t
{
  bool alive;
  glwcursortype_t cursortype;
  array_t<u8> cstr_title;
  array_t<glwcallback_t> callbacks;

#if OPENGL_INSTEAD_OF_SOFTWARE
  hashset_t texid_map;

  u32 vao;
  shader_tex_t shader_fullscreen;
  u32 glstream_fullscreen;
  array_t<f32> stream_fullscreen;
#endif // OPENGL_INSTEAD_OF_SOFTWARE

  //u32 keyrepeat_delay_millisec; // TODO: implement custom timer keyrepeating.
  //u32 keyrepeat_interval_millisec;
  bool alreadydn[glwkey_t::count];
  bool mousealreadydn[glwmousebtn_t::count];
  glwkeylocks_t locks;

  bool fullscreen;
  vec2<u32> dim;
  vec2<f32> dimf;
  vec2<s32> m;
  u32 dpi;

  HANDLE timer_anim;
  f64 timestep_fixed; // seconds
  u64 time_render0;
  u64 time_render1;

  HDC window_dc; // handle to display context.
#if OPENGL_INSTEAD_OF_SOFTWARE
  HGLRC hgl; // handle to opengl context.
  glwtarget_t target;
#else
  HDC fullscreen_bitmap_dc;
  HBITMAP fullscreen_bitmap;
  u32* fullscreen_bitmap_argb;
#endif // OPENGL_INSTEAD_OF_SOFTWARE
  HWND hwnd; // handle to window.
  HINSTANCE hi; // handle to WinAPI module the window is running under.

  bool target_valid;
};



Enumc( glwkeyevent_t )
{
  dn,
  up,
  repeat,
};

Enumc( glwmouseevent_t )
{
  dn,
  up,
  move,
  wheelmove,
};

constant enum_t glwwindowevent_resize = ( 1 << 0 );
constant enum_t glwwindowevent_dpichange = ( 1 << 1 );
constant enum_t glwwindowevent_focuschange = ( 1 << 2 );

#define __OnKeyEvent( name )      \
  void ( name )(                  \
    void* misc,                   \
    vec2<f32> origin,             \
    vec2<f32> dim,                \
    bool& fullscreen,             \
    bool& target_valid,           \
    glwkeyevent_t type,           \
    glwkey_t key,                 \
    glwkeylocks_t keylocks,       \
    bool* alreadydn               \
    )

#define __OnMouseEvent( name )      \
  void ( name )(                    \
    void* misc,                     \
    vec2<f32> origin,               \
    vec2<f32> dim,                  \
    bool& target_valid,             \
    glwcursortype_t& cursortype,    \
    glwmouseevent_t type,           \
    glwmousebtn_t btn,              \
    bool* alreadydn,                \
    bool* keyalreadydn,             \
    vec2<s32> m,                    \
    vec2<s32> raw_delta,            \
    s32 dwheel                      \
    )

#define __OnWindowEvent( name )      \
  void ( name )(                     \
    void* misc,                      \
    enum_t type,                     \
    vec2<u32> dim,                   \
    u32 dpi,                         \
    bool focused,                    \
    bool& target_valid               \
    )

#define __OnRender( name )      \
  void ( name )(                \
    void* misc,                 \
    vec2<f32> origin,           \
    vec2<f32> dim,              \
    f64 timestep_realtime,      \
    f64 timestep_fixed,         \
    bool& target_valid          \
    )

typedef __OnKeyEvent( *pfn_onkeyevent_t );
typedef __OnMouseEvent( *pfn_onmouseevent_t );
typedef __OnWindowEvent( *pfn_onwindowevent_t );
typedef __OnRender( *pfn_onrender_t );




void
GlwRegisterCallback( glwclient_t& client, glwcallback_t& callback )
{
  *AddBack( client.callbacks ) = callback;
}



bool
GlwMouseInside(
  vec2<s32> m,
  vec2<f32> origin,
  vec2<f32> dim
  )
{
  auto mx = Cast( f32, m.x );
  auto my = Cast( f32, m.y );
  return
    ( origin.x <= mx )  &&  ( mx < origin.x + dim.x )  &&
    ( origin.y <= my )  &&  ( my < origin.y + dim.y );
}

bool
GlwMouseInsideRect(
  vec2<s32> m,
  vec2<f32> p0,
  vec2<f32> p1
  )
{
  auto mx = Cast( f32, m.x );
  auto my = Cast( f32, m.y );
  return
    ( p0.x <= mx )  &&  ( mx < p1.x )  &&
    ( p0.y <= my )  &&  ( my < p1.y );
}


struct
glwkeybind_t
{
  glwkey_t key;
  glwkey_t alreadydn;
  glwkey_t alreadydn2;
  glwkey_t conflict;
  glwkey_t conflict2;
  glwkey_t conflict3;
};


Inl glwkeybind_t
_glwkeybind(
  glwkey_t key,
  glwkey_t alreadydn = glwkey_t::none,
  glwkey_t alreadydn2 = glwkey_t::none,
  glwkey_t conflict = glwkey_t::none,
  glwkey_t conflict2 = glwkey_t::none,
  glwkey_t conflict3 = glwkey_t::none
  )
{
  glwkeybind_t r = {};
  r.key = key;
  r.alreadydn = alreadydn;
  r.alreadydn2 = alreadydn2;
  r.conflict = conflict;
  r.conflict2 = conflict2;
  r.conflict3 = conflict3;
  return r;
}

bool
GlwKeybind( glwkey_t key, bool* alreadydn, glwkeybind_t& bind )
{
  // make sure primary key matches event key.
  bool r = ( bind.key == key );

  // make sure specified modifier keys are already dn.
  r &= Implies( bind.alreadydn  != glwkey_t::none, alreadydn[Cast( enum_t, bind.alreadydn  )] );
  r &= Implies( bind.alreadydn2 != glwkey_t::none, alreadydn[Cast( enum_t, bind.alreadydn2 )] );

  // make sure conflict keys are not already dn.
  r &= Implies( ( bind.conflict  != bind.alreadydn ) && ( bind.conflict  != bind.alreadydn2 ), !alreadydn[Cast( enum_t, bind.conflict  )] );
  r &= Implies( ( bind.conflict2 != bind.alreadydn ) && ( bind.conflict2 != bind.alreadydn2 ), !alreadydn[Cast( enum_t, bind.conflict2 )] );
  r &= Implies( ( bind.conflict3 != bind.alreadydn ) && ( bind.conflict3 != bind.alreadydn2 ), !alreadydn[Cast( enum_t, bind.conflict3 )] );
  return r;
}




#if OPENGL_INSTEAD_OF_SOFTWARE

  // We map u32 id_client to one of these mapping structs.
  struct
  glwtexid_mapping_t
  {
    u32 glid;
    u32 img_x;
    u32 img_y;
  };

#endif // OPENGL_INSTEAD_OF_SOFTWARE



void
GlwEarlyKill( glwclient_t& client )
{
  client.alive = 0;

  g_mainthread.signal_quit = 1;
  _ReadWriteBarrier();
}


void
GlwSetCursorType( glwclient_t& client, glwcursortype_t type )
{
  client.cursortype = type;
}



#if OPENGL_INSTEAD_OF_SOFTWARE

  void
  GlwUploadTexture(
    glwclient_t& client,
    u32 texid_client,
    void* mem,
    idx_t nbytes,
    u32 x,
    u32 y
    )
  {
    glwtexid_mapping_t mapping;
    mapping.img_x = x;
    mapping.img_y = y;
    glGenTextures( 1, &mapping.glid );  glVerify();

    // Insert ( texid_client, mapping ) into the texid_map, making sure it is a new texid_client.
    bool already_there;
    Add( client.texid_map, &texid_client, &mapping, &already_there, 0, 0 );
    AssertCrash( !already_there );

    glBindTexture( GL_TEXTURE_2D, mapping.glid );  glVerify();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );  glVerify();

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, mem );  glVerify();

    glGenerateMipmap( GL_TEXTURE_2D );  glVerify();

    glBindTexture( GL_TEXTURE_2D, 0 );  glVerify();
  }

  void
  GlwUpdateTexture(
    glwclient_t& client,
    u32 texid_client,
    void* mem,
    idx_t nbytes,
    u32 x,
    u32 y
    )
  {
    bool found;
    glwtexid_mapping_t mapping;
    Lookup( client.texid_map, &texid_client, &found, &mapping );
    AssertCrash( found );

    glBindTexture( GL_TEXTURE_2D, mapping.glid );  glVerify();

    // if x,y dims change, we have to do glTexImage to overwrite the old-size texture.
    if( mapping.img_x != x  ||  mapping.img_y != y ) {
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, mem );  glVerify();

      // Update the mapping in texid_map, since we updated img_x, img_y.
      glwtexid_mapping_t* inplace_mapping;
      LookupRaw( client.texid_map, &texid_client, &found, Cast( void**, &inplace_mapping ) );
      AssertCrash( found );
      inplace_mapping->img_x = x;
      inplace_mapping->img_y = y;

    } else {
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, mem );  glVerify();
    }

    glBindTexture( GL_TEXTURE_2D, 0 );  glVerify();
  }

  void
  GlwDeleteTexture( glwclient_t& client, u32 texid_client )
  {
    bool found;
    glwtexid_mapping_t mapping;
    Remove( client.texid_map, &texid_client, &found, &mapping );
    AssertCrash( found );

    glDeleteTextures( 1, &mapping.glid );  glVerify();
  }

  void
  GlwSetSwapInterval( glwclient_t& client, s32 swap_interval )
  {
    wglSwapIntervalEXT( swap_interval );  glVerify();
  }

#endif // OPENGL_INSTEAD_OF_SOFTWARE


void
GlwSetCursorVisible( glwclient_t& client, bool visible )
{
  ShowCursor( visible );
}


#if OPENGL_INSTEAD_OF_SOFTWARE

  Inl void
  GlwBindTexture(
    u8 tiu,
    u32 glid
    )
  {
    glActiveTexture( GL_TEXTURE0 + tiu );  glVerify();
    glBindTexture( GL_TEXTURE_2D, glid );  glVerify();
  }

#endif // OPENGL_INSTEAD_OF_SOFTWARE





struct
glwkeyvkcode_t
{
  glwkey_t key;
  u8 vkcode;
};

static const glwkeyvkcode_t g_glwkeyvkcodes[] =
{
  #define ENTRY( name, value, win_value )   { glwkey_t::##name, win_value },
  KEYVALMAP( ENTRY )
  #undef ENTRY
};

struct
glwkeystring_t
{
  glwkey_t key;
  slice_t string;
};

static const glwkeystring_t g_glwkeystrings[] =
{
  #define ENTRY( name, value, win_value )   { glwkey_t::##name, SliceFromCStr( #name ) },
  KEYVALMAP( ENTRY )
  #undef ENTRY
};

static bool g_init_keytables = 0;
static glwkey_t g_key_glw_from_os [ 256 ] = {};
static u8 g_key_os_from_glw [ 256 ] = {};
static slice_t g_string_from_key [ 256 ] = {};
static hashset_t g_key_from_string = {};



__OnMainKill( KillKeytables )
{
  Kill( g_key_from_string );
}

Inl void
_InitKeyTable()
{
  if( !g_init_keytables ) {
    g_init_keytables = 1;
    RegisterOnMainKill( KillKeytables, 0 );

    ForEach( keyvkcode, g_glwkeyvkcodes ) {
      g_key_os_from_glw[Cast( enum_t, keyvkcode.key )] = keyvkcode.vkcode;
      g_key_glw_from_os[keyvkcode.vkcode] = keyvkcode.key;
    }

    Init(
      g_key_from_string,
      256,
      sizeof( slice_t ),
      sizeof( glwkey_t ),
      0.8f,
      Equal_SliceContents,
      Hash_SliceContents
      );

    ForEach( keystring, g_glwkeystrings ) {
      g_string_from_key[Cast( enum_t, keystring.key )] = keystring.string;

      bool already_there;
      Add( g_key_from_string,
        Cast( void*, &keystring.string ),
        Cast( void*, &keystring.key ),
        &already_there,
        0,
        0
        );
      AssertCrash( !already_there );
    }
  }
}


Inl glwkey_t
KeyGlwFromOS( WPARAM key )
{
  _InitKeyTable();
  AssertCrash( key < _countof( g_key_glw_from_os ) );
  return g_key_glw_from_os[key];
}

Inl u8
KeyOSFromGlw( glwkey_t key )
{
  _InitKeyTable();
  return g_key_os_from_glw[Cast( enum_t, key )];
}

Inl void
KeyGlwFromString(
  slice_t keystring,
  bool* found,
  glwkey_t* found_key
  )
{
  _InitKeyTable();
  glwkey_t* key = 0;
  LookupRaw(
    g_key_from_string,
    &keystring,
    found,
    Cast( void**, &key )
    );
  if( found  &&  found_key ) {
    *found_key = *key;
  }
}

Inl slice_t
KeyStringFromGlw( glwkey_t key )
{
  _InitKeyTable();
  return g_string_from_key[Cast( enum_t, key )];
}



#if OPENGL_INSTEAD_OF_SOFTWARE

  Inl void
  OutputQuad(
    array_t<f32>& stream,
    vec2<f32> p0,
    vec2<f32> p1,
    f32 z,
    vec2<f32> tc0,
    vec2<f32> tc1
    )
  {
    auto pos = AddBack( stream, 3*6 + 2*6 );
    *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p0.y, z );  pos += 3;
    *Cast( vec2<f32>*, pos ) = _vec2( tc0.x, tc0.y );  pos += 2;
    *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p0.y, z );  pos += 3;
    *Cast( vec2<f32>*, pos ) = _vec2( tc1.x, tc0.y );  pos += 2;
    *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p1.y, z );  pos += 3;
    *Cast( vec2<f32>*, pos ) = _vec2( tc0.x, tc1.y );  pos += 2;
    *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p1.y, z );  pos += 3;
    *Cast( vec2<f32>*, pos ) = _vec2( tc1.x, tc1.y );  pos += 2;
    *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p1.y, z );  pos += 3;
    *Cast( vec2<f32>*, pos ) = _vec2( tc0.x, tc1.y );  pos += 2;
    *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p0.y, z );  pos += 3;
    *Cast( vec2<f32>*, pos ) = _vec2( tc1.x, tc0.y );  pos += 2;
  }

#endif // OPENGL_INSTEAD_OF_SOFTWARE



#if !OPENGL_INSTEAD_OF_SOFTWARE

  Inl void
  DeleteTarget( glwclient_t& client )
  {
    DeleteObject( client.fullscreen_bitmap );
    client.fullscreen_bitmap = 0;
    SelectObject( client.fullscreen_bitmap_dc, 0 );
    DeleteDC( client.fullscreen_bitmap_dc );
    client.fullscreen_bitmap_dc = 0;
  }

  Inl void
  ResizeTargetToMatchDim( glwclient_t& client )
  {
    DeleteTarget( client );

    client.fullscreen_bitmap_dc = CreateCompatibleDC( client.window_dc );
    AssertCrash( client.fullscreen_bitmap_dc );

    BITMAPINFO bi = { 0 };
    bi.bmiHeader.biSize = sizeof( bi.bmiHeader );
    bi.bmiHeader.biWidth = client.dim.x;
    AssertCrash( client.dim.y <= MAX_s32 );
    bi.bmiHeader.biHeight = -Cast( s32, client.dim.y ); // negative <=> top-down.
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    client.fullscreen_bitmap = CreateDIBSection(
      client.fullscreen_bitmap_dc,
      &bi,
      DIB_RGB_COLORS,
      Cast( void**, &client.fullscreen_bitmap_argb ),
      0,
      0
      );
    AssertCrash( client.fullscreen_bitmap );

    DIBSECTION ds;
    GetObject( client.fullscreen_bitmap, sizeof( ds ), &ds );

    SelectObject( client.fullscreen_bitmap_dc, client.fullscreen_bitmap );
  }

#endif // !OPENGL_INSTEAD_OF_SOFTWARE


Inl void
Viewport( glwclient_t& client )
{
#if OPENGL_INSTEAD_OF_SOFTWARE
  glViewport( 0, 0, client.dim.x, client.dim.y );  glVerify();
#endif // OPENGL_INSTEAD_OF_SOFTWARE

  if( client.dim.x && client.dim.y ) {

#if OPENGL_INSTEAD_OF_SOFTWARE
    Resize( client.target, client.dim.x, client.dim.y );
#else
    ResizeTargetToMatchDim( client );
#endif

    client.target_valid = 0;

#if OPENGL_INSTEAD_OF_SOFTWARE
    glUseProgram( client.shader_fullscreen.core.program );  glVerify();

    mat4x4r<f32> ndc_from_client;
    Ortho( &ndc_from_client, 0.0f, client.dimf.x, 0.0f, client.dimf.y, 0.0f, 1.0f );
    glUniformMatrix4fv( client.shader_fullscreen.loc_ndc_from_client, 1, 1, &ndc_from_client.row0.x );  glVerify();

    glUniform1i( client.shader_fullscreen.loc_tex_sampler, 0 );  glVerify();

    auto color = _vec4<f32>( 1, 1, 1, 1 );
    glUniform4fv( client.shader_fullscreen.loc_color, 1, &color.x );  glVerify();

    client.stream_fullscreen.len = 0;

    OutputQuad(
      client.stream_fullscreen,
      _vec2<f32>( 0, 0 ),
      client.dimf,
      0.0f,
      _vec2<f32>( 0, 0 ),
      _vec2<f32>( 1, 1 )
      );
#endif // OPENGL_INSTEAD_OF_SOFTWARE
  }
}


Inl void
_SetCursortype( glwcursortype_t type )
{
  switch( type ) {
    case glwcursortype_t::arrow:  SetCursor( LoadCursor( 0, IDC_ARROW ) );  break;
    case glwcursortype_t::ibeam:  SetCursor( LoadCursor( 0, IDC_IBEAM ) );  break;
    case glwcursortype_t::hand :  SetCursor( LoadCursor( 0, IDC_HAND  ) );  break;
    case glwcursortype_t::wait :  SetCursor( LoadCursor( 0, IDC_WAIT  ) );  break;
    default: UnreachableCrash();
  }
}

Inl vec2<u32>
_GetMonitorSize( HWND hwnd )
{
  MONITORINFO monitor_info = { sizeof( MONITORINFO ) /* other fields 0 */ };
  AssertWarn( GetMonitorInfo( MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST ), &monitor_info ) );
  auto w = Cast( u32, monitor_info.rcMonitor.right  - monitor_info.rcMonitor.left );
  auto h = Cast( u32, monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top  );
  return _vec2( w, h );
}

Inl void
_SetWindowSize( HWND hwnd, vec2<u32> dim )
{
  DWORD style = GetWindowLong( hwnd, GWL_STYLE );
  DWORD styleex = GetWindowLong( hwnd, GWL_EXSTYLE );
  RECT rect = { 0, 0, Cast( LONG, dim.x ), Cast( LONG, dim.y ) };
  AdjustWindowRectEx( &rect, style, 0, styleex ); // get actual win size.
  auto outer_w = rect.right  - rect.left;
  auto outer_h = rect.bottom - rect.top;
  BOOL r = SetWindowPos(
    hwnd, // window
    0, // zorder flag ( unused )
    0, 0, // x, y in client space ( unused )
    outer_w, outer_h, // w, h in client space
    SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED ); // size/positioning flags
  AssertWarn( r );
}

static void
_SetFullscreen( HWND hwnd, vec2<u32>& dim, vec2<f32>& dimf, bool fs )
{
  static bool place_prev_stored = 0;
  static WINDOWPLACEMENT place_prev = { sizeof( WINDOWPLACEMENT ) /* other fields 0 */ };
  static vec2<u32> windowdim = {};

  DWORD style = GetWindowLong( hwnd, GWL_STYLE );
  DWORD styleex = GetWindowLong( hwnd, GWL_EXSTYLE );

  if( fs ) {
    // ENTER FULLSCREEN MODE
    AssertWarn( GetWindowPlacement( hwnd, &place_prev ) );
    windowdim = dim;
    place_prev_stored = 1;

    AssertWarn( style & WS_OVERLAPPEDWINDOW );
    style &= ~WS_OVERLAPPEDWINDOW;
    style |= WS_POPUP;
    AssertWarn( SetWindowLong( hwnd, GWL_STYLE, style ) );

    MONITORINFO monitor_info = { sizeof( MONITORINFO ) /* other fields 0 */ };
    AssertWarn( GetMonitorInfo( MonitorFromWindow( hwnd, MONITOR_DEFAULTTOPRIMARY ), &monitor_info ) );
    LONG x = monitor_info.rcMonitor.left;
    LONG y = monitor_info.rcMonitor.top;
    LONG w = monitor_info.rcMonitor.right  - monitor_info.rcMonitor.left;
    LONG h = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;

    dim = _vec2( Cast( u32, w ), Cast( u32, h ) );
    dimf.x = Cast( f32, dim.x );
    dimf.y = Cast( f32, dim.y );
    BOOL r = SetWindowPos(
      hwnd, // window
      HWND_TOP, // zorder flag
      x, y, // x, y in client space
      w, h, // w, h in client space
      SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW ); // size / positioning flags
    AssertWarn( r );

  } else {
    // ENTER WINDOWED MODE
    style &= ~WS_POPUP;
    style |= WS_OVERLAPPEDWINDOW;
    AssertWarn( SetWindowLong( hwnd, GWL_STYLE, style ) );
    if( place_prev_stored ) {
      AssertWarn( SetWindowPlacement( hwnd, &place_prev ) );
      dim = windowdim;
      dimf.x = Cast( f32, dim.x );
      dimf.y = Cast( f32, dim.y );
      RECT rect = { 0, 0, Cast( LONG, windowdim.x ), Cast( LONG, windowdim.y ) };
      AdjustWindowRectEx( &rect, style, FALSE, styleex ); // get actual win size.
      auto outer_w = rect.right  - rect.left;
      auto outer_h = rect.bottom - rect.top;
      BOOL r = SetWindowPos(
        hwnd, // window
        0, // zorder flag ( unused )
        0, 0, // x, y in client space ( unused )
        outer_w, outer_h, // w, h in client space
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED ); // size/positioning flags
      AssertWarn( r );
    }
  }
}




void
_Render( glwclient_t& client )
{
  // TimeClock has more variance, and so it makes for jittery animation ( esp. on Win Server! ).
  // TimeTSC seems to be much smoother, so we use it instead.
  client.time_render0 = client.time_render1;
  client.time_render1 = TimeTSC();
  f64 timestep_realtime = TimeSecFromTSC64( client.time_render1 - client.time_render0 );

  if( !client.target_valid ) {
    // NOTE: anything that outputs drawcalls has the option of invalidating the target, if it knows
    //   the next frame also requires a render ( e.g. animation )
    client.target_valid = 1;

    Prof( MakeTargetValid );

#if OPENGL_INSTEAD_OF_SOFTWARE
    // render to texture.
    glBindFramebuffer( GL_FRAMEBUFFER, client.target.id );  glVerify();
    //glBindFramebuffer( GL_FRAMEBUFFER, 0 );  glVerify();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  glVerify();
#else // !OPENGL_INSTEAD_OF_SOFTWARE
    Prof( ZeroBitmap );
    Memzero( client.fullscreen_bitmap_argb, client.dim.x * client.dim.y * sizeof( client.fullscreen_bitmap_argb[0] ) );
    ProfClose( ZeroBitmap );
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

    ForLen( j, client.callbacks ) {
      auto& callback = client.callbacks.mem[j];
      if( callback.type == glwcallbacktype_t::render ) {
        Cast( pfn_onrender_t, callback.fn )(
          callback.misc,
          _vec2<f32>( 0, 0 ),
          client.dimf,
          timestep_realtime,
          client.timestep_fixed,
          client.target_valid
          );
      }
    }

#if OPENGL_INSTEAD_OF_SOFTWARE
    glFlush();  glVerify();
#else // !OPENGL_INSTEAD_OF_SOFTWARE

    Prof( BlitToScreen );
    BOOL bitblt_success = BitBlt(
      client.window_dc,
      0, 0,
      client.dim.x, client.dim.y,
      client.fullscreen_bitmap_dc,
      0, 0,
      SRCCOPY
      );
    AssertWarn( bitblt_success );
    ProfClose( BlitToScreen );
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  } // end if( !client.target_valid )

  // TODO: it seems we don't really need this RenderToScreenTex second pass on all calls to _Render.
  // i.e. we can do this only when !client.target_valid, and everything seems to work fine.
  // that'd save some cycles, and maybe we could simplify to avoid the offscreen->screen draw call.

#if OPENGL_INSTEAD_OF_SOFTWARE
  // render to screen.
  Prof( RenderToScreenTex );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );  glVerify();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  glVerify();

  glUseProgram( client.shader_fullscreen.core.program );  glVerify();

  GlwBindTexture( 0, client.target.color );

  glBindBuffer( GL_ARRAY_BUFFER, client.glstream_fullscreen );
  glBufferData( GL_ARRAY_BUFFER, client.stream_fullscreen.len * sizeof( f32 ), client.stream_fullscreen.mem, GL_STREAM_DRAW );  glVerify();
  glEnableVertexAttribArray( client.shader_fullscreen.attribloc_pos );  glVerify();
  glEnableVertexAttribArray( client.shader_fullscreen.attribloc_tc );  glVerify();
  glVertexAttribPointer( client.shader_fullscreen.attribloc_pos, 3, GL_FLOAT, 0, 5 * sizeof( f32 ), 0 );  glVerify();
  glVertexAttribPointer( client.shader_fullscreen.attribloc_tc, 2, GL_FLOAT, 0, 5 * sizeof( f32 ), Cast( void*, 3 * sizeof( f32 ) ) );  glVerify();

  auto vert_count = client.stream_fullscreen.len / 5;
  AssertCrash( client.stream_fullscreen.len % 5 == 0 );
  AssertCrash( vert_count <= MAX_s32 );
  glDrawArrays( GL_TRIANGLES, 0, Cast( s32, vert_count ) );  glVerify();

  glDisableVertexAttribArray( client.shader_fullscreen.attribloc_pos );  glVerify();
  glDisableVertexAttribArray( client.shader_fullscreen.attribloc_tc );  glVerify();

  glFlush();  glVerify();
  ProfClose( RenderToScreenTex );

  Prof( SwapBuffers );
  SwapBuffers( client.window_dc );
  ProfClose( SwapBuffers );
#endif // OPENGL_INSTEAD_OF_SOFTWARE
}


LRESULT CALLBACK
WindowProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
  static glwclient_t* client = 0;

  //Log( "msg( %u )", msg );

  switch( msg ) {
    case WM_CREATE: {
      auto data = Cast( CREATESTRUCT*, lp );
      client = Cast( glwclient_t*, data->lpCreateParams );
    } return 0;

    case WM_DESTROY: // fallthrough
    case WM_CLOSE: {
      PostQuitMessage( 0 );
      GlwEarlyKill( *client );
    } return 0;

    case WM_DPICHANGED: {
      auto dpi_x = LOWORD( wp );
      auto dpi_y = HIWORD( wp );
      auto rect = *Cast( RECT*, lp );

      AssertWarn( dpi_x == dpi_y );
      auto old_dpi = client->dpi;
      auto new_dpi = dpi_x;
      client->dpi = new_dpi;

      Log( "[DPICHANGED] dpi: %d -> %d", old_dpi, new_dpi );

      // Win32 gives us a very specific rect, and we shouldn't change it.
      BOOL r = SetWindowPos(
        client->hwnd, // window
        0, // zorder flag
        rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top,
        SWP_NOZORDER | SWP_NOACTIVATE
        );
      AssertWarn( r );

      ForLen( j, client->callbacks ) {
        auto& callback = client->callbacks.mem[j];
        if( callback.type == glwcallbacktype_t::windowevent ) {
          Cast( pfn_onwindowevent_t, callback.fn )(
            callback.misc,
            glwwindowevent_resize | glwwindowevent_dpichange,
            client->dim,
            client->dpi,
            1,
            client->target_valid
            );
        }
      }

      Viewport( *client );

    } return 0;

    case WM_PAINT: {
      _Render( *client );
      AssertWarn( ValidateRect( client->hwnd, 0 ) );
    } return 0;

    case WM_ERASEBKGND: {
      // do nothing.
    } return 1;

    case WM_SIZE: {
      client->dim.x = LOWORD( lp );
      client->dim.y = HIWORD( lp );
      client->dimf.x = Cast( f32, client->dim.x );
      client->dimf.y = Cast( f32, client->dim.y );

      ForLen( j, client->callbacks ) {
        auto& callback = client->callbacks.mem[j];
        if( callback.type == glwcallbacktype_t::windowevent ) {
          Cast( pfn_onwindowevent_t, callback.fn )(
            callback.misc,
            glwwindowevent_resize,
            client->dim,
            client->dpi,
            1,
            client->target_valid
            );
        }
      }

      Viewport( *client );
    } return 0;

    case WM_SYSCOMMAND: {
      switch( wp ) {
        case SC_MONITORPOWER:
        case SC_SCREENSAVE:
          return 0; // ignore the above commands
      }
    } break; // forward other cmds to DefWindowProc.

    case WM_KILLFOCUS: {
      ForLen( j, client->callbacks ) {
        auto& callback = client->callbacks.mem[j];
        if( callback.type == glwcallbacktype_t::windowevent ) {
          Cast( pfn_onwindowevent_t, callback.fn )(
            callback.misc,
            glwwindowevent_focuschange,
            client->dim,
            client->dpi,
            0,
            client->target_valid
            );
        }
      }
      // clear alreadydn status on window defocus.
      For( j, 0, Cast( enum_t, glwkey_t::count ) ) {
        client->alreadydn[j] = 0;
      }
      For( j, 0, Cast( enum_t, glwmousebtn_t::count ) ) {
        client->mousealreadydn[j] = 0;
      }
    } return 0;

    case WM_SETFOCUS: {
      ForLen( j, client->callbacks ) {
        auto& callback = client->callbacks.mem[j];
        if( callback.type == glwcallbacktype_t::windowevent ) {
          Cast( pfn_onwindowevent_t, callback.fn )(
            callback.misc,
            glwwindowevent_focuschange,
            client->dim,
            client->dpi,
            1,
            client->target_valid
            );
        }
      }
    } return 0;

#if !GLW_RAWINPUT_KEYBOARD
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
      BOOL repeat = ( lp & ( 1 << 30 ) );
      glwkeyevent_t type = ( repeat  ?  glwkeyevent_t::repeat  :  glwkeyevent_t::dn );
      auto key = KeyGlwFromOS( wp );
      if( key != glwkey_t::none ) {
        switch( key ) {
          case glwkey_t::capslock:
          case glwkey_t::numlock:
          case glwkey_t::scrolllock: {
            client->locks.caps   = GetKeyState( KeyOSFromGlw( glwkey_t::capslock   ) ) & 1;
            client->locks.num    = GetKeyState( KeyOSFromGlw( glwkey_t::numlock    ) ) & 1;
            client->locks.scroll = GetKeyState( KeyOSFromGlw( glwkey_t::scrolllock ) ) & 1;
          } break;
        }
        ForLen( j, client->callbacks ) {
          auto& callback = client->callbacks.mem[j];
          if( callback.type == glwcallbacktype_t::keyevent ) {
            auto fullscreen = client->fullscreen;
            Cast( pfn_onkeyevent_t, callback.fn )(
              callback.misc,
              _vec2<f32>( 0, 0 ),
              client->dimf,
              fullscreen,
              client->target_valid,
              type,
              key,
              client->locks,
              client->alreadydn
              );
            if( fullscreen != client->fullscreen ) {
              _SetFullscreen( client->hwnd, client->dim, client->dimf, fullscreen );
              client->fullscreen = fullscreen;
            }
          }
        }
        if( !repeat ) {
          client->alreadydn[Cast( enum_t, key )] = 1;
        }
      }
    } return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP: {
      auto key = KeyGlwFromOS( wp );
      if( key != glwkey_t::none ) {
        ForLen( j, client->callbacks ) {
          auto& callback = client->callbacks.mem[j];
          if( callback.type == glwcallbacktype_t::keyevent ) {
            auto fullscreen = client->fullscreen;
            Cast( pfn_onkeyevent_t, callback.fn )(
              callback.misc,
              _vec2<f32>( 0, 0 ),
              client->dimf,
              fullscreen,
              client->target_valid,
              glwkeyevent_t::up,
              key,
              client->locks,
              client->alreadydn
              );
            if( fullscreen != client->fullscreen ) {
              _SetFullscreen( client->hwnd, client->dim, client->dimf, fullscreen );
              client->fullscreen = fullscreen;
            }
          }
        }
        client->alreadydn[Cast( enum_t, key )] = 0;
      }
    } return 0;
#endif

    case WM_MOUSEMOVE: {
      // Update the server's mouse x,y but let rawinput handling send the packet.
      auto x = Cast( s32, Cast( s16, LOWORD( lp ) ) );
      auto y = Cast( s32, Cast( s16, HIWORD( lp ) ) );
      client->m = _vec2( x, y );
    } return 0;

    case WM_NCHITTEST: {
      LRESULT r = DefWindowProc( hwnd, msg, wp, lp );
      bool cursor_in_client = ( r == HTCLIENT );
      if( cursor_in_client ) {
        _SetCursortype( client->cursortype );
      } else {
        //SetCursor( LoadCursor( 0, IDC_ARROW ) );
      }
      return r;
    }

    case WM_INPUT: {
      const u32 RI_DATA_LEN = 256; // Needs to be >= 40 on x86, and >= 48 on x86_64.
      static u8 ri_data [ RI_DATA_LEN ];
      //UINT ri_data_size;     // Queried every time this message rolls around.
      // Retrieve ri_data_size.
      UINT ri_data_size = RI_DATA_LEN + 1; // so we know if something goes wrong.
      GetRawInputData(
        Cast( HRAWINPUT, lp ),
        RID_INPUT,
        0,
        &ri_data_size,
        sizeof( RAWINPUTHEADER )
        );
      AssertWarn( ri_data_size <= RI_DATA_LEN );
      // Retrieve the rawinputs themselves.
      UINT ri_data_size_res = GetRawInputData(
        Cast( HRAWINPUT, lp ),
        RID_INPUT,
        Cast( LPVOID, ri_data ),
        &ri_data_size,
        sizeof( RAWINPUTHEADER )
        );
      AssertWarn( ri_data_size == ri_data_size_res );
      auto& raw = *Cast( RAWINPUT*, ri_data );

      switch( raw.header.dwType ) {
        case RIM_TYPEMOUSE: {
          // send a mousemove if we moved
          auto dx = Cast( s32, raw.data.mouse.lLastX );
          auto dy = Cast( s32, raw.data.mouse.lLastY );
          auto raw_delta = _vec2( dx, dy );
          if( dx || dy ) {
            client->m += raw_delta;

            ForLen( j, client->callbacks ) {
              auto& callback = client->callbacks.mem[j];
              if( callback.type == glwcallbacktype_t::mouseevent ) {
                Cast( pfn_onmouseevent_t, callback.fn )(
                  callback.misc,
                  _vec2<f32>( 0, 0 ),
                  client->dimf,
                  client->target_valid,
                  client->cursortype,
                  glwmouseevent_t::move,
                  glwmousebtn_t::none,
                  client->mousealreadydn,
                  client->alreadydn,
                  client->m,
                  raw_delta,
                  0
                  );
              }
            }
          }

          // send a mousewheelmove if we moved
          if( raw.data.mouse.usButtonFlags & RI_MOUSE_WHEEL ) {
            s16 delta = Cast( SHORT, raw.data.mouse.usButtonData ) / WHEEL_DELTA;
            if( delta ) {
              ForLen( j, client->callbacks ) {
                auto& callback = client->callbacks.mem[j];
                if( callback.type == glwcallbacktype_t::mouseevent ) {
                  Cast( pfn_onmouseevent_t, callback.fn )(
                    callback.misc,
                    _vec2<f32>( 0, 0 ),
                    client->dimf,
                    client->target_valid,
                    client->cursortype,
                    glwmouseevent_t::wheelmove,
                    glwmousebtn_t::none,
                    client->mousealreadydn,
                    client->alreadydn,
                    client->m,
                    _vec2<s32>( 0, 0 ),
                    Cast( s32, delta )
                    );
                }
              }
            }
          }

          // send a mousedown/mouseup on press/release
          static const u32 flag_btndn[] = {
            0, // No 0th button
            RI_MOUSE_BUTTON_1_DOWN,
            RI_MOUSE_BUTTON_2_DOWN,
            RI_MOUSE_BUTTON_3_DOWN,
            RI_MOUSE_BUTTON_4_DOWN,
            RI_MOUSE_BUTTON_5_DOWN,
          };
          static const u32 flag_btnup[] = {
            0, // No 0th button
            RI_MOUSE_BUTTON_1_UP,
            RI_MOUSE_BUTTON_2_UP,
            RI_MOUSE_BUTTON_3_UP,
            RI_MOUSE_BUTTON_4_UP,
            RI_MOUSE_BUTTON_5_UP,
          };
          ULONG btns = raw.data.mouse.ulButtons;
          Fori( enum_t, code, 0, Cast( enum_t, glwmousebtn_t::count ) ) {
            auto btn = Cast( glwmousebtn_t, code );
            if( btns & flag_btndn[code] ) {
              ForLen( j, client->callbacks ) {
                auto& callback = client->callbacks.mem[j];
                if( callback.type == glwcallbacktype_t::mouseevent ) {
                  Cast( pfn_onmouseevent_t, callback.fn )(
                    callback.misc,
                    _vec2<f32>( 0, 0 ),
                    client->dimf,
                    client->target_valid,
                    client->cursortype,
                    glwmouseevent_t::dn,
                    btn,
                    client->mousealreadydn,
                    client->alreadydn,
                    client->m,
                    _vec2<s32>( 0, 0 ),
                    0
                    );
                }
              }
              client->mousealreadydn[Cast( enum_t, btn )] = 1;
            }
            if( btns & flag_btnup[code] ) {
              ForLen( j, client->callbacks ) {
                auto& callback = client->callbacks.mem[j];
                if( callback.type == glwcallbacktype_t::mouseevent ) {
                  Cast( pfn_onmouseevent_t, callback.fn )(
                    callback.misc,
                    _vec2<f32>( 0, 0 ),
                    client->dimf,
                    client->target_valid,
                    client->cursortype,
                    glwmouseevent_t::up,
                    btn,
                    client->mousealreadydn,
                    client->alreadydn,
                    client->m,
                    _vec2<s32>( 0, 0 ),
                    0
                    );
                }
              }
              client->mousealreadydn[Cast( enum_t, btn )] = 0;
            }
          }
        } return 0;

        case RIM_TYPEKEYBOARD: {
#if GLW_RAWINPUT_KEYBOARD
          USHORT key = raw.data.keyboard.VKey;
          USHORT up  = raw.data.keyboard.Flags & RI_KEY_BREAK;
          // TODO: find a soln
          if( up ) {
            if( gKeyboard.key_isdn[key] ) {
              gKeyboard.key_up[key] = 1;
              gKeyboard.key_isdn[key] = 0;
            }
          } else {
            if( !gKeyboard.key_isdn[ key ] ) {
              gKeyboard.key_dn[key] = 1;
              gKeyboard.key_isdn[key] = 1;
            }
          }
#endif
        } return 0;

        case RIM_TYPEHID: {
          // TODO: HID support.
        } return 0;

      }
    } return 0;// end case WM_INPUT
  }
  return DefWindowProc( hwnd, msg, wp, lp );
}



void
GlwInit(
  glwclient_t& client,
  u8* title,
  idx_t title_len,
  bool auto_dim_windowed = 1,
  vec2<u32> dim_windowed = { 0, 0 },
  u16 anim_interval_millisec = 16,
  bool fullscreen = 0,
  bool cursor_visible = 1,
  glwcursortype_t cursortype = glwcursortype_t::arrow,
  u8 bits_rgba = 32,
  u8 bits_depth = 24,
  u8 bits_stencil = 8
  )
{
  ProfFunc();

  // Opt out of windows's dpi auto-scaling, so we can have hi-res fonts on hi-dpi screens!
  auto res = SetProcessDpiAwareness( PROCESS_PER_MONITOR_DPI_AWARE );
  if( res != S_OK ) {
    Log( "SetProcessDpiAwareness to PER_MONITOR_DPI_AWARE failed!" );
  }
  auto old_dpictx = SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );
  if( !old_dpictx ) {
    Log( "SetThreadDpiAwarenessContext to PER_MONITOR_AWARE_V2 failed!" );
    old_dpictx = SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE );
  }
  if( !old_dpictx ) {
    Log( "SetThreadDpiAwarenessContext to PER_MONITOR_AWARE failed!" );
  }

  // Client initialization.
  Alloc( client.cstr_title, 32 );
  Alloc( client.callbacks, 8 );

#if OPENGL_INSTEAD_OF_SOFTWARE
  Alloc( client.stream_fullscreen, 6*3 + 6*2 );
#endif

  client.alive = 1;

  For( i, 0, Cast( enum_t, glwkey_t::count ) ) {
    client.alreadydn[i] = 0;
  }

  For( i, 0, Cast( enum_t, glwmousebtn_t::count ) ) {
    client.mousealreadydn[i] = 0;
  }

  client.locks.caps   = GetKeyState( KeyOSFromGlw( glwkey_t::capslock   ) ) & 1;
  client.locks.num    = GetKeyState( KeyOSFromGlw( glwkey_t::numlock    ) ) & 1;
  client.locks.scroll = GetKeyState( KeyOSFromGlw( glwkey_t::scrolllock ) ) & 1;

  client.m = _vec2<s32>( 0, 0 );

  // TODO: abstract this a bit, so it's not so flaky initializing a waitable timer.
  static const s64 delay_over_millisec = -10000;

  client.timer_anim = CreateWaitableTimer( 0, 0, 0 );
  AssertWarn( client.timer_anim );
  s32 period_millisec = Cast( s32, anim_interval_millisec );
  s64 delay = delay_over_millisec * period_millisec;
  AssertWarn( SetWaitableTimer( client.timer_anim, Cast( LARGE_INTEGER*, &delay ), period_millisec, 0, 0, 0 ) );

  client.timestep_fixed = Cast( f64, anim_interval_millisec ) / 1000.0;


#if OPENGL_INSTEAD_OF_SOFTWARE
  Init(
    client.texid_map,
    512,
    sizeof( u32 ),
    sizeof( glwtexid_mapping_t ),
    0.75f,
    Equal_FirstU32,
    Hash_FirstU32
    );
#endif // OPENGL_INSTEAD_OF_SOFTWARE

  client.hi = GetModuleHandle( 0 );

  // REGISTER RAW INPUTS
  RAWINPUTDEVICE rawinputs [ 2 ];
  u32 nrawinputs = 1;
  rawinputs[0].usUsagePage = 1;           // Generic desktop controls
  rawinputs[0].usUsage = 2;               // mouse
  rawinputs[0].dwFlags = 0;               // Default usage
  rawinputs[0].hwndTarget = 0;         // Follow mouse focus
#if GLW_RAWINPUT_KEYBOARD
  nrawinputs = 2;
  rawinputs[1].usUsagePage = 1;           // Generic desktop controls
  rawinputs[1].usUsage = 6;               // Keyboard
#if GLW_RAWINPUT_KEYBOARD_NOHOTKEY
  rawinputs[1].dwFlags = RIDEV_NOHOTKEYS; // Disable Windows keys
#else
  rawinputs[1].dwFlags = 0;
#endif
  rawinputs[1].hwndTarget = 0;         // Follow keyboard focus
#endif
  AssertWarn( RegisterRawInputDevices( rawinputs, nrawinputs, sizeof( RAWINPUTDEVICE ) ) );

  // CREATE THE WINDOW
  DWORD style = WS_OVERLAPPEDWINDOW;
  DWORD styleex = WS_EX_APPWINDOW;
  // start with dummy size; we'll resize before finally showing.
  // we do this so we can ask for the monitor size, which requires a window.
  RECT rect = { 0, 0, 800, 600 };
  AdjustWindowRectEx( &rect, style, FALSE, styleex );

  // Make a cstr title.
  if( !title || title_len == 0 ) {
    title = Str( "default title" );
    title_len = CsLen( title );
  }
  auto dst_title = AddBack( client.cstr_title, title_len + 1 );
  CsCopy( dst_title, title, title_len );

  // Autogen an icon with a random color
  vec3<f32> palette[] = {
    _vec3<f32>( 1, 0, 0 ),
    _vec3<f32>( 1, 1, 0 ),
    _vec3<f32>( 1, 0, 1 ),
    _vec3<f32>( 0, 1, 0 ),
    _vec3<f32>( 0, 1, 1 ),
    _vec3<f32>( 0, 0, 1 ),
    _vec3<f32>( 238 / 255.0f, 197 / 255.0f, 55 / 255.0f ),
    _vec3<f32>( 32 / 255.0f, 127 / 255.0f, 244 / 255.0f ),
    _vec3<f32>( 44 / 255.0f, 57 / 255.0f, 238 / 255.0f ),
    _vec3<f32>( 68 / 255.0f, 190 / 255.0f, 112 / 255.0f ),
    _vec3<f32>( 205 / 255.0f, 163 / 255.0f, 195 / 255.0f ),
    };
  u64 val;
  while( !_rdrand64_step( &val ) );
  val = val >> ( val & AllOnes( 3 ) );
  auto color = palette[ val % _countof( palette ) ];
  u8 bitmask_xor[32*32*4];
  For( i, 0, 32*32 ) {
    auto x = i / 32;
    auto y = i % 32;
    auto mod = CLAMP( Pow32( 2 * MIN4( x, y, 31 - x, 31 - y ) / 31.0f, 0.75f ), 0, 1 );
    bitmask_xor[4*i+0] = Round_u32_from_f32( 255.0f * mod * color.x ) & AllOnes( 8 );
    bitmask_xor[4*i+1] = Round_u32_from_f32( 255.0f * mod * color.y ) & AllOnes( 8 );
    bitmask_xor[4*i+2] = Round_u32_from_f32( 255.0f * mod * color.z ) & AllOnes( 8 );
    bitmask_xor[4*i+3] = Round_u32_from_f32( 255.0f * mod ) & AllOnes( 8 );
  }
  u8 bitmask_and[32*32];
  memset( bitmask_and, 0xFF, _countof( bitmask_and ) );
  auto icon = CreateIcon(
    client.hi,
    32,
    32,
    4,
    8,
    bitmask_and,
    bitmask_xor
    );

  // register this window.
  WNDCLASS wc;
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = client.hi;
//  wc.hIcon = LoadIcon( 0, IDI_WINLOGO );
  wc.hIcon = icon;
  wc.hCursor = 0;
  wc.hbrBackground = 0;
  wc.lpszMenuName = 0;
  wc.lpszClassName = Cast( char*, client.cstr_title.mem );
  AssertWarn( RegisterClass( &wc ) );

  client.hwnd = CreateWindowEx(
    styleex,
    Cast( char*, client.cstr_title.mem ),
    Cast( char*, client.cstr_title.mem ),
    style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // window style
    //0, 0, // x, y
    CW_USEDEFAULT, CW_USEDEFAULT, // x, y
    rect.right  - rect.left, // w
    rect.bottom - rect.top, // h
    0, // parent window
    0, // menu
    client.hi, // hinstance for this window
    &client // lp for WM_CREATE
    );

  AssertCrash( client.hwnd );

  // get the device context from the window.
  client.window_dc = GetDC( client.hwnd );
  AssertCrash( client.window_dc );

#if OPENGL_INSTEAD_OF_SOFTWARE
  // TODO: do we even need to ChoosePixelFormat when on software rendering?
  // PERF: ChoosePixelFormat is super slow, on the order of 0.5 seconds.
  Prof( ChoosePixelFormat );


#if OPENGL_INSTEAD_OF_SOFTWARE
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  bits_depth = 0;
  bits_stencil = 0;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  // apply a pixel format.
  PIXELFORMATDESCRIPTOR pfd = { 0 };
  pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
  pfd.nVersion = 1;
#if OPENGL_INSTEAD_OF_SOFTWARE
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | PFD_DEPTH_DONTCARE;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = bits_rgba;
  pfd.cDepthBits = bits_depth;
  pfd.cStencilBits = bits_stencil;
  pfd.iLayerType = PFD_MAIN_PLANE;
  int pixformat = ChoosePixelFormat( client.window_dc, &pfd );
  AssertWarn( pixformat );

  ProfClose( ChoosePixelFormat );

#if 0 // Interesting pixel format investigations:
  PIXELFORMATDESCRIPTOR pfd2 = { 0 };
  pfd2.nSize = sizeof( PIXELFORMATDESCRIPTOR );
  pfd2.nVersion = 1;
  auto pfd_count = DescribePixelFormat( client.window_dc, 0, sizeof( PIXELFORMATDESCRIPTOR ), 0 );
  AssertWarn( pfd_count );
  Fori( s32, i, 1, pfd_count + 1 ) {
    AssertWarn( DescribePixelFormat( client.window_dc, i, sizeof( PIXELFORMATDESCRIPTOR ), &pfd2 ) );
    if( pfd2.dwFlags & PFD_DRAW_TO_WINDOW  &&
//        pfd2.dwFlags & PFD_SUPPORT_OPENGL  &&
//        pfd2.dwFlags & PFD_DOUBLEBUFFER  &&
        pfd2.dwFlags & PFD_SUPPORT_COMPOSITION  &&
        !( pfd2.dwFlags & PFD_NEED_PALETTE )  &&
        !( pfd2.dwFlags & PFD_NEED_SYSTEM_PALETTE )  &&
        pfd2.iLayerType == 0  &&
        pfd2.iPixelType == 0 ) {

      u8 tmp[65];
      idx_t tmp_len;
      CsFromIntegerU(
        AL( tmp ),
        &tmp_len,
        Cast( u32, pfd2.dwFlags ),
        0,
        0,
        0,
        2,
        Cast( u8*, "01" )
        );

      int x = 0;
    }
  }

//  pixformat = 207;
//  pixformat = 213;
//  AssertWarn( DescribePixelFormat( client.window_dc, pixformat, sizeof( PIXELFORMATDESCRIPTOR ), &pfd2 ) );
#endif

  AssertWarn( DescribePixelFormat( client.window_dc, pixformat, sizeof( PIXELFORMATDESCRIPTOR ), &pfd ) );
  AssertWarn( SetPixelFormat( client.window_dc, pixformat, &pfd ) );
#endif // OPENGL_INSTEAD_OF_SOFTWARE


#if OPENGL_INSTEAD_OF_SOFTWARE
  // load wgl functions so we can create a wgl context.
  LoadWGLFunctions( client.window_dc );

  // create the wgl context.
  static const int context_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_FLAGS_ARB, 0
      | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if DBG
      | WGL_CONTEXT_DEBUG_BIT_ARB
#endif
    ,
    WGL_CONTEXT_PROFILE_MASK_ARB, 0
      | WGL_CONTEXT_CORE_PROFILE_BIT_ARB
      //| WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
    ,
    0 // terminator
  };
  client.hgl = wglCreateContextAttribsARB( client.window_dc, 0, context_attribs );
  if( !client.hgl ) {
    AssertWarn( 0 );
    client.hgl = wglCreateContext( client.window_dc );
    Log( "GL VERSION 3.3 context creation failed. falling back to default..." );
  }
  AssertCrash( client.hgl );

  AssertWarn( wglMakeCurrent( client.window_dc, client.hgl ) );  glVerify();

  LoadOpenGLFunctions( client.window_dc );

  auto versionstr = glGetString( GL_VERSION );
  int major, minor;
  glGetIntegerv( GL_MAJOR_VERSION, &major );
  glGetIntegerv( GL_MINOR_VERSION, &minor );
  Log( "[GL VERSION] string: %s, ints: %d.%d", versionstr, major, minor );

  glEnable( GL_BLEND );  glVerify();
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );  glVerify();

  glEnable( GL_DEPTH_TEST );  glVerify();
  glClearDepth( 0 );  glVerify();
  glDepthFunc( GL_GEQUAL );  glVerify();

  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );  glVerify();

  glDisable( GL_CULL_FACE );  glVerify();

  Init( client.shader_fullscreen );

  glGenBuffers( 1, &client.glstream_fullscreen );

  glGenVertexArrays( 1, &client.vao );  glVerify();
  glBindVertexArray( client.vao );  glVerify();
#endif // OPENGL_INSTEAD_OF_SOFTWARE

#if OPENGL_INSTEAD_OF_SOFTWARE
  Init( client.target );
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  client.fullscreen_bitmap_dc = 0;
  client.fullscreen_bitmap = 0;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  client.target_valid = 0;

  if( auto_dim_windowed ) {
    auto monitor_size = _GetMonitorSize( client.hwnd );
    dim_windowed = monitor_size / 2u;
  }
  AssertCrash( dim_windowed.x );
  AssertCrash( dim_windowed.y );
  client.dim = dim_windowed;
  client.dimf.x = Cast( f32, client.dim.x );
  client.dimf.y = Cast( f32, client.dim.y );
  // WARNING!!!
  // this triggers a WM_SIZE message, which we also use for handling resize.
  // our methodology here is to create everything with a dummy size, then resize at the last possible moment, here.
  _SetWindowSize( client.hwnd, client.dim );

  ShowWindow( client.hwnd, SW_SHOW );
  SetForegroundWindow( client.hwnd );
  SetFocus( client.hwnd );

  _SetFullscreen( client.hwnd, client.dim, client.dimf, fullscreen );
  client.fullscreen = fullscreen;

  _SetCursortype( cursortype );
  client.cursortype = cursortype;
  ShowCursor( cursor_visible );

  client.dpi = GetDpiForWindow( client.hwnd );



//  int real_pxfmt = GetPixelFormat( client.window_dc );
//  if( !real_pxfmt ) {
//    int err = GetLastError();
//    AssertCrash( !err );
//  }
//  PIXELFORMATDESCRIPTOR pfd3 = { 0 };
//  pfd3.nSize = sizeof( PIXELFORMATDESCRIPTOR );
//  pfd3.nVersion = 1;
//  AssertWarn( DescribePixelFormat( client.window_dc, real_pxfmt, sizeof( PIXELFORMATDESCRIPTOR ), &pfd3 ) );

}


void
GlwKill( glwclient_t& client )
{
#if OPENGL_INSTEAD_OF_SOFTWARE
  Kill( client.shader_fullscreen );
  glDeleteBuffers( 1, &client.glstream_fullscreen );
  Free( client.stream_fullscreen );

  glDeleteVertexArrays( 1, &client.vao );  glVerify();

  Kill( client.target );
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  DeleteTarget( client );
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  client.target_valid = 0;

#if OPENGL_INSTEAD_OF_SOFTWARE
  AssertWarn( wglMakeCurrent( 0, 0 ) );
  AssertWarn( wglDeleteContext( client.hgl ) );
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  AssertWarn( ReleaseDC( client.hwnd, client.window_dc ) );
  AssertWarn( DestroyWindow( client.hwnd ) );
  AssertWarn( UnregisterClass( Cast( char*, client.cstr_title.mem ), client.hi ) );

#if OPENGL_INSTEAD_OF_SOFTWARE
  client.hgl = 0;
#endif // OPENGL_INSTEAD_OF_SOFTWARE

  client.window_dc = 0;
  client.hwnd = 0;
  client.hi = 0;

  AssertCrash( !client.alive );
  Free( client.cstr_title );
  Free( client.callbacks );

#if OPENGL_INSTEAD_OF_SOFTWARE
  Kill( client.texid_map );
#endif // OPENGL_INSTEAD_OF_SOFTWARE

  CloseHandle( client.timer_anim );
}



#if 0
Inl void
ReserveGlStreams( glwclient_t& client, idx_t count )
{
  if( client.gl_streams.len < count ) {
    auto orig_size = client.gl_streams.len;
    auto num_add = count - orig_size;
    client.gl_streams.len += num_add;
    Reserve( client.gl_streams, client.gl_streams.len );
    AssertCrash( num_add <= MAX_s32 );
    glGenBuffers( Cast( s32, num_add ), &client.gl_streams[orig_size] );  glVerify();
  }
}
#endif


#if OPENGL_INSTEAD_OF_SOFTWARE

  u32
  GlwLookupGlid(
    glwclient_t& client,
    u32 texid_client
    )
  {
    glwtexid_mapping_t mapping;
    bool found;
    Lookup( client.texid_map, &texid_client, &found, &mapping );
    AssertCrash( found );
    return mapping.glid;
  }

#endif // OPENGL_INSTEAD_OF_SOFTWARE


void
GlwMainLoop( glwclient_t& client )
{
  HANDLE wait_timers[] = {
    client.timer_anim,
    g_mainthread.wake_asynctaskscompleted,
  };

  client.time_render1 = TimeTSC();

  do { // while( client.alive )
    bool do_queue = 0;
    bool do_render = 0;
    bool do_asynctaskscompleted = 0;

    static const s32 wait_timeout_millisec = 1000;
    Prof( GlwMainLoop_Sleep );
    DWORD waitres = MsgWaitForMultipleObjectsEx( _countof( wait_timers ), wait_timers, wait_timeout_millisec, QS_ALLINPUT, 0 );
    ProfClose( GlwMainLoop_Sleep );
    switch( waitres ) {
      case WAIT_FAILED:
      case WAIT_TIMEOUT: {
        AssertWarn( 0 );
      } break;
      case WAIT_OBJECT_0 + 0: { // client.timer_anim
        if( !client.target_valid ) {
          do_render = 1;
        }
      } break;
      case WAIT_OBJECT_0 + 1: { // g_mainthread.wake_asynctaskscompleted
        // TODO: should we do_queue ?
        do_asynctaskscompleted = 1;
        do_render = 1;
      } break;
      case WAIT_OBJECT_0 + 2: { // Extra return value, for when some input/window message is available.
        do_queue = 1;
        do_render = 1;
      } break;
      default: UnreachableCrash();
    }

    if( do_queue ) {
      // process winapi messages.
      MSG msg;
      while( PeekMessage( &msg, client.hwnd, 0, 0, PM_REMOVE ) ) {
        if( LOWORD( msg.message ) == WM_QUIT ) {
          GlwEarlyKill( client );
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
      }
    }

    if( do_asynctaskscompleted ) {
      Forever {
        bool success;
        maincompletedqueue_entry_t me;
        DequeueS( g_mainthread.maincompletedqueue, &me, &success );
        if( !success ) {
          break;
        }
        me.FnMainTaskCompleted( me.maincontext, me.asynccontext, &client.target_valid );
      }
    }

    do_render &= ( client.dim.x != 0 );
    do_render &= ( client.dim.y != 0 );
    if( do_render ) {
      _Render( client );
    }

  } while( client.alive );
}





constant idx_t c_propsize = 24;

CompileAssert( c_propsize >= sizeof( vec4<f32> ) );
CompileAssert( c_propsize >= sizeof( glwkeybind_t ) );


struct
propdb_t
{
  fsobj_t filename;
  hashset_t set;
  plist_t strdata;
  u64 time_lastwrite;
};

static propdb_t g_db = {};
static bool g_db_init = 0;


Inl void
_Kill( propdb_t& db )
{
  Kill( db.set );
  Kill( db.strdata );
}



// Assumes a file format s.t.
//   The following will exist on a line, with n >= 1:
//     key = value0 , value1 , ... , valueN-1 ;


Enumc( tokentype_t )
{
  comma,
  equals,
  semicolon,
  expr, // ident string or numeral string.

  COUNT
};


struct
token_t
{
  idx_t l;
  idx_t r;
  tokentype_t type;
};


Inl bool
IsNumeralStart( u8 c )
{
  bool r = IsNumber( c )  |  ( c == '-' );
  return r;
}



bool
Tokenize( array_t<token_t>& tokens, slice_t& src )
{
  idx_t pos = 0;
  token_t last_tkn;
  last_tkn.l = 0;
  last_tkn.r = 0;
  last_tkn.type = tokentype_t::expr;

  Forever {

    if( pos >= src.len ) {
      break;
    }

    u8 curr_src = src.mem[pos];
    switch( curr_src ) {
      case ',': {
        last_tkn.r = pos;
        *AddBack( tokens ) = last_tkn;

        token_t tkn;
        tkn.l = pos;
        tkn.r = pos + 1;
        tkn.type = tokentype_t::comma;
        *AddBack( tokens ) = tkn;

        last_tkn.l = pos + 1;
      } break;

      case '=': {
        last_tkn.r = pos;
        *AddBack( tokens ) = last_tkn;

        token_t tkn;
        tkn.l = pos;
        tkn.r = pos + 1;
        tkn.type = tokentype_t::equals;
        *AddBack( tokens ) = tkn;

        last_tkn.l = pos + 1;
      } break;

      case ';': {
        last_tkn.r = pos;
        *AddBack( tokens ) = last_tkn;

        token_t tkn;
        tkn.l = pos;
        tkn.r = pos + 1;
        tkn.type = tokentype_t::semicolon;
        *AddBack( tokens ) = tkn;

        last_tkn.l = pos + 1;
      } break;

      default: {

      } break;
    }

    pos += 1;
  }

  // remove lead/trail whitespace from tokentype_t::expr.
  ForLen( i, tokens ) {
    auto tkn = tokens.mem + i;
    if( tkn->type == tokentype_t::expr ) {
      while( tkn->l < tkn->r  &&  IsWhitespace( src.mem[tkn->l] ) ) {
        tkn->l += 1;
      }
      while( tkn->l < tkn->r  &&  IsWhitespace( src.mem[tkn->r - 1] ) ) {
        tkn->r -= 1;
      }
    }
  }

  return 1;
}




Inl u8*
StringOfTokenType( tokentype_t type )
{
#define CASEPRINTTKN( x )   case x: return Str( # x );
  switch( type ) {
    CASEPRINTTKN( tokentype_t::comma );
    CASEPRINTTKN( tokentype_t::equals );
    CASEPRINTTKN( tokentype_t::semicolon );
    CASEPRINTTKN( tokentype_t::expr );
    default: UnreachableCrash();
  }
#undef CASEPTRINTTKN
  return 0;
}



#if 0
Inl void
PrintTokenStream( array_t<token_t>& tokens, slice_t& src )
{
  ForLen( i, tokens ) {
    auto tkn = tokens.mem + i;
    idx_t tkn_len = tkn->r - tkn->l;
    auto tmp = MemHeapAlloc( u8, tkn_len );
    Memmove( &tmp[0], src.mem + tkn->l, tkn_len );
    tmp[tkn_len] = 0;
    printf(
      "( %s ) ( %llu, %llu ) ( %s )"
      "\n",
      StringOfTokenType( tkn->type ),
      tkn->l, tkn_len,
      &tmp[0]
      );
    MemHeapFree( tmp );
  }
}
#endif


struct
statement_t
{
  slice_t lhs;
  array_t<slice_t> rhs;
};

Inl void
Init( statement_t& stm )
{
  stm.lhs = {};
  Alloc( stm.rhs, 8 );
}

Inl void
Kill( statement_t& stm )
{
  stm.lhs = {};
  Free( stm.rhs );
}

struct
ast_t
{
  array_t<statement_t> stms;
};

Inl void
Init( ast_t& ast, idx_t capacity )
{
  Alloc( ast.stms, capacity );
}

Inl void
Kill( ast_t& ast )
{
  ForLen( i, ast.stms ) {
    auto stm = ast.stms.mem + i;
    Kill( *stm );
  }
  Free( ast.stms );
}


Inl token_t*
ExpectToken( array_t<token_t>& tokens, idx_t i )
{
  if( i >= tokens.len ) {
    Log( "Expected a token, but hit EOF instead!" );
    return 0;
  }
  return tokens.mem + i;
}

Inl token_t*
ExpectTokenOfType( array_t<token_t>& tokens, idx_t i, tokentype_t type )
{
  auto tkn = ExpectToken( tokens, i );
  if( !tkn ) return 0;
  if( tkn->type != type ) {
    Log( "Expected '%s', but found '%s' instead!", StringOfTokenType( type ), StringOfTokenType( tkn->type ) );
    return 0;
  }
  return tkn;
}

bool
Parse( ast_t& ast, array_t<token_t>& tokens, slice_t& src )
{
  idx_t i = 0;
  Forever {
    if( i >= tokens.len ) {
      break;
    }

    auto tkn_key = ExpectTokenOfType( tokens, i, tokentype_t::expr );
    if( !tkn_key ) return 0;
    i += 1;

    statement_t stm;
    Init( stm );

    stm.lhs.mem = src.mem + tkn_key->l;
    stm.lhs.len = tkn_key->r - tkn_key->l;

    auto tkn_equals = ExpectTokenOfType( tokens, i, tokentype_t::equals );
    if( !tkn_equals ) return 0;
    i += 1;

    bool read_values = 1;
    while( read_values ) {
      auto tkn_value = ExpectTokenOfType( tokens, i, tokentype_t::expr );
      if( !tkn_value ) return 0;
      i += 1;

      auto tkn_comma_or_semicolon = ExpectToken( tokens, i );
      if( !tkn_comma_or_semicolon ) return 0;
      i += 1;

      switch( tkn_comma_or_semicolon->type ) {
        case tokentype_t::comma: {
          // continue reading values
        } break;
        case tokentype_t::semicolon: {
          read_values = 0;
        } break;
        default: {
          Log( "Expected '%s' or '%s', but found '%s' instead!",
            StringOfTokenType( tokentype_t::comma ),
            StringOfTokenType( tokentype_t::semicolon ),
            StringOfTokenType( tkn_comma_or_semicolon->type )
            );
          return 0;
        } break;
      } // switch

      auto buf = AddBack( stm.rhs );
      buf->mem = src.mem + tkn_value->l;
      buf->len = tkn_value->r - tkn_value->l;
    } // while( read_values )

    *AddBack( ast.stms ) = stm;

  } // Forever
  return 1;
}


Templ Inl void
_AddToDb( propdb_t& db, slice_t& lhs, T& val )
{
  u8 propmem[c_propsize] = {};
  Memmove( propmem, &val, sizeof( val ) );
  slice_t name;
  name.len = lhs.len;
  name.mem = AddPlist( u8, db.strdata, name.len );
  Memmove( name.mem, lhs.mem, name.len );
  Add( db.set, &name, propmem, 0, 0, 1 );
}


Inl bool
_LoadFromMem( propdb_t& db, slice_t& mem )
{
  idx_t statement_estimate = mem.len / 10;
  array_t<token_t> tokens;
  Alloc( tokens, statement_estimate );
  ast_t ast;
  bool r = Tokenize( tokens, mem );
  if( r ) {
    Init( ast, statement_estimate );
    r = Parse( ast, tokens, mem );
    if( r ) {
      ForLen( i, ast.stms ) {
        auto stm = ast.stms.mem + i;
        if( 0 ) {
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 7 ), "keybind", 7 ) ) {
          if( stm->rhs.len <= 6 ) {
            bool add_keybind = 1;
            glwkey_t keys[6] = {};
            ForLen( j, stm->rhs ) {
              auto keystring = stm->rhs.mem + j;
              glwkey_t found_key;
              bool found;
              KeyGlwFromString( *keystring, &found, &found_key );
              if( found ) {
                keys[j] = found_key;
              } else {
                auto cstr = AllocCstr( *keystring );
                Log( "Not a valid key string: %s", cstr );
                MemHeapFree( cstr );
                r = 0;
                add_keybind = 0;
                break;
              }
            }
            if( add_keybind ) {
              auto val = _glwkeybind( keys[0], keys[1], keys[2], keys[3], keys[4], keys[5] );
              _AddToDb( db, stm->lhs, val );
            }
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected up to 6 key values: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 6 ), "string", 6 ) ) {
          if( stm->rhs.len != 1 ) {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          } else {
            auto rhs = stm->rhs.mem + 0;
            if( rhs->len < 2 ) {
              auto cstr = AllocCstr( stm->lhs );
              Log( "Expected double quotes around your string: %s", cstr );
              MemHeapFree( cstr );
              r = 0;
            } else {
              slice_t val;
              val.len = rhs->len - 2;
              val.mem = AddPlist( u8, db.strdata, val.len );
              Memmove( val.mem, rhs->mem + 1, val.len );
              _AddToDb( db, stm->lhs, val );
            }
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 4 ), "bool", 4 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = !!CsTo_u64( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 4 ), "rgba", 4 ) ) {
          if( stm->rhs.len != 4 ) {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected four numeric values in [0.0, 1.0]: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          } else {
            vec4<f32> val;
            bool success = 1;
            ForLen( j, stm->rhs ) {
              auto rhs = stm->rhs.mem + j;
              auto component = Cast( f32*, &val ) + j;
              if( !CsToFloat32( rhs->mem, rhs->len, *component ) ) {
                auto cstr = AllocCstr( stm->lhs );
                Log( "Unable to parse floating point value at index %llu: %s", j, cstr );
                MemHeapFree( cstr );
                success = 0;
                break;
              }
            }
            if( !success ) {
              r = 0;
            } else {
              _AddToDb( db, stm->lhs, val );
            }
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "f32", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_f32( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "f64", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_f64( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "u64", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_u64( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "s64", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_s64( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "u32", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_u32( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "s32", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_s32( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "u16", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_u16( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 3 ), "s16", 3 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_s16( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 2 ), "u8", 2 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_u8( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 2 ), "s8", 2 ) ) {
          if( stm->rhs.len == 1 ) {
            auto rhs = stm->rhs.mem + 0;
            auto val = CsTo_s8( rhs->mem, rhs->len );
            _AddToDb( db, stm->lhs, val );
          } else {
            auto cstr = AllocCstr( stm->lhs );
            Log( "Expected a single value: %s", cstr );
            MemHeapFree( cstr );
            r = 0;
          }
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 2 ), "v2", 2 ) ) {
          // TODO: implement
          auto cstr = AllocCstr( stm->lhs );
          Log( "We don't handle this property type yet: %s", cstr );
          MemHeapFree( cstr );
          r = 0;
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 2 ), "v3", 2 ) ) {
          // TODO: implement
          auto cstr = AllocCstr( stm->lhs );
          Log( "We don't handle this property type yet: %s", cstr );
          MemHeapFree( cstr );
          r = 0;
        } elif( MemEqual( stm->lhs.mem, MIN( stm->lhs.len, 2 ), "v4", 2 ) ) {
          // TODO: implement
          auto cstr = AllocCstr( stm->lhs );
          Log( "We don't handle this property type yet: %s", cstr );
          MemHeapFree( cstr );
          r = 0;
        } else {
          auto cstr = AllocCstr( stm->lhs );
          Log( "Unrecognized property type: %s", cstr );
          MemHeapFree( cstr );
          r = 0;
        }
      }
    }
  }

  Kill( ast );
  Free( tokens );
  return r;
}

void
_Init( propdb_t& db )
{
  ProfFunc();

  Init(
    db.set,
    512,
    sizeof( slice_t ),
    c_propsize,
    0.8f,
    Equal_SliceContents,
    Hash_SliceContents
    );

  Init( db.strdata, 32768 );

  db.time_lastwrite = 0;

  // TODO: OS abstraction.
  db.filename.len = GetModuleFileName( 0, Cast( LPSTR, db.filename.mem ), Cast( DWORD, Capacity( db.filename ) ) );
  db.filename = _StandardFilename( ML( db.filename ) );

  if( db.filename.len ) {
    auto last_dot = CsScanL( ML( db.filename ), '.' );
    AssertCrash( last_dot );
    db.filename.len = ( last_dot - db.filename.mem );
    db.filename.len += 1; // include dot.
    Memmove( AddBack( db.filename, 6 ), "config", 6 );
  }

  // TODO: timestep_fixed ?
  // TODO: font, with a fixed backup.
  // TODO: font size

  Log( "PROPDB INITIALIZE" );
  LogAddIndent( +1 );

  auto cstr = AllocCstr( ML( db.filename ) );
  Log( "Opening config file: %s", cstr );
  MemHeapFree( cstr );

  file_t file = FileOpen( ML( db.filename ), fileopen_t::always, fileop_t::RW, fileop_t::R );
  string_t mem;
  Zero( mem );
  if( file.loaded ) {
    mem = FileAlloc( file );
  } else {
    cstr = AllocCstr( ML( db.filename ) );
    Log( "Can't open the config file: %s", cstr );
    MemHeapFree( cstr );
  }
  FileFree( file );

  auto slice = SliceFromString( mem );
  if( !_LoadFromMem( db, slice ) ) {
    Log( "Failed to _LoadFromMem!" );
  }
  Free( mem );

  LogAddIndent( -1 );
  Log( "" );
}

Inl void*
_GetProp( propdb_t& db, u8* name )
{
  bool found;
  void* prop;
  auto slice = SliceFromCStr( name );
  LookupRaw( db.set, &slice, &found, Cast( void**, &prop ) );
  if( !found ) {
    Log( "PROPDB ACCESS FAILURE" );
    LogAddIndent( +1 );
    Log( "Property not found: %s", name );
    LogAddIndent( -1 );
    Log( "" );
    constant u8 c_zeroprop[c_propsize] = {};
    return Cast( void*, c_zeroprop );
  } else {
    return prop;
  }
}

__OnMainKill( KillPropdb )
{
  auto db = Cast( propdb_t*, user );
  _Kill( *db );
}

Inl void
_InitPropdb()
{
  if( !g_db_init ) {
    g_db_init = 1;
    _Init( g_db );
    RegisterOnMainKill( KillPropdb, &g_db );
  }
}

Inl void*
GetProp( u8* name )
{
  _InitPropdb();
  return _GetProp( g_db, name );
}



#define GetPropFromDb( T, name ) \
  *Cast( T*, GetProp( Str( #name ) ) )





struct
fontglyph_t
{
  vec2<f32> offset;
  vec2<f32> tc0;
  vec2<f32> tc1;
  vec2<f32> dim;
  f32 advance;
};




struct
fontadvance_t
{
  u32 codept;
  fontglyph_t* glyph;
  f32 absolute_x;
};

struct
fontlinespan_t
{
  idx_t pos;
  idx_t len;
  f32 width;
};

struct
fontlayout_t
{
  array_t<fontadvance_t> raw_advances;
  array_t<fontlinespan_t> advances_per_ln;
};

Inl void
FontInit( fontlayout_t& text )
{
  Alloc( text.raw_advances, 16 );
  Alloc( text.advances_per_ln, 16 );
}

Inl void
FontKill( fontlayout_t& text )
{
  Free( text.raw_advances );
  Free( text.advances_per_ln );
}

#define FontClear( text ) \
  do { \
    text.raw_advances.len = 0; \
    text.advances_per_ln.len = 0; \
  } while( 0 )

#define FontEmpty( text ) \
  ( text.raw_advances.len == 0 )


struct
font_t
{
  string_t ttf;
  stbtt_fontinfo info;
  array_t<idx_t> glyph_from_codept;
  array_t<fontglyph_t> glyphs;
  u32 texid;
  f32 scale;
  f32 ascent;
  f32 descent;
  f32 linegap;
#if OPENGL_INSTEAD_OF_SOFTWARE
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  vec2<u32> tex_dim;
  u32* tex_mem;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
};


#define FontGetGlyph( font, codept ) \
  ( font.glyphs.mem + font.glyph_from_codept.mem[codept] )


void
FontLoad(
  font_t& font,
  u8* filename_ttf,
  idx_t filename_ttf_len,
  f32 px_char_h
  )
{
  ProfFunc();

  file_t file = FileOpen( filename_ttf, filename_ttf_len, fileopen_t::only_existing, fileop_t::R, fileop_t::RW );
  AssertCrash( file.loaded );
  font.ttf = FileAlloc( file );
  FileFree( file );
  AssertCrash( font.ttf.mem != 0 );

  stbtt_InitFont( &font.info, font.ttf.mem, stbtt_GetFontOffsetForIndex( font.ttf.mem, 0 ) );

  font.scale = stbtt_ScaleForPixelHeight( &font.info, px_char_h );

  s32 ascent, descent, linegap;
  stbtt_GetFontVMetrics( &font.info, &ascent, &descent, &linegap );
  font.ascent = font.scale * ascent;
  font.descent = -font.scale * descent;
  font.linegap = CLAMP( font.scale * linegap, 1.0f, 2.0f );

  Alloc( font.glyph_from_codept, 1024 );
  font.glyph_from_codept.len = 1024;

  Alloc( font.glyphs, 1024 );
}

void
FontKill( font_t& font )
{
#if OPENGL_INSTEAD_OF_SOFTWARE
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  font.tex_dim = {};
  MemHeapFree( font.tex_mem );
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
  Free( font.glyph_from_codept );
  Free( font.glyphs );
  Free( font.ttf );
}


#define FontLineH( font ) \
  ( font.ascent + font.descent + font.linegap )


#define GLYGPHGEN 5
// 0 -- straight from stbtt
// 1 -- subpixel bgr blending
// 2 -- simple sharpen filter, border px passthru.
// 3 -- simple sharpen filter
// 4 -- sharpen filter
// 5 -- sharpen filter with subpixel bgr blending.

u32*
FontLoadGlyphImage(
  font_t& font,
  u32 codept,
  vec2<f32>& dimf,
  vec2<f32>& offsetf
  )
{
  ProfFunc();

#if GLYGPHGEN == 0
  vec2<s32> dim, offset;
  auto img_alpha = stbtt_GetCodepointBitmap(
    &font.info,
    font.scale,
    font.scale,
    codept,
    &dim.x,
    &dim.y,
    &offset.x,
    &offset.y
    );
  dimf = _vec2( Cast( f32, dim.x ), Cast( f32, dim.y ) );
  offsetf = _vec2( Cast( f32, offset.x ), Cast( f32, offset.y ) );
  auto img_alpha_len = dim.x * dim.y;
  auto img = MemHeapAlloc( u32, img_alpha_len );
  For( i, 0, img_alpha_len ) {
    auto dst = img + i;
    auto& src = img_alpha[i];
    *dst =
      ( src << 24 ) |
      ( src << 16 ) |
      ( src <<  8 ) |
      ( src <<  0 );
  }
  stbtt_FreeBitmap( img_alpha, 0 );
  return img;
#elif GLYGPHGEN == 1
  vec2<s32> dim_b, offset_b;
  auto img_b = stbtt_GetCodepointBitmapSubpixel(
    &font.info,
    font.scale,
    font.scale,
    0.0f / 3.0f,
    0.0f,
    codept,
    &dim_b.x,
    &dim_b.y,
    &offset_b.x,
    &offset_b.y
    );
  vec2<s32> dim_g, offset_g;
  auto img_g = stbtt_GetCodepointBitmapSubpixel(
    &font.info,
    font.scale,
    font.scale,
    1.0f / 3.0f,
    0.0f,
    codept,
    &dim_g.x,
    &dim_g.y,
    &offset_g.x,
    &offset_g.y
    );
  vec2<s32> dim_r, offset_r;
  auto img_r = stbtt_GetCodepointBitmapSubpixel(
    &font.info,
    font.scale,
    font.scale,
    2.0f / 3.0f,
    0.0f,
    codept,
    &dim_r.x,
    &dim_r.y,
    &offset_r.x,
    &offset_r.y
    );

  AssertWarn( dim_r.y == dim_g.y  &&  dim_g.y == dim_b.y );

  auto dim = MAX( MAX( dim_r, dim_g ), dim_b );
  dim.x += 4;

  auto offset = offset_b;

  auto align_r = offset_r.x - offset_b.x;
  auto align_g = offset_g.x - offset_b.x;
  auto align_b = 0;

  auto dim_blend = _vec2( dim.x, dim.y );
  auto img_blend_len = dim_blend.x * dim_blend.y;
  auto img_blend = MemHeapAlloc( vec4<f32>, img_blend_len );
  Memzero( img_blend, img_blend_len * sizeof( vec4<f32> ) );

  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim_r.x ) {
      auto idx = i + align_r;
      if( idx < dim_blend.x ) {
        auto& dst = img_blend[ j * dim_blend.x + idx ];
        auto val = Cast( f32, img_r[ j * dim_r.x + i ] ) / 255.0f;
        dst.x += val;
        dst.w += val / 3;
      }
    }
    Fori( s32, i, 0, dim_g.x ) {
      auto idx = i + align_g;
      if( idx < dim_blend.x ) {
        auto& dst = img_blend[ j * dim_blend.x + idx ];
        auto val = Cast( f32, img_g[ j * dim_g.x + i ] ) / 255.0f;
        dst.y += val;
        dst.w += val / 3;
      }
    }
    Fori( s32, i, 0, dim_b.x ) {
      auto idx = i + align_b;
      if( idx < dim_blend.x ) {
        auto& dst = img_blend[ j * dim_blend.x + idx ];
        auto val = Cast( f32, img_b[ j * dim_b.x + i ] ) / 255.0f;
        dst.z += val;
        dst.w += val / 3;
      }
    }
  }

  auto img_output_len = dim.x * dim.y;
  auto img_output = MemHeapAlloc( u32, img_output_len );
  Fori( s32, i, 0, img_output_len ) {
    auto& src = img_blend[i];
    auto& dst = img_output[i];
    dst =
      ( Cast( u8, src.x * 255.0f + 0.0001f ) <<  0 ) |
      ( Cast( u8, src.y * 255.0f + 0.0001f ) <<  8 ) |
      ( Cast( u8, src.z * 255.0f + 0.0001f ) << 16 ) |
      ( Cast( u8, src.w * 255.0f + 0.0001f ) << 24 );
  }
  dimf = _vec2( Cast( f32, dim.x ), Cast( f32, dim.y ) );
  offsetf = _vec2( Cast( f32, offset.x ), Cast( f32, offset.y ) );
  MemHeapFree( img_blend );
  stbtt_FreeBitmap( img_r, 0 );
  stbtt_FreeBitmap( img_g, 0 );
  stbtt_FreeBitmap( img_b, 0 );
  return img_output;
#elif GLYGPHGEN == 2
  vec2<s32> dim, offset;
  auto img_alpha = stbtt_GetCodepointBitmap(
    &font.info,
    font.scale,
    font.scale,
    codept,
    &dim.x,
    &dim.y,
    &offset.x,
    &offset.y
    );

  auto dim_blend = _vec2( dim.x, dim.y );
  auto img_blend_len = dim_blend.x * dim_blend.y;
  auto img_blend = MemHeapAlloc( vec4<f32>, img_blend_len );
  Memzero( img_blend, img_blend_len * sizeof( vec4<f32> ) );

  Fori( s32, j, 0, dim.y ) {
    {
      auto& dst = img_blend[ j * dim_blend.x + 0 ];
      auto val = Cast( f32, img_alpha[ j * dim.x + 0 ] ) / 255.0f;
      dst = _vec4( val );
    }
    {
      auto& dst = img_blend[ j * dim_blend.x + dim.x - 1 ];
      auto val = Cast( f32, img_alpha[ j * dim.x + dim.x - 1 ] ) / 255.0f;
      dst = _vec4( val );
    }
  }
  Fori( s32, i, 0, dim.x ) {
    {
      auto& dst = img_blend[ 0 * dim_blend.x + i ];
      auto val = Cast( f32, img_alpha[ 0 * dim.x + i ] ) / 255.0f;
      dst = _vec4( val );
    }
    {
      auto& dst = img_blend[ ( dim.y - 1 ) * dim_blend.x + i ];
      auto val = Cast( f32, img_alpha[ ( dim.y - 1 ) * dim.x + i ] ) / 255.0f;
      dst = _vec4( val );
    }
  }
  Fori( s32, j, 1, dim.y - 1 ) {
    Fori( s32, i, 1, dim.x - 1 ) {
      auto& dst = img_blend[ j * dim_blend.x + i ];
      static const f32 coeffs[] = {
         0, -1,  0,
        -1,  5, -1,
         0, -1,  0,
      };
      f32 vals[] = {
        Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i - 1 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i + 0 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i + 1 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i - 1 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i + 0 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i + 1 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i - 1 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i + 0 ] ) / 255.0f,
        Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i + 1 ] ) / 255.0f,
      };
      f32 val = 0;
      For( k, 0, _countof( coeffs ) ) {
        val += coeffs[k] * vals[k];
      }
      dst = _vec4( CLAMP( val, 0, 1 ) );
    }
  }

  auto img_output_len = dim.x * dim.y;
  auto img_output = MemHeapAlloc( u32, img_output_len );
  Fori( s32, i, 0, img_output_len ) {
    auto& src = img_blend[i];
    auto& dst = img_output[i];
    dst =
      ( Cast( u8, src.x * 255.0f + 0.0001f ) <<  0 ) |
      ( Cast( u8, src.y * 255.0f + 0.0001f ) <<  8 ) |
      ( Cast( u8, src.z * 255.0f + 0.0001f ) << 16 ) |
      ( Cast( u8, src.w * 255.0f + 0.0001f ) << 24 );
  }
  dimf = _vec2( Cast( f32, dim.x ), Cast( f32, dim.y ) );
  offsetf = _vec2( Cast( f32, offset.x ), Cast( f32, offset.y ) );
  MemHeapFree( img_blend );
  stbtt_FreeBitmap( img_alpha, 0 );
  return img_output;
#elif GLYGPHGEN == 3
  vec2<s32> dim, offset;
  auto img_alpha = stbtt_GetCodepointBitmap(
    &font.info,
    font.scale,
    font.scale,
    codept,
    &dim.x,
    &dim.y,
    &offset.x,
    &offset.y
    );

  auto dim_blend = _vec2( dim.x, dim.y );
  auto img_blend_len = dim_blend.x * dim_blend.y;
  auto img_blend = MemHeapAlloc( vec4<f32>, img_blend_len );
  Memzero( img_blend, img_blend_len * sizeof( vec4<f32> ) );

  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim.x ) {
      auto& dst = img_blend[ j * dim_blend.x + i ];
      static const f32 coeffs[] = {
         0, -1,  0,
        -1,  5, -1,
         0, -1,  0,
      };
      f32 vals[] = {
        ( j == 0  ||  i == 0 )                  ?  0.0f  :  Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i - 1 ] ) / 255.0f,
        ( j == 0 )                              ?  0.0f  :  Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i + 0 ] ) / 255.0f,
        ( j == 0  ||  i == dim.x - 1 )          ?  0.0f  :  Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i + 1 ] ) / 255.0f,
        ( i == 0 )                              ?  0.0f  :  Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i - 1 ] ) / 255.0f,
                                                            Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i + 0 ] ) / 255.0f,
        ( i == dim.x - 1 )                      ?  0.0f  :  Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i + 1 ] ) / 255.0f,
        ( j == dim.y - 1  ||  i == 0 )          ?  0.0f  :  Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i - 1 ] ) / 255.0f,
        ( j == dim.y - 1 )                      ?  0.0f  :  Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i + 0 ] ) / 255.0f,
        ( j == dim.y - 1  ||  i == dim.x - 1 )  ?  0.0f  :  Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i + 1 ] ) / 255.0f,
      };
      f32 val = 0;
      For( k, 0, _countof( coeffs ) ) {
        val += coeffs[k] * vals[k];
      }
      dst = _vec4( CLAMP( val, 0, 1 ) );
    }
  }

  auto img_output_len = dim.x * dim.y;
  auto img_output = MemHeapAlloc( u32, img_output_len );
  Fori( s32, i, 0, img_output_len ) {
    auto& src = img_blend[i];
    auto& dst = img_output[i];
    dst =
      ( Cast( u8, src.x * 255.0f + 0.0001f ) <<  0 ) |
      ( Cast( u8, src.y * 255.0f + 0.0001f ) <<  8 ) |
      ( Cast( u8, src.z * 255.0f + 0.0001f ) << 16 ) |
      ( Cast( u8, src.w * 255.0f + 0.0001f ) << 24 );
  }
  dimf = _vec2( Cast( f32, dim.x ), Cast( f32, dim.y ) );
  offsetf = _vec2( Cast( f32, offset.x ), Cast( f32, offset.y ) );
  MemHeapFree( img_blend );
  stbtt_FreeBitmap( img_alpha, 0 );
  return img_output;
#elif GLYGPHGEN == 4
  vec2<s32> dim, offset;
  auto img_alpha = stbtt_GetCodepointBitmap(
    &font.info,
    font.scale,
    font.scale,
    codept,
    &dim.x,
    &dim.y,
    &offset.x,
    &offset.y
    );

  auto dim_blend = _vec2( dim.x, dim.y );
  auto img_blend_len = dim_blend.x * dim_blend.y;
  auto img_blend = MemHeapAlloc( vec4<f32>, img_blend_len );
  Memzero( img_blend, img_blend_len * sizeof( vec4<f32> ) );

  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim.x ) {
      auto& dst = img_blend[ j * dim_blend.x + i ];
      static const f32 coeffs[] = {
        1 / 16.0f, 2 / 16.0f, 1 / 16.0f,
        2 / 16.0f, 4 / 16.0f, 2 / 16.0f,
        1 / 16.0f, 2 / 16.0f, 1 / 16.0f,
      };
      f32 vals[] = {
        ( j == 0  ||  i == 0 )                  ?  0.0f  :  Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i - 1 ] ) / 255.0f,
        ( j == 0 )                              ?  0.0f  :  Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i + 0 ] ) / 255.0f,
        ( j == 0  ||  i == dim.x - 1 )          ?  0.0f  :  Cast( f32, img_alpha[ ( j - 1 ) * dim.x + i + 1 ] ) / 255.0f,
        ( i == 0 )                              ?  0.0f  :  Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i - 1 ] ) / 255.0f,
                                                            Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i + 0 ] ) / 255.0f,
        ( i == dim.x - 1 )                      ?  0.0f  :  Cast( f32, img_alpha[ ( j + 0 ) * dim.x + i + 1 ] ) / 255.0f,
        ( j == dim.y - 1  ||  i == 0 )          ?  0.0f  :  Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i - 1 ] ) / 255.0f,
        ( j == dim.y - 1 )                      ?  0.0f  :  Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i + 0 ] ) / 255.0f,
        ( j == dim.y - 1  ||  i == dim.x - 1 )  ?  0.0f  :  Cast( f32, img_alpha[ ( j + 1 ) * dim.x + i + 1 ] ) / 255.0f,
      };
      f32 val = 0;
      For( k, 0, _countof( coeffs ) ) {
        val += coeffs[k] * vals[k];
      }
      static const f32 strength = 1.0f;
      val = vals[4] + strength * ( vals[4] - val );
      val = CLAMP( val, 0, 1 );
      dst = _vec4( val );
    }
  }

  auto img_output_len = dim.x * dim.y;
  auto img_output = MemHeapAlloc( u32, img_output_len );
  Fori( s32, i, 0, img_output_len ) {
    auto& src = img_blend[i];
    auto& dst = img_output[i];
    dst =
      ( Cast( u8, src.x * 255.0f + 0.0001f ) <<  0 ) |
      ( Cast( u8, src.y * 255.0f + 0.0001f ) <<  8 ) |
      ( Cast( u8, src.z * 255.0f + 0.0001f ) << 16 ) |
      ( Cast( u8, src.w * 255.0f + 0.0001f ) << 24 );
  }
  dimf = _vec2( Cast( f32, dim.x ), Cast( f32, dim.y ) );
  offsetf = _vec2( Cast( f32, offset.x ), Cast( f32, offset.y ) );
  MemHeapFree( img_blend );
  stbtt_FreeBitmap( img_alpha, 0 );
  return img_output;
#elif GLYGPHGEN == 5
  vec2<s32> dim_b, offset_b;
  auto img_b = stbtt_GetCodepointBitmapSubpixel(
    &font.info,
    font.scale,
    font.scale,
    0.0f / 3.0f,
    0.0f,
    codept,
    &dim_b.x,
    &dim_b.y,
    &offset_b.x,
    &offset_b.y
    );
  vec2<s32> dim_g, offset_g;
  auto img_g = stbtt_GetCodepointBitmapSubpixel(
    &font.info,
    font.scale,
    font.scale,
    1.0f / 6.0f,
    0.0f,
    codept,
    &dim_g.x,
    &dim_g.y,
    &offset_g.x,
    &offset_g.y
    );
  vec2<s32> dim_r, offset_r;
  auto img_r = stbtt_GetCodepointBitmapSubpixel(
    &font.info,
    font.scale,
    font.scale,
    1.0f / 3.0f,
    0.0f,
    codept,
    &dim_r.x,
    &dim_r.y,
    &offset_r.x,
    &offset_r.y
    );

  AssertWarn( dim_r.y == dim_g.y  &&  dim_g.y == dim_b.y );

  auto dim = Max( Max( dim_r, dim_g ), dim_b );

  // add space on left and right so we don't lose any info when blending.
  static const s32 spacing = 4;
  static const s32 half_spacing = 2;
  dim.x += spacing;

  // align everything to b, and move everything to the right 2px.
  auto offset = offset_b - _vec2<s32>( half_spacing, 0 );
  auto align_r = half_spacing + offset_r.x - offset_b.x;
  auto align_g = half_spacing + offset_g.x - offset_b.x;
  auto align_b = half_spacing + 0;

  auto img_blend_len = dim.x * dim.y;
  auto img_blend = MemHeapAlloc( vec4<f32>, img_blend_len );
  Memzero( img_blend, img_blend_len * sizeof( vec4<f32> ) );

  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim_r.x ) {
      auto idx = i + align_r;
      if( 0 <= idx  &&  idx < dim.x ) {
        auto& dst = img_blend[ j * dim.x + idx ];
        auto val = Cast( f32, img_r[ j * dim_r.x + i ] ) / 255.0f;
        dst.x += val;
        dst.w += val / 3;
      }
    }
    Fori( s32, i, 0, dim_g.x ) {
      auto idx = i + align_g;
      if( 0 <= idx  &&  idx < dim.x ) {
        auto& dst = img_blend[ j * dim.x + idx ];
        auto val = Cast( f32, img_g[ j * dim_g.x + i ] ) / 255.0f;
        dst.y += val;
        dst.w += val / 3;
      }
    }
    Fori( s32, i, 0, dim_b.x ) {
      auto idx = i + align_b;
      if( 0 <= idx  &&  idx < dim.x ) {
        auto& dst = img_blend[ j * dim.x + idx ];
        auto val = Cast( f32, img_b[ j * dim_b.x + i ] ) / 255.0f;
        dst.z += val;
        dst.w += val / 3;
      }
    }
  }

#if 1
  auto img_tmp = MemHeapAlloc( vec4<f32>, dim.x * dim.y );
  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim.x ) {
      static const f32 coeff = 1 / 28.0f;
      static const f32 coeffs[] = {
        -1, -2, -1,
        -2, 40, -2,
        -1, -2, -1,
      };
      vec4<f32> vals[] = {
        ( j == 0  ||  i == 0 )                  ?  _vec4( 0.0f )  :  img_blend[ ( j - 1 ) * dim.x + i - 1 ],
        ( j == 0 )                              ?  _vec4( 0.0f )  :  img_blend[ ( j - 1 ) * dim.x + i + 0 ],
        ( j == 0  ||  i == dim.x - 1 )          ?  _vec4( 0.0f )  :  img_blend[ ( j - 1 ) * dim.x + i + 1 ],
        ( i == 0 )                              ?  _vec4( 0.0f )  :  img_blend[ ( j + 0 ) * dim.x + i - 1 ],
                                                                     img_blend[ ( j + 0 ) * dim.x + i + 0 ],
        ( i == dim.x - 1 )                      ?  _vec4( 0.0f )  :  img_blend[ ( j + 0 ) * dim.x + i + 1 ],
        ( j == dim.y - 1  ||  i == 0 )          ?  _vec4( 0.0f )  :  img_blend[ ( j + 1 ) * dim.x + i - 1 ],
        ( j == dim.y - 1 )                      ?  _vec4( 0.0f )  :  img_blend[ ( j + 1 ) * dim.x + i + 0 ],
        ( j == dim.y - 1  ||  i == dim.x - 1 )  ?  _vec4( 0.0f )  :  img_blend[ ( j + 1 ) * dim.x + i + 1 ],
      };
      vec4<f32> val = {};
      For( k, 0, _countof( coeffs ) ) {
        val += _vec4( coeff * coeffs[k] ) * vals[k];
      }
      auto& dst = img_tmp[ j * dim.x + i ];
      dst = val;
    }
  }

  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim.x ) {
      auto& src = img_tmp[ j * dim.x + i ];
      auto& dst = img_blend[ j * dim.x + i ];
      dst = src;
    }
  }
  MemHeapFree( img_tmp );
#endif

#if 0
  ReverseFori( s32, j, 0, dim.y - 1 ) {
    ReverseFori( s32, i, 0, dim.x - 1 ) {
      auto& src00 = img_blend[ ( j + 0 ) * dim.x + i + 0 ];
      auto& src10 = img_blend[ ( j + 0 ) * dim.x + i + 1 ];
      auto& src01 = img_blend[ ( j + 1 ) * dim.x + i + 0 ];
      auto& src11 = img_blend[ ( j + 1 ) * dim.x + i + 1 ];

      bool close =
        ( ( src00.w - src10.w ) < 0.1f )  &
        ( ( src00.w - src01.w ) < 0.1f )  &
        ( ( src00.w - src11.w ) < 0.2f );

      if( close  &&  0.5f < src00.w  &&  src00.w < 0.75f ) {
        src00 /= src00.w;
        //src10 *= 1.0f;
        //src01 *= 1.0f;
        //src11 *= 1.0f;
      }
    }
  }
#endif

  auto img_output_len = dim.x * dim.y;
  auto img_output = MemHeapAlloc( u32, img_output_len );
  Fori( s32, j, 0, dim.y ) {
    Fori( s32, i, 0, dim.x ) {
      static const f32 coeffs[] = {
        1 / 16.0f, 2 / 16.0f, 1 / 16.0f,
        2 / 16.0f, 4 / 16.0f, 2 / 16.0f,
        1 / 16.0f, 2 / 16.0f, 1 / 16.0f,
      };
      vec4<f32> vals[] = {
        ( j == 0  ||  i == 0 )                  ?  _vec4( 0.0f )  :  img_blend[ ( j - 1 ) * dim.x + i - 1 ],
        ( j == 0 )                              ?  _vec4( 0.0f )  :  img_blend[ ( j - 1 ) * dim.x + i + 0 ],
        ( j == 0  ||  i == dim.x - 1 )          ?  _vec4( 0.0f )  :  img_blend[ ( j - 1 ) * dim.x + i + 1 ],
        ( i == 0 )                              ?  _vec4( 0.0f )  :  img_blend[ ( j + 0 ) * dim.x + i - 1 ],
                                                                     img_blend[ ( j + 0 ) * dim.x + i + 0 ],
        ( i == dim.x - 1 )                      ?  _vec4( 0.0f )  :  img_blend[ ( j + 0 ) * dim.x + i + 1 ],
        ( j == dim.y - 1  ||  i == 0 )          ?  _vec4( 0.0f )  :  img_blend[ ( j + 1 ) * dim.x + i - 1 ],
        ( j == dim.y - 1 )                      ?  _vec4( 0.0f )  :  img_blend[ ( j + 1 ) * dim.x + i + 0 ],
        ( j == dim.y - 1  ||  i == dim.x - 1 )  ?  _vec4( 0.0f )  :  img_blend[ ( j + 1 ) * dim.x + i + 1 ],
      };
//      if( vals[4][3] > 0.5f ) {
//        vals[4] *= vals[4];
//      }

      vec4<f32> val = {};
      For( k, 0, _countof( coeffs ) ) {
        val += _vec4( coeffs[k] ) * vals[k];
      }

      static const f32 sharpen_strength = 1.0f;
      auto& src = img_blend[ j * dim.x + i ];
      val = src + sharpen_strength * ( src - val );

      static const f32 max_value = 1.0f;
      static const f32 rec_max_value = 1 / max_value;
      val = Clamp( val * rec_max_value, _vec4<f32>( 0 ), _vec4<f32>( 1 ) );

      auto& dst = img_output[ j * dim.x + i ];
      dst =
        ( Cast( u8, val.x * 255.0f + 0.0001f ) <<  0 ) |
        ( Cast( u8, val.y * 255.0f + 0.0001f ) <<  8 ) |
        ( Cast( u8, val.z * 255.0f + 0.0001f ) << 16 ) |
        ( Cast( u8, val.w * 255.0f + 0.0001f ) << 24 );
    }
  }

  dimf = _vec2( Cast( f32, dim.x ), Cast( f32, dim.y ) );
  offsetf = _vec2( Cast( f32, offset.x ), Cast( f32, offset.y ) );
  MemHeapFree( img_blend );
  stbtt_FreeBitmap( img_r, 0 );
  stbtt_FreeBitmap( img_g, 0 );
  stbtt_FreeBitmap( img_b, 0 );
  return img_output;
#else
#error GLYPHGEN
#endif

}


struct
fontglyphimg_t
{
  u32* img;
  vec2<s32> dim;
  vec2<f32> dimf;
};

Inl void
FontLoadAscii( font_t& font )
{
  ProfFunc();

  array_t<fontglyphimg_t> glyphimgs;
  Alloc( glyphimgs, 128 );

  // TODO: load extended chars that we want.

  //Fori( u32, codept, ' ', '~' + 1 ) {
  Fori( u32, codept, 0, 256 ) {
    auto glyph_idx = font.glyphs.len;

    fontglyph_t glyph = {};
    fontglyphimg_t glyphimg = {};
    glyphimg.img = FontLoadGlyphImage(
      font,
      codept,
      glyph.dim,
      glyph.offset
      );

    glyphimg.dimf = glyph.dim + _vec2<f32>( 1, 1 );
    glyphimg.dim = _vec2( Round_s32_from_f32( glyphimg.dimf.x ), Round_s32_from_f32( glyphimg.dimf.y ) );

    s32 advance;
    stbtt_GetCodepointHMetrics( &font.info, Cast( s32, codept ), &advance, 0 );
    glyph.advance = font.scale * advance;

    *AddBack( font.glyphs ) = glyph;
    *AddBack( glyphimgs ) = glyphimg;

    Reserve( font.glyph_from_codept, codept + 1 );
    font.glyph_from_codept.len = MAX( font.glyph_from_codept.len, codept + 1 );
    font.glyph_from_codept.mem[codept] = glyph_idx;
  }

  kahan32_t x = {};
  kahan32_t y = {};
  f32 delta_y = 0;

  static const auto maxdim = _vec2<f32>( 512, 4096 );

  // reserve space for 1px white.
  Add( x, 1.0f );
  delta_y = 1.0f;

  AssertCrash( font.glyphs.len == glyphimgs.len );
  ForLen( i, font.glyphs ) {
    auto glyph = font.glyphs.mem + i;
    auto glyphimg = glyphimgs.mem + i;

    AssertCrash( y.sum + glyphimg->dimf.y <= maxdim.y ); // out of room!

    if( x.sum + glyphimg->dimf.x > maxdim.x ) {
      Add( y, delta_y );
      y.sum = Ceil32( y.sum );
      y.err = 0;
      delta_y = 0;
      x = {};
      --i; // retry this glyph.
    } else {
      delta_y = MAX( delta_y, glyphimg->dimf.y );
      glyph->tc0 = _vec2( x.sum, y.sum );
      glyph->tc1 = glyph->tc0 + glyph->dim;
      Add( x, glyphimg->dimf.x );
      x.sum = Ceil32( x.sum );
      x.err = 0;
    }
  }
  Add( y, delta_y );
  y.sum = Ceil32( y.sum );
  y.err = 0;

  auto dimf = _vec2( maxdim.x, y.sum );
  auto dim = _vec2( Round_s32_from_f32( dimf.x ), Round_s32_from_f32( dimf.y ) );
  auto img = MemHeapAlloc( u32, dim.x * dim.y );

  // write first px white.
  *img = MAX_u32;

  AssertCrash( font.glyphs.len == glyphimgs.len );
  ForLen( i, font.glyphs ) {
    auto glyph = font.glyphs.mem + i;
    auto glyphimg = glyphimgs.mem + i;

    auto p0 = _vec2( Round_s32_from_f32( glyph->tc0.x ), Round_s32_from_f32( glyph->tc0.y ) );
    auto glyphdim = _vec2( Round_s32_from_f32( glyph->dim.x ), Round_s32_from_f32( glyph->dim.y ) );
    Fori( s32, j, 0, glyphdim.y ) {
      Fori( s32, k, 0, glyphdim.x ) {
        auto dst = img + ( p0.y + j ) * dim.x + ( p0.x + k );
        auto src = glyphimg->img + j * glyphdim.x + k;
        *dst = *src;
      }
    }

#if OPENGL_INSTEAD_OF_SOFTWARE
    glyph->tc0 /= dimf;
    glyph->tc1 /= dimf;
#else // !OPENGL_INSTEAD_OF_SOFTWARE
    // leave tc's as absolute integers, to save some computation later.
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
  }

  ForLen( i, glyphimgs ) {
    auto glyphimg = glyphimgs.mem + i;
    MemHeapFree( glyphimg->img );
  }
  Free( glyphimgs );

#if OPENGL_INSTEAD_OF_SOFTWARE
  // TODO: consider switching the opengl here to use the GlwUploadTexture path.

  glDeleteTextures( 1, &font.texid );
  glGenTextures( 1, &font.texid );
  glBindTexture( GL_TEXTURE_2D, font.texid );  glVerify();
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    dim.x,
    dim.y,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    img
    );
  // NOTE: we use nearest filtering, since it looks a tiny bit better if we get the 1:1 pixel drawing wrong.
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );  glVerify();
  glGenerateMipmap( GL_TEXTURE_2D );  glVerify();

  glBindTexture( GL_TEXTURE_2D, 0 );  glVerify();

  MemHeapFree( img );
#else // !OPENGL_INSTEAD_OF_SOFTWARE

  font.tex_dim = _vec2( Cast( u32, dim.x ), Cast( u32, dim.y ) );
  font.tex_mem = img;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
}


#define FontGetAdvance( font, codept ) \
  ( FontGetGlyph( font, codept )->advance )


Inl f32
FontGetKerning( font_t& font, u32 codept, u32 codept2 )
{
  // PERF: fixed-size cache, some replacement policy
  auto advance = stbtt_GetCodepointKernAdvance( &font.info, Cast( s32, codept ), Cast( s32, codept2 ) );
  return font.scale * advance;
}



void
FontAddLayoutLine(
  font_t& font,
  fontlayout_t& layout,
  u8* text,
  idx_t text_len,
  u32 spaces_per_tab
  )
{
  ProfFunc();

  auto linespan = AddBack( layout.advances_per_ln );
  linespan->pos = layout.raw_advances.len;
  linespan->len = 0;
  kahan32_t x = {};
  For( i, 0, text_len ) {
    u32 codept = text[i];
    if( codept == '\t' ) {
      auto adv = AddBack( layout.raw_advances );
      adv->codept = ' ';
      adv->glyph = FontGetGlyph( font, adv->codept );
      adv->absolute_x = x.sum;
      Add( x, spaces_per_tab * adv->glyph->advance );
      linespan->len += 1;
    } else {
      auto adv = AddBack( layout.raw_advances );
      adv->codept = codept;
      adv->glyph = FontGetGlyph( font, adv->codept );
      adv->absolute_x = x.sum;
      if( i + 1 < text_len ) {
        u32 codept2 = text[i + 1];
        Add( x, adv->glyph->advance + FontGetKerning( font, codept, codept2 ) );
      } else {
        Add( x, adv->glyph->advance );
      }
      linespan->len += 1;
    }
  }
  linespan->width = x.sum;
}

Inl f32
FontSumAdvances(
  fontlayout_t& layout,
  idx_t line,
  idx_t char_offset,
  idx_t char_len
  )
{
  ProfFunc();

  if( !char_len ) {
    return 0.0f;
  }

  AssertCrash( line < layout.advances_per_ln.len );
  auto linespan = layout.advances_per_ln.mem + line;
  auto width = linespan->width;
  AssertCrash( char_offset + char_len <= linespan->len ); // addressing out of bounds from what you laid out previously!
  AssertCrash( linespan->pos + char_offset < layout.raw_advances.len );
  auto first = layout.raw_advances.mem + linespan->pos + char_offset;
  width -= first->absolute_x;
  if( char_offset + char_len < linespan->len ) {
    auto last = first + char_len;
    width -= ( linespan->width - last->absolute_x );
  }
  return width;
}


#if OPENGL_INSTEAD_OF_SOFTWARE

  struct
  shader_tex2_t
  {
    glwshader_t core;
    s32 loc_ndc_from_client;
    s32 loc_tex_sampler;
    s32 attribloc_pos;
    s32 attribloc_tccolor;
  };

  void
  ShaderInit( shader_tex2_t& shader )
  {
    shader.core = GlwLoadShader(
      R"STRING(
        #version 330 core
        layout( location = 0 ) in vec3 vertex_pos;
        layout( location = 1 ) in vec3 vertex_tccolor;
        out vec3 tccolor;
        uniform mat4 ndc_from_client;
        void main() {
          gl_Position = ndc_from_client * vec4( vertex_pos, 1 );
          tccolor = vertex_tccolor;
        }
      )STRING",

  #if 1
      R"STRING(
        #version 330 core
        in vec3 tccolor;
        layout( location = 0 ) out vec4 pixel;
        uniform sampler2D tex_sampler;
        void main() {
          vec2 tc = tccolor.xy;
          uint colorbits = uint( tccolor.z );
          vec4 color;
          color.x = ( ( colorbits >>  0u ) & 63u ) / 63.0f;
          color.y = ( ( colorbits >>  6u ) & 63u ) / 63.0f;
          color.z = ( ( colorbits >> 12u ) & 63u ) / 63.0f;
          color.w = ( ( colorbits >> 18u ) & 31u ) / 31.0f;
          vec4 texel = texture( tex_sampler, tc );
          pixel = color * texel;
        }
      )STRING"
  #else
      R"STRING(
        #version 330 core
        in vec3 tccolor;
        layout( location = 0 ) out vec4 pixel;
        uniform sampler2D tex_sampler;
        void main() {
          vec2 tc = tccolor.xy;
          uint colorbits = floatBitsToUint( tccolor.z );
          vec4 color;
          color.x = ( ( colorbits >>  0u ) & 0xFFu ) / 255.0f;
          color.y = ( ( colorbits >>  8u ) & 0xFFu ) / 255.0f;
          color.z = ( ( colorbits >> 16u ) & 0xFFu ) / 255.0f;
          color.w = ( ( colorbits >> 24u ) & 0xFFu ) / 255.0f;
          vec4 texel = texture( tex_sampler, tc );
          pixel = color * texel;
        }
      )STRING"
  #endif
      );


    // uniforms
    shader.loc_ndc_from_client = glGetUniformLocation( shader.core.program, "ndc_from_client" );  glVerify();
    shader.loc_tex_sampler = glGetUniformLocation( shader.core.program, "tex_sampler" );  glVerify();

    // attribs
    shader.attribloc_pos = glGetAttribLocation( shader.core.program, "vertex_pos" );  glVerify();
    shader.attribloc_tccolor = glGetAttribLocation( shader.core.program, "vertex_tccolor" );  glVerify();
  }

  void
  ShaderKill( shader_tex2_t& shader )
  {
    GlwUnloadShader( shader.core );
    shader.loc_ndc_from_client = -1;
    shader.loc_tex_sampler = -1;
    shader.attribloc_pos = -1;
    shader.attribloc_tccolor = -1;
  }

#endif // OPENGL_INSTEAD_OF_SOFTWARE


Inl f32
PackColorForShader( vec4<f32> color )
{
#if 1
  u32 res =
    ( ( Round_u32_from_f32( color.x * 63.0f ) & 63u ) <<  0u ) |
    ( ( Round_u32_from_f32( color.y * 63.0f ) & 63u ) <<  6u ) |
    ( ( Round_u32_from_f32( color.z * 63.0f ) & 63u ) << 12u ) |
    ( ( Round_u32_from_f32( color.w * 31.0f ) & 31u ) << 18u );
  return Cast( f32, res );
#else
  u32 res =
    ( ( Round_u32_from_f32( color.x * 255.0f ) & 0xFFu ) <<  0 ) |
    ( ( Round_u32_from_f32( color.y * 255.0f ) & 0xFFu ) <<  8 ) |
    ( ( Round_u32_from_f32( color.z * 255.0f ) & 0xFFu ) << 16 ) |
    ( ( Round_u32_from_f32( color.w * 255.0f ) & 0xFFu ) << 24 );
  return Reinterpret( f32, res );
#endif
}

Inl vec4<f32>
UnpackColorForShader( f32 colorf )
{
#if 1
  auto colorbits = Cast( u32, colorf );
  auto res = _vec4(
    ( ( colorbits >>  0u ) & 63u ) / 63.0f,
    ( ( colorbits >>  6u ) & 63u ) / 63.0f,
    ( ( colorbits >> 12u ) & 63u ) / 63.0f,
    ( ( colorbits >> 18u ) & 31u ) / 31.0f
    );
  return res;
#else
  auto colorbits = Reinterpret( u32, colorf );
  auto res = _vec4(
    ( ( colorbits >>  0u ) & 0xFFu ) / 255.0f,
    ( ( colorbits >>  8u ) & 0xFFu ) / 255.0f,
    ( ( colorbits >> 16u ) & 0xFFu ) / 255.0f,
    ( ( colorbits >> 24u ) & 0xFFu ) / 255.0f
    );
  return res;
#endif
}



Inl void
OutputQuad(
  array_t<f32>& stream,
  vec2<f32> p0,
  vec2<f32> p1,
  f32 z,
  vec2<f32> tc0,
  vec2<f32> tc1,
  vec4<f32> color
  )
{
#if OPENGL_INSTEAD_OF_SOFTWARE
  auto pos = AddBack( stream, 6*3 + 6*3 );
  auto colorf = PackColorForShader( color );
  //auto test = UnpackColorForShader( colorf );
  *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p0.y, z );         pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( tc0.x, tc0.y, colorf );  pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p0.y, z );         pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( tc1.x, tc0.y, colorf );  pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p1.y, z );         pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( tc0.x, tc1.y, colorf );  pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p1.y, z );         pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( tc1.x, tc1.y, colorf );  pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p1.y, z );         pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( tc0.x, tc1.y, colorf );  pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p0.y, z );         pos += 3;
  *Cast( vec3<f32>*, pos ) = _vec3( tc1.x, tc0.y, colorf );  pos += 3;
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  auto pos = AddBack( stream, 4*2 + 2 );
  auto colorf = PackColorForShader( color );
  //auto test = UnpackColorForShader( colorf );
  *Cast( vec2<f32>*, pos ) = p0;         pos += 2;
  *Cast( vec2<f32>*, pos ) = p1;         pos += 2;
  *Cast( vec2<f32>*, pos ) = tc0;        pos += 2;
  *Cast( vec2<f32>*, pos ) = tc1;        pos += 2;
  *pos++ = z;
  *pos++ = colorf;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
}

Inl bool
ClipQuadTex(
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec2<f32>& tc0,
  vec2<f32>& tc1,
  vec2<f32>& clip_pos,
  vec2<f32>& clip_dim
  )
{
  auto s_from_pix = ( tc1.x - tc0.x ) / ( p1.x - p0.x );
  auto t_from_pix = ( tc1.y - tc0.y ) / ( p1.y - p0.y );
  auto dx0 = ( p0.x - clip_pos.x );
  auto dx1 = ( p1.x - ( clip_pos.x + clip_dim.x ) );
  auto dy0 = ( p0.y - clip_pos.y );
  auto dy1 = ( p1.y - ( clip_pos.y + clip_dim.y ) );
  if( dx0 < 0 ) {
    tc0.x -= dx0 * s_from_pix;
    p0.x = clip_pos.x;
  }
  if( dx1 > 0 ) {
    tc1.x -= dx1 * s_from_pix;
    p1.x = clip_pos.x + clip_dim.x;
  }
  if( dy0 < 0 ) {
    tc0.y -= dy0 * t_from_pix;
    p0.y = clip_pos.y;
  }
  if( dy1 > 0 ) {
    tc1.y -= dy1 * t_from_pix;
    p1.y = clip_pos.y + clip_dim.y;
  }
  bool draw =
    ( p0.x < p1.x )  &
    ( p0.y < p1.y );
  return draw;
}

Inl bool
ClipQuad(
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec2<f32>& clip_pos,
  vec2<f32>& clip_dim
  )
{
  auto dx0 = ( p0.x - clip_pos.x );
  auto dx1 = ( p1.x - ( clip_pos.x + clip_dim.x ) );
  auto dy0 = ( p0.y - clip_pos.y );
  auto dy1 = ( p1.y - ( clip_pos.y + clip_dim.y ) );
  if( dx0 < 0 ) {
    p0.x = clip_pos.x;
  }
  if( dx1 > 0 ) {
    p1.x = clip_pos.x + clip_dim.x;
  }
  if( dy0 < 0 ) {
    p0.y = clip_pos.y;
  }
  if( dy1 > 0 ) {
    p1.y = clip_pos.y + clip_dim.y;
  }

  bool draw =
    ( p0.x < p1.x )  &
    ( p0.y < p1.y );
  return draw;
}

Inl void
DrawLine(
  )
{
#if 0
  // clip first, since clipping an oriented quad is lots of code.
  // we'll accept a little clipping error, up to half of linewidth.

  vec2<f32> delta = line.p1 - line.p0;
  vec2<f32> half_edge = 0.5f * line.width * Normalize( Perp( delta ) );

  vec2<f32> p0, p1;

  auto t000 =  ( line.clip_pos.x                   - line.p0.x ) / delta.x;
  auto t010 =  ( line.clip_pos.x + line.clip_dim.x - line.p0.x ) / delta.x;
  auto t100 = -( line.clip_pos.x                   - line.p1.x ) / delta.x;
  auto t110 = -( line.clip_pos.x + line.clip_dim.x - line.p1.x ) / delta.x;

  auto t001 =  ( line.clip_pos.y                   - line.p0.y ) / delta.y;
  auto t011 =  ( line.clip_pos.y + line.clip_dim.y - line.p0.y ) / delta.y;
  auto t101 = -( line.clip_pos.y                   - line.p1.y ) / delta.y;
  auto t111 = -( line.clip_pos.y + line.clip_dim.y - line.p1.y ) / delta.y;

  auto p00 = line.p0 - half_edge;
  auto p10 = line.p1 - half_edge;
  auto p01 = line.p0 + half_edge;
  auto p11 = line.p1 + half_edge;

  bool draw =
    ( line.p0.x != line.p1.x )  |
    ( line.p0.y != line.p1.y );
  if( draw ) {
    // TODO: implement
    //ImplementCrash();
    //glBindTexture( GL_TEXTURE_2D, 0 );
    //glColor4fv( &line.color.x );
    //glLineWidth( line.width );
    //glBegin( GL_LINES );
    //glVertex3f( line.p0.x, line.p0.y, line.z );
    //glVertex3f( line.p1.x, line.p1.y, line.z );
    //glEnd();
    //glLineWidth( 1 );
  }
#endif
}

Inl void
RenderLine(
  array_t<f32>& stream,
  vec4<f32> color,
  vec2<f32> p0,
  vec2<f32> p1,
  vec2<f32>& clip_pos,
  vec2<f32>& clip_dim,
  f32 z,
  f32 width
  )
{
  // TODO: implement
  ImplementCrash();
}

Inl void
RenderTri(
  array_t<f32>& stream,
  vec4<f32>& color,
  vec2<f32> p0,
  vec2<f32> p1,
  vec2<f32> p2,
  vec2<f32>& clip_pos,
  vec2<f32>& clip_dim,
  f32 z
  )
{
  AssertWarn( 0.0f <= z  &&  z <= 1.0f );

#if OPENGL_INSTEAD_OF_SOFTWARE
  // TODO: clipping.
  bool draw = 1;
  if( draw ) {
    auto pos = AddBack( stream, 3*3 + 3*3 );
    auto colorf = PackColorForShader( color );
    //auto test = UnpackColorForShader( colorf );
    *Cast( vec3<f32>*, pos ) = _vec3( p0.x, p0.y, z );      pos += 3;
    *Cast( vec3<f32>*, pos ) = _vec3<f32>( 0, 0, colorf );  pos += 3;
    *Cast( vec3<f32>*, pos ) = _vec3( p1.x, p1.y, z );      pos += 3;
    *Cast( vec3<f32>*, pos ) = _vec3<f32>( 0, 0, colorf );  pos += 3;
    *Cast( vec3<f32>*, pos ) = _vec3( p2.x, p2.y, z );      pos += 3;
    *Cast( vec3<f32>*, pos ) = _vec3<f32>( 0, 0, colorf );  pos += 3;
  }
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  // TODO_SOFTWARE_RENDER
#endif // !OPENGL_INSTEAD_OF_SOFTWARE
}

Inl void
RenderQuad(
  array_t<f32>& stream,
  vec4<f32>& color,
  vec2<f32> p0,
  vec2<f32> p1,
  vec2<f32>& clip_pos,
  vec2<f32>& clip_dim,
  f32 z
  )
{
  AssertWarn( 0.0f <= z  &&  z <= 1.0f );

  bool draw = ClipQuad(
    p0,
    p1,
    clip_pos,
    clip_dim
    );
  if( draw ) {
    OutputQuad(
      stream,
      p0,
      p1,
      z,
      _vec2<f32>( 0, 0 ),
      _vec2<f32>( 0, 0 ),
      color
      );
  }
}

Inl void
RenderTQuad(
  array_t<f32>& stream,
  vec4<f32>& color,
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec2<f32> tc0,
  vec2<f32> tc1,
  vec2<f32>& clip_pos,
  vec2<f32>& clip_dim,
  f32 z
  )
{
  AssertWarn( 0.0f <= z  &&  z <= 1.0f );

  bool draw = ClipQuadTex(
    p0,
    p1,
    tc0,
    tc1,
    clip_pos,
    clip_dim
    );

  if( draw ) {
    OutputQuad(
      stream,
      p0,
      p1,
      z,
      tc0,
      tc1,
      color
      );
  }
}

Inl void
RenderCodept(
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> pos,
  f32 z,
  vec2<f32> clip_pos,
  vec2<f32> clip_dim,
  vec4<f32> color,
  fontglyph_t* glyph
  )
{
  AssertWarn( 0.0f <= z  &&  z <= 1.0f );

  // add ascent so we're positioning the top-right of the textbox.
  // add linegap so the text is aligned to the bottom of the textbox.
  auto p0 = pos + glyph->offset + _vec2( 0.0f, font.ascent + font.linegap );

  // snap to pixels, so it doesn't look too bad when moving.
  p0 = Ceil( p0 );

  auto p1 = p0 + glyph->dim;

  RenderTQuad(
    stream,
    color,
    p0, p1,
    glyph->tc0, glyph->tc1,
    clip_pos, clip_dim,
    z
    );
}


f32
LayoutString(
  font_t& font,
  idx_t spaces_per_tab,
  u8* text,
  idx_t text_len
  )
{
  ProfFunc();
  f32 width = 0;
  For( i, 0, text_len ) {
    u8 c = *text++;
    if ( c == '\t' ) {
      width += spaces_per_tab * FontGetAdvance( font, c );
    } else {
      // TODO: draw unsupported chars as the box char, like browsers do.
      // for now, draw non-ASCII as ~
      if( ( c < ' ' ) | ( c > '~' ) ) {
        c = '~';
      }
      width += FontGetAdvance( font, c );
    }
  }
  return width;
}

void
DrawString(
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> pos,
  f32 z,
  vec2<f32> clip_pos,
  vec2<f32> clip_dim,
  vec4<f32> color,
  idx_t spaces_per_tab,
  u8* text,
  idx_t text_len
  )
{
  ProfFunc();
  AssertWarn( 0.0f <= z  &&  z <= 1.0f );

  auto glyph_space = FontGetGlyph( font, ' ' );

  For( i, 0, text_len ) {
    u8 c = *text++;
    // draw spaces for tabs.
    if ( c == '\t' ) {
      For( j, 0, spaces_per_tab ) {
        RenderCodept(
          stream,
          font,
          pos,
          z,
          clip_pos,
          clip_dim,
          color,
          glyph_space
          );
        pos.x += glyph_space->advance;
      }
    } else {
      // TODO: draw unsupported chars as the box char, like browsers do.
      // for now, draw non-ASCII as ~
      if( ( c < ' ' ) | ( c > '~' ) ) {
        c = '~';
      }
      auto glyph = FontGetGlyph( font, c );
      RenderCodept(
        stream,
        font,
        pos,
        z,
        clip_pos,
        clip_dim,
        color,
        glyph
        );
      pos.x += glyph->advance;
    }
  }
}


void
RenderText(
  array_t<f32>& stream,
  font_t& font,
  fontlayout_t& layout,
  vec4<f32> color,
  vec2<f32> clip_pos,
  vec2<f32> clip_dim,
  vec2<f32> pos,
  f32 z,
  idx_t line,
  idx_t char_offset,
  idx_t char_len
  )
{
  ProfFunc();
  AssertWarn( 0.0f <= z  &&  z <= 1.0f );

  auto ox = pos.x;
  AssertCrash( line < layout.advances_per_ln.len );
  auto linespan = layout.advances_per_ln.mem + line;
  AssertCrash( char_offset + char_len <= linespan->len );
  For( i, 0, char_len ) {
    AssertCrash( linespan->pos + char_offset + i < layout.raw_advances.len );
    auto advance = layout.raw_advances.mem + linespan->pos + char_offset + i;
    pos.x = ox + advance->absolute_x;
    RenderCodept(
      stream,
      font,
      pos,
      z,
      clip_pos,
      clip_dim,
      color,
      advance->glyph
      );
  }
}

Templ Inl vec2<f32>
ZRange(
  vec2<f32> zrange,
  T layer
  )
{
  auto v = _vec2(
    ( Cast( f32, layer ) + 0 ) / Cast( f32, T::COUNT ),
    ( Cast( f32, layer ) + 1 ) / Cast( f32, T::COUNT )
    );
  return v * zrange;
}

Templ Inl f32
GetZ(
  vec2<f32> zrange,
  T layer
  )
{
  auto s = Cast( f32, layer ) / Cast( f32, T::COUNT );
  return lerp( zrange.x, zrange.y, s );
}



// this is the equivalent of an idx_t offset for a contiguous array.
// the complexity is here to support arbitrary-sized paging, which we rely on for quick insert/delete.
//
// we used to have a public idx_t offset, and then internally cache transformations to this kind of page ptr.
// that was getting kind of slow for the content-dependent cursor move functions ( e.g. CursorLineD ).
// so we switched to this, and are still feeling it out.
// it's definitely less convenient / more complicated, since you now have to pass in concurrent ptrs.
// i'm still on the fence about this, whether it's better or not.
//
struct
content_ptr_t
{
  idx_t diff_idx;
  idx_t offset_into_diff;
};

Inl bool
Equal( content_ptr_t a, content_ptr_t b )
{
  bool r =
    ( a.diff_idx == b.diff_idx )  &&
    ( a.offset_into_diff == b.offset_into_diff );
  return r;
}

Inl bool
LEqual( content_ptr_t a, content_ptr_t b )
{
  bool r =
    ( a.diff_idx < b.diff_idx ) ||
    ( a.diff_idx == b.diff_idx  &&  a.offset_into_diff <= b.offset_into_diff );
  return r;
}

Inl bool
GEqual( content_ptr_t a, content_ptr_t b )
{
  // a >= b
  // b <= a
  return LEqual( b, a );
}

Inl bool
Less( content_ptr_t a, content_ptr_t b )
{
  // a < b
  // !( a >= b )
  // !( b <= a )
  return !LEqual( b, a );
}

Inl bool
Greater( content_ptr_t a, content_ptr_t b )
{
  // a > b
  // !( a <= b )
  return !LEqual( a, b );
}

Inl content_ptr_t
Max( content_ptr_t a, content_ptr_t b )
{
  if( a.diff_idx == b.diff_idx ) {
    a.offset_into_diff = MAX( a.offset_into_diff, b.offset_into_diff );
    return a;
  } elif( a.diff_idx < b.diff_idx ) {
    return b;
  } else {
    return a;
  }
}

Inl content_ptr_t
Max3( content_ptr_t a, content_ptr_t b, content_ptr_t c )
{
  return Max( Max( a, b ), c );
}

Inl content_ptr_t
Min( content_ptr_t a, content_ptr_t b )
{
  if( a.diff_idx == b.diff_idx ) {
    a.offset_into_diff = MIN( a.offset_into_diff, b.offset_into_diff );
    return a;
  } elif( a.diff_idx < b.diff_idx ) {
    return a;
  } else {
    return b;
  }
}

Inl content_ptr_t
Min3( content_ptr_t a, content_ptr_t b, content_ptr_t c )
{
  return Min( Min( a, b ), c );
}




struct
diff_t
{
  slice_t slice;

#if LNCACHE
  // we maintain all the beginning-of-line positions, at all times.
  // this is a sorted list, so getting the i'th element corresponds to the i'th sequential line.
  // we'll update these during insert/delete/etc. operations.
  // PERF: use pagearray_t / skiplist_t if this is too slow!
  // PERF: u32 ?
  array_t<idx_t> ln_starts;
#endif
};

Inl void
Kill( diff_t& diff )
{
  diff.slice = {};
#if LNCACHE
  Free( diff.ln_starts );
#endif
}

Inl bool
Equal( diff_t& a, diff_t& b )
{
  bool r =
    ( a.slice.mem == b.slice.mem )  &&
    ( a.slice.len == b.slice.len );
  return r;
}

Inl void
_AllocAndFillLnStarts( diff_t& diff )
{
#if LNCACHE
  idx_t count = 0;
  For( j, 0, diff.slice.len ) {
    if( diff.slice.mem[j] == '\n' ) {
      count += 1;
    }
  }
  Alloc( diff.ln_starts, MAX( 8, count ) );
  For( j, 0, diff.slice.len ) {
    if( diff.slice.mem[j] == '\n' ) {
      *AddBack( diff.ln_starts ) = j;
    }
  }
#endif
}



Enumc( undoableopertype_t )
{
  checkpt,
  add, // new diff stored in diff_new
  mod, // change stored in diff_old -> diff_new
  rem, // old diff stored in diff_old
};

struct
undoableoper_t
{
  undoableopertype_t type;
  diff_t diff_old;
  diff_t diff_new;
  idx_t idx; // in buf_t.diffs
};


Enumc( loadstate_t )
{
  none,
  file,
  emptyfile,
};


struct
buf_t
{
  array_t<diff_t> diffs;
  plist_t plist;
  idx_t content_len;
  array_t<undoableoper_t> history;
  idx_t history_idx;

  // external users of the buf_t can attach their persistent content_ptrs here.
  // we'll update these content_ptrs during insert/delete/etc. operations.
//  array_t<content_ptr_t> user_ptrs; // TODO: replace txt content tracking with this ?
};



// =================================================================================
// FIRST / LAST CALLS
//

Inl void
Zero( buf_t& buf )
{
  Zero( buf.plist );
  Zero( buf.diffs );
  buf.content_len = 0;
  Zero( buf.history );
  buf.history_idx = 0;
}

Inl void
Init( buf_t& buf )
{
  Zero( buf );
}

Inl void
Kill( buf_t& buf )
{
  ForLen( i, buf.diffs ) {
    auto diff = buf.diffs.mem + i;
    Kill( *diff );
  }
  Free( buf.diffs );
  Kill( buf.plist );
  Free( buf.history );

  Zero( buf );
}



// =================================================================================
// CONTENT READ CALLS
//

Inl content_ptr_t
GetBOF( buf_t& buf )
{
  content_ptr_t r;
  r.diff_idx = 0;
  r.offset_into_diff = 0;
  return r;
}

Inl content_ptr_t
GetEOF( buf_t& buf )
{
  content_ptr_t r;
  r.diff_idx = buf.diffs.len;
  r.offset_into_diff = 0;
  return r;
}



Inl bool
IsBOF( buf_t& buf, content_ptr_t pos )
{
  auto r = !pos.diff_idx  &&  !pos.offset_into_diff;
  return r;
}

Inl bool
IsEOF( buf_t& buf, content_ptr_t pos )
{
  auto r = pos.diff_idx >= buf.diffs.len;
  return r;
}



// returns the position past the end of what you read.
// since we're already iterating the diff list, may as well give that final position back.
Inl content_ptr_t
Contents(
  buf_t& buf,
  content_ptr_t start,
  u8* str,
  idx_t len
  )
{
  if( !buf.content_len || !len ) {
    return start;
  }

  AssertCrash( str );
  AssertCrash( start.diff_idx < buf.diffs.len );

  auto x = start;
  Forever {
    if( !len ) {
      break;
    }
    if( x.diff_idx >= buf.diffs.len ) {
      break;
    }
    auto diff = buf.diffs.mem + x.diff_idx;
    AssertCrash( x.offset_into_diff < diff->slice.len );
    auto rem = diff->slice.len - x.offset_into_diff;
    auto copy_done_with_this_diff = len < rem;
    auto ncopy = MIN( len, rem );
    Memmove( str, diff->slice.mem + x.offset_into_diff, ncopy );
    str += ncopy;
    len -= ncopy;

    if( !len  &&  copy_done_with_this_diff ) {
      x.offset_into_diff += ncopy;
      break;
    }

    x.diff_idx += 1;
    x.offset_into_diff = 0;
  }

  return x;
}

Inl idx_t
Contents(
  buf_t& buf,
  content_ptr_t start,
  content_ptr_t end,
  u8* str,
  idx_t len
  )
{
  if( !buf.content_len || !len ) {
    return 0;
  }

  AssertCrash( str );
  AssertCrash( start.diff_idx < buf.diffs.len );

  idx_t ncopied = 0;
  auto x = start;
  Forever {
    if( !len ) {
      break;
    }
    if( x.diff_idx >= buf.diffs.len ) {
      break;
    }
    auto diff = buf.diffs.mem + x.diff_idx;
    AssertCrash( x.offset_into_diff < diff->slice.len );
    auto ncopy = MIN( len, diff->slice.len - x.offset_into_diff );
    AssertCrash( ncopy <= len ); // caller must supply a buffer big enough to receive all contents.
    Memmove( str + ncopied, diff->slice.mem + x.offset_into_diff, ncopy );
    ncopied += ncopy;
    len -= ncopy;

    x.diff_idx += 1;
    x.offset_into_diff = 0;
  }

  return ncopied;
}


bool
Equal(
  buf_t& buf,
  content_ptr_t start,
  u8* __restrict str,
  idx_t len,
  bool case_sensitive
  )
{
  AssertCrash( start.diff_idx < buf.diffs.len );

  auto len_orig = len;
  idx_t ncompared = 0;
  auto diff_idx = start.diff_idx;
  auto offset_into_diff = start.offset_into_diff;
  Forever {
    if( !len ) {
      break;
    }
    if( diff_idx >= buf.diffs.len ) {
      break;
    }
    diff_t* __restrict diff = buf.diffs.mem + diff_idx;
    u8* __restrict slice_mem = diff->slice.mem;
    auto slice_len = diff->slice.len;

    AssertCrash( offset_into_diff < slice_len );
    auto ncompare = MIN( len, slice_len - offset_into_diff );
    AssertCrash( ncompare <= len ); // caller must supply a buffer big enough to receive all contents.

    u8* __restrict curr = slice_mem + offset_into_diff;
    u8* __restrict curr_end = curr + ncompare;

    if( case_sensitive ) {
      while( curr < curr_end ) {
        if( *curr != *str ) {
          return 0;
        }
        curr += 1;
        str += 1;
      }
    } else {
      while( curr < curr_end ) {
        if( CsToLower( *curr ) != CsToLower( *str ) ) {
          return 0;
        }
        curr += 1;
        str += 1;
      }
    }

    ncompared += ncompare;
    len -= ncompare;

    diff_idx += 1;
    offset_into_diff = 0;
  }

  AssertCrash( ncompared <= len_orig );
  return ncompared == len_orig;
}



// =================================================================================
// FILESYS INTERFACE CALLS
//

void
BufLoadEmpty( buf_t& buf )
{
  ProfFunc();

  Init( buf.plist, 256 );

  Alloc( buf.diffs, 8 );

  buf.content_len = 0;

  Alloc( buf.history, 8 );
  buf.history_idx = 0;
}


void
BufLoad( buf_t& buf, file_t& file )
{
  ProfFunc();
  AssertCrash( file.size < MAX_idx );

  Init( buf.plist, MAX( 256, Cast( idx_t, file.size ) ) );

  Alloc( buf.diffs, 256 );

  if( file.size ) {
    constant u64 c_chunk_size = 2*1024*1024 - 256; // leave a little space for plistheader_t
    u64 nchunks = file.size / c_chunk_size;
    u64 nremain = file.size % c_chunk_size;

    Fori( u64, i, 0, nchunks + 1 ) {
      auto chunk_size = ( i == nchunks )  ?  nremain  :  c_chunk_size;
      if( chunk_size ) {
        auto diff = AddBack( buf.diffs );
        diff->slice.len = Cast( idx_t, chunk_size );
        diff->slice.mem = AddPlist( u8, buf.plist, diff->slice.len );
        FileRead( file, i * c_chunk_size, ML( diff->slice ) );
        _AllocAndFillLnStarts( *diff );
      }
    }
  }

  buf.content_len = Cast( idx_t, file.size );

  Alloc( buf.history, 256 );
  buf.history_idx = 0;
}


void
BufSave( buf_t& buf, file_t& file )
{
  ProfFunc();

  // PERF: we shouldn't bother chunking file writes that are already appropriately sized.
  // i.e. if a diff is already a reasonable medium size, we shouldn't copy it for chunking.
  constant idx_t c_chunk_size = 2*1024*1024;
  string_t chunk;
  Alloc( chunk, c_chunk_size );

  idx_t contentlen = buf.content_len;
  idx_t nchunks = contentlen / c_chunk_size;
  idx_t nrem    = contentlen % c_chunk_size;

  idx_t bytepos = 0;
  content_ptr_t bufpos = GetBOF( buf );
  For( i, 0, nchunks + 1 ) {
    auto chunk_size = ( i == nchunks )  ?  nrem  :  c_chunk_size;
    if( chunk_size ) {
      bufpos = Contents( buf, bufpos, chunk.mem, chunk_size );
      FileWrite( file, bytepos, chunk.mem, chunk_size );
      bytepos += chunk_size;
    }
  }
  FileSetEOF( file, contentlen );

  Free( chunk );
}



// =================================================================================
// CONTENT-INDEPENDENT NAVIGATION CALLS
//   these only depend on file size.
//

Inl content_ptr_t
CursorCharL( buf_t& buf, content_ptr_t pos, idx_t len, idx_t* nchars_moved )
{
  if( !IsEOF( buf, pos ) ) {
    // since it can be easy to introduce an invalid content_ptr_t bug, we check here, in a common function.
    // look upstream for who created the 'pos' passed in here; they've made an invalid content_ptr_t!
    auto diff = buf.diffs.mem + pos.diff_idx;
    AssertCrash( pos.offset_into_diff < diff->slice.len );
  }

  idx_t nmoved = 0;
  Forever {
    if( !len  ||  IsBOF( buf, pos ) ) {
      break;
    }

    if( len <= pos.offset_into_diff ) {
      pos.offset_into_diff -= len;
      nmoved += len;
      break;

    } elif( pos.diff_idx ) {
      len -= pos.offset_into_diff + 1;
      nmoved += pos.offset_into_diff + 1;
      pos.diff_idx -= 1;
      auto diff = buf.diffs.mem + pos.diff_idx;
      AssertCrash( diff->slice.len );
      pos.offset_into_diff = diff->slice.len - 1;

    } else {
      // already at BOF
      break;
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

Inl content_ptr_t
CursorCharR( buf_t& buf, content_ptr_t pos, idx_t len, idx_t* nchars_moved )
{
  if( !IsEOF( buf, pos ) ) {
    // since it can be easy to introduce an invalid content_ptr_t bug, we check here, in a common function.
    // look upstream for who created the 'pos' passed in here; they've made an invalid content_ptr_t!
    auto diff = buf.diffs.mem + pos.diff_idx;
    AssertCrash( pos.offset_into_diff < diff->slice.len );
  }

  idx_t nmoved = 0;
  Forever {
    if( !len  ||  IsEOF( buf, pos ) ) {
      break;
    }
    AssertCrash( pos.diff_idx < buf.diffs.len );
    auto diff = buf.diffs.mem + pos.diff_idx;
    AssertCrash( pos.offset_into_diff <= diff->slice.len );
    if( pos.offset_into_diff + len < diff->slice.len ) {
      pos.offset_into_diff += len;
      nmoved += len;
      break;

    } elif( pos.diff_idx + 1 <= buf.diffs.len ) {
      len -= diff->slice.len - pos.offset_into_diff;
      nmoved += diff->slice.len - pos.offset_into_diff;
      pos.diff_idx += 1;
      pos.offset_into_diff = 0;

    } else {
      // already at EOF
      break;
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}



// =================================================================================
// CONTENT SEARCH CALLS
//

#if 1

void
FindFirstR(
  buf_t& buf,
  content_ptr_t start,
  u8* str,
  idx_t len,
  content_ptr_t* r,
  bool* found,
  bool case_sensitive,
  bool word_boundary
  )
{
  ProfFunc();

  // XXXXXXXXXXXXXXXXXXXXXX
  // XXXXXXXXXXXXXXXXXXXXXX
  // TODO: PERF: This is wicked slow; just way too much overhead for each byte.
  // XXXXXXXXXXXXXXXXXXXXXX
  // XXXXXXXXXXXXXXXXXXXXXX
  // try finding the firstchar/firstchar match quickly first, and then iterate on failure.
  // hopefully we can cut down loopiter overhead to the bare minimum.

  *found = 0;

  if( !buf.content_len || !len ) {
    return;
  }
  if( IsEOF( buf, start ) ) {
    return;
  }

  AssertCrash( str );

  auto firstchar = str[0];
  auto firstchar_lower = CsToLower( firstchar );
  diff_t* __restrict diff_end = buf.diffs.mem + buf.diffs.len;
  diff_t* __restrict diff = buf.diffs.mem + start.diff_idx;
  auto offset_into_diff = start.offset_into_diff;
  idx_t foundoffset = 0;

  while( diff < diff_end ) {

    // quickly iterate, looking for a firstchar match.
    bool foundfirst = 0;
    while( diff < diff_end ) {
      u8* __restrict slice_mem = diff->slice.mem;
      auto slice_len = diff->slice.len;

      u8* __restrict curr = slice_mem + offset_into_diff;
      u8* __restrict curr_end = slice_mem + slice_len;

      if( case_sensitive ) {
        while( curr < curr_end ) {
          if( *curr == firstchar ) {
            foundfirst = 1;
            foundoffset = curr - slice_mem;
            break;
          }
          curr += 1;
        }
      }
      else
      {
        while( curr < curr_end ) {
          if( CsToLower( *curr ) == firstchar_lower ) {
            foundfirst = 1;
            foundoffset = curr - slice_mem;
            break;
          }
          curr += 1;
        }
      }

      if( foundfirst ) {
        break;
      }

      offset_into_diff = 0;
      diff += 1;
    }

    // unable to find a firstchar match.
    if( !foundfirst ) {
      return;
    }

    content_ptr_t match_start;
    match_start.diff_idx = diff - buf.diffs.mem;
    match_start.offset_into_diff = foundoffset;

    auto next = CursorCharR( buf, match_start, 1, 0 );

    // now we have a firstchar match; verify the rest also matches.
    if( len > 1 ) {
      bool rest_matches = Equal(
        buf,
        next,
        str + 1,
        len - 1,
        case_sensitive
        );
      if( !rest_matches ) {
        diff = buf.diffs.mem + next.diff_idx;
        offset_into_diff = next.offset_into_diff;
        continue;
      }
    }

    // now we have a fullstring match!

    // eliminate the fullstring match, if it fails the word_boundary test.
    // if we're searching "0123456789" for "234", the test is:
    //   keep-match <=> InWord( "1" ) != InWord( "2" ) && InWord( "4" ) != InWord( "5" )
    // i.e. the first and last chars in the match have to have different "wordiness" than the chars outside.
    //
    if( word_boundary ) {
      idx_t nmoved;
      auto before_match_start = CursorCharL( buf, match_start, 1, &nmoved );
      if( nmoved ) {
        AssertCrash( before_match_start.diff_idx < buf.diffs.len );
        auto diff_before_match_start = buf.diffs.mem + before_match_start.diff_idx;
        AssertCrash( before_match_start.offset_into_diff < diff_before_match_start->slice.len );
        u8 c = diff_before_match_start->slice.mem[ before_match_start.offset_into_diff ];
        if( InWord( c ) == InWord( firstchar ) ) {
          diff = buf.diffs.mem + next.diff_idx;
          offset_into_diff = next.offset_into_diff;
          continue;
        }
      }
      auto after_match = CursorCharR( buf, match_start, len, &nmoved );
      if( nmoved == len ) {
        AssertCrash( after_match.diff_idx < buf.diffs.len );
        auto diff_before_x = buf.diffs.mem + after_match.diff_idx;
        AssertCrash( after_match.offset_into_diff < diff_before_x->slice.len );
        u8 c = diff_before_x->slice.mem[ after_match.offset_into_diff ];
        if( InWord( c ) == InWord( str[ len - 1 ] ) ) {
          diff = buf.diffs.mem + next.diff_idx;
          offset_into_diff = next.offset_into_diff;
          continue;
        }
      }
    }

    // our fullstring match passed the word_boundary test!
    *found = 1;
    *r = match_start;
    return;
  }


#if 0


  // PERF: is it faster to just copy chunks, and do linear search on those chunks?

  idx_t nmatched = 0;
  auto x = start;
  auto match_start = start;
  while( x.diff_idx < buf.diffs.len ) {

    auto diff = buf.diffs.mem + x.diff_idx;
    auto slice_mem = diff->slice.mem;
    auto slice_len = diff->slice.len;
    auto curr = slice_mem;

    For( i, x.offset_into_diff, slice_len ) {
      u8 match = str[ nmatched ];
      bool matched =
        ( case_sensitive  &&  *curr == match )  ||
        ( !case_sensitive  &&  CsToLower( *curr ) == CsToLower( match ) );
      if( matched ) {
        if( !nmatched ) {
          match_start = x;
        }
        nmatched += 1;
      } else {
        if( nmatched ) {
          nmatched = 0;
          x = match_start;
        }
      }

      curr += 1;

      if( nmatched == len ) {
        if( word_boundary ) {
          idx_t nmoved;
          auto before_match_start = CursorCharL( buf, match_start, 1, &nmoved );
          if( nmoved ) {
            AssertCrash( before_match_start.diff_idx < buf.diffs.len );
            auto diff_before_match_start = buf.diffs.mem + before_match_start.diff_idx;
            AssertCrash( before_match_start.offset_into_diff < diff_before_match_start->slice.len );
            u8 c = diff_before_match_start->slice.mem[ before_match_start.offset_into_diff ];
            // TODO: option to compare InWord( before_match_start ) to InWord( match_start )
            if( InWord( c ) ) {
              x = match_start;
              nmatched = 0;
            }
          }
          if( nmatched ) {
            auto before_x = CursorCharL( buf, x, 1, &nmoved );
            if( nmoved ) {
              // TODO: option to compare InWord( before_x ) to InWord( x )
              AssertCrash( before_x.diff_idx < buf.diffs.len );
              auto diff_before_x = buf.diffs.mem + before_x.diff_idx;
              AssertCrash( before_x.offset_into_diff < diff_before_x->slice.len );
              u8 c = diff_before_x->slice.mem[ before_x.offset_into_diff ];
              if( InWord( c ) ) {
                // no need to check before_match_start_valid, since it will only be invalid on a match starting at BOF.
                // i.e. match_start == BOF == before_match_start.
                // that's what we want for before_x, since we're incrementing by one below.
                x = match_start;
                nmatched = 0;
              }
            }
          }
          if( nmatched ) {
            *r = match_start;
            *found = 1;
            return;
          }
        } else {
          *r = match_start;
          *found = 1;
          return;
        }
      }

      x.offset_into_diff += 1;
    }

    x.diff_idx += 1;
    x.offset_into_diff = 0;
  }

#endif

}

#else

void
FindFirstR(
  buf_t& buf,
  content_ptr_t start,
  u8* str,
  idx_t len,
  content_ptr_t* r,
  bool* found,
  bool case_sensitive,
  bool word_boundary
  )
{
  ProfFunc();

  *found = 0;

  if( !buf.content_len || !len ) {
    return;
  }
  if( IsEOF( buf, start ) ) {
    return;
  }

  AssertCrash( str );

  // PERF: is it faster to just copy chunks, and do linear search on those chunks?

  idx_t nmatched = 0;
  auto x = start;
  auto before_x = start;
  bool before_x_valid = 0;
  auto match_start = start;
  auto before_match_start = start;
  bool before_match_start_valid = 0;
  Forever {
    if( x.diff_idx >= buf.diffs.len ) {
      break;
    }

    Forever {
      auto diff = buf.diffs.mem + x.diff_idx;

      AssertCrash( x.offset_into_diff < diff->slice.len );
      u8 curr = diff->slice.mem[ x.offset_into_diff ];
      u8 match = str[ nmatched ];
      bool case_match = ( case_sensitive  &&  curr == match );
      bool ignore_match = ( !case_sensitive  &&  ( CsToLower( curr ) == CsToLower( match ) ) );
      if( case_match  ||  ignore_match ) {
        if( !nmatched ) {
          if( before_x_valid ) {
            before_match_start_valid = 1;
            before_match_start = before_x;
          }
          match_start = x;
        }
        nmatched += 1;
      } else {
        if( nmatched ) {
          nmatched = 0;
          if( before_match_start_valid ) {
            before_x_valid = 1;
            before_x = before_match_start;
          }
          x = match_start;
        }
      }

      if( nmatched == len ) {
        if( word_boundary ) {
          if( before_match_start_valid ) {
            AssertCrash( before_match_start.diff_idx < buf.diffs.len );
            auto diff_before_match_start = buf.diffs.mem + before_match_start.diff_idx;
            AssertCrash( before_match_start.offset_into_diff < diff_before_match_start->slice.len );
            u8 c = diff_before_match_start->slice.mem[ before_match_start.offset_into_diff ];
            // TODO: option to compare InWord( before_match_start ) to InWord( match_start )
            if( InWord( c ) ) {
              before_x_valid = 1;
              before_x = before_match_start;
              x = match_start;
              nmatched = 0;
            }
          }
          if( nmatched ) {
            // TODO: option to compare InWord( before_x ) to InWord( x )
            AssertCrash( before_x.diff_idx < buf.diffs.len );
            auto diff_before_x = buf.diffs.mem + before_x.diff_idx;
            AssertCrash( before_x.offset_into_diff < diff_before_x->slice.len );
            u8 c = diff_before_x->slice.mem[ before_x.offset_into_diff ];
            if( InWord( c ) ) {
              // no need to check before_match_start_valid, since it will only be invalid on a match starting at BOF.
              // i.e. match_start == BOF == before_match_start.
              // that's what we want for before_x, since we're incrementing by one below.
              before_x_valid = 1;
              before_x = before_match_start;
              x = match_start;
              nmatched = 0;
            }
          }
          if( nmatched ) {
            *r = match_start;
            *found = 1;
            return;
          }
        } else {
          *r = match_start;
          *found = 1;
          return;
        }
      }

      if( x.offset_into_diff + 1 < diff->slice.len ) {
        before_x_valid = 1;
        before_x = x;
        x.offset_into_diff += 1;
      } else {
        break;
      }
    }

    before_x_valid = 1;
    before_x = x;
    x.diff_idx += 1;
    x.offset_into_diff = 0;
  }
}

#endif


void
FindFirstL(
  buf_t& buf,
  content_ptr_t start,
  u8* str,
  idx_t len,
  content_ptr_t* l,
  bool* found,
  bool case_sensitive,
  bool word_boundary
  )
{
  ProfFunc();

  *found = 0;

  if( !buf.content_len || !len ) {
    return;
  }
  if( IsBOF( buf, start ) ) {
    return;
  }

  AssertCrash( str );

  idx_t nmatched = 0;
  auto x = start;
  auto match_end = start;
  auto before_match_end = x;

  diff_t* diff;
  if( x.offset_into_diff ) {
    x.offset_into_diff -= 1;
    diff = buf.diffs.mem + x.diff_idx;
  } else {
    AssertCrash( x.diff_idx );
    x.diff_idx -= 1;
    diff = buf.diffs.mem + x.diff_idx;
    x.offset_into_diff = diff->slice.len;
  }

  Forever {
    Forever {
      if( !x.offset_into_diff ) {
        break;
      }
      x.offset_into_diff -= 1;

      AssertCrash( x.offset_into_diff < diff->slice.len );
      u8 curr = diff->slice.mem[ x.offset_into_diff ];
      u8 match = str[ len - nmatched - 1 ];
      bool case_match = ( case_sensitive  &&  curr == match );
      bool ignore_match = ( !case_sensitive  &&  ( CsToLower( curr ) == CsToLower( match ) ) );
      if( case_match  ||  ignore_match ) {
        if( !nmatched ) {
          before_match_end = x;
          match_end = CursorCharR( buf, x, 1, 0 );
        }
        nmatched += 1;
      } else {
        if( nmatched ) {
          x = before_match_end;
          nmatched = 0;
        }
      }

      if( nmatched == len ) {
        if( word_boundary ) {
          // TODO: option to compare InWord( before_x ) to InWord( x )
          auto before_x = CursorCharL( buf, x, 1, 0 );
          if( !Equal( before_x, x ) ) {
            AssertCrash( before_x.diff_idx < buf.diffs.len );
            auto diff_before_x = buf.diffs.mem + before_x.diff_idx;
            AssertCrash( before_x.offset_into_diff < diff_before_x->slice.len );
            u8 c = diff_before_x->slice.mem[ before_x.offset_into_diff ];
            if( InWord( c ) ) {
              x = before_match_end;
              nmatched = 0;
              continue;
            }
          }
          // TODO: option to compare InWord( before_match_end ) to InWord( match_end )
          if( !IsEOF( buf, match_end ) ) {
            AssertCrash( match_end.diff_idx < buf.diffs.len );
            auto diff_match_end = buf.diffs.mem + match_end.diff_idx;
            AssertCrash( match_end.offset_into_diff < diff_match_end->slice.len );
            u8 c = diff_match_end->slice.mem[ match_end.offset_into_diff ];
            if( InWord( c ) ) {
              x = before_match_end;
              nmatched = 0;
              continue;
            }
          }
        }
        *l = match_end;
        *found = 1;
        return;
      }

    }
    if( !x.diff_idx ) {
      break;
    }
    x.diff_idx -= 1;
    diff = buf.diffs.mem + x.diff_idx;
    x.offset_into_diff = diff->slice.len;
  }
}



// =================================================================================
// CONTENT MODIFY CALLS
//

Inl void
DiffInsert(
  buf_t& buf,
  idx_t idx,
  diff_t* diff
  )
{
  AssertCrash( diff->slice.len );
#if LNCACHE
  AssertCrash( diff->ln_starts.capacity );
#endif
  *AddAt( buf.diffs, idx ) = *diff;
  buf.content_len += diff->slice.len;
}

Inl void
DiffRemove(
  buf_t& buf,
  idx_t idx,
  diff_t* verify_removed
  )
{
  auto removed = buf.diffs.mem + idx;
  AssertCrash( removed->slice.len );
  AssertCrash( buf.content_len >= removed->slice.len );
  buf.content_len -= removed->slice.len;

  if( verify_removed ) {
    AssertCrash( Equal( *removed, *verify_removed ) );
  }

  Kill( *removed );
  RemAt( buf.diffs, idx );
}

Inl void
DiffReplace(
  buf_t& buf,
  idx_t idx,
  diff_t* dst_verify,
  diff_t* src
  )
{
  auto dst = buf.diffs.mem + idx;
  idx_t content_len_dst = dst->slice.len;

  if( dst_verify ) {
    AssertCrash( Equal( *dst, *dst_verify ) );
  }

  idx_t content_len_src = src->slice.len;
  Kill( *dst );
  *dst = *src;

  AssertCrash( buf.content_len + content_len_src >= content_len_dst );
  buf.content_len += content_len_src;
  buf.content_len -= content_len_dst;
}


Inl void
AddHistorical( buf_t& buf, undoableoper_t& undoable )
{
  // invalidate previous futures.
  AssertCrash( buf.history_idx <= buf.history.len );
  buf.history.len = buf.history_idx;
  *AddBack( buf.history ) = undoable;
  buf.history_idx += 1;
}

Inl void
ForwardDiffOper(
  buf_t& buf,
  idx_t diff_idx,
  diff_t* diff_old,
  diff_t* diff_new,
  undoableopertype_t type
  )
{
  undoableoper_t oper;
  oper.type = type;
  if( diff_old ) {
    oper.diff_old.slice = diff_old->slice;
#if LNCACHE
    auto ncopy = diff_old->ln_starts.len;
    if( ncopy ) {
      Alloc( oper.diff_old.ln_starts, ncopy );
      Copy( oper.diff_old.ln_starts, diff_old->ln_starts );
    } else {
      Alloc( oper.diff_old.ln_starts, diff_old->ln_starts.capacity );
    }
#endif
  } else {
    oper.diff_old.slice = {};
#if LNCACHE
    Zero( oper.diff_old.ln_starts );
#endif
  }
  if( diff_new ) {
    oper.diff_new.slice = diff_new->slice;
#if LNCACHE
    auto ncopy = diff_new->ln_starts.len;
    if( ncopy ) {
      Alloc( oper.diff_new.ln_starts, ncopy );
      Copy( oper.diff_new.ln_starts, diff_new->ln_starts );
    } else {
      Alloc( oper.diff_new.ln_starts, diff_new->ln_starts.capacity );
    }
#endif
  } else {
    oper.diff_new.slice = {};
#if LNCACHE
    Zero( oper.diff_new.ln_starts );
#endif
  }
  oper.idx = diff_idx;
  AddHistorical( buf, oper );

  // actually perform the operation:

  switch( type ) {
    case undoableopertype_t::add: {
      DiffInsert( buf, diff_idx, diff_new );
    } break;
    case undoableopertype_t::mod: {
      DiffReplace( buf, diff_idx, diff_old, diff_new );
    } break;
    case undoableopertype_t::rem: {
      DiffRemove( buf, diff_idx, diff_old );
    } break;
    case undoableopertype_t::checkpt: __fallthrough;
    default: UnreachableCrash();
  }
}


//
// split the pos.diff_idx diff at pos.offset_into_diff
//       [         diff         ]
//       [   left   |   right   ]
// returns a content_ptr_t to `right`
//
Inl content_ptr_t
_SplitDiffAt(
  buf_t& buf,
  content_ptr_t pos,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  AssertCrash( pos.diff_idx < buf.diffs.len );
  auto diff = buf.diffs.mem + pos.diff_idx;

  auto add_diff_idx = pos.diff_idx + 1;
  diff_t right;
  right.slice.mem = diff->slice.mem + pos.offset_into_diff;
  right.slice.len = diff->slice.len - pos.offset_into_diff;
  AssertCrash( right.slice.len );
  _AllocAndFillLnStarts( right );
  ForwardDiffOper(
    buf,
    add_diff_idx,
    0,
    &right,
    undoableopertype_t::add
    );

  // since buf.diffs is an array_t, we can realloc after the add.
  // so re-query diff.
  diff = buf.diffs.mem + pos.diff_idx;

  diff_t left;
  left.slice.mem = diff->slice.mem;
  left.slice.len = pos.offset_into_diff;
  AssertCrash( left.slice.len );
  _AllocAndFillLnStarts( left );
  ForwardDiffOper(
    buf,
    pos.diff_idx,
    diff,
    &left,
    undoableopertype_t::mod
    );

  // update any given ptrs to the same actual contents, after the split.
  // this is necessary for copy / move / swap, which require multiple concurrent ptrs.
  For( i, 0, concurrents_len ) {
    auto concurrent = concurrents[i];
    if( concurrent->diff_idx == pos.diff_idx ) {
      if( concurrent->offset_into_diff >= left.slice.len ) {
        concurrent->offset_into_diff -= left.slice.len;
        concurrent->diff_idx += 1;
        AssertCrash( concurrent->offset_into_diff < right.slice.len );
      }
    } elif( concurrent->diff_idx >= add_diff_idx ) {
      concurrent->diff_idx += 1;
    }
  }

  content_ptr_t r;
  r.diff_idx = add_diff_idx;
  r.offset_into_diff = 0;
  return r;
}


//
// insert contents at the given position.
// e.g. Insert( "0123456789", 4, "asdf" )' -> "0123asdf456789"
//
// here's what we do to all elements in the 'concurrents' list:
//
//   pointers BEFORE the inserted section remain unchanged.
//      [aaabbbb]
//        ^
//      [aaaXXXXbbbb]
//        ^
//
//   pointers AFTER the inserted section move right, to point to the same content as they did before.
//      [aaabbbb]
//           ^
//      [aaaXXXXbbbb]
//               ^
//
content_ptr_t
Insert(
  buf_t& buf,
  content_ptr_t pos,
  u8* str,
  idx_t len,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  AssertCrash( pos.diff_idx <= buf.diffs.len );

  if( !len ) {
    return pos;
  }

  if( pos.offset_into_diff ) {
    pos = _SplitDiffAt( buf, pos, concurrents, concurrents_len );
  }

  // ensure maximum-size chunks, so our per-chunk datastructures don't get too huge individually.
  constant idx_t c_chunk_size = 2*1024*1024 - 256; // leave a little space for plistheader_t
  idx_t nchunks = len / c_chunk_size;
  idx_t nremain = len % c_chunk_size;

  content_ptr_t end;
  end.diff_idx = pos.diff_idx + nchunks + 1;
  end.offset_into_diff = 0;

  // insert N new diffs at diff_idx.
  For( i, 0, nchunks + 1 ) {
    auto chunk_size = ( i == nchunks )  ?  nremain  :  c_chunk_size;
    auto add_diff_idx = pos.diff_idx + i;
    if( chunk_size ) {
      diff_t add;
      add.slice.len = chunk_size;
      add.slice.mem = AddPlist( u8, buf.plist, add.slice.len );
      AssertCrash( add.slice.len );
      Memmove( add.slice.mem, str + i * c_chunk_size, add.slice.len );
      _AllocAndFillLnStarts( add );
      ForwardDiffOper(
        buf,
        add_diff_idx,
        0,
        &add,
        undoableopertype_t::add
        );

      // update any given ptrs to the same actual contents, after the insertion.
      // PERF: move this out after this nchunks loop, so we aren't N^2
      For( j, 0, concurrents_len ) {
        auto concurrent = concurrents[j];
        if( concurrent->diff_idx >= add_diff_idx ) {
          concurrent->diff_idx += 1;
        }
      }
    }
  }

  //
  // to keep the overall diff count low, concat adjacent diff_t's if we can do it quickly.
  // we only merge the first newly-inserted diff with it's previous, and only if the total len is small.
  // this is because people actually type 1,000s of characters, and we don't want that many diffs.
  // it'd be cool if we could stream new characters right into existing diffs, i.e. avoid this
  // 'merge-after-the-fact', but that requires some deep introspection into plist_t that we don't have right now.
  // if we switched to pagearray_t, we probably could do that introspection. something to consider for later.
  //
  if( pos.diff_idx ) {
    AssertCrash( pos.diff_idx < buf.diffs.len );
    auto diff = buf.diffs.mem + pos.diff_idx;

    content_ptr_t prev;
    prev.diff_idx = pos.diff_idx - 1;
    prev.offset_into_diff = 0;
    AssertCrash( prev.diff_idx < buf.diffs.len );
    auto diff_prev = buf.diffs.mem + prev.diff_idx;

    constant idx_t c_combine_threshold = 128;
    auto combined_len = diff->slice.len + diff_prev->slice.len;
    if( combined_len < c_combine_threshold ) {
      diff_t repl;
      repl.slice.len = combined_len;
      repl.slice.mem = AddPlist( u8, buf.plist, repl.slice.len );
      AssertCrash( combined_len );
      Memmove( repl.slice.mem, diff_prev->slice.mem, diff_prev->slice.len );
      Memmove( repl.slice.mem + diff_prev->slice.len, diff->slice.mem, diff->slice.len );
      _AllocAndFillLnStarts( repl );

      auto repl_diff_slice_len = diff_prev->slice.len;
      auto repl_diff_idx = prev.diff_idx;
      auto del_diff_idx = pos.diff_idx;
      ForwardDiffOper(
        buf,
        repl_diff_idx,
        diff_prev,
        &repl,
        undoableopertype_t::mod
        );
      ForwardDiffOper(
        buf,
        del_diff_idx,
        diff,
        0,
        undoableopertype_t::rem
        );

      // don't use 'diff' or 'diff_prev' here, after the ForwardDiffOper calls above.

      // update any given ptrs to the same actual contents, after the insertion.
      For( j, 0, concurrents_len ) {
        auto concurrent = concurrents[j];
        if( concurrent->diff_idx == del_diff_idx ) {
          concurrent->diff_idx -= 1;
          concurrent->offset_into_diff += repl_diff_slice_len;
        } elif( concurrent->diff_idx > del_diff_idx ) {
          concurrent->diff_idx -= 1;
        }
      }
    }
  }

  return end;
}

Inl content_ptr_t
Insert(
  buf_t& buf,
  content_ptr_t pos,
  u8 c,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  return Insert(
    buf,
    pos,
    &c,
    1,
    concurrents,
    concurrents_len
    );
}


//
// delete contents in the range: [a, b)
// e.g. Delete( "0123456789", 4, 8 ) -> "012389"
//
// here's what we do to all elements in the 'concurrents' list:
//
//   pointers BEFORE the deleted section remain unchanged.
//      [aaaXXXXbbbb]
//        ^
//      [aaabbbb]
//        ^
//
//   pointers INTO a deleted section get moved left, to point to the content that was after the deleted section.
//      [aaaXXXXbbbb]
//            ^
//      [aaabbbb]
//          ^
//
//   pointers AFTER the deleted section move left, to point to the same content as they did before.
//      [aaaXXXXbbbb]
//               ^
//      [aaabbbb]
//           ^
//
void
Delete(
  buf_t& buf,
  content_ptr_t start,
  content_ptr_t end,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  if( IsEOF( buf, start ) ) {
    return;
  }

  // first, split diffs at all boundaries.

  // passing arbitrary numbers of outparams isn't pretty, when we have to tack some on.
  // PERF: allocating these outparam lists isn't great.
  fixedarray_t<content_ptr_t*> concurrents_and_start;
  fixedarray_t<content_ptr_t*> concurrents_and_end;
  Alloc( concurrents_and_start, concurrents_len + 1 );
  Alloc( concurrents_and_end,   concurrents_len + 1 );
  Memmove( AddBack( concurrents_and_start, concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_and_end,   concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  *AddBack( concurrents_and_start ) = &start;
  *AddBack( concurrents_and_end ) = &end;

  if( start.offset_into_diff ) {
    start = _SplitDiffAt( buf, start, ML( concurrents_and_end ) );
  }
  if( end.offset_into_diff ) {
    end = _SplitDiffAt( buf, end, ML( concurrents_and_start ) );
  }

  Free( concurrents_and_end );
  Free( concurrents_and_start );

  // now the problem is transformed into diff space.

  ReverseFor( rem_diff_idx, start.diff_idx, end.diff_idx ) {
    ForwardDiffOper(
      buf,
      rem_diff_idx,
      buf.diffs.mem + rem_diff_idx,
      0,
      undoableopertype_t::rem
      );

    // update any given ptrs to the same actual contents ( or left-nearest if contents are deleted ), after the deletion.
    // PERF: move this out after this start->end loop, so we aren't N^2
    For( i, 0, concurrents_len ) {
      auto concurrent = concurrents[i];
      if( concurrent->diff_idx == rem_diff_idx ) {
        concurrent->offset_into_diff = 0;
      } elif( concurrent->diff_idx > rem_diff_idx ) {
        concurrent->diff_idx -= 1;
      }
    }
  }
}


Inl content_ptr_t
Replace(
  buf_t& buf,
  content_ptr_t start,
  content_ptr_t end,
  u8* str,
  idx_t len,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  // passing arbitrary numbers of outparams isn't pretty, when we have to tack some on.
  // PERF: allocating these outparam lists isn't great.
  fixedarray_t<content_ptr_t*> new_concurrents;
  Alloc( new_concurrents, concurrents_len + 1 );
  Memmove( AddBack( new_concurrents, concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  *AddBack( new_concurrents ) = &start;

  Delete( buf, start, end, ML( new_concurrents ) );

  Free( new_concurrents );

  return Insert( buf, start, str, len, concurrents, concurrents_len );
}


//
//   pointers BEFORE the DST section remain unchanged.
//      [aaaabbcc]
//       ^
//      [aabbaabbcc]
//       ^
//
//   pointers INTO a SRC section move, to point to the same SRC content as they did before.
//   there's no mechanism to make duplicate pointers to the DST content. maybe there should be, if someone needs it.
//      [aaaabbcc]
//            ^
//      [aabbaabbcc]
//              ^
//
//   pointers AFTER the DST section move, to point to the same content as they did before.
//      [aaaabbcc]
//             ^
//      [aabbaabbcc]
//               ^
//
void
Copy(
  buf_t& buf,
  content_ptr_t src_start,
  content_ptr_t src_end,
  content_ptr_t dst,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  AssertCrash( LEqual( src_start, src_end ) );

  if( IsEOF( buf, src_start ) ) {
    return;
  }
  if( Equal( src_start, src_end ) ) {
    return;
  }

  // TODO: first part is a dupe of code from Move.

  // first, split diffs at all src/dst boundaries.

  // passing arbitrary numbers of outparams isn't pretty, when we have to tack some on.
  // PERF: allocating these outparam lists isn't great.
  fixedarray_t<content_ptr_t*> concurrents_dst_srcend;
  fixedarray_t<content_ptr_t*> concurrents_dst_srcstart;
  fixedarray_t<content_ptr_t*> concurrents_srcstart_srcend;
  Alloc( concurrents_dst_srcend,      concurrents_len + 2 );
  Alloc( concurrents_dst_srcstart,    concurrents_len + 2 );
  Alloc( concurrents_srcstart_srcend, concurrents_len + 2 );
  Memmove( AddBack( concurrents_dst_srcend,      concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_dst_srcstart,    concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_srcstart_srcend, concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  *AddBack( concurrents_dst_srcend ) = &dst;
  *AddBack( concurrents_dst_srcend ) = &src_end;
  *AddBack( concurrents_dst_srcstart ) = &dst;
  *AddBack( concurrents_dst_srcstart ) = &src_start;
  *AddBack( concurrents_srcstart_srcend ) = &src_start;
  *AddBack( concurrents_srcstart_srcend ) = &src_end;

  if( src_start.offset_into_diff ) {
    src_start = _SplitDiffAt( buf, src_start, ML( concurrents_dst_srcend ) );
  }
  if( src_end.offset_into_diff ) {
    src_end = _SplitDiffAt( buf, src_end, ML( concurrents_dst_srcstart ) );
  }
  if( dst.offset_into_diff ) {
    dst = _SplitDiffAt( buf, dst, ML( concurrents_srcstart_srcend ) );
  }

  Free( concurrents_dst_srcend );
  Free( concurrents_dst_srcstart );
  Free( concurrents_srcstart_srcend );

  // now the problem is transformed into diff space.

  //
  // 0123456789         initial
  //    ^----           src
  //       ^            dst
  // 0123453456789      after left loop
  // 012345345676789    after rght loop
  //
  //     auto src_end = src_idx + src_len;
  //     auto left = ( dst_idx > src_idx )  ?  dst_idx - src_idx  :  0;
  //     auto rght = ( src_end > dst_idx )  ?  src_end - dst_idx  :  0;
  //     For( i, 0, left ) {
  //       auto diff_src = buf.diffs.mem + src_idx;
  //       AddCopyAt( diff_src, dst_idx );
  //       dst_idx += 1;
  //       src_idx += 1;
  //     }
  //     src_idx += left;
  //     For( i, 0, rght ) {
  //       auto diff_src = buf.diffs.mem + src_idx;
  //       AddCopyAt( diff_src, dst_idx );
  //       dst_idx += 1;
  //       src_idx += 1;
  //       src_idx += 1;
  //     }
  //
  // transforms into:
  //
  //     For( i, 0, src_len ) {
  //       if( i == left ) {
  //         src_idx += left;
  //       }
  //
  //       auto diff_src = buf.diffs.mem + src_idx;
  //       AddCopyAt( diff_src, dst_idx );
  //       dst_idx += 1;
  //       src_idx += 1;
  //
  //       if( i >= left ) {
  //         src_idx += 1;
  //       }
  //     }
  //

  auto dst_idx = dst.diff_idx;
  auto src_idx = src_start.diff_idx;
  auto src_len = src_end.diff_idx - src_start.diff_idx;
  auto left = ( dst_idx > src_idx )  ?  dst_idx - src_idx  :  0;

  For( i, 0, src_len ) {
    // compensate src iterator for crossing dst's location
    if( i == left ) {
      src_idx += left;
    }

    auto diff_src = buf.diffs.mem + src_idx;

    diff_t copy;
    copy.slice = diff_src->slice;
    _AllocAndFillLnStarts( copy );
    auto add_diff_idx = dst_idx;
    ForwardDiffOper(
      buf,
      add_diff_idx,
      0,
      &copy,
      undoableopertype_t::add
      );

    // update any given ptrs to the same actual contents, after the insertion.
    // PERF: move this out after this loop, so we aren't N^2
    For( j, 0, concurrents_len ) {
      auto concurrent = concurrents[j];
      if( concurrent->diff_idx >= add_diff_idx ) {
        concurrent->diff_idx += 1;
      }
    }

    dst_idx += 1;
    src_idx += 1;

    // compensate src iterator for inserting a diff
    if( i >= left ) {
      src_idx += 1;
    }
  }
}


//
// pointers in 'concurrent' are updated to point to the same actual content they did before.
//
// note that if you are moving left ( i.e. dst < src_start ), then a concurrent with a value of src_end won't move.
// if you're tracking src_start and src_end, that means your start will update, but not your end.
// this is because this function doesn't know whether a concurrent ptr is a "list-end" ptr, or a real "content" ptr.
// since it'd be ugly to annotate the difference between these things, it's just up to the caller to pick.
// we return the moved src_end, so you can easily take that if you want.
//
content_ptr_t
Move(
  buf_t& buf,
  content_ptr_t src_start,
  content_ptr_t src_end,
  content_ptr_t dst,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  AssertCrash( LEqual( src_start, src_end ) );

  if( IsEOF( buf, src_start ) ) {
    return src_end;
  }
  if( Equal( src_start, src_end ) ) {
    return src_end;
  }
  if( LEqual( src_start, dst )  &&  LEqual( dst, src_end ) ) {
    return src_end;
  }

  // TODO: first part is a dupe of code from Copy.

  // first, split diffs at all src/dst boundaries.

  // passing arbitrary numbers of outparams isn't pretty, when we have to tack some on.
  // PERF: allocating these outparam lists isn't great.
  fixedarray_t<content_ptr_t*> concurrents_dst_srcend;
  fixedarray_t<content_ptr_t*> concurrents_dst_srcstart;
  fixedarray_t<content_ptr_t*> concurrents_srcstart_srcend;
  Alloc( concurrents_dst_srcend,      concurrents_len + 2 );
  Alloc( concurrents_dst_srcstart,    concurrents_len + 2 );
  Alloc( concurrents_srcstart_srcend, concurrents_len + 2 );
  Memmove( AddBack( concurrents_dst_srcend,      concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_dst_srcstart,    concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_srcstart_srcend, concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  *AddBack( concurrents_dst_srcend ) = &dst;
  *AddBack( concurrents_dst_srcend ) = &src_end;
  *AddBack( concurrents_dst_srcstart ) = &dst;
  *AddBack( concurrents_dst_srcstart ) = &src_start;
  *AddBack( concurrents_srcstart_srcend ) = &src_start;
  *AddBack( concurrents_srcstart_srcend ) = &src_end;

  if( src_start.offset_into_diff ) {
    src_start = _SplitDiffAt( buf, src_start, ML( concurrents_dst_srcend ) );
  }
  if( src_end.offset_into_diff ) {
    src_end = _SplitDiffAt( buf, src_end, ML( concurrents_dst_srcstart ) );
  }
  if( dst.offset_into_diff ) {
    dst = _SplitDiffAt( buf, dst, ML( concurrents_srcstart_srcend ) );
  }

  Free( concurrents_dst_srcend );
  Free( concurrents_dst_srcstart );
  Free( concurrents_srcstart_srcend );

  // now the problem is transformed into diff space.

  auto dst_idx = dst.diff_idx;
  auto src_idx = src_start.diff_idx;
  auto src_len = src_end.diff_idx - src_start.diff_idx;

  // we check the equivalent in content_ptr_t space above, as an early no-op.
  AssertCrash( !( src_idx <= dst_idx  &&  dst_idx <= src_idx + src_len ) );

  bool swap = dst_idx >= src_idx;
  if( swap ) {
    auto new_dst_idx = src_idx;
    auto new_src_idx = src_idx + src_len;
    auto new_src_len = dst_idx - src_idx - src_len;

    dst_idx = new_dst_idx;
    src_idx = new_src_idx;
    src_len = new_src_len;
  }

  auto dst_len = src_idx - dst_idx;
  auto total_len = dst_len + src_len;

  // XXXXXXXXXXXXXXXXXXXXXXX
  // XXXXXXXXXXXXXXXXXXXXXXX
  // XXXXXXXXXXXXXXXXXXXXXXX
  // TODO: this is wrong.
//  ImplementCrash();

  content_ptr_t moved_src_end;
  moved_src_end.offset_into_diff = 0;
  moved_src_end.diff_idx = swap  ?  dst_idx + dst_len  :  src_idx + src_len;
  if( moved_src_end.diff_idx < dst_idx  ||  moved_src_end.diff_idx >= src_idx + src_len ) {
    // no-op
  } elif( moved_src_end.diff_idx < src_idx ) {
    moved_src_end.diff_idx += src_len;
  } else {
    moved_src_end.diff_idx -= dst_len;
  }

  idx_t nreordered = 0;

  // bitlist for cycle detection.
  fixedarray_t<u64> bitlist;
  {
    auto quo = total_len / 64;
    auto rem = total_len % 64;
    auto nchunks = quo + ( rem > 0 );
    Alloc( bitlist, nchunks );
    For( i, 0, nchunks ) {
      bitlist.mem[i] = 0;
    }
  }

  // bitlist for tracking which concurrents we've already updated.
  fixedarray_t<u64> bitlist_concurrents;
  {
    auto quo = concurrents_len / 64;
    auto rem = concurrents_len % 64;
    auto nchunks = quo + ( rem > 0 );
    Alloc( bitlist_concurrents, nchunks );
    For( i, 0, nchunks ) {
      bitlist_concurrents.mem[i] = 0;
    }
  }

  For( i, 0, total_len ) {

    if( nreordered >= total_len ) {
      break;
    }

    // skip cycles we've already traversed.
    auto bit = bitlist.mem[ i / 64 ] & ( 1ULL << ( i % 64 ) );
    if( bit ) {
      continue;
    }

    auto s = src_idx + i;
    auto d = s - dst_len;

    idx_t t0_diff_idx = s;
    diff_t t0 = buf.diffs.mem[ s ];
    _AllocAndFillLnStarts( t0 );

    idx_t t1_diff_idx;
    diff_t t1;
    Forever {
      auto bit_idx = d - dst_idx;
      // detect cycle, and terminate cycle traversal.
      if( bitlist.mem[ bit_idx / 64 ] & ( 1ULL << ( bit_idx % 64 ) ) ) {
        break;
      }
      // mark dst as traversed.
      bitlist.mem[ bit_idx / 64 ] |= ( 1ULL << ( bit_idx % 64 ) );

      t1_diff_idx = d;
      t1 = buf.diffs.mem[ d ];
      _AllocAndFillLnStarts( t1 );
      ForwardDiffOper(
        buf,
        t1_diff_idx,
        buf.diffs.mem + t1_diff_idx,
        &t0,
        undoableopertype_t::mod
        );

      // update concurrents pointing at t0 to point to t1, since that's where we've shoved the content.
      For( j, 0, concurrents_len ) {
        // skip concurrents we've already updated.
        if( bitlist_concurrents.mem[ j / 64 ] & ( 1ULL << ( j % 64 ) ) ) {
          continue;
        }
        auto concurrent = concurrents[j];
        if( concurrent->diff_idx == t0_diff_idx ) {
          concurrent->diff_idx = t1_diff_idx;
          bitlist_concurrents.mem[ j / 64 ] |= ( 1ULL << ( j % 64 ) );
        }
      }

      nreordered += 1;
      if( nreordered >= total_len ) {
        break;
      }

      if( d < src_idx ) {
        d += src_len;
      } else {
        d -= dst_len;
      }

      bit_idx = d - dst_idx;
      // detect cycle, and terminate cycle traversal.
      if( bitlist.mem[ bit_idx / 64 ] & ( 1ULL << ( bit_idx % 64 ) ) ) {
        break;
      }
      // mark dst as traversed.
      bitlist.mem[ bit_idx / 64 ] |= ( 1ULL << ( bit_idx % 64 ) );

      t0_diff_idx = d;
      t0 = buf.diffs.mem[ d ];
      _AllocAndFillLnStarts( t0 );
      ForwardDiffOper(
        buf,
        t0_diff_idx,
        buf.diffs.mem + t0_diff_idx,
        &t1,
        undoableopertype_t::mod
        );

      // update concurrents pointing at t1 to point to t0, since that's where we've shoved the content.
      For( j, 0, concurrents_len ) {
        // skip concurrents we've already updated.
        if( bitlist_concurrents.mem[ j / 64 ] & ( 1ULL << ( j % 64 ) ) ) {
          continue;
        }
        auto concurrent = concurrents[j];
        if( concurrent->diff_idx == t1_diff_idx ) {
          concurrent->diff_idx = t0_diff_idx;
          bitlist_concurrents.mem[ j / 64 ] |= ( 1ULL << ( j % 64 ) );
        }
      }

      nreordered += 1;
      if( nreordered >= total_len ) {
        break;
      }

      if( d < src_idx ) {
        d += src_len;
      } else {
        d -= dst_len;
      }
    }
  }

  Free( bitlist_concurrents );
  Free( bitlist );

  return moved_src_end;
}

//
// pointers in 'concurrent' are updated to point to the same actual content they did before.
//
// note that your rightmost range's end ptr won't move.
// if you're tracking a/b_start, a/b_end, that means your rightmost of a/b start will update, but not your end.
// this is because this function doesn't know whether a concurrent ptr is a "list-end" ptr, or a real "content" ptr.
// since it'd be ugly to annotate the difference between these things, it's just up to the caller to pick.
// we return the moved a/b ends, so you can easily take them if you want.
//
void
Swap(
  buf_t& buf,
  content_ptr_t a_start,
  content_ptr_t a_end,
  content_ptr_t b_start,
  content_ptr_t b_end,
  content_ptr_t* moved_a_end,
  content_ptr_t* moved_b_end,
  content_ptr_t** concurrents,
  idx_t concurrents_len
  )
{
  AssertCrash(
    !( LEqual( b_start, a_start )  &&  LEqual( a_start, b_end ) ) &&
    !( LEqual( a_start, b_start )  &&  LEqual( b_start, a_end ) )
    );

  // passing arbitrary numbers of outparams isn't pretty, when we have to tack some on.
  // PERF: allocating these outparam lists isn't great.
  fixedarray_t<content_ptr_t*> concurrents_aend_bstart_bend;
  fixedarray_t<content_ptr_t*> concurrents_astart_aend_bend;
  fixedarray_t<content_ptr_t*> concurrents_astart_bstart_bend;
  fixedarray_t<content_ptr_t*> concurrents_astart_aend_bstart;
  Alloc( concurrents_aend_bstart_bend,   concurrents_len + 3 );
  Alloc( concurrents_astart_aend_bend,   concurrents_len + 3 );
  Alloc( concurrents_astart_bstart_bend, concurrents_len + 3 );
  Alloc( concurrents_astart_aend_bstart, concurrents_len + 3 );
  Memmove( AddBack( concurrents_aend_bstart_bend,   concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_astart_aend_bend,   concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_astart_bstart_bend, concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  Memmove( AddBack( concurrents_astart_aend_bstart, concurrents_len ), concurrents, concurrents_len * sizeof( concurrents[0] ) );
  *AddBack( concurrents_aend_bstart_bend ) = &a_end;
  *AddBack( concurrents_aend_bstart_bend ) = &b_start;
  *AddBack( concurrents_aend_bstart_bend ) = &b_end;
  *AddBack( concurrents_astart_aend_bend ) = &a_start;
  *AddBack( concurrents_astart_aend_bend ) = &a_end;
  *AddBack( concurrents_astart_aend_bend ) = &b_end;
  *AddBack( concurrents_astart_bstart_bend ) = &a_start;
  *AddBack( concurrents_astart_bstart_bend ) = &b_start;
  *AddBack( concurrents_astart_bstart_bend ) = &b_end;
  *AddBack( concurrents_astart_aend_bstart ) = &a_start;
  *AddBack( concurrents_astart_aend_bstart ) = &a_end;
  *AddBack( concurrents_astart_aend_bstart ) = &b_start;

  // split diffs so a, b have offset_into_diff=0.

  if( a_start.offset_into_diff ) {
    a_start = _SplitDiffAt( buf, a_start, ML( concurrents_aend_bstart_bend ) );
  }
  if( b_start.offset_into_diff ) {
    b_start = _SplitDiffAt( buf, b_start, ML( concurrents_astart_aend_bend ) );
  }
  if( a_end.offset_into_diff ) {
    a_end = _SplitDiffAt( buf, a_end, ML( concurrents_astart_bstart_bend ) );
  }
  if( b_end.offset_into_diff ) {
    b_end = _SplitDiffAt( buf, b_end, ML( concurrents_astart_aend_bstart ) );
  }

  Free( concurrents_astart_aend_bstart );
  Free( concurrents_astart_bstart_bend );
  Free( concurrents_astart_aend_bend );
  Free( concurrents_aend_bstart_bend );

  // now the problem is transformed into diff space.

  auto a_idx = a_start.diff_idx;
  auto b_idx = b_start.diff_idx;
  auto a_len = a_end.diff_idx - a_start.diff_idx;
  auto b_len = b_end.diff_idx - b_start.diff_idx;

  // we check the equivalent in content_ptr_t space above, as an early crash.
  AssertCrash(
    !( b_idx <= a_idx  &&  a_idx <= b_idx + b_len ) &&
    !( a_idx <= b_idx  &&  b_idx <= a_idx + a_len )
    );

  auto swap = a_idx >= b_idx;
  if( swap ) {
    auto new_a_idx = b_idx;
    auto new_a_len = b_len;
    auto new_b_idx = a_idx;
    auto new_b_len = a_len;

    a_idx = new_a_idx;
    a_len = new_a_len;
    b_idx = new_b_idx;
    b_len = new_b_len;
  }

  auto m_idx = a_idx + a_len;
  auto m_len = b_idx - m_idx;

  auto total_len = a_len + m_len + b_len;

  // XXXXXXXXXXXXXXXXXXXXXXX
  // XXXXXXXXXXXXXXXXXXXXXXX
  // XXXXXXXXXXXXXXXXXXXXXXX
  // TODO: this is wrong.
//  ImplementCrash();

  moved_a_end->offset_into_diff = 0;
  moved_a_end->diff_idx = swap  ?  a_idx + a_len  :  b_idx + b_len; // TODO: this is backwards, isn't it?
  if( moved_a_end->diff_idx < a_idx  ||  moved_a_end->diff_idx >= b_idx + b_len ) {
    // no-op
  } elif( moved_a_end->diff_idx >= b_idx ) {
    moved_a_end->diff_idx -= m_len + a_len;
  } elif( moved_a_end->diff_idx >= m_idx ) {
    moved_a_end->diff_idx += b_len - a_len;
  } else {
    moved_a_end->diff_idx += m_len + b_len;
  }

  moved_b_end->offset_into_diff = 0;
  moved_b_end->diff_idx = swap  ?  b_idx + b_len  :  a_idx + a_len; // TODO: this is backwards, isn't it?
  if( moved_b_end->diff_idx < a_idx  ||  moved_b_end->diff_idx >= b_idx + b_len ) {
    // no-op
  } elif( moved_b_end->diff_idx >= b_idx ) {
    moved_b_end->diff_idx -= m_len + a_len;
  } elif( moved_b_end->diff_idx >= m_idx ) {
    moved_b_end->diff_idx += b_len - a_len;
  } else {
    moved_b_end->diff_idx += m_len + b_len;
  }

  idx_t nreordered = 0;

  // bitlist for cycle detection.
  fixedarray_t<u64> bitlist;
  {
    auto quo = total_len / 64;
    auto rem = total_len % 64;
    auto nchunks = quo + ( rem > 0 );
    Alloc( bitlist, nchunks );
    For( i, 0, nchunks ) {
      bitlist.mem[i] = 0;
    }
  }

  // bitlist for tracking which concurrents we've already updated.
  fixedarray_t<u64> bitlist_concurrents;
  {
    auto quo = concurrents_len / 64;
    auto rem = concurrents_len % 64;
    auto nchunks = quo + ( rem > 0 );
    Alloc( bitlist_concurrents, nchunks );
    For( i, 0, nchunks ) {
      bitlist_concurrents.mem[i] = 0;
    }
  }

  For( i, 0, total_len ) {

    if( nreordered >= total_len ) {
      break;
    }

    // skip cycles we've already traversed.
    auto bit = bitlist.mem[ i / 64 ] & ( 1ULL << ( i % 64 ) );
    if( bit ) {
      continue;
    }

    auto s = b_idx + i;
    auto d = s - a_len - m_len;

    idx_t t0_diff_idx = s;
    diff_t t0 = buf.diffs.mem[ s ];
    _AllocAndFillLnStarts( t0 );

    idx_t t1_diff_idx;
    diff_t t1;
    Forever {
      auto bit_idx = d - a_idx;
      // detect cycle, and terminate cycle traversal.
      if( bitlist.mem[ bit_idx / 64 ] & ( 1ULL << ( bit_idx % 64 ) ) ) {
        break;
      }
      // mark dst as traversed.
      bitlist.mem[ bit_idx / 64 ] |= ( 1ULL << ( bit_idx % 64 ) );

      t1_diff_idx = d;
      t1 = buf.diffs.mem[ d ];
      _AllocAndFillLnStarts( t1 );
      ForwardDiffOper(
        buf,
        t1_diff_idx,
        buf.diffs.mem + t1_diff_idx,
        &t0,
        undoableopertype_t::mod
        );

      // update concurrents pointing at t0 to point to t1, since that's where we've shoved the content.
      For( j, 0, concurrents_len ) {
        // skip concurrents we've already updated.
        if( bitlist_concurrents.mem[ j / 64 ] & ( 1ULL << ( j % 64 ) ) ) {
          continue;
        }
        auto concurrent = concurrents[j];
        if( concurrent->diff_idx == t0_diff_idx ) {
          concurrent->diff_idx = t1_diff_idx;
          bitlist_concurrents.mem[ j / 64 ] |= ( 1ULL << ( j % 64 ) );
        }
      }

      nreordered += 1;
      if( nreordered >= total_len ) {
        break;
      }

      if( d >= b_idx ) {
        d -= m_len + a_len;
      } elif( d >= m_idx ) {
        d += b_len - a_len;
      } else {
        d += m_len + b_len;
      }

      bit_idx = d - a_idx;
      // detect cycle, and terminate cycle traversal.
      if( bitlist.mem[ bit_idx / 64 ] & ( 1ULL << ( bit_idx % 64 ) ) ) {
        break;
      }
      // mark dst as traversed.
      bitlist.mem[ bit_idx / 64 ] |= ( 1ULL << ( bit_idx % 64 ) );

      t0_diff_idx = d;
      t0 = buf.diffs.mem[ d ];
      _AllocAndFillLnStarts( t0 );
      ForwardDiffOper(
        buf,
        t0_diff_idx,
        buf.diffs.mem + t0_diff_idx,
        &t1,
        undoableopertype_t::mod
        );

      // update concurrents pointing at t1 to point to t0, since that's where we've shoved the content.
      For( j, 0, concurrents_len ) {
        // skip concurrents we've already updated.
        if( bitlist_concurrents.mem[ j / 64 ] & ( 1ULL << ( j % 64 ) ) ) {
          continue;
        }
        auto concurrent = concurrents[j];
        if( concurrent->diff_idx == t1_diff_idx ) {
          concurrent->diff_idx = t0_diff_idx;
          bitlist_concurrents.mem[ j / 64 ] |= ( 1ULL << ( j % 64 ) );
        }
      }

      nreordered += 1;
      if( nreordered >= total_len ) {
        break;
      }

      if( d >= b_idx ) {
        d -= m_len + a_len;
      } elif( d >= m_idx ) {
        d += b_len - a_len;
      } else {
        d += m_len + b_len;
      }
    }
  }

  Free( bitlist_concurrents );
  Free( bitlist );
}


// =================================================================================
// CONTENT MODIFY UNDO CALLS
//

void
UndoCheckpt( buf_t& buf )
{
  undoableoper_t checkpt = { undoableopertype_t::checkpt };
  AddHistorical( buf, checkpt );
}


void
Undo( buf_t& buf )
{
  AssertCrash( buf.history_idx <= buf.history.len );
  AssertCrash( buf.history_idx == buf.history.len  ||  buf.history.mem[buf.history_idx].type == undoableopertype_t::checkpt );
  if( !buf.history_idx ) {
    return;
  }

  bool loop = 1;
  while( loop ) {
    buf.history_idx -= 1;
    auto oper = buf.history.mem + buf.history_idx;

    // undo this diff operation:
    switch( oper->type ) {
      case undoableopertype_t::checkpt: {
        loop = 0;
      } break;
      case undoableopertype_t::add: {
        DiffRemove( buf, oper->idx, &oper->diff_new );
      } break;
      case undoableopertype_t::mod: {
        DiffReplace( buf, oper->idx, &oper->diff_new, &oper->diff_old );
      } break;
      case undoableopertype_t::rem: {
        DiffInsert( buf, oper->idx, &oper->diff_old );
      } break;
      default: UnreachableCrash();
    }
  }

  AssertCrash( buf.history_idx == buf.history.len  ||  buf.history.mem[buf.history_idx].type == undoableopertype_t::checkpt );
}



void
Redo( buf_t& buf )
{
  AssertCrash( buf.history_idx <= buf.history.len );
  AssertCrash( buf.history_idx == buf.history.len  ||  buf.history.mem[buf.history_idx].type == undoableopertype_t::checkpt );
  if( buf.history_idx == buf.history.len ) {
    return;
  }

  buf.history_idx += 1;

  bool loop = 1;
  while( loop ) {
    if( buf.history_idx == buf.history.len ) {
      break;
    }
    auto oper = buf.history.mem + buf.history_idx;

    // redo this diff operation:
    switch( oper->type ) {
      case undoableopertype_t::checkpt: {
        loop = 0;
      } break;
      case undoableopertype_t::add: {
        DiffInsert( buf, oper->idx, &oper->diff_new );
      } break;
      case undoableopertype_t::mod: {
        DiffReplace( buf, oper->idx, &oper->diff_old, &oper->diff_new );
      } break;
      case undoableopertype_t::rem: {
        DiffRemove( buf, oper->idx, &oper->diff_old );
      } break;
      default: UnreachableCrash();
    }

    if( loop ) {
      buf.history_idx += 1;
    }
  }

  AssertCrash( buf.history_idx == buf.history.len  ||  buf.history.mem[buf.history_idx].type == undoableopertype_t::checkpt );
}



// =================================================================================
// CONTENT-DEPENDENT NAVIGATION CALLS
//   TODO: should we expose: pfn_move_stop_t, MoveL, MoveR? some funcs below just call those with different pfn_move_stop_t's.
//

typedef bool ( *pfn_move_stop_t )( u8 c, void* data ); // PERF: macro-ize this

#if 0
                    Inl idx_t
                    MoveR( buf_t& buf, idx_t pos, pfn_move_stop_t MoveStop, void* data )
                    {
                      u8 c;
                      Forever {
                        if( IsEOF( buf, pos ) ) {
                          break;
                        }
                        Contents( buf, pos, &c );
                        if( MoveStop( c, data ) ) {
                          break;
                        }
                        ++pos;
                      }
                      return pos;
                    }
#endif


Inl content_ptr_t
MoveR(
  buf_t& buf,
  content_ptr_t pos,
  pfn_move_stop_t MoveStop,
  void* data,
  idx_t* nchars_moved
  )
{

  // TODO: which is better?
#if 0
  auto x = pos;
  for(
    ;
    x.diff_idx < buf.diffs.len;
    x.offset_into_diff = 0, x.diff_idx += 1)
  {
    auto diff = buf.diffs.mem + x.diff_idx;

    while( x.offset_into_diff++ < diff->slice.len ) {
      u8 c = diff->slice.mem[ x.offset_into_diff ];

      if( MoveStop( c, data ) ) {
        return x;
      }
    }
  }
  return x;
#endif


#if 0
  auto x = pos;

  Forever {
    if( x.diff_idx >= buf.diffs.len ) {
      break;
    }

    auto diff = buf.diffs.mem + x.diff_idx;

    Forever {
      AssertCrash( x.offset_into_diff < diff->slice.len );
      u8 c = diff->slice.mem[ x.offset_into_diff ];

      if( MoveStop( c, data ) ) {
        return x;
      }

      x.offset_into_diff += 1;
      if( x.offset_into_diff == diff->slice.len ) {
        break;
      }
    }
    x.offset_into_diff = 0;
    x.diff_idx += 1;
  }
  return x;
#endif


#if 1
  idx_t nmoved = 0;
  auto x = pos;
  Forever {
    if( IsEOF( buf, x ) ) {
      break;
    }

    auto after_x = CursorCharR( buf, x, 1, 0 );

    AssertCrash( x.diff_idx < buf.diffs.len );
    auto diff = buf.diffs.mem + x.diff_idx;
    AssertCrash( x.offset_into_diff < diff->slice.len );
    u8 c = diff->slice.mem[ x.offset_into_diff ];

    if( MoveStop( c, data ) ) {
      break;
    }

    x = after_x;
    nmoved += 1;
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return x;
#endif

}









#if 0
                      Inl idx_t
                      MoveL( buf_t& buf, idx_t pos, pfn_move_stop_t MoveStop, void* data )
                      {
                        u8 c;
                        Forever {
                          if( IsBOF( buf, pos ) ) {
                            break;
                          }
                          --pos;
                          Contents( buf, pos, &c );
                          if( MoveStop( c, data ) ) {
                            ++pos;
                            break;
                          }
                        }
                        return pos;
                      }
#endif


#if 0
  ReverseFori( idx_t, diff_idx, 0, MIN( pos.diff_idx + 1, buf.diffs.len ) ) {
    auto diff = buf.diffs.mem + diff_idx;
    ReverseFori( idx_t, offset_into_diff, 0, diff->slice.len ) {

    }
  }

  auto x = pos;
  Forever {
    if( IsBOF( buf, x ) ) {
      break;
    }
  }


  auto x = pos;
  for(
    ;
    !IsBOF( buf, x );
    x.offset_into_diff = 0, x.diff_idx -= 1)
  {
    auto diff = buf.diffs.mem + x.diff_idx;

    while( x.offset_into_diff++ < diff->slice.len ) {
      u8 c = diff->slice.mem[ x.offset_into_diff ];

      if( MoveStop( c, data ) ) {
        return x;
      }
    }
  }
  return x;
#endif


Inl content_ptr_t
MoveL(
  buf_t& buf,
  content_ptr_t pos,
  pfn_move_stop_t MoveStop,
  void* data,
  idx_t* nchars_moved
  )
{

  // TODO: which is better?

#if 0
  auto x = pos;
  AssertCrash( x.diff_idx < buf.diffs.len );

  Forever {
    auto diff = buf.diffs.mem + x.diff_idx;

    AssertCrash( x.offset_into_diff < diff->slice.len );
    u8 c = diff->slice.mem[ x.offset_into_diff ];

    if( MoveStop( c, data ) ) {
      x = CursorCharR( buf, x, 1 );
      return x;
    }

    if( !x.diff_idx ) {
      break;
    }

    x.diff_idx -= 1;
    auto diff = buf.diffs.mem + x.diff_idx;
    AssertCrash( diff->slice.len );
    x.offset_into_diff = diff->slice.len - 1;
  }
  return x;
#endif


#if 1
  idx_t nmoved = 0;
  auto x = pos;
  Forever {
    if( IsBOF( buf, x ) ) {
      break;
    }
    auto before_x = CursorCharL( buf, x, 1, 0 );
    auto after_x = x;
    x = before_x;

    AssertCrash( x.diff_idx < buf.diffs.len );
    auto diff = buf.diffs.mem + x.diff_idx;
    AssertCrash( x.offset_into_diff < diff->slice.len );
    u8 c = diff->slice.mem[ x.offset_into_diff ];

    if( MoveStop( c, data ) ) {
      x = after_x; // setup retval
      break;
    }

    nmoved += 1;
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return x;
#endif

}










Inl bool
MoveStopAtNonWordChar( u8 c, void* data )
{
  return !InWord( c );
}

content_ptr_t
CursorStopAtNonWordCharL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveL( buf, pos, MoveStopAtNonWordChar, 0, nchars_moved );
}

content_ptr_t
CursorStopAtNonWordCharR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveR( buf, pos, MoveStopAtNonWordChar, 0, nchars_moved );
}



Inl bool
MoveStopAtNewline( u8 c, void* data )
{
  return ( c == '\r' )  |  ( c == '\n' );
}

content_ptr_t
CursorStopAtNewlineL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveL( buf, pos, MoveStopAtNewline, 0, nchars_moved );
}

content_ptr_t
CursorStopAtNewlineR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveR( buf, pos, MoveStopAtNewline, 0, nchars_moved );
}



content_ptr_t
CursorCharInlineL( buf_t& buf, content_ptr_t pos, idx_t len, idx_t* nchars_moved )
{
  idx_t nmoved0;
  idx_t nmoved1;
  auto l0 = CursorCharL( buf, pos, len, &nmoved0 );
  auto l1 = CursorStopAtNewlineL( buf, pos, &nmoved1 );
  pos = Max( l0, l1 );
  if( nchars_moved ) {
    *nchars_moved = MAX( nmoved0, nmoved1 );
  }
  return pos;
}

content_ptr_t
CursorCharInlineR( buf_t& buf, content_ptr_t pos, idx_t len, idx_t* nchars_moved )
{
  idx_t nmoved0;
  idx_t nmoved1;
  auto r0 = CursorCharR( buf, pos, len, &nmoved0 );
  auto r1 = CursorStopAtNewlineR( buf, pos, &nmoved1 );
  pos = Min( r0, r1 );
  if( nchars_moved ) {
    *nchars_moved = MIN( nmoved0, nmoved1 );
  }
  return pos;
}



Inl bool
MoveSkipSpacetab( u8 c, void* data )
{
  return ( c != ' '  )  &  ( c != '\t' );
}

content_ptr_t
CursorSkipSpacetabL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveL( buf, pos, MoveSkipSpacetab, 0, nchars_moved );
}

content_ptr_t
CursorSkipSpacetabR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveR( buf, pos, MoveSkipSpacetab, 0, nchars_moved );
}



Inl bool
MoveStopAtSpacetab( u8 c, void* data )
{
  return ( c == ' '  )  ||  ( c == '\t' );
}

content_ptr_t
CursorStopAtSpacetabL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveL( buf, pos, MoveStopAtSpacetab, 0, nchars_moved );
}

content_ptr_t
CursorStopAtSpacetabR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  return MoveR( buf, pos, MoveStopAtSpacetab, 0, nchars_moved );
}


Inl bool
MoveStopAtBeforeNewlineL( u8 c, void* data )
{
  idx_t* state = Cast( idx_t*, data );
  if( *state == 2 ) {
    return 1;
  } elif( *state == 1 ) {
    if( c == '\r' ) {
      *state = 2;
      return 0;
    } else {
      return 1;
    }
  } else {
    if( c == '\n' ) {
      *state = 1;
      return 0;
    } elif( c == '\r' ) {
      *state = 2;
      return 0;
    } else {
      return 1;
    }
  }
}

content_ptr_t
CursorSingleNewlineL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  idx_t state = 0;
  return MoveL( buf, pos, MoveStopAtBeforeNewlineL, &state, nchars_moved );
}



Inl bool
MoveStopAtAfterNewlineR( u8 c, void* data )
{
  idx_t* state = Cast( idx_t*, data );
  if( *state == 2 ) {
    if( c == '\n' ) {
      *state = 1;
      return 0;
    } else {
      return 1;
    }
  } elif( *state == 1 ) {
    return 1;
  } else {
    if( c == '\n' ) {
      *state = 1;
      return 0;
    } elif( c == '\r' ) {
      *state = 2;
      return 0;
    } else {
      return 1;
    }
  }
}

content_ptr_t
CursorSingleNewlineR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  idx_t state = 0;
  return MoveR( buf, pos, MoveStopAtAfterNewlineR, &state, nchars_moved );
}


content_ptr_t
CursorSkipCharNewlineL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved ) // TODO: rename!
{
  idx_t nmoved;
  idx_t nmoved0;
  idx_t nmoved1;
  auto l0 = CursorCharL( buf, pos, 1, &nmoved0 );
  auto l1 = CursorSingleNewlineL( buf, pos, &nmoved1 );
  if( Equal( pos, l0 ) ) {
    pos = l1;
    nmoved = nmoved1;
  } else {
    if( Equal( pos, l1 ) ) {
      pos = l0;
      nmoved = nmoved0;
    } else {
      pos = Min( l0, l1 );
      nmoved = MIN( nmoved0, nmoved1 );
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

content_ptr_t
CursorSkipCharNewlineR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved ) // TODO: rename!
{
  idx_t nmoved;
  idx_t nmoved0;
  idx_t nmoved1;
  auto r0 = CursorCharR( buf, pos, 1, &nmoved0 );
  auto r1 = CursorSingleNewlineR( buf, pos, &nmoved1 );
  if( Equal( pos, r0 ) ) {
    pos = r1;
    nmoved = nmoved1;
  } else {
    if( Equal( pos, r1 ) ) {
      pos = r0;
      nmoved = nmoved0;
    } else {
      pos = Max( r0, r1 );
      nmoved = MAX( nmoved0, nmoved1 );
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}



content_ptr_t
CursorSkipWordSpacetabNewlineL( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved ) // TODO: rename!
{
  idx_t nmoved;
  idx_t nmoved0;
  idx_t nmoved1;
  idx_t nmoved2;
  auto l0 = CursorSkipCharNewlineL( buf, pos, &nmoved0 );
  auto l1 = CursorStopAtNonWordCharL( buf, pos, &nmoved1 );
  auto l2 = CursorSkipSpacetabL( buf, pos, &nmoved2 );
  // pick min of { l0, l1, l2 } s.t. pos != min.
  if( Equal( pos, l0 ) ) {
    if( Equal( pos, l1 ) ) {
      pos = l2;
      nmoved = nmoved2;
    } else {
      if( Equal( pos, l2 ) ) {
        pos = l1;
        nmoved = nmoved1;
      } else {
        pos = Min( l1, l2 );
        nmoved = MIN( nmoved1, nmoved2 );
      }
    }
  } else {
    if( Equal( pos, l1 ) ) {
      if( Equal( pos, l2 ) ) {
        pos = l0;
        nmoved = nmoved0;
      } else {
        pos = Min( l0, l2 );
        nmoved = MIN( nmoved0, nmoved2 );
      }
    } else {
      if( Equal( pos, l2 ) ) {
        pos = Min( l0, l1 );
        nmoved = MIN( nmoved0, nmoved1 );
      } else {
        pos = Min3( l0, l1, l2 );
        nmoved = MIN3( nmoved0, nmoved1, nmoved2 );
      }
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

content_ptr_t
CursorSkipWordSpacetabNewlineR( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved ) // TODO: rename!
{
  idx_t nmoved;
  idx_t nmoved0;
  idx_t nmoved1;
  idx_t nmoved2;
  auto r0 = CursorSkipCharNewlineR( buf, pos, &nmoved0 );
  auto r1 = CursorStopAtNonWordCharR( buf, pos, &nmoved1 );
  auto r2 = CursorSkipSpacetabR( buf, pos, &nmoved2 );
  // pick max of { r0, r1, r2 } s.t. pos != max.
  if( Equal( pos, r0 ) ) {
    if( Equal( pos, r1 ) ) {
      pos = r2;
      nmoved = nmoved2;
    } else {
      if( Equal( pos, r2 ) ) {
        pos = r1;
        nmoved = nmoved1;
      } else {
        pos = Max( r1, r2 );
        nmoved = MAX( nmoved1, nmoved2 );
      }
    }
  } else {
    if( Equal( pos, r1 ) ) {
      if( Equal( pos, r2 ) ) {
        pos = r0;
        nmoved = nmoved0;
      } else {
        pos = Max( r0, r2 );
        nmoved = MAX( nmoved0, nmoved2 );
      }
    } else {
      if( Equal( pos, r2 ) ) {
        pos = Max( r0, r1 );
        nmoved = MAX( nmoved0, nmoved1 );
      } else {
        pos = Max3( r0, r1, r2 );
        nmoved = MAX3( nmoved0, nmoved1, nmoved2 );
      }
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}



content_ptr_t
CursorHome( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  idx_t nmoved;
  idx_t nmoved0;
  idx_t nmoved1;
  auto line = CursorStopAtNewlineL( buf, pos, &nmoved0 );
  auto whitespace = CursorSkipSpacetabR( buf, line, &nmoved1 );
  if( Equal( pos, whitespace ) ) {
    pos = line;
    nmoved = nmoved0;
  } else {
    pos = whitespace;
    nmoved = nmoved1;
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

content_ptr_t
CursorEnd( buf_t& buf, content_ptr_t pos, idx_t* nchars_moved )
{
  idx_t nmoved;
  idx_t nmoved0;
  idx_t nmoved1;
  auto line = CursorStopAtNewlineR( buf, pos, &nmoved0 );
  if( Equal( pos, line ) ) {
    pos = CursorSkipSpacetabL( buf, pos, &nmoved1 );
    nmoved =
      ( nmoved0 <= nmoved1 )  ?
        nmoved1 - nmoved0  :
        nmoved0 - nmoved1;
  } else {
    pos = line;
    nmoved = nmoved0;
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}



Inl content_ptr_t
CursorLineU(
  buf_t& buf,
  content_ptr_t pos,
  idx_t pos_inline,
  idx_t* nchars_moved
  )
{
  ProfFunc();

  AssertCrash( !nchars_moved ); // TODO: implement
  idx_t nmoved = 0;

  auto line0_start = CursorStopAtNewlineL( buf, pos, 0 );
  auto line1_end = CursorSingleNewlineL( buf, line0_start, 0 );
  if( !Equal( line0_start, line1_end ) ) {
    auto line1_start = CursorStopAtNewlineL( buf, line1_end, 0 );
    auto inl = CursorCharR( buf, line1_start, pos_inline, 0 );
    pos = Min( line1_end, inl );
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

Inl content_ptr_t
CursorLineD(
  buf_t& buf,
  content_ptr_t pos,
  idx_t pos_inline,
  idx_t* nchars_moved
  )
{
  ProfFunc();

  AssertCrash( !nchars_moved ); // TODO: implement
  idx_t nmoved = 0;

  auto line0_end = CursorStopAtNewlineR( buf, pos, 0 );
  auto line1_start = CursorSingleNewlineR( buf, line0_end, 0 );
  if( !Equal( line0_end, line1_start ) ) {
    auto line1_end = CursorStopAtNewlineR( buf, line1_start, 0 );
    auto inl = CursorCharR( buf, line1_start, pos_inline, 0 );
    pos = Min( line1_end, inl );
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

content_ptr_t
CursorLineU(
  buf_t& buf,
  content_ptr_t pos,
  idx_t pos_inline,
  idx_t nlines,
  idx_t* dlines,
  idx_t* nchars_moved
  )
{
  ProfFunc();

  AssertCrash( !nchars_moved ); // TODO: implement
  idx_t nmoved = 0;

  if( dlines ) {
    *dlines = 0;
  }
  while( nlines-- ) {
    auto pre = pos;
    pos = CursorLineU( buf, pos, pos_inline, 0 );
    auto post = pos;
    if( Equal( pre, post ) ) {
      break;
    } else {
      if( dlines ) {
        *dlines += 1;
      }
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}

content_ptr_t
CursorLineD(
  buf_t& buf,
  content_ptr_t pos,
  idx_t pos_inline,
  idx_t nlines,
  idx_t* dlines,
  idx_t* nchars_moved
  )
{
  ProfFunc();

  AssertCrash( !nchars_moved ); // TODO: implement
  idx_t nmoved = 0;

  if( dlines ) {
    *dlines = 0;
  }
  while( nlines-- ) {
    auto pre = pos;
    pos = CursorLineD( buf, pos, pos_inline, 0 );
    auto post = pos;
    if( Equal( pre, post ) ) {
      break;
    } else {
      if( dlines ) {
        *dlines += 1;
      }
    }
  }
  if( nchars_moved ) {
    *nchars_moved = nmoved;
  }
  return pos;
}


Inl idx_t
CountLinesBetween(
  buf_t& buf,
  content_ptr_t ln_start0,
  content_ptr_t ln_start1
  )
{
  ProfFunc();

  // TODO: is this necessary?
  // Must pass in pos at start of line!
  auto test0 = CursorStopAtNewlineL( buf, ln_start0, 0 );
  auto test1 = CursorStopAtNewlineL( buf, ln_start1, 0 );
  AssertCrash( Equal( ln_start0, test0 ) );
  AssertCrash( Equal( ln_start1, test1 ) );

  auto yl = ln_start0;
  auto yr = ln_start1;

  idx_t count = 0;
  auto y = yr;
  while( Greater( y, yl ) ) {
    y = CursorLineU( buf, y, 0, 1, 0, 0 );
    count += 1;
  }

  return count;
}

Inl idx_t
CountBytesBetween(
  buf_t& buf,
  content_ptr_t a,
  content_ptr_t b
  )
{
  AssertCrash( LEqual( a, b ) );

  if( a.diff_idx == b.diff_idx ) {
    idx_t count = b.offset_into_diff - a.offset_into_diff;
    return count;
  }

  idx_t count = 0;
  AssertCrash( a.diff_idx < buf.diffs.len );
  auto diff_a = buf.diffs.mem + a.diff_idx;
  count += diff_a->slice.len - a.offset_into_diff;

  For( i, a.diff_idx + 1, b.diff_idx ) {
    AssertCrash( i < buf.diffs.len );
    auto diff = buf.diffs.mem + i;
    count += diff->slice.len;
  }

  if( b.diff_idx < buf.diffs.len ) {
    count += b.offset_into_diff;
  }
  return count;
}

Inl idx_t
CountCharsBetween(
  buf_t& buf,
  content_ptr_t a,
  content_ptr_t b
  )
{
  // TODO: wrong for UTF8!
  return CountBytesBetween( buf, a, b );
}



Inl string_t
AllocContents( buf_t& buf, content_ptr_t start, content_ptr_t end )
{
  string_t r;
  auto len = CountBytesBetween( buf, start, end );
  Alloc( r, len );
  Contents( buf, start, r.mem, len );
  return r;
}

Inl string_t
AllocContents( buf_t& buf, content_ptr_t ptr, idx_t len )
{
  string_t r;
  Alloc( r, len );
  Contents( buf, ptr, r.mem, len );
  return r;
}

Inl string_t
AllocContents( buf_t& buf )
{
  if( !buf.diffs.len ) {
    string_t r = {};
    return r;
  }
  content_ptr_t ptr = GetBOF( buf );
  return AllocContents( buf, ptr, buf.content_len );
}



struct
scrollpos_t
{
  content_ptr_t y; // pos at line start.
  f64 frac; // fractional line offset, in units of lines.
};


Enumc( seltype_t )
{
  none,
  s,
  m,
};


// if you change this struct, be sure to change: CsUndoFromTxt, ApplyCsUndo, cs_undo_absolute_t
// this also must match the set of things in __TxtConcurrentPtrs, so that our txt undo/redo restores all
// content ptrs to valid things.
struct
cs_undo_t
{
  content_ptr_t c;
  content_ptr_t s;
  idx_t c_inline;
  bool overwrite;
  seltype_t seltype;
  scrollpos_t scroll_target;
  scrollpos_t scroll_start;
  content_ptr_t scroll_end;
};

Inl bool
Equal( cs_undo_t& a, cs_undo_t& b )
{
  bool r = 1;
  r = r  &&  Equal( a.c, b.c );
  r = r  &&  Equal( a.s, b.s );
  r = r  &&  ( a.c_inline == b.c_inline );
  r = r  &&  ( a.overwrite == b.overwrite );
  r = r  &&  ( a.seltype == b.seltype );
  r = r  &&  Equal( a.scroll_target.y, b.scroll_target.y );
  r = r  &&  ( ABS( a.scroll_target.frac - b.scroll_target.frac ) < 0.2 );
  r = r  &&  Equal( a.scroll_start.y, b.scroll_start.y );
  r = r  &&  ( ABS( a.scroll_start.frac - b.scroll_start.frac ) < 0.2 );
  r = r  &&  Equal( a.scroll_end, b.scroll_end );
  return r;
}

struct
cs_undo_absolute_t
{
  idx_t c;
  idx_t s;
  idx_t c_inline;
  bool overwrite;
  seltype_t seltype;
  idx_t scroll_target_y;
  f64   scroll_target_frac;
  idx_t scroll_start_y;
  f64   scroll_start_frac;
  idx_t scroll_end;
};

Inl bool
Equal( cs_undo_absolute_t& a, cs_undo_absolute_t& b )
{
  bool r = 1;
  r = r  &&  ( a.c == b.c );
  r = r  &&  ( a.s == b.s );
  r = r  &&  ( a.c_inline == b.c_inline );
  r = r  &&  ( a.overwrite == b.overwrite );
  r = r  &&  ( a.seltype == b.seltype );
  r = r  &&  ( a.scroll_target_y == b.scroll_target_y );
  r = r  &&  ( ABS( a.scroll_target_frac - b.scroll_target_frac ) < 0.2 );
  r = r  &&  ( a.scroll_start_y == b.scroll_start_y );
  r = r  &&  ( ABS( a.scroll_start_frac - b.scroll_start_frac ) < 0.2 );
  r = r  &&  ( a.scroll_end == b.scroll_end );
  return r;
}



struct
txt_dblclick_t
{
  bool first_made;
  idx_t first_cursor;
  u64 first_clock;
};


struct
txt_t
{
  fsobj_t filename;
  buf_t buf;
  content_ptr_t c; // cursor position.
  content_ptr_t s; // select anchor.
  idx_t c_inline; // stored so cursor / select u,d will try to stay on same ch.
  bool overwrite;
  seltype_t seltype;
  scrollpos_t scroll_start; // this is the line-start of the FIRST line of the CURRENT scroll view.
  scrollpos_t scroll_target; // this is the line-start of the CENTER line of the TARGET scroll view.
  content_ptr_t scroll_end; // this is the line-start of the LAST line of the CURRENT scroll view.
  f64 scroll_vel;
  f32 scroll_x;
  u16 window_n_lines;
  array_t<cs_undo_t> undos_txtcs; // used in CmdTxtUndo/Redo. stores cursel state prior to each change.
  array_t<cs_undo_t> redos_txtcs; // no need to update as concurrents, since these are locked into txt undo, which always restores previous diff state.
  array_t<cs_undo_absolute_t> undos; // used in CmdCsUndo/Redo. stores cursel state prior to each change.
  array_t<cs_undo_absolute_t> redos; // we get away with not updating these as concurrents, since they're absolute indices.
  txt_dblclick_t dblclick;
  fontlayout_t layout; // created on every EditRender call. sent to Glw for rendering. used by EditControlMouse for screen -> cursel mapping.
  bool insert_spaces_for_tabs;
  u8 spaces_per_tab;
};

// all of the content_ptr_t's that must be updated as part of forward editing operations.
// must also match the set of things in cs_undo_t
#define __TxtConcurrentPtrs( txt ) \
  &( txt ).c, \
  &( txt ).s, \
  &( txt ).scroll_start.y, \
  &( txt ).scroll_target.y, \
  &( txt ).scroll_end \




#define __TxtCmd( name )   void ( name )( txt_t& txt, idx_t misc = 0, idx_t misc2 = 0 )
#define __TxtCmdDef( name )   void ( name )( txt_t& txt, idx_t misc, idx_t misc2 )
typedef __TxtCmdDef( *pfn_txtcmd_t );








// =================================================================================
// FIRST / LAST CALLS
//

void
Init( txt_t& txt )
{
  txt.filename.mem[0] = 0;
  txt.filename.len = 0;
  Init( txt.buf );
  txt.c = {};
  txt.s = {};
  txt.c_inline = 0;
  txt.overwrite = 0;
  txt.seltype = seltype_t::none;
  txt.scroll_start = {};
  txt.scroll_target = {};
  txt.scroll_end = {};
  txt.scroll_vel = 0;
  txt.scroll_x = 0;
  txt.window_n_lines = 0;
  Alloc( txt.undos_txtcs, 16 );
  Alloc( txt.redos_txtcs, 16 );
  Alloc( txt.undos, 16 );
  Alloc( txt.redos, 16 );
  txt.dblclick.first_made = 0;
  txt.dblclick.first_cursor = 0;
  txt.dblclick.first_clock = 0;
  FontInit( txt.layout );
}

void
Kill( txt_t& txt )
{
  txt.filename.mem[0] = 0;
  txt.filename.len = 0;
  Kill( txt.buf );
  Free( txt.undos_txtcs );
  Free( txt.redos_txtcs );
  Free( txt.undos );
  Free( txt.redos );
  txt.dblclick.first_made = 0;
  txt.dblclick.first_cursor = 0;
  txt.dblclick.first_clock = 0;
  FontKill( txt.layout );
}


void
TxtLoadEmpty( txt_t& txt )
{
  txt.filename.mem[0] = 0;
  txt.filename.len = 0;
  BufLoadEmpty( txt.buf );

  txt.spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );
  txt.insert_spaces_for_tabs = GetPropFromDb( bool, bool_insert_spaces_for_tabs );
}

void
TxtLoad( txt_t& txt, file_t& file )
{
  ProfFunc();
  Memmove( txt.filename.mem, ML( file.obj ) );
  txt.filename.len = file.obj.len;

  BufLoad( txt.buf, file );

  auto default_spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );
  auto default_insert_spaces_for_tabs = GetPropFromDb( bool, bool_insert_spaces_for_tabs );

  // analyze leading spaces and tabs to figure out what settings to use for this file.

  static const idx_t c_lines = 200;
  array_t<vec2<idx_t>> spacetabs;
  Alloc( spacetabs, c_lines );

#if 0
  idx_t pos = 0;
  auto lines = c_lines;
  while( lines ) {
    auto bol = pos;
    auto len = CursorSkipSpacetabR( txt.buf, bol ) - bol;
    if( len ) {
      lines -= 1;
      auto tmp = AllocContents( txt.buf, bol, len );
      auto spacetab = AddBack( spacetabs );
      *spacetab = _vec2<idx_t>( 0, 0 );
      For( i, 0, len ) {
        if( tmp.mem[i] == ' ' ) {
          spacetab->x += 1;
        } elif( tmp.mem[i] == '\t' ) {
          spacetab->y += 1;
        } else {
          UnreachableCrash();
        }
      }
      Free( tmp );
    }
    auto nextline = CursorLineD( txt.buf, pos, 0, 0 );
    if( nextline == pos ) {
      break;
    }
    pos = nextline;
  }
#endif

  auto uniform_pos = GetBOF( txt.buf );
  For( i, 0, c_lines ) {
    auto bol = CursorStopAtNewlineL( txt.buf, uniform_pos, 0 );
    idx_t len;
    auto bol_space = CursorSkipSpacetabR( txt.buf, bol, &len );
    if( len ) {
      auto tmp = AllocContents( txt.buf, bol, len );
      auto spacetab = AddBack( spacetabs );
      *spacetab = _vec2<idx_t>( 0, 0 );
      For( j, 0, len ) {
        if( tmp.mem[j] == ' ' ) {
          spacetab->x += 1;
        } elif( tmp.mem[j] == '\t' ) {
          spacetab->y += 1;
        } else {
          UnreachableCrash();
        }
      }
      Free( tmp );
    }

    uniform_pos = CursorCharR(
      txt.buf,
      uniform_pos,
      Round_idx_from_f32( Cast( f32, txt.buf.content_len ) / c_lines ),
      0
      );
  }

  bool enough_samples = ( spacetabs.len >= 4 );
  idx_t linecount_mixed = 0;
  idx_t linecount_allspaces = 0;
  idx_t linecount_alltabs = 0;
  ForLen( i, spacetabs ) {
    auto spacetab = spacetabs.mem + i;
    if( spacetab->x  &&  spacetab->y ) {
      linecount_mixed += 1;
    } elif( spacetab->x ) {
      linecount_allspaces += 1;
    } elif( spacetab->y ) {
      linecount_alltabs += 1;
      // remove the alltab line.
      *spacetab = spacetabs.mem[ spacetabs.len - 1 ];
      spacetabs.len -= 1;
      i -= 1;
    }
  }
  if( !enough_samples ) {
    txt.insert_spaces_for_tabs = default_insert_spaces_for_tabs;
    txt.spaces_per_tab = default_spaces_per_tab;
  } elif( c_lines * linecount_allspaces <= 5 * linecount_alltabs ) {
    txt.insert_spaces_for_tabs = 0;
    txt.spaces_per_tab = default_spaces_per_tab;
  } elif( 10 * linecount_mixed >= c_lines ) {
    txt.insert_spaces_for_tabs = default_insert_spaces_for_tabs;
    txt.spaces_per_tab = default_spaces_per_tab;
  } else {
    txt.insert_spaces_for_tabs = 1;
    // compute a fuzzy greatest-common-denominator for { spacetabs->x }
    f32 scores[16]; // 16 spaces_per_tab should be enough for anyone.
    f32 score_max = 0;
    ReverseFor( i, 1, _countof( scores ) ) { // exclude 1 spaces_per_tab, since that would always score highest.
      scores[i] = 0;
      auto rec_spaces_per_tab = 1.0f / Cast( f32, i + 1 );
      ForLen( j, spacetabs ) {
        auto spacetab = spacetabs.mem + j;
        auto divtest = Cast( f32, spacetab->x ) * rec_spaces_per_tab;
        scores[i] += Pow32( Frac32( divtest ), rec_spaces_per_tab );
      }
      score_max = MAX( score_max, scores[i] );
    }
    AssertWarn( score_max > 0 );
    For( i, 1, _countof( scores ) ) {
      scores[i] /= score_max;
    }
    bool found = 0;
    constant auto score_threshold = 0.15f;
    ReverseFor( i, 1, _countof( scores ) ) {
      if( scores[i] <= score_threshold ) {
        txt.spaces_per_tab = Cast( u8, i + 1 );
        found = 1;
        break;
      }
    }
    if( !found ) {
      txt.spaces_per_tab = default_spaces_per_tab;
    }
  }

  Free( spacetabs );
}

void
TxtSave( txt_t& txt, file_t& file )
{
  BufSave( txt.buf, file );
}

#define TxtLen( txt ) \
  ( ( txt ).buf.content_len )


// =================================================================================
// CURSOR / SELECT NAVIGATION
//

Inl idx_t
GetInline( txt_t& txt, content_ptr_t pos )
{
  idx_t pos_inline;
  CursorStopAtNewlineL( txt.buf, pos, &pos_inline );
  return pos_inline;
}



Inl void
ResetCInline( txt_t& txt )
{
  txt.c_inline = GetInline( txt, txt.c );
}



Inl void
CsUndoAbsoluteFromTxt( txt_t& txt, cs_undo_absolute_t* undo )
{
  auto bof = GetBOF( txt.buf );

  undo->c = CountCharsBetween( txt.buf, bof, txt.c );
  undo->s = CountCharsBetween( txt.buf, bof, txt.s );
  undo->c_inline = txt.c_inline;
  undo->overwrite = txt.overwrite;
  undo->seltype = txt.seltype;
  undo->scroll_target_y    = CountCharsBetween( txt.buf, bof, txt.scroll_target.y );
  undo->scroll_target_frac = txt.scroll_target.frac;
  undo->scroll_start_y    = CountCharsBetween( txt.buf, bof, txt.scroll_start.y );
  undo->scroll_start_frac = txt.scroll_start.frac;
  undo->scroll_end = CountCharsBetween( txt.buf, bof, txt.scroll_end );
}

Inl void
ApplyCsUndoAbsolute( txt_t& txt, cs_undo_absolute_t& undo )
{
  auto bof = GetBOF( txt.buf );
  auto eof = GetEOF( txt.buf );

  txt.c = Min( CursorCharR( txt.buf, bof, undo.c, 0 ), eof );
  txt.s = Min( CursorCharR( txt.buf, bof, undo.s, 0 ), eof );
  txt.c_inline = undo.c_inline;
  txt.overwrite = undo.overwrite;
  txt.seltype = undo.seltype;
  txt.scroll_target.frac = undo.scroll_target_frac;
  txt.scroll_target.y = Min( CursorCharR( txt.buf, bof, undo.scroll_target_y, 0 ), eof );
  txt.scroll_start.frac = undo.scroll_start_frac;
  txt.scroll_start.y = Min( CursorCharR( txt.buf, bof, undo.scroll_start_y, 0 ), eof );
  txt.scroll_end = Min( CursorCharR( txt.buf, bof, undo.scroll_end, 0 ), eof );
}

Inl void
CsUndoFromTxt( txt_t& txt, cs_undo_t* undo )
{
  undo->c = txt.c;
  undo->s = txt.s;
  undo->c_inline = txt.c_inline;
  undo->overwrite = txt.overwrite;
  undo->seltype = txt.seltype;
  undo->scroll_target = txt.scroll_target;
  undo->scroll_start = txt.scroll_start;
  undo->scroll_end = txt.scroll_end;
}

Inl void
ApplyCsUndo( txt_t& txt, cs_undo_t& undo )
{
  auto eof = GetEOF( txt.buf );

  txt.c = Min( undo.c, eof );
  txt.s = Min( undo.s, eof );
  txt.c_inline = undo.c_inline;
  txt.overwrite = undo.overwrite;
  txt.seltype = undo.seltype;
  txt.scroll_target.frac = undo.scroll_target.frac;
  txt.scroll_target.y = Min( undo.scroll_target.y, eof );
  txt.scroll_start.frac = undo.scroll_start.frac;
  txt.scroll_start.y = Min( undo.scroll_start.y, eof );
  txt.scroll_end = Min( undo.scroll_end, eof );
}



Inl bool
SelectIsZero( txt_t& txt )
{
  return ( txt.seltype != seltype_t::s )  |  Equal( txt.c, txt.s );
}



__TxtCmd( CmdScrollD )
{
  AssertCrash( !misc2 );

  auto delta_nlines = misc;
  txt.scroll_target.y = CursorLineD( txt.buf, txt.scroll_target.y, 0, delta_nlines, 0, 0 );
}

__TxtCmd( CmdScrollU )
{
  AssertCrash( !misc2 );

  auto delta_nlines = misc;
  txt.scroll_target.y = CursorLineU( txt.buf, txt.scroll_target.y, 0, delta_nlines, 0, 0 );
}



Inl void
MakeCursorVisible( txt_t& txt )
{
  auto c_ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );

#if 0
  // always center on the cursor:
  txt.scroll_target.y = c_ln_start;
  txt.scroll_target.frac = 0;
#else

  // We may have deleted from scroll_start to EOF, so we need to reset scroll_start so it's in-bounds.
  auto eof = GetEOF( txt.buf );
  if( Greater( txt.scroll_start.y, eof ) ) {
    txt.scroll_start.y = CursorStopAtNewlineL( txt.buf, eof, 0 );
    txt.scroll_start.frac = 0;
  }

  // We may have changed text around scroll_start, so we need to reset scroll_start to be BOL.
  txt.scroll_start.y = CursorStopAtNewlineL( txt.buf, txt.scroll_start.y, 0 );

  auto nlines = Cast( idx_t, txt.scroll_start.frac + 0.5 * txt.window_n_lines );
  auto scroll_half = CursorLineD( txt.buf, txt.scroll_start.y, 0, nlines, 0, 0 );

  idx_t half_scrollwin = Cast( idx_t, 0.225f * txt.window_n_lines );
  auto yl = CursorLineU( txt.buf, scroll_half, 0, half_scrollwin, 0, 0 );
  auto yr = CursorLineD( txt.buf, scroll_half, 0, half_scrollwin, 0, 0 );

  if( Less( c_ln_start, yl ) ) {
    idx_t delta_y = CountLinesBetween( txt.buf, c_ln_start, yl );
    scroll_half = CursorLineU( txt.buf, scroll_half, 0, delta_y, 0, 0 );
  } elif( Greater( c_ln_start, yr ) ) {
    idx_t delta_y = CountLinesBetween( txt.buf, yr, c_ln_start );
    scroll_half = CursorLineD( txt.buf, scroll_half, 0, delta_y, 0, 0 );
  }
  txt.scroll_target.y = scroll_half;
  txt.scroll_target.frac = 0;
#endif
}



Inl void
GetSelect( txt_t& txt, content_ptr_t* sl, content_ptr_t* sr )
{
  *sl = Min( txt.c, txt.s );
  *sr = Max( txt.c, txt.s );
}



Inl string_t
AllocSelection( txt_t& txt )
{
  AssertCrash( txt.seltype == seltype_t::s );
  content_ptr_t sl, sr;
  GetSelect( txt, &sl, &sr );
  return AllocContents( txt.buf, sl, sr );
}



struct
multisel_t
{
  content_ptr_t c_ln_start;
  content_ptr_t s_ln_start;
  idx_t c_inline; // always-in-bounds inline pos.
  idx_t s_inline; // always-in-bounds inline pos.
  content_ptr_t top_ln_start; // line start at multiselection top edge.
  content_ptr_t bot_ln_start; // line start at multiselection bot edge.
  idx_t left_inline; // inline pos of multiselection l edge.
  idx_t rght_inline; // inline pos of multiselection r edge.
};

#define __MultiselConcurrentPtrs( txt ) \
  &( ms ).c_ln_start, \
  &( ms ).s_ln_start, \
  &( ms ).top_ln_start, \
  &( ms ).bot_ln_start \



Inl void
GetMultiselect( txt_t& txt, multisel_t& ms )
{
  ms.c_ln_start = CursorStopAtNewlineL( txt.buf, txt.c, &ms.c_inline );
  ms.s_ln_start = CursorStopAtNewlineL( txt.buf, txt.s, &ms.s_inline );

  ms.top_ln_start = Min( ms.c_ln_start, ms.s_ln_start );
  ms.bot_ln_start = Max( ms.c_ln_start, ms.s_ln_start );

  // virtual c_inline if the txt.c_inline is past the end of txt.c's current line.
  idx_t c_inline = MAX( ms.c_inline, txt.c_inline );

  ms.left_inline = MIN( c_inline, ms.s_inline );
  ms.rght_inline = MAX( c_inline, ms.s_inline );
}



Inl void
AnchorSelect( txt_t& txt, content_ptr_t cursor )
{
  txt.s = cursor;
  txt.seltype = seltype_t::s;
}



Inl void
AddCsUndo( txt_t& txt )
{
  // invalidate previous future.
  // if this function is called, then something's changed, and it doesn't
  //  make sense to step forward in the UndoStack. so we clear the stack
  //  past the current time.
  txt.redos.len = 0;

  // dump current state onto the UndoStack.
  cs_undo_absolute_t state;
  CsUndoAbsoluteFromTxt( txt, &state );

  bool add = 1;
  if( txt.undos.len ) {
    auto last = txt.undos.mem + txt.undos.len - 1;
    add = !Equal( state, *last );
  }
  if( add ) {
    *AddBack( txt.undos ) = state;
  }
}



__TxtCmd( CmdCsUndo )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  if( !txt.undos.len ) {
    return;
  }

  cs_undo_absolute_t state;
  CsUndoAbsoluteFromTxt( txt, &state );
  *AddBack( txt.redos ) = state;

  cs_undo_absolute_t undo = txt.undos.mem[txt.undos.len - 1];
  RemBack( txt.undos );
  ApplyCsUndoAbsolute( txt, undo );
}

__TxtCmd( CmdCsRedo )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  if( !txt.redos.len ) {
    return;
  }

  cs_undo_absolute_t state;
  CsUndoAbsoluteFromTxt( txt, &state );
  *AddBack( txt.undos ) = state;

  cs_undo_absolute_t redo = txt.redos.mem[txt.redos.len - 1];
  RemBack( txt.redos );
  ApplyCsUndoAbsolute( txt, redo );
}



__TxtCmd( CmdToggleInsertMode )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  txt.overwrite = !txt.overwrite;
}



__TxtCmd( CmdCursorL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorSkipCharNewlineL( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectSelectL( txt_t& txt )
{
  idx_t c_inline = GetInline( txt, txt.c );
  auto c_move = CursorCharInlineL( txt.buf, txt.c, 1, 0 );
  if( Equal( txt.c, c_move )  ||  c_inline != txt.c_inline ) {
    if( txt.c_inline ) {
      txt.c_inline -= 1;
    }
  } else {
    txt.c = c_move;
    ResetCInline( txt );
  }
}

Inl void
SelectSelectL( txt_t& txt )
{
  txt.c = CursorSkipCharNewlineL( txt.buf, txt.c, 0 );
  ResetCInline( txt );
}

__TxtCmd( CmdSelectL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorSkipCharNewlineL( txt.buf, txt.c, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
        ResetCInline( txt );
      }
    } break;
    case seltype_t::s: {
      SelectSelectL( txt );
    } break;
    case seltype_t::m: {
      MultiselectSelectL( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorSkipCharNewlineR( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectSelectR( txt_t& txt )
{
  idx_t c_inline = GetInline( txt, txt.c );
  auto c_move = CursorCharInlineR( txt.buf, txt.c, 1, 0 );
  if( Equal( txt.c, c_move )  ||  c_inline != txt.c_inline ) {
    txt.c_inline += 1;
  } else {
    txt.c = c_move;
    ResetCInline( txt );
  }
}

Inl void
SelectSelectR( txt_t& txt )
{
  txt.c = CursorSkipCharNewlineR( txt.buf, txt.c, 0 );
  ResetCInline( txt );
}

__TxtCmd( CmdSelectR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorSkipCharNewlineR( txt.buf, txt.c, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
        ResetCInline( txt );
      }
    } break;
    case seltype_t::s: {
      SelectSelectR( txt );
    } break;
    case seltype_t::m: {
      MultiselectSelectR( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorU )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorLineU( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectU )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorLineU( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
      }
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.c = CursorLineU( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorD )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorLineD( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectD )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorLineD( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
      }
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.c = CursorLineD( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectWordAtCursor )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorStopAtNonWordCharR( txt.buf, txt.c, 0 );
      txt.s = CursorStopAtNonWordCharL( txt.buf, txt.c, 0 );
      txt.seltype = seltype_t::s;
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      // TODO: implement
      ImplementCrash();
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorHome )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorHome( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectSelectHome( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  auto c_ln_start = Less( txt.s, txt.c )  ?  ms.bot_ln_start  :  ms.top_ln_start;
  txt.c = c_ln_start;
  txt.c_inline = 0;
}

Inl void
SelectSelectHome( txt_t& txt )
{
  txt.c = CursorHome( txt.buf, txt.c, 0 );
  ResetCInline( txt );
}

__TxtCmd( CmdSelectHome )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorHome( txt.buf, txt.c, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
        ResetCInline( txt );
      }
    } break;
    case seltype_t::s: {
      SelectSelectHome( txt );
    } break;
    case seltype_t::m: {
      MultiselectSelectHome( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorEnd )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorEnd( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectSelectEnd( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  // walk each line of ms, and find the max c_inline across all lines.
  idx_t max_inline = 0;

  idx_t count = 0;
  auto ln_start = ms.bot_ln_start;
  Forever {
    idx_t lnlen;
    auto ln_end = CursorStopAtNewlineR( txt.buf, ln_start, &lnlen );
    max_inline = MAX( max_inline, lnlen );

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
      count += 1;
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  // set txt.c to line end, and txt.c_inline to the max c_inline found.
  txt.c = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
  txt.c_inline = max_inline;
}

Inl void
SelectSelectEnd( txt_t& txt )
{
  txt.c = CursorEnd( txt.buf, txt.c, 0 );
  ResetCInline( txt );
}

__TxtCmd( CmdSelectEnd )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorEnd( txt.buf, txt.c, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
        ResetCInline( txt );
      }
    } break;
    case seltype_t::s: {
      SelectSelectEnd( txt );
    } break;
    case seltype_t::m: {
      MultiselectSelectEnd( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorFileL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = GetBOF( txt.buf );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}

__TxtCmd( CmdCursorFileR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = GetEOF( txt.buf );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorGotoline )
{
  AssertCrash( !misc2 );

  ProfFunc();

  // PERF: very slow for big files!
  // a linecache of some kind would dramatically speed this up.

  auto nlines = misc;
  AddCsUndo( txt );
  txt.seltype = seltype_t::none;
  txt.c = CursorLineD( txt.buf, GetBOF( txt.buf ), 0, nlines, 0, 0 );
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSetSelection )
{
  AssertCrash( misc );
  AssertCrash( misc2 );

  auto posl = *Cast( content_ptr_t*, misc );
  auto posr = *Cast( content_ptr_t*, misc2 );

  auto c = Max( posl, posr );
  auto s = Min( posl, posr );
  AddCsUndo( txt );
  txt.seltype = seltype_t::s;
  txt.c = Min( c, GetEOF( txt.buf ) );
  txt.s = Min( s, GetEOF( txt.buf ) );
  MakeCursorVisible( txt );
}


__TxtCmd( CmdCursorJumpD )
{
  AssertCrash( !misc2 );

  auto nlines = misc;
  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorLineD( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectJumpD )
{
  AssertCrash( !misc2 );

  auto nlines = misc;
  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorLineD( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
      }
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.c = CursorLineD( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorJumpU )
{
  AssertCrash( !misc2 );

  auto nlines = misc;
  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorLineU( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectJumpU )
{
  AssertCrash( !misc2 );

  auto nlines = misc;
  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorLineU( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
      }
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.c = CursorLineU( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorSkipL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorSkipWordSpacetabNewlineL( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectSkipL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorSkipWordSpacetabNewlineL( txt.buf, txt.c, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
        ResetCInline( txt );
      }
    } break;
    case seltype_t::s: {
      txt.c = CursorSkipWordSpacetabNewlineL( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::m: {
      MultiselectSelectL( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCursorSkipR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      txt.c = CursorSkipWordSpacetabNewlineR( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::s:
    case seltype_t::m: {
      txt.seltype = seltype_t::none;
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectSkipR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto c_move = CursorSkipWordSpacetabNewlineR( txt.buf, txt.c, 0 );
      if( !Equal( txt.c, c_move ) ) {
        AnchorSelect( txt, txt.c );
        txt.c = c_move;
        ResetCInline( txt );
      }
    } break;
    case seltype_t::s: {
      txt.c = CursorSkipWordSpacetabNewlineR( txt.buf, txt.c, 0 );
      ResetCInline( txt );
    } break;
    case seltype_t::m: {
      MultiselectSelectR( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdSelectAll )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  txt.c = GetBOF( txt.buf );
  txt.s = GetEOF( txt.buf );
  ResetCInline( txt );
  txt.seltype = seltype_t::s;
}



__TxtCmd( CmdCursorMakePresent )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  txt.c = txt.scroll_start.y;
  auto nlines = txt.window_n_lines / 2;
  txt.c = CursorLineD( txt.buf, txt.c, txt.c_inline, nlines, 0, 0 );
  ResetCInline( txt );
}



__TxtCmd( CmdMultiCursorU )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      ResetCInline( txt ); // we don't want standard c_inline behavior for this Line move.
      txt.s = txt.c;
      txt.seltype = seltype_t::m;
    } break;
    case seltype_t::s: {
      ResetCInline( txt ); // we don't want standard c_inline behavior for this Line move.
      txt.seltype = seltype_t::m;
    } break;
    case seltype_t::m: {
    } break;
    default: UnreachableCrash();
  }
  txt.c = CursorLineU( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );

  // TODO: should we turn single-line multiselects into standard selects?
  //   definitely not for when we have txt.c_inline > eol.

  MakeCursorVisible( txt );
}

__TxtCmd( CmdMultiCursorD )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      ResetCInline( txt ); // we don't want standard c_inline behavior for this Line move.
      txt.s = txt.c;
      txt.seltype = seltype_t::m;
    } break;
    case seltype_t::s: {
      ResetCInline( txt ); // we don't want standard c_inline behavior for this Line move.
      txt.seltype = seltype_t::m;
    } break;
    case seltype_t::m: {
    } break;
    default: UnreachableCrash();
  }
  txt.c = CursorLineD( txt.buf, txt.c, txt.c_inline, 1, 0, 0 );

  // TODO: should we turn single-line multiselects into standard selects?
  //   definitely not for when we have txt.c_inline > eol.

  MakeCursorVisible( txt );
}





// =================================================================================
// CONTENT EDITING
//

Inl void
AddTxtUndo( txt_t& txt )
{
  // let buf_t add an internal checkpt so it can unwind enough times.
  UndoCheckpt( txt.buf );

  // invalidate previous futures.
  txt.redos_txtcs.len = 0;

  cs_undo_t state;
  CsUndoFromTxt( txt, &state );
  *AddBack( txt.undos_txtcs ) = state;
}


Inl void
MultiselectDelete( txt_t& txt, multisel_t& ms )
{
  AssertCrash( ms.left_inline != ms.rght_inline );

  auto ln_start = ms.bot_ln_start;
  Forever {
    // the selection range on the given line ln_start.
    auto ml = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, 0 );
    auto mr = CursorCharInlineR( txt.buf, ln_start, ms.rght_inline, 0 );

    content_ptr_t* concurrents[] = {
      &ln_start,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    Delete( txt.buf, ml, mr, AL( concurrents ) );

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  txt.c_inline = ms.left_inline;
}


Inl void
SelectDelete( txt_t& txt )
{
  content_ptr_t sl, sr;
  GetSelect( txt, &sl, &sr );

  content_ptr_t* concurrents[] = {
    __TxtConcurrentPtrs( txt )
  };
  Delete( txt.buf, sl, sr, AL( concurrents ) );

  txt.seltype = seltype_t::none;
  ResetCInline( txt );
}



Inl void
AddChar( txt_t& txt, u8 c )
{
  content_ptr_t* concurrents[] = {
    __TxtConcurrentPtrs( txt )
  };
  if( txt.overwrite ) {
    auto ln_end = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
    if( !Equal( txt.c, ln_end ) ) {
      auto repl_start = txt.c;
      auto repl_end = CursorCharR( txt.buf, txt.c, 1, 0 );
      Replace( txt.buf, repl_start, repl_end, &c, 1, AL( concurrents ) );
    } else {
      Insert( txt.buf, txt.c, &c, 1, AL( concurrents ) );
    }
  } else {
    Insert( txt.buf, txt.c, &c, 1, AL( concurrents ) );
  }
  ResetCInline( txt );
}

Inl void
SelectAddString( txt_t& txt, u8* str, idx_t str_len )
{
  if( txt.overwrite ) {
    SelectDelete( txt );
  } else {
    SelectDelete( txt );
    if( str_len ) {
      content_ptr_t* concurrents[] = {
        __TxtConcurrentPtrs( txt )
      };
      Insert( txt.buf, txt.c, str, str_len, AL( concurrents ) );
      ResetCInline( txt );
    }
  }
}

Inl void
MultiselectAddString( txt_t& txt, u8* str, idx_t str_len )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  if( txt.overwrite ) {
    if( ms.left_inline != ms.rght_inline ) {
      MultiselectDelete( txt, ms );
      return;

    } else {
      ms.rght_inline += str_len;
      MultiselectDelete( txt, ms );
      // txt.c/c_inline has probably changed, so ms.left_inline/rght_inline will be different!
      // so, re-get the ms so MultiselectAddChar can work!
      GetMultiselect( txt, ms );
    }
  } else {
    if( ms.left_inline != ms.rght_inline ) {
      MultiselectDelete( txt, ms );
      // txt.c/c_inline has probably changed, so ms.left_inline/rght_inline will be different!
      // so, re-get the ms so MultiselectAddChar can work!
      GetMultiselect( txt, ms );
    }

    AssertCrash( ms.left_inline == ms.rght_inline );
  }

  if( !str_len ) {
    return;
  }

  auto ln_start = ms.bot_ln_start;
  Forever {
    auto ln_end = CursorStopAtNewlineR( txt.buf, ln_start, 0 );

    idx_t ninline;
    auto pos = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
    content_ptr_t* concurrents[] = {
      &pos,
      &ln_start,
      &ln_end,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    if( ninline < ms.left_inline ) {
      auto nspaces = ms.left_inline - ninline;
      while( nspaces-- ) {
        Insert(
          txt.buf,
          ln_end,
          Str( " " ),
          1,
          AL( concurrents )
          );
      }
    }
    AssertCrash( LEqual( pos, ln_end ) );

    Insert( txt.buf, pos, str, str_len, AL( concurrents ) );

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  ResetCInline( txt );
}

// TODO: replace AddChar with this?
Inl void
AddString( txt_t& txt, u8* str, idx_t str_len )
{
  if( !str_len ) {
    return;
  }

  content_ptr_t* concurrents[] = {
    __TxtConcurrentPtrs( txt )
  };
  if( txt.overwrite ) {
    idx_t nmoved;
    auto ln_end = CursorStopAtNewlineR( txt.buf, txt.c, &nmoved );
    auto nreplace = MIN( str_len, nmoved );
    auto ninsert = str_len - nreplace;
    auto repl_start = txt.c;
    auto repl_end = CursorCharR( txt.buf, txt.c, nreplace, 0 );
    Replace( txt.buf, repl_start, repl_end, str, nreplace, AL( concurrents ) );
    Insert( txt.buf, txt.c, str + nreplace, ninsert, AL( concurrents ) );
  } else {
    Insert( txt.buf, txt.c, str, str_len, AL( concurrents ) );
  }
  ResetCInline( txt );
}


__TxtCmd( CmdAddChar )
{
  AssertCrash( !misc2 );

  auto c = Cast( u8, misc );
  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      AddChar( txt, c );
    } break;
    case seltype_t::s: {
      SelectAddString( txt, &c, 1 );
    } break;
    case seltype_t::m: {
      MultiselectAddString( txt, &c, 1 );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}

__TxtCmd( CmdAddAlpha )
{
  auto c = Cast( u8, misc );
  auto caps = ( misc2 != 0 );
  c = caps ? CsToUpper( c ) : CsToLower( c );
  CmdAddChar( txt, c );
}



__TxtCmd( CmdAddString )
{
  auto str = Cast( u8*, misc );
  auto str_len = Cast( idx_t, misc2 );
  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      AddString( txt, str, str_len );
    } break;
    case seltype_t::s: {
      SelectAddString( txt, str, str_len );
    } break;
    case seltype_t::m: {
      MultiselectAddString( txt, str, str_len );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectTabR( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  auto ln_start = ms.bot_ln_start;
  Forever {
    content_ptr_t* concurrents[] = {
      &ln_start,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    if( txt.insert_spaces_for_tabs ) {
      auto nspaces = txt.spaces_per_tab;
      while( nspaces-- ) {
        Insert( txt.buf, ln_start, Str( " " ), 1, AL( concurrents ) );
      }
    } else {
      Insert( txt.buf, ln_start, Str( "\t" ), 1, AL( concurrents ) );
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  idx_t ninc = ( txt.insert_spaces_for_tabs )  ?  txt.spaces_per_tab  :  1;
  txt.c_inline += ninc;
}

Inl void
TabR( txt_t& txt )
{
  content_ptr_t* concurrents[] = {
    __TxtConcurrentPtrs( txt )
  };
  if( txt.insert_spaces_for_tabs ) {
    Fori( u8, i, 0, txt.spaces_per_tab ) {
      Insert( txt.buf, txt.c, Str( " " ), 1, AL( concurrents ) );
    }
  } else {
    Insert( txt.buf, txt.c, Str( "\t" ), 1, AL( concurrents ) );
  }
  ResetCInline( txt );
}

__TxtCmd( CmdTabR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      TabR( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectTabR( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl bool
IsAllSpaces( buf_t& buf, content_ptr_t start, content_ptr_t end )
{
  auto tmp = AllocContents( buf, start, end ); // PERF: unnecessary copying.
  For( i, 0, tmp.len ) {
    if( tmp.mem[i] != ' ' ) {
      return 0;
    }
  }
  Free( tmp );
  return 1;
}

Inl bool
EveryNonEmptyMultiselLineIsLeadingAllSpaces( txt_t& txt, multisel_t& ms )
{
  auto ln_start = ms.bot_ln_start;
  Forever {
    auto spaces_end = CursorCharInlineR( txt.buf, ln_start, txt.spaces_per_tab, 0 );
    bool all_spaces = IsAllSpaces( txt.buf, ln_start, spaces_end );
    if( !all_spaces ) {
      return 0;
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }
  return 1;
}

Inl bool
EveryNonEmptyMultiselLineIsLeadingOneTab( txt_t& txt, multisel_t& ms )
{
  auto ln_start = ms.bot_ln_start;
  Forever {
    auto ln_end = CursorStopAtNewlineR( txt.buf, ln_start, 0 );

    if( Less( ln_start, ln_end ) ) {
      u8 c;
      Contents( txt.buf, ln_start, &c, 1 );
      if( c != '\t' ) {
        return 0;
      }
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }
  return 1;
}

Inl void
MultiselectTabL( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  // TODO: implement mixed tabs/spaces.
  bool all_spaces = EveryNonEmptyMultiselLineIsLeadingAllSpaces( txt, ms );
  bool all_tab = EveryNonEmptyMultiselLineIsLeadingOneTab( txt, ms );

  if( all_spaces  ||  all_tab ) {
    auto ln_start = ms.bot_ln_start;
    Forever {
      content_ptr_t* concurrents[] = {
        &ln_start,
        __MultiselConcurrentPtrs( ms ),
        __TxtConcurrentPtrs( txt )
      };

      if( all_spaces ) {
        auto del_start = ln_start;
        auto del_end = CursorCharInlineR( txt.buf, ln_start, txt.spaces_per_tab, 0 );
        Delete( txt.buf, del_start, del_end, AL( concurrents ) );
      } elif( all_tab ) {
        auto del_start = ln_start;
        auto del_end = CursorCharInlineR( txt.buf, ln_start, 1, 0 );
        Delete( txt.buf, del_start, del_end, AL( concurrents ) );
      }

      if( Greater( ln_start, ms.top_ln_start ) ) {
        ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
      } else {
        AssertCrash( Equal( ln_start, ms.top_ln_start ) );
        break;
      }
    }
  }

  if( all_spaces ) {
    idx_t ninc = txt.spaces_per_tab;
    txt.c_inline = ( txt.c_inline < ninc )  ?  0  :  ( txt.c_inline - ninc );

  } elif( all_tab ) {
    idx_t ninc = 1;
    txt.c_inline = ( txt.c_inline < ninc )  ?  0  :  ( txt.c_inline - ninc );
  }
}

Inl void
TabL( txt_t& txt )
{
  if( !TxtLen( txt ) ) {
    return;
  }

  auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );

  content_ptr_t* concurrents[] = {
    &ln_start,
    __TxtConcurrentPtrs( txt )
  };

  u8 first;
  Contents( txt.buf, ln_start, &first, 1 );
  if( first == '\t' ) {
    auto del_start = ln_start;
    auto del_end = CursorCharR( txt.buf, del_start, 1, 0 );
    Delete( txt.buf, del_start, del_end, AL( concurrents ) );
  } elif( first == ' ' ) {
    auto spaces_start = ln_start;
    auto spaces_end = CursorCharR( txt.buf, spaces_start, txt.spaces_per_tab, 0 );
    bool all_spaces = IsAllSpaces( txt.buf, spaces_start, spaces_end );
    if( all_spaces ) {
      Delete( txt.buf, spaces_start, spaces_end, AL( concurrents ) );
    }
  }
  // if first wasn't space or tab, then no-op.
}

__TxtCmd( CmdTabL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      TabL( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectTabL( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



#define __TCountMultiselFn( name ) \
  content_ptr_t \
  ( name )( \
    buf_t& buf, \
    content_ptr_t pos, \
    idx_t* nchars_moved \
    ) \

typedef __TCountMultiselFn( *pfn_tcountmultisel_t );

Templ Inl bool
GetTCountAcrossAllNonEmptyMultiselLines( txt_t& txt, multisel_t& ms, idx_t* nchars, T fn )
{
  bool min_valid = 0;
  idx_t min_nchars = 0;

  auto ln_start = ms.bot_ln_start;
  Forever {
    // the selection range on the given line ln_start.
    idx_t ninline;
    auto ml = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
    idx_t ln_len;
    auto ln_end = CursorStopAtNewlineR( txt.buf, ln_start, &ln_len );
    // skip lines that aren't long enough to reach ms.left_inline
    if( ln_len  &&  ninline == ms.left_inline ) {
      idx_t nmoved;
      fn( txt.buf, ml, &nmoved );
      if( !min_valid ) {
        min_valid = 1;
        min_nchars = nmoved;
      } else {
        min_nchars = MIN( min_nchars, nmoved );
        if( !min_nchars ) {
          return 0;
        }
      }
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  *nchars = min_nchars;
  return 1;
}

Inl bool
GetNumSpacetabsAcrossAllNonEmptyMultiselLinesL( txt_t& txt, multisel_t& ms, idx_t* nchars )
{
  return GetTCountAcrossAllNonEmptyMultiselLines<pfn_tcountmultisel_t>( txt, ms, nchars, CursorSkipSpacetabL );
}

Inl bool
GetNumSpacetabsAcrossAllNonEmptyMultiselLinesR( txt_t& txt, multisel_t& ms, idx_t* nchars )
{
  return GetTCountAcrossAllNonEmptyMultiselLines<pfn_tcountmultisel_t>( txt, ms, nchars, CursorSkipSpacetabR );
}

Inl bool
GetNumWordCharsAcrossAllNonEmptyMultiselLinesL( txt_t& txt, multisel_t& ms, idx_t* nchars )
{
  return GetTCountAcrossAllNonEmptyMultiselLines<pfn_tcountmultisel_t>( txt, ms, nchars, CursorStopAtNonWordCharL );
}

Inl bool
GetNumWordCharsAcrossAllNonEmptyMultiselLinesR( txt_t& txt, multisel_t& ms, idx_t* nchars )
{
  return GetTCountAcrossAllNonEmptyMultiselLines<pfn_tcountmultisel_t>( txt, ms, nchars, CursorStopAtNonWordCharR );
}

Inl void
MultiselectRemChL( txt_t& txt, multisel_t& ms, idx_t len )
{
  AssertCrash( ms.left_inline == ms.rght_inline );
  if( !ms.left_inline ) {
    return;
  }

  auto ln_start = ms.bot_ln_start;
  Forever {
    // the selection range on the given line ln_start.
    idx_t ninline;
    auto ml = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
    // skip lines that aren't long enough to reach ms.left_inline
    if( ninline == ms.left_inline ) {
      content_ptr_t* concurrents[] = {
        &ln_start,
        __MultiselConcurrentPtrs( ms ),
        __TxtConcurrentPtrs( txt )
      };
      auto del_start = CursorCharInlineL( txt.buf, ml, len, 0 );
      auto del_end = ml;
      Delete( txt.buf, del_start, del_end, AL( concurrents ) );
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  AssertCrash( len <= txt.c_inline );
  txt.c_inline -= len;
}

Inl void
MultiselectRemWordL( txt_t& txt, multisel_t& ms )
{
  AssertCrash( ms.left_inline == ms.rght_inline );
  if( !ms.left_inline ) {
    return;
  }

  idx_t nwordchars;
  auto all_wordchars = GetNumWordCharsAcrossAllNonEmptyMultiselLinesL( txt, ms, &nwordchars );
  idx_t nspacetabs;
  auto all_spacetabs = GetNumSpacetabsAcrossAllNonEmptyMultiselLinesL( txt, ms, &nspacetabs );

  idx_t nrem;
  if( all_wordchars ) {
    nrem = nwordchars;
  } elif( all_spacetabs ) {
    nrem = nspacetabs;
  } else {
    nrem = 1;
  }
  MultiselectRemChL( txt, ms, nrem );
}

__TxtCmd( CmdRemChL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      content_ptr_t* concurrents[] = {
        __TxtConcurrentPtrs( txt )
      };
      auto prev = CursorSkipCharNewlineL( txt.buf, txt.c, 0 );
      Delete( txt.buf, prev, txt.c, AL( concurrents ) );
      ResetCInline( txt );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectDelete( txt, ms );
      } else {
        MultiselectRemChL( txt, ms, 1 );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectRemChR( txt_t& txt, multisel_t& ms, idx_t len )
{
  AssertCrash( ms.left_inline == ms.rght_inline );

  auto ln_start = ms.bot_ln_start;
  Forever {
    // the selection range on the given line ln_start.
    idx_t ninline;
    auto ml = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
    // skip lines that aren't long enough to reach ms.left_inline
    if( ninline == ms.left_inline ) {
      content_ptr_t* concurrents[] = {
        &ln_start,
        __MultiselConcurrentPtrs( ms ),
        __TxtConcurrentPtrs( txt )
      };
      auto del_start = ml;
      auto del_end = CursorCharInlineR( txt.buf, del_start, len, 0 );
      Delete( txt.buf, del_start, del_end, AL( concurrents ) );
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }
}

Inl void
MultiselectRemWordR( txt_t& txt, multisel_t& ms )
{
  AssertCrash( ms.left_inline == ms.rght_inline );

  idx_t nwordchars;
  auto all_wordchars = GetNumWordCharsAcrossAllNonEmptyMultiselLinesR( txt, ms, &nwordchars );
  idx_t nspacetabs;
  auto all_spacetabs = GetNumSpacetabsAcrossAllNonEmptyMultiselLinesR( txt, ms, &nspacetabs );

  idx_t nrem;
  if( all_wordchars ) {
    nrem = nwordchars;
  } elif( all_spacetabs ) {
    nrem = nspacetabs;
  } else {
    nrem = 1;
  }
  MultiselectRemChR( txt, ms, nrem );
}

__TxtCmd( CmdRemChR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      content_ptr_t* concurrents[] = {
        __TxtConcurrentPtrs( txt )
      };
      auto next = CursorSkipCharNewlineR( txt.buf, txt.c, 0 );
      Delete( txt.buf, txt.c, next, AL( concurrents ) );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectDelete( txt, ms );
      } else {
        MultiselectRemChR( txt, ms, 1 );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdRemWordL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      content_ptr_t* concurrents[] = {
        __TxtConcurrentPtrs( txt )
      };
      auto prev = CursorSkipWordSpacetabNewlineL( txt.buf, txt.c, 0 );
      Delete( txt.buf, prev, txt.c, AL( concurrents ) );
      ResetCInline( txt );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectDelete( txt, ms );
      } else {
        MultiselectRemWordL( txt, ms );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdRemWordR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      content_ptr_t* concurrents[] = {
        __TxtConcurrentPtrs( txt )
      };
      auto next = CursorSkipWordSpacetabNewlineR( txt.buf, txt.c, 0 );
      Delete( txt.buf, txt.c, next, AL( concurrents ) );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectDelete( txt, ms );
      } else {
        MultiselectRemWordR( txt, ms );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectRemLineR( txt_t& txt, multisel_t& ms )
{
  AssertCrash( ms.left_inline == ms.rght_inline );

  auto ln_start = ms.bot_ln_start;
  Forever {
    // the selection range on the given line ln_start.
    idx_t ninline;
    auto ml = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
    // skip lines that aren't long enough to reach ms.left_inline
    if( ninline == ms.left_inline ) {
      content_ptr_t* concurrents[] = {
        &ln_start,
        __MultiselConcurrentPtrs( ms ),
        __TxtConcurrentPtrs( txt )
      };
      auto del_start = ml;
      auto del_end = CursorStopAtNewlineR( txt.buf, del_start, 0 );
      Delete( txt.buf, del_start, del_end, AL( concurrents ) );
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }
}

__TxtCmd( CmdRemLineR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      content_ptr_t* concurrents[] = {
        __TxtConcurrentPtrs( txt )
      };
      auto next = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
      Delete( txt.buf, txt.c, next, AL( concurrents ) );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectDelete( txt, ms );
      } else {
        MultiselectRemLineR( txt, ms );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdAddLn )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );
      content_ptr_t* concurrents[] = {
        &ln_start,
        __TxtConcurrentPtrs( txt )
      };
      Insert(
        txt.buf,
        txt.c,
        Str( "\r\n" ),
        2,
        AL( concurrents )
        );
      // ln_indent is an end-ptr, so we have to determine its position AFTER the insert.
      // this is so we only copy the leading spacetabs, and not the newly-added newline.
      auto ln_indent = CursorSkipSpacetabR( txt.buf, ln_start, 0 );
      Copy(
        txt.buf,
        ln_start,
        ln_indent,
        txt.c,
        AL( concurrents )
        );
      ResetCInline( txt );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectDelete( txt, ms );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectRemLn( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  auto ln_start = ms.bot_ln_start;
  Forever {
    auto ln_end = CursorStopAtNewlineR( txt.buf, ln_start, 0 );

    content_ptr_t* concurrents[] = {
      &ln_start,
      &ln_end,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    auto y_next_start = CursorSingleNewlineR( txt.buf, ln_end, 0 );
    Delete( txt.buf, ln_start, y_next_start, AL( concurrents ) );

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  txt.seltype = seltype_t::none;

  idx_t yl_len;
  auto yl_end = CursorStopAtNewlineR( txt.buf, ms.top_ln_start, &yl_len );
  if( ms.left_inline <= yl_len ) {
    txt.c = CursorCharR( txt.buf, ms.top_ln_start, ms.c_inline, 0 );
  } else {
    txt.c = yl_end;
  }

  txt.c_inline = ms.left_inline;
}

Inl void
RemLn( txt_t& txt )
{
  auto ln0_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );

  {
    content_ptr_t* concurrents[] = {
      &ln0_start,
      __TxtConcurrentPtrs( txt )
    };
    auto ln0_end = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
    auto ln1_start = CursorSingleNewlineR( txt.buf, ln0_end, 0 );
    Delete( txt.buf, ln0_start, ln1_start, AL( concurrents ) );
  }

  idx_t ln0_len;
  auto ln0_end = CursorStopAtNewlineR( txt.buf, ln0_start, &ln0_len );
  if( txt.c_inline <= ln0_len ) {
    txt.c = CursorCharR( txt.buf, ln0_start, txt.c_inline, 0 );
  } else {
    txt.c = ln0_end;
  }
}

__TxtCmd( CmdRemLn )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      RemLn( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectRemLn( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectCommentElseUncomment( txt_t& txt, bool comment )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  auto ln_start = ms.bot_ln_start;
  Forever {
    content_ptr_t* concurrents[] = {
      &ln_start,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    if( comment ) {
      Insert( txt.buf, ln_start, Str( "//" ), 2, AL( concurrents ) );
    } else {
      auto del_start = ln_start;
      auto del_end = CursorCharInlineR( txt.buf, del_start, 2, 0 );
      Delete( txt.buf, del_start, del_end, AL( concurrents ) );
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  ResetCInline( txt );
}

Inl void
Comment( txt_t& txt )
{
  content_ptr_t* concurrents[] = {
    __TxtConcurrentPtrs( txt )
  };
  auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );
  Insert( txt.buf, ln_start, Str( "//" ), 2, AL( concurrents ) );
  txt.c_inline += 2;
}

__TxtCmd( CmdComment )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      Comment( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectCommentElseUncomment( txt, 1 );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl bool
MultiselectIsAllCommentLines( txt_t& txt, multisel_t& ms )
{
  auto ln_start = ms.bot_ln_start;
  Forever {
    auto slash_start = ln_start;
    auto slash_end = CursorCharR( txt.buf, ln_start, 2, 0 );
    u8 tmp[2];
    auto tmp_len = Contents( txt.buf, slash_start, slash_end, AL( tmp ) );
    bool is_comment = MemEqual( tmp, tmp_len, "//", 2 );
    if( !is_comment ) {
      return 0;
    }

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  return 1;
}


Inl void
MultiselectUncomment( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );
  bool all_comments = MultiselectIsAllCommentLines( txt, ms );
  if( all_comments ) {
    MultiselectCommentElseUncomment( txt, 0 );
  }
}

Inl void
Uncomment( txt_t& txt )
{
  auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );
  auto ln_end = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
  auto slash_end = CursorCharR( txt.buf, ln_start, 2, 0 );
  auto tmp = AllocContents( txt.buf, ln_start, slash_end );
  content_ptr_t* concurrents[] = {
    &ln_start,
    &ln_end,
    &slash_end,
    __TxtConcurrentPtrs( txt )
  };
  bool is_comment = MemEqual( ML( tmp ), "//", 2 );
  if( is_comment ) {
    auto del_start = ln_start;
    auto del_end = slash_end;
    Delete( txt.buf, del_start, del_end, AL( concurrents ) );
    txt.c_inline = MAX( txt.c_inline, 2 ) - 2;
  }
  Free( tmp );
}

__TxtCmd( CmdUncomment )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      Uncomment( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectUncomment( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}


Inl void
MultiselectToggleComment( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  bool all_comments = MultiselectIsAllCommentLines( txt, ms );
  MultiselectCommentElseUncomment( txt, !all_comments );
}

Inl void
ToggleComment( txt_t& txt )
{
  auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );
  auto ln_end = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
  auto slash_end = CursorCharR( txt.buf, ln_start, 2, 0 );
  auto tmp = AllocContents( txt.buf, ln_start, slash_end );
  content_ptr_t* concurrents[] = {
    &ln_start,
    &ln_end,
    &slash_end,
    __TxtConcurrentPtrs( txt )
  };
  bool is_comment = MemEqual( ML( tmp ), "//", 2 );
  if( is_comment ) {
    auto del_start = ln_start;
    auto del_end = slash_end;
    Delete( txt.buf, del_start, del_end, AL( concurrents ) );
    txt.c_inline = MAX( txt.c_inline, 2 ) - 2;
  } else {
    Insert( txt.buf, ln_start, Str( "//" ), 2, AL( concurrents ) );
    txt.c_inline += 2;
  }
  Free( tmp );
}

__TxtCmd( CmdToggleComment )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      ToggleComment( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectToggleComment( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
CommentSelection( txt_t& txt )
{
  content_ptr_t sl, sr;
  GetSelect( txt, &sl, &sr );
  content_ptr_t* concurrents[] = {
    &sl,
    &sr,
    __TxtConcurrentPtrs( txt )
  };
  Insert( txt.buf, sr, Str( "*/" ), 2, AL( concurrents ) );
  Insert( txt.buf, sl, Str( "/*" ), 2, AL( concurrents ) );
  ResetCInline( txt );
}

__TxtCmd( CmdCommentSelection )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none:
    case seltype_t::m: {
    } break;
    case seltype_t::s: {
      CommentSelection( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
UncommentSelection( txt_t& txt )
{
  content_ptr_t sl, sr;
  GetSelect( txt, &sl, &sr );

  // TODO: allow selecting just the thing inside the comment
  // TODO: allow whitespace inside the comment ?

  auto open_start = sl;
  auto open_end = CursorCharR( txt.buf, sl, 2, 0 );

  auto close_start = CursorCharL( txt.buf, sr, 2, 0 );
  auto close_end = sr;

  auto open = AllocContents( txt.buf, open_start, open_end );
  auto close = AllocContents( txt.buf, close_start, close_end );

  auto is_comment =
    MemEqual( ML( open ), Str( "/*" ), 2 )  &&
    MemEqual( ML( close ), Str( "*/" ), 2 );
  if( is_comment ) {
    content_ptr_t* concurrents[] = {
      &open_start,
      &open_end,
      &close_start,
      &close_end,
      __TxtConcurrentPtrs( txt )
    };
    Delete( txt.buf, close_start, close_end, AL( concurrents ) );
    Delete( txt.buf, open_start, open_end, AL( concurrents ) );
    ResetCInline( txt );
  }

  Free( open );
  Free( close );
}

__TxtCmd( CmdUncommentSelection )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none:
    case seltype_t::m: {
    } break;
    case seltype_t::s: {
      UncommentSelection( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}


#define USE_SWAP_FOR_PERMUTELINES 0 // TODO: need some more testing+validation of buf retvals from Copy, Move, Swap.


Inl void
MultiselectPermuteU( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  idx_t c_ln_len;
  CursorStopAtNewlineR( txt.buf, ms.c_ln_start, &c_ln_len );
  idx_t s_ln_len;
  CursorStopAtNewlineR( txt.buf, ms.s_ln_start, &s_ln_len );

  auto c_before_s = LEqual( txt.c, txt.s );

  auto r1_start = ms.top_ln_start;
  auto r1_end = CursorStopAtNewlineR( txt.buf, ms.bot_ln_start, 0 );
  auto r0_end = CursorSingleNewlineL( txt.buf, r1_start, 0 );
  auto r0_start = CursorStopAtNewlineL( txt.buf, r0_end, 0 );
  if( !Equal( r0_end, r1_start ) ) { // newline actually skipped
#if USE_SWAP_FOR_PERMUTELINES
    bool c_was_r0_end = Equal( txt.c, r0_end );
    bool c_was_r1_end = Equal( txt.c, r1_end );
    bool s_was_r0_end = Equal( txt.s, r0_end );
    bool s_was_r1_end = Equal( txt.s, r1_end );
    content_ptr_t moved_r0_end;
    content_ptr_t moved_r1_end;
    content_ptr_t* concurrents[] = {
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    Swap(
      txt.buf,
      r0_start,
      r0_end,
      r1_start,
      r1_end,
      &moved_r0_end,
      &moved_r1_end,
      AL( concurrents )
      );

    // i.e. it doesn't know that it's really a list-end pointer.
    // so, fix txt.c/s if we're in that case.
    // TODO: should we do this for all concurrents here; e.g. scroll_start, scroll_end?
    if( c_was_r0_end ) {
      txt.c = moved_r0_end;
    }
    if( c_was_r1_end ) {
      txt.c = moved_r1_end;
    }
    if( s_was_r0_end ) {
      txt.s = moved_r0_end;
    }
    if( s_was_r1_end ) {
      txt.s = moved_r1_end;
    }
#else // !USE_SWAP_FOR_PERMUTELINES
    auto r0_len = CountCharsBetween( txt.buf, r0_start, r0_end );
    content_ptr_t* concurrents[] = {
      &r0_start,
      &r0_end,
      &r1_start,
      &r1_end,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    // r0 newline r1 => r0 r1 newline
    Move(
      txt.buf,
      r0_end, // src_start
      r1_start, // src_end
      r1_end, // dst
      AL( concurrents )
      );
    // r0 r1 newline => r1 newline r0
    Move(
      txt.buf,
      r0_start, // src_start
      CursorCharR( txt.buf, r0_start, r0_len, 0 ), // src_end
      r1_end, // dst
      AL( concurrents )
      );

    if( c_ln_len ) {
      txt.c = CursorCharR( txt.buf, ms.c_ln_start, ms.c_inline, 0 );
    } else {
      txt.c = c_before_s  ?  r1_start  :  r0_end;
    }
    if( s_ln_len ) {
      txt.s = CursorCharR( txt.buf, ms.s_ln_start, ms.s_inline, 0 );
    } else {
      txt.s = c_before_s  ?  r0_end  :  r1_start;
    }
#endif // !USE_SWAP_FOR_PERMUTELINES
  }
}

Inl void
PermuteU( txt_t& txt )
{
  idx_t c_inline;
  auto r1_start = CursorStopAtNewlineL( txt.buf, txt.c, &c_inline );
  auto r1_end = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
  auto r0_end = CursorSingleNewlineL( txt.buf, r1_start, 0 );
  auto r0_start = CursorStopAtNewlineL( txt.buf, r0_end, 0 );
  if( !Equal( r0_end, r1_start ) ) { // newline actually skipped
#if USE_SWAP_FOR_PERMUTELINES
    bool c_was_r0_end = Equal( txt.c, r0_end );
    bool c_was_r1_end = Equal( txt.c, r1_end );
    content_ptr_t moved_r0_end;
    content_ptr_t moved_r1_end;
    content_ptr_t* concurrents[] = {
      __TxtConcurrentPtrs( txt )
    };
    Swap(
      txt.buf,
      r0_start,
      r0_end,
      r1_start,
      r1_end,
      &moved_r0_end,
      &moved_r1_end,
      AL( concurrents )
      );

    // i.e. it doesn't know that it's really a list-end pointer.
    // so, fix txt.c if we're in that case.
    // TODO: should we do this for all concurrents here; e.g. scroll_start, scroll_end?
    if( c_was_r0_end ) {
      txt.c = moved_r0_end;
    }
    if( c_was_r1_end ) {
      txt.c = moved_r1_end;
    }
#else // !USE_SWAP_FOR_PERMUTELINES
    auto r0_len = CountCharsBetween( txt.buf, r0_start, r0_end );
    auto r1_len = CountCharsBetween( txt.buf, r1_start, r1_end );
    content_ptr_t* concurrents[] = {
      &r0_start,
      &r0_end,
      &r1_start,
      &r1_end,
      __TxtConcurrentPtrs( txt )
    };
    // r0 newline r1 => r0 r1 newline
    Move(
      txt.buf,
      r0_end, // src_start
      r1_start, // src_end
      r1_end, // dst
      AL( concurrents )
      );
    // r0 r1 newline => r1 newline r0
    Move(
      txt.buf,
      r0_start, // src_start
      CursorCharR( txt.buf, r0_start, r0_len, 0 ), // src_end
      r1_end, // dst
      AL( concurrents )
      );

    if( r1_len ) {
      txt.c = CursorCharR( txt.buf, r1_start, c_inline, 0 );
    } else {
      txt.c = r0_end;
    }
#endif // !USE_SWAP_FOR_PERMUTELINES
  }
}

__TxtCmd( CmdPermuteU )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      PermuteU( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectPermuteU( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



Inl void
MultiselectPermuteD( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  idx_t c_ln_len;
  CursorStopAtNewlineR( txt.buf, ms.c_ln_start, &c_ln_len );
  idx_t s_ln_len;
  CursorStopAtNewlineR( txt.buf, ms.s_ln_start, &s_ln_len );

  auto c_before_s = LEqual( txt.c, txt.s );

  auto r0_start = ms.top_ln_start;
  auto r0_end = CursorStopAtNewlineR( txt.buf, ms.bot_ln_start, 0 );
  auto r1_start = CursorSingleNewlineR( txt.buf, r0_end, 0 );
  auto r1_end = CursorStopAtNewlineR( txt.buf, r1_start, 0 );
  if( !Equal( r0_end, r1_start ) ) { // newline actually skipped
#if USE_SWAP_FOR_PERMUTELINES
    bool c_was_r0_end = Equal( txt.c, r0_end );
    bool c_was_r1_end = Equal( txt.c, r1_end );
    bool s_was_r0_end = Equal( txt.s, r0_end );
    bool s_was_r1_end = Equal( txt.s, r1_end );
    content_ptr_t moved_r0_end;
    content_ptr_t moved_r1_end;
    content_ptr_t* concurrents[] = {
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    Swap(
      txt.buf,
      r0_start,
      r0_end,
      r1_start,
      r1_end,
      &moved_r0_end,
      &moved_r1_end,
      AL( concurrents )
      );

    // i.e. it doesn't know that it's really a list-end pointer.
    // so, fix txt.c/s if we're in that case.
    // TODO: should we do this for all concurrents here; e.g. scroll_start, scroll_end?
    if( c_was_r0_end ) {
      txt.c = moved_r0_end;
    }
    if( c_was_r1_end ) {
      txt.c = moved_r1_end;
    }
    if( s_was_r0_end ) {
      txt.s = moved_r0_end;
    }
    if( s_was_r1_end ) {
      txt.s = moved_r1_end;
    }
#else // !USE_SWAP_FOR_PERMUTELINES
    auto r0_len = CountCharsBetween( txt.buf, r0_start, r0_end );
    content_ptr_t* concurrents[] = {
      &r0_start,
      &r0_end,
      &r1_start,
      &r1_end,
      __MultiselConcurrentPtrs( ms ),
      __TxtConcurrentPtrs( txt )
    };
    // r0 newline r1 => r0 r1 newline
    Move(
      txt.buf,
      r0_end, // src_start
      r1_start, // src_end
      r1_end, // dst
      AL( concurrents )
      );
    // r0 r1 newline => r1 newline r0
    Move(
      txt.buf,
      r0_start, // src_start
      CursorCharR( txt.buf, r0_start, r0_len, 0 ), // src_end
      r1_end, // dst
      AL( concurrents )
      );

    if( c_ln_len ) {
      txt.c = CursorCharR( txt.buf, ms.c_ln_start, ms.c_inline, 0 );
    } else {
      txt.c = c_before_s  ?  r0_start  :  r1_end;
    }
    if( s_ln_len ) {
      txt.s = CursorCharR( txt.buf, ms.s_ln_start, ms.s_inline, 0 );
    } else {
      txt.s = c_before_s  ?  r1_end  :  r0_start;
    }
#endif // !USE_SWAP_FOR_PERMUTELINES
  }
}

Inl void
PermuteD( txt_t& txt )
{
  idx_t c_inline;
  auto r0_start = CursorStopAtNewlineL( txt.buf, txt.c, &c_inline );
  auto r0_end = CursorStopAtNewlineR( txt.buf, txt.c, 0 );
  auto r1_start = CursorSingleNewlineR( txt.buf, r0_end, 0 );
  auto r1_end = CursorStopAtNewlineR( txt.buf, r1_start, 0 );
  if( !Equal( r0_end, r1_start ) ) { // newline actually skipped
#if USE_SWAP_FOR_PERMUTELINES
    bool c_was_r0_end = Equal( txt.c, r0_end );
    bool c_was_r1_end = Equal( txt.c, r1_end );
    content_ptr_t moved_r0_end;
    content_ptr_t moved_r1_end;
    content_ptr_t* concurrents[] = {
      __TxtConcurrentPtrs( txt )
    };
    Swap(
      txt.buf,
      r0_start,
      r0_end,
      r1_start,
      r1_end,
      &moved_r0_end,
      &moved_r1_end,
      AL( concurrents )
      );

    // Swap doesn't move the r1_end, since it's actually pointing to content outside the swap.
    // i.e. it doesn't know that it's really a list-end pointer.
    // so, fix txt.c if we're in that case.
    // TODO: should we do this for all concurrents here; e.g. scroll_start, scroll_end?
    if( c_was_r0_end ) {
      txt.c = moved_r0_end;
    }
    if( c_was_r1_end ) {
      txt.c = moved_r1_end;
    }
#else // !USE_SWAP_FOR_PERMUTELINES
    auto r0_len = CountCharsBetween( txt.buf, r0_start, r0_end );
    content_ptr_t* concurrents[] = {
      &r0_start,
      &r0_end,
      &r1_start,
      &r1_end,
      __TxtConcurrentPtrs( txt )
    };
    // r0 newline r1 => r0 r1 newline
    Move(
      txt.buf,
      r0_end, // src_start
      r1_start, // src_end
      r1_end, // dst
      AL( concurrents )
      );
    // r0 r1 newline => r1 newline r0
    Move(
      txt.buf,
      r0_start, // src_start
      CursorCharR( txt.buf, r0_start, r0_len, 0 ), // src_end
      r1_end, // dst
      AL( concurrents )
      );

    if( r0_len ) {
      txt.c = CursorCharR( txt.buf, r0_start, c_inline, 0 );
    } else {
      txt.c = r1_end;
    }
#endif // !USE_SWAP_FOR_PERMUTELINES
  }
}

__TxtCmd( CmdPermuteD )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      PermuteD( txt );
    } break;
    case seltype_t::s: // defer to multiselect implementation. this works because c, s are the same across sel, multisel!
    case seltype_t::m: {
      MultiselectPermuteD( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdTxtUndo )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  if( !txt.undos_txtcs.len ) {
    return;
  }

  cs_undo_t state;
  CsUndoFromTxt( txt, &state );
  *AddBack( txt.redos_txtcs ) = state;

  // ApplyCsUndo depends on buf.content_len, so we need to update buf first!
  Undo( txt.buf );

  cs_undo_t undo = txt.undos_txtcs.mem[txt.undos_txtcs.len - 1];
  RemBack( txt.undos_txtcs );
  ApplyCsUndo( txt, undo );

  MakeCursorVisible( txt );
}

__TxtCmd( CmdTxtRedo )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  if( !txt.redos_txtcs.len ) {
    return;
  }

  cs_undo_t state;
  CsUndoFromTxt( txt, &state );
  *AddBack( txt.undos_txtcs ) = state;

  // ApplyCsUndo depends on buf.content_len, so we need to update buf first!
  Redo( txt.buf );

  cs_undo_t redo = txt.redos_txtcs.mem[txt.redos_txtcs.len - 1];
  RemBack( txt.redos_txtcs );
  ApplyCsUndo( txt, redo );

  MakeCursorVisible( txt );
}



// TODO: move to os interface.
Inl void
SendToClipboardText( u8* text, idx_t text_len )
{
  HGLOBAL win_mem = {};

  HWND hwnd = GetForegroundWindow();
  AssertWarn( hwnd );
  if( !hwnd ) {
    return;
  }

  idx_t retry = 0;
  while( retry < 100  &&  !OpenClipboard( hwnd ) ) {
    TimeSleep( 1 ); // block until we get the clipboard.
    retry += 1;
  }

  AssertWarn( EmptyClipboard() );

  // we have to go thru windows-specific allocator.
  win_mem = GlobalAlloc( GMEM_MOVEABLE, text_len + 1 );
  AssertWarn( win_mem );

  u8* tmp_mem = Cast( u8*, GlobalLock( win_mem ) );
  AssertWarn( tmp_mem );
  Memmove( tmp_mem, text, text_len );
  Memmove( tmp_mem + text_len, "", 1 ); // insert nul-terminator for CF_OEMTEXT prereq.
  GlobalUnlock( win_mem );

  HANDLE hclip = SetClipboardData( CF_OEMTEXT, win_mem );
  AssertWarn( hclip );

  AssertWarn( CloseClipboard() );

  // cannot be freed before CloseClipboard call.
  GlobalFree( win_mem );
}


#define USECLIPBOARDTXT( name )   void ( name )( u8* text, idx_t text_len, void* misc )
typedef USECLIPBOARDTXT( *pfn_useclipboardtxt_t );

// TODO: move to os interface.
Inl void
GetFromClipboardText( pfn_useclipboardtxt_t UseClipboardTxt, void* misc )
{
  HWND hwnd = GetForegroundWindow();
  AssertWarn( hwnd );
  if( !hwnd ) {
    return;
  }

  idx_t retry = 0;
  while( retry < 100  &&  !OpenClipboard( hwnd ) ) {
    TimeSleep( 1 ); // block until we get the clipboard.
    retry += 1;
  }

  // TODO: why can't we retrieve OEMTEXT?
  HANDLE clip = GetClipboardData( CF_TEXT );
  if( clip ) {
    u8* text = Cast( u8*, GlobalLock( clip ) );
    idx_t text_len = CsLen( text );
    UseClipboardTxt( text, text_len, misc );
    GlobalUnlock( clip );
  } else {
    UseClipboardTxt( 0, 0, misc );
  }

  AssertWarn( CloseClipboard() );
}

Inl
USECLIPBOARDTXT( UseClipboardTxtFillBuf )
{
  auto& buf = *Cast( buf_t*, misc );
  auto bof = GetBOF( buf );
  auto eof = GetEOF( buf );
  Delete( buf, bof, eof, 0, 0 );
  Insert( buf, bof, text, text_len, 0, 0 );
}

Inl void
GetFromClipboardText( buf_t* buf )
{
  GetFromClipboardText( UseClipboardTxtFillBuf, buf );
}



Inl void
MultiselectCopy( txt_t& txt, multisel_t& ms )
{
  AssertCrash( ms.left_inline != ms.rght_inline );

  array_t<u8> clip;
  Alloc( clip, 32768 ); // PERF: smarter allocation size

  auto ln_start = ms.bot_ln_start;
  Forever {
    // the selection range on the given line ln_start.
    idx_t ninline;
    auto ml = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
    idx_t ninline_right;
    auto mr = CursorCharInlineR( txt.buf, ln_start, ms.rght_inline, &ninline_right );

    // TODO: os specific - windows. probably allow different options for what eol to send to clipboard.
    u8* eol = Str( "\r\n" );
    idx_t eol_len = 2;

    AssertCrash( ninline <= ninline_right );
    auto copy_len = ninline_right - ninline;
    if( copy_len ) {
      auto copy = AddBack( clip, copy_len );
      Contents( txt.buf, ml, copy, copy_len );
    }
    auto dst_eol = AddBack( clip, eol_len );
    Memmove( dst_eol, eol, eol_len );

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  SendToClipboardText( ML( clip ) );

  Free( clip );
}

Inl void
SelectCopy( txt_t& txt )
{
  content_ptr_t sl, sr;
  GetSelect( txt, &sl, &sr );

  auto sel_len = CountCharsBetween( txt.buf, sl, sr );
  if( sel_len ) {

    array_t<u8> clip;
    Alloc( clip, sel_len );

    constant idx_t c_chunk_size = 4096;
    idx_t n0 = sel_len / c_chunk_size;
    idx_t n1 = sel_len % c_chunk_size;

    auto pos = sl;

    // TODO: don't use buf_t newlines; insert CRLF explicitly!

    // dump buf_t data within sl,sr into clip.
    while( n0-- ) {
      auto dst = AddBack( clip, c_chunk_size );
      pos = Contents( txt.buf, pos, dst, c_chunk_size );
    }
    auto dst = AddBack( clip, n1 );
    pos = Contents( txt.buf, pos, dst, n1 );

    SendToClipboardText( ML( clip ) );

    Free( clip );
  }
}

__TxtCmd( CmdCopy )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  switch( txt.seltype ) {
    case seltype_t::none: {
    } break;
    case seltype_t::s: {
      SelectCopy( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectCopy( txt, ms );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdCut )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
    } break;
    case seltype_t::s: {
      SelectCopy( txt );
      SelectDelete( txt );
    } break;
    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      if( ms.left_inline != ms.rght_inline ) {
        MultiselectCopy( txt, ms );
        MultiselectDelete( txt, ms );
      }
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}


Inl void
MultiselectPaste( txt_t& txt )
{
  multisel_t ms;
  GetMultiselect( txt, ms );

  if( ms.left_inline != ms.rght_inline ) {
    MultiselectDelete( txt, ms );
    // txt.c/c_inline has probably changed, so ms.left_inline/rght_inline will be different!
    // so, re-get the ms so MultiselectAddChar can work!
    GetMultiselect( txt, ms );
  }

  AssertCrash( ms.left_inline == ms.rght_inline );

  buf_t to_paste;
  Init( to_paste );
  BufLoadEmpty( to_paste );
  GetFromClipboardText( &to_paste );

  idx_t lns_to_skip = 0;

  auto ln_start = ms.bot_ln_start;
  Forever {
    auto ln_end = CursorStopAtNewlineR( txt.buf, ln_start, 0 );

    auto paste_start = CursorLineD( to_paste, GetBOF( to_paste ), 0, lns_to_skip, 0, 0 );
    lns_to_skip += 1;

    auto paste_end = CursorStopAtNewlineR( to_paste, paste_start, 0 );
    auto paste_contents = AllocContents( to_paste, paste_start, paste_end );

    if( paste_contents.len ) {
      idx_t ninline;
      auto pos = CursorCharInlineR( txt.buf, ln_start, ms.left_inline, &ninline );
      AssertCrash( LEqual( ln_start, pos ) );
      content_ptr_t* concurrents[] = {
        &paste_start,
        &paste_end,
        &pos,
        &ln_start,
        &ln_end,
        __MultiselConcurrentPtrs( ms ),
        __TxtConcurrentPtrs( txt )
      };
      if( ninline < ms.left_inline ) {
        auto nspaces = ms.left_inline - ninline;
        while( nspaces-- ) {
          Insert(
            txt.buf,
            ln_end,
            Str( " " ),
            1,
            AL( concurrents )
            );
        }
      }
      AssertCrash( LEqual( pos, ln_end ) );

      Insert(
        txt.buf,
        pos,
        ML( paste_contents ),
        AL( concurrents )
        );
    }
    Free( paste_contents );

    if( Greater( ln_start, ms.top_ln_start ) ) {
      ln_start = CursorLineU( txt.buf, ln_start, 0, 1, 0, 0 );
    } else {
      AssertCrash( Equal( ln_start, ms.top_ln_start ) );
      break;
    }
  }

  auto ln0_start = GetBOF( to_paste );
  idx_t dx;
  auto ln0_end = CursorStopAtNewlineR( to_paste, ln0_start, &dx );
  Kill( to_paste );

  txt.c_inline += dx;
}

Inl
USECLIPBOARDTXT( UseClipboardTxtPaste )
{
  auto& txt = *Cast( txt_t*, misc );

  content_ptr_t* concurrents[] = {
    __TxtConcurrentPtrs( txt )
  };
  Insert( txt.buf, txt.c, text, text_len, AL( concurrents ) );
  ResetCInline( txt );
}

Inl void
Paste( txt_t& txt )
{
  GetFromClipboardText( UseClipboardTxtPaste, &txt );
}

__TxtCmd( CmdPaste )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddTxtUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      Paste( txt );
    } break;
    case seltype_t::s: {
      SelectDelete( txt );
      Paste( txt );
    } break;
    case seltype_t::m: {
      MultiselectPaste( txt );
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}


#if 0
__TxtCmd( CmdFindCursorWordR )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      idx_t word_start = CursorStopAtNonWordCharL( txt.buf, txt.c );
      idx_t word_end = CursorStopAtNonWordCharR( txt.buf, txt.c );
      idx_t word_len = word_end - word_start;
      if( word_len ) {
        auto word = MemHeapAlloc( u8, word_len );
        Contents( txt.buf, word_start, word, word_len );
        idx_t new_c;
        bool found;
        FindFirstR( txt.buf, word_end, word, word_len, &new_c, &found, 1 );
        if( found ) {
          txt.c = new_c;
          ResetCInline( txt );
        }
        MemHeapFree( word );
      }
    } break;
    case seltype_t::s: {
      // TODO: find next selection match?
    } break;
    case seltype_t::m: {
      // TODO: do what?
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}

__TxtCmd( CmdFindCursorWordL )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddCsUndo( txt );
  switch( txt.seltype ) {
    case seltype_t::none: {
      idx_t word_start = CursorStopAtNonWordCharL( txt.buf, txt.c );
      idx_t word_end = CursorStopAtNonWordCharR( txt.buf, txt.c );
      idx_t word_len = word_end - word_start;
      if( word_len ) {
        auto word = MemHeapAlloc( u8, word_len );
        Contents( txt.buf, word_start, word, word_len );
        idx_t new_c;
        bool found;
        FindFirstL( txt.buf, word_start, word, word_len, &new_c, &found, 1 );
        if( found ) {
          txt.c = new_c;
          ResetCInline( txt );
        }
        MemHeapFree( word );
      }
    } break;
    case seltype_t::s: {
      // TODO: find next selection match?
    } break;
    case seltype_t::m: {
      // TODO: do what?
    } break;
    default: UnreachableCrash();
  }
  MakeCursorVisible( txt );
}
#endif


struct
txtfind_t
{
  u8* str;
  idx_t str_len;
  bool case_sens;
  bool word_boundary;
};

__TxtCmd( CmdFindStringR )
{
  AssertCrash( !misc2 );

  auto& txtfind = *Cast( txtfind_t*, misc );
  AddCsUndo( txt );
  if( txtfind.str_len ) {
    content_ptr_t new_c;
    bool found;
    FindFirstR(
      txt.buf,
      txt.c,
      txtfind.str,
      txtfind.str_len,
      &new_c,
      &found,
      txtfind.case_sens,
      txtfind.word_boundary
      );
    if( found ) {
      txt.c = new_c;
      AnchorSelect( txt, txt.c );
      txt.c = CursorCharR( txt.buf, txt.c, txtfind.str_len, 0 );
      ResetCInline( txt );
    }
  }
  MakeCursorVisible( txt );
}

__TxtCmd( CmdFindStringL )
{
  AssertCrash( !misc2 );

  auto& txtfind = *Cast( txtfind_t*, misc );
  AddCsUndo( txt );
  if( txtfind.str_len ) {
    content_ptr_t new_c;
    bool found;
    FindFirstL(
      txt.buf,
      txt.c,
      txtfind.str,
      txtfind.str_len,
      &new_c,
      &found,
      txtfind.case_sens,
      txtfind.word_boundary
      );
    if( found ) {
      txt.c = new_c;
      AnchorSelect( txt, txt.c );
      txt.c = CursorCharL( txt.buf, txt.c, txtfind.str_len, 0 );
      ResetCInline( txt );
    }
  }
  MakeCursorVisible( txt );
}



__TxtCmd( CmdRemTrailspace )
{
  AssertCrash( !misc );
  AssertCrash( !misc2 );

  AddTxtUndo( txt );
  auto bol = GetBOF( txt.buf );
  Forever {
    auto eol = CursorStopAtNewlineR( txt.buf, bol, 0 );
    auto new_eol = CursorSkipSpacetabL( txt.buf, eol, 0 );
    if( Greater( eol, new_eol ) ) {
      content_ptr_t* concurrents[] = {
        &bol,
        &eol,
        &new_eol,
        __TxtConcurrentPtrs( txt )
      };
      Delete( txt.buf, new_eol, eol, AL( concurrents ) );
    }
    auto new_bol = CursorSingleNewlineR( txt.buf, eol, 0 );
    if( Equal( new_bol, bol ) ) {
      break;
    }
    bol = new_bol;
  }
  ResetCInline( txt );
}


// =================================================================================
// TXT DRAW CALLS
//

// loop:
//   pull window_n_chars from txt.buf at pos
//   render those chars.
//   increment y.
//   if word wrap, then
//     pos += window_n_chars.
//   else
//     pos = scan( pos+1, \n ).
//
// really, we want to output per-char render commands, and then combine as much as possible.
//   struct drawchar_t
//   {
//     u8* c; // points to
//     vec4<f32>* color;
//     u16 x; // window space line pos., u16 for safe drawchar_t <-> drawchar_t comparison.
//     u16 y; // window space line no., u16 for safe drawchar_t <-> drawchar_t comparison.
//   };



//Inl f32
//GetPxLineno( txt_t& txt, f32 FontCharW )
//{
//  if( !opts.lineno_show )
//    return 0.0f;
//  u8 tmp_str [ 32 ];
//  if( opts.lineno_fix_width )
//    Cstr::cstr_from_u64( tmp_str, nlines( *txt.buf ) - 1 );
//  else
//    Cstr::cstr_from_u64( tmp_str, txt.scroll.ln_end - 1 );
//  u8 LinenoDigits = Cast( u8, CsLen( tmp_str ) );
//  f32 PxLineno = FontCharW * ( LinenoDigits + 1 );
//  return PxLineno;
//}


Inl void
TxtUpdateScrolling(
  txt_t& txt,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  f64 timestep_realtime,
  f64 timestep_fixed
  )
{
  ProfFunc();

  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );

  auto line_h = FontLineH( font );
  auto timestep = MIN( timestep_realtime, timestep_fixed );

  txt.window_n_lines = Cast( u16, dim.y / line_h );

  auto bof = GetBOF( txt.buf );
  auto eof = GetEOF( txt.buf );
  AssertCrash( LEqual( txt.scroll_start.y, eof ) );

  Prof( TxtUpdateScrollingVertical );

  // ensure these are on line starts.
  txt.scroll_start.y = CursorStopAtNewlineL( txt.buf, txt.scroll_start.y, 0 );
  txt.scroll_target.y = CursorStopAtNewlineL( txt.buf, txt.scroll_target.y, 0 );

  // =================================================================================
  //
  // SCROLLING
  {
    // reduce fractional position to integer position.
    s32 target_dlines = Cast( s32, txt.scroll_target.frac );
    txt.scroll_target.frac -= target_dlines;
    if( target_dlines < 0 ) {
      txt.scroll_target.y = CursorLineU( txt.buf, txt.scroll_target.y, 0, -target_dlines, 0, 0 );
    } elif( target_dlines > 0 ) {
      txt.scroll_target.y = CursorLineD( txt.buf, txt.scroll_target.y, 0, target_dlines, 0, 0 );
    }

    scrollpos_t scroll_half = txt.scroll_start;
    scroll_half.frac += 0.5 * txt.window_n_lines;

    auto target_dist_yl = Min( txt.scroll_target.y, scroll_half.y );
    auto target_dist_yr = Max( txt.scroll_target.y, scroll_half.y );
    f64 target_dist_sign = Less( scroll_half.y, txt.scroll_target.y )  ?  1  :  -1;
    auto target_dist_nlines = CountLinesBetween( txt.buf, target_dist_yl, target_dist_yr );
    auto target_dist = target_dist_sign * Cast( f64, target_dist_nlines );
    target_dist += txt.scroll_target.frac - scroll_half.frac;

    // PERF: until we optimize line traversal, smooth scrolling large distances is too slow.
    // eg 0.6612 sec spent in CmdCursorGotoline, and 1.088 sec spent in TxtUpdateScrollingVertical here.
    // so, avoid the smooth scrolling by jumping straight to the destination.

    if( 10.0 * txt.window_n_lines < ABS( target_dist ) ) {
      txt.scroll_start.frac += target_dist;

    } else {
      constant f64 mass = 1.0;
      constant f64 spring_k = 500.0;
      static f64 friction_k = 2.2 * Sqrt64( mass * spring_k ); // 2 * Sqrt( m * k ) is critical damping, but we want a little overdamping.

      f64 force_spring = spring_k * target_dist;
      f64 force_fric = -friction_k * txt.scroll_vel;
      f64 force = force_spring + force_fric;

      // TODO: solve ODE and use explicit soln.

      f64 accel = force / mass;
      f64 delta_vel = timestep * accel;
      txt.scroll_vel += delta_vel;

      // snap to 0 for small velocities to minimize pixel jitter.
      if( ABS( txt.scroll_vel ) <= 2.0f ) {
        txt.scroll_vel = 0;
      }

      f64 delta_pos = timestep * txt.scroll_vel;
      txt.scroll_start.frac += delta_pos;
    }

    auto eof_ln_start = CursorStopAtNewlineL( txt.buf, eof, 0 );
    bool at_bof = Equal( txt.scroll_start.y, bof );
    bool at_eof = Equal( txt.scroll_start.y, eof_ln_start );
    if( at_bof ) {
      txt.scroll_start.frac = MAX( txt.scroll_start.frac, -0.5 * txt.window_n_lines );
    }
    if( at_eof ) {
      txt.scroll_start.frac = MIN( txt.scroll_start.frac, 0.5 * txt.window_n_lines );
    }

    s32 to_scroll = Cast( s32, txt.scroll_start.frac );
    bool multi_line = !at_bof  ||  !at_eof;
    bool allow_bof_reduce = at_bof  &&  to_scroll > 0;
    bool allow_eof_reduce = at_eof  &&  to_scroll < 0;
    bool allow_mid_reduce = !at_bof  &&  !at_eof;
    bool allow_reduce = multi_line  &&  ( allow_bof_reduce | allow_eof_reduce | allow_mid_reduce );
    if( allow_reduce ) {
      // reduce fractional position to integer position.
      txt.scroll_start.frac -= to_scroll; // take only fractional part.

      if( to_scroll < 0 ) {
        txt.scroll_start.y = CursorLineU( txt.buf, txt.scroll_start.y, 0, -to_scroll, 0, 0 );
      } elif( to_scroll > 0 ) {
        txt.scroll_start.y = CursorLineD( txt.buf, txt.scroll_start.y, 0, to_scroll, 0, 0 );
      }
    }
#if 0
    // don't let scrolling go past first line or last line, fractionally.
    bool scroll_on_bof = ( !txt.scroll_start.y );
    bool scroll_on_eof = ( txt.scroll_start.y == eof_ln_start );
    if( scroll_on_bof ) {
      txt.scroll_start.frac = MAX( txt.scroll_start.frac, 0 );
      if( txt.scroll_vel < 0 ) {
        txt.scroll_vel = 0;
      }
    }
    if( scroll_on_eof ) {
      txt.scroll_start.frac = MIN( txt.scroll_start.frac, 0 );
      if( txt.scroll_vel > 0 ) {
        txt.scroll_vel = 0;
      }
    }
#endif
  }

  auto scroll_end = CursorLineD( txt.buf, txt.scroll_start.y, 0, txt.window_n_lines + 1, 0, 0 );
  txt.scroll_end = CursorStopAtNewlineR( txt.buf, scroll_end, 0 );

  ProfClose( TxtUpdateScrollingVertical );

  // horizontal scrolling

  Prof( TxtUpdateScrollingHorizontal );

  auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );
  auto temp = AllocContents( txt.buf, ln_start, txt.c );
  auto offset = LayoutString( font, spaces_per_tab, ML( temp ) );

  if( offset > txt.scroll_x + 0.8f * dim.x ) {
    txt.scroll_x = offset - 0.8f * dim.x;
  } elif( offset < txt.scroll_x + 0.2f * dim.x ) {
    txt.scroll_x = MAX( 0, offset - 0.2f * dim.x );
  }
  Free( temp );

  ProfClose( TxtUpdateScrollingHorizontal );
}


constant idx_t c_max_line_len = 40; // sanity bounds.

void
TxtLayout(
  txt_t& txt,
  font_t& font
  )
{
  ProfFunc();

  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );

  FontClear( txt.layout );

  auto line_start = txt.scroll_start.y;
  line_start = CursorLineU( txt.buf, line_start, 0, 1, 0, 0 );

  u8 line[c_max_line_len];

  Forever {
    idx_t line_len;
    auto line_end = CursorStopAtNewlineR( txt.buf, line_start, &line_len );
    line_len = MIN( c_max_line_len, line_len ); // account for hard end-clipping
    Contents( txt.buf, line_start, line, line_len );

    if( line_len ) {
      // add the line text to the layout_t.
      FontAddLayoutLine(
        font,
        txt.layout,
        line,
        line_len,
        spaces_per_tab
        );
    }

    auto line_start_next = CursorSingleNewlineR( txt.buf, line_end, 0 );
    if( Equal( line_end, line_start_next ) | Greater( line_start_next, txt.scroll_end ) ) {
      break;
    }
    line_start = line_start_next;
  }
}

void
TxtLayoutSingleLineSubset(
  txt_t& txt,
  content_ptr_t start,
  idx_t len,
  font_t& font
  )
{
  ProfFunc();

  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );

  FontClear( txt.layout );

  if( len ) {
    u8 line[c_max_line_len];
    auto line_len = MIN( len, c_max_line_len ); // account for hard end-clipping
    Contents( txt.buf, start, line, line_len );

    FontAddLayoutLine(
      font,
      txt.layout,
      line,
      line_len,
      spaces_per_tab
      );
  }
}




void
TxtRenderSelection(
  txt_t& txt,
  array_t<f32>& stream,
  vec2<f32> origin,
  vec2<f32> dim,
  content_ptr_t line_start,
  content_ptr_t line_end,
  f32 px_space_advance,
  idx_t nchars,
  idx_t layout_line_idx,
  f32 x,
  f32 y,
  f32 z,
  f32 line_h
  )
{
  auto rgba_selection_bkgd = GetPropFromDb( vec4<f32>, rgba_selection_bkgd );
  auto px_cursor_w = GetPropFromDb( u8, u8_px_cursor_w );

  switch( txt.seltype ) {
    case seltype_t::none: {
    } break;

    case seltype_t::s: {
      content_ptr_t sl, sr;
      GetSelect( txt, &sl, &sr );
      // draw s_selection on line.
      auto sel_start = Max( line_start, sl );
      auto sel_end   = Min( line_end, sr );
      // fixup sel_end to show newlines inside the sel region.
      f32 overhang = 0.0f;
      if( Equal( line_end, sel_end )  &&  !Equal( line_end, sr ) ) {
        overhang = px_space_advance;
      }
      if( LEqual( sel_start, sel_end ) ) {
        auto sel_x0 = 0.0f;
        auto sel_x1 = 0.0f;
        if( nchars ) {
          auto nbeforesel = CountCharsBetween( txt.buf, line_start, sel_start );
          auto sel_len = CountCharsBetween( txt.buf, sel_start, sel_end );

          // account for hard end-clipping
          nbeforesel = MIN( nchars, nbeforesel );
          sel_len = MIN( nchars - nbeforesel, sel_len );

          sel_x0 = FontSumAdvances(
            txt.layout,
            layout_line_idx,
            0,
            nbeforesel
            );
          sel_x1 = sel_x0 + FontSumAdvances(
            txt.layout,
            layout_line_idx,
            nbeforesel,
            sel_len
            );
        }
        auto s0 = _vec2( x + sel_x0, y );
        auto s1 = _vec2( x + sel_x1 + overhang, y + line_h );
        RenderQuad(
          stream,
          rgba_selection_bkgd,
          origin + s0,
          origin + s1,
          origin, dim,
          z
          );
      }
    } break;

    case seltype_t::m: {
      multisel_t ms;
      GetMultiselect( txt, ms );
      // draw m_selection on line.
      auto sel_start = Min( CursorCharR( txt.buf, line_start, ms.left_inline, 0 ), line_end );
      auto sel_end   = Min( CursorCharR( txt.buf, line_start, ms.rght_inline, 0 ), line_end );
      bool m_x_valid = Less( sel_start, sel_end );
      bool m_y_valid = LEqual( ms.top_ln_start, line_start ) & LEqual( line_start, ms.bot_ln_start );
      bool m_draw = ( m_x_valid  &&  m_y_valid );
      if( m_draw ) {
        auto sel_x0 = 0.0f;
        auto sel_x1 = 0.0f;
        if( nchars ) {
          auto nbeforesel = CountCharsBetween( txt.buf, line_start, sel_start );
          auto sel_len = CountCharsBetween( txt.buf, sel_start, sel_end );

          // account for hard end-clipping
          nbeforesel = MIN( nchars, nbeforesel );
          sel_len = MIN( nchars - nbeforesel, sel_len );

          sel_x0 = FontSumAdvances(
            txt.layout,
            layout_line_idx,
            0,
            nbeforesel
            );
          sel_x1 = sel_x0 + FontSumAdvances(
            txt.layout,
            layout_line_idx,
            nbeforesel,
            sel_len
            );
        }
        auto s0 = _vec2( x + sel_x0, y );
        auto s1 = _vec2( x + sel_x1, y + line_h );
        if( Equal( sel_start, sel_end ) ) {
          s1.x += px_cursor_w;
        }
        RenderQuad(
          stream,
          rgba_selection_bkgd,
          origin + s0,
          origin + s1,
          origin, dim,
          z
          );
      }
    } break;
    default: UnreachableCrash();
  }
}

void
TxtRenderCursor(
  txt_t& txt,
  array_t<f32>& stream,
  vec2<f32> origin,
  vec2<f32> dim,
  content_ptr_t line_start,
  content_ptr_t line_end,
  f32 px_space_advance,
  idx_t nchars,
  idx_t layout_line_idx,
  f32 x,
  f32 y,
  f32 z,
  f32 line_h
  )
{
  auto px_cursor_w = GetPropFromDb( u8, u8_px_cursor_w );

  if( LEqual( line_start, txt.c ) & LEqual( txt.c, line_end ) ) {
    f32 cursor_advance = 0.0f;
    if( nchars ) {
      auto nbeforecur = CountCharsBetween( txt.buf, line_start, txt.c );

      // account for hard end-clipping
      nbeforecur = MIN( nchars, nbeforecur );

      cursor_advance = FontSumAdvances(
        txt.layout,
        layout_line_idx,
        0,
        nbeforecur
        );
    }

    f32 cx = x + cursor_advance;

    vec2<f32> p0, p1;
    if( txt.overwrite ) {
      p0 = origin + _vec2( cx, y + line_h );
      p1 = origin + _vec2( cx + px_space_advance, y + line_h + px_cursor_w );
    } else {
      p0 = origin + _vec2( cx, y );
      p1 = origin + _vec2( cx + px_cursor_w, y + line_h );
    }
    RenderQuad(
      stream,
      GetPropFromDb( vec4<f32>, rgba_cursor ),
      p0,
      p1,
      origin, dim,
      z
      );
  }
}





Enumc( txtlayer_t )
{
  bkgd,
  cursorline,
  match,
  sel,
  txt,
  cur,
  scroll_bkgd,
  scroll_btn,

  COUNT
};


// NOTE: because of layout_t, you can't use both EditRender and RenderSingleLineSubset on the same txt_t !

void
TxtRenderSingleLineSubset(
  txt_t& txt,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  bool draw_bkgd,
  bool draw_cursor,
  bool draw_sel
  )
{
  ProfFunc();

  auto rgba_text = GetPropFromDb( vec4<f32>, rgba_text );

  auto line_h = FontLineH( font );
  auto px_space_advance = FontGetAdvance( font, ' ' );

  if( draw_bkgd ) {
    RenderQuad(
      stream,
      GetPropFromDb( vec4<f32>, rgba_text_bkgd ),
      origin,
      origin + dim,
      origin, dim,
      GetZ( zrange, txtlayer_t::bkgd )
      );
  }

  if( !FontEmpty( txt.layout ) ) {
    RenderText(
      stream,
      font,
      txt.layout,
      rgba_text,
      origin, dim,
      origin,
      GetZ( zrange, txtlayer_t::txt ),
      0,
      0,
      txt.layout.raw_advances.len
      );
  }

  // draw selection.
  auto bof = GetBOF( txt.buf );
  auto eof = GetEOF( txt.buf );
  if( draw_sel ) {
    TxtRenderSelection(
      txt,
      stream,
      origin,
      dim,
      bof,
      eof,
      px_space_advance,
      TxtLen( txt ),
      0,
      0.0f,
      0.0f,
      GetZ( zrange, txtlayer_t::sel ),
      line_h
      );
  }

  // draw cursor.
  if( draw_cursor ) {
    TxtRenderCursor(
      txt,
      stream,
      origin,
      dim,
      bof,
      eof,
      px_space_advance,
      TxtLen( txt ),
      0,
      0.0f,
      0.0f,
      GetZ( zrange, txtlayer_t::cur ),
      line_h
      );
  }
}



struct
wordspan_t
{
  content_ptr_t l;
  content_ptr_t r;
  bool is_ident;
};


Inl f32
EstimateLinearPos( txt_t& txt, content_ptr_t pos )
{
  ProfFunc();

  if( IsEOF( txt.buf, pos ) ) {
    return 1;
  }

  idx_t cumulative_sum = 0;
  ForLen( i, txt.buf.diffs ) { // PERF: this is slow / bad for large diff counts.
    auto diff = txt.buf.diffs.mem + i;
    if( i < pos.diff_idx ) {
      cumulative_sum += diff->slice.len;
    }
  }

  AssertCrash( pos.diff_idx < txt.buf.diffs.len );
  auto diff = txt.buf.diffs.mem + pos.diff_idx;
  AssertCrash( pos.offset_into_diff <= diff->slice.len );
  AssertCrash( diff->slice.len );
  auto t = Cast( f32, cumulative_sum + pos.offset_into_diff ) / Cast( f32, txt.buf.content_len );
  auto linear_pos = CLAMP( t, 0, 1 );
  return linear_pos;
}

Inl vec3<f32>
GetScrollPos( txt_t& txt )
{
  return _vec3<f32>(
    CLAMP( EstimateLinearPos( txt, txt.scroll_start.y ), 0, 1 ),
    CLAMP( EstimateLinearPos( txt, txt.c ), 0, 1 ),
    CLAMP( EstimateLinearPos( txt, txt.scroll_end ), 0, 1 )
    );
}

Inl f32
GetScrollTarget( txt_t& txt )
{
  return CLAMP( EstimateLinearPos( txt, txt.scroll_target.y ), 0, 1 );
}

Inl bool
ScrollbarVisible(
  vec2<f32> dim,
  f32 px_scroll
  )
{
  bool r = dim.x > 2 * px_scroll  &&  dim.y > 4 * px_scroll;
  return r;
}

Inl void
GetScrollBtnUp(
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec2<f32> origin,
  vec2<f32> dim,
  f32 px_scroll
  )
{
  p0 = origin + _vec2( dim.x - px_scroll, 0.0f ) + _vec2( 1.0f );
  p1 = origin + _vec2( dim.x, px_scroll ) - _vec2( 1.0f );
}

Inl void
GetScrollBtnDn(
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec2<f32> origin,
  vec2<f32> dim,
  f32 px_scroll
  )
{
  p0 = origin + dim - _vec2( px_scroll ) + _vec2( 1.0f );
  p1 = origin + dim - _vec2( 1.0f );
}

Inl void
GetScrollBtnPos(
  vec2<f32>& p0,
  vec2<f32>& p1,
  vec3<f32> t,
  vec2<f32> origin,
  vec2<f32> dim,
  f32 px_scroll
  )
{
  auto track_len = dim.y - 3 * px_scroll;
  p0 = origin + _vec2( dim.x - px_scroll, px_scroll + track_len * t.x ) + _vec2( 1.0f );
  p1 = origin + _vec2( dim.x, 2 * px_scroll + track_len * t.z ) - _vec2( 1.0f );
}



void
TxtRender(
  txt_t& txt,
  bool& target_valid,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  bool draw_cursor,
  bool draw_cursorline,
  bool draw_cursorwordmatch,
  bool allow_scrollbar
  )
{
  ProfFunc();

  // invalidate cached target, since we know animation will require a re-render.
  if( ABS( txt.scroll_vel ) > 1e-2f ) {
    target_valid = 0;
  }

  auto rgba_text = GetPropFromDb( vec4<f32>, rgba_text );
  auto rgba_wordmatch_text = GetPropFromDb( vec4<f32>, rgba_wordmatch_text );
  auto rgba_wordmatch_bkgd = GetPropFromDb( vec4<f32>, rgba_wordmatch_bkgd );

  auto scroll_pct = GetPropFromDb( f32, f32_scroll_pct );
  auto px_scroll = MAX( 16.0f, Round32( scroll_pct * MIN( dim.x, dim.y ) ) );

  auto line_h = FontLineH( font );
  auto px_space_advance = FontGetAdvance( font, ' ' );

  // render the scrollbar.
  if( allow_scrollbar  &&  ScrollbarVisible( dim, px_scroll ) ) {
    auto t = GetScrollPos( txt );

    auto rgba_scroll_btn = GetPropFromDb( vec4<f32>, rgba_scroll_btn );

    RenderQuad(
      stream,
      GetPropFromDb( vec4<f32>, rgba_scroll_bkgd ),
      origin + _vec2( dim.x - px_scroll, 0.0f ),
      origin + dim,
      origin, dim,
      GetZ( zrange, txtlayer_t::scroll_bkgd )
      );

    vec2<f32> btn_up0, btn_up1;
    GetScrollBtnUp( btn_up0, btn_up1, origin, dim, px_scroll );
    RenderQuad(
      stream,
      rgba_scroll_btn,
      btn_up0,
      btn_up1,
      origin, dim,
      GetZ( zrange, txtlayer_t::scroll_btn )
      );

    vec2<f32> btn_dn0, btn_dn1;
    GetScrollBtnDn( btn_dn0, btn_dn1, origin, dim, px_scroll );
    RenderQuad(
      stream,
      rgba_scroll_btn,
      btn_dn0,
      btn_dn1,
      origin, dim,
      GetZ( zrange, txtlayer_t::scroll_btn )
      );

    vec2<f32> btn_pos0, btn_pos1;
    GetScrollBtnPos( btn_pos0, btn_pos1, t, origin, dim, px_scroll );
    RenderQuad(
      stream,
      rgba_scroll_btn,
      btn_pos0,
      btn_pos1,
      origin, dim,
      GetZ( zrange, txtlayer_t::scroll_btn )
      );

    dim.x -= px_scroll;
  }

  // draw bkgd
  RenderQuad(
    stream,
    GetPropFromDb( vec4<f32>, rgba_text_bkgd ),
    origin,
    origin + dim,
    origin, dim,
    GetZ( zrange, txtlayer_t::bkgd )
    );

  // =================================================================================
  //
  // OUTPUT

  auto word_l = CursorStopAtNonWordCharL( txt.buf, txt.c, 0 );
  auto word_r = CursorStopAtNonWordCharR( txt.buf, txt.c, 0 );
  auto word = AllocContents( txt.buf, word_l, word_r );

  f32 y0 = Cast( f32, -line_h * txt.scroll_start.frac );

  kahan32_t x;
  kahan32_t y = { y0 };

  auto line_start = txt.scroll_start.y;
  line_start = CursorLineU( txt.buf, line_start, 0, 1, 0, 0 );

  // don't actually offset for the additional line above, if there is one.
  if( !Equal( line_start, txt.scroll_start.y ) ) {
    Sub( y, line_h );
  }
  y.sum = Ceil32( y.sum );
  y.err = 0;

  // layout_t doesn't store empty lines, so we have to skip them when looking up the layout info.
  idx_t layout_line_idx = 0;

  Forever {
    idx_t line_len;
    auto line_end = CursorStopAtNewlineR( txt.buf, line_start, &line_len );
    line_len = MIN( c_max_line_len, line_len ); // account for hard end-clipping

    // WARNING!!!
    // line_end is potentially past what we'll layout and draw!
    // see the clamp to c_max_line_len.

    x = { 0 - txt.scroll_x };

    // draw cursorline.
    if( draw_cursorline ) {
      if( LEqual( line_start, txt.c ) & LEqual( txt.c, line_end ) ) {
        RenderQuad(
          stream,
          GetPropFromDb( vec4<f32>, rgba_cursorline_bkgd ),
          origin + _vec2( 0.0f, y.sum ),
          origin + _vec2( dim.x, y.sum + line_h ),
          origin, dim,
          GetZ( zrange, txtlayer_t::cursorline )
          );
      }
    }

    // draw line text.
    if( line_len ) {

      if( draw_cursorwordmatch  &&  word.len ) {

        auto linepos = origin + _vec2( x.sum, y.sum );

        array_t<wordspan_t> spans;
        Alloc( spans, 64 );
        {
          wordspan_t span;
          span.l = line_start;
          span.r = span.l;
          span.is_ident = 0;

          Forever {
            if( CountCharsBetween( txt.buf, line_start, span.l ) >= line_len ) {
              AssertCrash( spans.len );
              auto lastspan = spans.mem + spans.len - 1;
              auto clipped_line_end = CursorCharR( txt.buf, line_start, line_len, 0 );
              lastspan->l = Min( lastspan->l, clipped_line_end );
              lastspan->r = Min( lastspan->r, clipped_line_end );
              break;
            }
            span.r = CursorStopAtNonWordCharR( txt.buf, span.l, 0 );

            if( !Equal( span.l, span.r ) ) {
              span.is_ident = 1;
              *AddBack( spans ) = span;
              span.l = span.r;
              span.is_ident = 0;

            } else {
              auto sr = span.l;
              Forever {
                if( Equal( sr, line_end ) ) {
                  span.r = sr;
                  break;
                }
                sr = CursorCharR( txt.buf, sr, 1, 0 );
                span.r = CursorStopAtNonWordCharR( txt.buf, sr, 0 );
                if( !Equal( sr, span.r ) ) {
                  span.r = sr;
                  break;
                }
              }
              *AddBack( spans ) = span;
              span.l = span.r;
              span.is_ident = 1;
            }
          }
        }

        auto match_start_x = x;
        ForLen( i, spans ) {

          auto span = spans.mem + i;
          auto span_len = CountBytesBetween( txt.buf, span->l, span->r );
          AssertWarn( span_len );
          auto tmp = MemHeapAlloc( u8, span_len );
          Contents( txt.buf, span->l, tmp, span_len );
          bool is_match = 0;
          if( span->is_ident ) {
            is_match = MemEqual( tmp, span_len, ML( word ) );
          }
          MemHeapFree( tmp );

          vec4<f32> color;
          if( is_match ) {
            color = rgba_wordmatch_text;
          } else {
            color = rgba_text;
          }

          auto char_offset = CountCharsBetween( txt.buf, line_start, span->l );
          auto char_len = span_len;

          // TODO: don't OOB char_offset/char_len in txt.layout, which has the c_max_line_len limit!

          auto advance_x = FontSumAdvances(
            txt.layout,
            layout_line_idx,
            char_offset,
            char_len
            );

          // draw match background quad
          if( is_match ) {
            RenderQuad(
              stream,
              rgba_wordmatch_bkgd,
              origin + _vec2( match_start_x.sum, y.sum ),
              origin + _vec2( match_start_x.sum + advance_x, y.sum + line_h ),
              origin, dim,
              GetZ( zrange, txtlayer_t::match )
              );
          }

          // draw text.
          RenderText(
            stream,
            font,
            txt.layout,
            color,
            origin, dim,
            linepos,
            GetZ( zrange, txtlayer_t::txt ),
            layout_line_idx,
            char_offset,
            char_len
            );

          // horizontal advance.
          Add( match_start_x, advance_x );
        }
        Free( spans );

      } else { // !draw_cursorwordmatch
        RenderText(
          stream,
          font,
          txt.layout,
          rgba_text,
          origin, dim,
          origin + _vec2( x.sum, y.sum ),
          GetZ( zrange, txtlayer_t::txt ),
          layout_line_idx,
          0,
          line_len
          );
      }
    } // end if( line_len )

    // draw selection.
    TxtRenderSelection(
      txt,
      stream,
      origin,
      dim,
      line_start,
      line_end,
      px_space_advance,
      line_len,
      layout_line_idx,
      x.sum,
      y.sum,
      GetZ( zrange, txtlayer_t::sel ),
      line_h
      );

    // draw cursor.
    if( draw_cursor ) {
      TxtRenderCursor(
        txt,
        stream,
        origin,
        dim,
        line_start,
        line_end,
        px_space_advance,
        line_len,
        layout_line_idx,
        x.sum,
        y.sum,
        GetZ( zrange, txtlayer_t::cur ),
        line_h
        );
    } // end if( draw_cursor )

    // line advance.
    Add( y, line_h );
    y.sum = Ceil32( y.sum );
    y.err = 0;

    if( line_len ) {
      layout_line_idx += 1;
    }

    auto line_start_next = CursorSingleNewlineR( txt.buf, line_end, 0 );
    if( Equal( line_end, line_start_next )  ||  Greater( line_start_next, txt.scroll_end ) ) {
      break;
    }
    line_start = line_start_next;
  }

  Free( word );
}



// TXT INPUTS -> OPERATIONS DISPATCH.
//

struct
linerect_t
{
  vec2<f32> p0;
  vec2<f32> p1;
  content_ptr_t line_start;
};

Inl content_ptr_t
MapMouseToCursor(
  txt_t& txt,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<s32> m,
  font_t& font,
  vec2<s8> px_click_correct
  )
{
  // TODO: rendering should build up the datastructures we need for mouse hit testing.
  // i'm duplicating the relevant rendering code for now.
  // i've already had to redo this in a major way once; it'll likely happen again.
  // that'll also save some perf, since we don't have to do as much work on mouse move.

  array_t<linerect_t> linerects;
  Alloc( linerects, txt.window_n_lines + 2 ); // 1 extra on top and bot.

  {
    auto line_h = FontLineH( font );

    f32 y0 = Cast( f32, -line_h * txt.scroll_start.frac );

    kahan32_t x;
    kahan32_t y = { y0 };

    auto line_start = txt.scroll_start.y;
    line_start = CursorLineU( txt.buf, line_start, 0, 1, 0, 0 );

    // don't actually offset for the additional line above, if there is one.
    if( !Equal( line_start, txt.scroll_start.y ) ) {
      Sub( y, line_h );
    }
    y.sum = Ceil32( y.sum );
    y.err = 0;

    Forever {
      auto line_end = CursorStopAtNewlineR( txt.buf, line_start, 0 );

      x = { 0 - txt.scroll_x };

      // PERF: could save some cycles by comparing in origin-relative space.
      auto linerect = AddBack( linerects );
      linerect->p0 = origin + _vec2<f32>( 0, y.sum );
      linerect->p1 = origin + _vec2<f32>( dim.x, y.sum + line_h );
      linerect->line_start = line_start;

      // line advance.
      Add( y, line_h );
      y.sum = Ceil32( y.sum );
      y.err = 0;

      auto line_start_next = CursorSingleNewlineR( txt.buf, line_end, 0 );
      if( Equal( line_end, line_start_next )  ||  Greater( line_start_next, txt.scroll_end ) ) {
        break;
      }
      line_start = line_start_next;
    }
  }

  auto c = txt.scroll_start.y;

  // PERF: could save some cycles by comparing in origin-relative space.
  f32 min_distance = MAX_f32;
  ForLen( i, linerects ) {
    auto linerect = linerects.mem + i;
    auto mp = _vec2( Cast( f32, m.x ), Cast( f32, m.y ) );
    if( PtInBox( mp, linerect->p0, linerect->p1, 0.001f ) ) {
      c = linerect->line_start;
      break;
    }
    auto distance = DistanceToBox( mp, linerect->p0, linerect->p1 );
    if( distance < min_distance ) {
      min_distance = distance;
      c = linerect->line_start;
    }
  }

  Free( linerects );

  return c;



//  f64 y_frac = ( m.y - origin.y + px_click_correct.y + ( txt.scroll_start.frac * line_h ) ) / dim.y;
//  y_frac = CLAMP( y_frac, 0, 1 );
//  auto nlines_screen_max = Cast( idx_t, dim.y / line_h );
//  auto cy = Cast( idx_t, y_frac * nlines_screen_max );
//  AssertWarn( cy <= MAX_s16 );
//  auto c = CursorLineD( txt.buf, txt.scroll_start.y, 0, cy, 0, 0 );
//
//#if 1
//  idx_t cx = 0;
//#else
//
//#if 1
//  f32 x_frac = ( m.x - origin.x + px_click_correct.x ) / Cast( f32, char_w );
//  auto cx = Cast( idx_t, x_frac );
//#else
//  f32 x_frac = ( m.x - origin.x + px_click_correct.x ) / dim.x;
//  x_frac = CLAMP( x_frac, 0, 1 );
//  auto nchars_screen_max = Cast( idx_t, dim.x / font.char_w );
//  auto cx = Cast( idx_t, x_frac * nchars_screen_max );
//#endif
//
//#endif
//
//  auto line_end = CursorStopAtNewlineR( txt.buf, c, 0 );
//  c = MIN( c + cx, line_end );
//
//  return c;
}

Inl void
SetScrollPosFraction(
  txt_t& txt,
  f32 t
  )
{
  auto p = Round_idx_from_f32( t * TxtLen( txt ) );
  auto pos = CursorCharR( txt.buf, GetBOF( txt.buf ), p, 0 );
  txt.scroll_target.y = CursorStopAtNewlineL( txt.buf, pos, 0 );
  txt.scroll_target.frac = 0;
}

void
TxtControlMouse(
  txt_t& txt,
  bool& target_valid,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  glwmouseevent_t type,
  glwmousebtn_t btn,
  bool* alreadydn,
  bool* keyalreadydn,
  vec2<s32> m,
  vec2<s32> raw_delta,
  s32 dwheel,
  bool allow_scrollbar
  )
{
  ProfFunc();

  auto px_click_correct = _vec2<s8>(); // TODO: mouse control.
  auto scroll_nlines = GetPropFromDb( u8, u8_scroll_nlines );
  auto scroll_sign = GetPropFromDb( s8, s8_scroll_sign );
//  auto dblclick_period_sec = GetPropFromDb( f64, f64_dblclick_period_sec );
  auto scroll_continuous = GetPropFromDb( bool, bool_scroll_continuous );
  auto scroll_continuous_sensitivity = GetPropFromDb( f64, f64_scroll_continuous_sensitivity );

  auto scroll_pct = GetPropFromDb( f32, f32_scroll_pct );
  auto px_scroll = MAX( 16.0f, Round32( scroll_pct * MIN( dim.x, dim.y ) ) );

  bool ctrl  = keyalreadydn[Cast( enum_t, glwkey_t::ctrl )];
  bool shift = keyalreadydn[Cast( enum_t, glwkey_t::shift )];
  bool alt   = keyalreadydn[Cast( enum_t, glwkey_t::alt )];
  bool mod_isdn = ( ctrl | shift | alt );

  if( ScrollbarVisible( dim, px_scroll ) ) {

    vec2<f32> btn_up[2];
    GetScrollBtnUp( btn_up[0], btn_up[1], origin, dim, px_scroll );

    vec2<f32> btn_dn[2];
    GetScrollBtnDn( btn_dn[0], btn_dn[1], origin, dim, px_scroll );

    vec2<f32> btn_pos[2];
    GetScrollBtnPos( btn_pos[0], btn_pos[1], GetScrollPos( txt ), origin, dim, px_scroll );

    f32 t = GetScrollTarget( txt );

    if( GlwMouseInsideRect( m, btn_up[0], btn_up[1] ) ) {
      switch( type ) {
        case glwmouseevent_t::dn: {
          SetScrollPosFraction( txt, CLAMP( t - 0.1f, 0, 1 ) );
          target_valid = 0;
        } break;
        case glwmouseevent_t::wheelmove:
        case glwmouseevent_t::up:
        case glwmouseevent_t::move: {
        } break;
        default: UnreachableCrash();
      }

    } elif( GlwMouseInsideRect( m, btn_dn[0], btn_dn[1] ) ) {
      switch( type ) {
        case glwmouseevent_t::dn: {
          SetScrollPosFraction( txt, CLAMP( t + 0.1f, 0, 1 ) );
          target_valid = 0;
        } break;
        case glwmouseevent_t::wheelmove:
        case glwmouseevent_t::up:
        case glwmouseevent_t::move: {
        } break;
        default: UnreachableCrash();
      }

    } elif( GlwMouseInsideRect( m, btn_pos[0], btn_pos[1] ) ) {
      // TODO: scrollbar rect interactivity.
    }

    dim.x -= px_scroll;
  }

  if( !GlwMouseInside( m, origin, dim ) ) {
    // clear all interactivity state.
    txt.dblclick.first_made = 0;
    return;
  }

  switch( type ) {

    case glwmouseevent_t::wheelmove: {
      if( dwheel  &  ( !mod_isdn ) ) {
        dwheel *= scroll_sign;
        if( scroll_continuous ) {
          txt.scroll_target.frac += scroll_continuous_sensitivity * dwheel;
        } else {
          dwheel *= scroll_nlines;
          txt.scroll_target.frac += dwheel;
        }
        target_valid = 0;
      }
    } break;

    case glwmouseevent_t::dn: {

      switch( btn ) {
        case glwmousebtn_t::l: {

          auto c_pre_move = txt.c;
          auto c_move = MapMouseToCursor(
            txt,
            origin,
            dim,
            m,
            font,
            px_click_correct
            );

          if( ctrl ) {
            auto l = CursorStopAtNonWordCharL( txt.buf, c_move, 0 );
            auto r = CursorStopAtNonWordCharR( txt.buf, c_move, 0 );
            if( !Equal( l, r ) ) {
              txt.seltype = seltype_t::s;
              txt.c = r;
              txt.s = l;
            } else {
              txt.c = c_move;
            }
          } else {
            if( SelectIsZero( txt ) ) {
              if( shift ) {
                txt.seltype = seltype_t::s;
                txt.s = c_pre_move;
                txt.c = c_move;
              } else {
                txt.c = c_move;
              }
            } else {
              if( shift ) {
                txt.c = c_move;
              } else {
                txt.seltype = seltype_t::none;
                txt.c = c_move;
              }
            }
          }
          ResetCInline( txt );
          target_valid = 0;

          //// TODO: handle interaction with C,S,A
          //bool same_cursor = ( c_move == txt.dblclick.first_cursor );
          //bool double_click = And( txt.dblclick.first_made, same_cursor );
          //if( double_click ) {
          //  u64 curr_clock = TimeClock();
          //  f64 dblclick_period = TimeSecFromClocks64( txt.dblclick.first_clock, curr_clock );
          //  if( dblclick_period <= dblclick_period_sec ) {
          //    CmdSelectWordAtCursor( txt );
          //    txt.dblclick.first_made = 0;
          //  } else {
          //    txt.dblclick.first_clock = TimeClock();
          //  }
          //} else {
          //  txt.dblclick.first_made = 1;
          //  txt.dblclick.first_clock = TimeClock();
          //  txt.dblclick.first_cursor = c_move;
          //}
        } break;

        case glwmousebtn_t::r:
        case glwmousebtn_t::m:
        case glwmousebtn_t::b4:
        case glwmousebtn_t::b5: {
        } break;

        default: UnreachableCrash();
      }
    } break;

    case glwmouseevent_t::up: {
    } break;

    case glwmouseevent_t::move: {
      if( alreadydn[Cast( enum_t, glwmousebtn_t::l )] ) {
        auto c_pre_move = txt.c;
        auto c_move = MapMouseToCursor(
          txt,
          origin,
          dim,
          m,
          font,
          px_click_correct
          );

        if( !Equal( c_move, txt.c )  &&  !ctrl ) {
          if( SelectIsZero( txt ) ) {
            if( !Equal( c_pre_move, c_move ) ) {
              txt.seltype = seltype_t::s;
              txt.s = c_pre_move;
              txt.c = c_move;
              ResetCInline( txt );
            }
          } else {
            if( !Equal( c_pre_move, c_move ) ) {
              txt.c = c_move;
              ResetCInline( txt );
            }
          }
        }
        target_valid = 0;
      }
    } break;

    default: UnreachableCrash();
  }
}


// TODO: combine keybinds and cmdmap ?

struct
txt_cmdmap_t
{
  glwkeybind_t keybind;
  pfn_txtcmd_t fn;
  idx_t misc;
  idx_t misc2;
};

Inl txt_cmdmap_t
_txtcmdmap(
  glwkeybind_t keybind,
  pfn_txtcmd_t fn,
  idx_t misc = 0,
  idx_t misc2 = 0
  )
{
  txt_cmdmap_t r;
  r.keybind = keybind;
  r.fn = fn;
  r.misc = misc;
  r.misc2 = misc2;
  return r;
}


void
TxtControlKeyboardType(
  txt_t& txt,
  bool& target_valid,
  bool& content_changed,
  bool& ran_cmd,
  glwkeyevent_t type,
  glwkey_t key,
  glwkeylocks_t& keylocks,
  bool* alreadydn,
  bool allow_ln_add
  )
{
  ProfFunc();

  switch( type ) {
    case glwkeyevent_t::dn:
    case glwkeyevent_t::repeat: {

      bool caps_lock = keylocks.caps;
      txt_cmdmap_t table[] = {
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_a ), CmdAddAlpha , 'a' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_b ), CmdAddAlpha , 'b' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_c ), CmdAddAlpha , 'c' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_d ), CmdAddAlpha , 'd' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_e ), CmdAddAlpha , 'e' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_f ), CmdAddAlpha , 'f' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_g ), CmdAddAlpha , 'g' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_h ), CmdAddAlpha , 'h' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_i ), CmdAddAlpha , 'i' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_j ), CmdAddAlpha , 'j' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_k ), CmdAddAlpha , 'k' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_l ), CmdAddAlpha , 'l' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_m ), CmdAddAlpha , 'm' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_n ), CmdAddAlpha , 'n' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_o ), CmdAddAlpha , 'o' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_p ), CmdAddAlpha , 'p' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_q ), CmdAddAlpha , 'q' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_r ), CmdAddAlpha , 'r' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_s ), CmdAddAlpha , 's' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_t ), CmdAddAlpha , 't' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_u ), CmdAddAlpha , 'u' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_v ), CmdAddAlpha , 'v' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_w ), CmdAddAlpha , 'w' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_x ), CmdAddAlpha , 'x' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_y ), CmdAddAlpha , 'y' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_z ), CmdAddAlpha , 'z' ,  caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_A ), CmdAddAlpha , 'A' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_B ), CmdAddAlpha , 'B' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_C ), CmdAddAlpha , 'C' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_D ), CmdAddAlpha , 'D' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_E ), CmdAddAlpha , 'E' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_F ), CmdAddAlpha , 'F' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_G ), CmdAddAlpha , 'G' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_H ), CmdAddAlpha , 'H' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_I ), CmdAddAlpha , 'I' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_J ), CmdAddAlpha , 'J' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_K ), CmdAddAlpha , 'K' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_L ), CmdAddAlpha , 'L' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_M ), CmdAddAlpha , 'M' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_N ), CmdAddAlpha , 'N' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_O ), CmdAddAlpha , 'O' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_P ), CmdAddAlpha , 'P' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_Q ), CmdAddAlpha , 'Q' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_R ), CmdAddAlpha , 'R' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_S ), CmdAddAlpha , 's' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_T ), CmdAddAlpha , 'T' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_U ), CmdAddAlpha , 'U' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_V ), CmdAddAlpha , 'V' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_W ), CmdAddAlpha , 'W' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_X ), CmdAddAlpha , 'X' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_Y ), CmdAddAlpha , 'Y' , !caps_lock ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_Z ), CmdAddAlpha , 'Z' , !caps_lock ),

        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_0 ), CmdAddChar , '0' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_1 ), CmdAddChar , '1' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_2 ), CmdAddChar , '2' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_3 ), CmdAddChar , '3' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_4 ), CmdAddChar , '4' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_5 ), CmdAddChar , '5' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_6 ), CmdAddChar , '6' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_7 ), CmdAddChar , '7' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_8 ), CmdAddChar , '8' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_9 ), CmdAddChar , '9' ),

        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_space           ), CmdAddChar , ' '  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_tilde           ), CmdAddChar , '~'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_graveaccent     ), CmdAddChar , '`'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_exclamation     ), CmdAddChar , '!'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_atsign          ), CmdAddChar , '@'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_pound           ), CmdAddChar , '#'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_dollar          ), CmdAddChar , '$'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_percent         ), CmdAddChar , '%'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_caret           ), CmdAddChar , '^'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_ampersand       ), CmdAddChar , '&'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_star            ), CmdAddChar , '*'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_paren_l         ), CmdAddChar , '('  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_paren_r         ), CmdAddChar , ')'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_minus           ), CmdAddChar , '-'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_underscore      ), CmdAddChar , '_'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_plus            ), CmdAddChar , '+'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_equals          ), CmdAddChar , '='  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_bracketsquare_l ), CmdAddChar , '['  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_bracketsquare_r ), CmdAddChar , ']'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_bracketcurly_l  ), CmdAddChar , '{'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_bracketcurly_r  ), CmdAddChar , '}'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_bracketangle_l  ), CmdAddChar , '<'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_bracketangle_r  ), CmdAddChar , '>'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_slashback       ), CmdAddChar , '\\' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_slashforw       ), CmdAddChar , '/'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_verticalbar     ), CmdAddChar , '|'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_semicolon       ), CmdAddChar , ';'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_colon           ), CmdAddChar , ':'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_quotesingle     ), CmdAddChar , '\'' ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_quotedouble     ), CmdAddChar , '"'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_comma           ), CmdAddChar , ','  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_period          ), CmdAddChar , '.'  ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_question        ), CmdAddChar , '?'  ),

        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_tab_r      ), CmdTabR     ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_ch_l   ), CmdRemChL   ),
        _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_word_l ), CmdRemWordL ),
      };
      ForEach( entry, table ) {
        if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
          entry.fn( txt, entry.misc, entry.misc2 );
          target_valid = 0;
          content_changed = 1;
          ran_cmd = 1;
        }
      }

      if( allow_ln_add ) {
        if( GlwKeybind( key, alreadydn, GetPropFromDb( glwkeybind_t, keybind_txt_ln_add ) ) ) {
          CmdAddLn( txt, 0, 0 );
          target_valid = 0;
          content_changed = 1;
          ran_cmd = 1;
        }
      }

      bool num_lock = keylocks.num;
      if( num_lock ) {
        txt_cmdmap_t numpadtable[] = {
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_div     ), CmdAddChar , '/' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_mul     ), CmdAddChar , '*' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_sub     ), CmdAddChar , '-' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_add     ), CmdAddChar , '+' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_decimal ), CmdAddChar , '.' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_0       ), CmdAddChar , '0' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_1       ), CmdAddChar , '1' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_2       ), CmdAddChar , '2' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_3       ), CmdAddChar , '3' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_4       ), CmdAddChar , '4' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_5       ), CmdAddChar , '5' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_6       ), CmdAddChar , '6' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_7       ), CmdAddChar , '7' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_8       ), CmdAddChar , '8' ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_type_numpad_9       ), CmdAddChar , '9' ),
        };
        ForEach( entry, numpadtable ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( txt, entry.misc, entry.misc2 );
            target_valid = 0;
            content_changed = 1;
            ran_cmd = 1;
          }
        }
      }

    } break;

    case glwkeyevent_t::up: {
    } break;

    default: UnreachableCrash();
  }
}


void
TxtControlKeyboardNoContentChange(
  txt_t& txt,
  bool kb_command,
  bool& target_valid,
  bool& ran_cmd,
  glwkeyevent_t type,
  glwkey_t key,
  glwkeylocks_t& keylocks,
  bool* alreadydn
  )
{
  ProfFunc();

  if( kb_command ) {
    switch( type ) {
      case glwkeyevent_t::dn:
      case glwkeyevent_t::repeat: {
        txt_cmdmap_t table[] = {
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_copy                ), CmdCopy              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cs_undo             ), CmdCsUndo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cs_redo             ), CmdCsRedo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_toggle_insertmode   ), CmdToggleInsertMode  ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_l            ), CmdCursorL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_r            ), CmdCursorR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_u            ), CmdCursorU           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_d            ), CmdCursorD           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_home         ), CmdCursorHome        ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_end          ), CmdCursorEnd         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_page_d       ), CmdCursorJumpD       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_page_u       ), CmdCursorJumpU       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_skip_l       ), CmdCursorSkipL       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_skip_r       ), CmdCursorSkipR       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_l            ), CmdSelectL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_r            ), CmdSelectR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_u            ), CmdSelectU           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_d            ), CmdSelectD           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_home         ), CmdSelectHome        ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_end          ), CmdSelectEnd         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_page_d       ), CmdSelectJumpD       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_page_u       ), CmdSelectJumpU       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_skip_l       ), CmdSelectSkipL       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_skip_r       ), CmdSelectSkipR       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_all          ), CmdSelectAll         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_u            ), CmdScrollU           , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_d            ), CmdScrollD           , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_page_u       ), CmdScrollU           , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_page_d       ), CmdScrollD           , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_make_present ), CmdCursorMakePresent ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_multicursor_u       ), CmdMultiCursorU      ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_multicursor_d       ), CmdMultiCursorD      ),
        };
        ForEach( entry, table ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( txt, entry.misc, entry.misc2 );
            target_valid = 0;
            ran_cmd = 1;
          }
        }
      } break;

      case glwkeyevent_t::up: {
      } break;

      default: UnreachableCrash();
    }
  }
}

void
TxtControlKeyboard(
  txt_t& txt,
  bool kb_command,
  bool& target_valid,
  bool& content_changed,
  bool& ran_cmd,
  glwkeyevent_t type,
  glwkey_t key,
  glwkeylocks_t& keylocks,
  bool* alreadydn
  )
{
  ProfFunc();

  if( kb_command ) {
    switch( type ) {
      case glwkeyevent_t::dn:
      case glwkeyevent_t::repeat: {
        txt_cmdmap_t table_contentchange[] = {
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cut                 ), CmdCut                ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_paste               ), CmdPaste              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_txt_undo            ), CmdTxtUndo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_txt_redo            ), CmdTxtRedo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_tab_r               ), CmdTabR               ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_tab_l               ), CmdTabL               ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_ch_l            ), CmdRemChL             ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_ch_r            ), CmdRemChR             ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_word_l          ), CmdRemWordL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_word_r          ), CmdRemWordR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_line_r          ), CmdRemLineR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_comment_selection   ), CmdCommentSelection   ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_uncomment_selection ), CmdUncommentSelection ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_add              ), CmdAddLn              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_rem              ), CmdRemLn              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_comment          ), CmdComment            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_uncomment        ), CmdUncomment          ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_comment_toggle   ), CmdToggleComment      ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_permute_u        ), CmdPermuteU           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_ln_permute_d        ), CmdPermuteD           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_trailspace      ), CmdRemTrailspace      ),
        };
        ForEach( entry, table_contentchange ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( txt, entry.misc, entry.misc2 );
            target_valid = 0;
            content_changed = 1;
            ran_cmd = 1;
          }
        }

        txt_cmdmap_t table[] = {
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_copy                ), CmdCopy              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cs_undo             ), CmdCsUndo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cs_redo             ), CmdCsRedo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_toggle_insertmode   ), CmdToggleInsertMode  ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_l            ), CmdCursorL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_r            ), CmdCursorR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_u            ), CmdCursorU           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_d            ), CmdCursorD           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_home         ), CmdCursorHome        ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_end          ), CmdCursorEnd         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_page_d       ), CmdCursorJumpD       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_page_u       ), CmdCursorJumpU       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_skip_l       ), CmdCursorSkipL       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_skip_r       ), CmdCursorSkipR       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_l            ), CmdSelectL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_r            ), CmdSelectR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_u            ), CmdSelectU           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_d            ), CmdSelectD           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_home         ), CmdSelectHome        ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_end          ), CmdSelectEnd         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_page_d       ), CmdSelectJumpD       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_page_u       ), CmdSelectJumpU       , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_skip_l       ), CmdSelectSkipL       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_skip_r       ), CmdSelectSkipR       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_all          ), CmdSelectAll         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_u            ), CmdScrollU           , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_d            ), CmdScrollD           , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_page_u       ), CmdScrollU           , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_scroll_page_d       ), CmdScrollD           , Cast( idx_t, txt.window_n_lines / 2 ) ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_make_present ), CmdCursorMakePresent ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_multicursor_u       ), CmdMultiCursorU      ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_multicursor_d       ), CmdMultiCursorD      ),
        };
        ForEach( entry, table ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( txt, entry.misc, entry.misc2 );
            target_valid = 0;
            ran_cmd = 1;
          }
        }
      } break;

      case glwkeyevent_t::up: {
      } break;

      default: UnreachableCrash();
    }
  }

  if( !ran_cmd ) {
    if( !kb_command ) {
      TxtControlKeyboardType(
        txt,
        target_valid,
        content_changed,
        ran_cmd,
        type,
        key,
        keylocks,
        alreadydn,
        1
        );
    }
  }
}


void
TxtControlKeyboardSingleLine(
  txt_t& txt,
  bool kb_command,
  bool& target_valid,
  bool& content_changed,
  bool& ran_cmd,
  glwkeyevent_t type,
  glwkey_t key,
  glwkeylocks_t& keylocks,
  bool* alreadydn
  )
{
  ProfFunc();

  if( kb_command ) {
    switch( type ) {
      case glwkeyevent_t::dn:
      case glwkeyevent_t::repeat: {
        txt_cmdmap_t table_contentchange[] = {
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cut        ), CmdCut               ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_paste      ), CmdPaste             ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_txt_undo   ), CmdTxtUndo           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_txt_redo   ), CmdTxtRedo           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_tab_r      ), CmdTabR              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_tab_l      ), CmdTabL              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_ch_l   ), CmdRemChL            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_ch_r   ), CmdRemChR            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_word_l ), CmdRemWordL          ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_word_r ), CmdRemWordR          ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_rem_line_r ), CmdRemLineR          ),
        };
        ForEach( entry, table_contentchange ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( txt, entry.misc, entry.misc2 );
            target_valid = 0;
            content_changed = 1;
            ran_cmd = 1;
          }
        }

        txt_cmdmap_t table[] = {
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_copy                ), CmdCopy              ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cs_undo             ), CmdCsUndo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cs_redo             ), CmdCsRedo            ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_toggle_insertmode   ), CmdToggleInsertMode  ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_l            ), CmdCursorL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_r            ), CmdCursorR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_home         ), CmdCursorHome        ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_end          ), CmdCursorEnd         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_skip_l       ), CmdCursorSkipL       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_cursor_skip_r       ), CmdCursorSkipR       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_l            ), CmdSelectL           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_r            ), CmdSelectR           ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_home         ), CmdSelectHome        ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_end          ), CmdSelectEnd         ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_skip_l       ), CmdSelectSkipL       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_skip_r       ), CmdSelectSkipR       ),
          _txtcmdmap( GetPropFromDb( glwkeybind_t, keybind_txt_select_all          ), CmdSelectAll         ),
        };
        ForEach( entry, table ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( txt, entry.misc, entry.misc2 );
            target_valid = 0;
            ran_cmd = 1;
          }
        }
      } break;

      case glwkeyevent_t::up: {
      } break;

      default: UnreachableCrash();
    }
  }

  if( !ran_cmd ) {
    if( !kb_command ) {
      TxtControlKeyboardType(
        txt,
        target_valid,
        content_changed,
        ran_cmd,
        type,
        key,
        keylocks,
        alreadydn,
        0
        );
    }
  }
}




struct
cmd_t
{
  txt_t txt_display;
  txt_t txt_cmd;
};


void
CmdInit( cmd_t& cmd )
{
  Init( cmd.txt_display );
  Init( cmd.txt_cmd );

  TxtLoadEmpty( cmd.txt_display );
  TxtLoadEmpty( cmd.txt_cmd );

  fsobj_t cwd;
  FsGetCwd( cwd );

  CmdAddString( cmd.txt_display, Cast( idx_t, cwd.mem ), cwd.len );
  CmdAddChar( cmd.txt_display, '>' );
}

void
CmdKill( cmd_t& cmd )
{
  Kill( cmd.txt_display );
  Kill( cmd.txt_cmd );
}




#define __CmdCmd( name )   void ( name )( cmd_t& cmd )
typedef __CmdCmd( *pfn_cmdcmd_t );



// assume dir has only '/' chars, no '\\' chars.
Inl void
ChangeCwd( cmd_t& cmd, u8* dir, idx_t dir_len )
{
  fsobj_t cwd;
  FsGetCwd( cwd );

  bool do_set = 0;

  // parse loop for processing 'dir'
  Forever {

    if( dir_len == 0 ) {
      do_set = 1; // success !
      break;
    }

    if( dir_len >= 1  &&  dir[0] == '/' ) {
      dir_len -= 1;
      dir += 1;
    }

    if( dir_len >= 2  &&  CsEquals( dir, 2, Str( ".." ), 2, 1 ) ) {
      u8* cwd_lastslash = CsScanL( ML( cwd ), '/' );
      if( !cwd_lastslash ) {
        static const u8* msg = Str( "cannot cd up; at root." );
        CmdAddString( cmd.txt_display, Cast( idx_t, msg ), CsLen( msg ) );
        CmdAddLn( cmd.txt_display );
        break;
      } else {
        cwd.len = CsLen( cwd.mem, cwd_lastslash );
        dir += 2;
        dir_len -= 2;
      }
      continue;
    }

    // must check for '.' after check for '..' so it's unambiguous.
    if( dir_len >= 1  &&  dir[0] == '.' ) {
      dir_len -= 1;
      dir += 1;
      continue;
    }

    u8* subdir_end = CsScanR( dir, dir_len, '/' );
    if( !subdir_end ) {
      subdir_end = dir + dir_len;
    }
    u8* subdir = dir;
    idx_t subdir_len = CsLen( subdir, subdir_end );

    fsobj_t tmp = cwd;
    Memmove( AddBack( tmp ), "/", 1 );
    Memmove( AddBack( tmp, subdir_len ), subdir, subdir_len );

    bool subdir_exists = DirExists( ML( tmp ) );
    if( !subdir_exists ) {
      static const u8* msg = Str( "cannot cd; subdir doesn't exist: " );
      CmdAddString( cmd.txt_display, Cast( idx_t, msg ), CsLen( msg ) );
      CmdAddString( cmd.txt_display, Cast( idx_t, subdir ), subdir_len );
      CmdAddLn( cmd.txt_display );
      break;
    } else {
      Memmove( AddBack( cwd ), "/", 1 );
      Memmove( AddBack( cwd, subdir_len ), subdir, subdir_len );

      dir_len -= subdir_len;
      continue;
    }
  }
  if( do_set ) {
    FsSetCwd( cwd );
  }
}


#define COM_LEN   32767


__CmdCmd( CmdExecute )
{
  // force cursor to end of txt_display
  CmdCursorFileR( cmd.txt_display );

  auto input_start = GetBOF( cmd.txt_cmd.buf );
  auto input_end = GetEOF( cmd.txt_cmd.buf );
  auto input_len = TxtLen( cmd.txt_cmd );

  if( input_len ) {
    u8* input = MemHeapAlloc( u8, input_len );

    Contents( cmd.txt_cmd.buf, input_start, input, input_len );

    CmdSelectAll( cmd.txt_cmd );
    CmdRemChL( cmd.txt_cmd );

    CmdAddString( cmd.txt_display, Cast( idx_t, input ), input_len );
    CmdAddLn( cmd.txt_display );

    // built-in commands:
    if( CsEquals( input, 3, Str( "cd " ), 3, 0 ) ) {
      u8* dir = input + 3;
      idx_t dir_len = input_len - 3;
      CsReplace( dir, dir_len, '\\', '/' );
      ChangeCwd( cmd, dir, dir_len );

    } elif( CsEquals( input, 2, Str( "ls" ), 2, 0 )  ||
            CsEquals( input, 3, Str( "dir" ), 3, 0 ) ) {
      fsobj_t cwd;
      FsGetCwd( cwd );
      plist_t mem;
      Init( mem, 32768 );
      array_t<slice_t> objs;
      Alloc( objs, 256 );
      FsFindDirs( objs, mem, ML( cwd ), 0 );
      ForLen( i, objs ) {
        auto& obj = objs.mem[i];
        CmdAddString( cmd.txt_display, Cast( idx_t, obj.mem ), obj.len );
        CmdAddLn( cmd.txt_display );
      }
      objs.len = 0;
      FsFindFiles( objs, mem, ML( cwd ), 0 );
      ForLen( i, objs ) {
        auto& obj = objs.mem[i];
        CmdAddString( cmd.txt_display, Cast( idx_t, obj.mem ), obj.len );
        CmdAddLn( cmd.txt_display );
      }
      Free( objs );
      Kill( mem );

    // calling other .exe/.bat
    } else {

      // TODO: call OS function.

      HANDLE child_stdout_r = 0;
      HANDLE child_stdout_w = 0;

      const idx_t PIPEBUFLEN = 4095;
      u8 pipebuf[PIPEBUFLEN + 1];

      SECURITY_ATTRIBUTES security;
      security.nLength = sizeof( SECURITY_ATTRIBUTES );
      security.bInheritHandle = TRUE;
      security.lpSecurityDescriptor = 0;

      BOOL res;
      res = CreatePipe( &child_stdout_r, &child_stdout_w, &security, 0 );
      AssertWarn( res );
      res = SetHandleInformation( child_stdout_r, HANDLE_FLAG_INHERIT, 0 );
      AssertWarn( res );

      PROCESS_INFORMATION process = { 0 };
      STARTUPINFO startup = { 0 };
      startup.cb = sizeof( STARTUPINFO );
      startup.hStdError = child_stdout_w;
      startup.hStdOutput = child_stdout_w;
      startup.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
      startup.wShowWindow = SW_HIDE;
      startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

      if( input_len + 1 >= COM_LEN ) {
        static const u8* msg = Str( "input command is too long!" );
        CmdAddString( cmd.txt_display, Cast( idx_t, msg ), CsLen( msg ) );
        CmdAddLn( cmd.txt_display );

      } else {
        u8 com[COM_LEN];
        CsCopy( com, input, MIN( input_len, COM_LEN-1 ) );

        BOOL created = CreateProcess(
          0,
          Cast( char*, com ),
          0,
          0,
          TRUE,
          0,
          0,
          0,
          &startup,
          &process
          );

        if( created ) {
          // Set the child free from the parent.
          CloseHandle( process.hProcess );
          CloseHandle( process.hThread );
        } else {
          static const u8* msg = Str( "can't find command!" );
          CmdAddString( cmd.txt_display, Cast( idx_t, msg ), CsLen( msg ) );
          CmdAddLn( cmd.txt_display );
        }

        // Close the parent attachment to the write half of the pipe.
        CloseHandle( child_stdout_w );

        if( created ) {
          Forever {
            DWORD nread;
            BOOL read_res = ReadFile( child_stdout_r, pipebuf, PIPEBUFLEN, &nread, 0 );
            if( !read_res || !nread ) {
              break;
            }

            pipebuf[nread] = 0; // insert nul-terminator.

            CmdAddString( cmd.txt_display, Cast( idx_t, pipebuf ), nread );
          }
        }

        CloseHandle( child_stdout_r );
      }
    }
    MemHeapFree( input );
  } else {
    CmdAddLn( cmd.txt_display );
  }

  fsobj_t cwd;
  FsGetCwd( cwd );

  CmdAddString( cmd.txt_display, Cast( idx_t, cwd.mem ), cwd.len );
  CmdAddChar( cmd.txt_display, '>' );
}



void
CmdRender(
  cmd_t& cmd,
  bool& target_valid,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  f64 timestep_realtime,
  f64 timestep_fixed
  )
{
  ProfFunc();

  f32 y_frac = 0.8f; // TODO: make this adjustable by mouse.

  auto u_origin = origin;
  auto u_dim = dim * _vec2<f32>( 1, y_frac );

  auto delta = _vec2<f32>( 0, u_dim.y );
  auto d_origin = u_origin + delta;
  auto d_dim = dim - delta;

  // add y padding between the two rects
  static const f32 px_padding = 2.0f;
  u_dim.y = MAX( u_origin.y, u_dim.y - px_padding / 2 );
  d_origin.y += px_padding / 2;
  d_dim.y = MAX( d_origin.y, d_dim.y - px_padding / 2 );

  TxtUpdateScrolling(
    cmd.txt_display,
    font,
    u_origin,
    u_dim,
    timestep_realtime,
    timestep_fixed
    );
  TxtLayout(
    cmd.txt_display,
    font
    );
  TxtRender(
    cmd.txt_display,
    target_valid,
    stream,
    font,
    u_origin,
    u_dim,
    zrange,

    1,
    1,
    0,
    1
    );

  TxtUpdateScrolling(
    cmd.txt_cmd,
    font,
    d_origin,
    d_dim,
    timestep_realtime,
    timestep_fixed
    );
  TxtLayout(
    cmd.txt_cmd,
    font
    );
  TxtRender(
    cmd.txt_cmd,
    target_valid,
    stream,
    font,
    d_origin,
    d_dim,
    zrange,

    1,
    1,
    1,
    1
    );

  RenderLine(
    stream,
    _vec4<f32>( 1, 1, 1, 1 ),
    _vec2<f32>( origin.x, u_origin.y + u_dim.y ),
    _vec2<f32>( origin.x + dim.x, u_origin.y + u_dim.y ),
    origin, dim,
    1, // TODO: proper z
    1
    );
}


void
CmdControlMouse(
  cmd_t& cmd,
  bool& target_valid,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  glwmouseevent_t type,
  glwmousebtn_t btn,
  bool* alreadydn,
  bool* keyalreadydn,
  vec2<s32> m,
  vec2<s32> raw_delta,
  s32 dwheel,
  bool allow_scrollbar
  )
{
  ProfFunc();

  TxtControlMouse(
    cmd.txt_display,
    target_valid,
    font,
    origin,
    dim,
    type,
    btn,
    alreadydn,
    keyalreadydn,
    m,
    raw_delta,
    dwheel,
    allow_scrollbar
    );
}




struct
cmd_cmdmap_t
{
  glwkeybind_t keybind;
  pfn_cmdcmd_t fn;
};

Inl cmd_cmdmap_t
_cmdcmdmap(
  glwkeybind_t keybind,
  pfn_cmdcmd_t fn
  )
{
  cmd_cmdmap_t r;
  r.keybind = keybind;
  r.fn = fn;
  return r;
}


void
CmdControlKeyboard(
  cmd_t& cmd,
  bool kb_command,
  bool& target_valid,
  bool& ran_cmd,
  glwkeyevent_t type,
  glwkey_t key,
  glwkeylocks_t& keylocks,
  bool* alreadydn
  )
{
  ProfFunc();

  if( kb_command ) {
    switch( type ) {
      case glwkeyevent_t::dn: {
        // cmd level commands
        cmd_cmdmap_t table[] = {
          _cmdcmdmap( GetPropFromDb( glwkeybind_t, keybind_cmd_execute ), CmdExecute ),
        };
        ForEach( entry, table ) {
          if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
            entry.fn( cmd );
            target_valid = 0;
            ran_cmd = 1;
          }
        }
      } __fallthrough;

      case glwkeyevent_t::repeat:
      case glwkeyevent_t::up: {
      } break;

      default: UnreachableCrash();
    }
  }

  if( !ran_cmd ) {
    bool content_changed = 0;
    TxtControlKeyboardSingleLine(
      cmd.txt_cmd,
      kb_command,
      target_valid,
      content_changed,
      ran_cmd,
      type,
      key,
      keylocks,
      alreadydn
      );
  }
}



Enumc( editmode_t )
{
  editfile, // defer to txt operations on the active txt_t
  editfile_findrepl,
  editfile_gotoline,

  switchopened, // choose between opened files

  fileopener, // choose a new file to open and make active
  fileopener_renaming,

  externalmerge,

  findinfiles,
};



// should we bother using buf for filecontentsearch / findinfiles ?
// presumably it will only ever be slower than raw string search.
// we never need any content-editing capabilities, even for replacement, since we open in the editor for that.
// maybe if we implement largefile-streaming at some point, then it might be worth it.
// we can probably delete this macro.
#define USE_BUF_FOR_FILECONTENTSEARCH 0

struct
foundinfile_t
{
  slice_t name;
  slice_t sample;
#if USE_BUF_FOR_FILECONTENTSEARCH
  content_ptr_t l;
  content_ptr_t r;
#else
  idx_t l;
  idx_t r;
#endif
  idx_t len;
  idx_t offset_into_sample;
};

Inl void
Zero( foundinfile_t& f )
{
  f.name = {};
  f.sample = {};
  f.l = {};
  f.r = {};
  f.len = 0;
  f.offset_into_sample = 0;
}

Inl void
Init( foundinfile_t& f )
{
  Zero( f );
}

Inl void
Kill( foundinfile_t& f )
{
  Zero( f );
}




// per-thread context to execute the filecontentsearch task.
struct
asynccontext_filecontentsearch_t
{
  // input / readonly:
  array_t<slice_t>* filenames;
  idx_t filenames_start;
  idx_t filenames_count;
  array_t<slice_t>* ignored_filetypes_list;
  slice_t key;
  bool case_sens;
  bool word_boundary;

  // output / modifiable:
  plist_t mem;
  pagearray_t<foundinfile_t> matches;


  u8 cache_line_padding_to_avoid_thrashing[64]; // last thing, since this type is packed into an array_t
};




Enumc( findinfilesfocus_t )
{
  dir,
  ignored_filetypes,
  query,
  replacement,
  COUNT
};

struct
findinfiles_t
{
  pagearray_t<foundinfile_t> matches;
  plist_t mem;
  findinfilesfocus_t focus;
  txt_t dir;
  txt_t query;
  bool case_sens;
  bool word_boundary;
  txt_t replacement;
  txt_t ignored_filetypes;
  idx_t cursor;
  idx_t scroll_start;
  idx_t scroll_end;
  idx_t pageupdn_distance;
  slice_t matches_dir;

  u8 cache_line_padding_to_avoid_thrashing[64];
  array_t<asynccontext_filecontentsearch_t> asynccontexts;
  idx_t ncontexts_active;
  idx_t max_ncontexts_active;

  // used by all other threads as readonly:
  array_t<slice_t> filenames;
  array_t<slice_t> ignored_filetypes_list;
  string_t key;
};

Inl void
Init( findinfiles_t& fif )
{
  Init( fif.mem, 128*1024 );
  Init( fif.matches, 256 );
  Init( fif.dir );
  TxtLoadEmpty( fif.dir );
  fsobj_t cwd;
  FsGetCwd( cwd );
  CmdAddString( fif.dir, Cast( idx_t, cwd.mem ), cwd.len );
  Init( fif.query );
  TxtLoadEmpty( fif.query );
  fif.case_sens = 0;
  fif.word_boundary = 0;
  Init( fif.replacement );
  TxtLoadEmpty( fif.replacement );
  Init( fif.ignored_filetypes );
  TxtLoadEmpty( fif.ignored_filetypes );
  auto default_filetypes = GetPropFromDb( slice_t, string_findinfiles_ignored_file_extensions );
  CmdAddString( fif.ignored_filetypes, Cast( idx_t, default_filetypes.mem ), default_filetypes.len );
  fif.focus = findinfilesfocus_t::query;
  fif.cursor = 0;
  fif.scroll_start = 0;
  fif.scroll_end = 0;
  fif.pageupdn_distance = 0;
  fif.matches_dir = {};
  fif.ncontexts_active = 0;
  fif.max_ncontexts_active = 0;
  Alloc( fif.filenames, 4096 );
  Alloc( fif.asynccontexts, 16 );
  Alloc( fif.ignored_filetypes_list, 16 );
  Zero( fif.key );
}

Inl void
Kill( findinfiles_t& fif )
{
  Kill( fif.matches );
  Kill( fif.mem );
  Kill( fif.dir );
  Kill( fif.query );
  fif.case_sens = 0;
  fif.word_boundary = 0;
  Kill( fif.replacement );
  Kill( fif.ignored_filetypes );
  fif.focus = findinfilesfocus_t::query;
  fif.cursor = 0;
  fif.scroll_start = 0;
  fif.scroll_end = 0;
  fif.pageupdn_distance = 0;
  fif.matches_dir = {};
  fif.ncontexts_active = 0;
  fif.max_ncontexts_active = 0;
  Free( fif.filenames );

  ForLen( i, fif.asynccontexts ) {
    auto ac = fif.asynccontexts.mem + i;
    Kill( ac->mem );
    Kill( ac->matches );
  }
  Free( fif.asynccontexts );

  Free( fif.ignored_filetypes_list );
  Free( fif.key );
}








struct
findreplace_t
{
  bool focus_find; // else replace.
  bool case_sens;
  bool word_boundary;
  txt_t find;
  txt_t replace;
};

Inl void
Init( findreplace_t& fr )
{
  Init( fr.find );
  TxtLoadEmpty( fr.find );
  Init( fr.replace );
  TxtLoadEmpty( fr.replace );
  fr.focus_find = 1;
  fr.case_sens = 0;
  fr.word_boundary = 0;
}

Inl void
Kill( findreplace_t& fr )
{
  Kill( fr.find );
  Kill( fr.replace );
  fr.focus_find = 1;
  fr.case_sens = 0;
  fr.word_boundary = 0;
}




struct
fileopener_dblclick_t
{
  bool first_made;
  idx_t first_cursor;
  u64 first_clock;
};

struct
fileopener_row_t
{
  slice_t name;
  slice_t sizetxt;
  u64 size;
  bool is_file;
  bool readonly;
};

Inl void
Zero( fileopener_row_t& row )
{
  row.name.len = 0;
  row.sizetxt.len = 0;
  row.size = 0;
  row.is_file = 0;
  row.readonly = 0;
}



Enumc( fileopenerfocus_t )
{
  dir,
  ignored_filetypes,
  ignored_substrings,
  query,
  COUNT
};

Enumc( fileopener_opertype_t )
{
  checkpt,
  rename,
  changecwd,
  delfile,
  deldir,
  newfile,
  newdir,
  COUNT
};

struct
fileopener_oper_t
{
  fileopener_opertype_t type;
  union
  {
    struct
    {
      fsobj_t src;
      fsobj_t dst;
    }
    rename;

    struct
    {
      fsobj_t src;
      fsobj_t dst;
    }
    changecwd;

    fsobj_t delfile;
    fsobj_t deldir;
    fsobj_t newfile;
    fsobj_t newdir;
  };
};

// per-thread context to execute the FileopenerFillPool task.
// note that we only have 1 background thread at a time doing this work.
// that's because it's likely limited by disk performance, which threading won't help with.
struct
asynccontext_fileopenerfillpool_t
{
  // input / readonly:
  fsobj_t cwd;

  // output / modifiable:
  pagearray_t<fileopener_row_t> pool;
  plist_t pool_mem;
};

struct
fileopener_t
{
  array_t<fileopener_oper_t> history;
  idx_t history_idx;

  // pool is the backing store of actual files/dirs given our cwd and query.
  // based on the query, we'll filter down to some subset, stored in matches.
  // ui just shows matches.
  fileopenerfocus_t focus;
  txt_t cwd;
  fsobj_t last_cwd_for_changecwd_rollback;
  txt_t query;
  txt_t ignored_filetypes;
  txt_t ignored_substrings;
  pagearray_t<fileopener_row_t> pool;
  plist_t pool_mem; // reset everytime we fillpool.
  pagearray_t<fileopener_row_t*> matches; // points into pool.
  idx_t cursor; // index into matches.
  idx_t scroll_start;
  idx_t scroll_end;
  idx_t pageupdn_distance;

  u8 cache_line_padding_to_avoid_thrashing[64];
  asynccontext_fileopenerfillpool_t asynccontext_fillpool;
  u8 cache_line_padding_to_avoid_thrashing2[64];
  idx_t ncontexts_active;

  plist_t matches_mem; // reset everytime we regenerate matches.
  array_t<slice_t> ignored_filetypes_list; // uses matches_mem as backing memory.
  array_t<slice_t> ignored_substrings_list;

  fileopener_dblclick_t dblclick;

  idx_t renaming_row; // which row we're renaming, if in renaming mode.
  txt_t renaming_txt;
};

void
Init( fileopener_t& fo )
{
  Alloc( fo.history, 512 );
  fo.history_idx = 0;

  fo.focus = fileopenerfocus_t::query;

  Init( fo.cwd );
  TxtLoadEmpty( fo.cwd );
  fsobj_t cwd;
  FsGetCwd( cwd );
  CmdAddString( fo.cwd, Cast( idx_t, cwd.mem ), cwd.len );
  fo.last_cwd_for_changecwd_rollback = {};

  Init( fo.query );
  TxtLoadEmpty( fo.query );

  Init( fo.ignored_filetypes );
  TxtLoadEmpty( fo.ignored_filetypes );
  auto default_filetypes = GetPropFromDb( slice_t, string_fileopener_ignored_file_extensions );
  CmdAddString( fo.ignored_filetypes, Cast( idx_t, default_filetypes.mem ), default_filetypes.len );

  Init( fo.ignored_substrings );
  TxtLoadEmpty( fo.ignored_substrings );
  auto default_substrings = GetPropFromDb( slice_t, string_fileopener_ignored_substrings );
  CmdAddString( fo.ignored_substrings, Cast( idx_t, default_substrings.mem ), default_substrings.len );

  Init( fo.pool, 128 );
  Init( fo.pool_mem, 128*1024 );

  Init( fo.matches, 128 );

  fo.cursor = 0;
  fo.scroll_start = 0;
  fo.scroll_end = 0;
  fo.pageupdn_distance = 0;

  fo.asynccontext_fillpool.cwd.len = 0;
  Init( fo.asynccontext_fillpool.pool, 128 );
  Init( fo.asynccontext_fillpool.pool_mem, 128*1024 );
  fo.ncontexts_active = 0;

  Init( fo.matches_mem, 128*1024 );
  Alloc( fo.ignored_filetypes_list, 16 );
  Alloc( fo.ignored_substrings_list, 16 );

  fo.dblclick.first_made = 0;
  fo.dblclick.first_cursor = 0;
  fo.dblclick.first_clock = 0;

  fo.renaming_row = 0;
  Init( fo.renaming_txt );
  TxtLoadEmpty( fo.renaming_txt );
}

void
Kill( fileopener_t& fo )
{
  Free( fo.history );
  fo.history_idx = 0;

  fo.focus = fileopenerfocus_t::query;

  Kill( fo.cwd );
  fo.last_cwd_for_changecwd_rollback = {};
  Kill( fo.query );
  Kill( fo.ignored_filetypes );
  Kill( fo.ignored_substrings );

  Kill( fo.pool );
  Kill( fo.pool_mem );

  Kill( fo.matches );

  fo.cursor = 0;
  fo.scroll_start = 0;
  fo.scroll_end = 0;
  fo.pageupdn_distance = 0;

  fo.asynccontext_fillpool.cwd.len = 0;
  Kill( fo.asynccontext_fillpool.pool );
  Kill( fo.asynccontext_fillpool.pool_mem );
  fo.ncontexts_active = 0;

  Kill( fo.matches_mem );
  Free( fo.ignored_filetypes_list );
  Free( fo.ignored_substrings_list );

  fo.dblclick.first_made = 0;
  fo.dblclick.first_cursor = 0;
  fo.dblclick.first_clock = 0;

  fo.renaming_row = 0;
  Kill( fo.renaming_txt );
}






struct
edittxtopen_t
{
  txt_t txt;
  bool unsaved;
  bool horz_l; // else r.  // TODO: replace with an enum.
  u64 time_lastwrite;
};

struct
edit_t
{
  editmode_t mode;
  listwalloc_t<edittxtopen_t> opened;
  plist_t mem;

  edittxtopen_t* active[2];
  bool horzview;
  bool horzfocus_l; // else r.

  // openedmru
  listwalloc_t<edittxtopen_t*> openedmru;

  // opened
  array_t<edittxtopen_t*> search_matches;
  txt_t opened_search;
  idx_t opened_cursor;
  idx_t opened_scroll_start;
  idx_t opened_scroll_end;

  idx_t nlines_screen;

  findinfiles_t findinfiles;

  fileopener_t fileopener;

  txt_t gotoline;

  findreplace_t findrepl;

  editmode_t mode_before_externalmerge;
  bool horzview_before_externalmerge;
  edittxtopen_t* active_externalmerge;
  file_t file_externalmerge;
};

#define __EditCmd( name )   void ( name )( edit_t& edit, idx_t misc = 0 )
#define __EditCmdDef( name )   void ( name )( edit_t& edit, idx_t misc )
typedef __EditCmdDef( *pfn_editcmd_t );





#if 0
// NOTE: opens a txt not backed by a real file!
void
EditOpenEmptyTxt( edit_t& edit )
{
  txt_t txt;
  Init( txt );
  LoadEmpty( txt.buf );
  // set txt.filename so we can uniquely identify this empty txt.
  TimeDate( txt.filename.mem, Capacity( txt.filename ), &txt.filename.len );
  AddBack( edit.opened, &txt );

  if( TxtLen( edit.opened_list ) ) {
    CmdAddLn( edit.opened_list );
  }
  CmdAddString( edit.opened_list, Cast( idx_t, txt.filename.mem ), txt.filename.len );
}
#endif


Inl edittxtopen_t*
EditGetOpenedFile( edit_t& edit, u8* filename, idx_t filename_len )
{
  ForList( elem, edit.opened ) {
    auto txt = &elem->value.txt;
    bool already_open = MemEqual( ML( txt->filename ), filename, filename_len );
    if( already_open ) {
      return &elem->value;
    }
  }
  return 0;
}

Inl void
MoveOpenedToFrontOfMru( edit_t& edit, edittxtopen_t* open )
{
  ForList( elem, edit.openedmru ) {
    if( elem->value == open ) {
      Rem( edit.openedmru, elem );
      InsertFirst( edit.openedmru, elem );
      AssertCrash( edit.openedmru.len == edit.opened.len );
      return;
    }
  }
  AssertCrash( edit.openedmru.len == edit.opened.len );
  UnreachableCrash();
}

Inl void
OnFileOpen( edit_t& edit, edittxtopen_t* open )
{
  auto allowed = GetPropFromDb( bool, bool_run_on_open_allowed );
  if( !allowed ) {
    return;
  }

  auto show_window = GetPropFromDb( bool, bool_run_on_open_show_window );
  auto run_on_open = GetPropFromDb( slice_t, string_run_on_open );
  string_t cmd;
  Alloc( cmd, run_on_open.len + 1 + open->txt.filename.len );
  Memmove( cmd.mem, ML( run_on_open ) );
  Memmove( cmd.mem + run_on_open.len, " ", 1 );
  Memmove( cmd.mem + run_on_open.len + 1, ML( open->txt.filename ) );
  pagearray_t<u8> output;
  Init( output, 1024 );
  Log( "RUN_ON_OPEN" );
  LogAddIndent( +1 );
  {
    auto cstr = AllocCstr( cmd );
    Log( "executing command: %s", cstr );
    MemHeapFree( cstr );
  }

  s32 r = Execute( SliceFromString( cmd ), output, show_window );

  Log( "retcode: %d", r );

#if 0
  {
    // TODO: LogInline each of the elems of output pagearray.
    auto output_str = StringFromPlist( output );
    auto cstr = AllocCstr( output_str );
    Log( "output: %s", cstr );
    MemHeapFree( cstr );
    Free( output_str );
  }
#endif

  Kill( output );
  LogAddIndent( -1 );
}

void
EditOpen( edit_t& edit, file_t& file, edittxtopen_t** opened, bool* opened_existing )
{
  // detect files we already have open.
  auto open = EditGetOpenedFile( edit, ML( file.obj ) );
  if( open ) {
    *opened_existing = 1;

  } else {
    *opened_existing = 0;

    auto elem = AddLast( edit.opened );
    open = &elem->value;
    open->unsaved = 0;
    open->horz_l = edit.horzfocus_l;
    Init( open->txt );
    TxtLoad( open->txt, file );
    open->time_lastwrite = file.time_lastwrite;

    auto mruelem = AddLast( edit.openedmru );
    mruelem->value = open;
    AssertCrash( edit.openedmru.len == edit.opened.len );

    OnFileOpen( edit, open );
  }
  *opened = open;
}

void
EditOpenAndSetActiveTxt( edit_t& edit, file_t& file )
{
  edittxtopen_t* open = 0;
  bool opened_existing = 0;
  EditOpen( edit, file, &open, &opened_existing );
  AssertCrash( open );

  edit.horzfocus_l = open->horz_l;
  edit.active[edit.horzfocus_l] = open;
  MoveOpenedToFrontOfMru( edit, open );
}






// =================================================================================
// FILE OPENER

Inl fileopener_oper_t*
FileopenerAddHistorical( fileopener_t& fo, fileopener_opertype_t type )
{
  // invalidate previous futures.
  AssertCrash( fo.history_idx <= fo.history.len );
  fo.history.len = fo.history_idx;

  fo.history_idx += 1;
  auto oper = AddBack( fo.history );
  oper->type = type;
  return oper;
}




Inl void
FileopenerSizestrFromSize( u8* dst, idx_t dst_len, idx_t* dst_size, u64 size )
{
#if 0
  static u64 gb = ( 1 << 30 );
  static u64 mb = ( 1 << 20 );
  static u64 kb = ( 1 << 10 );
  if( size > gb ) {
    size = Cast( u64, 0.5 + size / Cast( f64, gb ) );
    CsFrom_u64( dst, dst_len, size );
    CsAddBack( dst, Str( "g" ), 1 );
    *dst_size = CsLen( dst );
  } elif( size > mb ) {
    size = Cast( u64, 0.5 + size / Cast( f64, mb ) );
    CsFrom_u64( dst, dst_len, size );
    CsAddBack( dst, Str( "m" ), 1 );
    *dst_size = CsLen( dst );
  } elif( size > kb ) {
    size = Cast( u64, 0.5 + size / Cast( f64, kb ) );
    CsFrom_u64( dst, dst_len, size );
    CsAddBack( dst, Str( "k" ), 1 );
    *dst_size = CsLen( dst );
  } else {
    CsFrom_u64( dst, dst_len, size );
    CsAddBack( dst, Str( "b" ), 1 );
    *dst_size = CsLen( dst );
  }
#else
  CsFrom_u64( dst, dst_len, dst_size, size, 1 );
#endif
}


Inl void
FileopenerResetCS( fileopener_t& fo )
{
  fo.cursor = 0;
  fo.scroll_start = 0;
}

Inl void
FileopenerFixupCS( fileopener_t& fo )
{
  fo.cursor = MIN( fo.cursor, MAX( fo.matches.totallen, 1 ) - 1 );
  fo.scroll_start = MIN( fo.scroll_start, MAX( fo.matches.totallen, 1 ) - 1 );
}

Inl void
FileopenerMakeCursorVisible( fileopener_t& fo )
{
  FileopenerFixupCS( fo );

  bool offscreen_u = fo.scroll_start && ( fo.cursor < fo.scroll_start );
  bool offscreen_d = fo.scroll_end && ( fo.scroll_end <= fo.cursor );
  if( offscreen_u ) {
    auto dlines = fo.scroll_start - fo.cursor;
    fo.scroll_start -= dlines;
  } elif( offscreen_d ) {
    auto dlines = fo.cursor - fo.scroll_end + 1;
    fo.scroll_start += dlines;
  }

  FileopenerFixupCS( fo );
}

Inl void
ParseSpaceSeparatedList(
  plist_t& dst_mem,
  array_t<slice_t>& dst,
  buf_t& src
  )
{
  Reserve( dst, src.content_len / 2 );

  auto pos = GetBOF( src );
  Forever {
    auto elem_start = CursorSkipSpacetabR( src, pos, 0 );
    idx_t elem_len;
    auto elem_end = CursorStopAtSpacetabR( src, elem_start, &elem_len );
    if( !elem_len ) {
      auto eof = GetEOF( src );
      elem_end = CursorSkipSpacetabL( src, eof, 0 );
      if( Less( elem_end, elem_start ) ) {
        break;
      }
      elem_len = CountCharsBetween( src, elem_start, elem_end );
      if( !elem_len ) {
        break;
      }
    }

    auto elem = AddBack( dst );
    elem->mem = AddPlist( u8, dst_mem, elem_len );
    elem->len = elem_len;
    Contents( src, elem_start, ML( *elem ) );

    pos = elem_end;
  }
}

Inl void
FileopenerUpdateMatches( fileopener_t& fo )
{
  Reset( fo.matches_mem );
  Reset( fo.matches );
  fo.ignored_filetypes_list.len = 0;
  fo.ignored_substrings_list.len = 0;

  ParseSpaceSeparatedList(
    fo.matches_mem,
    fo.ignored_filetypes_list,
    fo.ignored_filetypes.buf
    );

  ParseSpaceSeparatedList(
    fo.matches_mem,
    fo.ignored_substrings_list,
    fo.ignored_substrings.buf
    );

  bool must_match_key = TxtLen( fo.query );
  auto key = AllocContents( fo.query.buf );

  if( fo.pool.totallen ) {
    auto pa_iter = MakeIteratorAtLinearIndex( fo.pool, 0 );
    For( i, 0, fo.pool.totallen ) {
      auto elem = GetElemAtIterator( fo.pool, pa_iter );
      pa_iter = IteratorMoveR( fo.pool, pa_iter );

      auto last_dot = CsScanL( ML( elem->name ), '.' );
      slice_t ext = {};
      if( last_dot ) {
        auto fileext = last_dot + 1;
        ext.mem = fileext;
        ext.len = elem->name.mem + elem->name.len - fileext;
      }

      bool include = 1;

      // ignore files with extensions in the 'ignore' list.
      if( ext.len ) {
        ForLen( j, fo.ignored_filetypes_list ) {
          auto filter = fo.ignored_filetypes_list.mem + j;
          if( CsEquals( ML( ext ), ML( *filter ), 0 ) ) {
            include = 0;
            break;
          }
        }
      }

      // ignore things which match the 'ignored_substrings' list.
      ForLen( j, fo.ignored_substrings_list ) {
        auto filter = fo.ignored_substrings_list.mem + j;
        idx_t pos;
        if( CsIdxScanR( &pos, ML( elem->name ), ML( *filter ), 0, 0 ) ) {
          include = 0;
        }
      }

      // ignore things which don't match the 'query' key.
      if( must_match_key ) {
        idx_t pos;
        if( !CsIdxScanR( &pos, ML( elem->name ), ML( key ), 0, 0 ) ) {
          include = 0;
        }
      }

      if( include ) {
        auto instance = AddBack( fo.matches, 1 );
        *instance = elem;
      }
    }
  }
  Free( key );

  FileopenerFixupCS( fo );
  FileopenerMakeCursorVisible( fo );
}

// note we only push 1 of these at a time, since it probably doesn't make sense to parallelize FsFindDirsAndFiles.
// we're primarily limited by disk speed, so more than 1 thread probably doesn't make sense.
// but, this unblocks the main thread ui, which is good since this can be slow.
__AsyncTask( AsyncTask_FileopenerFillPool )
{
  ProfFunc();

  auto ac = Cast( asynccontext_fileopenerfillpool_t*, asyncqueue_entry->asynccontext );

  Reset( ac->pool_mem );

  // TODO: add signal_quit checks into FsFindDirsAndFiles.
  // TODO: add PushMainTaskCompleted incremental results into FsFindDirsAndFiles.
  Prof( foFill_FsFindDirsAndFiles );
  array_t<dir_or_file_t> objs; // PERF: make these pointers into pool_mem ? or just make this pagearray_t ?
  Alloc( objs, ac->pool.totallen + 64 );
  FsFindDirsAndFiles( objs, ac->pool_mem, ML( ac->cwd ), 1 );
  ProfClose( foFill_FsFindDirsAndFiles );

  Prof( foFill_TotalFillEntries );
  ForLen( i, objs ) {
    if( g_mainthread.signal_quit ) {
      return;
    }
    auto obj = objs.mem + i;

    auto elem = AddBack( ac->pool, 1 );
    AssertCrash( obj->name.len >= ac->cwd.len + 1 );
    elem->name.mem = obj->name.mem + ac->cwd.len + 1; // take the name only.
    elem->name.len = obj->name.len - ac->cwd.len - 1;
    elem->is_file = obj->is_file;
    if( obj->is_file ) {
      elem->size = obj->filesize;
      elem->readonly = obj->readonly;
      elem->sizetxt.mem = AddPlist( u8, ac->pool_mem, 64 );
      FileopenerSizestrFromSize( elem->sizetxt.mem, 64, &elem->sizetxt.len, obj->filesize );
    } else {
      elem->size = 0;
      elem->readonly = 0;
      elem->sizetxt.len = 0;
    }
  }
  ProfClose( foFill_TotalFillEntries );
  Free( objs );

  // note that we don't send signals back to the main thread in a more fine-grained fashion.
  // we just send the final signal here, after everything's been done.
  // this is because the ac's results datastructure is shared across all its results.

  // TODO: pass fine-grained results back to the main thread ?
  PushMainTaskCompleted( asyncqueue_entry );
}

__MainTaskCompleted( MainTaskCompleted_FileopenerFillPool )
{
  ProfFunc();

  auto fo = Cast( fileopener_t*, maincontext );
  auto ac = Cast( asynccontext_fileopenerfillpool_t*, asynccontext );

  AssertCrash( fo->ncontexts_active );
  fo->ncontexts_active -= 1;

  if( ac->pool.totallen ) {
    auto pa_iter = MakeIteratorAtLinearIndex( ac->pool, 0 );
    For( i, 0, ac->pool.totallen ) {
      auto elem = GetElemAtIterator( ac->pool, pa_iter );
      pa_iter = IteratorMoveR( ac->pool, pa_iter );

      auto instance = AddBack( fo->pool, 1 );
      *instance = *elem;
    }
  }

  FileopenerUpdateMatches( *fo );

  *target_valid = 0;
}

Inl void
FileopenerFillPool( fileopener_t& fo )
{
  // prevent reentrancy while async stuff is executing
  if( fo.ncontexts_active ) {
    LogUI( "[EDIT] FileopenerFillPool already in progress!" );
    return;
  }

  Reset( fo.asynccontext_fillpool.pool );
  Reset( fo.asynccontext_fillpool.pool_mem );
  fo.asynccontext_fillpool.cwd.len = 0;

  Reset( fo.pool );
  Reset( fo.pool_mem );

  // add the pseudo up directory.
  {
    auto elem = AddBack<fileopener_row_t>( fo.pool );
    elem->name.len = 2;
    elem->name.mem = AddPlist( u8, fo.pool_mem, elem->name.len );
    Memmove( elem->name.mem, Str( ".." ), 2 );
    elem->is_file = 0;
    elem->size = 0;
    elem->readonly = 0;
    elem->sizetxt.len = 0;
  }

  auto cwd = AllocContents( fo.cwd.buf );
  // TODO: user error if cwd is longer than fixed-size fsobj_t can handle.
  Memmove( AddBack( fo.asynccontext_fillpool.cwd, cwd.len ), ML( cwd ) );
  Free( cwd );

  fo.ncontexts_active += 1;

  asyncqueue_entry_t entry;
  entry.FnAsyncTask = AsyncTask_FileopenerFillPool;
  entry.FnMainTaskCompleted = MainTaskCompleted_FileopenerFillPool;
  entry.asynccontext = &fo.asynccontext_fillpool;
  entry.maincontext = &fo;
  PushAsyncTask( 0, entry );
}

//#if 0
//  auto cwd = AllocContents( fo.cwd.buf );
//
//  plist_t mem;
//  Init( mem, 32768 );
//  array_t<dir_or_file_t> objs;
//  Alloc( objs, fo.pool.capacity + 64 );
//  FsFindDirsAndFiles( objs, mem, ML( cwd ), 1 );
//  ForLen( i, objs ) {
//    auto obj = objs.mem + i;
//    auto elem = AddBack( fo.pool );
//    Init( *elem );
//    AssertCrash( obj->name.len >= cwd.len + 1 );
//    auto nameonly = obj->name.mem + cwd.len + 1;
//    auto nameonly_len = obj->name.len - cwd.len - 1;
//    elem->name.len = 0;
//    Memmove( AddBack( elem->name, nameonly_len ), nameonly, nameonly_len );
//    elem->is_file = obj->is_file;
//    if( obj->is_file ) {
//      auto file = FileOpen( obj->name.mem, obj->name.len, fileopen_t::only_existing, fileop_t::R, fileop_t::RW );
//      if( file.loaded ) {
//        elem->size = file.size;
//        elem->readonly = file.readonly;
//        FileopenerSizestrFromSize( elem->sizetxt.mem, Capacity( elem->sizetxt ), &elem->sizetxt.len, elem->size );
//      }
//      FileFree( file );
//    } else {
//      elem->size = 0;
//      elem->readonly = 0;
//      elem->sizetxt.len = 0;
//    }
//  }
//  Free( cwd );
//  Free( objs );
//  Kill( mem );
//#endif

__EditCmd( CmdMode_editfile_from_fileopener )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  if( !edit.opened.len ) {
    return;
  }
  edit.mode = editmode_t::editfile;

  auto clear_search_on_switch = GetPropFromDb( bool, bool_fileopener_clear_search_on_switch );
  if( clear_search_on_switch ) {
    CmdSelectAll( edit.fileopener.query );
    CmdRemChL( edit.fileopener.query );
  }
}

Inl void
_SwitchToFileopener( edit_t& edit )
{
  edit.mode = editmode_t::fileopener;

  auto clear_search_on_switch = GetPropFromDb( bool, bool_fileopener_clear_search_on_switch );
  if( clear_search_on_switch ) {
    FileopenerUpdateMatches( edit.fileopener );
    FileopenerResetCS( edit.fileopener );
  }
}

__EditCmd( CmdMode_fileopener_from_editfile )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  _SwitchToFileopener( edit );
}

__EditCmd( CmdMode_fileopener_from_switchopened )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  _SwitchToFileopener( edit );
}

Inl void
FileopenerChangeCwd( fileopener_t& fo, slice_t dst )
{
  CmdSelectAll( fo.cwd );
  CmdRemChL( fo.cwd );
  CmdAddString( fo.cwd, Cast( idx_t, dst.mem ), dst.len );
}

Inl void
FileopenerCwdUp( fileopener_t& fo )
{
  if( !TxtLen( fo.cwd ) ) {
    return;
  }

  auto cwd = AllocContents( fo.cwd.buf );
  idx_t new_cwd_len;
  bool res = MemScanIdxRev( &new_cwd_len, ML( cwd ), "/", 1 );
  if( res ) {
    fsobj_t obj;
    obj.len = 0;
    Memmove( AddBack( obj, cwd.len ), ML( cwd ) );

    auto oper = FileopenerAddHistorical( fo, fileopener_opertype_t::changecwd );
    oper->changecwd.src = obj;

    idx_t nrem = cwd.len - new_cwd_len;
    RemBack( obj, nrem );

    oper->changecwd.dst = obj;

    FileopenerChangeCwd( fo, SliceFromArray( obj ) );
  }
  Free( cwd );
}

Inl void
FileopenerOpenRow( edit_t& edit, fileopener_row_t* row )
{
  fsobj_t name;
  name.len = 0;
  auto cwd = AllocContents( edit.fileopener.cwd.buf );
  Memmove( AddBack( name, cwd.len ), ML( cwd ) );
  Memmove( AddBack( name ), "/", 1 );
  Memmove( AddBack( name, row->name.len ), ML( row->name ) );

  if( row->is_file ) {
    auto file = FileOpen( ML( name ), fileopen_t::only_existing, fileop_t::R, fileop_t::R );
    if( file.loaded ) {
      EditOpenAndSetActiveTxt( edit, file );
      CmdMode_editfile_from_fileopener( edit );
    }
    FileFree( file );

  } else {
    bool up_dir = MemEqual( ML( row->name ), "..", 2 );
    if( up_dir ) {
      FileopenerCwdUp( edit.fileopener );

    } else {
      fsobj_t fsobj;
      fsobj.len = 0;
      Memmove( AddBack( fsobj, cwd.len ), ML( cwd ) );

      auto oper = FileopenerAddHistorical( edit.fileopener, fileopener_opertype_t::changecwd );
      oper->changecwd.src = fsobj;

      oper->changecwd.dst = name;

      FileopenerChangeCwd( edit.fileopener, SliceFromArray( name ) );
    }

    FileopenerFillPool( edit.fileopener );
    FileopenerUpdateMatches( edit.fileopener );
    FileopenerResetCS( edit.fileopener );
  }

  Free( cwd );
}


__EditCmd( CmdFileopenerUpdateMatches )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  FileopenerUpdateMatches( edit.fileopener );
}

__EditCmd( CmdFileopenerRefresh )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  FileopenerFillPool( edit.fileopener );
  FileopenerUpdateMatches( edit.fileopener );
}

__EditCmd( CmdFileopenerChoose )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  AssertCrash( edit.fileopener.cursor < edit.fileopener.matches.totallen );
  auto row = *LookupElemByLinearIndex( edit.fileopener.matches, edit.fileopener.cursor );
  FileopenerOpenRow( edit, row );
}

__EditCmd( CmdFileopenerFocusD )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  edit.fileopener.focus = Cast( fileopenerfocus_t, ( Cast( enum_t, edit.fileopener.focus ) + 1 ) % Cast( enum_t, fileopenerfocus_t::COUNT ) );
  AssertCrash( Cast( enum_t, edit.fileopener.focus ) < Cast( enum_t, fileopenerfocus_t::COUNT ) );

  // save previous cwd, in case we fail to apply a changecwd.
  if( edit.fileopener.focus == fileopenerfocus_t::dir ) {
    auto cwd = AllocContents( edit.fileopener.cwd.buf );
    Memmove( AddBack( edit.fileopener.last_cwd_for_changecwd_rollback, cwd.len ), ML( cwd ) );
    Free( cwd );
  }
}

__EditCmd( CmdFileopenerFocusU )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  edit.fileopener.focus = Cast( fileopenerfocus_t, ( Cast( enum_t, edit.fileopener.focus ) - 1 ) % Cast( enum_t, fileopenerfocus_t::COUNT ) );
  AssertCrash( Cast( enum_t, edit.fileopener.focus ) < Cast( enum_t, fileopenerfocus_t::COUNT ) );

  // save previous cwd, in case we fail to apply a changecwd.
  if( edit.fileopener.focus == fileopenerfocus_t::dir ) {
    auto cwd = AllocContents( edit.fileopener.cwd.buf );
    Memmove( AddBack( edit.fileopener.last_cwd_for_changecwd_rollback, cwd.len ), ML( cwd ) );
    Free( cwd );
  }
}

__EditCmd( CmdFileopenerCursorU )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  auto nlines = misc ? misc : 1;
  edit.fileopener.cursor -= MIN( nlines, edit.fileopener.cursor );
  FileopenerMakeCursorVisible( edit.fileopener );
}

__EditCmd( CmdFileopenerCursorD )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  auto nlines = misc ? misc : 1;
  edit.fileopener.cursor += nlines;
  FileopenerFixupCS( edit.fileopener );
  FileopenerMakeCursorVisible( edit.fileopener );
}

__EditCmd( CmdFileopenerScrollU )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  auto nlines = misc ? misc : 1;
  edit.fileopener.scroll_start -= MIN( nlines, edit.fileopener.scroll_start );
}

__EditCmd( CmdFileopenerScrollD )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  auto nlines = misc ? misc : 1;
  edit.fileopener.scroll_start += nlines;
  FileopenerFixupCS( edit.fileopener );
}



__EditCmd( CmdFileopenerRecycle )
{
  AssertCrash( edit.mode == editmode_t::fileopener );

  AssertCrash( edit.fileopener.cursor < edit.fileopener.matches.totallen );
  auto row = *LookupElemByLinearIndex( edit.fileopener.matches, edit.fileopener.cursor );

  fsobj_t name;
  name.len = 0;
  auto cwd = AllocContents( edit.fileopener.cwd.buf );
  Memmove( AddBack( name, cwd.len ), ML( cwd ) );
  Free( cwd );
  Memmove( AddBack( name ), "/", 1 );
  Memmove( AddBack( name, row->name.len ), ML( row->name ) );

  if( row->is_file ) {
    auto oper = FileopenerAddHistorical( edit.fileopener, fileopener_opertype_t::delfile );
    oper->delfile = name;

    if( FileRecycle( ML( name ) ) ) {
      FileopenerFillPool( edit.fileopener );
      FileopenerUpdateMatches( edit.fileopener );
    } else {
      auto tmp = AllocCstr( ML( name ) );
      LogUI( "[DIR] Failed to delete file: \"%s\"!", tmp );
      MemHeapFree( tmp );
    }
  } else {
    auto oper = FileopenerAddHistorical( edit.fileopener, fileopener_opertype_t::deldir );
    oper->deldir = name;

    if( DirRecycle( ML( name ) ) ) {
      FileopenerFillPool( edit.fileopener );
      FileopenerUpdateMatches( edit.fileopener );
    } else {
      auto tmp = AllocCstr( ML( name ) );
      LogUI( "[DIR] Failed to delete edit.fileopener: \"%s\"!", tmp );
      MemHeapFree( tmp );
    }
  }
  FileopenerFixupCS( edit.fileopener );
}


__EditCmd( CmdFileopenerChangeCwdUp )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  FileopenerCwdUp( edit.fileopener );
  FileopenerFillPool( edit.fileopener );
  FileopenerUpdateMatches( edit.fileopener );
  FileopenerResetCS( edit.fileopener );
}

__EditCmd( CmdFileopenerNewFile )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  u8* default_name = Str( "new_file" );
  idx_t default_name_len = CsLen( default_name );

  fsobj_t name;
  name.len = 0;
  auto cwd = AllocContents( edit.fileopener.cwd.buf );
  Memmove( AddBack( name, cwd.len ), ML( cwd ) );
  Free( cwd );
  Memmove( AddBack( name ), "/", 1 );
  Memmove( AddBack( name, default_name_len ), default_name, default_name_len );
  Memmove( AddBack( name ), "0", 1 );
  Memmove( AddBack( name, 4 ), ".txt", 4 );

  u32 suffix_num = 0;
  idx_t last_suffix_len = 1;
  embeddedarray_t<u8, 64> suffix;
  Forever {
    bool exists = FileExists( ML( name ) );
    if( !exists ) {
      break;
    }
    RemBack( name, 4 + last_suffix_len );

    suffix_num += 1;
    CsFromIntegerU( suffix.mem, Capacity( suffix ), &suffix.len, suffix_num );
    last_suffix_len = suffix.len;

    Memmove( AddBack( name, suffix.len ), ML( suffix ) );
    Memmove( AddBack( name, 4 ), ".txt", 4 );
  }

  auto oper = FileopenerAddHistorical( edit.fileopener, fileopener_opertype_t::newfile );
  oper->newfile = name;

  file_t file = FileOpen( ML( name ), fileopen_t::only_new, fileop_t::RW, fileop_t::R );
  AssertWarn( file.loaded );
  FileFree( file );
  FileopenerFillPool( edit.fileopener );
  FileopenerUpdateMatches( edit.fileopener );
}

__EditCmd( CmdFileopenerNewDir )
{
  AssertCrash( edit.mode == editmode_t::fileopener );
  u8* default_name = Str( "new_dir" );
  idx_t default_name_len = CsLen( default_name );

  fsobj_t name;
  name.len = 0;
  auto cwd = AllocContents( edit.fileopener.cwd.buf );
  Memmove( AddBack( name, cwd.len ), ML( cwd ) );
  Free( cwd );
  Memmove( AddBack( name ), "/", 1 );
  Memmove( AddBack( name, default_name_len ), default_name, default_name_len );
  Memmove( AddBack( name ), "0", 1 );

  u32 suffix_num = 0;
  idx_t last_suffix_len = 1;
  embeddedarray_t<u8, 64> suffix;
  Forever {
    bool exists = DirExists( ML( name ) );
    if( !exists ) {
      break;
    }
    RemBack( name, last_suffix_len );

    suffix_num += 1;
    CsFromIntegerU( suffix.mem, Capacity( suffix ), &suffix.len, suffix_num );
    last_suffix_len = suffix.len;

    Memmove( AddBack( name, suffix.len ), ML( suffix ) );
  }

  auto oper = FileopenerAddHistorical( edit.fileopener, fileopener_opertype_t::newdir );
  oper->newfile = name;

  bool created = DirCreate( ML( name ) );
  AssertWarn( created );
  FileopenerFillPool( edit.fileopener );
  FileopenerUpdateMatches( edit.fileopener );
}


__EditCmd( CmdMode_fileopener_renaming_from_fileopener )
{
  AssertCrash( edit.mode == editmode_t::fileopener );

  edit.mode = editmode_t::fileopener_renaming;

  AssertCrash( edit.fileopener.cursor < edit.fileopener.matches.totallen );
  auto row = *LookupElemByLinearIndex( edit.fileopener.matches, edit.fileopener.cursor );
  edit.fileopener.renaming_row = edit.fileopener.cursor;

  CmdSelectAll( edit.fileopener.renaming_txt );
  CmdAddString( edit.fileopener.renaming_txt, Cast( idx_t, row->name.mem ), row->name.len );
  CmdSelectAll( edit.fileopener.renaming_txt );
}

Inl void
FileopenerRename( edit_t& edit, slice_t& src, slice_t& dst )
{
  if( FileMove( ML( dst ), ML( src ) ) ) {
    FileopenerFillPool( edit.fileopener );
    FileopenerUpdateMatches( edit.fileopener );
  } else {
    auto tmp0 = AllocCstr( src );
    auto tmp1 = AllocCstr( dst );
    LogUI( "[DIR] Failed to rename: \"%s\" -> \"%s\"!", tmp0, tmp1 );
    MemHeapFree( tmp0 );
    MemHeapFree( tmp1 );
  }
}

__EditCmd( CmdFileopenerRenamingApply )
{
  AssertCrash( edit.mode == editmode_t::fileopener_renaming );

  AssertCrash( edit.fileopener.renaming_row < edit.fileopener.matches.totallen );
  auto row = *LookupElemByLinearIndex( edit.fileopener.matches, edit.fileopener.renaming_row );

  // TODO: prevent entry into mode fileopener_renaming, if the cursor is on '..'
  // Renaming the dummy parent edit.fileopener does bad things!
  if( !MemEqual( "..", 2, ML( row->name ) ) ) {
    auto cwd = AllocContents( edit.fileopener.cwd.buf );

    fsobj_t newname;
    newname.len = 0;
    Memmove( AddBack( newname, cwd.len ), ML( cwd ) );
    Memmove( AddBack( newname ), "/", 1 );
    idx_t newnameonly_len = TxtLen( edit.fileopener.renaming_txt );
    Contents(
      edit.fileopener.renaming_txt.buf,
      GetBOF( edit.fileopener.renaming_txt.buf ),
      AddBack( newname, newnameonly_len ),
      newnameonly_len
      );

    fsobj_t name;
    name.len = 0;
    Memmove( AddBack( name, cwd.len ), ML( cwd ) );
    Memmove( AddBack( name ), "/", 1 );
    Memmove( AddBack( name, row->name.len ), ML( row->name ) );

    auto oper = FileopenerAddHistorical( edit.fileopener, fileopener_opertype_t::rename );
    oper->rename.src = name;
    oper->rename.dst = newname;

    auto src = SliceFromArray( name );
    auto dst = SliceFromArray( newname );
    FileopenerRename( edit, src, dst );

    Free( cwd );
  }

  edit.fileopener.renaming_row = 0;
  CmdSelectAll( edit.fileopener.renaming_txt );
  CmdRemChL( edit.fileopener.renaming_txt );

  FileopenerFixupCS( edit.fileopener );

  edit.mode = editmode_t::fileopener;
}

__EditCmd( CmdFileopenerUndo ) // TODO: finish writing this.
{
//  DEBUG_PrintUndoRedo( dir, "PRE-UNDO\n" );
  AssertCrash( edit.fileopener.history_idx <= edit.fileopener.history.len );
  AssertCrash( edit.fileopener.history_idx == edit.fileopener.history.len  ||  edit.fileopener.history.mem[edit.fileopener.history_idx].type == fileopener_opertype_t::checkpt );
  if( !edit.fileopener.history_idx ) {
    return;
  }

  bool loop = 1;
  while( loop ) {
    edit.fileopener.history_idx -= 1;
    auto oper = edit.fileopener.history.mem + edit.fileopener.history_idx;

    switch( oper->type ) {
      case fileopener_opertype_t::checkpt: {
        loop = 0;
      } break;

      // undo this operation:

      case fileopener_opertype_t::rename: {
        slice_t src;
        slice_t dst;
        dst.mem = oper->rename.src.mem;
        dst.len = oper->rename.src.len;
        src.mem = oper->rename.dst.mem;
        src.len = oper->rename.dst.len;
        FileopenerRename( edit, src, dst );
      } break;
      case fileopener_opertype_t::changecwd: {
        slice_t dst;
        dst.mem = oper->changecwd.src.mem;
        dst.len = oper->changecwd.src.len;
        FileopenerChangeCwd( edit.fileopener, dst );
      } break;
      case fileopener_opertype_t::delfile: {
      } break;
      case fileopener_opertype_t::deldir: {
      } break;
      case fileopener_opertype_t::newfile: {
      } break;
      case fileopener_opertype_t::newdir: {
      } break;
      default: UnreachableCrash();
    }
    CompileAssert( Cast( enum_t, fileopener_opertype_t::COUNT ) == 7 );
  }

  AssertCrash( edit.fileopener.history_idx == edit.fileopener.history.len  ||  edit.fileopener.history.mem[edit.fileopener.history_idx].type == fileopener_opertype_t::checkpt );
//  DEBUG_PrintUndoRedo( edit.fileopener, "POST-UNDO\n" );
}

__EditCmd( CmdFileopenerRedo )
{
}






void
EditInit( edit_t& edit )
{
  Init( edit.mem, 32768 );

  edit.mode = editmode_t::fileopener;
  Init( edit.opened, &edit.mem );

  Init( edit.openedmru, &edit.mem );

  edit.active[0] = 0;
  edit.active[1] = 0;
  edit.horzview = 0;
  edit.horzfocus_l = 0;

  Alloc( edit.search_matches, 128 );
  Init( edit.opened_search );
  TxtLoadEmpty( edit.opened_search );
  edit.opened_cursor = 0;
  edit.opened_scroll_start = 0;
  edit.opened_scroll_end = 0;
  edit.nlines_screen = 0;

  Init( edit.findinfiles );

  Init( edit.fileopener );
  CmdFileopenerRefresh( edit );

  Init( edit.gotoline );
  TxtLoadEmpty( edit.gotoline );

  Init( edit.findrepl );

  edit.mode_before_externalmerge = edit.mode;
  edit.horzview_before_externalmerge = edit.horzview;
  edit.active_externalmerge = 0;
  edit.file_externalmerge = {};
}

void
EditKill( edit_t& edit )
{
  edit.mode = editmode_t::fileopener;
  ForList( elem, edit.opened ) {
    Kill( elem->value.txt );
  }
  Kill( edit.opened );

  Kill( edit.openedmru );

  Free( edit.search_matches );
  Kill( edit.opened_search );
  edit.opened_cursor = 0;
  edit.opened_scroll_start = 0;
  edit.opened_scroll_end = 0;
  edit.nlines_screen = 0;

  Kill( edit.findinfiles );

  edit.active[0] = 0;
  edit.active[1] = 0;
  Kill( edit.fileopener );
  Kill( edit.gotoline );

  Kill( edit.findrepl );

  edit.mode_before_externalmerge = edit.mode;
  edit.horzview_before_externalmerge = edit.horzview;
  edit.active_externalmerge = 0;
  edit.file_externalmerge = {};

  Kill( edit.mem );
}




Inl edittxtopen_t*
GetActiveOpen( edit_t& edit )
{
  return edit.active[edit.horzfocus_l];
}

Inl txt_t&
GetActiveFindReplaceTxt( findreplace_t& fr )
{
  if( fr.focus_find ) {
    return fr.find;
  } else {
    return fr.replace;
  }
}




__EditCmd( CmdMode_editfile_gotoline_from_editfile )     { edit.mode = editmode_t::editfile_gotoline; }
__EditCmd( CmdMode_fileopener_from_fileopener_renaming ) { edit.mode = editmode_t::fileopener; }
__EditCmd( CmdMode_editfile_from_editfile_findrepl )     { edit.mode = editmode_t::editfile; }
__EditCmd( CmdMode_editfile_from_editfile_gotoline )     { edit.mode = editmode_t::editfile; }








// =================================================================================
// FINDIN FILES

Inl void
FindinFilesResetCS( findinfiles_t& fif )
{
  fif.cursor = 0;
  fif.scroll_start = 0;
}

Inl void
FindinFilesMakeCursorVisible( findinfiles_t& fif )
{
  bool offscreen_u = fif.scroll_start  &&  ( fif.cursor < fif.scroll_start );
  bool offscreen_d = fif.scroll_end  &&  ( fif.scroll_end <= fif.cursor );
  if( offscreen_u ) {
    fif.scroll_start = fif.cursor;
  } elif( offscreen_d ) {
    auto dlines = fif.cursor - fif.scroll_end + 1;
    fif.scroll_start += dlines;
  }
}




Inl void
AsyncFileContentSearch( asynccontext_filecontentsearch_t* ac )
{
  For( i, ac->filenames_start, ac->filenames_start + ac->filenames_count ) {
    auto obj = ac->filenames->mem + i;

    if( g_mainthread.signal_quit ) {
      return;
    }

    Prof( tmp_ApplyFilterFiletype );
    auto last_dot = CsScanL( ML( *obj ), '.' );
    slice_t ext = {};
    if( last_dot ) {
      auto fileext = last_dot + 1;
      ext.mem = fileext;
      ext.len = obj->mem + obj->len - fileext;
    }
    bool include = 1;
    if( ext.len ) {
      ForLen( j, *ac->ignored_filetypes_list ) {
        auto filter = ac->ignored_filetypes_list->mem + j;
        if( CsEquals( ML( ext ), ML( *filter ), 0 ) ) {
          include = 0;
          break;
        }
      }
    }
    ProfClose( tmp_ApplyFilterFiletype );
    if( include ) {
      Prof( tmp_FileOpen );
      file_t file = FileOpen( ML( *obj ), fileopen_t::only_existing, fileop_t::R, fileop_t::RW );
      ProfClose( tmp_FileOpen );
      // TODO: log when !file.loaded
      if( file.loaded ) {
        // collect all instances of the key.

#if USE_BUF_FOR_FILECONTENTSEARCH
        buf_t buf;
        Init( buf );
        BufLoad( buf, file );
        // TODO: close file after loading? we don't need to writeback or anything.
        bool found = 1;
        content_ptr_t pos = GetBOF( buf );
        while( found ) {
          content_ptr_t pos_match;
          FindFirstR(
            buf,
            pos,
            ML( ac->key ),
            &pos_match,
            &found,
            ac->case_sens,
            ac->word_boundary
            );
          if( found ) {
            Prof( tmp_AddMatch );
            auto instance = AddBack( ac->matches, 1 );
            instance->name = *obj;
            instance->l = pos_match;
            instance->r = CursorCharR( buf, pos_match, ac->key.len, 0 );
            instance->len = ac->key.len;
            auto sample_start = MIN(
              instance->l,
              CursorSkipSpacetabR( buf, CursorStopAtNewlineL( buf, pos_match, 0 ), 0 )
              );
            auto sample_end = MIN(
              CursorCharR( buf, sample_start, 4096, 0 ), // Samples don't scroll, so this should be wide enough for anyone.
              CursorSkipSpacetabL( buf, CursorStopAtNewlineR( buf, pos_match, 0 ), 0 )
              );
            instance->sample.len = CountBytesBetween( buf, sample_start, sample_end );
            instance->sample.mem = AddPlist( u8, ac->mem, instance->sample.len );
            AssertCrash( LEqual( sample_start, instance->l ) );
            instance->offset_into_sample = CountCharsBetween( buf, sample_start, instance->l );
            Contents( buf, sample_start, ML( instance->sample ) );
            ProfClose( tmp_AddMatch );
          }
        }

        Kill( buf );
#else
        Prof( tmp_FileAlloc );
        string_t mem = FileAlloc( file );
        ProfClose( tmp_FileAlloc );
        Prof( tmp_ContentSearch );
        bool found = 1;
        idx_t pos = 0;
        while( found ) {
          idx_t res = 0;
          found = CsIdxScanR( &res, mem.mem + pos, mem.len - pos, ML( ac->key ), ac->case_sens, ac->word_boundary );
          if( found ) {
            Prof( tmp_AddMatch );
            pos += res;

            auto instance = AddBack( ac->matches, 1 );
            instance->name = *obj;
            instance->l = pos;
            instance->r = pos + ac->key.len;
            instance->len = ac->key.len;
            auto sample_start = MIN( instance->l, CursorSkipSpacetabR( ML( mem ), CursorStopAtNewlineL( ML( mem ), pos ) ) );
            auto sample_end = MIN( sample_start + 4096, CursorSkipSpacetabL( ML( mem ), CursorStopAtNewlineR( ML( mem ), pos ) ) ); // Samples don't scroll, so this should be wide enough for anyone.
            instance->sample.len = sample_end - sample_start;
            instance->sample.mem = AddPlist( u8, ac->mem, instance->sample.len );
            AssertCrash( sample_start <= instance->l );
            instance->offset_into_sample = instance->l - sample_start;
            Memmove( instance->sample.mem, mem.mem + sample_start, instance->sample.len );

            pos = CursorCharR( ML( mem ), pos );
            ProfClose( tmp_AddMatch );
          }
        }
        ProfClose( tmp_ContentSearch );
        Free( mem );
#endif
      }
      FileFree( file );
    }
  }
}

__AsyncTask( AsyncTask_FileContentSearch )
{
  ProfFunc();

  auto ac = Cast( asynccontext_filecontentsearch_t*, asyncqueue_entry->asynccontext );
  AsyncFileContentSearch( ac );

  // note that we don't send signals back to the main thread in a more fine-grained fashion.
  // we just send the final signal here, after everything's been done.
  // this is because the ac's results datastructure is shared across all its results.
  // our strategy of splitting into small, separate ac's at the start seems to work well enough.
  PushMainTaskCompleted( asyncqueue_entry );
}

__MainTaskCompleted( MainTaskCompleted_FileContentSearch )
{
  ProfFunc();

  auto fif = Cast( findinfiles_t*, maincontext );
  auto ac = Cast( asynccontext_filecontentsearch_t*, asynccontext );

  AssertCrash( fif->ncontexts_active );
  fif->ncontexts_active -= 1;

  if( ac->matches.totallen ) {
    auto pa_iter = MakeIteratorAtLinearIndex( ac->matches, 0 );
    For( i, 0, ac->matches.totallen ) {
      auto elem = GetElemAtIterator( ac->matches, pa_iter );
      pa_iter = IteratorMoveR( ac->matches, pa_iter );

      auto instance = AddBack( fif->matches, 1 );
      *instance = *elem;
    }
  }

  *target_valid = 0;
}


__EditCmd( CmdFindinfilesRefresh )
{
  Prof( tmp_CmdFindinfilesRefresh );
  AssertCrash( edit.mode == editmode_t::findinfiles );

  auto& fif = edit.findinfiles;

  // prevent reentrancy while async stuff is executing
  if( fif.ncontexts_active ) {
    LogUI( "[EDIT] FindinFilesRefresh already in progress!" );
    return;
  }

  ForLen( i, fif.asynccontexts ) {
    auto ac = fif.asynccontexts.mem + i;
    Kill( ac->mem );
    Kill( ac->matches );
  }
  fif.asynccontexts.len = 0;

  fif.filenames.len = 0;
  fif.ignored_filetypes_list.len = 0;
  Free( fif.key );

  Reset( fif.matches );
  Reset( fif.mem );

  if( TxtLen( fif.query ) ) {
    fif.matches_dir.len = TxtLen( fif.dir );
    fif.matches_dir.mem = AddPlist( u8, fif.mem, fif.matches_dir.len );
    Contents( fif.dir.buf, GetBOF( fif.dir.buf ), ML( fif.matches_dir ) );

    Prof( tmp_MakeFilterFiletypes );

    ParseSpaceSeparatedList(
      fif.mem,
      fif.ignored_filetypes_list,
      fif.ignored_filetypes.buf
      );

    fif.key = AllocContents( fif.query.buf );

    ProfClose( tmp_MakeFilterFiletypes );

    // This call is ~3% of the cost of this function, after the OS / filesys caches filesys metadata.
    // So, we'll do this part on the main thread, then fan out.
    //
    // If we really need to, we could multithread this:
    // - Have some input queues of directories to process.
    // - Have some result queues of files.
    // - Each thread will:
    //   - Pop off a directory.
    //   - Enumerate all child files and directories.
    //   - Push files onto result queues.
    //   - Push child directories onto input queues.
    Prof( tmp_FsFindFiles );
    fif.filenames.len = 0;
    FsFindFiles( fif.filenames, fif.mem, ML( fif.matches_dir ), 1 );
    ProfClose( tmp_FsFindFiles );

    // PERF: optimize chunksize and per-thread-queue sizes
    // PERF: filesize-level chunking, for a more even distribution.
    constant idx_t chunksize = 512;
    auto quo = fif.filenames.len / chunksize;
    auto rem = fif.filenames.len % chunksize;
    Reserve( fif.asynccontexts, quo + 1 );
    For( i, 0, quo + 1 ) {
      auto count = ( i == quo )  ?  rem  :  chunksize;
      if( count ) {
        auto ac = AddBack( fif.asynccontexts );
        ac->filenames = &fif.filenames;
        ac->filenames_start = i * chunksize;
        ac->filenames_count = count;
        ac->ignored_filetypes_list = &fif.ignored_filetypes_list;
        ac->key = SliceFromString( fif.key );
        ac->case_sens = fif.case_sens;
        ac->word_boundary = fif.word_boundary;
        Init( ac->mem, 128*1024 );
        Init( ac->matches, 256 );

        fif.ncontexts_active += 1;
        fif.max_ncontexts_active = MAX( fif.max_ncontexts_active, fif.ncontexts_active );

        asyncqueue_entry_t entry;
        entry.FnAsyncTask = AsyncTask_FileContentSearch;
        entry.FnMainTaskCompleted = MainTaskCompleted_FileContentSearch;
        entry.asynccontext = ac;
        entry.maincontext = &fif;
        PushAsyncTask( i, entry );
      }
    }
  }

  FindinFilesResetCS( fif );
}



__EditCmd( CmdMode_findinfiles_from_editfile )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  edit.mode = editmode_t::findinfiles;
//  CmdFindinfilesRefresh( edit );
}

__EditCmd( CmdMode_editfile_from_findinfiles )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  edit.mode = editmode_t::editfile;
//  edit.findinfiles.matches.len = 0;
//  CmdSelectAll( edit.findinfiles.query );
//  CmdRemChL( edit.findinfiles.query );
//  FindinFilesResetCS( edit.findinfiles );
}

__EditCmd( CmdFindinfilesChoose )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto foundinfile = LookupElemByLinearIndex( fif.matches, fif.cursor );
  auto file = FileOpen( ML( foundinfile->name ), fileopen_t::only_existing, fileop_t::R, fileop_t::R );
  if( file.loaded ) {
    EditOpenAndSetActiveTxt( edit, file );
    CmdMode_editfile_from_findinfiles( edit );
    auto open = GetActiveOpen( edit );
    AssertCrash( open );
#if USE_BUF_FOR_FILECONTENTSEARCH
    CmdSetSelection( open->txt, Cast( idx_t, &foundinfile->l ), Cast( idx_t, &foundinfile->r ) );
#else
    auto bof = GetBOF( open->txt.buf );
    auto sel_l = CursorCharR( open->txt.buf, bof, foundinfile->l, 0 );
    auto sel_r = CursorCharR( open->txt.buf, sel_l, foundinfile->len, 0 );
    CmdSetSelection( open->txt, Cast( idx_t, &sel_l ), Cast( idx_t, &sel_r ) );
#endif
  }
  FileFree( file );
}

__EditCmd( CmdFindinfilesFocusD )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  fif.focus = Cast( findinfilesfocus_t, ( Cast( enum_t, fif.focus ) + 1 ) % Cast( enum_t, findinfilesfocus_t::COUNT ) );
  AssertCrash( Cast( enum_t, fif.focus ) < Cast( enum_t, findinfilesfocus_t::COUNT ) );
}

__EditCmd( CmdFindinfilesFocusU )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  fif.focus = Cast( findinfilesfocus_t, ( Cast( enum_t, fif.focus ) - 1 ) % Cast( enum_t, findinfilesfocus_t::COUNT ) );
  AssertCrash( Cast( enum_t, fif.focus ) < Cast( enum_t, findinfilesfocus_t::COUNT ) );
}

__EditCmd( CmdFindinfilesCursorU )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto nlines = misc ? misc : 1;
  fif.cursor -= MIN( nlines, fif.cursor );
  FindinFilesMakeCursorVisible( fif );
}

__EditCmd( CmdFindinfilesCursorD )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto nlines = misc ? misc : 1;
  fif.cursor += MIN( nlines, fif.matches.totallen - 1 - fif.cursor );
  FindinFilesMakeCursorVisible( fif );
}

__EditCmd( CmdFindinfilesScrollU )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto nlines = misc ? misc : 1;
  fif.scroll_start -= MIN( nlines, fif.scroll_start );
}

__EditCmd( CmdFindinfilesScrollD )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto nlines = misc ? misc : 1;
  fif.scroll_start += MIN( nlines, fif.matches.totallen - 1 - fif.scroll_start );
}

Inl void
ReplaceInFile( edit_t& edit, foundinfile_t* match, slice_t query, slice_t replacement )
{
  auto open = EditGetOpenedFile( edit, ML( match->name ) );
  if( !open ) {
    auto file = FileOpen( ML( match->name ), fileopen_t::only_existing, fileop_t::R, fileop_t::R );
    if( file.loaded ) {
      bool opened_existing = 0;
      EditOpen( edit, file, &open, &opened_existing );
      AssertCrash( open );
    } else {
      auto cstr = AllocCstr( match->name );
      LogUI( "[EDIT] ReplaceInFile failed to load file: \"%s\"", cstr );
      MemHeapFree( cstr );
    }
    FileFree( file );
  }
  if( open ) {

#if USE_BUF_FOR_FILECONTENTSEARCH
    CmdSetSelection( open->txt, Cast( idx_t, &match->l ), Cast( idx_t, &match->r ) );
#else
    auto bof = GetBOF( open->txt.buf );
    auto sel_l = CursorCharR( open->txt.buf, bof, match->l, 0 );
    auto sel_r = CursorCharR( open->txt.buf, sel_l, match->len, 0 );
    CmdSetSelection( open->txt, Cast( idx_t, &sel_l ), Cast( idx_t, &sel_r ) );
#endif

    auto contents = AllocSelection( open->txt );
    if( CsEquals( ML( contents ), ML( query ), 1 ) ) {
      CmdAddString( open->txt, Cast( idx_t, replacement.mem ), replacement.len );
      open->unsaved = 1;
    } else {
      auto cstr0 = AllocCstr( query );
      auto cstr1 = AllocCstr( contents );
      LogUI( "[EDIT] ReplaceInFile failed, query \"%s\" didn't match contents \"%s\"!", cstr0, cstr1 );
      MemHeapFree( cstr0 );
      MemHeapFree( cstr1 );
    }
    Free( contents );
  }
}

__EditCmd( CmdFindinfilesReplaceAtCursor )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto replacement = AllocContents( fif.replacement.buf );
  auto query = AllocContents( fif.query.buf );
  auto match = LookupElemByLinearIndex( fif.matches, fif.cursor );
  ReplaceInFile( edit, match, SliceFromString( query ), SliceFromString( replacement ) );
  Free( replacement );
  Free( query );
}

__EditCmd( CmdFindinfilesReplaceAll )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  auto replacement = AllocContents( fif.replacement.buf );
  auto query = AllocContents( fif.query.buf );
  ReverseForPrev( page, fif.matches.current_page ) {
    ReverseForLen( i, *page ) {
      auto match = Cast( foundinfile_t*, page->mem ) + i;
      ReplaceInFile( edit, match, SliceFromString( query ), SliceFromString( replacement ) );
    }
  }
  Free( replacement );
  Free( query );
}

__EditCmd( CmdFindinfilesToggleCaseSens )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  fif.case_sens = !fif.case_sens;
}

__EditCmd( CmdFindinfilesToggleWordBoundary )
{
  AssertCrash( edit.mode == editmode_t::findinfiles );
  auto& fif = edit.findinfiles;
  fif.word_boundary = !fif.word_boundary;
}






// =================================================================================
// EDIT FILE


__EditCmd( CmdMode_editfile_findrepl_from_editfile )
{
  edit.mode = editmode_t::editfile_findrepl;
  edit.findrepl.focus_find = 1;
}

__EditCmd( CmdEditfileFindreplToggleFocus )
{
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  edit.findrepl.focus_find = !edit.findrepl.focus_find;
}

__EditCmd( CmdEditfileFindreplToggleCaseSens )
{
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  edit.findrepl.case_sens = !edit.findrepl.case_sens;
}

__EditCmd( CmdEditfileFindreplToggleWordBoundary )
{
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  edit.findrepl.word_boundary = !edit.findrepl.word_boundary;
}




Inl void
Save( edit_t& edit, edittxtopen_t* open )
{
  AssertCrash( open );
  file_t file = FileOpen( ML( open->txt.filename ), fileopen_t::only_existing, fileop_t::W, fileop_t::R );
  if( file.loaded ) {
    if( open->time_lastwrite != file.time_lastwrite ) {
      auto tmp = AllocCstr( ML( open->txt.filename ) );
      u64 timediff =
        MAX( open->time_lastwrite, file.time_lastwrite ) -
        MIN( open->time_lastwrite, file.time_lastwrite );
      LogUI( "[EDIT SAVE] Warning: stomping on external changes made %llu seconds ago: \"%s\"", timediff, tmp );
      MemHeapFree( tmp );
    }
    TxtSave( open->txt, file );
    open->unsaved = 0;
    open->time_lastwrite = file.time_lastwrite;
  } else {
    file = FileOpen( ML( open->txt.filename ), fileopen_t::only_new, fileop_t::W, fileop_t::R );
    AssertWarn( file.loaded );
    if( file.loaded ) {
      TxtSave( open->txt, file );
      open->unsaved = 0;
      open->time_lastwrite = file.time_lastwrite;
    } else {
      auto tmp = AllocCstr( ML( open->txt.filename ) );
      LogUI( "[EDIT SAVE] Failed to open file for write: \"%s\"", tmp );
      MemHeapFree( tmp );
    }
  }
  FileFree( file );
}

__EditCmd( CmdSave )
{
  ProfFunc();
  auto open = GetActiveOpen( edit );
  if( !open ) {
    return;
  }
  Save( edit, open );
}

__EditCmd( CmdSaveAll )
{
  ProfFunc();
  ForList( elem, edit.opened ) {
    auto open = &elem->value;
    Save( edit, open );
  }
}

__EditCmd( CmdEditfileFindreplFindR )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  auto open = GetActiveOpen( edit );
  if( !open ) {
    return;
  }
  auto find = AllocContents( edit.findrepl.find.buf );
  txtfind_t txtfind = { ML( find ), edit.findrepl.case_sens, edit.findrepl.word_boundary };
  CmdFindStringR( open->txt, Cast( idx_t, &txtfind ) );
  Free( find );
}

__EditCmd( CmdEditfileFindreplFindL )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  auto open = GetActiveOpen( edit );
  if( !open ) {
    return;
  }
  auto find = AllocContents( edit.findrepl.find.buf );
  txtfind_t txtfind = { ML( find ), edit.findrepl.case_sens, edit.findrepl.word_boundary };
  CmdFindStringL( open->txt, Cast( idx_t, &txtfind ) );
  Free( find );
}

Inl void
ReplaceSelection( txt_t& txt, slice_t find, slice_t replace )
{
  content_ptr_t sl, sr;
  GetSelect( txt, &sl, &sr );
  auto sel = AllocContents( txt.buf, sl, CountBytesBetween( txt.buf, sl, sr ) );
  if( MemEqual( ML( find ), ML( sel ) ) ) {
    CmdAddString( txt, Cast( idx_t, replace.mem ), replace.len );
  }
  Free( sel );
}

__EditCmd( CmdEditfileFindreplReplaceR )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  auto open = GetActiveOpen( edit );
  if( !open ) {
    return;
  }
  open->unsaved = 1;
  auto find = AllocContents( edit.findrepl.find.buf );
  auto repl = AllocContents( edit.findrepl.replace.buf );
  ReplaceSelection( open->txt, SliceFromString( find ), SliceFromString( repl ) );
  txtfind_t txtfind = { ML( find ), edit.findrepl.case_sens, edit.findrepl.word_boundary };
  CmdFindStringR( open->txt, Cast( idx_t, &txtfind ) );
  Free( find );
  Free( repl );
}

__EditCmd( CmdEditfileFindreplReplaceL )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::editfile_findrepl );
  auto open = GetActiveOpen( edit );
  if( !open ) {
    return;
  }
  open->unsaved = 1;
  auto find = AllocContents( edit.findrepl.find.buf );
  auto repl = AllocContents( edit.findrepl.replace.buf );
  ReplaceSelection( open->txt, SliceFromString( find ), SliceFromString( repl ) );
  txtfind_t txtfind = { ML( find ), edit.findrepl.case_sens, edit.findrepl.word_boundary };
  CmdFindStringL( open->txt, Cast( idx_t, &txtfind ) );
  Free( find );
  Free( repl );
}

__EditCmd( CmdEditfileGotolineChoose )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::editfile_gotoline );
  auto open = GetActiveOpen( edit );
  if( !open ) {
    return;
  }
  auto gotoline = AllocContents( edit.gotoline.buf );
  bool valid = 1;
  For( i, 0, gotoline.len ) {
    if( !IsNumber( gotoline.mem[i] ) ) {
      valid = 0;
      break;
    }
  }
  if( valid ) {
    auto lineno = CsTo_u64( ML( gotoline ) );
    AssertCrash( lineno <= MAX_idx );
    if( lineno ) {
      lineno -= 1;
    }
    CmdCursorGotoline( open->txt, Cast( idx_t, lineno ) );
    CmdSelectAll( edit.gotoline );
    CmdRemChL( edit.gotoline );
    CmdMode_editfile_from_editfile_gotoline( edit );
  }
  Free( gotoline );
}

__EditCmd( CmdEditfileSwapHorz )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  auto open_l = edit.active[1];
  auto open_r = edit.active[0];
  if( open_l  &&  open_r ) {
    open_l->horz_l = !open_l->horz_l;
    open_r->horz_l = !open_r->horz_l;
  }
  edit.active[1] = open_r;
  edit.active[0] = open_l;
  edit.horzfocus_l = !edit.horzfocus_l;
}

__EditCmd( CmdEditfileSwapHorzFocus )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  edit.horzfocus_l = !edit.horzfocus_l;
  auto open = edit.active[edit.horzfocus_l];
  if( open ) {
    MoveOpenedToFrontOfMru( edit, open );
  }
}

__EditCmd( CmdEditfileMoveHorzL )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  if( !edit.horzfocus_l ) {
    auto open_focus = edit.active[edit.horzfocus_l];
    if( open_focus ) {
      open_focus->horz_l = !open_focus->horz_l;
    }
    edit.horzfocus_l = !edit.horzfocus_l;
    edit.active[edit.horzfocus_l] = open_focus;
  }
}

__EditCmd( CmdEditfileMoveHorzR )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  if( edit.horzfocus_l ) {
    auto open_focus = edit.active[edit.horzfocus_l];
    if( open_focus ) {
      open_focus->horz_l = !open_focus->horz_l;
    }
    edit.horzfocus_l = !edit.horzfocus_l;
    edit.active[edit.horzfocus_l] = open_focus;
  }
}

__EditCmd( CmdEditfileToggleHorzview )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  edit.horzview = !edit.horzview;
}







// =================================================================================
// SWITCH OPENED

Inl void
MakeOpenedCursorVisible( edit_t& edit )
{
  edit.opened_scroll_start = MIN( edit.opened_scroll_start, MAX( edit.search_matches.len, 1 ) - 1 );
  edit.opened_cursor = MIN( edit.opened_cursor, MAX( edit.search_matches.len, 1 ) - 1 );

  bool offscreen_u = edit.opened_scroll_start && ( edit.opened_cursor < edit.opened_scroll_start );
  bool offscreen_d = edit.opened_scroll_end && ( edit.opened_scroll_end <= edit.opened_cursor );
  if( offscreen_u ) {
    auto dlines = edit.opened_scroll_start - edit.opened_cursor;
    edit.opened_scroll_start -= dlines;
  } elif( offscreen_d ) {
    auto dlines = edit.opened_cursor - edit.opened_scroll_end + 1;
    edit.opened_scroll_start += dlines;
  }
}

__EditCmd( CmdSwitchopenedCursorU )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  auto nlines = misc ? misc : 1;
  edit.opened_cursor -= MIN( nlines, edit.opened_cursor );
  MakeOpenedCursorVisible( edit );
}

__EditCmd( CmdSwitchopenedCursorD )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  auto nlines = misc ? misc : 1;
  edit.opened_cursor += nlines;
  edit.opened_cursor = MIN( edit.opened_cursor, MAX( edit.search_matches.len, 1 ) - 1 );
  MakeOpenedCursorVisible( edit );
}

__EditCmd( CmdSwitchopenedScrollU )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  auto nlines = misc ? misc : 1;
  edit.opened_scroll_start -= MIN( nlines, edit.opened_scroll_start );
}

__EditCmd( CmdSwitchopenedScrollD )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  auto nlines = misc ? misc : 1;
  edit.opened_scroll_start += nlines;
  edit.opened_scroll_start = MIN( edit.opened_scroll_start, MAX( edit.search_matches.len, 1 ) - 1 );
}

__EditCmd( CmdSwitchopenedMakeCursorPresent )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  edit.opened_cursor = edit.opened_scroll_start;
  CmdSwitchopenedCursorD( edit, edit.nlines_screen / 2 );
}

__EditCmd( CmdUpdateSearchMatches )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  edit.search_matches.len = 0;
  if( !TxtLen( edit.opened_search ) ) {
    ForList( elem, edit.openedmru ) {
      auto open = elem->value;
      *AddBack( edit.search_matches ) = open;
    }
  } else {
    auto key = AllocContents( edit.opened_search.buf );
    ForList( elem, edit.openedmru ) {
      auto open = elem->value;
      idx_t pos;
      if( CsIdxScanR( &pos, ML( open->txt.filename ), ML( key ), 0, 0 ) ) {
        *AddBack( edit.search_matches ) = open;
      }
    }
    Free( key );
  }
  edit.opened_scroll_start = MIN( edit.opened_scroll_start, MAX( edit.search_matches.len, 1 ) - 1 );
  edit.opened_cursor = MIN( edit.opened_cursor, MAX( edit.search_matches.len, 1 ) - 1 );
  MakeOpenedCursorVisible( edit );
}

__EditCmd( CmdMode_switchopened_from_editfile )
{
  AssertCrash( edit.mode == editmode_t::editfile );
  edit.mode = editmode_t::switchopened;
  CmdUpdateSearchMatches( edit );
  edit.opened_scroll_start = 0;
  edit.opened_cursor = 0;

  // since the search_matches list is populated from openedmru, it's in mru order.
  // to allow quick change to the second mru entry ( the first is already the active txt ),
  // put the cursor over the second mru entry.
  if( edit.search_matches.len > 1 ) {
    edit.opened_cursor += 1;
  }
}

__EditCmd( CmdMode_editfile_from_switchopened )
{
  AssertCrash( edit.mode == editmode_t::switchopened );
  edit.mode = editmode_t::editfile;
  edit.search_matches.len = 0;
  CmdSelectAll( edit.opened_search );
  CmdRemChL( edit.opened_search );
}

Inl fsobj_t*
EditGetOpenedSelection( edit_t& edit )
{
  AssertCrash( edit.opened_cursor < edit.search_matches.len );
  auto open = edit.search_matches.mem[edit.opened_cursor];
  return &open->txt.filename;
}

__EditCmd( CmdSwitchopenedChoose )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::switchopened );
  auto obj = EditGetOpenedSelection( edit );
  if( obj ) {
    auto file = FileOpen( obj->mem, obj->len, fileopen_t::only_existing, fileop_t::R, fileop_t::R );
    if( file.loaded ) {
      EditOpenAndSetActiveTxt( edit, file );
      CmdMode_editfile_from_switchopened( edit );
    }
    FileFree( file );
  }
}

__EditCmd( CmdSwitchopenedCloseFile )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::switchopened );
  AssertCrash( edit.openedmru.len == edit.opened.len );
  auto file = EditGetOpenedSelection( edit );
  AssertCrash( file );
  auto open = EditGetOpenedFile( edit, file->mem, file->len );
  AssertCrash( open );
  Save( edit, open );
  if( !open->unsaved ) {
    bool mruremoved = 0;
    ForList( elem, edit.openedmru ) {
      if( elem->value == open ) {
        Rem( edit.openedmru, elem );
        mruremoved = 1;
        break;
      }
    }
    AssertCrash( mruremoved );
    Kill( open->txt );
    bool removed = 0;
    ForList( elem, edit.opened ) {
      if( &elem->value == open ) {
        Rem( edit.opened, elem );
        removed = 1;
        break;
      }
    }
    AssertCrash( removed );
    For( i, 0, 2 ) {
      if( edit.active[i] == open ) {
        auto elem = edit.openedmru.first;
        edit.active[i] = elem  ?  elem->value  :  0;
      }
    }
    if( !edit.opened.len ) {
      CmdMode_fileopener_from_switchopened( edit );
    } else {
      CmdUpdateSearchMatches( edit );
    }
  }
  AssertCrash( edit.openedmru.len == edit.opened.len );
}






// =================================================================================
// EXTERNAL CHANGES

Inl void
CmdCheckForExternalChanges( edit_t& edit )
{
  ProfFunc();
  ForList( elem, edit.opened ) {
    auto open = &elem->value;
    edit.file_externalmerge = FileOpen( ML( open->txt.filename ), fileopen_t::only_existing, fileop_t::R, fileop_t::R );
    if( edit.file_externalmerge.loaded  &&  open->time_lastwrite != edit.file_externalmerge.time_lastwrite ) {
      // some other program updated the file.
      if( open->unsaved ) {
        // we have unsaved changes; need to decide if we want to stomp the external changes, or discard ours.
        if( edit.mode != editmode_t::externalmerge ) {
          edit.mode_before_externalmerge = edit.mode;
          edit.horzview_before_externalmerge = edit.horzview;
          edit.mode = editmode_t::externalmerge;
          edit.horzview = 0;
        }
        edit.active_externalmerge = open;
        return;

      } else {
        // no unsaved changes, so just silently reload the file.
        cs_undo_absolute_t cs;
        CsUndoAbsoluteFromTxt( open->txt, &cs );
        Kill( open->txt );
        Init( open->txt );
        TxtLoad( open->txt, edit.file_externalmerge );
        ApplyCsUndoAbsolute( open->txt, cs );
        open->unsaved = 0;
        open->time_lastwrite = edit.file_externalmerge.time_lastwrite;
      }
    }
    FileFree( edit.file_externalmerge );
  }

  // all external changes handled.
  if( edit.mode == editmode_t::externalmerge ) {
    edit.mode = edit.mode_before_externalmerge;
    edit.horzview = edit.horzview_before_externalmerge;
  }
  return;
}

__EditCmd( CmdExternalmergeDiscardLocalChanges )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::externalmerge );
  auto open = edit.active_externalmerge;
  AssertCrash( open->unsaved );
  AssertCrash( edit.file_externalmerge.loaded );

  // discard our local changes.
  cs_undo_absolute_t cs;
  CsUndoAbsoluteFromTxt( open->txt, &cs );
  Kill( open->txt );
  Init( open->txt );
  TxtLoad( open->txt, edit.file_externalmerge );
  ApplyCsUndoAbsolute( open->txt, cs );
  open->unsaved = 0;
  open->time_lastwrite = edit.file_externalmerge.time_lastwrite;

  FileFree( edit.file_externalmerge );

  // continue checking other opened files.
  CmdCheckForExternalChanges( edit );
}

__EditCmd( CmdExternalmergeKeepLocalChanges )
{
  ProfFunc();
  AssertCrash( edit.mode == editmode_t::externalmerge );
  auto open = edit.active_externalmerge;
  AssertCrash( open->unsaved );
  AssertCrash( edit.file_externalmerge.loaded );

  // discard the external changes.
  FileFree( edit.file_externalmerge );
  Save( edit, open );
  AssertCrash( !open->unsaved );

  // continue checking other opened files.
  CmdCheckForExternalChanges( edit );
}


Inl void
EditWindowEvent(
  edit_t& edit,
  enum_t type,
  vec2<u32> dim,
  u32 dpi,
  bool focused,
  bool& target_valid
  )
{
  if( type & glwwindowevent_resize ) {
  }
  if( type & glwwindowevent_dpichange ) {
  }
  if( type & glwwindowevent_focuschange ) {
    if( focused ) {
      CmdCheckForExternalChanges( edit );
      target_valid = 0;
    }
  }
}




// =================================================================================
// RENDERING

Enumc( editlayer_t )
{
  bkgd,
  sel,
  txt,

  COUNT
};


void
_RenderTxt(
  txt_t& txt,
  bool& target_valid,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  f64 timestep_realtime,
  f64 timestep_fixed,
  bool draw_cursor,
  bool draw_cursorline,
  bool draw_cursorwordmatch,
  bool allow_scrollbar
  )
{
  TxtUpdateScrolling(
    txt,
    font,
    origin,
    dim,
    timestep_realtime,
    timestep_fixed
    );
  TxtLayout(
    txt,
    font
    );
  TxtRender(
    txt,
    target_valid,
    stream,
    font,
    origin,
    dim,
    zrange,
    draw_cursor,
    draw_cursorline,
    draw_cursorwordmatch,
    allow_scrollbar
    );
}


void
_RenderStatusBar(
  edittxtopen_t& open,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32>& origin,
  vec2<f32>& dim,
  vec2<f32> zrange
  )
{
  auto rgba_text = GetPropFromDb( vec4<f32>, rgba_text );
  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );
  auto line_h = FontLineH( font );

  // status bar, so we know what file is open and if it's unsaved.

  u8 diff_count[128];
  idx_t diff_count_len = 0;
  CsFromIntegerU( AL( diff_count ), &diff_count_len, open.txt.buf.diffs.len, 1 );
  DrawString(
    stream,
    font,
    origin,
    GetZ( zrange, editlayer_t::txt ),
    origin,
    dim,
    rgba_text,
    spaces_per_tab,
    diff_count, diff_count_len
    );

  auto filename_w = LayoutString( font, spaces_per_tab, ML( open.txt.filename ) );
  DrawString(
    stream,
    font,
    origin + _vec2<f32>( 0.5f * ( dim.x - filename_w ), 0 ),
    GetZ( zrange, editlayer_t::txt ),
    origin,
    dim,
    rgba_text,
    spaces_per_tab,
    ML( open.txt.filename )
    );

  if( open.unsaved ) {
    static const auto unsaved_label = Str( "-- UNSAVED --" );
    static const idx_t unsaved_label_len = CsLen( unsaved_label );
    auto label_w = LayoutString( font, spaces_per_tab, unsaved_label, unsaved_label_len );
    DrawString(
      stream,
      font,
      origin + _vec2<f32>( dim.x - label_w, 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      unsaved_label, unsaved_label_len
      );
  }
  origin.y += line_h;
  dim.y -= line_h;
}


void
_RenderBothSides(
  edit_t& edit,
  bool& target_valid,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  f64 timestep_realtime,
  f64 timestep_fixed
  )
{
  if( edit.horzview ) {
    auto open_r = edit.active[0];
    auto open_l = edit.active[1];

    if( open_l ) {
      auto origin_l = origin;
      auto dim_l = _vec2( dim.x / 2, dim.y );

      _RenderStatusBar(
        *open_l,
        stream,
        font,
        origin_l,
        dim_l,
        zrange
        );
      _RenderTxt(
        open_l->txt,
        target_valid,
        stream,
        font,
        origin_l,
        dim_l,
        zrange,
        timestep_realtime,
        timestep_fixed,
        edit.horzfocus_l,
        edit.horzfocus_l,
        edit.horzfocus_l,
        1
        );
    }
    if( open_r ) {
      auto origin_r = _vec2( origin.x + dim.x / 2, origin.y );
      auto dim_r =  _vec2( dim.x / 2, dim.y );

      _RenderStatusBar(
        *open_r,
        stream,
        font,
        origin_r,
        dim_r,
        zrange
        );
      _RenderTxt(
        open_r->txt,
        target_valid,
        stream,
        font,
        origin_r,
        dim_r,
        zrange,
        timestep_realtime,
        timestep_fixed,
        !edit.horzfocus_l,
        !edit.horzfocus_l,
        !edit.horzfocus_l,
        1
        );
    }
  } else {
    auto open = edit.active[edit.horzfocus_l];
    if( open ) {
      _RenderStatusBar(
        *open,
        stream,
        font,
        origin,
        dim,
        zrange
        );
      _RenderTxt(
        open->txt,
        target_valid,
        stream,
        font,
        origin,
        dim,
        zrange,
        timestep_realtime,
        timestep_fixed,
        1,
        1,
        1,
        1
        );
    }
  }
}

Enumc( dirlayer_t )
{
  bkgd,
  sel,
  txt,

  COUNT
};





void
FileopenerRender(
  edit_t& edit,
  bool& target_valid,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  f64 timestep_realtime,
  f64 timestep_fixed,
  bool draw_cursor,
  bool draw_cursorline,
  bool draw_cursorwordmatch,
  bool allow_scrollbar
  )
{
  ProfFunc();

  fileopener_t& fo = edit.fileopener;

  AssertCrash( edit.mode == editmode_t::fileopener  ||  edit.mode == editmode_t::fileopener_renaming );

  auto rgba_text = GetPropFromDb( vec4<f32>, rgba_text );
  auto rgba_cursor_bkgd = GetPropFromDb( vec4<f32>, rgba_cursor_bkgd );
  auto rgba_cursor_size_text = GetPropFromDb( vec4<f32>, rgba_cursor_size_text );
  auto rgba_size_text = GetPropFromDb( vec4<f32>, rgba_size_text );
  auto rgba_cursor_text = GetPropFromDb( vec4<f32>, rgba_cursor_text );
  auto px_column_spacing = GetPropFromDb( f32, f32_px_column_spacing );
  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );

  auto line_h = FontLineH( font );

  static const auto label_dir = SliceFromCStr( "Dir: " );
  auto label_dir_w = LayoutString( font, spaces_per_tab, ML( label_dir ) );

  static const auto label_ignore_ext = SliceFromCStr( "Ext Ignore: " );
  auto label_ignore_ext_w = LayoutString( font, spaces_per_tab, ML( label_ignore_ext ) );

  static const auto label_ignore_substring = SliceFromCStr( "Substring Ignore: " );
  auto label_ignore_substring_w = LayoutString( font, spaces_per_tab, ML( label_ignore_substring ) );

  static const auto label_find = SliceFromCStr( "Find: " );
  auto label_find_w = LayoutString( font, spaces_per_tab, ML( label_find ) );

  auto maxlabelw = MAX4( label_dir_w, label_ignore_ext_w, label_ignore_substring_w, label_find_w );

  { // draw cwd
    DrawString(
      stream,
      font,
      origin + _vec2<f32>( maxlabelw - label_dir_w, 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      ML( label_dir )
      );

    TxtLayoutSingleLineSubset(
      fo.cwd,
      GetBOF( fo.cwd.buf ),
      TxtLen( fo.cwd ),
      font
      );
    TxtRenderSingleLineSubset(
      fo.cwd,
      stream,
      font,
      origin + _vec2<f32>( maxlabelw, 0 ),
      dim - _vec2<f32>( maxlabelw, 0 ),
      ZRange( zrange, editlayer_t::txt ),
      0,
      ( fo.focus == fileopenerfocus_t::dir ),
      ( fo.focus == fileopenerfocus_t::dir )
      );

    origin.y += line_h;
    dim.y -= line_h;
  }

  { // ignored_filetypes bar
    DrawString(
      stream,
      font,
      origin + _vec2<f32>( maxlabelw - label_ignore_ext_w, 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      ML( label_ignore_ext )
      );

    TxtLayoutSingleLineSubset(
      fo.ignored_filetypes,
      GetBOF( fo.ignored_filetypes.buf ),
      TxtLen( fo.ignored_filetypes ),
      font
      );
    TxtRenderSingleLineSubset(
      fo.ignored_filetypes,
      stream,
      font,
      origin + _vec2<f32>( maxlabelw, 0 ),
      dim - _vec2<f32>( maxlabelw, 0 ),
      ZRange( zrange, editlayer_t::txt ),
      0,
      ( fo.focus == fileopenerfocus_t::ignored_filetypes ),
      ( fo.focus == fileopenerfocus_t::ignored_filetypes )
      );

    origin.y += line_h;
    dim.y -= line_h;
  }

  { // ignored_substrings bar
    DrawString(
      stream,
      font,
      origin + _vec2<f32>( maxlabelw - label_ignore_substring_w, 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      ML( label_ignore_substring )
      );

    TxtLayoutSingleLineSubset(
      fo.ignored_substrings,
      GetBOF( fo.ignored_substrings.buf ),
      TxtLen( fo.ignored_substrings ),
      font
      );
    TxtRenderSingleLineSubset(
      fo.ignored_substrings,
      stream,
      font,
      origin + _vec2<f32>( maxlabelw, 0 ),
      dim - _vec2<f32>( maxlabelw, 0 ),
      ZRange( zrange, editlayer_t::txt ),
      0,
      ( fo.focus == fileopenerfocus_t::ignored_substrings ),
      ( fo.focus == fileopenerfocus_t::ignored_substrings )
      );

    origin.y += line_h;
    dim.y -= line_h;
  }

  { // search bar
    DrawString(
      stream,
      font,
      origin + _vec2<f32>( maxlabelw - label_find_w, 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      ML( label_find )
      );

    TxtLayoutSingleLineSubset(
      fo.query,
      GetBOF( fo.query.buf ),
      TxtLen( fo.query ),
      font
      );
    TxtRenderSingleLineSubset(
      fo.query,
      stream,
      font,
      origin + _vec2<f32>( maxlabelw, 0 ),
      dim - _vec2<f32>( maxlabelw, 0 ),
      ZRange( zrange, editlayer_t::txt ),
      0,
      ( fo.focus == fileopenerfocus_t::query ),
      ( fo.focus == fileopenerfocus_t::query )
      );

    origin.y += line_h;
    dim.y -= line_h;
  }

  // result count
  if( fo.ncontexts_active ) {
    static const auto label = Str( "scanning directory..." );
    static const idx_t label_len = CsLen( label );
    auto label_w = LayoutString( font, spaces_per_tab, label, label_len );

    DrawString(
      stream,
      font,
      origin + _vec2<f32>( 0.5f * ( dim.x - label_w ), 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      label, label_len
      );

    origin.y += line_h;
    dim.y -= line_h;

  } else {
    static const auto label = Str( " results" );
    static const idx_t label_len = CsLen( label );
    auto label_w = LayoutString( font, spaces_per_tab, label, label_len );

    u8 count[128];
    idx_t count_len = 0;
    CsFromIntegerU( count, _countof( count ), &count_len, fo.matches.totallen, 1 );
    auto count_w = LayoutString( font, spaces_per_tab, count, count_len );

    DrawString(
      stream,
      font,
      origin + _vec2<f32>( 0.5f * ( dim.x - count_w - label_w ), 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      count, count_len
      );

    DrawString(
      stream,
      font,
      origin + _vec2<f32>( 0.5f * ( dim.x - count_w - label_w ) + count_w, 0 ),
      GetZ( zrange, editlayer_t::txt ),
      origin,
      dim,
      rgba_text,
      spaces_per_tab,
      label, label_len
      );

    origin.y += line_h;
    dim.y -= line_h;
  }

  auto nlines_screen_floored = Cast( idx_t, dim.y / line_h );
  fo.pageupdn_distance = MAX( 1, nlines_screen_floored / 2 );
  fo.scroll_end = fo.scroll_start + MIN( nlines_screen_floored, fo.matches.totallen );

  AssertCrash( fo.scroll_start <= fo.matches.totallen );
  auto nlines_render = MIN( 1 + nlines_screen_floored, fo.matches.totallen - fo.scroll_start );

  { // draw bkgd
    RenderQuad(
      stream,
      GetPropFromDb( vec4<f32>, rgba_text_bkgd ),
      origin,
      origin + dim,
      origin, dim,
      GetZ( zrange, dirlayer_t::bkgd )
      );
  }

  { // cursor
    if( fo.scroll_start <= fo.cursor  &&  fo.cursor < fo.scroll_end ) {
      auto p0 = _vec2<f32>( origin.x, line_h * ( fo.cursor - fo.scroll_start ) );
      auto p1 = p0 + _vec2( dim.x, line_h );
      RenderQuad(
        stream,
        rgba_cursor_bkgd,
        origin + p0,
        origin + p1,
        origin, dim,
        GetZ( zrange, editlayer_t::sel )
        );
    }
  }


  { // elem sizes
    f32 max_sizetxt_w = 0;

    if( fo.matches.totallen ) {
      auto pa_iter = MakeIteratorAtLinearIndex( fo.matches, fo.scroll_start );
      For( i, 0, nlines_render ) {
        auto elem = *GetElemAtIterator( fo.matches, pa_iter );
        pa_iter = IteratorMoveR( fo.matches, pa_iter );

        auto sizetxt_w = LayoutString( font, spaces_per_tab, ML( elem->sizetxt ) );
        max_sizetxt_w = MAX( max_sizetxt_w, sizetxt_w );
      }
    }
    if( fo.matches.totallen ) {
      auto pa_iter = MakeIteratorAtLinearIndex( fo.matches, fo.scroll_start );
      For( i, 0, nlines_render ) {
        auto elem = *GetElemAtIterator( fo.matches, pa_iter );
        pa_iter = IteratorMoveR( fo.matches, pa_iter );

        auto sizetxt_w = LayoutString( font, spaces_per_tab, ML( elem->sizetxt ) ); // PERF: cache this?
        auto elem_origin = origin + _vec2( max_sizetxt_w - sizetxt_w, line_h * i );
        auto elem_dim = _vec2( sizetxt_w, line_h );
        DrawString(
          stream,
          font,
          elem_origin,
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          ( fo.cursor == fo.scroll_start + i )  ?  rgba_cursor_size_text  :  rgba_size_text,
          spaces_per_tab,
          ML( elem->sizetxt )
          );
      }
    }
    origin.x += max_sizetxt_w + px_column_spacing;
    dim.x -= max_sizetxt_w + px_column_spacing;
  }

  { // elem readonlys
    static const auto readonly_label = Str( "readonly" );
    static const idx_t readonly_label_len = CsLen( readonly_label );
    auto readonly_w = LayoutString( font, spaces_per_tab, readonly_label, readonly_label_len );
    bool readonly_visible = 0;

    if( fo.matches.totallen ) {
      auto pa_iter = MakeIteratorAtLinearIndex( fo.matches, fo.scroll_start );
      For( i, 0, nlines_render ) {
        auto elem = *GetElemAtIterator( fo.matches, pa_iter );
        pa_iter = IteratorMoveR( fo.matches, pa_iter );

        auto elem_origin = origin + _vec2<f32>( 0, line_h * i );
        auto elem_dim = _vec2( dim.x, line_h );
        if( elem->readonly ) {
          readonly_visible = 1;
          DrawString(
            stream,
            font,
            elem_origin,
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            ( fo.cursor == fo.scroll_start + i )  ?  rgba_cursor_size_text  :  rgba_size_text,
            spaces_per_tab,
            readonly_label, readonly_label_len
            );
        }
      }
    }
    if( readonly_visible ) {
      origin.x += readonly_w + px_column_spacing;
      dim.x -= readonly_w + px_column_spacing;
    }
  }

  { // elem names
    if( fo.matches.totallen ) {
      auto pa_iter = MakeIteratorAtLinearIndex( fo.matches, fo.scroll_start );
      For( i, 0, nlines_render ) {
        auto elem = *GetElemAtIterator( fo.matches, pa_iter );
        pa_iter = IteratorMoveR( fo.matches, pa_iter );

        auto elem_origin = origin + _vec2<f32>( 0, line_h * i );
        auto elem_dim = _vec2( dim.x, line_h );
        if( edit.mode == editmode_t::fileopener_renaming  &&
            fo.renaming_row == fo.scroll_start + i )
        {
          TxtUpdateScrolling(
            fo.renaming_txt,
            font,
            elem_origin,
            elem_dim,
            timestep_realtime,
            timestep_fixed
            );
          TxtLayout(
            fo.renaming_txt,
            font
            );
          TxtRender(
            fo.renaming_txt,
            target_valid,
            stream,
            font,
            elem_origin,
            elem_dim,
            ZRange( zrange, editlayer_t::txt ),
            1,
            0,
            0,
            0
            );
        } else {
          DrawString(
            stream,
            font,
            elem_origin,
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            ( fo.cursor == fo.scroll_start + i )  ?  rgba_cursor_text  :  rgba_text,
            spaces_per_tab,
            ML( elem->name )
            );
        }
      }
    }
  }
}


void
EditRender(
  edit_t& edit,
  bool& target_valid,
  array_t<f32>& stream,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<f32> zrange,
  f64 timestep_realtime,
  f64 timestep_fixed
  )
{
  ProfFunc();

  auto rgba_text = GetPropFromDb( vec4<f32>, rgba_text );
  auto rgba_cursor_bkgd = GetPropFromDb( vec4<f32>, rgba_cursor_bkgd );
  auto rgba_cursor_size_text = GetPropFromDb( vec4<f32>, rgba_cursor_size_text );
  auto rgba_wordmatch_bkgd = GetPropFromDb( vec4<f32>, rgba_wordmatch_bkgd );
  auto rgba_size_text = GetPropFromDb( vec4<f32>, rgba_size_text );
  auto rgba_cursor_text = GetPropFromDb( vec4<f32>, rgba_cursor_text );
  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );

  auto line_h = FontLineH( font );

  switch( edit.mode ) {

    case editmode_t::editfile: {
      _RenderBothSides(
        edit,
        target_valid,
        stream,
        font,
        origin,
        dim,
        zrange,
        timestep_realtime,
        timestep_fixed
        );
    } break;

    case editmode_t::switchopened: {
      static const auto header = Str( "SWITCH TO FILE ( MRU ):" );
      static const auto header_len = CsLen( header );
      auto header_w = LayoutString( font, spaces_per_tab, header, header_len );
      DrawString(
        stream,
        font,
        origin + _vec2<f32>( 0.5f * ( dim.x - header_w ), 0 ),
        GetZ( zrange, editlayer_t::txt ),
        origin, dim,
        rgba_text,
        spaces_per_tab,
        header, header_len
        );
      origin.y += line_h;
      dim.y -= line_h;

      edit.opened_scroll_start = MIN( edit.opened_scroll_start, MAX( edit.search_matches.len, 1 ) - 1 );
      edit.opened_cursor = MIN( edit.opened_cursor, MAX( edit.search_matches.len, 1 ) - 1 );

      auto nlines_screen_max = Cast( idx_t, dim.y / line_h );
      if( nlines_screen_max ) {
        nlines_screen_max -= 1; // for search bar.
      }
      edit.nlines_screen = MIN( nlines_screen_max, edit.search_matches.len - edit.opened_scroll_start );
      edit.opened_scroll_end = edit.opened_scroll_start + edit.nlines_screen;

      // render search bar.
      static const auto search = Str( "Search: " );
      static const auto search_len = CsLen( search );
      auto search_w = LayoutString( font, spaces_per_tab, search, search_len );
      DrawString(
        stream,
        font,
        origin,
        GetZ( zrange, editlayer_t::txt ),
        origin, dim,
        rgba_text,
        spaces_per_tab,
        search, search_len
        );

      auto searchtxt_origin = origin + _vec2<f32>( search_w, 0 );
      auto searchtxt_dim = dim - _vec2<f32>( search_w, 0 );

      TxtLayoutSingleLineSubset(
        edit.opened_search,
        GetBOF( edit.opened_search.buf ),
        TxtLen( edit.opened_search ),
        font
        );
      TxtRenderSingleLineSubset(
        edit.opened_search,
        stream,
        font,
        searchtxt_origin, searchtxt_dim,
        ZRange( zrange, editlayer_t::txt ),
        0,
        1,
        1
        );

      origin.y += line_h;
      dim.y -= line_h;

      // render current cursor.
      if( edit.opened_scroll_start <= edit.opened_cursor  &&  edit.opened_cursor < edit.opened_scroll_end ) {
        auto p0 = _vec2<f32>( origin.x, line_h * ( edit.opened_cursor - edit.opened_scroll_start ) );
        auto p1 = p0 + _vec2( dim.x, line_h );
        RenderQuad(
          stream,
          rgba_cursor_bkgd,
          origin + p0,
          origin + p1,
          origin, dim,
          GetZ( zrange, editlayer_t::bkgd )
          );
      }
      static const auto unsaved = Str( " unsaved " );
      static const auto unsaved_len = CsLen( unsaved );
      auto unsaved_w = LayoutString( font, spaces_per_tab, unsaved, unsaved_len );
      For( i, 0, edit.nlines_screen ) {
        idx_t rowidx = ( i + edit.opened_scroll_start );
        if( rowidx >= edit.search_matches.len ) {
          break;
        }

        auto open = edit.search_matches.mem[rowidx];
        auto row_origin = origin + _vec2( 0.0f, line_h * i );
        auto row_dim = _vec2( dim.x, line_h );

        if( open->unsaved ) {
          DrawString(
            stream,
            font,
            row_origin,
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            rgba_text,
            spaces_per_tab,
            unsaved, unsaved_len
            );
        }

        row_origin.x += unsaved_w;
        row_dim.x -= unsaved_w;

        DrawString(
          stream,
          font,
          row_origin,
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( open->txt.filename )
          );
      }
    } break;

    case editmode_t::fileopener_renaming: __fallthrough;
    case editmode_t::fileopener: {
      FileopenerRender(
        edit,
        target_valid,
        stream,
        font,
        origin,
        dim,
        zrange,
        timestep_realtime,
        timestep_fixed,
        1,
        1,
        1,
        1
      );
    } break;

    case editmode_t::editfile_findrepl: {

      { // case sens
        auto bind0 = GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_toggle_case_sensitive );
        auto key0 = KeyStringFromGlw( bind0.key );
        AssertCrash( !key0.mem[key0.len] ); // cstr generated by compiler.
        auto label = AllocString( "Case sensitive: %u -- Press [ %s ] to toggle.", edit.findrepl.case_sens, key0.mem );
        auto label_w = LayoutString( font, spaces_per_tab, ML( label ) );
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( dim.x - label_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label )
          );
        Free( label );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // word boundary
        auto bind0 = GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_toggle_word_boundary );
        auto key0 = KeyStringFromGlw( bind0.key );
        AssertCrash( !key0.mem[key0.len] ); // cstr generated by compiler.
        auto label = AllocString( "Whole word: %u -- Press [ %s ] to toggle.", edit.findrepl.word_boundary, key0.mem );
        auto label_w = LayoutString( font, spaces_per_tab, ML( label ) );
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( dim.x - label_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label )
          );
        Free( label );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // find
        static const auto find_label = Str( "Find: " );
        static const idx_t find_label_len = CsLen( find_label );
        auto findlabel_w = LayoutString( font, spaces_per_tab, find_label, find_label_len );
        DrawString(
          stream,
          font,
          origin,
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          find_label, find_label_len
          );

        auto origin_find = origin + _vec2( findlabel_w, 0.0f );
        auto dim_find = dim - _vec2( findlabel_w, 0.0f );

        auto show_cursor = edit.findrepl.focus_find;

        TxtLayoutSingleLineSubset(
          edit.findrepl.find,
          GetBOF( edit.findrepl.find.buf ),
          TxtLen( edit.findrepl.find ),
          font
          );
        TxtRenderSingleLineSubset(
          edit.findrepl.find,
          stream,
          font,
          origin_find,
          dim_find,
          zrange,
          0,
          show_cursor,
          1
          );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // replace
        static const auto replace_label = Str( "Repl: " );
        static const idx_t replace_label_len = CsLen( replace_label );
        auto replacelabel_w = LayoutString( font, spaces_per_tab, replace_label, replace_label_len );
        DrawString(
          stream,
          font,
          origin,
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          replace_label,
          replace_label_len
          );

        auto origin_replace = origin + _vec2( replacelabel_w, 0.0f );
        auto dim_replace = dim - _vec2( replacelabel_w, 0.0f );

        auto show_cursor = !edit.findrepl.focus_find;

        TxtLayoutSingleLineSubset(
          edit.findrepl.replace,
          GetBOF( edit.findrepl.replace.buf ),
          TxtLen( edit.findrepl.replace ),
          font
          );
        TxtRenderSingleLineSubset(
          edit.findrepl.replace,
          stream,
          font,
          origin_replace,
          dim_replace,
          zrange,
          0,
          show_cursor,
          1
          );

        origin.y += line_h;
        dim.y -= line_h;
      }

      _RenderBothSides(
        edit,
        target_valid,
        stream,
        font,
        origin,
        dim,
        zrange,
        timestep_realtime,
        timestep_fixed
        );
    } break;

    case editmode_t::editfile_gotoline: {

      static const auto gotoline_label = Str( "Go to line: " );
      static const idx_t gotoline_label_len = CsLen( gotoline_label );
      auto gotolinelabel_w = LayoutString( font, spaces_per_tab, gotoline_label, gotoline_label_len );
      DrawString(
        stream,
        font,
        origin,
        GetZ( zrange, editlayer_t::txt ),
        origin,
        dim,
        rgba_text,
        spaces_per_tab,
        gotoline_label,
        gotoline_label_len
        );

      auto origin_gotoline = origin + _vec2( gotolinelabel_w, 0.0f );
      auto dim_gotoline = dim - _vec2( gotolinelabel_w, 0.0f );

      TxtLayoutSingleLineSubset(
        edit.gotoline,
        GetBOF( edit.gotoline.buf ),
        TxtLen( edit.gotoline ),
        font
        );
      TxtRenderSingleLineSubset(
        edit.gotoline,
        stream,
        font,
        origin_gotoline,
        dim_gotoline,
        zrange,
        1,
        1,
        1
        );

      origin.y += line_h;
      dim.y -= line_h;

      _RenderBothSides(
        edit,
        target_valid,
        stream,
        font,
        origin,
        dim,
        zrange,
        timestep_realtime,
        timestep_fixed
        );
    } break;

    case editmode_t::externalmerge: {
      auto open = edit.active_externalmerge;

      _RenderStatusBar(
        *open,
        stream,
        font,
        origin,
        dim,
        zrange
        );

      static auto label0 = SliceFromCStr( "--- External change detected! ---" );
      auto bind0 = GetPropFromDb( glwkeybind_t, keybind_externalmerge_keep_local_changes );
      auto bind1 = GetPropFromDb( glwkeybind_t, keybind_externalmerge_discard_local_changes );
      auto key0 = KeyStringFromGlw( bind0.key );
      auto key1 = KeyStringFromGlw( bind1.key );
      AssertCrash( !key0.mem[key0.len] ); // cstr generated by compiler.
      AssertCrash( !key1.mem[key1.len] ); // cstr generated by compiler.
      auto label1 = AllocString( "Press [ %s ] to keep your changes.", key0.mem );
      auto label2 = AllocString( "Press [ %s ] to discard your changes.", key1.mem );
      DrawString(
        stream,
        font,
        origin + _vec2( 0.5f * ( dim.x - LayoutString( font, spaces_per_tab, ML( label0 ) ) ), 0.0f ),
        GetZ( zrange, editlayer_t::txt ),
        origin,
        dim,
        rgba_text,
        spaces_per_tab,
        ML( label0 )
        );

      origin.y += line_h;
      dim.y -= line_h;

      DrawString(
        stream,
        font,
        origin,
        GetZ( zrange, editlayer_t::txt ),
        origin,
        dim,
        rgba_text,
        spaces_per_tab,
        ML( label1 )
        );
      Free( label1 );

      origin.y += line_h;
      dim.y -= line_h;

      DrawString(
        stream,
        font,
        origin,
        GetZ( zrange, editlayer_t::txt ),
        origin,
        dim,
        rgba_text,
        spaces_per_tab,
        ML( label2 )
        );
      Free( label2 );

      origin.y += line_h;
      dim.y -= line_h;

      _RenderTxt(
        open->txt,
        target_valid,
        stream,
        font,
        origin,
        dim,
        zrange,
        timestep_realtime,
        timestep_fixed,
        1,
        1,
        1,
        1
        );
    } break;

    case editmode_t::findinfiles: {
      auto& fif = edit.findinfiles;

      { // case sens
        auto bind0 = GetPropFromDb( glwkeybind_t, keybind_findinfiles_toggle_case_sensitive );
        auto key0 = KeyStringFromGlw( bind0.key );
        AssertCrash( !key0.mem[key0.len] ); // cstr generated by compiler.
        auto label = AllocString( "Case sensitive: %u -- Press [ %s ] to toggle.", fif.case_sens, key0.mem );
        auto label_w = LayoutString( font, spaces_per_tab, ML( label ) );
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( dim.x - label_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label )
          );
        Free( label );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // word boundary
        auto bind0 = GetPropFromDb( glwkeybind_t, keybind_findinfiles_toggle_word_boundary );
        auto key0 = KeyStringFromGlw( bind0.key );
        AssertCrash( !key0.mem[key0.len] ); // cstr generated by compiler.
        auto label = AllocString( "Whole word: %u -- Press [ %s ] to toggle.", fif.word_boundary, key0.mem );
        auto label_w = LayoutString( font, spaces_per_tab, ML( label ) );
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( dim.x - label_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label )
          );
        Free( label );

        origin.y += line_h;
        dim.y -= line_h;
      }

      static const auto label_dir = SliceFromCStr( "Dir: " );
      auto label_dir_w = LayoutString( font, spaces_per_tab, ML( label_dir ) );

      static const auto label_ignore = SliceFromCStr( "Ignore: " );
      auto label_ignore_w = LayoutString( font, spaces_per_tab, ML( label_ignore ) );

      static const auto label_find = SliceFromCStr( "Find: " );
      auto label_find_w = LayoutString( font, spaces_per_tab, ML( label_find ) );

      static const auto label_repl = SliceFromCStr( "Repl: " );
      auto label_repl_w = LayoutString( font, spaces_per_tab, ML( label_repl ) );

      auto maxlabelw = MAX4( label_dir_w, label_ignore_w, label_find_w, label_repl_w );

      { // dir bar
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( maxlabelw - label_dir_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label_dir )
          );

        TxtLayoutSingleLineSubset(
          fif.dir,
          GetBOF( fif.dir.buf ),
          TxtLen( fif.dir ),
          font
          );
        TxtRenderSingleLineSubset(
          fif.dir,
          stream,
          font,
          origin + _vec2<f32>( maxlabelw, 0 ),
          dim - _vec2<f32>( maxlabelw, 0 ),
          ZRange( zrange, editlayer_t::txt ),
          0,
          ( fif.focus == findinfilesfocus_t::dir ),
          ( fif.focus == findinfilesfocus_t::dir )
          );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // ignored_filetypes bar
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( maxlabelw - label_ignore_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label_ignore )
          );

        TxtLayoutSingleLineSubset(
          fif.ignored_filetypes,
          GetBOF( fif.ignored_filetypes.buf ),
          TxtLen( fif.ignored_filetypes ),
          font
          );
        TxtRenderSingleLineSubset(
          fif.ignored_filetypes,
          stream,
          font,
          origin + _vec2<f32>( maxlabelw, 0 ),
          dim - _vec2<f32>( maxlabelw, 0 ),
          ZRange( zrange, editlayer_t::txt ),
          0,
          ( fif.focus == findinfilesfocus_t::ignored_filetypes ),
          ( fif.focus == findinfilesfocus_t::ignored_filetypes )
          );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // search bar
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( maxlabelw - label_find_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label_find )
          );

        TxtLayoutSingleLineSubset(
          fif.query,
          GetBOF( fif.query.buf ),
          TxtLen( fif.query ),
          font
          );
        TxtRenderSingleLineSubset(
          fif.query,
          stream,
          font,
          origin + _vec2<f32>( maxlabelw, 0 ),
          dim - _vec2<f32>( maxlabelw, 0 ),
          ZRange( zrange, editlayer_t::txt ),
          0,
          ( fif.focus == findinfilesfocus_t::query ),
          ( fif.focus == findinfilesfocus_t::query )
          );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // replace bar
        DrawString(
          stream,
          font,
          origin + _vec2<f32>( maxlabelw - label_repl_w, 0 ),
          GetZ( zrange, editlayer_t::txt ),
          origin,
          dim,
          rgba_text,
          spaces_per_tab,
          ML( label_repl )
          );

        TxtLayoutSingleLineSubset(
          fif.replacement,
          GetBOF( fif.replacement.buf ),
          TxtLen( fif.replacement ),
          font
          );
        TxtRenderSingleLineSubset(
          fif.replacement,
          stream,
          font,
          origin + _vec2<f32>( maxlabelw, 0 ),
          dim - _vec2<f32>( maxlabelw, 0 ),
          ZRange( zrange, editlayer_t::txt ),
          0,
          ( fif.focus == findinfilesfocus_t::replacement ),
          ( fif.focus == findinfilesfocus_t::replacement )
          );

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // result count
        if( fif.ncontexts_active ) {
          static const auto label = Str( " % files scanned..." );
          static const idx_t label_len = CsLen( label );
          auto label_w = LayoutString( font, spaces_per_tab, label, label_len );

          u8 count[128];
          idx_t count_len = 0;
          auto pct = 100.0f - ( fif.ncontexts_active * 100.0f ) / fif.max_ncontexts_active;
          count_len = sprintf_s( Cast( char*, count ), _countof( count ), "%.2f", pct );
          auto count_w = LayoutString( font, spaces_per_tab, count, count_len );

          DrawString(
            stream,
            font,
            origin + _vec2<f32>( 0.5f * ( dim.x - count_w - label_w ), 0 ),
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            rgba_text,
            spaces_per_tab,
            count, count_len
            );

          DrawString(
            stream,
            font,
            origin + _vec2<f32>( 0.5f * ( dim.x - count_w - label_w ) + count_w, 0 ),
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            rgba_text,
            spaces_per_tab,
            label, label_len
            );
        } else {
          static const auto label = Str( " results" );
          static const idx_t label_len = CsLen( label );
          auto label_w = LayoutString( font, spaces_per_tab, label, label_len );

          u8 count[128];
          idx_t count_len = 0;
          CsFromIntegerU( AL( count ), &count_len, fif.matches.totallen, 1 );
          auto count_w = LayoutString( font, spaces_per_tab, count, count_len );

          DrawString(
            stream,
            font,
            origin + _vec2<f32>( 0.5f * ( dim.x - count_w - label_w ), 0 ),
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            rgba_text,
            spaces_per_tab,
            count, count_len
            );

          DrawString(
            stream,
            font,
            origin + _vec2<f32>( 0.5f * ( dim.x - count_w - label_w ) + count_w, 0 ),
            GetZ( zrange, editlayer_t::txt ),
            origin,
            dim,
            rgba_text,
            spaces_per_tab,
            label, label_len
            );
        }

        origin.y += line_h;
        dim.y -= line_h;
      }

      { // bkgd
        RenderQuad(
          stream,
          GetPropFromDb( vec4<f32>, rgba_text_bkgd ),
          origin,
          origin + dim,
          origin, dim,
          GetZ( zrange, editlayer_t::bkgd )
          );
      }

      auto nlines_screen_floored = Cast( idx_t, dim.y / line_h );
      fif.pageupdn_distance = MAX( 1, nlines_screen_floored / 2 );
      fif.scroll_end = fif.scroll_start + MIN( nlines_screen_floored, fif.matches.totallen );

      if( fif.matches.totallen ) {
        fontlayout_t layout;
        FontInit( layout );

        auto pa_iter = MakeIteratorAtLinearIndex( fif.matches, fif.scroll_start );

        AssertCrash( fif.scroll_start <= fif.matches.totallen );
        auto nlines_render = MIN( 1 + nlines_screen_floored, fif.matches.totallen - fif.scroll_start );
        For( i, 0, nlines_render ) {
          auto elem = GetElemAtIterator( fif.matches, pa_iter );
          pa_iter = IteratorMoveR( fif.matches, pa_iter );

          auto is_cursor = fif.scroll_start + i == fif.cursor;

          // cursor
          if( is_cursor ) {
            auto p0 = _vec2<f32>( 0, line_h * i );
            auto p1 = p0 + _vec2( dim.x, line_h );
            RenderQuad(
              stream,
              rgba_cursor_bkgd,
              origin + p0,
              origin + p1,
              origin, dim,
              GetZ( zrange, editlayer_t::sel )
              );
          }

          // elem names
          {
            auto elem_origin = origin + _vec2<f32>( 0, line_h * i );
            auto elem_dim = _vec2( dim.x, line_h );
            slice_t name = elem->name;
            auto prefix_len = fif.matches_dir.len + 1; // include forwslash
            AssertCrash( name.len >= prefix_len );
            name.mem += prefix_len;
            name.len -= prefix_len;

            f32 name_w = LayoutString( font, spaces_per_tab, ML( name ) );
            f32 space_w = LayoutString( font, spaces_per_tab, Str( "   " ), 3 );
            DrawString(
              stream,
              font,
              elem_origin,
              GetZ( zrange, editlayer_t::txt ),
              origin,
              dim,
              is_cursor  ?  rgba_cursor_text  :  rgba_text,
              spaces_per_tab,
              ML( name )
              );

            elem_origin.x += name_w + space_w;
            elem_dim.x -= name_w + space_w;

            FontAddLayoutLine( font, layout, ML( elem->sample ), spaces_per_tab );
            idx_t line = i;
            f32 match_start = FontSumAdvances( layout, line, 0, elem->offset_into_sample );
            f32 match_end = match_start + FontSumAdvances( layout, line, elem->offset_into_sample, elem->len );

            RenderQuad(
              stream,
              rgba_wordmatch_bkgd,
              elem_origin + _vec2<f32>( match_start, 0 ),
              elem_origin + _vec2<f32>( match_end, elem_dim.y ),
              origin, dim,
              GetZ( zrange, editlayer_t::sel )
              );

            RenderText(
              stream,
              font,
              layout,
              is_cursor  ?  rgba_cursor_text  :  rgba_text,
              origin,
              dim,
              elem_origin,
              GetZ( zrange, editlayer_t::txt ),
              line,
              0, elem->sample.len
              );
          }
        }

        FontKill( layout );
      }

    } break;

    default: UnreachableCrash();
  }
}









// =================================================================================
// MOUSE

Inl idx_t
FileopenerMapMouseToCursor(
  fileopener_t& fo,
  vec2<f32> origin,
  vec2<f32> dim,
  vec2<s32> m,
  font_t& font,
  vec2<s8> px_click_correct
  )
{
  auto line_h = FontLineH( font );

  //idx_t c;

  f32 y_frac = ( m.y - origin.y + px_click_correct.y ) / dim.y;
  idx_t nlines_screen_max = Cast( idx_t, dim.y / line_h );
  idx_t cy = Cast( idx_t, y_frac * nlines_screen_max );

  //f32 x_frac = ( m.x - origin.x + px_click_correct.x ) / Cast( f32, font.char_w );
  //idx_t cx = Cast( idx_t, x_frac );
  //idx_t line_end;
  //CursorStopAtNewlineR( txt.buf, c, &line_end );
  //c = MIN( c + cx, line_end );
  //
  //*pos = c;

  cy += fo.scroll_start;
  if( cy ) {
    cy -= 1; // -1 for cwd display line.
  }
  idx_t r = MIN( cy, fo.matches.totallen - 1 );
  return r;
}

void
FileopenerControlMouse(
  edit_t& edit,
  bool& target_valid,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  glwmouseevent_t type,
  glwmousebtn_t btn,
  bool* alreadydn,
  bool* keyalreadydn,
  vec2<s32> m,
  vec2<s32> raw_delta,
  s32 dwheel
  )
{
  ProfFunc();

  auto px_click_correct = _vec2<s8>(); // TODO: mouse control.
  auto scroll_nlines = GetPropFromDb( u8, u8_scroll_nlines );
  auto scroll_sign = GetPropFromDb( s8, s8_scroll_sign );
  auto dblclick_period_sec = GetPropFromDb( f64, f64_dblclick_period_sec );

  bool ctrl  = keyalreadydn[Cast( enum_t, glwkey_t::ctrl )];
  bool shift = keyalreadydn[Cast( enum_t, glwkey_t::shift )];
  bool alt   = keyalreadydn[Cast( enum_t, glwkey_t::alt )];
  bool mod_isdn = ( ctrl | shift | alt );

  if( !GlwMouseInside( m, origin, dim ) ) {
    // clear all interactivity state.
    edit.fileopener.dblclick.first_made = 0;
    return;
  }

  switch( type ) {

    case glwmouseevent_t::wheelmove: {
      if( dwheel  &&  !mod_isdn ) {
        dwheel *= scroll_sign;
        dwheel *= scroll_nlines;
        if( dwheel < 0 ) {
          CmdFileopenerScrollU( edit, Cast( idx_t, -dwheel ) );
        } else {
          CmdFileopenerScrollD( edit, Cast( idx_t, dwheel ) );
        }
        target_valid = 0;
      }
    } break;

    case glwmouseevent_t::dn: {

      switch( btn ) {
        case glwmousebtn_t::l: {

          edit.fileopener.cursor = FileopenerMapMouseToCursor( edit.fileopener, origin, dim, m, font, px_click_correct );
          bool same_cursor = ( edit.fileopener.cursor == edit.fileopener.dblclick.first_cursor );
          bool double_click = ( edit.fileopener.dblclick.first_made & same_cursor );
          if( double_click ) {
            if( TimeSecFromClocks64( TimeClock() - edit.fileopener.dblclick.first_clock ) <= dblclick_period_sec ) {
              edit.fileopener.dblclick.first_made = 0;
              CmdFileopenerChoose( edit );
            } else {
              edit.fileopener.dblclick.first_clock = TimeClock();
            }
          } else {
            edit.fileopener.dblclick.first_made = 1;
            edit.fileopener.dblclick.first_clock = TimeClock();
            edit.fileopener.dblclick.first_cursor = edit.fileopener.cursor;
          }
          target_valid = 0;
        } break;

        case glwmousebtn_t::r:
        case glwmousebtn_t::m:
        case glwmousebtn_t::b4:
        case glwmousebtn_t::b5: {
        } break;

        default: UnreachableCrash();
      }
    } break;

    case glwmouseevent_t::up: {
    } break;

    case glwmouseevent_t::move: {
      //printf( "move ( %d, %d )\n", m.x, m.y );
    } break;

    default: UnreachableCrash();
  }
}


void
EditControlMouse(
  edit_t& edit,
  bool& target_valid,
  font_t& font,
  vec2<f32> origin,
  vec2<f32> dim,
  glwmouseevent_t type,
  glwmousebtn_t btn,
  bool* alreadydn,
  bool* keyalreadydn,
  vec2<s32> m,
  vec2<s32> raw_delta,
  s32 dwheel,
  bool allow_scrollbar
  )
{
  ProfFunc();
  auto line_h = FontLineH( font );

  switch( edit.mode ) {
    case editmode_t::editfile: {
      // status bar
      origin.y += line_h;
      dim.y -= line_h;

      edittxtopen_t* open = 0;
      vec2<f32> origin_active = {};
      vec2<f32> dim_active = {};

      if( edit.horzview ) {
        auto open_r = edit.active[0];
        auto open_l = edit.active[1];

        auto origin_l = origin;
        auto dim_l = _vec2( dim.x / 2, dim.y );

        auto origin_r = _vec2( origin.x + dim.x / 2, origin.y );
        auto dim_r =  _vec2( dim.x / 2, dim.y );

        if( open_l  &&  GlwMouseInside( m, origin_l, dim_l ) ) {
          open = open_l;
          origin_active = origin_l;
          dim_active = dim_l;
        } elif( open_r  &&  GlwMouseInside( m, origin_r, dim_r ) ) {
          open = open_r;
          origin_active = origin_r;
          dim_active = dim_r;
        }
      } else {
        open = edit.active[edit.horzfocus_l];
        if( open  &&  GlwMouseInside( m, origin, dim ) ) {
          origin_active = origin;
          dim_active = dim;
        }
      }
      if( open ) {
        TxtControlMouse(
          open->txt,
          target_valid,
          font,
          origin_active,
          dim_active,
          type,
          btn,
          alreadydn,
          keyalreadydn,
          m,
          raw_delta,
          dwheel,
          allow_scrollbar
          );
      }
    } break;

    case editmode_t::fileopener: {
      FileopenerControlMouse(
        edit,
        target_valid,
        font,
        origin,
        dim,
        type,
        btn,
        alreadydn,
        keyalreadydn,
        m,
        raw_delta,
        dwheel
        );
    } break;

//    default: UnreachableCrash();
  }
}








// =================================================================================
// KEYBOARD

struct
edit_cmdmap_t
{
  glwkeybind_t keybind;
  pfn_editcmd_t fn;
  idx_t misc;
};

Inl edit_cmdmap_t
_editcmdmap(
  glwkeybind_t keybind,
  pfn_editcmd_t fn,
  idx_t misc = 0
  )
{
  edit_cmdmap_t r;
  r.keybind = keybind;
  r.fn = fn;
  r.misc = misc;
  return r;
}

Inl void
ExecuteCmdMap(
  edit_t& edit,
  edit_cmdmap_t* table,
  idx_t table_len,
  glwkey_t key,
  bool* alreadydn,
  bool& target_valid,
  bool& ran_cmd
  )
{
  For( i, 0, table_len ) {
    auto entry = table + i;
    if( GlwKeybind( key, alreadydn, entry->keybind ) ) {
      entry->fn( edit, entry->misc );
      target_valid = 0;
      ran_cmd = 1;
    }
  }
}



void
EditControlKeyboard(
  edit_t& edit,
  bool kb_command,
  bool& target_valid,
  bool& ran_cmd,
  glwkeyevent_t type,
  glwkey_t key,
  glwkeylocks_t& keylocks,
  bool* alreadydn
  )
{
  ProfFunc();

  switch( edit.mode ) {
    case editmode_t::editfile: {
      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_switchopened_from_editfile      ), CmdMode_switchopened_from_editfile      ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_fileopener_from_editfile        ), CmdMode_fileopener_from_editfile        ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_findrepl_from_editfile ), CmdMode_editfile_findrepl_from_editfile ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_findinfiles_from_editfile       ), CmdMode_findinfiles_from_editfile       ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_gotoline_from_editfile ), CmdMode_editfile_gotoline_from_editfile ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_swap_horz                   ), CmdEditfileSwapHorz                     ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_swap_horzfocus              ), CmdEditfileSwapHorzFocus                ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_move_horz_l                 ), CmdEditfileMoveHorzL                    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_move_horz_r                 ), CmdEditfileMoveHorzR                    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_toggle_horzview             ), CmdEditfileToggleHorzview               ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } break;

          case glwkeyevent_t::repeat:
          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        auto open = GetActiveOpen( edit );
        if( open ) {
          TxtControlKeyboard(
            open->txt,
            kb_command,
            target_valid,
            open->unsaved,
            ran_cmd,
            type,
            key,
            keylocks,
            alreadydn
            );
        }
      }
    } break;

    case editmode_t::editfile_findrepl: {
      auto& active_findreplace = GetActiveFindReplaceTxt( edit.findrepl );

      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_from_editfile_findrepl ), CmdMode_editfile_from_editfile_findrepl ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } __fallthrough;

          case glwkeyevent_t::repeat: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_toggle_focus          ), CmdEditfileFindreplToggleFocus        ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_toggle_case_sensitive ), CmdEditfileFindreplToggleCaseSens     ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_toggle_word_boundary  ), CmdEditfileFindreplToggleWordBoundary ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_find_l                ), CmdEditfileFindreplFindL              ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_find_r                ), CmdEditfileFindreplFindR              ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_repl_l                ), CmdEditfileFindreplReplaceL           ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_findrepl_repl_r                ), CmdEditfileFindreplReplaceR           ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } break;

          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        bool content_changed = 0;
        TxtControlKeyboardSingleLine(
          active_findreplace,
          kb_command,
          target_valid,
          content_changed,
          ran_cmd,
          type,
          key,
          keylocks,
          alreadydn
          );
      }
    } break;

    case editmode_t::editfile_gotoline: {

      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_from_editfile_gotoline ), CmdMode_editfile_from_editfile_gotoline ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_editfile_gotoline_choose             ), CmdEditfileGotolineChoose               ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } __fallthrough;

          case glwkeyevent_t::repeat:
          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        bool content_changed = 0;
        TxtControlKeyboardSingleLine(
          edit.gotoline,
          kb_command,
          target_valid,
          content_changed,
          ran_cmd,
          type,
          key,
          keylocks,
          alreadydn
          );
      }
    } break;

    case editmode_t::switchopened: {
      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_from_switchopened ), CmdMode_editfile_from_switchopened ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_choose             ), CmdSwitchopenedChoose              ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } __fallthrough;

          case glwkeyevent_t::repeat: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_closefile           ), CmdSwitchopenedCloseFile         ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_cursor_u            ), CmdSwitchopenedCursorU           ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_cursor_d            ), CmdSwitchopenedCursorD           ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_cursor_page_u       ), CmdSwitchopenedCursorU           , edit.nlines_screen ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_cursor_page_d       ), CmdSwitchopenedCursorD           , edit.nlines_screen ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_scroll_u            ), CmdSwitchopenedScrollU           , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_scroll_d            ), CmdSwitchopenedScrollD           , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_scroll_page_u       ), CmdSwitchopenedScrollU           , edit.nlines_screen ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_scroll_page_d       ), CmdSwitchopenedScrollD           , edit.nlines_screen ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_switchopened_make_cursor_present ), CmdSwitchopenedMakeCursorPresent ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } break;

          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        bool content_changed = 0;
        TxtControlKeyboardSingleLine(
          edit.opened_search,
          kb_command,
          target_valid,
          content_changed,
          ran_cmd,
          type,
          key,
          keylocks,
          alreadydn
          );
        // auto-update the matches, since it's pretty fast.
        if( content_changed ) {
          CmdUpdateSearchMatches( edit );
        }
      }
    } break;

    case editmode_t::fileopener: {
      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_from_fileopener            ), CmdMode_editfile_from_fileopener            ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_fileopener_renaming_from_fileopener ), CmdMode_fileopener_renaming_from_fileopener ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_recycle_file_or_dir           ), CmdFileopenerRecycle                        ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_change_cwd_up                 ), CmdFileopenerChangeCwdUp                    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_refresh                       ), CmdFileopenerRefresh                        ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_newfile                       ), CmdFileopenerNewFile                        ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_newdir                        ), CmdFileopenerNewDir                         ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_choose                        ), CmdFileopenerChoose                         ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } __fallthrough;

          case glwkeyevent_t::repeat: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_focus_u       ), CmdFileopenerFocusU  ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_focus_d       ), CmdFileopenerFocusD  ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_cursor_u      ), CmdFileopenerCursorU ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_cursor_d      ), CmdFileopenerCursorD ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_cursor_page_u ), CmdFileopenerCursorU , edit.fileopener.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_cursor_page_d ), CmdFileopenerCursorD , edit.fileopener.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_scroll_u      ), CmdFileopenerScrollU , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_scroll_d      ), CmdFileopenerScrollD , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_scroll_page_u ), CmdFileopenerScrollU , edit.fileopener.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_scroll_page_d ), CmdFileopenerScrollD , edit.fileopener.pageupdn_distance ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } break;

          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        switch( edit.fileopener.focus ) {
          case fileopenerfocus_t::dir: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              edit.fileopener.cwd,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            if( content_changed ) {
//              CmdFileopenerRefresh( edit );
            }
          } break;
          case fileopenerfocus_t::query: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              edit.fileopener.query,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            // auto-update the matches, since it's pretty fast.
            if( content_changed ) {
              CmdFileopenerUpdateMatches( edit );
            }
          } break;
          case fileopenerfocus_t::ignored_filetypes: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              edit.fileopener.ignored_filetypes,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            // auto-update the matches, since it's pretty fast.
            if( content_changed ) {
              CmdFileopenerUpdateMatches( edit );
            }
          } break;
          case fileopenerfocus_t::ignored_substrings: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              edit.fileopener.ignored_substrings,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            // auto-update the matches, since it's pretty fast.
            if( content_changed ) {
              CmdFileopenerUpdateMatches( edit );
            }
          } break;
          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        if( kb_command ) {
          switch( type ) {
            case glwkeyevent_t::dn: {
              edit_cmdmap_t table[] = {
                _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_from_fileopener ), CmdMode_editfile_from_fileopener ),
                _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
                _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
              };
              ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
            } break;

            case glwkeyevent_t::repeat:
            case glwkeyevent_t::up: {
            } break;

            default: UnreachableCrash();
          }
        }
      }
    } break;

    case editmode_t::fileopener_renaming: {
      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // dir level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_fileopener_from_fileopener_renaming ), CmdMode_fileopener_from_fileopener_renaming ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_fileopener_renaming_apply                ), CmdFileopenerRenamingApply                  ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } __fallthrough;

          case glwkeyevent_t::repeat:
          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        bool content_changed = 0;
        TxtControlKeyboardSingleLine(
          edit.fileopener.renaming_txt,
          kb_command,
          target_valid,
          content_changed,
          ran_cmd,
          type,
          key,
          keylocks,
          alreadydn
          );
      }
    } break;

    case editmode_t::externalmerge: {
      auto open = edit.active_externalmerge;

      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_externalmerge_discard_local_changes ), CmdExternalmergeDiscardLocalChanges ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_externalmerge_keep_local_changes    ), CmdExternalmergeKeepLocalChanges    ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } break;

          case glwkeyevent_t::repeat:
          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        TxtControlKeyboardNoContentChange(
          open->txt,
          kb_command,
          target_valid,
          ran_cmd,
          type,
          key,
          keylocks,
          alreadydn
          );
      }
    } break;

    case editmode_t::findinfiles: {
      auto& fif = edit.findinfiles;
      if( kb_command ) {
        switch( type ) {
          case glwkeyevent_t::dn: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_mode_editfile_from_findinfiles ), CmdMode_editfile_from_findinfiles ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_replace_at_cursor  ), CmdFindinfilesReplaceAtCursor     ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_replace_all        ), CmdFindinfilesReplaceAll          ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_refresh            ), CmdFindinfilesRefresh             ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_choose             ), CmdFindinfilesChoose              ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_save    ), CmdSave    ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_saveall ), CmdSaveAll ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } __fallthrough;

          case glwkeyevent_t::repeat: {
            // edit level commands
            edit_cmdmap_t table[] = {
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_focus_u               ), CmdFindinfilesFocusU             ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_focus_d               ), CmdFindinfilesFocusD             ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_cursor_u              ), CmdFindinfilesCursorU            ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_cursor_d              ), CmdFindinfilesCursorD            ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_cursor_page_u         ), CmdFindinfilesCursorU            , fif.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_cursor_page_d         ), CmdFindinfilesCursorD            , fif.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_scroll_u              ), CmdFindinfilesScrollU            , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_scroll_d              ), CmdFindinfilesScrollD            , Cast( idx_t, GetPropFromDb( f32, f32_lines_per_jump ) ) ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_scroll_page_u         ), CmdFindinfilesScrollU            , fif.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_scroll_page_d         ), CmdFindinfilesScrollD            , fif.pageupdn_distance ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_toggle_case_sensitive ), CmdFindinfilesToggleCaseSens     ),
              _editcmdmap( GetPropFromDb( glwkeybind_t, keybind_findinfiles_toggle_word_boundary  ), CmdFindinfilesToggleWordBoundary ),
            };
            ExecuteCmdMap( edit, AL( table ), key, alreadydn, target_valid, ran_cmd );
          } break;

          case glwkeyevent_t::up: {
          } break;

          default: UnreachableCrash();
        }
      }

      if( !ran_cmd ) {
        switch( fif.focus ) {
          case findinfilesfocus_t::dir: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              fif.dir,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            if( content_changed ) {
//              CmdFindinfilesRefresh( edit );
            }
          } break;
          case findinfilesfocus_t::ignored_filetypes: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              fif.ignored_filetypes,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            if( content_changed ) {
//              CmdFindinfilesUpdateMatches( edit );
            }
          } break;
          case findinfilesfocus_t::query: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              fif.query,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
            if( content_changed ) {
//              CmdFindinfilesUpdateMatches( edit );
            }
          } break;
          case findinfilesfocus_t::replacement: {
            bool content_changed = 0;
            TxtControlKeyboardSingleLine(
              fif.replacement,
              kb_command,
              target_valid,
              content_changed,
              ran_cmd,
              type,
              key,
              keylocks,
              alreadydn
              );
          } break;
        }
      }
    } break;

    default: UnreachableCrash();
  }
}




struct
notify_t
{
  slice_t msg;
  kahan32_t t;
};

struct
notifyui_t
{
  plist_t mem;
  listwalloc_t<notify_t> msgs;
  plist_t lmem; // TODO: why do we crash w/ 1 plist?
  bool fadedin;
};

Inl void
Init( notifyui_t& ui )
{
  Init( ui.mem, 65536 );
  Init( ui.lmem, 65536 );
  Init( ui.msgs, &ui.lmem );
  ui.fadedin = 0;
}

Inl void
Kill( notifyui_t& ui )
{
  Kill( ui.mem );
  Kill( ui.lmem );
  Kill( ui.msgs );
  ui.fadedin = 0;
}


Enumc( app_active_t )
{
  edit,
  cmd,
};


struct
app_t
{
  glwclient_t client;
  edit_t edit;
  cmd_t cmd;
  app_active_t active;
  bool fullscreen;
  bool kb_command; // else edit.
  array_t<f32> stream;

#if OPENGL_INSTEAD_OF_SOFTWARE
  u32 glstream;
  shader_tex2_t shader;
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  array_t<font_t> fonts;
  notifyui_t notifyui;
};

app_t* App()
{
  static app_t g_app = {};
  return &g_app;
}


void
AppInit( app_t* app )
{
  Init( app->notifyui );
  Alloc( app->stream, 65536 );
  Alloc( app->fonts, 16 );
  app->fullscreen = 0;
  app->active = app_active_t::edit;
  app->kb_command = 1;
  EditInit( app->edit );
  CmdInit( app->cmd );

  GlwInit(
    app->client,
    Str( "te" ), 2
    );

#if OPENGL_INSTEAD_OF_SOFTWARE
  glGenBuffers( 1, &app->glstream );
  ShaderInit( app->shader );
#endif // OPENGL_INSTEAD_OF_SOFTWARE
}

void
AppKill( app_t* app )
{
  ForLen( i, app->fonts ) {
    auto& font = app->fonts.mem[i];
    FontKill( font );
  }
  Free( app->fonts );

#if OPENGL_INSTEAD_OF_SOFTWARE
  ShaderKill( app->shader );
  glDeleteBuffers( 1, &app->glstream );
#endif // OPENGL_INSTEAD_OF_SOFTWARE

  Free( app->stream );
  EditKill( app->edit );
  CmdKill( app->cmd );
  Kill( app->notifyui );

  GlwKill( app->client );
}



void
LogUI( void* cstr ... )
{
  auto app = App();
  static embeddedarray_t<u8, 32768> buffer;

  va_list args;
  va_start( args, cstr );

  buffer.len += vsprintf_s(
    Cast( char* const, buffer.mem + buffer.len ),
    MAX( Capacity( buffer ), buffer.len ) - buffer.len,
    Cast( const char* const, cstr ),
    args
    );

  va_end( args );

  auto notify_elem = AddLast( app->notifyui.msgs );
  auto notify = &notify_elem->value;
  notify->msg.mem = AddPlist( u8, app->notifyui.mem, buffer.len );
  notify->msg.len = buffer.len;
  Memmove( notify->msg.mem, ML( buffer ) );
  notify->t = {};

  buffer.len = 0;

  app->client.target_valid = 0;
}


Enumc( fontid_t )
{
  normal,
};


void
LoadFont(
  app_t* app,
  enum_t fontid,
  u8* filename_ttf,
  idx_t filename_ttf_len,
  f32 char_h
  )
{
  AssertWarn( fontid < 10000 ); // sanity check.
  Reserve( app->fonts, fontid + 1 );
  app->fonts.len = MAX( app->fonts.len, fontid + 1 );

  auto& font = app->fonts.mem[fontid];
  FontLoad( font, filename_ttf, filename_ttf_len, char_h );
  FontLoadAscii( font );
}

void
UnloadFont( app_t* app, enum_t fontid )
{
  auto font = app->fonts.mem + fontid;
  FontKill( *font );
}

font_t&
GetFont( app_t* app, enum_t fontid )
{
  return app->fonts.mem[fontid];
}


Enumc( applayer_t )
{
  edit,
  bkgd,
  txt,
  COUNT
};


__OnRender( AppOnRender )
{
  ProfFunc();

  auto app = Cast( app_t*, misc );
  auto& font = GetFont( app, Cast( enum_t, fontid_t::normal ) );
  auto line_h = FontLineH( font );
  auto orig_dim = dim;
  auto zrange = _vec2<f32>( 0, 1 );

  auto spaces_per_tab = GetPropFromDb( u8, u8_spaces_per_tab );
  auto rgba_notify_text = GetPropFromDb( vec4<f32>, rgba_notify_text );
  auto rgba_notify_bkgd = GetPropFromDb( vec4<f32>, rgba_notify_bkgd );
  auto rgba_mode_command = GetPropFromDb( vec4<f32>, rgba_mode_command );
  auto rgba_mode_edit = GetPropFromDb( vec4<f32>, rgba_mode_edit );
  auto notification_lifetime = GetPropFromDb( f32, f32_notification_lifetime );
  auto modeoutline_pct = GetPropFromDb( f32, f32_modeoutline_pct );

  auto timestep = MIN( timestep_realtime, timestep_fixed );

  // draw the global kb_command as a colored outline.
  auto px_border = MAX( 3.0f, Round32( modeoutline_pct * MIN( dim.x, dim.y ) ) );
  RenderQuad(
    app->stream,
    app->kb_command ? rgba_mode_command : rgba_mode_edit,
    origin, origin + _vec2( px_border, dim.y ),
    origin, dim,
    GetZ( zrange, applayer_t::bkgd )
    );
  RenderQuad(
    app->stream,
    app->kb_command ? rgba_mode_command : rgba_mode_edit,
    origin + _vec2<f32>( dim.x - px_border, 0 ), origin + dim,
    origin, dim,
    GetZ( zrange, applayer_t::bkgd )
    );
  origin.x += px_border;
  dim.x -= 2 * px_border;

  switch( app->active ) {

    case app_active_t::edit: {
      EditRender(
        app->edit,
        target_valid,
        app->stream,
        font,
        origin,
        dim,
        ZRange( zrange, applayer_t::edit ),
        timestep_realtime,
        timestep_fixed
        );
    } break;

    case app_active_t::cmd: {
      CmdRender(
        app->cmd,
        target_valid,
        app->stream,
        font,
        origin,
        dim,
        ZRange( zrange, applayer_t::edit ),
        timestep_realtime,
        timestep_fixed
        );
    } break;

    default: UnreachableCrash();
  }

  auto notifyui = &app->notifyui;
  if( notifyui->msgs.len ) {
    auto elem = notifyui->msgs.first;
    auto notify = &elem->value;

    target_valid = 0;

    if( notify->t.sum >= notification_lifetime ) {
      auto remove = elem;
      elem = elem->next;
      Rem( notifyui->msgs, remove );
      Reclaim( notifyui->msgs, remove );
    }
    if( elem ) {
      notify = &elem->value;

      Add( notify->t, Cast( f32, timestep ) );

      auto msg_origin = origin;
      auto msg_dim = dim;
      msg_origin.y += dim.y - line_h;
      msg_dim.y    -= dim.y - line_h;

      auto rampup = 0.05f * notification_lifetime; // TODO: propdb coeffs
      auto middle = 0.9f * notification_lifetime;
      auto rampdn = 0.05f * notification_lifetime;
      auto color_bkgd = rgba_notify_bkgd;
      auto color_text = rgba_notify_text;
      if( notify->t.sum < rampup ) {
        auto t = notify->t.sum / rampup;
        color_text.w = Smoothstep32( t );
        if( !notifyui->fadedin ) {
          color_bkgd.w = Smoothstep32( t );
        }
      } elif( notify->t.sum < rampup + middle ) {
        notifyui->fadedin = 1;
      } else {
        auto t = ( notify->t.sum - rampup - middle ) / rampdn;
        color_text.w = 1.0f - Smoothstep32( t );
        if( !notifyui->fadedin  ||  notifyui->msgs.len == 1 ) {
          color_bkgd.w = 1.0f - Smoothstep32( t );
        }
      }

      RenderQuad(
        app->stream,
        color_bkgd,
        msg_origin,
        msg_origin + msg_dim,
        origin, dim,
        GetZ( zrange, applayer_t::bkgd )
        );

      f32 count_w = 0.0f;
      if( notifyui->msgs.len > 1 ) {
        auto count = AllocString( " %d left ", notifyui->msgs.len - 1 );
        count_w = LayoutString( font, spaces_per_tab, ML( count ) );
        DrawString(
          app->stream,
          font,
          msg_origin,
          GetZ( zrange, applayer_t::txt ),
          msg_origin,
          msg_dim,
          color_text,
          spaces_per_tab,
          ML( count )
          );
        Free( count );
      }

      auto w = LayoutString( font, spaces_per_tab, ML( notify->msg ) );
      DrawString(
        app->stream,
        font,
        msg_origin + _vec2<f32>( MAX( count_w, 0.5f * ( msg_dim.x - w ) ), 0 ),
        GetZ( zrange, applayer_t::txt ),
        msg_origin,
        msg_dim,
        color_text,
        spaces_per_tab,
        ML( notify->msg )
        );
    }
  } else {
    notifyui->fadedin = 0;
    Reset( notifyui->mem );
  }

  if( 0 )
  { // display timestep_realtime, as a way of tracking how long rendering takes.
    embeddedarray_t<u8, 64> tmp;
    CsFrom_f64( tmp.mem, Capacity( tmp ), &tmp.len, 1000 * timestep_realtime );
    auto w = LayoutString( font, spaces_per_tab, ML( tmp ) );
    DrawString(
      app->stream,
      font,
      origin + _vec2<f32>( dim.x - w, 0 ),
      GetZ( zrange, applayer_t::txt ),
      origin,
      dim,
      rgba_notify_text,
      spaces_per_tab,
      ML( tmp )
      );
  }

  if( app->stream.len ) {
#if OPENGL_INSTEAD_OF_SOFTWARE
    glUseProgram( app->shader.core.program );  glVerify();

#if 0
    app->stream.len = 0;
    OutputQuad(
      app->stream,
      _vec2<f32>( 0, Ceil32( offset ) ) + _vec2<f32>( 0, 0 ),
      _vec2<f32>( 0, Ceil32( offset ) ) + _vec2<f32>( 512, 108 ),
      1.0f,
      _vec2<f32>( 0, 0 ),
      _vec2<f32>( 1, 1 ),
      _vec4<f32>( 1, 1, 1, 1 )
      );
#endif

    // we invert y, since we layout from the top-left as the origin.
    // NOTE: it's important we use orig_dim, since we need ortho-size to match viewport-size for 1:1 pixel rendering.
    mat4x4r<f32> ndc_from_client;
    Ortho( &ndc_from_client, 0.0f, orig_dim.x, orig_dim.y, 0.0f, 0.0f, 1.0f );
    glUniformMatrix4fv( app->shader.loc_ndc_from_client, 1, 1, &ndc_from_client.row0.x );  glVerify();

    glUniform1i( app->shader.loc_tex_sampler, 0 );  glVerify();
    GlwBindTexture( 0, font.texid );

    glBindBuffer( GL_ARRAY_BUFFER, app->glstream );
    glBufferData( GL_ARRAY_BUFFER, app->stream.len * sizeof( f32 ), app->stream.mem, GL_STREAM_DRAW );  glVerify();
    glEnableVertexAttribArray( app->shader.attribloc_pos );  glVerify();
    glEnableVertexAttribArray( app->shader.attribloc_tccolor );  glVerify();
    glVertexAttribPointer( app->shader.attribloc_pos, 3, GL_FLOAT, 0, 6 * sizeof( f32 ), 0 );  glVerify();
    glVertexAttribPointer( app->shader.attribloc_tccolor, 3, GL_FLOAT, 0, 6 * sizeof( f32 ), Cast( void*, 3 * sizeof( f32 ) ) );  glVerify();

    auto vert_count = app->stream.len / 6;
    AssertCrash( app->stream.len % 6 == 0 );
    AssertCrash( vert_count <= MAX_s32 );
    glDrawArrays( GL_TRIANGLES, 0, Cast( s32, vert_count ) );  glVerify();

    glDisableVertexAttribArray( app->shader.attribloc_pos );  glVerify();
    glDisableVertexAttribArray( app->shader.attribloc_tccolor );  glVerify();

    //Log( "bytes sent to gpu: %llu", sizeof( f32 ) * app->stream.len );
#else // !OPENGL_INSTEAD_OF_SOFTWARE

#define RAND_NOISE 0

  #if RAND_NOISE
    rng_xorshift32_t rng;
    Init( rng, TimeTSC() );
  #endif


#define LOGFILLRATE 0

  #if LOGFILLRATE
    inc_stats_t stats_texalpha;
    Init( stats_texalpha );

    inc_stats_t stats_solidalpha;
    Init( stats_solidalpha );

    inc_stats_t stats_solidopaque;
    Init( stats_solidopaque );
  #endif


    //
    // PERF: frame stats on 4k TV, with main_test.asm open, 2 pages down:
    //
    // TotalFrame          12.918 ms
    //   ZeroBitmap           1.092 ms
    //   AppOnRender          8.841 ms
    //     tmp_DrawTexQuads     4.769 ms
    //     tmp_DrawPlainRects   3.388 ms
    //   BlitToScreen         2.985 ms
    //
    // TotalFrame          12.374 ms
    //   ZeroBitmap           0.972 ms
    //   AppOnRender          7.896 ms
    //     tmp_DrawTexQuads     4.993 ms
    //     tmp_DrawOpaqueQuads  1.344 ms
    //     tmp_DrawAlphaQuads   0.887 ms
    //   BlitToScreen         3.506 ms
    //
    // MakeTargetValid     10.050 ms
    //   ZeroBitmap           1.067 ms
    //   AppOnRender          5.869 ms
    //     tmp_DrawTexQuads     3.365 ms
    //     tmp_DrawOpaqueQuads  1.485 ms
    //     tmp_DrawAlphaQuads   0.543 ms
    //   BlitToScreen         3.114 ms
    //



    auto pos = app->stream.mem;
    auto end = app->stream.mem + app->stream.len;
    AssertCrash( app->stream.len % 10 == 0 );
    while( pos < end ) {
      Prof( tmp_UnpackStream );
      vec2<u32> p0;
      vec2<u32> p1;
      vec2<u32> tc0;
      vec2<u32> tc1;
      p0.x  = Round_u32_from_f32( *pos++ );
      p0.y  = Round_u32_from_f32( *pos++ );
      p1.x  = Round_u32_from_f32( *pos++ );
      p1.y  = Round_u32_from_f32( *pos++ );
      tc0.x = Round_u32_from_f32( *pos++ );
      tc0.y = Round_u32_from_f32( *pos++ );
      tc1.x = Round_u32_from_f32( *pos++ );
      tc1.y = Round_u32_from_f32( *pos++ );
      *pos++; // this is z; but we don't do z reordering right now.
      auto color = UnpackColorForShader( *pos++ );
      auto color_a = Cast( u8, Round_u32_from_f32( color.w * 255.0f ) & 0xFFu );
      auto color_r = Cast( u8, Round_u32_from_f32( color.x * 255.0f ) & 0xFFu );
      auto color_g = Cast( u8, Round_u32_from_f32( color.y * 255.0f ) & 0xFFu );
      auto color_b = Cast( u8, Round_u32_from_f32( color.z * 255.0f ) & 0xFFu );
      auto coloru = ( color_a << 24 ) | ( color_r << 16 ) | ( color_g <<  8 ) | color_b;
      auto color_argb = _mm_set_ps( color.w, color.x, color.y, color.z );
      auto color255_argb = _mm_mul_ps( color_argb, _mm_set1_ps( 255.0f ) );
      bool just_copy =
        color.x == 1.0f  &&
        color.y == 1.0f  &&
        color.z == 1.0f  &&
        color.w == 1.0f;
      bool no_alpha = color.w == 1.0f;

      AssertCrash( p0.x <= p1.x );
      AssertCrash( p0.y <= p1.y );
      auto dstdim = p1 - p0;
      AssertCrash( tc0.x <= tc1.x );
      AssertCrash( tc0.y <= tc1.y );
      auto srcdim = tc1 - tc0;
      AssertCrash( p0.x + srcdim.x <= app->client.dim.x );
      AssertCrash( p0.y + srcdim.y <= app->client.dim.y );
      AssertCrash( p0.x + dstdim.x <= app->client.dim.x ); // should have clipped by now.
      AssertCrash( p0.y + dstdim.y <= app->client.dim.y );
      AssertCrash( tc0.x + srcdim.x <= font.tex_dim.x );
      AssertCrash( tc0.y + srcdim.y <= font.tex_dim.y );
      ProfClose( tmp_UnpackStream );

      bool subpixel_quad =
        ( p0.x == p1.x )  ||
        ( p0.y == p1.y );

      // below our resolution, so ignore it.
      // we're not going to do linear-filtering rendering here.
      // something to consider for the future.
      if( subpixel_quad ) {
        continue;
      }

      // for now, assume quads with empty tex coords are just plain rects, with the given color.
      bool nontex_quad =
        ( !srcdim.x  &&  !srcdim.y );

      if( nontex_quad ) {
        u32 copy = 0;
        if( just_copy ) {
          copy = MAX_u32;
        } elif( no_alpha ) {
          copy = ( 0xFF << 24 ) | coloru;
        }

        if( just_copy  ||  no_alpha ) {
          Prof( tmp_DrawOpaqueQuads );

  #if LOGFILLRATE
          auto t0 = TimeTSC();
  #endif

#if 0
          // PERF: this is ~0.7 cycles / pixel
          // turns out rep stosd is hard to beat.
          // movdqu ( _mm_storeu_si128 ), plus remainder rep stosd, is ~1 cycle / pixel, slower than this.
          // vmovdqu ( _mm256_storeu_si256 ), plus remainder rep stosd, also ~1 cycle / pixel.

          Fori( u32, j, 0, dstdim.y ) {
            auto dst = app->client.fullscreen_bitmap_argb + ( p0.y + j ) * app->client.dim.x + ( p0.x + 0 );
            Fori( u32, k, 0, dstdim.x ) {
              *dst++ = copy;
            }
          }

#elif 1
          // PERF: this is ~0.7 cycles / pixel
          // suprisingly, rep stosq is about the same as rep stosd!
          // presumably the hardware folks made that happen, so old 32bit code also got faster.
          auto copy2 = Pack( copy, copy );
          Fori( u32, j, 0, dstdim.y ) {
            auto dst = app->client.fullscreen_bitmap_argb + ( p0.y + j ) * app->client.dim.x + ( p0.x + 0 );
            if( dstdim.x & 1 ) {
              *dst++ = copy;
            }
            auto dim2 = dstdim.x / 2;
            auto dst2 = Cast( u64*, dst );
            Fori( u32, k, 0, dim2 ) {
              *dst2++ = copy2;
            }
          }

#elif 0
          // PERF: this is ~0.6 cycles / pixel
          // vmovntdq ( _mm256_stream_si256 ), plus alignment, and remainder, is sometimes faster.
          // on a horizontal 4k monitor, it looks like our rows are long enough to pay for alignment setup.
          // but on a smaller monitor, the rep stos* wins out. so keep that one enabled for now.

          auto copy8 = _mm256_set1_epi32( *Cast( s32*, &copy ) );
          Fori( u32, j, 0, dstdim.y ) {
            auto dst = app->client.fullscreen_bitmap_argb + ( p0.y + j ) * app->client.dim.x + ( p0.x + 0 );
            auto ntoalign = ( RoundUpMultiple32( Cast( idx_t, dst ) ) - Cast( idx_t, dst ) ) / 4;
            if( dstdim.x <= ntoalign ) {
              Fori( u32, k, 0, dstdim.x ) {
                *dst++ = copy;
              }
              continue;
            }
            Fori( u32, k, 0, ntoalign ) {
              *dst++ = copy;
            }
            auto dimx = dstdim.x - ntoalign;
            auto quo = dimx / 8;
            auto rem = dimx % 8;
            auto dst8 = Cast( __m256i*, dst );
            Fori( u32, k, 0, quo ) {
              _mm256_stream_si256( dst8, copy8 );
              ++dst8;
            }
            dst = Cast( u32*, dst8 );
            Fori( u32, k, 0, rem ) {
              *dst++ = copy;
            }
          }
#endif

  #if LOGFILLRATE
          auto dt = TimeTSC() - t0;
          auto count = dstdim.x * dstdim.y;
          AddMean( stats_solidopaque, Cast( f32, dt ) / count, count );
  #endif

        } else {
          Prof( tmp_DrawAlphaQuads );
          // more complicated / slow alpha case.
  #if LOGFILLRATE
          auto t0 = TimeTSC();
  #endif
          Fori( u32, j, 0, dstdim.y ) {
            auto dst = app->client.fullscreen_bitmap_argb + ( p0.y + j ) * app->client.dim.x + ( p0.x + 0 );
            Fori( u32, k, 0, dstdim.x ) {

#if 0
              // PERF: this is ~17 cycles / pixel

              u8 dst_a = ( *dst >> 24 ) & 0xFF;
              u8 dst_r = ( *dst >> 16 ) & 0xFF;
              u8 dst_g = ( *dst >>  8 ) & 0xFF;
              u8 dst_b = ( *dst >>  0 ) & 0xFF;
              auto src_factor = color.w;
              auto dst_factor = 1 - src_factor;
  #if RAND_NOISE
              u32 val = Rand32( rng );
              u8 src = Cast( u8, val ) | ( 3 << 6 ); // rand from 192 to 255.
  #else
              u8 src = 255;
  #endif
              auto out_a = Cast( u8, Round_u32_from_f32( ( src           * src_factor + dst_a * dst_factor ) ) );
              auto out_r = Cast( u8, Round_u32_from_f32( ( src * color.x * src_factor + dst_r * dst_factor ) ) );
              auto out_g = Cast( u8, Round_u32_from_f32( ( src * color.y * src_factor + dst_g * dst_factor ) ) );
              auto out_b = Cast( u8, Round_u32_from_f32( ( src * color.z * src_factor + dst_b * dst_factor ) ) );
              *dst++ = ( out_a << 24 ) | ( out_r << 16 ) | ( out_g << 8 ) | out_b;

#elif 1
              // PERF: this is ~6 cycles / pixel

              // unpack dst
              auto dstf = _mm_set_ss( *Cast( f32*, dst ) );
              auto dsti = _mm_cvtepu8_epi32( _mm_castps_si128( dstf ) );
              auto dst_argb = _mm_cvtepi32_ps( dsti );

              // src = color
              auto src_argb = color255_argb;

              // out = dst + src_a * ( src - dst )
              auto src_a = _mm_set1_ps( color.w );
              auto diff_argb = _mm_sub_ps( src_argb, dst_argb );
              auto out_argb = _mm_fmadd_ps( src_a, diff_argb, dst_argb );

              // put out in [0, 255]
              auto outi = _mm_cvtps_epi32( out_argb );
              auto out = _mm_packs_epi32( outi, _mm_set1_epi32( 0 ) );
              out = _mm_packus_epi16( out, _mm_set1_epi32( 0 ) );

              _mm_storeu_si32( dst, out ); // equivalent to: *dst = out.m128i_u32[0]
              ++dst;
#endif
            }
          }
  #if LOGFILLRATE
          auto dt = TimeTSC() - t0;
          auto count = dstdim.x * dstdim.y;
          AddMean( stats_solidalpha, Cast( f32, dt ) / count, count );
  #endif
        }
      } else {
        Prof( tmp_DrawTexQuads );
        // for now, assume quads with nonempty tex coords are exact-size copies onto dst.
        // that's true of all our text glyphs, which makes for faster code here.
        //
        // TODO: handle scaling up, or make a separate path for scaled-up tex, non-tex, etc. quads
        // we'll need separate paths for tris, lines anyways, so maybe do that.
  #if LOGFILLRATE
        auto t0 = TimeTSC();
  #endif
        Fori( u32, j, 0, srcdim.y ) {
          auto dst = app->client.fullscreen_bitmap_argb + ( p0.y + j ) * app->client.dim.x + ( p0.x + 0 );
          auto src = font.tex_mem + ( tc0.y + j ) * font.tex_dim.x + ( tc0.x + 0 );
          Fori( u32, k, 0, srcdim.x ) {
#if 0
            // PERF: this is ~27 cycles / pixel

            u8 dst_a = ( *dst >> 24 ) & 0xFF;
            u8 dst_r = ( *dst >> 16 ) & 0xFF;
            u8 dst_g = ( *dst >>  8 ) & 0xFF;
            u8 dst_b = ( *dst >>  0 ) & 0xFF;
            u8 src_a = ( *src >> 24 ) & 0xFF;
            u8 src_r = ( *src >> 16 ) & 0xFF;
            u8 src_g = ( *src >>  8 ) & 0xFF;
            u8 src_b = ( *src >>  0 ) & 0xFF;
            auto src_factor = color.w * src_a / 255.0f;
            auto dst_factor = 1 - src_factor;
            auto out_a = Cast( u8, Round_u32_from_f32( ( src_a           * src_factor + dst_a * dst_factor ) ) );
            auto out_r = Cast( u8, Round_u32_from_f32( ( src_r * color.x * src_factor + dst_r * dst_factor ) ) );
            auto out_g = Cast( u8, Round_u32_from_f32( ( src_g * color.y * src_factor + dst_g * dst_factor ) ) );
            auto out_b = Cast( u8, Round_u32_from_f32( ( src_b * color.z * src_factor + dst_b * dst_factor ) ) );
            *dst++ = ( out_a << 24 ) | ( out_r << 16 ) | ( out_g << 8 ) | out_b;
            ++src;

#elif 1
            // PERF: this is ~7 cycles / pixel

            // given color in [0, 1]
            // given src, dst in [0, 255]
            // src *= color
            // fac = src_a / 255
            // out = dst + fac * ( src - dst )

            // unpack src, dst
            auto dstf = _mm_set_ss( *Cast( f32*, dst ) );
            auto dsti = _mm_cvtepu8_epi32( _mm_castps_si128( dstf ) );
            auto dst_argb = _mm_cvtepi32_ps( dsti );

            auto srcf = _mm_set_ss( *Cast( f32*, src ) );
            auto srci = _mm_cvtepu8_epi32( _mm_castps_si128( srcf ) );
            auto src_argb = _mm_cvtepi32_ps( srci );

            // src *= color
            src_argb = _mm_mul_ps( src_argb, color_argb );

            // out = dst + src_a * ( src - dst )
            auto fac = _mm_shuffle_ps( src_argb, src_argb, 255 ); // equivalent to set1( m128_f32[3] )
            fac = _mm_mul_ps( fac, _mm_set1_ps( 1.0f / 255.0f ) );
            auto diff_argb = _mm_sub_ps( src_argb, dst_argb );
            auto out_argb = _mm_fmadd_ps( fac, diff_argb, dst_argb );

            // get out in [0, 255]
            auto outi = _mm_cvtps_epi32( out_argb );
            auto out = _mm_packs_epi32( outi, _mm_set1_epi32( 0 ) );
            out = _mm_packus_epi16( out, _mm_set1_epi32( 0 ) );

            _mm_storeu_si32( dst, out ); // equivalent to: *dst = out.m128i_u32[0]
            ++dst;
            ++src;
#endif
          }
        }

  #if LOGFILLRATE
        auto dt = TimeTSC() - t0;
        auto count = srcdim.x * srcdim.y;
        AddMean( stats_texalpha, Cast( f32, dt ) / count, count );
  #endif
      }
    }

  #if LOGFILLRATE
    Log( "quad solid opaque ( cycles / %u px ): %f   total cycles: %f",
      stats_solidopaque.count,
      stats_solidopaque.mean.sum,
      stats_solidopaque.count * stats_solidopaque.mean.sum
      );
    Log( "quad solid alpha ( cycles / %u px ): %f   total cycles: %f",
      stats_solidalpha.count,
      stats_solidalpha.mean.sum,
      stats_solidalpha.count * stats_solidalpha.mean.sum
      );
    Log( "quad textured alpha ( cycles / %u px ): %f   total cycles: %f",
      stats_texalpha.count,
      stats_texalpha.mean.sum,
      stats_texalpha.count * stats_texalpha.mean.sum
      );
  #endif

#endif // !OPENGL_INSTEAD_OF_SOFTWARE

    app->stream.len = 0;
  }
}


#define __AppCmd( name )   void ( name )( app_t* app )
typedef __AppCmd( *pfn_appcmd_t );


__AppCmd( SwitchEditFromCmd ) { app->active = app_active_t::edit; }
__AppCmd( SwitchCmdFromEdit ) { app->active = app_active_t::cmd ; }


__AppCmd( CmdToggleKbCommand )
{
  app->kb_command = !app->kb_command;
}

struct
app_cmdmap_t
{
  glwkeybind_t keybind;
  pfn_appcmd_t fn;
};

Inl app_cmdmap_t
_appcmdmap(
  glwkeybind_t keybind,
  pfn_appcmd_t fn
  )
{
  app_cmdmap_t r;
  r.keybind = keybind;
  r.fn = fn;
  return r;
}


__OnKeyEvent( AppOnKeyEvent )
{
  ProfFunc();

  auto app = Cast( app_t*, misc );

  // Global key event handling:
  bool ran_cmd = 0;
  switch( type ) {
    case glwkeyevent_t::dn:
    case glwkeyevent_t::repeat: {

#if 0
      glwkeybind_t tmp = { glwkey_t::fn_9 };
      if( GlwKeybind( key, alreadydn, tmp ) ) {
        LogUI( "Notification test, %d", glwkey_t::k );
      }
#endif

      app_cmdmap_t table[] = {
        _appcmdmap( GetPropFromDb( glwkeybind_t, keybind_app_toggle_kbcommand ), CmdToggleKbCommand ),
      };
      ForEach( entry, table ) {
        if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
          entry.fn( app );
          target_valid = 0;
          ran_cmd = 1;
        }
      }

      if( GlwKeybind( key, alreadydn, GetPropFromDb( glwkeybind_t, keybind_app_switch_kbcommand_from_kbedit ) ) ) {
        if( !app->kb_command ) {
          app->kb_command = 1;
          target_valid = 0;
          ran_cmd = 1;
        }
      }

      if( GlwKeybind( key, alreadydn, GetPropFromDb( glwkeybind_t, keybind_app_quit ) ) ) {
        GlwEarlyKill( app->client );
        target_valid = 0;
        ran_cmd = 1;
      }
    } break;

    case glwkeyevent_t::up: {
      switch( key ) {
#if 0
        case glwkey_t::esc: {
          GlwEarlyKill( app->client );
          ran_cmd = 1;
        } break;
#endif

        case glwkey_t::fn_11: {
          app->fullscreen = !app->fullscreen;
          fullscreen = app->fullscreen;
          ran_cmd = 1;
        } break;

#if PROF_ENABLED
        case glwkey_t::fn_9: {
          LogUI( "Started profiling." );
          ProfEnable();
          ran_cmd = 1;
        } break;

        case glwkey_t::fn_10: {
          ProfDisable();
          LogUI( "Stopped profiling." );
          ran_cmd = 1;
        } break;
#endif

      }
    } break;

    default: UnreachableCrash();
  }
  if( !ran_cmd ) {
    switch( app->active ) {

      case app_active_t::edit: {
        if( app->kb_command ) {
          switch( type ) {
            case glwkeyevent_t::dn: {
              app_cmdmap_t table[] = {
                _appcmdmap( GetPropFromDb( glwkeybind_t, keybind_app_switch_cmd_from_edit ), SwitchCmdFromEdit ),
              };
              ForEach( entry, table ) {
                if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
                  entry.fn( app );
                  target_valid = 0;
                  ran_cmd = 1;
                }
              }
            } break;

            case glwkeyevent_t::up:
            case glwkeyevent_t::repeat: {
            } break;

            default: UnreachableCrash();
          }
        }
        if( !ran_cmd ) {
          EditControlKeyboard(
            app->edit,
            app->kb_command,
            target_valid,
            ran_cmd,
            type,
            key,
            keylocks,
            alreadydn
            );
        }
      } break;

      case app_active_t::cmd: {
        if( app->kb_command ) {
          switch( type ) {
            case glwkeyevent_t::dn: {
              app_cmdmap_t table[] = {
                _appcmdmap( GetPropFromDb( glwkeybind_t, keybind_app_switch_edit_from_cmd ), SwitchEditFromCmd ),
              };
              ForEach( entry, table ) {
                if( GlwKeybind( key, alreadydn, entry.keybind ) ) {
                  entry.fn( app );
                  target_valid = 0;
                  ran_cmd = 1;
                }
              }
            } break;

            case glwkeyevent_t::up:
            case glwkeyevent_t::repeat: {
            } break;

            default: UnreachableCrash();
          }
        }
        if( !ran_cmd ) {
          CmdControlKeyboard(
            app->cmd,
            app->kb_command,
            target_valid,
            ran_cmd,
            type,
            key,
            keylocks,
            alreadydn
            );
        }
      } break;

      default: UnreachableCrash();
    } // end switch( app->active )
  }
}



__OnMouseEvent( AppOnMouseEvent )
{
  ProfFunc();

  auto app = Cast( app_t*, misc );

  auto& font = GetFont( app, Cast( enum_t, fontid_t::normal ) );

  switch( app->active ) {

    case app_active_t::edit: {
      cursortype = glwcursortype_t::arrow;

      EditControlMouse(
        app->edit,
        target_valid,
        font,
        origin,
        dim,
        type,
        btn,
        alreadydn,
        keyalreadydn,
        m,
        raw_delta,
        dwheel,
        1
        );
    } break;

    case app_active_t::cmd: {
      cursortype = glwcursortype_t::arrow;

      CmdControlMouse(
        app->cmd,
        target_valid,
        font,
        origin,
        dim,
        type,
        btn,
        alreadydn,
        keyalreadydn,
        m,
        raw_delta,
        dwheel,
        1
        );
    } break;

    default: UnreachableCrash();
  }
}


// TODO: move to config file once it has comment support.
#if 0
  static const f32 fontsize_px = 16.0f; // 0.1666666666 i
  static const f32 fontsize_px = 44.0f / 3.0f; // 0.1527777777777 i

    Str( "c:/windows/fonts/droidsansmono.ttf" ),
    Str( "c:/windows/fonts/lucon.ttf" ),
    Str( "c:/windows/fonts/liberationmono-regular.ttf" ),
    Str( "c:/windows/fonts/consola.ttf" ),
    Str( "c:/windows/fonts/ubuntumono-r.ttf" ),
    Str( "c:/windows/fonts/arial.ttf" ),
    Str( "c:/windows/fonts/times.ttf" ),
#endif


__OnWindowEvent( AppOnWindowEvent )
{
  ProfFunc();

  auto app = Cast( app_t*, misc );

  if( type & glwwindowevent_resize ) {
  }
  if( type & glwwindowevent_focuschange ) {
  }
  if( type & glwwindowevent_dpichange ) {
    auto fontsize_normal = GetPropFromDb( f32, f32_fontsize_normal );
    auto filename_font_normal = GetPropFromDb( slice_t, string_filename_font_normal );

    UnloadFont( app, Cast( idx_t, fontid_t::normal ) );
    LoadFont(
      app,
      Cast( idx_t, fontid_t::normal ),
      ML( filename_font_normal ),
      fontsize_normal * Cast( f32, dpi )
      );
  }

  switch( app->active ) {
    case app_active_t::edit: {
      EditWindowEvent(
        app->edit,
        type,
        dim,
        dpi,
        focused,
        target_valid
        );
    } break;

    case app_active_t::cmd: {
    } break;

    default: UnreachableCrash();
  }
}



int
Main( array_t<slice_t>& args )
{
  PinThreadToOneCore();

  auto app = App();
  AppInit( app );

  if( args.len ) {
    auto arg = args.mem + 0;
    auto file = FileOpen( arg->mem, arg->len, fileopen_t::only_existing, fileop_t::R, fileop_t::R );
    if( file.loaded ) {
      EditOpenAndSetActiveTxt( app->edit, file );
      CmdMode_editfile_from_fileopener( app->edit );
    } else {
      u8 tmp[1024];
      CsCopy( tmp, arg->mem, MIN( arg->len, _countof( tmp ) - 1 ) );
      MessageBoxA(
        0,
        Cast( LPCSTR, tmp ),
        "Couldn't open file!",
        0
        );
      return -1;
    }
    FileFree( file );
  }


  auto fontsize_normal = GetPropFromDb( f32, f32_fontsize_normal );
  auto filename_font_normal = GetPropFromDb( slice_t, string_filename_font_normal );

  LoadFont(
    app,
    Cast( idx_t, fontid_t::normal ),
    ML( filename_font_normal ),
    fontsize_normal * Cast( f32, app->client.dpi )
    );

#if OPENGL_INSTEAD_OF_SOFTWARE
  GlwSetSwapInterval( app->client, 0 );
#else // !OPENGL_INSTEAD_OF_SOFTWARE
  // TODO_SOFTWARE_RENDER
#endif // !OPENGL_INSTEAD_OF_SOFTWARE

  glwcallback_t callbacks[] = {
    { glwcallbacktype_t::keyevent           , app, AppOnKeyEvent            },
    { glwcallbacktype_t::mouseevent         , app, AppOnMouseEvent          },
    { glwcallbacktype_t::windowevent        , app, AppOnWindowEvent         },
    { glwcallbacktype_t::render             , app, AppOnRender              },
  };
  ForEach( callback, callbacks ) {
    GlwRegisterCallback( app->client, callback );
  }

  GlwMainLoop( app->client );

  // Do this before destroying any datastructures, so other threads stop trying to access things.
  SignalQuitAndWaitForTaskThreads();

  AppKill( app );

  return 0;
}




Inl void
IgnoreSurroundingSpaces( u8*& a, idx_t& a_len )
{
  while( a_len  &&  IsWhitespace( a[0] ) ) {
    a += 1;
    a_len -= 1;
  }
  // TODO: prog_cmd_line actually has two 0-terms!
  while( a_len  &&  ( !a[a_len - 1]  ||  IsWhitespace( a[a_len - 1] ) ) ) {
    a_len -= 1;
  }
}

int WINAPI
WinMain( HINSTANCE prog_inst, HINSTANCE prog_inst_prev, LPSTR prog_cmd_line, int prog_cmd_show )
{
  MainInit();

  u8* cmdline = Str( prog_cmd_line );
  idx_t cmdline_len = CsLen( Str( prog_cmd_line ) );

  array_t<slice_t> args;
  Alloc( args, 64 );

  while( cmdline_len ) {
    IgnoreSurroundingSpaces( cmdline, cmdline_len );
    if( !cmdline_len ) {
      break;
    }
    auto arg = cmdline;
    idx_t arg_len = 0;
    auto quoted = cmdline[0] == '"';
    if( quoted ) {
      arg = cmdline + 1;
      auto end = CsScanR( arg, cmdline_len - 1, '"' );
      arg_len = !end  ?  0  :  end - arg;
      IgnoreSurroundingSpaces( arg, arg_len );
    } else {
      auto end = CsScanR( arg, cmdline_len - 1, ' ' );
      arg_len = !end  ?  cmdline_len  :  end - arg;
      IgnoreSurroundingSpaces( arg, arg_len );
    }
    if( arg_len ) {
      auto add = AddBack( args );
      add->mem = arg;
      add->len = arg_len;
    }
    cmdline = arg + arg_len;
    cmdline_len -= arg_len;
    if( quoted ) {
      cmdline += 1;
      cmdline_len -= 1;
    }
  }
  auto r = Main( args );
  Free( args );

  Log( "Main returned: %d", r );

  MainKill();
  return r;
}




// OVERVIEW:
#if 0

  text editor
    keyboard type
    mouse cursor/select
    textual undo/redo
    cursor/select undo/redo
    multi-line cursor/select
    textual copy/paste
    smooth scrolling
    find next/prev
    find and replace next/prev

  directory viewer
    see files/dirs in current directory
    delete files/dirs
    create files/dirs
    rename files/dirs

  command prompt
    execute programs from textual commands in current directory.
    change current directory.
    show textual output of program executions.
    copy/paste of textual commands and textual output.

  quick swapping between these modes
    text editor -> directory viewer
    text editor -> command prompt
    directory viewer -> text editor
      open file for editing
    directory viewer -> command prompt
      propogate the current directory
    command prompt -> text editor
      open file for editing
    command prompt -> directory viewer

  hotloaded options
    colors
    keybinds
    scrolling

#endif



// BUG LIST:
#if 0

  TimeSec32() seems to report at multiples of .015625.
    the SecondsFromCycles32( CurrentCycle() ) path also seems to do this.
    this goes away when we use the 64 versions... simple printf issue maybe? or something deeper?

  text clipping is sometimes too aggressive at the bottom of the screen in txt_t
    see cmd, or dir when resizing window.

#endif



// TODO LIST:
#if 0

  !!!!!!!!!NOTE!!!!!!!!!
      SVN REV 530 is the last revision before the content_ptr_t rewrite!
      rollback to that for diffing, if need be.
  !!!!!!!!!NOTE!!!!!!!!!

  figure out why FindFirstR/L are so wicked slow, after the content_ptr_t rewrite.
    i've switched the filecontentsearch back to using cstr, since we don't need the full editing capability there.
    maybe that's the best final option, but we also want incremental search in a txt to be fast.



  software rendering, instead of opengl
    OPENGL_INSTEAD_OF_SOFTWARE
    TODO_SOFTWARE_RENDER

  add a 'currently available commands and keybinds' view, on F1-hold or something.

  add ln_start caching, so we can speed up gotoline, cursorcharu/d, scrolling, etc.
    LNCACHE



  prevent double-opening of a file, by normalizing to full filepath on "te src/main.cpp" usage.
    we can also strip '..'s, '.'s, etc.

  command to copy full path of currently open file.

  hotload config file

  when we move a txtopen across horz, find the mru txtopen on the source side, so we don't have a txtopen dupe.
    maybe tag each mru entry with a 'left or right' flag, so we can do this.
    or, just pick the next mru, no matter which side it's on, and stick it on the source side.

  move clipboard handling to os interface.

  tests for filesys.

  look at the ship asm and fix issues.

  reduce heap allocations per-tick.

    change array_t to delay-alloc until add/reserve
      nuances here around not adding more code to arrays we always add to.
      maybe add a lazyarray_t ?

    change plist_t to use string_t for pages
    change BufLoad to use chunk-size pages in its plist
      we currently try to add a single page the size of the file, which will fail due to virtualalloc_threshold.


    change to keep this array alive longer than just TxtRender?

        array_t<wordspan_t> spans;
        Alloc( spans, 64 );

      proj64d.exe!MemHeapAllocBytes(unsigned __int64 nbytes) Line 1135	C++
      proj64d.exe!Alloc<wordspan_t>(array_t<wordspan_t> & array, unsigned __int64 nelems) Line 39	C++
      proj64d.exe!TxtRender(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 4481	C++
      proj64d.exe!_RenderTxt(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 2677	C++
      proj64d.exe!_RenderBothSides(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 2846	C++
      proj64d.exe!EditRender(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 3311	C++
      proj64d.exe!AppOnRender(void * misc, vec2<float> origin, vec2<float> dim, double timestep_realtime, double timestep_fixed, bool & target_valid) Line 293	C++
      proj64d.exe!_Render(glwclient_t & client) Line 1378	C++
      proj64d.exe!GlwMainLoop(glwclient_t & client) Line 2375	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1198	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++


    change this to use a max-size stack array:

        For( i, 0, c_lines ) {
          auto bol = CursorStopAtNewlineL( txt.buf, uniform_pos, 0 );
          idx_t len;
          auto bol_space = CursorSkipSpacetabR( txt.buf, bol, &len );
          if( len ) {
            auto tmp = AllocContents( txt.buf, bol, len );

      proj64d.exe!AllocContents(buf_t & buf, content_ptr_t ptr, unsigned __int64 len) Line 3213	C++
      proj64d.exe!TxtLoad(txt_t & txt, file_t & file) Line 257	C++
      proj64d.exe!EditOpen(edit_t & edit, file_t & file, edittxtopen_t * * opened, bool * opened_existing) Line 635	C++
      proj64d.exe!EditOpenAndSetActiveTxt(edit_t & edit, file_t & file) Line 652	C++
      proj64d.exe!FileopenerOpenRow(edit_t & edit, fileopener_row_t * row) Line 1110	C++
      proj64d.exe!CmdFileopenerChoose(edit_t & edit, unsigned __int64 misc) Line 1160	C++
      proj64d.exe!ExecuteCmdMap(edit_t & edit, edit_cmdmap_t * table, unsigned __int64 table_len, glwkey_t key, bool * alreadydn, bool & target_valid, bool & ran_cmd) Line 4409	C++
      proj64d.exe!EditControlKeyboard(edit_t & edit, bool kb_command, bool & target_valid, bool & ran_cmd, glwkeyevent_t type, glwkey_t key, glwkeylocks_t & keylocks, bool * alreadydn) Line 4647	C++
      proj64d.exe!AppOnKeyEvent(void * misc, vec2<float> origin, vec2<float> dim, bool & fullscreen, bool & target_valid, glwkeyevent_t type, glwkey_t key, glwkeylocks_t keylocks, bool * alreadydn) Line 980	C++
      proj64d.exe!WindowProc(HWND__ * hwnd, unsigned int msg, unsigned __int64 wp, __int64 lp) Line 1607	C++


        Prof( TxtUpdateScrollingHorizontal );

        auto ln_start = CursorStopAtNewlineL( txt.buf, txt.c, 0 );
        auto temp = AllocContents( txt.buf, ln_start, txt.c );
        auto offset = LayoutString( font, spaces_per_tab, ML( temp ) );

      proj64d.exe!AllocContents(buf_t & buf, content_ptr_t start, content_ptr_t end) Line 3204	C++
      proj64d.exe!TxtUpdateScrolling(txt_t & txt, font_t & font, vec2<float> origin, vec2<float> dim, double timestep_realtime, double timestep_fixed) Line 3854	C++
      proj64d.exe!_RenderTxt(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 2660	C++
      proj64d.exe!_RenderBothSides(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 2846	C++
      proj64d.exe!EditRender(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 3311	C++
      proj64d.exe!AppOnRender(void * misc, vec2<float> origin, vec2<float> dim, double timestep_realtime, double timestep_fixed, bool & target_valid) Line 293	C++
      proj64d.exe!_Render(glwclient_t & client) Line 1378	C++
      proj64d.exe!GlwMainLoop(glwclient_t & client) Line 2375	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1198	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++


        auto word_l = CursorStopAtNonWordCharL( txt.buf, txt.c, 0 );
        auto word_r = CursorStopAtNonWordCharR( txt.buf, txt.c, 0 );
        auto word = AllocContents( txt.buf, word_l, word_r );

      proj64d.exe!AllocContents(buf_t & buf, content_ptr_t start, content_ptr_t end) Line 3204	C++
      proj64d.exe!TxtRender(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 4424	C++
      proj64d.exe!_RenderTxt(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 2677	C++
      proj64d.exe!_RenderBothSides(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 2846	C++
      proj64d.exe!EditRender(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 3311	C++
      proj64d.exe!AppOnRender(void * misc, vec2<float> origin, vec2<float> dim, double timestep_realtime, double timestep_fixed, bool & target_valid) Line 293	C++
      proj64d.exe!_Render(glwclient_t & client) Line 1378	C++
      proj64d.exe!GlwMainLoop(glwclient_t & client) Line 2375	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1198	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++


        auto span = spans.mem + i;
        auto span_len = CountBytesBetween( txt.buf, span->l, span->r );
        AssertWarn( span_len );
        auto tmp = MemHeapAlloc( u8, span_len );
        Contents( txt.buf, span->l, tmp, span_len );

      proj64d.exe!MemHeapAllocBytes(unsigned __int64 nbytes) Line 1135	C++
      proj64d.exe!TxtRender(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 4529	C++
      proj64d.exe!_RenderTxt(txt_t & txt, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed, bool draw_cursor, bool draw_cursorline, bool draw_cursorwordmatch, bool allow_scrollbar) Line 2677	C++
      proj64d.exe!_RenderBothSides(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 2846	C++
      proj64d.exe!EditRender(edit_t & edit, bool & target_valid, array_t<float> & stream, font_t & font, vec2<float> origin, vec2<float> dim, vec2<float> zrange, double timestep_realtime, double timestep_fixed) Line 3311	C++
      proj64d.exe!AppOnRender(void * misc, vec2<float> origin, vec2<float> dim, double timestep_realtime, double timestep_fixed, bool & target_valid) Line 293	C++
      proj64d.exe!_Render(glwclient_t & client) Line 1378	C++
      proj64d.exe!GlwMainLoop(glwclient_t & client) Line 2375	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1198	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++



    hundreds of 192 byte allocations via:
      proj64d.exe!MemHeapAllocBytes(unsigned __int64 nbytes) Line 1140	C++
      proj64d.exe!Alloc<slice_t>(array_t<slice_t> & array, unsigned __int64 nelems) Line 39	C++
      proj64d.exe!Init(statement_t & stm) Line 201	C++
      proj64d.exe!Parse(ast_t & ast, array_t<token_t> & tokens, slice_t & src) Line 271	C++
      proj64d.exe!_LoadFromMem(propdb_t & db, slice_t & mem) Line 340	C++
      proj64d.exe!_Init(propdb_t & db) Line 631	C++
      proj64d.exe!_InitPropdb() Line 672	C++
      proj64d.exe!GetProp(unsigned char * name) Line 680	C++
      proj64d.exe!TxtLoadEmpty(txt_t & txt) Line 199	C++
      proj64d.exe!EditInit(edit_t & edit) Line 1523	C++
      proj64d.exe!AppInit(app_t * app) Line 125	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1148	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++

    hundreds of 164 byte allocations via:
      proj64d.exe!MemHeapAllocBytes(unsigned __int64 nbytes) Line 1136	C++
      proj64d.exe!Alloc<unsigned char>(array_t<unsigned char> & array, unsigned __int64 nelems) Line 39	C++
      proj64d.exe!Init(num_t & num, unsigned __int64 size) Line 1072	C++
      proj64d.exe!CsToFloat32(unsigned char * src, unsigned __int64 src_len, float & dst) Line 1509	C++
      proj64d.exe!_LoadFromMem(propdb_t & db, slice_t & mem) Line 419	C++
      proj64d.exe!_Init(propdb_t & db) Line 631	C++
      proj64d.exe!_InitPropdb() Line 672	C++
      proj64d.exe!GetProp(unsigned char * name) Line 680	C++
      proj64d.exe!TxtLoadEmpty(txt_t & txt) Line 199	C++
      proj64d.exe!EditInit(edit_t & edit) Line 1523	C++
      proj64d.exe!AppInit(app_t * app) Line 125	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1148	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++

    thousands of allocations via stb truetype, e.g.
      proj64d.exe!MemHeapAllocBytes(unsigned __int64 nbytes) Line 1140	C++
      proj64d.exe!stbtt__hheap_alloc(stbtt__hheap * hh, unsigned __int64 size, void * userdata) Line 2281	C++
      proj64d.exe!stbtt__new_active(stbtt__hheap * hh, stbtt__edge * e, int off_x, float start_point, void * userdata) Line 2361	C++
      proj64d.exe!stbtt__rasterize_sorted_edges(stbtt__bitmap * result, stbtt__edge * e, int n, int vsubsample, int off_x, int off_y, void * userdata) Line 2775	C++
      proj64d.exe!stbtt__rasterize(stbtt__bitmap * result, stbtt__point * pts, int * wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert, void * userdata) Line 2971	C++
      proj64d.exe!stbtt_Rasterize(stbtt__bitmap * result, float flatness_in_pixels, stbtt_vertex * vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert, void * userdata) Line 3129	C++
      proj64d.exe!stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo * info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int * width, int * height, int * xoff, int * yoff) Line 3175	C++
      proj64d.exe!stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo * info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int * width, int * height, int * xoff, int * yoff) Line 3211	C++
      proj64d.exe!FontLoadGlyphImage(font_t & font, unsigned int codept, vec2<float> & dimf, vec2<float> & offsetf) Line 500	C++
      proj64d.exe!FontLoadAscii(font_t & font) Line 735	C++
      proj64d.exe!LoadFont(app_t * app, unsigned int fontid, unsigned char * filename_ttf, unsigned __int64 filename_ttf_len, float char_h) Line 215	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1186	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++

      proj64d.exe!MemHeapAllocBytes(unsigned __int64 nbytes) Line 1136	C++
      proj64d.exe!stbtt__GetGlyphShapeTT(const stbtt_fontinfo * info, int glyph_index, stbtt_vertex * * pvertices) Line 1523	C++
      proj64d.exe!stbtt_GetGlyphShape(const stbtt_fontinfo * info, int glyph_index, stbtt_vertex * * pvertices) Line 2127	C++
      proj64d.exe!stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo * info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int * width, int * height, int * xoff, int * yoff) Line 3144	C++
      proj64d.exe!stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo * info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int * width, int * height, int * xoff, int * yoff) Line 3211	C++
      proj64d.exe!FontLoadGlyphImage(font_t & font, unsigned int codept, vec2<float> & dimf, vec2<float> & offsetf) Line 526	C++
      proj64d.exe!FontLoadAscii(font_t & font) Line 735	C++
      proj64d.exe!LoadFont(app_t * app, unsigned int fontid, unsigned char * filename_ttf, unsigned __int64 filename_ttf_len, float char_h) Line 215	C++
      proj64d.exe!Main(array_t<slice_t> & args) Line 1186	C++
      proj64d.exe!WinMain(HINSTANCE__ * prog_inst, HINSTANCE__ * prog_inst_prev, char * prog_cmd_line, int prog_cmd_show) Line 1262	C++




  combine keybinds and cmdmap so we don't have to make all these lists on the stack on every keyevent.
    we can't do this easily if we need to pass in params to the cmd :(

  each font needs its own geometry output stream ( pos + tccolor ), since the texture is different.

  add debug macros that keep track of begin/end call pairs, and check for failure on program exit.



  in fileopener_t, finish adding undo/redo.

  add a progress indicator for dirmode_t::search, since it can take a long time.

  add unsaved indicator to dirmode_t::search view.

  add copyfile + copydir.

  in fileopener_t, add a proper selection set!
    we need a hashset that prevents duplicates.

  in fileopener_t, add Back.
    this requires an UndoStack with units of ( fsobj_t cwd, size_t cursor ).

  in fileopener_t, add sorting and sort states.

  in fileopener_t, add cursor wraparound, given some option is set to 1.

  in fileopener_t, add directory size computation.
    this probably requires a ( fullpathdir, lastwritetime ) cache that we check/update as necessary with a bkgd thread.
    we need a way of queueing a new fullpathdir for the bkgd thread, and a way of getting that result.



  in txt_t, USE_SWAP_FOR_PERMUTELINES
    also need to finish the retvals for Copy, Move, Swap.

  get rid of txt_t cmd*jump*; can just use cmd* with a default argument of 1.

  change txt_t's cs_undo_t strategy to use a history instead of double-stacks.

  in txt_t, move the basic text rendering into buf_t, so we have cleaner separation.
    eliminate the split between single-line and multi-line rendering.

  in txt_t, make CmdFindCursorWordL/R loop around back to start if it doesn't move.

  add mouse control to the btn_pos in txt's scrollbar.
    more complex since we have to allow m to go out of the strict scrollbar region.

  in txt_t, immediately after a dblclick to open a file from fileopener_t, the click+drag selection kicks in.
    we don't have enough state to distinguish these two cases, so we need to store an extra timer that disables click+drag selection after a dblclick was detected. ugh.

  in txt_t, add a px_between_lines option.

  in txt_t, add comment color-highlighting.

  in txt_t, fix SelectCopy so that it doesn't assume buf has CRLF. it has to add explicitly!

  in txt_t, add guidelines at user-defined ch spacings.

  in txt_t, on SaveToFile, just use the EOL type found on loading. so set a flag on Load, and use that same type on SaveToFile.

  in txt_t, have multicursor CmdSelectHome alternate between x=0 and x=x0 thru spaces.

  in txt_t, fix CmdTabL so that it can handle mixed tabs and spaces and do the right thing.
    first pass: verify we can remove 1 tab / N spaces from each ms line.
    second pass: remove 1 tab / N spaces from each ms line.

  in txt_t, consider allowing smooth scrolling towards an undo point.
    right now we store scroll_start/end in cs_undo_t, because we don't update them as concurrents in txt undo/redo.
    perhaps we should, or change them to be stored as cs_undo_absolute_t's



  in cmd_t, make the top_frac line adjustable by mouse.

  in cmd_t, draw old command executions in a darker color, so it's easy to spot the last and current command.



  figure out why we have to SetClipboardData with CF_OEMTEXT, and GetClipboardData with CF_TEXT. other configurations don't seem to work!

  if the program detects a fatal error, save any unsaved work for recovery.

  robust handling of options file paths.
    right now, paths are hardcoded, which is bad.  we need relative paths.
    also, if relative paths don't exist, create them.
    same for the options files--if they don't exist, fill in defaults.
    this means we should just have a options_default/ directory that we can copy from if needed.



  make buf_t + txt_t undo optional.

  autosave!
    do it on window defocus.
    do it on some idle interval.

  generalize fileopener_t so it's a generic table that we can fill with whatever we want.
    not sure that's worth it; we've got some differences between fileopener, switchopened, findinfiles.

  in txt_t, add line wrapping.
    be careful not to split words, unless the word is larger than a line.
    make sure this is a txt_t option.
    make sure the lineno is drawn only once.
    problems:
      mouse input remapping.
      cursor behavior might need to change, ie we might want CursorU to behave like normal even though it's staying inside the same line.
    better yet, have a CmdWrapAllLines, and CmdUnwrapAllLines. then we don't have to worry about changing txt much.
      you can't unwrap without significant contextual information!

  in txt_t, add advanced skip_l, skip_r options.
    camel-case.
    underscores.
    non-alphabet chars ( eg '.', '->', '//', etc ).

  in txt_t, add auto-indenting.
    do }-matching, so it scans back through for a matching { and indents appropriately. have an enable-bool.

  in txt_t, add support for non-ascii files ( utf8, etc ).

  in txt_t, add language-specific text coloring.
    define a keyword with a str. all text matching str ( word boundaries ) is set to a specified color.
    define a text range with str start, str end. all text in range is set to a specified color.
    need some way to specify name mappings like typedef, define, struct, etc.
    also need to make options for coloring the background, not just text.
    change the word to keyword matching to use a hashtable.
      we already do string hashing in hotloader.cpp; reuse that.

  in txt_t, add advanced cursel movement logic for non-monospace
    think about cursel up/down; we'd want to choose a vertical line where the cursel is, and minimize the distance from that line, instead of just a c_inline scheme.
    this also applies to lines with tabs.

#endif
