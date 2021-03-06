/**************************************************************************************************

  FOUNDATION FOR (NON REALTIME) PROGRAMS
The contents of this file belong to the public domain.

.doc:
	...

***************************************************************************************************/



/**************************************************************************************************
  HEADER
***************************************************************************************************/
#if !defined(FOUNDATION_HEADER)
#define FOUNDATION_HEADER

////////////////////////
// CONTEXT CRACKING
#if defined(__clang__)
  #define COMPILER_CLANG 1
  #if defined(_WIN32)
    #define OS_WIN 1
  #elif defined(__gnu_linux__)
    #define OS_LNX 1
  #elif defined(__APPLE__) && defined(__MACH__)
    #define OS_MAC 1
  #else
    #error "No operating system defined in this compiler."
  #endif
  #if defined(__amd64__)
    #define ARCH_X64 1
  #elif defined(__i386__)
    #define ARCH_X86 1
  #elif defined(__arm__)
    #define ARCH_ARM 1
  #elif defined(__aarch64__)
    #define ARCH_ARM64 1
  #else
    #error "No architecture defined in this compiler."
  #endif
#elif defined(__GNUC__)
  #define COMPILER_GCC 1
  #if defined(_WIN32)
    #define OS_WIN 1
  #elif defined(__gnu_linux__)
    #define OS_LNX 1
  #elif defined(__APPLE__) && defined(__MACH__)
    #define OS_MAC 1
  #else
    #error "No operating system defined in this compiler."
  #endif
  #if defined(__amd64__)
    #define ARCH_X64 1
  #elif defined(__i386__)
    #define ARCH_X86 1
  #elif defined(__arm__)
    #define ARCH_ARM 1
  #elif defined(__aarch64__)
    #define ARCH_ARM64 1
  #else
    #error "No architecture defined in this compiler."
  #endif
#elif defined(_MSC_VER)
  #define COMPILER_CL 1
  #if defined(_WIN32)
    #define OS_WIN 1
  #else
    #error "No operating system defined in this compiler."
  #endif
  #if defined(_M_AMD64)
    #define ARCH_X64 1
  #elif defined(_M_I86)
    #define ARCH_X86 1
  #elif defined(_M_ARM)
    #define ARCH_ARM 1
  #else
    #error "No architecture defined in this compiler."
  #endif
#else
  #error "No context defined in this compiler."
#endif

////////////////////////
// SYNTAX SUGAR
#define null  0
#define true  1
#define false 0

#define local    static
#define global   static
#define persist  static
#define function static

#define unused(a)  ((void)a)
#define sizeof(a)  ((int) sizeof(a))
#define countof(a) ((int)(sizeof(a) / sizeof(*(a))))
#define cast(t, x) ((t)(x))
//.note: These were written with a lowecase first letter because they were supposed to be more like actual C keywords.

#define _Glue(x, y) x##y
#define Glue(x, y) _Glue(x, y)
#define ToStr(s) #s
#define UniqueId(Name) Glue(Name, __LINE__)
#define Todo(s)

#define AssertBreak() __debugbreak();
#if defined(ENABLE_ASSERT)
  #define Assert(Expr, ...) if (!(Expr)) AssertBreak();
#else
	#define Assert()
#endif
#define StaticAssert(Expr) typedef char UniqueId(_compile_assert_afray)[(Expr) ? 1 : -1]

#define Member(t,m) (((t*)0)->m)
#define MemberOffset(t,m) IntFromPtr(&Member(t,m))

#define IntFromPtr(p) (u64)((u8*)p - (u8*)0)
#define PtrFromInt(x) (void*)((u8*)0 + (x))

#define Min(x, y)      ((x) <= (y)? (x) : (y))
#define Max(x, y)      ((x) >= (y)? (x) : (y))
#define Clamp(x, v, y) ((v) <  (x)? (x) : (y) < (v)? (y) : (v))
#define ClampNorm(x, v, y) Clamp(0, v, 1)

#define IsPowerOf2(x)     (((x) != 0) && ((x) &  ((x) - 1)) == 0)
#define AlignUpPow2(x, p) (((x) +   (p) - 1)  & ~((p) - 1))
#define AlignDoPow2(x, p)  ((x) & ~((p) - 1))
#define AlignUpPtr(p, a)   ((void *)AlignUpPow2((uintptr_t)(p), (a)))
#define AlignDoPtr(p, a)   ((void *)AlignDoPow2((uintptr_t)(p), (a)))

#define Kb(x) ((x) << 10)
#define Mb(x) ((x) << 20)
#define Gb(x) ((x) << 30)
#define Tb(x) ((x) << 40)

#define Thousand(x) (x * 1000)
#define Million(x)  (x * 1000000)
#define Billion(x)  (x * 1000000000llu)
#define Trillion(x) (x * 1000000000000llu)

#define Kilo(x) Thousand(x)
#define Mega(x) Million(x)
#define Giga(x) Billion(x)
#define Tera(x) Trillion(x)

#define Flag(n) ((1u) << n)

#define ItrNum(i, n)    for (size (i) = 0; (i) < (n); (i) += 1)
#define ItrPtr(i, s, e) for (size (i) = 0; (s) < (e); (i) += 1, (s)++)

////////////////////////
// BASIC TYPES
typedef unsigned long long size;
typedef unsigned char      byte;

typedef unsigned char          u8;
typedef unsigned short         u16;
typedef unsigned int           u32;
typedef unsigned long long int u64;
StaticAssert(sizeof(u8)  == 1);
StaticAssert(sizeof(u16) == 2);
StaticAssert(sizeof(u32) == 4);
StaticAssert(sizeof(u64) == 8);
#define u8Max  0xFF
#define u16Max 0xFFFF
#define u32Max 0xFFFFFFFF
#define u64Max 0xFFFFFFFFFFFFFFFFllu

typedef signed char   i8;
typedef short         i16;
typedef int           i32;
typedef long long int i64;
StaticAssert(sizeof(i8)  == 1);
StaticAssert(sizeof(i16) == 2);
StaticAssert(sizeof(i32) == 4);
StaticAssert(sizeof(i64) == 8);
#define i8Min  (i8 )0x80
#define i16Min (i16)0x8000
#define i32Min (i32)0x80000000
#define i64Min (i64)0x8000000000000000llu
#define i8Max  (i8 )0x7F
#define i16Max (i16)0x7FFF
#define i32Max (i32)0x7FFFFFFF
#define i64Max (i64)0x7FFFFFFFFFFFFFFFllu

typedef char           c8;
typedef unsigned short c16;
typedef unsigned int   c32;
StaticAssert(sizeof(c8)  == 1);
StaticAssert(sizeof(c16) == 2);
StaticAssert(sizeof(c32) == 4);
#define IsSlash(c) ((c) == '\\' || (c) == '/')
#define IsDigit(c) ((c) >= '0' && (c) <= '9')
#define IsUpper(c) ((c) >= 'A' && (c) <= 'Z')
#define IsLower(c) ((c) >= 'a' && (c) <= 'z')
#define ToUpper(c) (IsLower(c)? (c) + 'A'-'a' : c)
#define ToLower(c) (IsUpper(c)? (c) + 'a'-'A' : c)
#define IsAlpha(c) (IsLower(c)  || IsUpper(c))

typedef float  r32;
typedef double r64;
StaticAssert(sizeof(r32) == 4);
StaticAssert(sizeof(r64) == 8);
#define r32Pi    3.14159265359f
#define r32Tau   6.28318530718f
#define r32Euler 2.71828182846f
#define r64Pi    3.14159265359
#define r64Tau   6.28318530718
#define r64Euler 2.71828182846

typedef i8  b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

////////////////////////
// COMPUND TYPES
typedef union _i32v2 {
  struct {
    i32 x, y;
  };
  struct {
    i32 w, h;
  };
	i32 c[2];
} i32v2;
typedef union _r32v2 {
  struct {
    r32 x, y;
  };
  struct {
    r32 w, h;
  };
	r32 c[2];
} r32v2;

typedef union _u64r1 {
  struct  {
    u64 Min, Max;
  };
  struct  {
    u64 First, AfterLast;
  };
  u64 c[2];
} u64r1;
typedef union _r32r1 {
  struct  {
    r32 Min, Max;
  };
  r32 c[2];
} r32r1;
typedef union _i32r2 {
  struct {
    i32v2 Min, Max;
  };
  struct {
    i32v2 p1, p2;
  };
  struct {
    r32 x0, y0, x1, y1;
  };
  i32v2 v[2];
  i32   c[4];
} i32r2;
typedef union _r32r2 {
  struct {
    r32v2 Min, Max;
  };
  struct {
    r32v2 p1, p2;
  };
  struct {
    r32 x0, y0, x1, y1;
  };
  r32v2 v[2];
  r32   c[4];
} r32r2;

////////////////////////
// MATH FUNCTIONS
function r32 r32Trunc(r32);
function r32 r32Floor(r32);
function r32 r32Ceil(r32);

function r32 r32Abs(r32);
function r64 r64Abs(r64);

function r32 r32Sign(r32);
function r64 r64Sign(r64);

function r32 r32Sqrt(r32);
function r64 r64Sqrt(r64);

function r32 r32Log(r32);
function r64 r64Log(r64);

function r32 r32Sin(r32);
function r64 r64Sin(r64);

function r32 r32Cos(r32);
function r64 r64Cos(r64);

function r32 r32Tan(r32);
function r64 r64Tan(r64);

function r32 r32Atan(r32);
function r64 r64Atan(r64);

function r32 r32Pow(r32, r32);
function r64 r64Pow(r64, r64);

inline i32v2 I32v2(i32, i32);
inline i32v2 i32v2Add(i32v2, i32v2);
inline i32v2 i32v2Sub(i32v2, i32v2);

inline r32v2 R32v2(r32, r32);
inline r32v2 r32v2Add(r32v2, r32v2);
inline r32v2 r32v2Sub(r32v2, r32v2);

inline u64r1 U64r1(u64, u64);
inline u64r1 u64r1Add(u64r1, u64r1);
inline u64r1 u64r1Sub(u64r1, u64r1);
inline b32   u64r1Overlaps(u64r1, u64r1);
inline b32   u64r1Contains(u64r1, u64);
inline u64   u64r1Len(u64r1);
inline u64   u64r1Mid(u64r1);

inline r32r1 R32r1(r32, r32);
inline r32r1 r32r1Add(r32r2, r32r2);
inline r32r1 r32r1Sub(r32r2, r32r2);
inline b32   r32r1Overlaps(r32r2, r32r2);
inline b32   r32r1Contains(r32r2, r32);
inline r32   r32r1Len(r32r2);
inline r32   r32r1Mid(r32r2);

inline i32r2 I32r2(i32v2, i32v2);
inline i32r2 i32r2Add(i32r2, i32r2);
inline i32r2 i32r2Sub(i32r2, i32r2);
inline b32   i32r2Overlaps(i32r2, i32r2);
inline b32   i32r2Contains(i32r2, i32);
inline i32   i32r2Len(i32r2);
inline i32   i32r2Mid(i32r2);

inline r32r2 R32r2Add(r32v2, r32v2);
inline r32r2 r32r2Add(r32r2, r32r2);
inline r32r2 r32r2Sub(r32r2, r32r2);
inline b32   r32r2Overlaps(r32r2, r32r2);
inline b32   r32r2Contains(r32r2, r32);
inline r32   r32r2Len(r32r2);
inline r32   r32r2Mid(r32r2);

////////////////////////
// MEMORY
enum _mem_flags {
  mem_Unaccessible = Flag(0),
  mem_Readonly = Flag(1),
  mem_Runnable = Flag(2),
};

typedef struct _pool {
	size Cap;
	size Pos;
} pool;
function pool *PoolReserve  (size);
function void  PoolRelease  (pool*);
function void *PoolPush     (pool*, size);
function void  PoolPopTo    (pool*, size);
function void  PoolPopAmount(pool*, size);

typedef struct _pool_snap {
  pool *Pool;
  size  Pos;
} pool_snap;
function pool_snap GetPoolSnapshot(pool*);
function void      EndPoolSnapshot(pool_snap);

////////////////////////
// STRINGS
typedef struct _str8 {
	c8  *Ptr;
	size Len;
} str8;
typedef struct _str16 {
	c16 *Ptr;
	size Len;
} str16;
typedef struct _str32 {
	c32 *Ptr;
	size Len;
} str32;
inline   str8  Str8      (c8*,  size);
inline   str8  Str8Range (c8*,  c8*);
function str8  Str8Cstr  (c8*);
inline   str16 Str16     (c16*, size);
inline   str16 Str16Range(c16*, c16*);
function str16 Str16Cstr (c16*);
inline   str32 Str32     (c32*, size);
inline   str32 Str32Range(c32*, c32*);
function str32 Str32Cstr (c32*);

typedef struct _utf_char {
	u32  Code;
	size Size;
} utf_char;
function utf_char DecodeUtf8 (c8*,  u32);
function c32      EncodeUtf8 (c8*,  u32);
function utf_char DecodeUtf16(c16*, u32);
function c32      EncodeUtf16(c16*, u32);

function str32 ConvertStr8ToStr32(pool*, str8);
function str8  ConvertStr32ToStr8(pool*, str32);
function str16 ConvertStr8ToStr16(pool*, str8);
function str8  ConvertStr16ToStr8(pool*, str16);

////////////////////////
// DYNAMIC ARRAY
Todo();

////////////////////////
// HASH TABLE
Todo();

////////////////////////
// TIME
typedef enum _month {
  month_Jan,
  month_reb,
  month_Mar,
  month_Apr,
  month_May,
  month_Jun,
  month_Jul,
  month_Aug,
  month_Sep,
  month_Oct,
  month_Nov,
  month_Dec
} month;
typedef enum _week_day {
  week_day_Sun,
  week_day_Mon,
  week_day_Tue,
  week_day_Wed,
  week_day_Thu,
  week_day_Fri,
  week_day_Sat
} week_day;
typedef u64 dense_time;
typedef struct _date_time {
  u16 Msec;  // [0, 999]
  u8  Sec;   // [0, 59]
  u8  Min;   // [0, 59]
  u8  Hour;  // [0, 23]
  u8  Day;   // [1, 31]
  u8  Month; // [1, 12]
  i16 Year;  // 1 = 1CE, 2020 = 2020CE, 0 = 1BCE, -100 = 101BCE, etc.
} date_time;
function dense_time DenseTimeFromDate(date_time*);
function date_time  DateTimeFromDense(dense_time);

////////////////////////
// FILE PROPERTIES
typedef struct _file_properties {
  u32  Flags;
  b32  IsDir;
  size Size;
  dense_time Creation;
  dense_time Modified;
} file_properties;

////////////////////////
// SYSTEM INTERFACE
function byte *SysMemReserve(size, u32);
function void  SysMemRelease(byte*, size);

function u64  SysGetMicroseconds(void); //.note: Does not return 'dense_time'!
function void SysSleep(u32);

function file_properties SysGetFileProps(str8);
function str8 SysOpenFile  (pool*, str8);
function b32  SysSaveFile  (str8,  str8);
function b32  SysDeleteFile(str8);
function b32  SysRenameFile(str8, str8);
function b32  SysCreateDir (str8);
function b32  SysDeleteDir (str8);

function b32 SysInit(i32, c8**);
function void SysEnd(void);

////////////////////////
// SYSTEM WINDOW
typedef struct _window window;
#define WINDOW_COMMON struct { \
  b32 Error;                   \
  b32 Quit;                    \
}
function void SysGetInput(window*);

////////////////////////
// OPENGL
typedef void gl_clear_color_proc(r32, r32, r32, r32);
typedef void gl_clear_proc(u32);
#define WINDOW_OPENGL_COMMON struct { \
  gl_clear_color_proc *GlClearColor;  \
  gl_clear_proc       *GlClear;       \
}
#define GL_COLOR_BUFFER_BIT 0x00004000

function window *SysCreateWindowWithOpenGL(void);
function void    SysBeginRenderingWithOpenGL(window*);
function void    SysEndRenderingWithOpenGL  (window*);

#endif//FOUNDATION_HEADER

/**************************************************************************************************
  PLTFORM INDEPENDENT IMPLEMENTATION
***************************************************************************************************/
#if !defined(FOUNDATION_IMPLEMENTATION)
#define FOUNDATION_IMPLEMENTATION

////////////////////////
// MATH FUNCTIONS
function r32 r32Trunc(r32 x) {
  Todo();
  return 0;
}
function r32 r32Floor(r32 x) {
  Todo();
  return 0;
}
function r32 r32Ceil(r32 x) {
  Todo();
  return 0;
}

function r32 r32Abs(r32 x) {
  Todo();
  return 0;
}
function r64 r64Abs(r64 x) {
  Todo();
  return 0;
}

function r32 r32Sign(r32 x) {
  Todo();
  return 0;
}
function r64 r64Sign(r64 x) {
  Todo();
  return 0;
}

function r32 r32Sqrt(r32 x) {
  Todo();
  return 0;
}
function r64 r64Sqrt(r64 x) {
  Todo();
  return 0;
}

function r32 r32Log(r32 x) {
  Todo();
  return 0;
}
function r64 r64Log(r64 x) {
  Todo();
  return 0;
}

function r32 r32Sin(r32 x) {
  Todo();
  return 0;
}
function r64 r64Sin(r64 x) {
  Todo();
  return 0;
}

function r32 r32Cos(r32 x) {
  Todo();
  return 0;
}
function r64 r64Cos(r64 x) {
  Todo();
  return 0;
}

function r32 r32Tan(r32 x) {
  Todo();
  return 0;
}
function r64 r64Tan(r64 x) {
  Todo();
  return 0;
}

function r32 r32Atan(r32 x) {
  Todo();
  return 0;
}
function r64 r64Atan(r64 x) {
  Todo();
  return 0;
}

function r32 r32Pow(r32 x, r32 p) {
  Todo();
  return 0;
}
function r64 r64Pow(r64 x, r64 p) {
  Todo();
  return 0;
}

inline i32v2 I32v2(i32 x, i32 y) {
  i32v2 Res = {x, y};
  return Res;
}
inline i32v2 i32v2Add(i32v2 a, i32v2 b) {
  i32v2 Res = {a.x + b.x, a.y + b.y};
  return Res;
}
inline i32v2 i32v2Sub(i32v2 a, i32v2 b) {
  i32v2 Res = {a.x - b.x, a.y - b.y};
  return Res;
}

inline r32v2 R32v2(r32 x, r32 y) {
  r32v2 Res = {x, y};
  return Res;
}
inline r32v2 r32v2Add(r32v2 a, r32v2 b) {
  r32v2 Res = {a.x + b.x, a.y + b.y};
  return Res;
}
inline r32v2 r32v2Sub(r32v2 a, r32v2 b) {
  r32v2 Res = {a.x - b.x, a.y - b.y};
  return Res;
}

inline u64r1 U64r1(u64 Min, u64 Max) {
  u64r1 Res = {Min, Max};
  return Res;
}
inline u64r1 u64r1Add(u64r1 r1, u64r1 r2) {
  u64r1 Res = {0};
  Todo();
  return Res;
}
inline u64r1 u64r1Sub(u64r1 r1, u64r1 r2) {
  u64r1 Res = {0};
  Todo();
  return Res;
}
inline b32 u64r1Overlaps(u64r1 r1, u64r1 r2) { return false; }
inline b32 u64r1Contains(u64r1 r, u64 x) { return false; }
inline u64 u64r1Len(u64r1 r) {
  Todo();
  return 0;
}
inline u64 u64r1Mid(u64r1 r) {
  Todo();
  return 0;
}

inline r32r1 R32r1(r32 Min, r32 Max) {
  r32r1 Res = {Min, Max};
  return Res;
}
inline r32r1 r32r1Add(r32r2 r1, r32r2 r2) {
  r32r1 Res = {0};
  Todo();
  return Res;
}
inline r32r1 r32r1Sub(r32r2 r1, r32r2 r2) {
  r32r1 Res = {0};
  Todo();
  return Res;
}
inline b32 r32r1Overlaps(r32r2 r1, r32r2 r2) { return false; }
inline b32 r32r1Contains(r32r2 r, r32 x) { return false; }
inline r32 r32r1Len(r32r2 r) {
  Todo();
  return 0;
}
inline r32 r32r1Mid(r32r2 r) {
  Todo();
  return 0;
}

inline i32r2 I32r2(i32v2 Min, i32v2 Max) {
  i32r2 Res = {Min, Max};
  return Res;
}
inline i32r2 i32r2Add(i32r2 r1, i32r2 r2) {
  i32r2 Res = {0};
  Todo();
  return Res;
}
inline i32r2 i32r2Sub(i32r2 r1, i32r2 r2) {
  i32r2 Res = {0};
  Todo();
  return Res;
}
inline b32 i32r2Overlaps(i32r2 r1, i32r2 r2) { return false; }
inline b32 i32r2Contains(i32r2 r, i32 x) { return false; }
inline i32 i32r2Len(i32r2 r) {
  Todo();
  return 0;
}
inline i32 i32r2Mid(i32r2 r) {
  Todo();
  return 0;
}

inline r32r2 R32r2(r32v2 Min, r32v2 Max) {
  r32r2 Res = {Min, Max};
  return Res;
}
inline r32r2 r32r2Add(r32r2 r1, r32r2 r2) {
  r32r2 Res = {0};
  Todo();
  return Res;
}
inline r32r2 r32r2Sub(r32r2 r1, r32r2 r2) {
  r32r2 Res = {0};
  Todo();
  return Res;
}
inline b32 r32r2Overlaps(r32r2 r1, r32r2 r2) { return false; }
inline b32 r32r2Contains(r32r2 r, r32 x) { return false; }
inline r32 r32r2Len(r32r2 r) {
  Todo();
  return 0;
}
inline r32 r32r2Mid(r32r2 r) {
  Todo();
  return 0;
}

////////////////////////
// MEMORY
function pool *PoolReserve(size Cap) {
  if (Cap == 0)
    Cap = Gb(1);
  byte *Mem = SysMemReserve(Cap, 0);
  pool *Res = cast(pool*, Mem);
  Res->Cap = Cap;
  Res->Pos = AlignUpPow2(sizeof(pool), 64);
  Assert(Res != null, "Failed to create pool.");
  return Res;
}
function void PoolRelease(pool *Pool) {
  SysMemRelease(cast(byte*, Pool), Pool->Cap);
}
function void *PoolPush(pool *Pool, size Size) {
  void *Res = null;
	if (Pool->Pos + Size <= Pool->Cap) {
		Res = cast(byte*, Pool) + Pool->Pos;
    Pool->Pos += Size;
	}
  Assert(Res != null, "Failed to push to pool.");
  return Res;
}
function void PoolPopTo(pool *Pool, size Pos) {
  if (Pos < Pool->Pos)
    Pool->Pos = Pos;
}
function void PoolPopAmount(pool *Pool, size Amount) {
  if (Amount < Pool->Pos)
    PoolPopTo(Pool, Pool->Pos - Amount);
}

function pool_snap GetPoolSnapshot(pool *Pool) {
  pool_snap Res = {Pool, Pool->Pos};
  return Res;
}
function void EndPoolSnapshot(pool_snap Snap) {
  PoolPopTo(Snap.Pool, Snap.Pos);
}

////////////////////////
// DYNAMIC ARRAY
Todo();

////////////////////////
// HASH TABLE
Todo();

////////////////////////
// STRINGS
inline str8 Str8(c8 *Ptr, size Len) {
	str8 Res = {Ptr, Len};
	return Res;
}
inline str8 Str8Range(c8 *Start, c8 *End) {
	str8 Res = {Start, (size)(End - Start)};
	return Res;
}
function str8 Str8Cstr(c8 *Str) {
  c8 *Ptr = Str;
  while (*Ptr != '\0')
    Ptr++;
	str8 Res = {Str, (size)(Ptr - Str)};
	return Res;
}
inline str16 Str16(c16 *Ptr, size Len) {
	str16 Res = {Ptr, Len};
	return Res;
}
inline str16 Str16Range(c16 *Start, c16 *End) {
	str16 Res = {Start, (size)(End - Start)};
	return Res;
}
function str16 Str16Cstr(c16 *Str) {
  c16 *Ptr = Str;
  while (*Ptr != 0)
    Ptr++;
	str16 Res = {Str, (size)(Ptr - Str)};
	return Res;
}
inline str32 Str32(c32 *Ptr, size Len) {
	str32 Res = {Ptr, Len};
	return Res;
}
inline str32 Str32Range(c32 *Start, c32 *End) {
	str32 Res = {Start, (size)(End - Start)};
	return Res;
}
function str32 Str32Cstr(c32 *Str) {
  c32 *Ptr = Str;
  while (*Ptr != 0)
    Ptr++;
	str32 Res = {Str, (size)(Ptr - Str)};
	return Res;
}

function utf_char DecodeUtf8(c8 *Ptr, u32 Len) {
	local u8 CodepointLen[] = {
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		0, 0, 0, 0,
		0, 0, 0, 0,
		2, 2, 2, 2,
		3, 3,
		4,
		0
	};
	local u8 FirstByteMask[] = {0, 0x7F, 0x1F, 0x0F, 0x07};
	local u8 FinalShift[] = {0, 18, 12, 6, 0};
	utf_char Res = {0};
	if (Len > 0) {
		Res.Code = '#';
		Res.Size = 1;
		byte Byte = Ptr[0];
		u8 Size = CodepointLen[Byte >> 3];
		if (0 < Size && Size <= Len) {
			size Codepoint = (Byte & FirstByteMask[Size]) << 18;
			switch (Size) {
				case 4: Codepoint |= ((Ptr[3] & 0x3F) << 0);
				case 3: Codepoint |= ((Ptr[2] & 0x3F) << 6);
				case 2: Codepoint |= ((Ptr[1] & 0x3F) << 12);
				default: break;
			}
      Codepoint >>= FinalShift[Size];
			Res.Code    = Codepoint;
			Res.Size = Size;
		}
	}
	return Res;
}
function c32 EncodeUtf8(c8 *Dst, u32 Codepoint) {
	size Size = 0;
	if (Codepoint < (1 << 8)) {
		Dst[0] = Codepoint;
		Size = 1;
	}
	else
	if (Codepoint < (1 << 11)) {
		Dst[0] = 0xC0 | (Codepoint >> 6);
		Dst[1] = 0x80 | (Codepoint & 0x3F);
		Size = 2;
	}
	else
	if (Codepoint < (1 << 16)) {
		Dst[0] = 0xE0 |  (Codepoint >> 12);
		Dst[1] = 0x80 | ((Codepoint >> 6) & 0x3F);
		Dst[2] = 0x80 |  (Codepoint       & 0x3F);
		Size = 3;
	}
	else
	if (Codepoint < (1 << 21)) {
		Dst[0] = 0xF0 |  (Codepoint >> 18);
		Dst[1] = 0x80 | ((Codepoint >> 12) & 0x3F);
		Dst[2] = 0x80 | ((Codepoint >> 6)  & 0x3F);
		Dst[3] = 0x80 |  (Codepoint        & 0x3F);
		Size = 4;
	}
	else {
		Dst[0] = '#';
		Size = 1;
	}
	return Size;
}
function utf_char DecodeUtf16(c16 *Ptr, u32 Size) {
	utf_char Res = {'#', 1};
	u16 c1 = Ptr[0];
	if (c1 < 0xD800 || 0xDFFF < c1)
		Res.Code = c1;
	else {
		u16 c2 = Ptr[1];
		if (0xD800 <= c1 && c1 < 0xDC00 &&
		    0xDC00 <= c2 && c2 < 0xE000) {
			Res.Code = (((c1 - 0xDC00) << 10) | (c2 - 0xDC00)) + 0x10000,
			Res.Size = 2;
		}
	}
	return Res;
}
function c32 EncodeUtf16(c16 *Dst, u32 Codepoint) {
	size Len = 0;
	if (Codepoint < 0x10000) {
		Dst[0] = Codepoint;
		Len = 1;
	}
	else {
		size c = Codepoint - 0x10000;
		Dst[0] = (c >> 10)   + 0xD800;
		Dst[1] = (c & 0x3FF) + 0xDC00;
		Len = 2;
	}
	return Len;
}

function str32 ConvertStr8ToStr32(pool *Pool, str8 Str) {
  str32 Res = {0};
  Todo();
  return Res;
}
function str8 ConvertStr32ToStr8(pool *Pool, str32 Str) {
  str8 Res = {0};
  Todo();
  return Res;
}
function str16 ConvertStr8ToStr16(pool *Pool, str8 Str) {
  str16 Res = {0};
  Todo();
  return Res;
}
function str8 ConvertStr16ToStr8(pool *Pool, str16 Str) {
  str8 Res = {0};
  Todo();
  return Res;
}

////////////////////////
// TIME
function dense_time DenseTimeFromDate(date_time *Date) {
  u32 EncodedYear = (u32)((i32)Date->Year + 0x8000);
  dense_time Res = 0;
  Res += EncodedYear;
  Res *= 12;
  Res += (Date->Month - 1);
  Res *= 31;
  Res += Date->Day;
  Res *= 24;
  Res += Date->Hour;
  Res *= 60;
  Res += Date->Min;
  Res *= 61;
  Res += Date->Sec;
  Res *= 1000;
  Res += Date->Msec;
  return Res;
}
function date_time DateTimeFromDense(dense_time Dense) {
  date_time Res = {0};
  Res.Msec = Dense%1000;
  Dense /= 1000;
  Res.Sec = Dense%61;
  Dense /= 61;
  Res.Min = Dense%60;
  Dense /= 60;
  Res.Hour = Dense%24;
  Dense /= 24;
  Res.Day = Dense%31;
  Dense /= 31;
  Res.Month = (Dense%12) + 1;
  Dense /= 12;
  Res.Year = ((i32)(Dense) - 0x8000);
  return Res;
}

////////////////////////
// SYSTEM INTERFACE

#endif//FOUNDATION_IMPLEMENTATION

/**************************************************************************************************
  PLATFORM DEPENDENT IMPLEMENTATIONS
***************************************************************************************************/

#if defined(OS_WIN)

#undef function
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "timeapi.h"
#define function static

#include <stdio.h>

global pool *GlobalWin32Pool;
global str8 *GlobalWin32Argv;
global i32   GlobalWin32Argc;
global u64   GlobalWin32TicksPerSecond;
global u64   GlobalWin32TicksPerSecond;
global u64   GlobalWin32TicksUponStart;

////////////////////////
// MEMORY
function byte *SysMemReserve(size Size, u32 Flags) {
  u32 ProtectionFlags = 0;
  if (Flags & mem_Unaccessible)
    ProtectionFlags |= PAGE_NOACCESS;
  if (Flags & mem_Readonly)
    ProtectionFlags |= PAGE_READONLY;
  if (Flags & mem_Runnable)
    ProtectionFlags |= PAGE_EXECUTE;
  if (!(ProtectionFlags & (PAGE_READONLY|PAGE_NOACCESS)))
    ProtectionFlags |= PAGE_READWRITE;
  return cast(byte*, VirtualAlloc(null, Size, MEM_RESERVE | MEM_COMMIT, ProtectionFlags));
}
function void SysMemRelease(byte *Ptr, size Size) {
  VirtualFree(Ptr, Size, MEM_RELEASE);
}

////////////////////////
// TIME
function u64 SysGetMicroseconds() {
  LARGE_INTEGER PerfCounter = {0};
  if (QueryPerformanceCounter(&PerfCounter))
    return PerfCounter.QuadPart*Million(1)/GlobalWin32TicksPerSecond;
  return -1;
}
function void SysSleep(u32 Milliseconds) {
  Sleep(Milliseconds);
}

////////////////////////
// FILES
function file_properties SysGetFileProps(str8 Path) {
  file_properties Res = {0};

  pool_snap Snap      = GetPoolSnapshot(GlobalWin32Pool);
  str16     PathUtf16 = ConvertStr8ToStr16(Snap.Pool, Path);

  WIN32_FILE_ATTRIBUTE_DATA Attributes = {0};
  if (GetFileAttributesExW(cast(WCHAR*, PathUtf16.Ptr), GetFileExInfoStandard, &Attributes)) {
    Todo();
  }
  return Res;
}

function str8 SysOpenFile(pool *Pool, str8 Path) {
  str8 Res = {};

  pool_snap Snap      = GetPoolSnapshot(GlobalWin32Pool);
  str16     PathUtf16 = ConvertStr8ToStr16(Snap.Pool, Path);

  HANDLE File = CreateFileW(cast(WCHAR*, PathUtf16.Ptr), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (File != INVALID_HANDLE_VALUE) {
    DWORD HiSize    = 0;
    DWORD LoSize    = GetFileSize(File, &HiSize);
    u64   TotalSize = ((cast(u64, HiSize)) << 32) | cast(u64, LoSize);

    pool_snap RestorePoint = GetPoolSnapshot(Pool);
    byte     *Buffer       = cast(byte*, PoolPush(Pool, sizeof(byte)*TotalSize));

    byte *Ptr         = Buffer;
    byte *OnePastLast = Buffer + TotalSize;
    b32   Success     = true;
    while (Ptr < OnePastLast) {
      DWORD ToRead = cast(DWORD, OnePastLast - Ptr);
      if (ToRead > u32Max)
        ToRead = u32Max;

      DWORD ActuallyRead = 0;
      if (!ReadFile(File, Ptr, ToRead, &ActuallyRead, 0)) {
        Success = false;
        break;
      }
      Ptr += ActuallyRead;
    }
  
    if (Success) {
      Res.Ptr = cast(c8*, Buffer);
      Res.Len = TotalSize;
    }
    else
      EndPoolSnapshot(RestorePoint);
    CloseHandle(File);
  }

  EndPoolSnapshot(Snap);
  return Res;
}
function b32 SysSaveFile(str8 Path, str8 Data) {
  Todo();
  return false;
}
function b32 SysDeleteFile(str8 Path) {
  pool_snap Snap      = GetPoolSnapshot(GlobalWin32Pool);
  str16     PathUtf16 = ConvertStr8ToStr16(Snap.Pool, Path);
  b32       Success   = DeleteFileW(cast(WCHAR*, PathUtf16.Ptr));
  EndPoolSnapshot(Snap);
  return Success;
}
function b32 SysRenameFile(str8 Old, str8 New) {
  pool_snap Snap     = GetPoolSnapshot(GlobalWin32Pool);
  str16     OldUtf16 = ConvertStr8ToStr16(Snap.Pool, Old);
  str16     NewUtf16 = ConvertStr8ToStr16(Snap.Pool, New);
  b32       Success  = MoveFileW(cast(WCHAR*, OldUtf16.Ptr), cast(WCHAR*, NewUtf16.Ptr));
  EndPoolSnapshot(Snap);
  return Success;
}
function b32 SysCreateDir(str8 Path) {
  pool_snap Snap      = GetPoolSnapshot(GlobalWin32Pool);
  str16     PathUtf16 = ConvertStr8ToStr16(Snap.Pool, Path);
  b32       Success   = CreateDirectoryW(cast(WCHAR*, PathUtf16.Ptr), 0);
  EndPoolSnapshot(Snap);
  return Success;
}
function b32 SysDeleteDir(str8 Path) {
  pool_snap Snap      = GetPoolSnapshot(GlobalWin32Pool);
  str16     PathUtf16 = ConvertStr8ToStr16(Snap.Pool, Path);
  b32       Success   = RemoveDirectoryW(cast(WCHAR*, PathUtf16.Ptr));
  EndPoolSnapshot(Snap);
  return Success;
}

////////////////////////
// SETUP
function b32 SysInit(i32 Argc, c8 **Argv) {
  pool *Pool = PoolReserve(0);

  LARGE_INTEGER PerfFrequency = {0};
  if (!QueryPerformanceFrequency(&PerfFrequency)) {
    fprintf(stderr, "Could not retrieve performance frequency.");
    return false;
  }
  timeBeginPeriod(1);
  LARGE_INTEGER PerfCounter = {0};
  if (!QueryPerformanceCounter(&PerfCounter)) {
    fprintf(stderr, "Could not retrieve performance counter.");
    return false;
  }
  
  GlobalWin32Argc           = Argc;
  GlobalWin32Argv           = cast(str8*, PoolPush(Pool, Argc*sizeof(str8)));
  ItrNum (i, Argc)
    GlobalWin32Argv[i]      = Str8Cstr(cast(u8*, Argv[i]));
  GlobalWin32TicksPerSecond = PerfFrequency.QuadPart;
  GlobalWin32TicksUponStart = PerfCounter.QuadPart;
  GlobalWin32Pool           = Pool;

  return true;
}

function void SysEnd() {
  PoolRelease(GlobalWin32Pool);
}

// GRAPHICS
#define WGL_ARB_pixel_format 1
#define WGL_NUMBER_PIXEL_FORMATS_ARB      0x2000
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_DRAW_TO_BITMAP_ARB            0x2002
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_NEED_PALETTE_ARB              0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB       0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB        0x2006
#define WGL_SWAP_METHOD_ARB               0x2007
#define WGL_NUMBER_OVERLAYS_ARB           0x2008
#define WGL_NUMBER_UNDERLAYS_ARB          0x2009
#define WGL_TRANSPARENT_ARB               0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB     0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB   0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB    0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB   0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB   0x203B
#define WGL_SHARE_DEPTH_ARB               0x200C
#define WGL_SHARE_STENCIL_ARB             0x200D
#define WGL_SHARE_ACCUM_ARB               0x200E
#define WGL_SUPPORT_GDI_ARB               0x200F
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_STEREO_ARB                    0x2012
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_RED_BITS_ARB                  0x2015
#define WGL_RED_SHIFT_ARB                 0x2016
#define WGL_GREEN_BITS_ARB                0x2017
#define WGL_GREEN_SHIFT_ARB               0x2018
#define WGL_BLUE_BITS_ARB                 0x2019
#define WGL_BLUE_SHIFT_ARB                0x201A
#define WGL_ALPHA_BITS_ARB                0x201B
#define WGL_ALPHA_SHIFT_ARB               0x201C
#define WGL_ACCUM_BITS_ARB                0x201D
#define WGL_ACCUM_RED_BITS_ARB            0x201E
#define WGL_ACCUM_GREEN_BITS_ARB          0x201F
#define WGL_ACCUM_BLUE_BITS_ARB           0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB          0x2021
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023
#define WGL_AUX_BUFFERS_ARB               0x2024
#define WGL_NO_ACCELERATION_ARB           0x2025
#define WGL_GENERIC_ACCELERATION_ARB      0x2026
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_SWAP_EXCHANGE_ARB             0x2028
#define WGL_SWAP_COPY_ARB                 0x2029
#define WGL_SWAP_UNDEFINED_ARB            0x202A
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_TYPE_COLORINDEX_ARB           0x202C

#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB       0x2093
#define WGL_CONTEXT_FLAGS_ARB             0x2094

#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef HGLRC wgl_create_context_proc(HDC);
typedef BOOL  wgl_delete_context_proc(HGLRC);
typedef BOOL  wgl_make_current_proc(HDC, HGLRC);
typedef PROC  wgl_get_proc_address_proc(LPCSTR);

typedef BOOL  wgl_choose_pixel_format_arb_proc(HDC, const int*, const float*, UINT, int*, UINT*);
typedef HGLRC wgl_create_context_attribs_arb_proc(HDC, HGLRC, const int*);

struct _window {
  WINDOW_COMMON;
  WINDOW_OPENGL_COMMON;

  wgl_make_current_proc *WglMakeCurrent;

  HWND  WindowHandle;
  HDC   DeviceContext;
  HGLRC OpenglContext;

  void *MainFiber;
  void *MessageFiber;
};

LRESULT CALLBACK WindowProc(HWND WindowHandle, UINT Message, WPARAM wParam, LPARAM lParam) {
  LRESULT Result = 0;
  window *Window = cast(window*, GetWindowLongPtr(WindowHandle, GWLP_USERDATA));
  switch (Message) {
    case WM_DESTROY:
      Window->Quit = true;
      break;
    case WM_TIMER:
      SwitchToFiber(Window->MainFiber);
      break;
    case WM_ENTERMENULOOP:
    case WM_ENTERSIZEMOVE:
      SetTimer(WindowHandle, 1, 1, 0);
      break;
    case WM_EXITMENULOOP:
    case WM_EXITSIZEMOVE:
      KillTimer(WindowHandle, 1);
      break;
    default:
      Result = DefWindowProcW(WindowHandle, Message, wParam, lParam);
  }
  return Result;
}
function void CALLBACK MessageFiberProc(void *MainFiber) {
  while (true) {
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&Message);
      DispatchMessage(&Message);
    }
    SwitchToFiber(MainFiber);
  }
}

function void SysGetInput(window *Window) {
  SwitchToFiber(Window->MessageFiber);
}
function window *SysCreateWindowWithOpenGL(void) {
  window *Window = PoolPush(GlobalWin32Pool, sizeof(window));
  ZeroMemory(Window, sizeof(*Window));
  Window->MainFiber    = ConvertThreadToFiber(0);
  Window->MessageFiber = CreateFiber(0, (PFIBER_START_ROUTINE)MessageFiberProc, Window->MainFiber);
  Assert(Window->MainFiber && Window->MessageFiber);

  HINSTANCE Instance = GetModuleHandleW(null);
  HMODULE OpenglModule = LoadLibraryA("opengl32.dll");
  if (OpenglModule == 0) {
    fprintf(stderr, "Could not load opengl module.");
    Window->Error = true;
    return Window;
  }
  wgl_create_context_proc   *WglCreateContextProc  = cast(wgl_create_context_proc*,   GetProcAddress(OpenglModule, "wglCreateContext"));
  wgl_delete_context_proc   *WglDeleteContextProc  = cast(wgl_delete_context_proc*,   GetProcAddress(OpenglModule, "wglDeleteContext"));
  wgl_make_current_proc     *WglMakeCurrentProc    = cast(wgl_make_current_proc*,     GetProcAddress(OpenglModule, "wglMakeCurrent"));
  wgl_get_proc_address_proc *WglGetProcAddressProc = cast(wgl_get_proc_address_proc*, GetProcAddress(OpenglModule, "wglGetProcAddress"));
  if (!WglCreateContextProc || !WglMakeCurrentProc || !WglGetProcAddressProc) {
    fprintf(stderr, "Could not load opengl module functions.");
    Window->Error = true;
    return Window;
  }

  // Creating dummy class and window so that we can load modern opengl stuff.
  WNDCLASSW DummyClass = {0};
  DummyClass.lpfnWndProc   = DefWindowProcW;
  DummyClass.hInstance     = Instance;
  DummyClass.lpszClassName = L"dummy class";
  if (!RegisterClassW(&DummyClass)) {
    fprintf(stderr, "Could not create window class.");
    Window->Error = true;
    return Window;
  }
  HWND DummyWindowHandle = CreateWindowW(L"dummy class", L"dummy title", WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
  if (!DummyWindowHandle) {
    fprintf(stderr, "Could not create dummy window for loading opengl.");
    Window->Error = true;
    return Window;
  }
  HDC DummyDeviceContext = GetDC(DummyWindowHandle);
  PIXELFORMATDESCRIPTOR DummyPixelFormatDescriptor = {0};
  DummyPixelFormatDescriptor.nSize      = sizeof(DummyPixelFormatDescriptor);
  DummyPixelFormatDescriptor.nVersion   = 1;
  DummyPixelFormatDescriptor.dwFlags    = PFD_SUPPORT_OPENGL;
  DummyPixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
  DummyPixelFormatDescriptor.cColorBits = 24;
  INT DummyPixelFormatIdx = ChoosePixelFormat(DummyDeviceContext, &DummyPixelFormatDescriptor);
  if (!DummyPixelFormatIdx) {
    fprintf(stderr, "Could not choose dummy pixel format.");
    ReleaseDC(DummyWindowHandle, DummyDeviceContext);
    Window->Error = true;
    return Window;
  }
  if (!SetPixelFormat(DummyDeviceContext, DummyPixelFormatIdx, &DummyPixelFormatDescriptor)) {
    fprintf(stderr, "Could not set dummy pixel format.");
    ReleaseDC(DummyWindowHandle, DummyDeviceContext);
    Window->Error = true;
    return Window;
  }

  HGLRC DummyOpenglContext = WglCreateContextProc(DummyDeviceContext);
  if (!DummyOpenglContext) {
    fprintf(stderr, "Could not create dummy opengl context.");
    ReleaseDC(DummyWindowHandle, DummyDeviceContext);
    Window->Error = true;
    return Window;
  }

  WglMakeCurrentProc(DummyDeviceContext, DummyOpenglContext);
  wgl_choose_pixel_format_arb_proc    *WglChoosePixelFormatARBProc    = cast(wgl_choose_pixel_format_arb_proc*,    WglGetProcAddressProc("wglChoosePixelFormatARB"));
  wgl_create_context_attribs_arb_proc *WglCreateContextAttribsARBProc = cast(wgl_create_context_attribs_arb_proc*, WglGetProcAddressProc("wglCreateContextAttribsARB"));
  if (!WglChoosePixelFormatARBProc || !WglCreateContextAttribsARBProc) {
    fprintf(stderr, "Could not load opengl functions.");
    ReleaseDC(DummyWindowHandle, DummyDeviceContext);
    Window->Error = true;
    return Window;
  }

  WglMakeCurrentProc(null, null);
  ReleaseDC(DummyWindowHandle, DummyDeviceContext);
  Assert(WglDeleteContextProc(DummyOpenglContext));
  Assert(DestroyWindow(DummyWindowHandle));

  // Now the actual window.
  WNDCLASSW Class = {0};
  Class.lpfnWndProc   = WindowProc;
  Class.hInstance     = Instance;
  Class.hCursor       = LoadCursor(0, IDC_ARROW);
  Class.lpszClassName = L"actual class";
  if (!RegisterClassW(&Class)) {
    fprintf(stderr, "Could not create window class.");
    Window->Error = true;
    return Window;
  }
  Window->WindowHandle = CreateWindowW(L"actual class", L"title", WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
  if (!Window->WindowHandle) {
    fprintf(stderr, "Could not create window.");
    Window->Error = true;
    return Window;
  }
  SetWindowLongPtr(Window->WindowHandle, GWLP_USERDATA, (LONG_PTR)Window);

  Window->DeviceContext = GetDC(Window->WindowHandle);
  INT FormatAttribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, true,
    WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
    WGL_SWAP_METHOD_ARB,    WGL_SWAP_EXCHANGE_ARB,
    WGL_SUPPORT_OPENGL_ARB, true,
    WGL_DOUBLE_BUFFER_ARB,  true,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,     24,
    WGL_RED_BITS_ARB,       8,
    WGL_GREEN_BITS_ARB,     8,
    WGL_BLUE_BITS_ARB,      8,
    0,
  };
  INT  PixelFormatIdx  = 0;
  UINT NumberOfFormats = 0;
  if(!WglChoosePixelFormatARBProc(Window->DeviceContext, FormatAttribs, null, 1, &PixelFormatIdx, &NumberOfFormats) || NumberOfFormats == 0) {
    fprintf(stderr, "Could not choose pixel format.");
    ReleaseDC(Window->WindowHandle, Window->DeviceContext);
    Window->Error = true;
    return Window;
  }
  fprintf(stdout, "Pixel format: %d.\n", PixelFormatIdx);

  PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {0};
  if (!SetPixelFormat(Window->DeviceContext, PixelFormatIdx, &PixelFormatDescriptor)) {
    fprintf(stderr, "Could not set pixel format.");
    ReleaseDC(Window->WindowHandle, Window->DeviceContext);
    Window->Error = true;
    return Window;
  }

  INT ContextAttribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };
  Window->OpenglContext = WglCreateContextAttribsARBProc(Window->DeviceContext, 0, ContextAttribs);
  if (!Window->OpenglContext) {
    fprintf(stderr, "Could not create opengl context.");
    ReleaseDC(Window->WindowHandle, Window->DeviceContext);
    Window->Error = true;
    return Window;
  }

  Window->GlClearColor = cast(gl_clear_color_proc*, GetProcAddress(OpenglModule, "glClearColor"));
  Window->GlClear      = cast(gl_clear_proc*,       GetProcAddress(OpenglModule, "glClear"));
  if (!Window->GlClearColor || !Window->GlClear) {
    fprintf(stderr, "Could not load opengl procedures.");
    Window->Error = true;
    return Window;
  }

  ShowWindow(Window->WindowHandle, SW_SHOW);

  Window->WglMakeCurrent = WglMakeCurrentProc;

  if (Window->Error == true)
    ExitProcess(1);

  return Window;
}
function void SysBeginRenderingWithOpengl(window *Window) {
  Window->DeviceContext = GetDC(Window->WindowHandle);
  Window->WglMakeCurrent(Window->DeviceContext, Window->OpenglContext);
}
function void SysEndRenderingWithOpengl(window *Window) {
  SwapBuffers(Window->DeviceContext);
  ReleaseDC(Window->WindowHandle, Window->DeviceContext);
}

#elif defined(OS_LNX)

Todo();

#elif defined(OS_MAC)

Todo();

#endif