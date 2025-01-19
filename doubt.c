///// ---------- ---------- ---------- PREAMBLE ---------- ---------- ---------- /////

/// @brief Doubt: A Programming Language
/// @author Michael Burgess <doubt at $myname co.uk>
/// @note This is a work-in-progress, and not yet ready for use.
/// @note This is a snapshot of some initial work developing 
//          a second-order probabilistic programming language called Doubt.


/// #region Documentation
/// @section Probabilistic Programming Language: Interpreting Doubt 
/// @subsection Introduction: What is a PPL?
/// A probabilistic programming language (PPL) is a programming language that is designed to
///  represent and manipulate probabilistic models. These models are used to describe the
///  uncertainty in data and the relationships between variables. PPLs are used in a wide range
///  of applications, including machine learning, artificial intelligence, and statistics.
///
/// #endregion

/// #region Preamble
/// cspell:ignore MCMC repr
#pragma region ShimC23H

/// @note Defines to make the code more portable across c23/earlier versions
/// eg., vs code does not support c23 yet, so we need to define some things\
///     that are not yet supported in c23, but are in c99/11/17
///     eg., nullptr, static_assert, alignas, alignof, typeof

#ifndef COMPILING
    #define constexpr const 
    #define alignas _Alignas
    #define alignof _Alignof
    #define typeof __typeof__
    #define static_assert _Static_assert
    #define nullptr NULL

#endif

#pragma endregion

#pragma region DoubtPreambleH

/// This file is here to preview some initial work developing
/// a second-order probabilistic programming language.
/// This is not the development version (which is local, and not released)
///... but a quick snapshot of some progress to provide some insight into the code.


/// Compile with:
/// clang -std=c23 -DTEST -o doubt_test doubt.c
/// cf. ./compile.sh

constexpr char __glo_example1[] =
"const xs = [0, 1, 2, 3]\n"
"const ys = [0, 1, 4, 6]\n"
"struct Person :=\n"
"    /// @brief This is struct definition\n"
"    /// @note This will be remembered on the scope\n"
"    name : FixString \n"
"fn main() :=\n"
"    demo()\n"
"    log(1, 2, xs, ys)\n"
"    log({city: \"London\"})\n"
"    log(Person {name = \"Michael\"})\n"
"    log(for( x <- range(1, 3) ) x)\n"
"    log(infer(model(), #MCMC).take(3))\n"
"fn demo() :=\n"
"    loop \n"
"        i <- range(3)\n"
"    in \n"
"        match i \n"
"            if 1 -> log(\"Yay\")\n"
"            else -> log(\"Nay\")\n"
"loop fn model() :=\n"
"    let \n"
"        f = fn(x, a, b) -> a * x + b \n"
"        m = sample(normal(2, f(1, 2, 3)))\n"
"        c = sample(normal(1, 2))\n"
"        sigma = sample(gamma(1, 1))\n"
"    in\n"
"        log(\"x =\", f(10, m, c))\n"
"        observe(normal(f(10, m, c), sigma));\n"
"        return [m, c, sigma]\n";


#pragma endregion

#pragma region LicenseH

#define DOUBT_VERSION "Version: v0.1"
#define DOUBT_AUTHOR "Author: Michael J. Burgess (doubt@michaelburgess.co.uk)"
#define DOUBT_LICENSE_SHORT                                                                                                          \
    "License (short): "                                                                                                              \
    "\n    * All non-commercial uses permitted if and only if author & copyright notice maintained."                                 \
    "\n    * Commercial use permitted if and only if included in novel product & author & copyright notice maintained"               \
    "\n        * A novel product is one which employs this software *as-is* to create a novel product"                               \
    "\n        * E.g., Use of this software as a host programming environment to develop new programs is permitted. "                \
    "\n        * Modifications or transformations to the source creates a *derivative* product, and so no commercial is permitted. " \
    "\n    * Verbatim resale not permitted. Omitting copyright notice & authorship not permitted in any circumstance."               \
    "\n    * No warranty provided. Use at your own risk."                                                                            \
    "\n    * These terms should be read as implying a broad commercial and non-commercial scope to freely use this software."        \
    "\n    * This is source-available, not open-source sofware: "                                                                    \
    "\n        * The author will not freely support or maintain this software.  "                                                    \
    "\n        * For paid support, please contact the author.  "

#define DOUBT_LICENCE_LONG " @todo Please contact the author"

#pragma endregion

//#endregion
/// ---------- ---------- ---------- HEADERS: CONFIG ---------- ---------- ------- ///
//#region Headers_Config 

#pragma region ClangConfigH
// cspell:ignore Wlanguage Wstrict
/// @note
/// ignore language-extension-token strict-prototypes
/// ignore language-extension-token missing-prototypes

// #pragma clang diagnostic ignored "-Wstrict-prototypes"
// #pragma clang diagnostic ignored "-Wlanguage-extension-token"


#pragma endregion

#pragma region ConfiguringDefines

#define ARRAY_SIZE_SMALL 16

// #ifdef TEST
    #define DEBUG_LOGGING_ON
    #define DEBUG_ABORT_ON_ASSERT
    #define DEBUG_ABORT_ON_ERROR
    #define DEBUG_REQUIRE_ASSERTS
    #define TRACEBACK_ABORT_ON_OVERFLOW
    // #define DEBUG_MEMORY
// #endif

#pragma endregion

#pragma region cIncludes

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h> /// @note include `sleep`

#ifdef _WIN32
    /// @note Windows specific includes
    /// @todo list the windows specific dependencies
    #include <windows.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    /// @note MacOS, BSD specific includes
    /// @todo list the MacOS, BSD specific dependencies

    /// @note in-use: arc4random
    #include <stdlib.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <string.h>
#endif

#pragma endregion

#pragma region cLibSafeReplacements

#define require_safe(safe_call)\
    if ((safe_call) < 0) {\
        puts("Error (SAFE CALL FAILED): " #safe_call);\
        exit(EXIT_FAILURE);}

/// @brief Returns the number of characters printed, or a negative value if an error occurs.
int clib_fprintf_safe(FILE *stream, const char *format, ...);


/// @brief Safely formats a string and stores it in a buffer.
///
/// @param buffer The destination buffer where the formatted string will be stored.
/// @param size The size of the destination buffer in bytes.
/// @param format The format string (similar to printf).
/// @param args The variable argument list.
///
/// @return The number of characters written (excluding the nullptr terminator) on success.
///         Returns -1 on error, setting errno appropriately.
int clib_vsnprintf_safe(char *buffer, size_t size, const char *format, va_list args);


/// @brief
/// Safe version of memcpy that checks for nullptr pointers and overlapping regions.
/// Returns 0 on success, -1 on error.
int clib_memcpy_safe(void *dest, size_t dest_size, const void *src, size_t count);


/// @brief Safely sets a specified block of memory to zero.
///
/// @param dest The destination buffer to be zeroed.
/// @param count The number of bytes to set to zero.
///
/// @return 0 on success, -1 on error (sets errno appropriately).
////
int clib_memset_zero_safe(void *dest, size_t dest_size, size_t count);

/// @brief Generates a random double value between 0 and 1.
///
/// @param
/// @return
double clib_random_improved(void);


int clib_memset_zero_safe(void *dest, size_t dest_size, size_t count) {
    if (dest == nullptr) {
        errno = EINVAL; // Invalid argument
        return -1;
    }

    if (count > dest_size) {
        // Prevent buffer overflow
        errno = ERANGE; // Range error
        return -1;
    }

    /// @note turn off: clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling
    /// this is now safe because we check for nullptr pointers and buffer overflow

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // NOLINT(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memset(dest, 0, count);// NOLINT
    #pragma clang diagnostic pop

    return 0;
}

int clib_vsnprintf_safe(char *buffer, size_t size, const char *format, va_list args) {
    // Check for nullptr pointers
    if (buffer == nullptr || format == nullptr) {
        errno = EINVAL; // Invalid argument
        return -1;
    }

    // Ensure that size is greater than zero to hold at least the nullptr terminator
    if (size == 0) {
        errno = ERANGE; // Range error
        return -1;
    }

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    /// this is now safe because we check for nullptr pointers and buffer overflow
    // NOLINT(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    int ret = vsnprintf(buffer, size, format, args); // NOLINT
    #pragma clang diagnostic pop

    // Check for encoding errors or other vsnprintf failures
    if (ret < 0) {
        // vsnprintf encountered an encoding error
        errno = EIO; // Input/output error
        return -1;
    }

    // Check if the output was truncated
    if ((size_t)ret >= size) {
        // Output was truncated
        errno = ERANGE; // Range error
        return -1;
    }

    return ret;
}

int clib_fprintf_safe(FILE *stream, const char *format, ...) {
    if (stream == nullptr) {
        errno = EINVAL; // Invalid argument
        return -1;
    }

    if (format == nullptr) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);

    if (ret < 0) {
        // vfprintf sets errno appropriately
        return -1;
    }

    return ret;
}


double clib_random_native(void) {
    /// @note turn off: clang-analyzer-security.insecureAPI.rand

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // NOLINT(clang-analyzer-security.insecureAPI.rand)
    return ((double)rand() / (double)RAND_MAX); // NOLINT
    #pragma clang diagnostic pop
}

double clib_strtod_safe(const char *str_without_null, size_t length) {
    if (str_without_null == nullptr) {
        errno = EINVAL; // Invalid argument
        return 0.0;
    }

    if (length == 0) {
        errno = EINVAL; // Invalid argument
        return 0.0;
    }

    char *endptr = nullptr;
    double result = strtod(str_without_null, &endptr);

    if (endptr == str_without_null) {
        // No conversion performed
        errno = EINVAL; // Invalid argument
        return 0.0;
    }

    if (errno == ERANGE) {
        // Overflow or underflow
        return 0.0;
    }

    return result;
}
int clib_strtoi_safe(const char *str_without_null, size_t length) {
    if (str_without_null == nullptr) {
        errno = EINVAL; // Invalid argument
        return 0;
    }

    if (length == 0) {
        errno = EINVAL; // Invalid argument
        return 0;
    }

    char *endptr = nullptr;
    int result = (int)strtol(str_without_null, &endptr, 10);

    if (endptr == str_without_null) {
        // No conversion performed
        errno = EINVAL; // Invalid argument
        return 0;
    }

    if (errno == ERANGE) {
        // Overflow or underflow
        return 0;
    }

    return result;
}

double clib_random_improved(void) {
#ifdef _WIN32
    unsigned int rand_num;
    // rand_s returns 0 on success
    if (rand_s(&rand_num) != 0) {
        // rand_s failed, fallback to rand()
        // Note: rand() is not thread-safe and less secure
        return clib_random_native();
    }
    // Scale rand_num to [0.0, 1.0)
    return ((double)rand_num) / ((double)UINT32_MAX + 1.0);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    uint32_t rand_num = arc4random();
    return ((double)rand_num) / ((double)UINT32_MAX + 1.0);
#elif defined(__linux__)
    static int urandom_fd = open("/dev/urandom", O_RDONLY);
    uint32_t rand_num;
    if (urandom_fd == -1) {
        // Failed to open /dev/urandom, fallback to rand()
        return clib_random_native();
    } else {
        ssize_t result = read(urandom_fd, &rand_num, sizeof(rand_num));
        if (result != sizeof(rand_num)) {
            // Failed to read enough bytes, fallback to rand()
            return clib_random_native();
        } else {
            return ((double)rand_num) / ((double)UINT32_MAX + 1.0);
        }
    }
#else
    // Unsupported platform, fallback to rand()
    return clib_random_native();
#endif
}

// Safe memcpy implementation
int clib_memcpy_safe(void *dest, size_t dest_size, const void *src, size_t count) {
    if (dest == nullptr || src == nullptr) {
        errno = EINVAL;
        return -1;
    }

    if (count > dest_size) {
        // Prevent buffer overflow
        errno = ERANGE;
        return -1;
    }

    // Check for overlapping regions
    if ((src < dest && (char *)src + count > (char *)dest) ||
        (dest < src && (char *)dest + count > (char *)src)) {
        // Regions overlap
        errno = EINVAL;
        return -1;
    }

    /// turn off: clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling


    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // NOLINT(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    memcpy(dest, src, count); // NOLINT
    #pragma clang diagnostic pop

    return 0;
}
#pragma endregion

#pragma region MacroHelpers

#define _STR_HELPER(x) #x
#define _STR(x) _STR_HELPER(x)

// #define _NUM_ARGS(...) ( sizeof( (void *[]){__VA_ARGS__} ) / sizeof(void *) )

#pragma endregion

//#endregion
/// ---------- ---------- ---------- HEADERS: BOXES ---------- ---------- -------- ///
//#region Headers_FixedBoxedDataType


#pragma region GlobalContextMacroH

#define ctx() __glo_GlobalContext
#define ctx_last_time() __glo_GlobalContext.debug.last_time
#define ctx_current_arena() __glo_GlobalContext.arenas.current
#define ctx_current_heap() __glo_GlobalContext.heaps.current
#define ctx_change_arena(alloc_name) ctx_current_arena() = alloc_name;

#define ctx_trace_me(moreinfo)\
    Tracer_push(ctx().debug.callstack, s(__func__), moreinfo, s(__FILE__), __LINE__)

#define ctx_trace_print(depth) Tracer_print_stack(ctx().debug.callstack, depth)

#pragma endregion

#pragma region MetaValuesMetaDataH

typedef enum MetaType {

    UBX_NULL = 0,
    UBX_BOOL = 1,
    UBX_INT = 2,
    UBX_UINT = 3,
    UBX_BYTE = 4,
    UBX_CHAR = 5,
    UBX_FLOAT = 6,
    UBX_TAG = 7, /// @note a tag is a 12-char (unboxed) string
    UBX_FLAGS = 8, /// @note by-value enum flags
    UBX_PTR = 9,     // for internal generic pointers
    UBX_STATE = 10,
    UBX_EMPTY_UBX_END = 11,


    /// @brief Boxed struct types on fixed arenas
    ///    (ie., copy/return unsafe, copy-by-ref, arena allocated, no GC)
    /// Typically created by the interpreter for internal use
    /// Two relevant arenas: global and local
    ///  Function stack frames are allocated on the local arena
    ///  Global variables and constants are allocated on the global arena
    ///     The global arena is never freed,
    ///         the local arena is freed on function exit.

    /// @brief
    /// a kind is a type object that represents a type
    /// all kinds live on the global arena
    /// .. maybe: provide a user-facing ability to free them?
    /// .. maybe: a warning if the global arena is getting too full etc.
    //UBX_PTR_KIND = 7, /// @todo -- a reified type
    UBX_PTR_ARENA = 12,
    UBX_PTR_HEAP = 13,


    /// @brief Garbage collected types
    ///    (ie., copy-by-ref, heap allocated, GC)
    // UBX_DYN_DOUBLE = 12, /// @todo -- maybe UBX_DYN_NUMERIC
    // UBX_PTR_HEAP = 12,
    // UBX_PTR_ARENA = 13, /// @todo -- maybe remove

    /// @brief
    /// why bother splitting collections from objects?
    /// well, maybe:
    /// objects require using their struct references, and generic methods
    /// and may support vtables, etc.
    /// while collections can be treated more efficiently as just the predef set available
    // UBX_DYN_FN = 14, /// @todo  -- dont need, if putting fn on heap, then native object

    /// @brief Error type
    /// @todo -- consider a special arena for error types which is never freed?
    /// ie., i guess move the __glo runtime error to a special arena
    /// & perhaps provide user-facing access to it, including free'ing
    UBX_PTR_ERROR = 15,



    //// ---- BOXED: ----


    // BOXED_UNKNOWN = 0,


    /// @brief FIXED TYPES
    /// often arena allocated, always boxed
    /// these are fixed size, static types

    BXD_FIX_STR = 's',
    BXD_FIX_ARRAY = 'a',
    BXD_FIX_DICT = 'd',
    BXD_FIX_KVPAIR = 'p',
    BXD_FIX_SCOPE = '^',
    BXD_FIX_AST = '%',
    BXD_FIX_DISTRIBUTION = '~', /// @todo -- think about this
    BXD_FIX_SYSTEM_INFO = '!',/// @todo -- think about this, prob remove for a dict/obj
    BXD_FIX_MODULE = '\\',
    BXD_FIX_STRUCT = '$',
    BXD_FIX_ENUM = '|',
    BXD_FIX_TRAIT = '/',
    BXD_FIX_TYPE = '?',
    BXD_FIX_OBJECT = 't',
    BXD_FIX_ARRAY_VIEW = '-',
    BXD_FIX_VEC_TAGS = '#',


    /// @brief NUMERIC TYPES
    BXD_FIX_NUM_128BIT = '_',
    BXD_FIX_NUM_DOUBLE = '.',

    /// @todo
    // BXD_FIX_NUM_BIGINT = '^',
    // BXD_FIX_NUM_RATIONAL = '/',
    // BXD_FIX_NUM_COMPLEX = 'j',
    // BXD_FIX_NUM_QUATERNION = 'q',


    /// @brief  double-based vectors (linear algebra), with unboxed doubles
    BXD_FIX_VEC_DOUBLE_1 = '1',
    BXD_FIX_VEC_DOUBLE_2 = '2',
    BXD_FIX_VEC_DOUBLE_3 = '3',
    BXD_FIX_VEC_DOUBLE_4 = '4',



    /// @brief Flx-ible types
    /// sometimes gc'd, sometimes heap allocated
    /// maybe arena allocated (eg., as local vars)
    /// these are resizing, dynamic types, with more capacities
    /// they are always boxed, and the payload is a ptr to the value

    BXD_FLX_OBJECT = 'o',

    /// @brief nullptrABLE TYPES

    // probably not needed because box is nullable
    // and can unwrap to box_error()
    // BXD_FLX_NULL = '0',
    // BXD_FLX_OPTION = '?',
    // BXD_FLX_RESULT = '!',
    // BXD_FLX_MAYBE = '~',

    /// COLLECTIONS & CONTAINERS

    /// @brief generic collection type
    BXD_FLX_STR = 'S',/// @todo rope-based string builder
    BXD_FLX_ARRAY = 'A',
    BXD_FLX_QUEUE = 'Q',
    BXD_FLX_QUEUE_PRIORITY = 'P',
    BXD_FLX_STACK = 'K',
    BXD_FLX_DICT = 'D',      /// @note (FixStr, Box)
    BXD_FLX_SET = '>',

    // BXD_FLX_SET_SORTED = 'u',
    BXD_FLX_GRAPH_DIRECTED_WEIGHTED = 'G',
    BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED = 'g',
    BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED = 'U',
    BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED = 'u',

    /// @brief matrix types
    BXD_FLX_VEC_DOUBLE_N = 'V',
    // BXD_FIX_VECTOR = 'V', //box-based vector type -- not needed, this is an array?
    BXD_FLX_MATRIX = 'M', /// box-based matrices, eg., if ints then unboxed
    BXD_FLX_MATRIX_DOUBLE = 'm', /// double-based matrices, with unboxed doubles
    BXD_FLX_TENSOR = '@',
    BXD_FLX_TENSOR_DOUBLE = '<', /// double-based tensors, with unboxed doubles

    /// @brief
    /// a dataframe is a collection of columns given by VEC_ types
    BXD_FLX_DATAFRAME = 'F',


    /// @brief the underlying is unboxed (ie., data is stored directly in the collection)
    ///    ... the collection is boxed (ie., the payload is a ptr to the value)
    ///    This allows for efficient processing of the collection data
    BXD_FLX_VEC_BOOL = 'B',
    BXD_FLX_VEC_INT = 'I',
    BXD_FLX_VEC_TAGS = 'T',
    BXD_FLX_VEC_STR = 'L', // L for lines
    BXD_FLX_VEC_OBJS = 'O', // ie., of this type
    // BXD_FIX_VEC_CHAR = 'C',
} MetaType;

typedef enum LifetimeType {
    LIFETIME_UNKNOWN = 0,
    LIFETIME_ARENA_AUTO = 1,
    LIFETIME_HEAP_GC = 2,
    LIFETIME_FOREVER = 3,
    LIFETIME_MODULE,
    LIFETIME_FUNCTION,
    LIFETIME_THREAD,
    LIFETIME_ENUM_SIZE
} LifetimeType;

typedef enum LifetimeStatusType {
    LIVING_UNMANAGED = 0,
    LIVING_ALIVE,     // Object is alive and accessible
    LIVING_DEAD,      // Object marked for deletion
    LIVING_PINNED,    // Object is pinned and cannot be moved
    LIVING_ENUM_SIZE
} LifetimeStatusType;


typedef struct MetaValue MetaValue;

typedef struct MetaData {
    size_t size;            // Size of the data
    size_t capacity;        // Capacity of the data
    size_t alloc_size;    // Size of each element

    MetaType type;           // Type of the data

    LifetimeType lifetime;  // Lifetime of the data
    LifetimeStatusType state;

    bool marked;        // GC marking flag

    MetaValue *next;
} MetaData;

typedef struct MetaValue {
    struct MetaData meta;
    void *data;             // Pointer to the actual data
}  MetaValue;


#pragma endregion

#pragma region FixStrH

typedef struct FixStr {
    const char *cstr;
    size_t size;
} FixStr;


#define ANSI_COL_RESET       "\x1b[0m"
#define ANSI_COL_CYAN       "\x1b[36m" // Cyan
#define ANSI_COL_GREEN        "\x1b[32m" // Green
#define ANSI_COL_YELLOW     "\x1b[33m" // Yellow
#define ANSI_COL_RED       "\x1b[31m" // Red
#define ANSI_COL_MAGENTA "\x1b[35m" // Magenta
#define ANSI_COL_BLUE        "\x1b[34m" // Blue
#define ANSI_COL_WHITE     "\x1b[37m" // White

// Bright Colors
#define ANSI_COL_BRIGHT_BLACK "\x1b[90m"
#define ANSI_COL_BRIGHT_RED   "\x1b[91m"
#define ANSI_COL_BRIGHT_GREEN "\x1b[92m"
#define ANSI_COL_BRIGHT_YELLOW "\x1b[93m"
#define ANSI_COL_BRIGHT_BLUE  "\x1b[94m"
#define ANSI_COL_BRIGHT_MAGENTA "\x1b[95m"
#define ANSI_COL_BRIGHT_CYAN  "\x1b[96m"
#define ANSI_COL_BRIGHT_WHITE "\x1b[97m"

// Background Colors (Optional)
#define ANSI_COL_BG_RED       "\x1b[41m"
#define ANSI_COL_BG_GREEN     "\x1b[42m"
#define ANSI_COL_BG_YELLOW    "\x1b[43m"
#define ANSI_COL_BG_BLUE      "\x1b[44m"
#define ANSI_COL_BG_MAGENTA   "\x1b[45m"
#define ANSI_COL_BG_CYAN      "\x1b[46m"
#define ANSI_COL_BG_WHITE     "\x1b[47m"

#define FixStr_len(str) (str).size

#define FixStr_is_empty(str) ((str).size == 0 || (str).cstr == nullptr)
#define FixStr_empty() ((FixStr) {0})
#define FixStr_from_cstr(txt) ((FixStr){ .cstr = txt, .size = strlen(txt)})
#define FixStr_chr_at(line, at) ((line).cstr[at])
#define FixStr_chr_eq(line, at, chr) ((line).cstr[at] == chr)
#define s(txt) ((FixStr){ .cstr = (txt), .size = sizeof(txt) - 1 })
#define ch(chr) ((FixStr){ .cstr = &(chr), .size = 1 })
#define fmt(str) (int) str.size, str.cstr
#define fmt_ref(str_ptr) (int) (str_ptr)->size, (str_ptr)->cstr

char *cstr_repeat_new(const char *cstr, size_t count);
bool write_bytes_to_file(const char *filename, const char *bytes, size_t len);
unsigned char *read_bytes_from_file_cnew(const char *filename,  size_t *out_len);

int char_to_base37(char c);
char base37_to_char(uint8_t value);

// Function declarations
void FixStr_puts(FixStr str);
void FixStr_print_variadic(size_t count, ...);
void FixStr_printf(const char *format, ...);
FixStr FixStr_read_file_new(const char *filename);
size_t FixStr_hash(FixStr str);
FixStr FixStr_trim_delim(FixStr str);
FixStr FixStr_new(const char *cstr, size_t len);
FixStr FixStr_join_new(FixStr one, ...);
FixStr FixStr_col_new(const char *color_code, FixStr str);
FixStr FixStr_chr(char chr);
FixStr FixStr_glue_new(FixStr left, FixStr right);
FixStr FixStr_glue_sep_new(FixStr left, FixStr sep, FixStr right);
FixStr FixStr_firstN(FixStr str, size_t n);
bool FixStr_eq(FixStr left, FixStr right);
bool FixStr_eq_cstr(FixStr left, const char *right);
bool FixStr_eq_chr(FixStr left, char right);
FixStr FixStr_copy(FixStr old);
bool FixStr_contains(FixStr str, FixStr part);
bool FixStr_index_of(FixStr str, FixStr part, size_t *out_index);
bool FixStr_last_index(FixStr str, FixStr part, size_t *out_index);
bool FixStr_contains_chr(FixStr str, char chr);
int FixStr_index_of_chr(FixStr str, char chr);
int FixStr_last_index_of_chr(FixStr str, char chr);
FixStr FixStr_ncopy(FixStr str, size_t start, size_t end);
FixStr FixStr_sub(FixStr str, size_t start, size_t size);
FixStr FixStr_replace(FixStr str, char target, char replacement);
FixStr FixStr_trim(FixStr str);
FixStr FixStr_upper(FixStr str);
FixStr FixStr_lower(FixStr str);
int FixStr_cmp(FixStr left, FixStr right);
int FixStr_cmp_natural(FixStr left, FixStr right);
FixStr FixStr_join_chr_new(FixStr str, char chr);
FixStr FixStr_join_cstr_new(FixStr str, const char *cstr);
FixStr *FixStr_split_chr(FixStr str, char delimiter, size_t *out_count);
FixStr *FixStr_lines(FixStr str, size_t *out_count);
FixStr FixStr_repeat_new(FixStr str, size_t count);
FixStr FixStr_fmt_new(FixStr format, ...);
FixStr FixStr_fmt_va_new(FixStr format, va_list args);

void FixStr_aro_free(FixStr str);

FixStr FixStr_part(FixStr line, size_t start, size_t end);


#pragma endregion

#pragma region BoxH

/// @note Box is the internal type of data for the interpreter
///     nullptr, INT, BOOL, FLOAT are actually all unboxed (the payload is the value)
///     all others are boxed (the payload is a ptr to the value)
///     A distinction is made between BOX_ collections and DYN_ collections
///         the latter have dynamic capacities, (todo?) always heap allocated
///         the former have fixed capacities and (todo?) should always been on an arena
///         and, at least, are emitted and used by the interpreter internally
///             eg., function args are packed into FixArrays

/// @note UNBOXED TYPES GO HERE, cf. `interpreter.h`

// typedef struct MetaTypeInfo {
//     MetaType type;
//     size_t runtime_size;
//     bool is_managed;
// } MetaTypeInfo;

// max 16 types
/// @note
/// these types are only relevant when values leak to the user via boxing
/// otherwise, we just directly create the underlying structs
///     ie., in the interpreter, we just use the structs directly
// typedef enum MetaType {
//     /// @brief Unboxed types
//     ///     (ie.,  copy/return-safe, copy-by-value, fixed size, no alloc/GC)
//     UBX_NULL = 0,
//     UBX_BOOL = 1,
//     UBX_INT = 2,
//     UBX_UINT = 3,
//     UBX_BYTE = 4,
//     UBX_CHAR = 5,
//     UBX_FLOAT = 6,
//     UBX_TAG = 7, /// @note a tag is a 12-char (unboxed) string
//     UBX_FLAGS = 8, /// @note by-value enum flags
//     UBX_PTR = 9,     // for internal generic pointers
//     UBX_EMPTY_UBX_END = 10,
//     UBX_STATE = 11,

//     /// @brief Boxed struct types on fixed arenas
//     ///    (ie., copy/return unsafe, copy-by-ref, arena allocated, no GC)
//     /// Typically created by the interpreter for internal use
//     /// Two relevant arenas: global and local
//     ///  Function stack frames are allocated on the local arena
//     ///  Global variables and constants are allocated on the global arena
//     ///     The global arena is never freed,
//     ///         the local arena is freed on function exit.

//     /// @brief
//     /// a kind is a type object that represents a type
//     /// all kinds live on the global arena
//     /// .. maybe: provide a user-facing ability to free them?
//     /// .. maybe: a warning if the global arena is getting too full etc.
//     //UBX_PTR_KIND = 7, /// @todo -- a reified type
//     UBX_PTR_ARENA = 12,
//     UBX_PTR_HEAP = 13,


//     /// @brief Garbage collected types
//     ///    (ie., copy-by-ref, heap allocated, GC)
//     // UBX_DYN_DOUBLE = 12, /// @todo -- maybe UBX_DYN_NUMERIC
//     // UBX_PTR_HEAP = 12,
//     // UBX_PTR_ARENA = 13, /// @todo -- maybe remove

//     /// @brief
//     /// why bother splitting collections from objects?
//     /// well, maybe:
//     /// objects require using their struct references, and generic methods
//     /// and may support vtables, etc.
//     /// while collections can be treated more efficiently as just the predef set available
//     // UBX_DYN_FN = 14, /// @todo  -- dont need, if putting fn on heap, then native object

//     /// @brief Error type
//     /// @todo -- consider a special arena for error types which is never freed?
//     /// ie., i guess move the __glo runtime error to a special arena
//     /// & perhaps provide user-facing access to it, including free'ing
//     UBX_PTR_ERROR = 15,
// } MetaType;

FixStr ubx_nameof(MetaType bt) {
    assert(bt < UBX_EMPTY_UBX_END);

    static const char *names[] = {
        [UBX_NULL] = "nullptr",
        [UBX_BOOL] = "bool",
        [UBX_INT] = "int",
        [UBX_UINT] = "uint",
        [UBX_BYTE] = "byte",
        [UBX_CHAR] = "char",
        [UBX_FLOAT] = "float",
        [UBX_TAG] = "tag",
        [UBX_FLAGS] = "flags",
        [UBX_PTR] = "ptr",
        [UBX_EMPTY_UBX_END] = "empty",
        [UBX_STATE] = "state",
        [UBX_PTR_ARENA] = "boxed(arena)",
        [UBX_PTR_HEAP] = "boxed(heap)",
        [UBX_PTR_ERROR] = "error",
    };

    return FixStr_from_cstr(names[bt]);
}

/// @brief For boxing pointers as (Number|Pointer) types
typedef struct Box {
    uint64_t payload : 60;
    uint8_t type : 4;
} Box;


#define Box_is_Boxed(b)\
     ((b).type == UBX_PTR_ARENA || (b).type == UBX_PTR_HEAP || (b).type == UBX_PTR_ERROR)


#define Box_is_Unboxed(b) ((b).type <= UBX_EMPTY_UBX_END)

/// @todo consider adding an additional 64bit hidden field
///... maybe before the type field, to store additional data

// typedef enum FixType {
//     /// @note -- we keep errors out of here,
//     /// so we can know if there's an error at box'd level
//     /// i.e., without having to deref the box payload to get this type




// } FixType;

typedef enum GenericTypes {
    GT_UNKNOWN = 0, /// @note -- /any(none) = NOTHING
    GT_ANY, /// @note -- /any() = ANY_VALUE + FIX, FLEX...
    GT_ANY_VALUE, // /value UBX_NULL, UBX_BOOL, UBX_INT, UBX_UINT, UBX_BYTE, UBX_CHAR, UBX_FLOAT, UBX_TAG, UBX_FLAGS, UBX_PTR, UBX_EMPTY_UBX_END, UBX_STATE
    GT_ANY_LOGICAL,  /// @note -- /any(logical) = UBX_BOOL, UBX_FLAGS, UBX_EMPTY_UBX_END
    GT_ANY_NUMERIC, /// @note -- /any(num) = UBX_INT, UBX_UINT, UBX_BYTE, UBX_CHAR, UBX_FLOAT,
                /// -- /numeric = BXD_FIX_NUM_128BIT, BXD_FIX_NUM_DOUBLE, UBX_EMPTY_UBX_END
    GT_ANY_INTEGRAL, /// @note -- /any(int) = UBX_INT, UBX_UINT, UBX_BYTE, UBX_CHAR, UBX_EMPTY_UBX_END, UBX_PTR_ERROR
                    /// -- BXD_FIX_NUM_128BIT
    GT_ANY_FLOATING, /// @note -- /any(float) -- UBX_FLOAT, BXD_FIX_NUM_DOUBLE, UBX_EMPTY_UBX_END,
    GT_ANY_STR,    /// @note -- /any(str) UBX_TAG, BXD_FIX_STR, BXD_FLX_STR, BXD_FLX_VEC_CHAR
    GT_ANY_COL,    /// @note -- /any(col) = BXD_FLX_ARRAY, BXD_FLX_DICT, BXD_FLX_SET, BXD_FLX_QUEUE, BXD_FLX_STACK
    GT_ANY_SEQ,    /// @note -- /any(seq) = BXD_FLX_ARRAY, BXD_FLX_QUEUE, BXD_FLX_STACK, BXD_FIX_VEC_CHAR, BXD_FIX_VEC_TAGS, ...
    GT_ANY_MAYBE, /// @note -- /any(maybe) = UBX_NULL, UBX_EMPTY_UBX_END,
    GT_ANY_STATIC, /// @note -- /any(static) = UBX_PTR_ARENA
    GT_ANY_DYN, /// @note -- /any(dyn) = UBX_PTR_HEAP
    GT_ANY_FIX, /// @note -- /any(fix) = UBX_NULL, UBX_BOOL, UBX_INT, UBX_UINT, UBX_BYTE, UBX_CHAR, UBX_FLOAT, UBX_TAG, UBX_FLAGS, UBX_PTR, UBX_EMPTY_UBX_END, UBX_STATE, BXD_FIX_STR, BXD_FIX_ARRAY, BXD_FIX_DICT, BXD_FIX_KVPAIR, BXD_FIX_SCOPE, BXD_FIX_AST, BXD_FIX_DISTRIBUTION, BXD_FIX_SYSTEM_INFO, BXD_FIX_MODULE, BXD_FIX_STRUCT, BXD_FIX_ENUM, BXD_FIX_TRAIT, BXD_FIX_TYPE, BXD_FIX_OBJECT, BXD_FIX_ARRAY_VIEW, BXD_FIX_VEC_TAGS, BXD_FIX_NUM_128BIT, BXD_FIX_NUM_DOUBLE, BXD_FIX_VEC_DOUBLE_1, BXD_FIX_VEC_DOUBLE_2, BXD_FIX_VEC_DOUBLE_3, BXD_FIX_VEC_DOUBLE_4
    GT_ANY_FLEX, /// @note -- /any(flex) = BXD_FLX_OBJECT, BXD_FLX_STR, BXD_FLX_ARRAY, BXD_FLX_QUEUE, BXD_FLX_QUEUE_PRIORITY, BXD_FLX_STACK, BXD_FLX_DICT, BXD_FLX_SET, BXD_FLX_GRAPH_DIRECTED_WEIGHTED, BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED, BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED, BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED, BXD_FLX_VEC_DOUBLE_N, BXD_FLX_MATRIX, BXD_FLX_MATRIX_DOUBLE, BXD_FLX_TENSOR, BXD_FLX_TENSOR_DOUBLE, BXD_FLX_DATAFRAME, BXD_FLX_VEC_BOOL, BXD_FLX_VEC_INT, BXD_FLX_VEC_TAGS, BXD_FLX_VEC_STR, BXD_FLX_VEC_OBJS, BXD_FLX_VEC_CHAR
    GT_ANY_BXD_FIX_NUM, /// @note -- /any(num) = BXD_FIX_NUM_128BIT, BXD_FIX_NUM_DOUBLE
    GT_ANY_VEC, /// @note -- /any(vec) = BXD_FIX_VEC_DOUBLE_1, BXD_FIX_VEC_DOUBLE_2, BXD_FIX_VEC_DOUBLE_3, BXD_FIX_VEC_DOUBLE_4, BXD_FLX_VEC_BOOL, BXD_FLX_VEC_INT, BXD_FLX_VEC_TAGS, BXD_FLX_VEC_STR, BXD_FLX_VEC_OBJS, BXD_FLX_VEC_CHAR
    GT_ANY_GRAPH, /// @note -- /any(graph) = BXD_FLX_GRAPH_DIRECTED_WEIGHTED, BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED, BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED, BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED
    GT_ANY_MATRIX, /// @note -- /any(matrix) = BXD_FLX_MATRIX, BXD_FLX_MATRIX_DOUBLE
    GT_ANY_DATA, /// @note -- /any(tensor) =  BXD_FLX_TENSOR, BXD_FLX_TENSOR_DOUBLE, BXD_FLX_MATRIX, BXD_FLX_MATRIX_DOUBLE, BXD_FLX_VEC_DOUBLE, BXD_FLX_VEC_INT, BXD_FLX_VEC_TAGS, BXD_FLX_VEC_STR, BXD_FLX_VEC_OBJS, BXD_FLX_VEC_BOOL, BXD_FLX_VEC_CHAR
    GT_ANY_TYPE, /// @note -- /any(type) = BXD_FIX_TYPE, BXD_FIX_TRAIT, BXD_FIX_ENUM, BXD_FIX_STRUCT
    GT_ANY_OBJECT, /// @note -- /any(object) = BXD_FIX_OBJECT, BXD_FLX_OBJECT
    GT_ANY_STRUCT, /// @note -- /any(struct) = BXD_FIX_STRUCT
    GT_ENUM_SIZE
} GenericTypes;


FixStr bt_nameof(MetaType type) {
    static const char *names[] = {
        [BXD_FIX_STR] = "str",
        [BXD_FIX_ARRAY] = "array",
        [BXD_FIX_DICT] = "dict",
        [BXD_FIX_KVPAIR] = "kvpair",
        [BXD_FIX_SCOPE] = "scope",
        [BXD_FIX_AST] = "ast",
        [BXD_FIX_DISTRIBUTION] = "distribution",
        [BXD_FIX_SYSTEM_INFO] = "system_info",
        [BXD_FIX_MODULE] = "module",
        [BXD_FIX_STRUCT] = "struct",
        [BXD_FIX_ENUM] = "enum",
        [BXD_FIX_TRAIT] = "trait",
        [BXD_FIX_TYPE] = "type",
        [BXD_FIX_OBJECT] = "object",
        [BXD_FIX_ARRAY_VIEW] = "array_view",
        [BXD_FIX_VEC_TAGS] = "vec_tags",
        [BXD_FIX_NUM_128BIT] = "num_128bit",
        [BXD_FIX_NUM_DOUBLE] = "num_double",
        [BXD_FIX_VEC_DOUBLE_1] = "vec_double_1",
        [BXD_FIX_VEC_DOUBLE_2] = "vec_double_2",
        [BXD_FIX_VEC_DOUBLE_3] = "vec_double_3",
        [BXD_FIX_VEC_DOUBLE_4] = "vec_double_4",
        [BXD_FLX_OBJECT] = "object",
        [BXD_FLX_STR] = "str",
        [BXD_FLX_ARRAY] = "array",
        [BXD_FLX_QUEUE] = "queue",
        [BXD_FLX_QUEUE_PRIORITY] = "queue_priority",
        [BXD_FLX_STACK] = "stack",
        [BXD_FLX_DICT] = "dict",
        [BXD_FLX_SET] = "set",
        [BXD_FLX_GRAPH_DIRECTED_WEIGHTED] = "graph_directed_weighted",
        [BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED] = "graph_directed_unweighted",
        [BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED] = "graph_undirected_weighted",
        [BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED] = "graph_undirected_unweighted",
        [BXD_FLX_VEC_DOUBLE_N] = "vec_double_n",
        [BXD_FLX_MATRIX] = "matrix",
        [BXD_FLX_MATRIX_DOUBLE] = "matrix_double",
        [BXD_FLX_TENSOR] = "tensor",
        [BXD_FLX_TENSOR_DOUBLE] = "tensor_double",
        [BXD_FLX_DATAFRAME] = "dataframe",
        [BXD_FLX_VEC_BOOL] = "vec_bool",
        [BXD_FLX_VEC_INT] = "vec_int",
        [BXD_FLX_VEC_TAGS] = "vec_tags",
        [BXD_FLX_VEC_STR] = "vec_str",
        [BXD_FLX_VEC_OBJS] = "vec_objs",
    };

    return FixStr_from_cstr(names[type]);
}


typedef struct Boxed {
    MetaData meta;
} Boxed;

#define Boxed_as(type, boxed) ((type *) (boxed))
#define Box_Boxed_meta(box) ((Boxed *) (box.payload))->meta

#define Box_is_Boxed_type(b, t) \
    Box_is_Boxed(b) && (Box_Boxed_meta(b).type == t)

#define Boxed_unwrap(boxed_type, box) \
    ((boxed_type *) Boxed_unwrap_checked(box, boxed_type))

inline void *Boxed_unwrap_checked(Box b, MetaType t) {
    require_positive(t);
    require_not_null(b.payload);

    if(Box_is_Boxed_type(b, t)) {
        return (void *) b.payload;
    }

    return nullptr;
}

typedef enum BoxStateType {
    BXS_CONTINUE = 'C',
    BXS_BREAK = 'B',
    BXS_RETURN ='R',
    BXS_YIELD = 'Y',
    BXS_DONE = 'D',
    BXS_GOTO = 'G',
    BXS_EXIT = 'X',
    /// @note maybe add suspend, resume, etc.
} BoxStateType;

FixStr st_nameof(BoxStateType cs) {
    static const char *names[] = {
        [BXS_BREAK] = "break",
        [BXS_CONTINUE] = "continue",
        [BXS_RETURN] = "return",
        [BXS_YIELD] = "yield",
        [BXS_DONE] = "done",
        [BXS_GOTO] = "goto",
        [BXS_EXIT] = "exit"
    };

    return FixStr_from_cstr(names[cs]);
}


FixStr Box_to_FixStr(Box b);

bool Box_try_numeric(Box box, double *out_value);

size_t Box_hash(Box box);
bool Box_eq(Box a, Box b);
void Box_aro_free(Box b);

#pragma endregion

#pragma region FixIter


typedef struct FixIter {
    size_t iter_index;
    size_t iter_from;
    size_t iter_to;
    Box iter_fn;
    Box iter_state;
} FixIter;

#pragma endregion

#pragma region FixErrorH

typedef enum FixErrorType {
    BXE_NONE = 0,
    BXE_NATIVE = 1,
    BXE_INTERPRETER,
    BXE_NOT_IMPLEMENTED,
} FixErrorType;

typedef struct FixError {
    MetaData meta;

    FixErrorType code;
    FixStr message;
    FixStr location;

    /// @brief
    /// @todo
    struct FixErrorInfo {
        FixStr user_file;
        size_t user_line;
        size_t user_col;
    } info;
} FixError;
#pragma endregion

#pragma region FixArrayH


typedef struct FixArray {
    MetaData meta;
    Box *data;    // Array of Box data
} FixArray;


typedef struct FixArrayView {
    MetaData meta;
    FixArray *data;
    size_t start;
    size_t end;
} FixArrayView;


// FixArray functions
void FixArray_Arena_new_data(FixArray *array, size_t size);
FixStr FixArray_to_FixStr(FixArray *array);
Box FixArray_set(FixArray *box_array, Box element, size_t index);
Box FixArray_append(FixArray *array, Box element);
Box FixArray_overwrite(FixArray *array, size_t index, Box element);
Box FixArray_weak_unset(FixArray *array, size_t index);
void FixArray_aro_free(FixArray *box_array);


#pragma endregion

#pragma region FixDictH


typedef struct FixKvPair {
    size_t hash;
    FixStr key;
    Box value;
    struct FixKvPair *next; // Pointer to the next entry in the chain
} FixKvPair;

typedef struct FixDict {
    MetaData meta;
    FixKvPair **data; // Array of pointers to FixKvPair (chains)
} FixDict;



#define FixDict_iter_items(fd, entry) for(size_t __i = 0; __i < cap(fd); __i++)\
    for(FixKvPair *entry = fd.data[__i]; entry != nullptr; entry = entry->next)

// FixKvPair functions
FixKvPair *FixKvPair_new(FixStr key, Box value);
FixStr FixKvPair_to_FixStr(FixKvPair *pair);



// FixDict functions
FixStr FixDict_to_FixStr(FixDict *d);
void FixDict_data_new(FixDict *dict, size_t size);
Box FixDict_get(FixDict *dict, FixStr key);
FixStr FixDict_keys_to_FixStr(FixDict *dict);
void FixDict_debug_print_entry(FixKvPair *entry);
void FixDict_debug_print(FixDict dict);
bool FixDict_find(FixDict *dict, FixStr key, FixKvPair *out);
void FixDict_from_pairs(FixDict *empty_dict, FixKvPair **pairs, size_t num_pairs);
void FixDict_set(FixDict *dict, FixStr key, Box value);
FixKvPair *FixDict_remove(FixDict *dict, FixStr key);
void FixDict_aro_free(FixDict *dict);
void FixDict_merge(FixDict *dest, FixDict *src);

#pragma endregion

#pragma region FixScopeH


typedef struct FixScope {
    MetaData meta;
    struct FixScope *parent;
    FixDict data;
    FixStr doc_string;
} FixScope;


#define FixScope_empty(dstr) (FixScope){.data = {0}, .parent = nullptr, .doc_string = dstr}

#define FixScope_define_local(scope_ptr, name, value)\
    native_log_debug(s("Defining in Scope: %.*s"), fmt((scope_ptr)->doc_string));\
    FixDict_set(&((scope_ptr)->data), name, value)


// FixScope functions
void FixScope_data_new(FixScope *self, FixScope *parent);
void FixScope_sized_new(FixScope *self, FixScope *parent, size_t num_data);
void FixScope_debug_print(FixScope scope);
void FixScope_merge_local(FixScope *dest, FixScope *src);
bool FixScope_lookup(FixScope *scope, FixStr name, Box *out_value);
bool FixScope_has(FixScope *scope, FixStr name);
void FixScope_aro_free(FixScope *scope);
FixStr FixScope_to_FixStr(FixScope *scope);

#pragma endregion

#pragma region FixAstH
#pragma endregion

#pragma region FixDistH
#pragma endregion

#pragma region FixFn

typedef enum FixFnType {
    FN_USER = 'u',
    FN_MACRO = 'm',
    FN_ASYNC = 'a',
    FN_LOOP = 'l',
    FN_GENERATOR = 'g',
    FN_NATIVE = 'n',
    FN_NATIVE_0 = '0',
    FN_NATIVE_1 = '1',
    FN_NATIVE_2 = '2',
    FN_NATIVE_3 = '3',
} FixFnType;

typedef struct FixFn  {
    FixStr name;
    FixDict signature;
    FixScope enclosure;
    FixFnType type;
    void *fnptr;
    void *code;
} FixFn;
/// @note castable to FixFn
typedef struct FixFnAsync  {
    FixFn next;
    /// @todo etc.
} FixFnAsync;


// FixFn functions
static FixFn *FixFn_new(FixFnType type, FixStr name,
    FixDict args_defaults, FixScope enclosure, void *fnptr, void *code);
Box FixFn_call(FixFn *ffn, FixScope scope, FixArray args);
Box FixFn_test_native2(FixFn *fn, FixScope parent, Box one, Box two);
#pragma endregion

#pragma region FixSystemInfoH
#pragma endregion

#pragma region FixModuleH
#pragma endregion

#pragma region FixStructH


typedef struct FixStruct {
    FixStr name;
    FixStr doc_string;
    FixDict fields;
    FixScope namespace;
} FixStruct;

// FixStruct functions
FixStruct *FixStruct_new(FixStr name, size_t num_fields);
FixStr FixStruct_to_FixStr(FixStruct *s);




#pragma endregion

#pragma region FixEnumH
#pragma endregion

#pragma region FixTraitH
#pragma endregion

#pragma region FixTypeH
#pragma endregion

#pragma region FixObjectH


#pragma endregion

#pragma region FixVectorsH


typedef struct FlxVector {
    MetaData meta;
    Box *data;
} FlxVector;

typedef struct FixVector1F {
    MetaData meta;
    double x;
} FixVector1F;

typedef struct FixVector2F {
    MetaData meta;
    double x;
    double y;
} FixVector2F;

typedef struct FixVector3F {
    MetaData meta;
    double x;
    double y;
    double z;
} FixVector3F;

typedef struct FixVector4F {
    MetaData meta;
    double x;
    double y;
    double z;
    double w;
} FixVector4F;

#pragma endregion

#pragma region FixNumerics
#pragma endregion

#pragma region FixMathH

typedef enum {
    DIST_UNKNOWN,
    DIST_USER_DEFINED,
    DIST_NORMAL,
    DIST_GAMMA,
    DIST_BETA,
    DIST_POISSON,
    DIST_BINOMIAL,
    DIST_BERNOULLI,
    DIST_EXPONENTIAL,
    DIST_UNIFORM,
    DIST_BETA_BINOMIAL,
    DIST_DIRICHLET,
    DIST_GAMMA_POISSON,
    DIST_INV_GAMMA,
    DIST_LAPLACE,
    DIST_LOG_NORMAL,
    DIST_NEG_BINOMIAL,
    DIST_PARETO,
    DIST_TRIANGULAR,
    DIST_UNIFORM_INT,
    DIST_WEIBULL,
    DIST_BOLTZMANN,
    DIST_GEV,
    DIST_GUMBEL,
    DIST_RAYLEIGH,
    DIST_WALD,
    DIST_MAXWELL,
    DIST_POWER_LAW,
} FixDistType;

typedef struct FixDist {
    FixDistType type;
    FixStr *param_names;
    FixStr *param_descriptions;
    size_t num_params;
    double *params;
    double (*sample_fn_ptr)(struct FixDist *dist);
    double (*pdf_fn_ptr)(struct FixDist *dist, double x);
    double (*log_pdf_fn_ptr)(struct FixDist *dist, double x);
} FixDist;

typedef enum {
    INFERENCE_MCMC,
    INFERENCE_VI,
    INFERENCE_MAP
} InferMethod;

typedef struct {
    double ** samples;
    size_t *num_samples;
} FixInferResult;

/**
 * FixStructure representing a probabilistic model.
 * This structure should encapsulate the model's parameters, observed data, and any other relevant information.
 */
typedef struct FixInferModel {
    FixDist **priors;
    FixDist *likelihood;
    double *params;
    size_t num_params;
    double *data;
    size_t num_data;
    size_t num_target_samples;
} FixInferModel;


#pragma endregion


#pragma region MetaTypes


#pragma endregion

#pragma region BoxedApi

// Test function prototypes
// int FixArray_test_main();
// int FixDict_test_main();
// int FixArray_test_main();
// int FlxDict_test_main();
// int FixScope_test_main();
// int FixFn_test_main();
// void Box_test_main();

// Generic Box functions
FixStr Boxed_to_FixStr(Boxed *boxed);




#pragma endregion

#pragma region FixErrorTypeH


#define error_print_all() error_print_lastN(ctx().debug.num_errors)

FixError *error_push(FixError error);
FixError error_getlast(FixArray errors);
void error_print_lastN(size_t n);
void error_print(FixError error);


#define error_push_runtime(erc, erm, erl)\
    error_push((FixError){.code = (erc), .message = (erm), .location = (erl)})

#define native_error(...) \
    error_push_runtime(BXE_NATIVE, FixStr_fmt_new(__VA_ARGS__), s(__FILE__ ":" _STR(__LINE__)))



#pragma endregion

//#endregion
/// ---------- ---------- ---------- HEADERS: UTIL ---------- ---------- --------- ///
//#region Headers_Utilities 

#pragma region LoggingH

typedef enum LogLevel {
    LL_DEFAULT,
    LL_DEBUG,
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_RECOVERABLE,
    LL_ASSERT,
    LL_TEST,
    LL_ENUM_SIZE,
} LogLevel;


FixStr ll_nameof(LogLevel ll) {
    static char *names[] = {
        [LL_DEFAULT] = "DEFAULT",
        [LL_DEBUG] = "DEBUG",
        [LL_INFO] = "INFO",
        [LL_WARNING] = "WARNING",
        [LL_ERROR] = "ERROR",
        [LL_RECOVERABLE] = "RECOVERABLE",
        [LL_ASSERT] = "ASSERT",
        [LL_TEST] = "TEST",
        [LL_ENUM_SIZE] = "<LL_ERROR>"
    };

    return FixStr_from_cstr(names[ll]);
}



// Updated log_message with colored output
void log_message(LogLevel level, FixStr format, ...) {
    if (level < 0 || level >= LL_ENUM_SIZE) {
        level = LL_ERROR;
    }

    const char *level_cols[]= {
        [LL_DEBUG] = ANSI_COL_BLUE,
        [LL_INFO] = ANSI_COL_CYAN,
        [LL_WARNING] = ANSI_COL_MAGENTA,
        [LL_ERROR] = ANSI_COL_RED,
        [LL_RECOVERABLE] = ANSI_COL_CYAN,
        [LL_ASSERT] = ANSI_COL_RED,
        [LL_TEST] = ANSI_COL_GREEN,
    };

    FixStr level_str = FixStr_col_new(level_cols[level], ll_nameof(level));
    if (FixStr_is_empty(level_str)) {
        clib_fprintf_safe(stderr, "[%.*s] ", fmt(ll_nameof(level)));
    } else {
        clib_fprintf_safe(stderr, "[%.*s] ", fmt(level_str));
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format.cstr, args);
    va_end(args);
    clib_fprintf_safe(stderr, "\n");

    #ifdef DEBUG_ABORT_ON_ERROR
        if(level == LL_ERROR) { abort(); }
    #endif

    #ifdef DEBUG_ABORT_ON_ASSERT
        if(level == LL_ASSERT) { abort(); }
    #endif
}



#pragma endregion

#pragma region DebugH
/// @brief Behaviour which will compile-away given non-debug state
#define NOOP ((void) 0)

#ifdef DEBUG_LOGGING_ON
    /// file location information is suppressed w/o debug
    #define sMSG(message) \
        s(__FILE__ ":" _STR(__LINE__) ": " message)

    /// @todo unused
    // static inline FixStr FixStr_debug_message(const char *message, const char *func, const char *file, int line) {
    //     static char buffer[256];
    //     snprintf(buffer, sizeof(buffer), "%s (%s) %s:%d", message, func, file, line);
    //     return (FixStr){ .cstr = buffer, .size = strlen(buffer) };
    // }

    // #define sBUF(message) FixStr_debug_message(message, __func__, __FILE__, __LINE__)

    #define debugger\
        FixStr_println(sMSG("<- Debugger: HERE! ->\n\n")); \
        abort()

    #define watch(var, cond) ((cond) ? (abort(), var) : var)

    #define LOG(format, ...) \
        log_message(LL_DEBUG, sMSG(format), __VA_ARGS__)

    #define log_assert(condition, message)\
        if(!(condition)) {log_message(LL_ASSERT, s("%.*s FAIL: %s"), fmt(message), _STR(condition));}

#else
    #define LOG(level, format, ...) NOOP
    #define sMSG(message) s(message)
    #define watch(var, cond) var
    #define debugger NOOP
    #define log_assert(condition, ...) NOOP
#endif


#ifdef DEBUG_REQUIRE_ASSERTS
    #define require(condition) assert(condition)
    #define require_not_null(expr) assert(((void*)expr) != nullptr)
    #define require_positive(expr) assert((expr) > 0)
    #define require_null(expr) assert((expr) == nullptr)
#else
    /// @todo swap out all checks
    #define require_not_null(expr)
#endif





#pragma endregion

#pragma region ArenaH


/// @brief Arena-based memory management
/// @note This is a simple memory management system which allocates memory in blocks
///... and frees all memory in a block at once.
///... ie., an arena is a pool of memory with a common lifetime, and all memory is
///... freed at once when the arena is reset or destroyed.
///... Used here for compiler-internal, user: globals, data, and per-module globals.


typedef struct Arena {
    void **blocks;
    size_t *block_sizes;  // Array to track sizes of blocks
    size_t block_size;
    size_t used;
    size_t num_blocks;
    bool is_live;
    LifetimeType lifetime;
} Arena;

#define Arena_is_forever(a_ptr) ((a_ptr)->lifetime == LIFETIME_FOREVER)

#define Arena_MetaValue_init_new(typed_ptr, size)\
    Arena_MetaValue_new_data((MetaValue *) (typed_ptr), sizeof((typed_ptr)->data[0]), size);


unsigned char *Arena_save_cnew(Arena *a, size_t *out_size);
bool Arena_load_cnew(Arena *a, unsigned char *data, size_t size);

bool Arena_init(Arena *a, size_t block_size);
void Arena_MetaValue_new_data(MetaValue *array, size_t datum_size, size_t size);
void Arena_aro_free(void *ptr); //a no-op
void Arena_aro_free_underlying(Arena *a);
void *Arena_new(size_t size, MetaType type);
void *Arena_cextend(void *ptr, size_t old_size, size_t new_size);
void Arena_reset(Arena *a);
void Arena_rewind(Arena *a, size_t pAst_size);
void Arena_print_stats(Arena *a);
// void Arena_test_main();


#define ARENA_1MB (1024 * 1024)

#pragma endregion

#pragma region ErrorH

#define require_index_bounded(index, limit) \
if ((index) >= (limit)) log_message(LL_ERROR, sMSG("Index out of bounds: %zu >= %zu"), (size_t)(index), (size_t)(limit))

#define require_enum_bounded(enum_val, max_val) \
if ((enum_val) < 0 || (enum_val) >= (max_val)) log_message(LL_ERROR, sMSG("Enum out of range: %d"), (int)(enum_val)); \


#define require_resources_cleanup(...) \
    void *_NULLables[] = {__VA_ARGS__};\
    for(size_t i = 0; i < sizeof(_NULLables) / sizeof(_NULLables[0]); i++) {\
        if(_NULLables[i] != nullptr) \
            log_message(LL_ERROR, sMSG("Resource not freed: %p"), _NULLables[i]);\
    }
#define error_oom() log_message(LL_ERROR, sMSG("Allocation failed!"))
#define error_never() log_message(LL_ERROR, sMSG("Unreachable!"))

#pragma endregion

#pragma region TracebackH

typedef struct TraceEntry {
    FixStr fn_name;
    FixStr argument;
    FixStr file;
    size_t line;
} TraceEntry;

typedef struct CallStack {
    TraceEntry data[512];
    size_t top;
} CallStack;


void Tracer_pop(CallStack *traces);
void Tracer_print_stack(CallStack *ts, size_t depth);
void Tracer_push(CallStack *traces, FixStr fn_name, FixStr arg, FixStr file, size_t line);

#pragma endregion

#pragma region ArrayHelperH
/// @todo: maybe, _SIZE_SMALL_BRIEF, and _LONGLIVE = 32, 16
#define ARRAY_SIZE_SMALL 16
#define ARRAY_SIZE_MEDIUM 256


#define ARRAY_REQUIRE_CHECKS

#ifdef ARRAY_REQUIRE_CHECKS
    #define require_small_array(count)\
        log_assert(count <= ARRAY_SIZE_SMALL, sMSG("Small array beyond capacity."));

    #define require_medium_array(count)\
        log_assert(count <= ARRAY_SIZE_MEDIUM, sMSG("Medium array beyond capacity."));

    #define require_not_null_array(data)\
        log_assert(data != nullptr, sMSG("Array is nullptr."));

    #define require_size_bounded(size, index)\
        log_assert(index < size, sMSG("Index out of bounds."));

    #define require_capacity_bounded(capacity, index)\
        log_assert(index < capacity, sMSG("Index out of bounds."));
#else
    #define require_small_array(count) NOOP
    #define require_medium_array(count) NOOP
    #define require_not_null_array(arr) NOOP
    #define require_size_bounded(arr, index) NOOP
    #define require_capacity_bounded(arr, index) NOOP
#endif


#define cnew_carray(arr, arr_cap) \
    (arr)->meta.alloc_size = arr_cap * sizeof(typeof((arr)->data[0])); /* NOLINT */\
    (arr)->meta.capacity = arr_cap; \
    (arr)->meta.size = 0;\
    (arr)->data = cnew((arr)->meta.alloc_size); \
    if(!(arr)->data) { require_resources_cleanup((arr)->data); error_oom(); }\
    require_safe(clib_memset_zero_safe((arr)->data, (arr)->meta.alloc_size, (arr)->meta.alloc_size));

/// @todo -- just remove the ref versions and accept pointers in all cases

#define len(arr) (arr).meta.size
#define cap(arr) (arr).meta.capacity

#define push(arr, elem) \
    (arr).data[(arr).meta.size++] = elem

#define pop(arr) \
    (arr).data[--(arr).meta.size]

#define last(arr) \
    (arr).data[(arr).meta.size - 1]

#define first(arr) \
    (arr).data[0]

#define set_zero_size(arr) \
    (arr).meta.size = 0

#define is_empty(arr) \
    (arr).meta.size == 0

#define is_full(arr) \
    (arr).meta.size == (arr).meta.capacity

#define at(arr, index) \
    (require_not_null((arr).data), (arr).data[index])

#define set(arr, index, value) \
    (require_not_null((arr).data), (arr).data[index] = value)


#define len_ref(arr) \
    (arr)->meta.size

#define capacity_ref(arr) \
    (arr)->meta.capacity

#define incr_len_ref(arr) \
    (arr)->meta.size++

#define decr_len_ref(arr) \
    (arr)->meta.size--

#define push_ref(arr, elem) \
    (arr)->data[(arr)->meta.size++] = elem

#define pop_ref(arr) \
    (arr)->data[--(arr)->meta.size]

#define last_ref(arr) \
    (arr)->data[(arr)->meta.size - 1]

#define first_ref(arr) \
    (arr)->data[0]

#define set_zero_len_ref(arr) \
    (arr)->meta.size = 0

#define is_empty_ref(arr) \
    (arr)->meta.size == 0

#define at_ref(arr, index) \
    (arr)->data[index]

#define set_ref(arr, index, value) \
    (arr)->data[index] = value

// typedef struct IndexSlice {
//     size_t start;
//     size_t end;
// } IndexSlice;

// typedef struct GenericSlice {
//     void *start;
//     void *end;
// } GenericSlice;

// typedef struct Array {
//     void *data;
//     size_t size;
//     size_t capacity;
//     size_t elem_size;
// } GenericArray;

// typedef struct ArraySlice {
//     void *data;
//     size_t size;
//     size_t elem_size;
// } GenericArraySlice;

// typedef struct FatPtr {
//     void *data;
//     size_t size;
// } FatPtr;
#pragma endregion

#pragma region HeapAndMemoryAllocators

/// @brief Global memory context
/// @todo revisit this
/// eg., do we need an internal context to point to this?
///     ... or can we just inline on the internal context?

/// @todo
/// ar_compiler, maybe reset after each file compilation?


#define DEBUG_MEMORY

#ifdef DEBUG_MEMORY
    //  ctx_debug_memory() = {0};


    //__glo_GlobalContext.aro_free(ptr)
    #define aro_free(ptr) NOOP
    #define Arena_alloc(size) Arena_new(size, 0)
    #define Heap_alloc(size) Heap_cnew(size, 0)

    #define ctx_debug_memory()\
        __glo_GlobalContext.debug.os_memory

    #define ctx_allocators()\
        __glo_GlobalContext.allocators

    #define aro_new(boxed_type) \
        ctx_allocators().aro_new(sizeof(boxed_type##_T), boxed_type)
    #define gco_new(boxed_type) \
        ctx_allocators().gco_new(sizeof(boxed_type##_T), boxed_type)

    #define ctx_debug_memory_reset() \
        ctx_debug_memory().num_allocations = 0; \
        ctx_debug_memory().is_free = false;

    /// @todo -- instead, have alloca as an a-allocator
    #define stack_new(alloc_size) \
        ((log_message(LL_DEBUG, sMSG("Allocating %zu bytes (alloca)"), (alloc_size)), \
        ctx_debug_memory().allocations[ctx_debug_memory().num_allocations++] = alloca(alloc_size)))

    #define cnew(alloc_size) \
        ((log_message(LL_DEBUG, sMSG("Allocating %zu bytes (malloc)"), (alloc_size)), \
        ctx_debug_memory().allocations[ctx_debug_memory().num_allocations++] = malloc(alloc_size)))


    #define cextend(ptr, alloc_size) \
        (log_message(LL_DEBUG, sMSG("Reallocating %zu bytes (realloc)"), (alloc_size)), \
        ctx_debug_memory().allocations[ctx_debug_memory().num_allocations++] = realloc(ptr, alloc_size))

    #define cfree(ptr) \
        (log_message(LL_DEBUG, sMSG("Freeing memory: %p"), ptr), \
        ctx_debug_memory().is_free = true, \
        free(ptr))

    #define cfree_log_leaks() \
        for(size_t i = 0; i < ctx_debug_memory().num_allocations; i++) { \
            if(ctx_debug_memory().is_free) { \
                log_message(LL_DEBUG, sMSG("Freed memory: %p"), ctx_debug_memory().allocations[i]); \
            } else { \
                log_message(LL_DEBUG, sMSG("Leaked memory: %p"), ctx_debug_memory().allocations[i]); \
            } \
        }

#else
    #define cnew(count, size) calloc(count, size)
    #define cextend(ptr, count_size) realloc(ptr, count_size)
    #define cfree(ptr) free(ptr)
    #define cfree_log_leaks() NOOP
#endif


#define HEAP_BASE_CAPACITY (8 * (1022 * 1024)) // 8 MB
#define HEAP_GC_1MB (1024 * 1024) // 1MB
#define HEAP_GC_2GB (2 * 1024 * 1024 * 1024) // 2 Gb
#define HEAP_GC_PREALLOC_RATIO (1.25)
#define HEAP_GC_DEFAULT_THRESHOLD (256 * HEAP_GC_1MB)


// Error codes for GC operations
typedef enum GCError {
    GC_ZERO = 0,
    GC_SUCCESS = 1,
    GC_ERR_HEAP_FULL,
    GC_ERR_INVALID_POINTER,
    GC_ERR_INVALID_TYPE,
    GC_ERR_ENUM_SIZE
} GCError;

// Forward declaration
struct Heap;

// FixStructure representing an object in the heap

/// @note data/payload attributes



// FixStructure representing the heap
typedef struct Heap {
    MetaData meta;
    MetaValue *free_list;      // Linked list of free data
    MetaValue **data;       // Dynamic array of all allocated data
    size_t gc_threshold;        // Threshold to trigger GC
    size_t generation_count;    // Track GC cycles
} Heap;


void *Heap_cnew(size_t alloc_size, MetaType type);
void Heap_data_cnew(size_t initial_capacity, size_t gc_threshold);
void Heap_destroy(Heap *heap);
void Heap_gc_aro_free(MetaValue *obj);

/// @brief Create a new GC thread
///     ... since GC can stop-the-world, we run it in a separate thread
///     ... to avoid blocking the main thread
/// @param heap
// void Heap_gc_thread_create(Heap *heap);
// void Heap_gc_thread_destroy(Heap *heap);
// void Heap_gc_mark(Heap *heap, MetaValue *obj);
// void Heap_gc_sweep();



#pragma endregion

#pragma region ThreadManagementH

/// @brief Thread management
/// ... we use the system threads for now, but we can switch to a custom thread pool later
/// ... for now, we just need to manage the main thread, the GC thread, and the UI thread
/// ... the main thread is the main execution thread
/// ... the GC thread is the garbage collection thread
/// ... the UI thread is the user interface thread (used by user code)



typedef struct Thread {
    pthread_t id;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned int flags;
} Thread;



/// @todo -- get this from the system
#define CPU_CORES 4

#pragma endregion

#pragma region GlobalContext


/// @brief
/// @todo perhaps allocate on ar_native and free in bulk?
typedef struct SourceInfo {
    FixStr name;
    FixStr source_file;
    FixStr source_code;
    size_t start_line_num;
    size_t end_line_num;
} SourceInfo;

typedef struct ParseContext {
    struct Source {
        struct FixStr *lines;
        size_t size;
    } source;

    struct Lexer {
        MetaData meta;
        struct Token *data;
        FixStr      indent;
    } lexer;

    struct Parser {
        MetaData meta;
        struct Ast *data;
        size_t depth;
        size_t current_token;
        CallStack traces;
    } parser;
} ParseContext;

struct GlobalContext {

    struct {
        Arena *current;
        Arena compiler;
        Arena interpreter_global;
        Arena interpreter_local;
        Arena gc_backing;
    } arenas;

    struct {
        Heap *current;
        Heap interpreter;
    } heaps;

    struct {
        Thread *main;
        Thread *gc;
        Thread *ui;
        Thread *compute[CPU_CORES];
    } threads;


    struct {
        /// @note Arena
        void *(*aro_new)(size_t size, MetaType type);
        void (*aro_free)(void *); /// no-op

        /// @note Heap (GC)
        void *(*gco_new)(size_t size, MetaType type);
        void (*collect)(void *);

    } allocators;

    struct {
        SourceInfo sources[128];
        size_t num_sources;
        FixStr code_examples[ARRAY_SIZE_SMALL];
        size_t num_code_examples;
        FixStr *str_precache;
        MetaType *boxed_precache;
    } info;

    struct {
        struct LastTime {
            struct timespec start;
            struct timespec end;
        } last_time;


        struct GlobalPerfMetrics *perf_metrics;
        size_t num_metrics;
        struct CallStack *callstack;
        FixError errors[ARRAY_SIZE_SMALL]; // @todo -- make this a dynamic array
        size_t num_errors;

        struct {
            void *allocations[1024];
            size_t num_allocations;
            FixStr line;
            bool is_free;
        } os_memory;
    } debug;

    ParseContext pctx;

    struct {
        bool _none;
    } interpreter;



} ctx() = {0};




void GlobalContext_setup(void) {
    ctx_setup(&ctx(), nullptr, nullptr);
    ctx_parser_setup(ctx_parser());
}


// void *ctx_Arena_cextend(void *mem, size_t old_size, size_t new_size) {
//     return Arena_cextend(ctx().arena, mem, old_size, new_size);
// }




#pragma endregion

#pragma region TimerH




void LastTime_start(void) {
    clock_gettime(CLOCK_MONOTONIC, &ctx_last_time().start);
}

void LastTime_end(void) {
    clock_gettime(CLOCK_MONOTONIC, &ctx_last_time().start);
}

double LastTime_elapsed_ms(void) {
    return 1000 * (
        (double) (ctx_last_time().end.tv_sec - ctx_last_time().start.tv_sec) +
        (double) (ctx_last_time().end.tv_nsec - ctx_last_time().start.tv_nsec) / 1e9
    );
}


#pragma endregion

#pragma region PerformanceAndProfilingH

/// @note profiling prototypes
// void perf_LastTime_start();
// void perf_LastTime_end();
// double perf_LastTime_elapsed_ms();



#pragma endregion

#pragma region CliH

typedef struct CliOption {
    FixStr name;
    FixStr value;
    FixStr or_else;
    bool is_set;
} CliOption;

void cli_parse_args(int argc, char **argv, CliOption *out_opts, size_t num_opts);
#define cli_parse_opts(opts) (cli_parse_args(argc, argv, opts, sizeof(opts) / sizeof(CliOption)))
#define cli_opt_flag(o_name) (CliOption)\
    {.name = o_name, .value = FixStr_empty(), .or_else = FixStr_empty(), .is_set = false}

#define cli_opt_default(o_name, o_or_else) (CliOption)\
    {.name = o_name, .value = FixStr_empty(), .or_else = o_or_else, .is_set = false}

#define cli_opt_get(options, index)\
 (options[index].is_set ? options[index].value : options[index].or_else)


#pragma endregion

#pragma region JsonH
#pragma endregion

#pragma region UnitTestH

typedef struct TestReport {
    FixStr messages[32];
    bool results[32];
    size_t num_results;
} TestReport;

typedef void (*UnitTestFnPtr)(TestReport *r);

typedef struct TestSuite {
    UnitTestFnPtr *tests;
    TestReport reports[32];
    size_t num_tests;
} TestSuite;

// void TestSuite_new(TestSuite *ts, UnitTestFnPtr *tests, size_t num_tests);
void TestSuite_new(TestSuite *ts, UnitTestFnPtr *tests, void *_end);
void TestReport_push(TestReport *r, bool result, FixStr message);
void TestSuite_print(TestSuite *ts);
void TestSuite_run(TestSuite *ts);
void TestSuite_register(TestSuite *ts,  UnitTestFnPtr fnptr);

#define _NUM_TEST_ARGS(...) ( sizeof( (UnitTestFnPtr []){__VA_ARGS__} ) / sizeof(UnitTestFnPtr) )

#define test_suite(...)     {\
    .num_tests = _NUM_TEST_ARGS(__VA_ARGS__), \
    .tests = ((UnitTestFnPtr[]) { __VA_ARGS__ })\
}


#define TEST_STR(expr) "(" _STR(expr) ")"

#define test_assert(report, expr, message) \
    TestReport_push(report, (bool) (expr), s(message TEST_STR(expr)))

#pragma endregion

//#endregion
/// ---------- ---------- ---------- HEADERS: FLEX ---------- ---------- --------- ///
//#region Headers_FlexibleDataTypes

#pragma region FlxObjectH

/// @brief
/// @todo consider whether there should be object types
///     .... eg., class data, internal data... cf. PyObject
typedef struct FlxObject {
    MetaData meta;

    /// @todo -- maybe de-nest
    FixStruct *data;
} FlxObject;



// FlxObject functions
FixStr FlxObject_to_FixStr(FlxObject *obj);
Box FlxObject_getattr(FlxObject *obj, FixStr name);
Box FlxObject_getattr_checked(FlxObject *obj, FixStr name, MetaType expected_type);
Box FlxObject_find_method(Box obj, FixScope *scope, FixStr name);
FlxObject *FlxObject_new(FixStruct *def);
FlxObject *FlxObject_from_FixDict(FixStruct *def, FixDict fields);
void FlxObject_set_field(FlxObject *obj, FixStr field, Box value);
bool FlxObject_get_field(FlxObject *obj, FixStr field, Box *out_value);
#pragma endregion

#pragma region FlxStrH

// FixStructure representing a dynamic string
typedef struct FlxStr {
    MetaData meta;
    char *cstr;       // Pointer to the string data (nullptr-terminated)
} FlxStr;

FlxStr *FixStr_flexible_new(FixStr str);



// FlxStr functions
FlxStr* FlxStr_init(struct Heap *heap, const char *initial_str);
Box FlxStr_join_new(FlxStr *a, char *data, size_t len);
const char* FlxStr_to_cstr(const FlxStr *dyn_str);
Box FlxStr_set(FlxStr *dyn_str, const char *str);
void FlxStr_aro_free(FlxStr *dyn_str);
void FlxStr_print(const FlxStr *dyn_str);

#pragma endregion

#pragma region FlxArrayH


/// @brief A generic collection type
/// @note Supports vector (random access), set, queue and stack APIs
///     ... API methods check the `type` to ensure its correct
typedef struct FlxArray {
    MetaData meta;
    Box *data;
} FlxArray;


// Additional FlxArray functions
FlxArray* FixArray_init(size_t initial_capacity);
Box FlxArray_to_set(FlxArray *array);
Box FlxArray_to_stack(FlxArray *array);
Box FlxArray_to_queue(FlxArray *array);
Box FlxArray_to_vector(FlxArray *array);
Box FlxArray_to_FixArray(FlxArray *array);
Box FlxArray_clear(FlxArray *array);
Box FlxArray_contains_byhash(FlxArray *array);
Box FlxArray_contains_byvalue(FlxArray *array);
Box FlxArray_append_first(FlxArray *array);
Box FlxArray_append_last(FlxArray *array);
Box FlxArray_append_offer(FlxArray *array, Box element);
Box FlxArray_append_unique(FlxArray *array, Box element);
Box FlxArray_set_add(FlxArray *array, size_t index, Box element);
Box FlxArray_set_overwrite(FlxArray *array, size_t index, Box element);
Box FlxArray_remove_weak_byindex(FlxArray *array, size_t index);
Box FlxArray_remove_weak_byvalue(FlxArray *array, Box element);
Box FlxArray_remove_weak_byhash(FlxArray *array, Box element);
Box FlxArray_remove_strong_byindex(FlxArray *array, size_t index);
Box FlxArray_remove_strong_byvalue(FlxArray *array, Box element);
Box FlxArray_remove_strong_byhash(FlxArray *array, Box element);
Box FlxArray_is_empty(FixArray *array);
Box FlxArray_first(FlxArray *array);
Box FlxArray_remove_first(FlxArray *array);
Box FlxArray_last(FlxArray *array);
Box FlxArray_remove_last(FlxArray *array);
Box FlxArray_indexof(FlxArray *array);


#pragma endregion

#pragma region FlxDictH


typedef struct FlxKvPair {
    size_t hash;
    Box key;
    Box value;
    bool occupied;
} FlxKvPair;

typedef struct FlxDict {
    MetaData meta;
    FlxKvPair *data;        // Array of dictionary data
} FlxDict;



// FlxDict functions
FlxDict *FlxDict_new(size_t initial_capacity);
bool FlxDict_find(FlxDict *dict, Box key, FlxKvPair *out);
void FlxDict_aro_free(FlxDict *dict);
Box FlxDict_set(FlxDict *dict, Box key, Box val);
FixStr FlxDict_to_FixStr(FlxDict *d);

#pragma endregion

#pragma region FlxQueueAndStackH
#pragma endregion

#pragma region FlxSetH

typedef struct FlxSet {
    MetaData meta;
    Box *data;          // Array of Box data for separate chaining
} FlxSet;



#pragma endregion

#pragma region FlxGraphH

typedef struct FixGraphNode {
    size_t hash;
    Box value;
    bool occupied;
} FixGraphNode;

typedef struct FixGraphEdge {
    size_t hash;
    Box source;
    Box target;
    Box weight;
    bool occupied;
} FixGraphEdge;

typedef struct FlxGraph {
    MetaData meta;
    FixGraphNode *nodes;
    FixGraphEdge *edges;
    size_t num_nodes;
    size_t num_edges;
} FlxGraph;
#pragma endregion

#pragma region FlxDataFrameH


typedef struct FlxDataFrame {
    MetaData meta;
    Boxed *columns;
} FlxDataFrame;


#pragma endregion

#pragma region FlxMatrixAndTensorH


typedef struct FlxMatrix {
    MetaData meta;
    Box *data;
    size_t rows;
    size_t columns;
} FlxMatrix;

typedef struct FlxMatrixF {
    MetaData meta;
    double *data;
    size_t rows;
    size_t columns;
} FlxMatrixF;

#pragma endregion

#pragma region FlxVectorsH


typedef struct FlxVecDouble {
    MetaData meta;
    double *data;
} FlxVecDouble;

typedef struct FlxVecInt {
    MetaData meta;
    int *data;
} FlxVecInt;

typedef struct FlxVecFixStr {
    MetaData meta;
    FixStr *data;
} FlxVecFixStr;

typedef struct FlxVecCat {
    MetaData meta;
    int *data;
    FixStr *categories;
} FlxVecCat;

typedef struct FlxVecChar {
    MetaData meta;
    char *data;
} FlxVecChar;

typedef struct FlxVecBool {
    MetaData meta;
    bool *data;
} FlxVecBool;

#pragma endregion

#pragma region BoxMacroTypeHelpers
/// @todo -- check which of these are not in use and remove

/// @brief Used to map box types to c-types at compile-time (ie., via macros).

typedef FixStr BXD_FIX_STR_T;
typedef FixArray BXD_FIX_ARRAY_T;
typedef FixDict BXD_FIX_DICT_T;
typedef FixKvPair BXD_FIX_KVPAIR_T;
typedef FixScope BXD_FIX_SCOPE_T;
typedef struct FixAst * BXD_FIX_AST_T;
typedef FixDist BXD_FIX_DISTRIBUTION_T;
// typedef Fix BXD_FIX_SYSTEM_INFO_T;
// typedef Fix BXD_FIX_MODULE_T;
typedef FixStruct BXD_FIX_STRUCT_T;
// typedef Fix BXD_FIX_ENUM_T;
// typedef Fix BXD_FIX_TRAIT_T;
// typedef Fix BXD_FIX_TYPE_T;
// typedef BXD_FIX_OBJECT BXD_FIX_OBJECT_T;
// typedef Fix BXD_FIX_ARRAY_VIEW_T;
// typedef Fix BXD_FIX_VEC_TAGS_T;
// typedef Fix BXD_FIX_NUM_128BIT_T;
typedef double BXD_FIX_NUM_DOUBLE_T;
// typedef Fix BXD_FIX_VEC_DOUBLE_1_T;
// typedef Fix BXD_FIX_VEC_DOUBLE_2_T;
// typedef Fix BXD_FIX_VEC_DOUBLE_3_T;
// typedef Fix BXD_FIX_VEC_DOUBLE_4_T;
typedef FlxObject BXD_FLX_OBJECT_T;
typedef FlxStr BXD_FLX_STR_T;
typedef FlxArray BXD_FLX_ARRAY_T;
typedef FlxDict BXD_FLX_DICT_T;
// typedef Flx BXD_FLX_QUEUE_T;
// typedef Flx BXD_FLX_QUEUE_PRIORITY_T;
// typedef Flx BXD_FLX_STACK_T;
// typedef Flx BXD_FLX_SET_T;
// typedef Flx BXD_FLX_GRAPH_DIRECTED_WEIGHTED_T;
// typedef Flx BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED_T;
// typedef Flx BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED_T;
// typedef Flx BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED_T;
// typedef Flx BXD_FLX_VEC_DOUBLE_N_T;
// typedef Flx BXD_FLX_MATRIX_T;
// typedef Flx BXD_FLX_MATRIX_DOUBLE_T;
// typedef Flx BXD_FLX_TENSOR_T;
// typedef Flx BXD_FLX_TENSOR_DOUBLE_T;
// typedef Flx BXD_FLX_DATAFRAME_T;
typedef FlxVecBool BXD_FLX_VEC_BOOL_T;
typedef FlxVecInt BXD_FLX_VEC_INT_T;
// typedef Flx BXD_FLX_VEC_TAGS_T;
typedef FlxVecFixStr BXD_FLX_VEC_STR_T;
typedef FlxVector BXD_FLX_VEC_OBJS_T;


typedef void * UBX_NULL_T;
typedef bool UBX_BOOL_T;
typedef int64_t UBX_INT_T;
typedef uint64_t UBX_UINT_T;
typedef uint8_t UBX_BYTE_T;
typedef char UBX_CHAR_T;
typedef float UBX_FLOAT_T;
typedef char * UBX_TAG_T;
typedef uint64_t UBX_FLAGS_T;
typedef void * UBX_PTR_T;
typedef bool UBX_EMPTY_UBX_END_T;
typedef uint64_t UBX_STATE_T;
typedef Boxed * UBX_PTR_ARENA_T;
typedef Boxed * UBX_PTR_HEAP_T;
typedef FixError * UBX_PTR_ERROR_T;

#define Box_ctypeof(MetaTypeName) MetaTypeName##_T
#define Box_unwrap(MetaTypeName, box) ((MetaTypeName##_T) box.payload)
#define Box_wrap(box_type, box_payload) ((Box) \
    {.payload = ((MetaTypeName##_T) box_payload), .type = box_type}))

//// @todo -- replace with wrap/unwrap functions

#define Box_unwrap_int(box) ((int64_t) box.payload)
#define Box_unwrap_bool(box) ((bool) (box.payload == 1))
#define Box_unwrap_typed_ptr(type, box) ((type *) box.payload)
#define Box_unwrap_ptr(box) (box.payload)
#define Box_is_bool(box) (box.type == UBX_BOOL && (box.payload == 1 || box.payload == 0))



#define Box_unwrap_double(box) *Box_unwrap_typed_ptr(double, box)
#define Box_unwrap_FixError(box) Box_unwrap_typed_ptr(FixError, box)
#define Box_unwrap_FixStr(box) *Box_unwrap_typed_ptr(FixStr, box)
#define Box_unwrap_FixArray(box) Box_unwrap_typed_ptr(FixArray, box)
#define Box_unwrap_FixDict(box) Box_unwrap_typed_ptr(FixDict, box)
#define Box_unwrap_FixIter(box) Box_unwrap_typed_ptr(FixIter, box)
#define Box_unwrap_FixFn(box) Box_unwrap_typed_ptr(FixFn, box)
#define Box_unwrap_FixObject(box) Box_unwrap_typed_ptr(FlxObject, box)
#define Box_unwrap_FixStruct(box) Box_unwrap_typed_ptr(FixStruct, box)
#define Box_unwrap_FixScope(box) Box_unwrap_typed_ptr(FixScope, box)
#define Box_unwrap_Boxed(box) Box_unwrap_typed_ptr(Boxed, box)


#define Box_wrap_int(int_value) ((Box) {.payload = (uint64_t) int_value, .type=UBX_INT})
#define Box_wrap_bool(bool_value) ((Box) {.payload = (uint64_t) (bool_value), .type=UBX_BOOL})
#define Box_wrap_typed_ptr(ptr_type, ptr_value) ((Box) {.payload = (uint64_t) ptr_value, .type=ptr_type})
#define Box_wrap_ptr(ptr_value) Box_wrap_typed_ptr(UBX_PTR, ptr_value)

#define Box_wrap_FixError(ptr) Box_wrap_typed_ptr(UBX_PTR_ERROR, ptr)
#define Box_wrap_BoxedArena(ptr) Box_wrap_typed_ptr(UBX_PTR_ARENA, ptr)
#define Box_wrap_BoxedHeap(ptr) Box_wrap_typed_ptr(UBX_PTR_HEAP, ptr)



static inline Box Box_wrap_float(float float_value) {
  uint64_t payload = 0;
  /// @note int clib_memcpy_safe(void *dest, size_t dest_size, const void *src, size_t count)
  clib_memcpy_safe(&payload, sizeof(float), &float_value, sizeof(float));
  return (Box){.payload = payload, .type = UBX_FLOAT};
}

static inline float Box_unwrap_float(Box box) {
    float float_value = 0;
    /// @note this doesn't work because we cannot take the address of a bitfield
    /// clib_memcpy_safe(&float_value, sizeof(float), &box.payload, sizeof(float));
    /// so instead we use a temporary variable to copy the payload and then copy it to the float value
    /// this is a workaround for the above issue
    uint64_t payload = box.payload;
    clib_memcpy_safe(&float_value, sizeof(float), &payload, sizeof(float));

    return float_value;
}


Box Box_wrap_tag(FixStr tag_string) {
    Box box = {0}; // Initialize all fields to 0

    // Ensure the tag size does not exceed 11 characters
    // if (tag_string.size > 11) {
    //     native_error("Tag '%.*s' exceeds maximum size of 11 characters (truncating).", fmt(tag_string));
    // }

    size_t max_size = tag_string.size > 11 ? 11 : tag_string.size;
    uint64_t encoded = 0;

    for (size_t i = 0; i < max_size; ++i) {
        char c = toupper(tag_string.cstr[i]); // NOLINT(*-narrowing-conversions)
        int value = char_to_base37(c);
        encoded = encoded * 37 + value;
    }

    box.payload = encoded;
    box.type = UBX_TAG;
    return box;
}

FixStr Box_unwrap_tag(Box tag) {
    uint64_t encoded = tag.payload;
    char temp[12];
    size_t index = 0;

    while (encoded > 0 && index < 11) {
        uint8_t value = encoded % 37;
        encoded /= 37;
        temp[index++] = base37_to_char(value);
    }

    if (index == 0) {
        // native_error("Tags cannot be empty.");
        temp[index++] = '_'; // Assuming '_' represents an empty tag
    }

    // Reverse the characters since the least significant digit was decoded first
    for (size_t i = 0; i < index / 2; ++i) {
        char swap = temp[i];
        temp[i] = temp[index - i - 1];
        temp[index - i - 1] = swap;
    }

    temp[0] = '#';
    return FixStr_new(temp, index);
}


bool Box_tag_eq(Box tag, FixStr value) {
    if (tag.type != UBX_TAG) {
        return false;
    }

    constexpr size_t MAX_LENGTH = 11;
    uint64_t encoded = 0;
    size_t encode_size = value.size > MAX_LENGTH ? MAX_LENGTH : value.size;

    for (size_t i = 0; i < encode_size; ++i) {
        char c = toupper(value.cstr[i]); // NOLINT(*-narrowing-conversions)
        int val = char_to_base37(c);
        encoded = encoded * 37 + val;
    }

    // If the provided string has fewer than MAX_LENGTH characters,
    // it's implicitly padded with zeros (leading characters treated as lower significance)
    // To ensure consistent encoding, we need to shift the encoded value accordingly.
    // However, since encoding processes from left to right, shorter strings have smaller encoded values.

    // To accurately compare, encode the provided string and compare directly
    return (encoded == tag.payload);
}



double Box_force_numeric(Box o) {
    if(o.type == UBX_FLOAT) {
        return Box_unwrap_float(o);
    } else if(Box_is_Boxed(o)) {
        if(Box_Boxed_meta(o).type == BXD_FIX_NUM_DOUBLE) {
            return *(double *) o.payload;
        } else if(Box_Boxed_meta(o).type == BXD_FIX_NUM_128BIT) {
            return *(double *) o.payload;
        }
    } else if(o.type == UBX_TAG) {
        return FixStr_hash(Box_unwrap_tag(o));
    }

    return (double) o.payload;
}

#define box(box, ub_type) Box_unwrap_##ub_type(Box_unwrap_typed_ptr(box))

#define Box_typeof(box) ubx_nameof(box.type)

#define Box_null()  ((Box) {.payload = (uint64_t) 0, .type = UBX_NULL})
#define Box_empty()  ((Box) {.payload = (uint64_t) 0, .type = UBX_EMPTY_UBX_END})
#define Box_state(cs_type) ((Box) {.payload = (uint64_t) cs_type, .type = UBX_STATE})
#define Box_exit() Box_state(BXS_EXIT)
#define Box_true()  ((Box) {.payload = (uint64_t) 1, .type = UBX_BOOL})
#define Box_false() ((Box) {.payload = (uint64_t) 0, .type = UBX_BOOL})
#define Box_error_empty() ((Box) {.payload = (uint64_t) 0, .type = UBX_PTR_ERROR})
#define Box_done() Box_state(BXS_DONE)

// #define Box_is_ptr(box) (box.type == UBX_PTR)
#define Box_is_error(box) (box.type == UBX_PTR_ERROR)
#define Box_is_null(box) (box.type == UBX_NULL && ((void*)box.payload) == nullptr)
#define Box_is_state(box, cs_type) (box.type == UBX_BOOL && box.payload == cs_type)
#define Box_is_state_end(box) (box.type == UBX_BOOL && \
    box.payload == BXS_BREAK || box.payload == BXS_RETURN || box.payload == BXS_YIELD)

#define Box_is_hashof(boxTag, tagFixStr) (boxTag.type == UBX_INT && ((size_t) Box_unwrap_int(boxTag) == FixStr_hash(tagFixStr)))

static inline bool Box_is_truthy(Box box) {
    return
        (box.type == UBX_BOOL && Box_unwrap_bool(box) != false) ||
        (box.type == UBX_INT && Box_unwrap_int(box) != 0) ||
        (box.type == UBX_FLOAT && Box_unwrap_float(box) != 0.0 )
        ||  false;
        // (box.type == UBX_PTR && box.payload != 0)
}



bool Box_is_object(Box b) {
    return Box_is_Boxed(b) && Box_Boxed_meta(b).type == BXD_FLX_OBJECT;
}

bool Box_is_numeric(Box o) {
    return (o.type == UBX_INT || o.type == UBX_FLOAT) ||
           (Box_is_Boxed(o) && (
                Box_Boxed_meta(o).type == BXD_FIX_NUM_DOUBLE
            ||  Box_Boxed_meta(o).type == BXD_FIX_NUM_128BIT)
    );
}


/// @todo -- remove these
#define dyn_size(dc) (len_ref(dc))
#define dyn_capacity(dc) (capacity_ref(dc))
#define dyn_type(dc) (dc->meta.type)





FlxVecCat FlxVecCat_new(size_t num_categories);
FlxDict FlxVecCat_histogram(FlxVecCat *cat);
FixStr FlxVecCat_to_FixStr(FlxVecCat *cat);


Box FixIter_update(FixIter *iter, Box result);
Box FixIter_get(FixIter *iter);


#define FixFnFromNative(fn_type, fn_name, fn_native_ptr) (FixFn) {\
    .type = fn_type, .name = fn_name,\
    .fnptr = (void *)fn_native_ptr, .code = nullptr, \
    .signature = {0}, .enclosure = {0} }

// typedef Box (*FixFnUser)(FixFn *fn, FixScope *parent, FixArray *args);
typedef Box (*FixFnGenerator)(FixFn *fn, FixScope parent, FixArray args, FixIter iter);
// typedef Box (*FixFnAsync)(FixFn *fn, FixScope *parent, FixArray *args);
// typedef Box (*FixFnMacro)(FixFn *fn, FixScope *parent, FixArray *args);
typedef Box (*FixFnUser)(FixFn *fn, FixScope parent, FixArray args);
typedef Box (*FixFnNative)(FixFn *fn, FixScope parent, FixArray args);
typedef Box (*FixFnNative0)(FixFn *fn, FixScope parent);
typedef Box (*FixFnNative1)(FixFn *fn, FixScope parent, Box one);
typedef Box (*FixFnNative2)(FixFn *fn, FixScope parent, Box one, Box two);
typedef Box (*FixFnNative3)(FixFn *fn, FixScope parent, Box one, Box two, Box three);


#define FixFn_typed_call(type, ffn, ...) (((type) ffn->fnptr)(ffn, __VA_ARGS__))


#define FixFn_is_native(ffn) (ffn->type == FN_NATIVE) ||     \
    (ffn->type == FN_NATIVE_0) || (ffn->type == FN_NATIVE_1) \
    (ffn->type == FN_NATIVE_2) || (ffn->type == FN_NATIVE_3)



// typedef struct BoxGenFn  {
//     FixFn next;
//     FixIter iter;
// } BoxGenFn;
#pragma endregion

//#endregion
/// ---------- ---------- ---------- HEADERS: PARSING ---------- ---------- ------ ///
//#region Headers_Parsing

#pragma region ParsingTokenH

typedef enum TokenType {
    TT_INDENT = 0,
    TT_DEDENT,
    TT_END,
    TT_TAG,
    TT_IDENTIFIER,
    TT_TYPE,
    TT_ANNOTATION,
    TT_KEYWORD,
    TT_FLOAT,
    TT_DOUBLE,
    TT_INT,
    TT_BRA_OPEN,
    TT_BRA_CLOSE,
    TT_STRING,
    TT_ASSIGN,
    TT_OP,
    TT_SEP,
    TT_DISCARD,
    TT_IGNORE,
    TT_DEREF,
    TT_DOC_COMMENT,
} TokenType;


FixStr tt_nameof(TokenType tt) {
    static const char *names[] = {
        [TT_INDENT] = "token(indent)",
        [TT_DEDENT] = "token(dedent)",
        [TT_END] = "token(end)",
        [TT_TAG] = "token(tag)",
        [TT_IDENTIFIER] = "token(identifier)",
        [TT_TYPE] = "token(type)",
        [TT_ANNOTATION] = "token(type_annotation)",
        [TT_KEYWORD] = "token(keyword)",
        [TT_DOUBLE] = "token(double)",
        [TT_INT] = "token(int)",
        [TT_BRA_OPEN] = "token(open_bracket)",
        [TT_BRA_CLOSE] = "token(close_bracket)",
        [TT_STRING] = "token(string)",
        [TT_ASSIGN] = "token(assign)",
        [TT_OP] = "token(op)",
        [TT_SEP] = "token(sep)",
        [TT_DISCARD] = "token(discard)",
        [TT_IGNORE] = "token(ignore)",
        [TT_DEREF] = "token(deref)",
        [TT_DOC_COMMENT] = "token(doc_comment)",
    };

    return FixStr_from_cstr(names[tt]);
}

typedef struct Token {
    TokenType type;
    FixStr value;
    size_t line;
    size_t col;
} Token;


#define lex_print_all(lexer)\
    lex_print_data(lexer.data, 0, len(lexer), len(lexer)+1)


// void lexer_data_print(Token *data, size_t size);
// FixStr lexer_data_to_readable(Token *data, size_t size);
// FixStr lexer_data_to_json(Token *data, size_t size);

#pragma endregion

#pragma region ParsingAstH

typedef enum AstType {
    AST_ZERO,
    AST_DISCARD,
    AST_BOX,
    AST_EXPRESSION,
    AST_BOP,
    AST_UOP,
    AST_ID,
    AST_INT,
    AST_FLOAT,
    AST_DOUBLE,
    AST_STR,
    AST_TAG,
    // AST_CALL, // @todo what's this for?
    AST_FN_DEF_CALL,
    AST_METHOD_CALL,
    AST_MEMBER_ACCESS,
    AST_BLOCK,
    AST_FN_DEF,
    AST_FN_DEF_MACRO,
    AST_FN_DEF_ANON,
    AST_FN_DEF_GEN,
    AST_STRUCT_DEF,
    AST_OBJECT_LITERAL,
    AST_TRAIT,
    AST_IF,
    AST_MOD,
    AST_FOR,
    AST_LOOP,
    AST_LEF_DEF,
    AST_BINDING,
    AST_RETURN,
    AST_MATCH,
    AST_DICT,
    AST_VEC,
    AST_USE,
    AST_MODULE,
    AST_MUTATION,
    AST_CONST_DEF,
    AST_YIELD,
    AST_IGNORE,
    AST_TYPE_ANNOTATION,
    AST_DESTRUCTURE_ASSIGN,
    AST_GENERIC_TYPE,

    AST_RANGE_SUGAR,

    AST_ENUM_SIZE,
} AstType;


FixStr Ast_nameof(AstType ast) {

    if(ast >= AST_ENUM_SIZE || ast < 0) {
        return FixStr_from_cstr("ast(<ERROR>)");
    }

    static const char *names[] = {
        [AST_ZERO] = "ast(<ERROR(ZERO)>)",
        [AST_DISCARD] = "ast(discard)",
        [AST_BOX] = "ast(box<PARTIAL>)",
        [AST_EXPRESSION] = "ast(expression)",
        [AST_BOP] = "ast(bop)",
        [AST_UOP] = "ast(uop)",
        [AST_ID] = "ast(id)",
        [AST_INT] = "ast(int)",
        [AST_FLOAT] = "ast(float)",
        [AST_DOUBLE] = "ast(double)",
        [AST_STR] = "ast(str)",
        [AST_TAG] = "ast(tag)",
        // [AST_CALL] = "ast(call)",
        [AST_METHOD_CALL] = "ast(method_call)",
        [AST_MEMBER_ACCESS] = "ast(member_access)",
        [AST_BLOCK] = "ast(block)",
        [AST_FN_DEF] = "ast(fn)",
        [AST_FN_DEF_CALL] = "ast(call_fn)",
        [AST_FN_DEF_MACRO] = "ast(fn_macro)",
        [AST_FN_DEF_ANON] = "ast(fn_anon)",
        [AST_FN_DEF_GEN] = "ast(fn_gen)",
        [AST_STRUCT_DEF] = "ast(struct)",
        [AST_OBJECT_LITERAL] = "ast(object_literal)",
        [AST_TRAIT] = "ast(trait)",
        [AST_IF] = "ast(if)",
        [AST_FOR] = "ast(for)",
        [AST_LOOP] = "ast(loop)",
        [AST_LEF_DEF] = "ast(let)",
        [AST_BINDING] = "ast(binding)",
        [AST_RETURN] = "ast(return)",
        [AST_MATCH] = "ast(match)",
        [AST_DICT] = "ast(dict)",
        [AST_VEC] = "ast(vec)",
        [AST_USE] = "ast(use)",
        [AST_MODULE] = "ast(module)",
        [AST_MUTATION] = "ast(mutation)",
        [AST_CONST_DEF] = "ast(const)",
        [AST_YIELD] = "ast(yield)",
        [AST_IGNORE] = "ast(ignore)",
        [AST_TYPE_ANNOTATION] = "ast(type_annotation)",
        [AST_RANGE_SUGAR] = "ast(range_sugar)",
        [AST_ENUM_SIZE] = "ast(<ERROR(NUM_TYPES)>)"
    };

    return FixStr_from_cstr(names[ast]);
}

typedef struct Ast Ast;


#define Ast_require_type(node, ast, error_fn) \
    require_not_null(node); \
    require_positive(node->type); \
    error_fn

struct Ast {
    AstType type;
    size_t line;
    size_t col;
    union {
        /// @brief Sugar for range expressions
        /// @example
        /// `1 to 10` is sugar for `range(1, 10, 1)`
        /// @example
        /// `1 to 10 by 2` is sugar for `range(1, 10, 2)`
        /// @example
        /// `0 til 10` is sugar for `range(0, 9, 1)`
        struct AstRangeSugar {
            Ast *start;
            Ast *end;
            Ast *step;
        } range_sugar;
        // Box box;  /// @todo for compile-time partial eval of AST
        struct AstTypeAnnotation {
            FixStr qualifier;               // e.g., "ref", "mut", etc. (optional)
            FixStr base_type;               // Base type name, e.g., "Vec", "Dict"
            Ast *type_params[ARRAY_SIZE_SMALL];       // Array of type parameter AST nodes (optional)
            size_t num_type_params;    // Number of type parameters
            FixStr size_constraint;         // Size constraint, e.g., "32", "dyn", "?"
        } type_annotation;
        struct AstDestructureAssign {
            Ast **variables;
            size_t num_variables;
            Ast *expression;
        } destructure_assign;
        struct AstIdentifier {
            FixStr name;
        } id;
        struct AstDoubleValue {
            double value;
        } dble;
        struct AstIntValue {
            int value;
        } integer;
        struct AstFixStringValue {
            FixStr value;
        } str;
        struct {
            FixStr name;
        } tag;
        struct AstBOP {
            FixStr op;
            Ast *left;
            Ast *right;
        } bop;
        struct AstUOP {
            FixStr op;
            Ast *operand;
        } uop;
        struct AstFnCall {
            Ast *callee;
            Ast **args;
            size_t num_args;
        } call;
        struct AstMethodCall {
            Ast *target;
            FixStr method;
            Ast **args;
            size_t num_args;
        } method_call;
        struct AstMemberAccess {
            Ast *target;
            FixStr property;
        } member_access;
        struct AstBlock {
            Ast **statements;
            size_t num_statements;
            bool transparent;
        } block;
        struct AstFnDef {
            FixStr name;
            struct FnParam {
                FixStr name;
                Ast *type;
                Ast *default_value;
                // FixStr *tags;
                // size_t num_tags;
            } *params;
            size_t num_params;
            Ast *body;
            bool is_macro;
            bool is_generator;  // Add this flag
        } fn;
        struct AstFnAnon {
            struct AnonFnParam {
                FixStr name;
                FixStr type;
                Ast *default_value;
                // FixStr *tags;  /// @todo is this const?
                size_t num_tags;
            } *params;
            size_t num_params;
            Ast *body;
        } fn_anon;
        struct AstFixStructDef {
            FixStr name;
            struct FixStructField {
                FixStr name;
                // bool is_static;
                Ast *type;
                Ast *default_value;
            } *fields;
            size_t num_fields;
        } struct_def;
        struct AstObjectLiteral {
            FixStr struct_name;
            struct AstObjectField {
                FixStr name;
                Ast *value;
            } *fields;
            size_t num_fields;
        } object_literal;
        struct AstTraitDef {
            FixStr name;
            struct TraitMethod {
                FixStr name;
                FixStr *params;  /// @todo is this const?
                size_t num_params;
                Ast *body;
            } *methods;
            size_t num_methods;
        } trait;
        struct AstIf {
            Ast *condition;
            Ast *body;
            Ast *else_body;
        } if_stmt;
        struct AstLoop {
            Ast **bindings;
            size_t num_bindings;
            Ast *body;
            Ast *condition;
            bool yields;
        } loop;
        struct AstLet {
            Ast **bindings;
            size_t num_bindings;
            Ast *body;  /// @todo rename this
        } let_stmt;
        struct AstBinding {
            FixStr identifier;
            FixStr assign_op;
            Ast *expression;
        } binding;  /// @todo  :  go over all the inline bindings and replace with this
        struct AstReturn {
            Ast *value;
        } return_stmt;
        struct AstMatch {
            Ast *expression;
            struct MatchCase {
                Ast *condition;
                Ast *expression;
                FixStr kind; /// @todo -- remove
            } *cases;
            size_t num_cases;
        } match;
        struct AstDict {
            struct AstDictEntry {
                Ast *key;
                Ast *value;
            } *data;
            size_t size;
        } dict;
        struct AstVec {
            Ast **data;
            size_t size;
        } vec;  /// @todo  : change to vec
        struct AstUse {
            FixStr module_path;
            FixStr alias;
            bool wildcard;
        } use_stmt;
        struct AstMutation {
            Ast *target;
            FixStr op;
            Ast *value;
            bool is_broadcast;
        } mutation;
        struct AstConst {
            FixStr name;
            Ast *value;
        } const_stmt;
        struct AstYield {
            Ast *value;
        } yield_stmt;
        struct AstIgnore {
            Ast *nullptr_expression;
        } ignore_stmt;
        struct AstExpression {
            Ast *expression;
        } exp_stmt;
        struct AstModule {
            FixStr name;
        } mod_stmt;
    };
};


FixStr Ast_to_FixStr(Ast *node);
void Ast_print(Ast *node);
FixStr Ast_to_json(Ast *node, size_t indent) ;
FixStr FixAst_to_json(Ast *node, size_t indent) ;
FixStr Ast_dict_to_json(Ast *node, size_t indent) ;
FixStr Ast_vec_to_json(Ast *node, size_t indent) ;
FixStr Ast_use_to_json(Ast *node, size_t indent) ;
FixStr Ast_ignore_to_json(Ast *node, size_t indent) ;
FixStr Ast_yield_to_json(Ast *node, size_t indent) ;
FixStr Ast_expression_to_json(Ast *node, size_t indent) ;
FixStr Ast_module_to_json(Ast *node, size_t indent) ;
FixStr Ast_struct_def_to_json(Ast *node, size_t indent) ;
FixStr Ast_const_to_json(Ast *node, size_t indent) ;
FixStr Ast_trait_to_json(Ast *node, size_t indent) ;
FixStr Ast_object_literal_to_json(Ast *node, size_t indent) ;
FixStr Ast_block_to_json(Ast *node, size_t indent) ;
FixStr Ast_if_to_json(Ast *node, size_t indent) ;
FixStr Ast_fn_def_to_json(Ast *node, size_t indent) ;
FixStr Ast_fn_call_to_json(Ast *node, size_t indent) ;
FixStr Ast_binary_op_to_json(Ast *node, size_t indent) ;
FixStr Ast_unary_op_to_json(Ast *node, size_t indent) ;
FixStr Ast_literal_to_json(Ast *node, size_t indent) ;
FixStr Ast_identifier_to_json(Ast *node, size_t indent) ;
FixStr Ast_fn_anon_to_json(Ast *node, size_t indent) ;
FixStr Ast_method_call_to_json(Ast *node, size_t indent) ;
FixStr Ast_member_access_to_json(Ast *node, size_t indent) ;
FixStr Ast_let_to_json(Ast *node, size_t indent) ;
FixStr Ast_loop_to_json(Ast *node, size_t indent) ;
FixStr Ast_for_to_json(Ast *node, size_t indent) ;
FixStr Ast_binding_to_json(Ast *node, size_t indent) ;
FixStr Ast_return_to_json(Ast *node, size_t indent) ;
FixStr Ast_match_to_json(Ast *node, size_t indent) ;
FixStr Ast_mutation_to_json(Ast *node, size_t indent) ;

#pragma endregion

#pragma region ParseContext

#define ctx_parser() (&ctx().pctx)

#define ctx_parser_append(node) \
    ctx().pctx.parser.meta.size++;\
    ctx().pctx.parser.data = node

#define pctx_trace(arg) \
    Tracer_push(&ctx_parser()->parser.traces, s(__func__), arg, s(__FILE__), __LINE__)

/// @todo rationalize this better, eg., wrt sizeof's
#define PARSING_PREALLOC (128 * 1024)

void ctx_parser_setup(ParseContext *pctx) {
    require_not_null(pctx);

    /// @todo, presumably init an arena

    Arena_MetaValue_init_new(&pctx->lexer, PARSING_PREALLOC);
    Arena_MetaValue_init_new(&pctx->parser, PARSING_PREALLOC);
}


#pragma endregion

#pragma region ParsingH
void lex(struct Lexer *lex, struct Source *src);
bool lex_is_keyword(FixStr s);

void parse(ParseContext *pctx);
// void parse_print_ast(Ast *ast);
// void parse_print_FixAst(Ast *ast, size_t depth);
// void parse_Ast_from_source(ParseContext *p, Source *src);
void parse_error(ParseContext *pctx, Token *token, FixStr message);

Token *parse_consume_type(ParseContext *p, FixStr value, TokenType type, FixStr error_message);
Ast *parse_statement(ParseContext *p);
// uint8_t parse_peek_precedence(ParseContext *p);
Ast *parse_expression(ParseContext *p, size_t precedence);
Ast *parse_expression_part(ParseContext *p);
Ast *parse_keyword_statement(ParseContext *p,bool in_expr);
Ast *parse_identifier(ParseContext *p);
Ast *parse_macro_def(ParseContext *p);
Ast *parse_loop(ParseContext *p);
Ast *parse_for(ParseContext *p);
Ast *parse_if(ParseContext *p);
Ast *parse_binding(ParseContext *p);
Ast **parse_bindings(ParseContext *p, size_t *out_num_bindings);
void     parse_post_bindings(ParseContext *p,Ast *node);
Ast *parse_block(ParseContext *p);
Ast *parse_bracketed(ParseContext *p);
// Ast *parse_dereference(ParseContext *p);
Ast *parse_unary(ParseContext *p);
Ast *parse_literal(ParseContext *p);
Ast *parse_post_call(ParseContext *p,Ast *calleeNode);
Ast *parse_object_literal(ParseContext *p);
Ast *parse_match(ParseContext *p);
Ast *parse_vec(ParseContext *p);
Ast *parse_dict(ParseContext *p);
Ast *parse_return(ParseContext *p);
Ast *parse_yield(ParseContext *p);
Ast *parse_ignore(ParseContext *p);
Ast *parse_trait(ParseContext *p);
Ast *parse_struct(ParseContext *p);
Ast *parse_use(ParseContext *p);
Ast *parse_let(ParseContext *p);
Ast *parse_const(ParseContext *p);
Ast *parse_keyword(ParseContext *p,FixStr keyword, AstType type);
Ast *parse_function(ParseContext *p,bool is_loop);
Ast *parse_post_anon(ParseContext *p);
Ast *parse_mutation(ParseContext *p);

#pragma endregion

//#endregion
/// ---------- ---------- ---------- HEADERS: INTERPRETER ---------- ---------- -- ///
//#region Headers_Interpreter

#pragma region NativesH
void native_add_prelude(FixScope scope);

// typedef Box (*FixFnNative)(FixFn *fn, FixScope parent, FixArray args);
// typedef Box (*FixFnNative0)(FixFn *fn, FixScope parent);
// typedef Box (*FixFnNative1)(FixFn *fn, FixScope parent, Box one);
// typedef Box (*FixFnNative2)(FixFn *fn, FixScope parent, Box one, Box two);
// typedef Box (*FixFnNative3)(FixFn *fn, FixScope parent, Box one, Box two, Box three);

// Native function prototypes
FixArray *CliArgs_to_FixArray(int argc, char **argv);
// double lib_rand_0to1();
double lib_rand_normal(double mean, double stddev);
Box native_log(FixFn *self, FixScope parent, FixArray args);
Box native_range(FixFn *self, FixScope parent, FixArray args);
Box native_infer(FixFn *self, FixScope parent, Box loopFn, Box strategyTag);
Box native_sample(FixFn *self, FixScope parent, Box distObject);
Box native_take(FixFn *self, FixScope parent, FixArray args);




/// @todo make these non-variadic
/// @note useful native prototypes
Box native_print(FixFn *self, FixScope parent, FixArray args);
Box native_input(FixFn *self, FixScope parent, FixArray args);
Box native_len(FixFn *self, FixScope parent, Box arg);
Box native_sum(FixFn *self, FixScope parent, FixArray args);
Box native_max(FixFn *self, FixScope parent, FixArray args);
Box native_min(FixFn *self, FixScope parent, FixArray args);
Box native_abs(FixFn *self, FixScope parent, FixArray args);
Box native_pow(FixFn *self, FixScope parent, FixArray args);
Box native_sqrt(FixFn *self, FixScope parent, FixArray args);
Box native_floor(FixFn *self, FixScope parent, FixArray args);
Box native_ceil(FixFn *self, FixScope parent, FixArray args);
Box native_round(FixFn *self, FixScope parent, FixArray args);

#pragma endregion

#pragma region InterpErrorH

#ifdef INTERP_DEBUG_VERBOSE
    #define interp_log_debug(...) log_message(LL_DEBUG, __VA_ARGS__)
#else
    #define interp_log_debug(...)
#endif

#define interp_error(...) \
    error_push((FixError){\
        .code = BXE_INTERPRETER,\
        .message = FixStr_fmt_new(__VA_ARGS__),\
        .location = s(__FILE__ ":" _STR(__LINE__))});\
    interp_graceful_exit()

#pragma endregion

#pragma region BoxedPrecache

// Precache function prototypes
// bool interp_precache_getstr(FixStr str, FixKvPair *out);
// void interp_precache_setstr(FixStr str);
// void interp_precache_test_main();


// struct BoxPreCache {
//     /// @todo since ints, floats are just copy-inside-boxes, we dont need to intern
//     // ... but doubles & strings would benefit

//     // Box small_positive_integers[256]; // -
//     // Box small_negative_integers[32]; // -
//     // Box interned_doubles[256]; /// @todo consider this
//     FixDict str_precache;
// } __glo_precache = {0};

#define interp_precache() ctx().info

#pragma endregion

#pragma region InterpreterEvaluatorH

void interp_graceful_exit(void);

// Running code
Box interp_run_from_source(ParseContext *p, FixStr source, int argc, char **argv);
Box interp_run_ast(Ast *ast);

Box interp_eval_if(Ast *node, FixScope *scope);
Box interp_eval_vec(Ast *node, FixScope *scope);
Box interp_eval_literal(Ast *node, FixScope *scope);
Box interp_eval_identifier(Ast *node, FixScope *scope);
Box interp_eval_call_fn(Ast *node, FixScope *scope);
Box interp_eval_block(Ast *node, FixScope *scope);
Box interp_eval_fn_def(Ast *node, FixScope *scope);
Box interp_eval_loop(Ast *node, FixScope *scope);

Box interp_eval_loop_with_condition(Ast *node, FixScope *scope);
Box interp_eval_loop_with_streams(Ast *node, FixScope *scope);
Box interp_eval_loop_with_infinite(Ast *node, FixScope *scope);
Box interp_eval_for(Ast *node, FixScope *scope);
Box interp_eval_expression(Ast *node, FixScope *scope);
Box interp_eval_const_def(Ast *node, FixScope *scope);
Box interp_eval_let_def(Ast *node, FixScope *scope);
Box interp_eval_binary_op(Ast *node, FixScope *scope);
Box interp_eval_unary_op(Ast *node, FixScope *scope);
Box interp_eval_match(Ast *node, FixScope *scope);
Box interp_eval_return(Ast *node, FixScope *scope);
Box interp_eval_mutation(Ast *node, FixScope *scope);

Box interp_eval_dict(Ast *node, FixScope *scope);
Box interp_eval_method_call(Ast *node, FixScope *scope);
Box interp_eval_member_access(Ast *node, FixScope *scope);
Box interp_eval_module(Ast *node, FixScope *scope);
Box interp_eval_fn_anon(Ast *node, FixScope *scope);
Box interp_eval_use(Ast *node, FixScope *scope);
Box interp_eval_struct_def(Ast *node, FixScope *scope);
Box interp_eval_object_literal(Ast *node, FixScope *scope);

Box interp_eval_load_module(Ast *node);

#pragma endregion

#pragma region InterpreterMinimalRuntimeH
Box interp_eval_fn(Box fn_obj, int num_args, Box *args);
Box interp_eval_uop(FixStr op, Box operand);
Box interp_eval_bop(FixStr op, Box left, Box right);

// void interp_eval_def(FixStr name, Box value, FixScope *scope);
// bool interp_eval_lookup(FixStr name, FixScope *scope, Box *out_value);
Box interp_eval_ast(Ast *node, FixScope *scope);
Box FixFn_Ast_call(FixFn *fn, FixArray args, FixScope parent);

#pragma endregion

//#endregion
///---------- ---------- ----------  HEADERS: /END    ---------- ---------- ------ ///

///---------------------------------- IMPLEMENTATION ------------------------------///
///---------------------------------- IMPLEMENTATION ------------------------------///

///---------- ---------- ---------- IMPL: INTERNALS ---------- ---------- -------- ///
//#region Implementation_InternalUtil

#pragma region GlobalContextImpl


void ctx_setup(struct GlobalContext *ctx, Heap *heap, Arena *arena) {
    require_not_null(ctx);

    heap = heap ? heap : &ctx->heaps.interpreter;
    arena = arena ? arena : &ctx->arenas.compiler;

    ctx->arenas.current = arena;
    ctx->heaps.current = heap;
    ctx->allocators.aro_new = Arena_new;
    ctx->allocators.aro_free = Arena_aro_free;
    ctx->allocators.gco_new = Heap_cnew;

    if(arena == nullptr) {
        log_message(LL_INFO, sMSG("Setting up default arenas."));
        Arena_init(&ctx->arenas.compiler, 8 * ARENA_1MB);
        Arena_init(&ctx->arenas.interpreter_global, 8 * ARENA_1MB);
        Arena_init(&ctx->arenas.interpreter_local, 2 * ARENA_1MB);
        /// @todo -- may not be the right approach
        // Arena_init(&ctx->arenas.gc_backing, 8 * ARENA_1MB);
    }

    if(heap == nullptr) {
        log_message(LL_INFO, sMSG("Setting up default heaps."));
        Heap_data_cnew(
            HEAP_BASE_CAPACITY,
            HEAP_BASE_CAPACITY / HEAP_GC_PREALLOC_RATIO
        );
    }

    ctx->debug.callstack = nullptr; /// @todo
    ctx->debug.perf_metrics = nullptr; /// @todo

    require_not_null(ctx->arenas.current);
    require_not_null(ctx->heaps.current);
}

#pragma endregion

#pragma region LoggingImpl

void logging_test_main(void) {
    printf("Running Logging Tests...\n");
    log_message(LL_INFO, s("This is an info message."));
    log_message(LL_WARNING, s("This is a warning message."));
    log_message(LL_DEBUG, s("This is a debug message."));
    log_message(LL_RECOVERABLE, s("This is a recoverable message."));
    log_message(LL_TEST, s("This is a test message."));

    /// @todo this doesn't work atm, fix it

    // #ifdef DEBUG_ABORT_ON_ERROR
    //     #undef DEBUG_ABORT_ON_ERROR
    //     log_message(LL_ERROR, s("This is an error message."));
    //     #define DEBUG_ABORT_ON_ERROR
    // #else
    //     log_message(LL_ERROR, s("This is an error message."));
    // #endif

    // #ifdef DEBUG_ABORT_ON_ASSERT
    //     #undef DEBUG_ABORT_ON_ASSERT
    //     log_message(LL_ASSERT, s("This is an assert message."));
    //     #define DEBUG_ABORT_ON_ASSERT
    // #else
    //     log_message(LL_ASSERT, s("This is an assert message."));
    // #endif

    printf("Logging Tests Completed.\n\n");
}


#pragma endregion

#pragma region FixStringImpl

/// @todo rename to indicate allocators vs. views vs. borrows/etc.
/// eg. FixStr_part_view(), FixStr_lines_new()

FixStr FixStr_new(const char *cstr, size_t len) {
    if (!cstr) return FixStr_empty();

    char *buffer =  (char *) Arena_alloc(len);

    if (!buffer) { error_oom(); return FixStr_empty(); }

    require_safe(clib_memcpy_safe(buffer, len, cstr, len));
    return (FixStr) {.cstr = buffer, .size = len};
}

FixStr FixStr_readlines_new(FixStr filename, size_t start_line, size_t endline) {
    FILE *file = fopen(filename.cstr, "r");
    if (!file) {
        log_message(LL_ERROR, s("Failed to open file: "), filename);
        return FixStr_empty();
    }

    size_t line_num = 0;
    size_t max_line = endline - start_line;
    unsigned long max_size = 1024;
    size_t buffer_size = max_line * max_size;
    char *buffer = (char *) Arena_alloc(buffer_size);
    if (!buffer) { fclose(file); error_oom(); return FixStr_empty(); }

    while (line_num < endline) {
        if (line_num < start_line) {
            if (!fgets(buffer, max_size, file)) { // NOLINT(*-narrowing-conversions)
                fclose(file);
                return FixStr_empty();
            }
        } else {
            if (!fgets(buffer + (line_num - start_line) * max_size, max_size, file)) {
                fclose(file);
                return FixStr_new(buffer, (line_num - start_line) * max_size);
            }
        }
        line_num++;
    }

    fclose(file);
    return FixStr_new(buffer, buffer_size);
}
// FixStr FixStr_new(const char *cstr) {
//     if (!cstr) return FixStr_empty();

//     size_t len = strlen(cstr);
//     char *buffer = (char *) gco_new(len);

//     if (!buffer) { error_oom(); return FixStr_empty(); }

//     memcpy(buffer, cstr, len);
//     return (FixStr) {.cstr = buffer, .size = len};
// }

FixStr FixStr_unowned_cnew(FixStr str) {
    if (FixStr_is_empty(str)) return FixStr_empty();

    char *buffer = (char *) cnew(str.size);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    ///clib_memcpy_safe(void *dest, size_t dest_size, const void *src, size_t count)
    require_safe(clib_memcpy_safe(buffer, str.size, str.cstr, str.size));

    return (FixStr) {.cstr = buffer, .size = str.size};
}

FixStr FixStr_col_new(const char *color_code, FixStr str) {
    size_t color_len = strlen(color_code);
    size_t reset_len = strlen(ANSI_COL_RESET);
    size_t new_size = color_len + str.size + reset_len;

    // Allocate memory for the new colored string
    char *buffer = Arena_alloc(new_size + 1);
    if (!buffer) {
        error_oom();
        return FixStr_empty();
    }

    require_safe(clib_memcpy_safe(buffer, color_len, color_code, color_len));
    require_safe(clib_memcpy_safe(buffer + color_len, str.size, str.cstr, str.size));
    require_safe(clib_memcpy_safe(buffer + color_len + str.size, reset_len, ANSI_COL_RESET, reset_len));

    buffer[new_size] = '\0';

    FixStr colored_str = { .cstr = buffer, .size = new_size };
    return colored_str;
}


/// @brief Trims leading and trailing repeat delimiters from a string
/// ....  eg., 'quote' -> quote
/// ....  eg., ''quote'' -> quote
/// ....  eg., """quote""" -> quote
/// @param str
/// @return
FixStr FixStr_trim_delim(FixStr str) {
    if (str.size < 2) return str;

    while (str.cstr[0] == str.cstr[str.size - 1]) {
        str = (FixStr) {.cstr = str.cstr + 1, .size = str.size - 2};
    }
    return str;
}

FixStr FixStr_firstN(FixStr str, size_t n) {
    if (n > str.size) {
        n = str.size;
    }
    return (FixStr) {.cstr = str.cstr, .size = n};
}

FixStr FixStr_read_file_new(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_message(LL_ERROR, s("Failed to open file: "), s(filename));
        return FixStr_empty();
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *) Arena_alloc(size + 1);
    if (!buffer) { fclose(file); error_oom(); return FixStr_empty(); }

    size_t read = fread(buffer, 1, size, file);
    buffer[read] = '\0';

    fclose(file);
    return (FixStr) {.cstr = buffer, .size = read};
}



bool FixStr_starts_with(FixStr str, FixStr prefix) {
    if (str.size < prefix.size) return false;
    return memcmp(str.cstr, prefix.cstr, prefix.size) == 0;
}

size_t FixStr_hash(FixStr str) {
    const uint64_t FNV_offset_basis = 14695981039346656037UL;
    const uint64_t FNV_prime = 1099511628211UL;
    size_t hash = FNV_offset_basis;

    for (size_t i = 0; i < str.size; i++) {
        hash ^= (unsigned char)(str.cstr[i]);
        hash *= FNV_prime;
    }

    return hash;
}

// bool FixStr_is_empty(FixStr str) {
//     return (str.size == 0) || (str.cstr == nullptr);
// }

/// @brief
/// @param str
/// @return
/// @todo investigate this impl
double FixStr_to_double(FixStr str) {
    return atof(str.cstr);
}

int FixStr_to_int(FixStr str) {
    return atoi(str.cstr);
}

FixStr FixStr_part(FixStr line, size_t start, size_t end) {
    return (FixStr) {.cstr = line.cstr + start, .size = end - start};
}

FixStr FixStr_ncopy(FixStr str, size_t start, size_t end) {
    return FixStr_new(str.cstr + start, end - start);
}

FixStr FixStr_copy(FixStr old) {
    return FixStr_new(old.cstr, old.size);
}

FixStr FixStr_readline_new(size_t max) {
    char *buffer = (char *) Arena_alloc(max + 1);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    if (!fgets(buffer, max, stdin)) {
        return FixStr_empty();
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }

    return (FixStr) {.cstr = buffer, .size = len};
}
// Print a FixStr to stdout
void FixStr_puts(FixStr str) {
    if (str.cstr && str.size > 0) {
        printf("%.*s", (int)str.size, str.cstr);
    } else {
        puts("(nullptr)");
    }
}

#define FixStr_print(...) FixStr_array_print(\
    ((FixStr[]) { __VA_ARGS__ }), sizeof(((FixStr[]) { __VA_ARGS__ }))/sizeof(FixStr)\
)

void FixStr_array_print(FixStr *array, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (FixStr_is_empty(array[i])) {
            puts("(nullptr)");
        } else {
            printf("%.*s ", fmt(array[i]));
        }
    }
    printf("\n");
}



void FixStr_println(FixStr str) {
    if (str.cstr && str.size > 0) {
        printf("%.*s\n", (int)str.size, str.cstr);
    } else {
        puts("(nullptr)\n");
    }
}
// Print formatted string to stdout
void FixStr_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Create a FixStr from a single character
/// @todo is static alloc of buffers etc. here right?
FixStr FixStr_chr(char chr) {
    // Using a static buffer to hold the character
    static char buffer[1];
    buffer[0] = chr;
    return (FixStr){ .cstr = buffer, .size = 1 };
}


// Concatenate two FixStr instances
FixStr FixStr_glue_new(FixStr left, FixStr right) {
    if(left.size == 0 && right.size == 0) return FixStr_empty();
    if(right.size == 0) return left;
    if(left.size == 0) return right;

    size_t new_len = left.size + right.size;
    char *buffer = (char *) Arena_alloc(new_len);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    require_safe(clib_memcpy_safe(buffer, left.size, left.cstr, left.size));
    require_safe(clib_memcpy_safe(buffer + left.size, right.size, right.cstr, right.size));

    buffer[new_len] = '\0';

    return (FixStr){ .cstr = buffer, .size = new_len };
}

FixStr FixStr_glue_sep_new(FixStr left, FixStr sep, FixStr right) {
    size_t new_len = left.size + sep.size + right.size;
    char *buffer = (char *) Arena_alloc(new_len);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    /// old: memcpy(buffer, left.cstr, left.size);
    /// old: memcpy(buffer + left.size, sep.cstr, sep.size);
    /// old: memcpy(buffer + left.size + sep.size, right.cstr, right.size);
    require_safe(clib_memcpy_safe(buffer, left.size, left.cstr, left.size));
    require_safe(clib_memcpy_safe(buffer + left.size, sep.size, sep.cstr, sep.size));
    require_safe(clib_memcpy_safe(buffer + left.size + sep.size, right.size, right.cstr, right.size));

    buffer[new_len] = '\0';

    return (FixStr){ .cstr = buffer, .size = new_len };
}


// Check if two FixStr instances are equal
bool FixStr_eq(FixStr left, FixStr right) {
    if (left.size != right.size) return false;
    return memcmp(left.cstr, right.cstr, left.size) == 0;
}


inline bool FixStr_eq_cstr(FixStr left, const char *right) {
    return FixStr_eq(left, FixStr_from_cstr(right));
}

FixStr FixStr_join_new(FixStr one, ...) {
    va_list args;
    va_start(args, one);

    size_t total_len = one.size;
    FixStr current = one;
    while (current.size > 0) {
        total_len += current.size;
        current = va_arg(args, FixStr);
    }

    char *buffer = (char *) Arena_alloc(total_len + 1);
    if (!buffer) { va_end(args); error_oom(); return FixStr_empty(); }

    //old: memcpy(buffer, one.cstr, one.size);
    require_safe(clib_memcpy_safe(buffer, one.size, one.cstr, one.size));
    size_t offset = one.size;

    current = one;
    while (current.size > 0) {
        // old: memcpy(buffer + offset, current.cstr, current.size);
        require_safe(clib_memcpy_safe(buffer + offset, current.size, current.cstr, current.size));
        offset += current.size;
        current = va_arg(args, FixStr);
    }

    buffer[total_len] = '\0';
    va_end(args);

    return (FixStr){ .cstr = buffer, .size = total_len };
}

char *cstr_repeat_new(const char *cstr, size_t count) {
    size_t len = strlen(cstr);
    size_t new_len = len * count;
    char *buffer = (char *) Arena_alloc(new_len + 1);
    if (!buffer) { error_oom(); return nullptr; }

    for (size_t i = 0; i < count; i++) {
        ///old: memcpy(buffer + i * len, cstr, len);
        require_safe(clib_memcpy_safe(buffer + i * len, len, cstr, len));
    }
    buffer[new_len] = '\0';

    return buffer;
}

bool write_bytes_to_file(const char *filename, const char *bytes, size_t len) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        log_message(LL_ERROR, s("Failed to open file: "), s(filename));
        return false;
    }

    size_t written = fwrite(bytes, 1, len, file);
    fclose(file);

    return written == len;
}

#define MAX_FILE_SIZE (1 << 30) // 1 GB

unsigned char *read_bytes_from_file_cnew(const char *filename, size_t *out_len) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        log_message(LL_ERROR, s("Failed to open file: "), s(filename));
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(size < 0 || (size_t) size > MAX_FILE_SIZE) {
        fclose(file);
        log_message(LL_ERROR, s("File too large or invalid size: %ld"), size);
        return nullptr;
    }

    unsigned char *buffer = (unsigned char *) cnew(size);
    if (!buffer) { fclose(file); error_oom(); return nullptr; }

    size_t read = fread(buffer, 1, size, file);
    fclose(file);

    if (out_len) *out_len = read;
    return buffer;
}

int char_to_base37(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A'; // 'A' -> 0, 'B' -> 1, ..., 'Z' -> 25
    } else if (c >= '0' && c <= '9') {
        return 26 + (c - '0'); // '0' -> 26, ..., '9' -> 35
    } else {
        return 36; // map invalids to _
    }
}

char base37_to_char(uint8_t value) {
    if (value <= 25) {
        return 'A' + value; // 0 -> 'A', ..., 25 -> 'Z'
    } else if (value <= 35) {
        return '0' + (value - 26); // 26 -> '0', ..., 35 -> '9'
    } else if (value == 36) {
        return '_'; // 36 -> '_'
    } else {
        return '?'; // Placeholder for invalid values
    }
}


// Helper function to create indentation string
FixStr FixStr_repeat_new(FixStr str, size_t count) {
    if(count == 0) return FixStr_empty();
    size_t new_len = str.size * count;
    char *buffer = (char *) Arena_alloc(new_len);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    for (size_t i = 0; i < count; i++) {
        ///old: memcpy(buffer + i * str.size, str.cstr, str.size);
        require_safe(clib_memcpy_safe(buffer + i * str.size, str.size, str.cstr, str.size));
    }
    buffer[new_len] = '\0';

    return (FixStr){ .cstr = buffer, .size = new_len };
}

FixStr FixStr_fmt_new(FixStr format, ...) {
    va_list args;
    va_start(args, format);
    FixStr result = FixStr_fmt_va_new(format, args);
    va_end(args);
    return result;
}


// Assuming FixStr and FixStr_empty() are defined appropriately elsewhere

FixStr FixStr_fmt_va_new(FixStr format, va_list args) {
    FixStr result = FixStr_empty();
    char *buffer = nullptr;
    int len = 0;
    va_list args_copy;

    // Initialize args_copy
    va_copy(args_copy, args);

    // Determine the required length
    len = clib_vsnprintf_safe(nullptr, 0, format.cstr, args);
    if (len < 0) {
        goto cleanup;
    }

    // Allocate buffer
    buffer = (char *) Arena_alloc(len + 1);
    if (!buffer) {
        error_oom();
        goto cleanup;
    }

    // Write the formatted string to the buffer
    if (clib_vsnprintf_safe(buffer, len + 1, format.cstr, args_copy) < 0) {
        // If writing fails, free the buffer and set it to nullptr
        buffer = nullptr;
        goto cleanup;
    }

    // Populate the result FixStr
    result.cstr = buffer;
    result.size = len;

cleanup:
    // Perform cleanup actions
    va_end(args_copy);

    // If buffer allocation or writing failed, ensure buffer is nullptr
    if (buffer == nullptr) {
        result = FixStr_empty();
    }

    return result;
}


// Check if FixStr equals a single character
bool FixStr_eq_chr(FixStr left, char right) {
    return (left.size == 1) && (left.cstr[0] == right);
}

// Get a substring from a FixStr
FixStr FixStr_sub(FixStr str, size_t start, size_t size) {
    if (start >= str.size) return FixStr_empty();
    if (start + size > str.size) {
        size = str.size - start;
    }
    return (FixStr){ .cstr = str.cstr + start, .size = size };
}


bool FixStr_contains(FixStr str, FixStr part) {
    return FixStr_index_of(str, part, nullptr);
}

bool FixStr_index_of(FixStr str, FixStr part, size_t *out_index) {
    if (part.size == 0) {
        return false;
    }
    if (part.size > str.size) {
        return false;
    }

    for (size_t i = 0; i <= str.size - part.size; ++i) {
        if (memcmp(str.cstr + i, part.cstr, part.size) == 0) {
            if(out_index) *out_index = i;
            return true;
        }
    }
    return false;
}

bool FixStr_last_index(FixStr str, FixStr part, size_t *out_index) {
    if (part.size == 0) {
        return false;
    }

    if (part.size > str.size) {
        return false;
    }

    for (size_t i = str.size - part.size + 1; i-- > 0;) {
        if (memcmp(str.cstr + i, part.cstr, part.size) == 0) {
            if(out_index) *out_index = i;
            return true;
        }
    }

    return false;
}


// Find the index of a character in FixStr
int FixStr_index_of_chr(FixStr str, char chr) {
    for (size_t i = 0; i < str.size; i++) {
        if (str.cstr[i] == chr) return (int)i;
    }
    return -1;
}

// Find the last index of a character in FixStr
int FixStr_last_index_of_chr(FixStr str, char chr) {
    for (size_t i = str.size; i > 0; i--) {
        if (str.cstr[i-1] == chr) return (int)(i-1);
    }
    return -1;
}

// Replace all occurrences of a character in FixStr
FixStr FixStr_replace(FixStr str, char target, char replacement) {
    char *buffer = (char *) Arena_alloc(str.size + 1);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    size_t new_len = 0;
    for (size_t i = 0; i < str.size; i++) {
        buffer[new_len++] = (str.cstr[i] == target) ? replacement : str.cstr[i];
    }
    buffer[new_len] = '\0';

    return (FixStr){ .cstr = buffer, .size = new_len };
}

// Trim whitespace from both ends of FixStr
FixStr FixStr_trim(FixStr str) {
    size_t start = 0;
    while (start < str.size && isspace((unsigned char)str.cstr[start])) start++;

    size_t end = str.size;
    while (end > start && isspace((unsigned char)str.cstr[end - 1])) end--;

    return FixStr_sub(str, start, end - start);
}

// Convert FixStr to uppercase
FixStr FixStr_upper(FixStr str) {
    char *buffer = (char *) Arena_alloc(str.size + 1);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    for (size_t i = 0; i < str.size; i++) {
        buffer[i] = toupper((unsigned char)str.cstr[i]);
    }
    buffer[str.size] = '\0';

    return (FixStr){ .cstr = buffer, .size = str.size };
}

// Convert FixStr to lowercase
FixStr FixStr_lower(FixStr str) {
    char *buffer = (char *) Arena_alloc(str.size + 1);
    if (!buffer) { error_oom(); return FixStr_empty(); }

    for (size_t i = 0; i < str.size; i++) {
        buffer[i] = tolower((unsigned char)str.cstr[i]);
    }
    buffer[str.size] = '\0';

    return (FixStr){ .cstr = buffer, .size = str.size };
}

// Compare two FixStr instances lexicographically
int FixStr_cmp(FixStr left, FixStr right) {
    size_t min_len = left.size < right.size ? left.size : right.size;
    int cmp = memcmp(left.cstr, right.cstr, min_len);
    if (cmp != 0) return cmp;
    if (left.size < right.size) return -1;
    if (left.size > right.size) return 1;
    return 0;
}

// Compare two FixStr instances using natural ordering
int FixStr_cmp_natural(FixStr left, FixStr right) {
    size_t i = 0, j = 0;
    while (i < left.size && j < right.size) {
        if (isdigit(left.cstr[i]) && isdigit(right.cstr[j])) {
            // Extract numbers
            long num1 = strtol(left.cstr + i, nullptr, 10);
            long num2 = strtol(right.cstr + j, nullptr, 10);
            if (num1 < num2) return -1;
            if (num1 > num2) return 1;
            // Advance i and j past the number
            while (i < left.size && isdigit((unsigned char)left.cstr[i])) i++;
            while (j < right.size && isdigit((unsigned char)right.cstr[j])) j++;
        } else {
            if (left.cstr[i] < right.cstr[j]) return -1;
            if (left.cstr[i] > right.cstr[j]) return 1;
            i++;
            j++;
        }
    }
    if (i < left.size) return 1;
    if (j < right.size) return -1;
    return 0;
}

// Append a character to FixStr
FixStr FixStr_join_chr_new(FixStr str, char chr) {
    char *buffer = (char *) Arena_alloc(str.size + 2);
    if (!buffer) { error_oom(); return FixStr_empty(); }
    ///old: memcpy(buffer, str.cstr, str.size);
    require_safe(clib_memcpy_safe(buffer, str.size, str.cstr, str.size));
    buffer[str.size] = chr;
    buffer[str.size + 1] = '\0';
    return (FixStr){ .cstr = buffer, .size = str.size + 1 };
}

// Append a C-string to FixStr
FixStr FixStr_join_cstr_new(FixStr str, const char *cstr) {
    size_t cstr_len = strlen(cstr);
    size_t new_len = str.size + cstr_len;
    char *buffer = (char *) Arena_alloc(new_len + 1);
    if (!buffer) { error_oom(); return FixStr_empty(); }
    /// old: memcpy(buffer, str.cstr, str.size);
    /// old: memcpy(buffer + str.size, cstr, cstr_len);
    require_safe(clib_memcpy_safe(buffer, str.size, str.cstr, str.size));
    require_safe(clib_memcpy_safe(buffer + str.size, cstr_len, cstr, cstr_len));
    buffer[new_len] = '\0';
    return (FixStr){ .cstr = buffer, .size = new_len };
}

// Check if FixStr contains a character
bool FixStr_contains_chr(FixStr str, char chr) {
    for (size_t i = 0; i < str.size; i++) {
        if (str.cstr[i] == chr) return true;
    }
    return false;
}


// Split FixStr by a delimiter, returns an array of FixStr and sets out_count
FixStr *FixStr_split_chr(FixStr str, char delimiter, size_t *out_count) {
    size_t count = 1;
    for (size_t i = 0; i < str.size; i++) {
        if (str.cstr[i] == delimiter) count++;
    }

    FixStr *result = (FixStr *) Arena_alloc(sizeof(FixStr) * count);
    if (!result) { error_oom(); *out_count = 0; return nullptr; }

    size_t start = 0;
    size_t idx = 0;
    for (size_t i = 0; i < str.size; i++) {
        if (str.cstr[i] == delimiter) {
            size_t len = i - start;
            result[idx++] = (FixStr){ .cstr = str.cstr + start, .size = len };
            start = i + 1;
        }
    }
    // Last segment
    result[idx++] = (FixStr){ .cstr = str.cstr + start, .size = str.size - start };
    *out_count = count;
    return result;
}

// Split FixStr into lines by a delimiter (typically '\n')
FixStr *FixStr_lines(FixStr str, size_t *out_count) {
    return FixStr_split_chr(str, '\n', out_count);
}

// Free memory allocated for FixStr
void FixStr_aro_free(FixStr str) {
    if (str.cstr) {
        aro_free((void*)str.cstr);
    }
}


// Simple test suite for the FixStr library
int string_test_main(void) {
    // Test FixStr_from_cstr
    FixStr hello = FixStr_from_cstr("Hello");
    assert(hello.size == 5);
    assert(FixStr_eq(hello, s("Hello")));

    // Test FixStr_glue_new
    FixStr world = s(" World");
    FixStr hello_world = FixStr_glue_new(hello, world);
    assert(hello_world.size == 11);
    assert(FixStr_eq(hello_world, s("Hello World")));

    // Test FixStr_print
    printf("FixStr_print: ");
    FixStr_println(hello_world); // Should print "Hello World"
    printf("\n");

    // Test FixStr_eq
    assert(FixStr_eq(hello, s("Hello")));
    assert(!FixStr_eq(hello, s("hello")));

    // Test FixStr_eq_chr
    FixStr nl = s("\n");
    assert(FixStr_eq_chr(nl, '\n'));
    assert(!FixStr_eq_chr(nl, 'h'));

    // Test FixStr_trim
    FixStr padded = s("  Hello  ");
    FixStr trimmed = FixStr_trim(padded);
    assert(FixStr_eq(trimmed, hello));

    // Test FixStr_upper and FixStr_lower
    FixStr upper = FixStr_upper(hello);
    assert(FixStr_eq(upper, s("HELLO")));

    FixStr lower = FixStr_lower(upper);
    assert(FixStr_eq(lower, s("hello")));

    // Test FixStr_replace
    FixStr replaced = FixStr_replace(hello_world, ' ', '_');
    assert(FixStr_eq(replaced, s("Hello_World")));

    // Test FixStr_contains
    assert(FixStr_contains_chr(hello_world, 'W'));
    assert(!FixStr_contains_chr(hello_world, 'w'));

    assert(FixStr_contains(hello_world, s("World")));
    assert(!FixStr_contains(hello_world, s("world")));

    // Test FixStr_split_chr
    size_t count;
    FixStr *parts = FixStr_split_chr(s("Hello World"), ' ', &count);
    assert(count == 2);
    assert(FixStr_eq(parts[0], s("Hello")));
    assert(FixStr_eq(parts[1], s("World")));
    aro_free(parts);

    // Test FixStr_cmp
    assert(FixStr_cmp(s("A"), s("A")) == 0);
    assert(FixStr_cmp(s("B"), s("A")) > 0);
    assert(FixStr_cmp(s("A"), s("B")) < 0);

    // Test FixStr_cmp_natural
    FixStr file2 = s("file2");
    FixStr file10 = s("file10");
    assert(FixStr_cmp_natural(file2, file10) < 0);
    assert(FixStr_cmp_natural(file10, file2) > 0);
    assert(FixStr_cmp_natural(file2, FixStr_from_cstr("file2")) == 0);

    // Test FixStr_join_chr_new
    FixStr appended_char = FixStr_join_chr_new(hello, '!');
    assert(FixStr_eq(appended_char, FixStr_from_cstr("Hello!")));
    FixStr_aro_free(appended_char);

    // Test FixStr_join_cstr_new
    FixStr appended_cstr = FixStr_join_cstr_new(hello, ", everyone");
    assert(FixStr_eq(appended_cstr, FixStr_from_cstr("Hello, everyone")));
    FixStr_aro_free(appended_cstr);

    // All tests passed
    printf("All string tests passed.\n");
    return 0;
}

#pragma endregion

#pragma region ArenaImpl

#ifdef DEBUG_MEMORY
    #define mem_debug_log(...)\
        log_message(LL_DEBUG, __VA_ARGS__);
#else
    #define mem_debug_log(msg, ...) NOOP
#endif


static bool Arena_block_cnew(Arena *a, size_t size) {
    require_not_null(a);
    require_positive(size);

    mem_debug_log(sMSG("Block usage: %zu/%zu"), a->used, a->block_size);

    size_t alloc_size = (size > a->block_size) ? size : a->block_size;

    void *block = cnew(alloc_size);
    if (!block) {
        error_oom();
        return false;
    }

    if (a->num_blocks % 32 == 0) {
        size_t new_capacity = a->num_blocks + 32;

        void **new_blocks = cextend(a->blocks, new_capacity * sizeof(void *));
        if (!new_blocks) {
            error_oom();
            free(block);
            return false;
        }

        size_t *new_block_sizes = cextend(a->block_sizes, new_capacity * sizeof(size_t));
        if (!new_block_sizes) {
            error_oom();
            free(block);
            return false;
        }

        a->blocks = new_blocks;
        a->block_sizes = new_block_sizes;
    }

    a->used = 0;
    a->blocks[a->num_blocks] = block;
    a->block_sizes[a->num_blocks] = alloc_size;
    a->num_blocks++;

    mem_debug_log(sMSG("Block allocation complete. Number of blocks: %zu"), a->num_blocks);

    return true;
}

bool Arena_init(Arena *a, size_t block_size) {
    require_null(a->blocks);    // ensure it hasn't been init'd already
    require_not_null(a);
    require_positive(block_size);

    a->block_size = block_size;
    a->used = 0;
    a->num_blocks = 0;
    a->blocks = nullptr;

    return Arena_block_cnew(a, block_size);
}


void Arena_MetaValue_new_data(MetaValue *array, size_t datum_size, size_t size) {
    require_not_null(array);

    if(size == 0) {
        array->data = nullptr;
        return;
    }

    array->meta.alloc_size = size * datum_size;
    array->meta.capacity = size;
    array->meta.size = 0;

    array->data = Arena_alloc(array->meta.alloc_size);
    if(!array->data) { require_resources_cleanup(array->data); error_oom(); }

    require_safe(
        clib_memset_zero_safe(array->data, array->meta.alloc_size, array->meta.alloc_size)
    );
}

unsigned char *Arena_save_cnew(Arena *a, size_t *out_size) {
    require_not_null(a);
    require_not_null(out_size);

    size_t total_size = 0;
    for (size_t i = 0; i < a->num_blocks; i++) {
        total_size += a->block_sizes[i];
    }

    if(total_size == 0) {
        *out_size = 0;
        return nullptr;
    }

    unsigned char *data = cnew(total_size);

    if (!data) {
        require_resources_cleanup(data);
        error_oom();
        return nullptr;
    }

    size_t offset = 0;
    for (size_t i = 0; i < a->num_blocks; i++) {
        ///old: memcpy(data + offset, a->blocks[i], a->block_sizes[i]);
        require_safe(clib_memcpy_safe(data + offset, a->block_sizes[i], a->blocks[i], a->block_sizes[i]));
        offset += a->block_sizes[i];
    }

    *out_size = total_size;
    return data;
}

bool Arena_load_cnew(Arena *a, unsigned char *data, size_t size) {
    require_not_null(a);
    require_not_null(data);
    require_positive(size);

    if (a->blocks) {
        log_message(LL_WARNING, s("Arena already contains data. Deallocating existing data."));
        Arena_aro_free_underlying(a);
    }

    a->block_size = size;
    a->used = size;
    a->num_blocks = 1;
    a->blocks = (void **) cnew(sizeof(void *));
    a->block_sizes = (size_t *) cnew(sizeof(size_t));

    if (!a->blocks || !a->block_sizes) {
        error_oom();
        return false;
    }

    a->blocks[0] = data;
    a->block_sizes[0] = size;

    return true;
}


void Arena_aro_free(void *p) {
    /// @note compiler hint to ignore unused parameter
    (void)p;
    //return; //no-op
}

void Arena_aro_free_underlying(Arena *a) {
    require_not_null(a);

    for(size_t i = 0; i < a->num_blocks; i++) {
        free(a->blocks[i]);
    }
    free(a->blocks);

    a->blocks = nullptr;
    a->num_blocks = 0;
}

void *Arena_new(size_t alloc_size, MetaType type) {
    Arena *a = ctx_current_arena();

    require_not_null(a);
    require_positive(alloc_size);


    // Align to 8 bytes
    alloc_size = (alloc_size + 7) & ~((size_t)7);
    mem_debug_log(sMSG("Allocating size: %zu (aligned size: %zu)"), alloc_size, alloc_size);

    if ((a->used + alloc_size) > a->block_size) {
        mem_debug_log(sMSG("Growing blocks"));
        if (!Arena_block_cnew(a, alloc_size)) {
            error_oom();
            return nullptr;
        }
    }

    void *ptr = (char *)a->blocks[a->num_blocks - 1] + a->used;

    if(type) {
        Boxed *box = (Boxed *)ptr;
        box->meta.type = type;
        box->meta.alloc_size = alloc_size;
        box->meta.capacity = 0;
        box->meta.size = 0;
        box->meta.lifetime = a->lifetime;
    }
    a->used += alloc_size;
    return ptr;
}



/// @brief Not needed since Arena_new() already reallocates if needed
/// @param a
/// @param ptr
/// @param old_size
/// @param new_size
/// @return
void *Arena_cextend(void *ptr, size_t old_size, size_t new_size) {
    require_positive(new_size);

    // Align sizes to 8 bytes
    old_size = (old_size + 7) & ~((size_t)7);
    new_size = (new_size + 7) & ~((size_t)7);

    mem_debug_log(sMSG("Reallocating ptr: %p, old size: %zu, new size: %zu"), ptr, old_size, new_size);

    // If the new size is less than or equal to the old size, no reallocation is needed
    if (new_size <= old_size) {
        return ptr;
    } else {
        mem_debug_log(sMSG("Reallocation performing clib_memcpy_safe."));
    }

    // Allocate new memory for the larger size
    void *new_ptr = Arena_alloc(new_size);
    if (!new_ptr) {
        error_oom();
        return nullptr;
    }

    // Copy the old data to the new location
    if (ptr) {
        // old: memcpy(new_ptr, ptr, old_size);
        require_safe(clib_memcpy_safe(new_ptr, old_size, ptr, old_size));
    }

    mem_debug_log(sMSG("Reallocation complete. New ptr: %p"), new_ptr);
    return new_ptr;
}


void Arena_reset(Arena *a) {
    require_not_null(a);
    a->used = 0;
}

/// @todo: maybe Arena_reset_withzero()

void Arena_test_main(void) {
    // Arena a = {0};
    FixStr_println(s("All tests passed successfully.\n"));
}


#pragma endregion

#pragma region HeapImpl



static const size_t PRIMES[] = {
    13, 53, 97, 193, 389, 769,
    1543, 3079, 6151, 12289, 24593,
    49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457,
    1610612741
};
static const size_t NUM_PRIMES = sizeof(PRIMES) / sizeof(PRIMES[0]);

static inline size_t HeapArray_next_capacity(size_t initial_capacity) {
    for (size_t i = 0; i < NUM_PRIMES; i++) {
        if (PRIMES[i] >= initial_capacity) {
            return PRIMES[i];
        }
    }

    error_oom();
    log_assert(false, sMSG("Failed to find next prime capacity."));
    return (size_t) (initial_capacity * 2);
}


void Heap_data_cnew(size_t initial_capacity, size_t gc_threshold) {
    Heap *h = ctx_current_heap();
    h->free_list = nullptr;
    h->gc_threshold = gc_threshold;
    h->generation_count = 0;
    cnew_carray(h, initial_capacity);
}

void *Heap_cnew(size_t alloc_size, MetaType type) {
    Heap *heap = ctx_current_heap();
    MetaValue *obj = nullptr;

    // Reuse from free list if available
    if (heap->free_list) {
        obj = heap->free_list;
        heap->free_list = heap->free_list->meta.next;
    } else {
        // Expand the data array if necessary
        if (is_empty_ref(heap)) {
            size_t new_capacity = HeapArray_next_capacity(capacity_ref(heap));
            MetaValue **new_data = cextend(heap->data, sizeof(MetaValue *) * new_capacity);
            if (!new_data) {
                log_message(LL_ERROR, sMSG("Heap allocation failed: no space to expand data array."));
                return nullptr;
            }
            //old: (new_data + capacity_ref(heap), 0, sizeof(MetaValue *) * (new_capacity - capacity_ref(heap)));
            require_safe(clib_memset_zero_safe(new_data + capacity_ref(heap),
                sizeof(MetaValue *) * (new_capacity - capacity_ref(heap)),
                sizeof(MetaValue *) * (new_capacity - capacity_ref(heap))));
            heap->data = new_data;
            heap->meta.capacity = new_capacity;
        }

        // Allocate new MetaValue
        obj = cnew(sizeof(MetaValue));
        if (!obj) {
            log_message(LL_ERROR, sMSG("Heap allocation failed: unable to allocate MetaValue."));
            return nullptr;
        }
        obj->meta.marked = false;
        obj->meta.next = nullptr;
        push_ref(heap, obj);
    }

    obj->meta.type = type;
    obj->meta.alloc_size = alloc_size;
    // obj->meta.capacity = size;
    obj->data = cnew(alloc_size);
    if (!obj->data) {
        log_message(LL_ERROR, sMSG("Heap allocation failed: unable to allocate object data."));
        // Return obj to free list
        obj->meta.state = LIVING_DEAD;
        return nullptr;
    }

    return obj->data;
}

void Heap_test_main(void) {
    log_assert(false, sMSG("Heap tests not implemented"));
}

#pragma endregion

#pragma region ThreadManagementImpl



/// @brief
/// @param thread
void Thread_init(Thread *thread) {
    pthread_mutex_init(&thread->mutex, nullptr);
    pthread_cond_init(&thread->cond, nullptr);
    pthread_attr_init(&thread->attr);
    pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
}
void Thread_destroy(Thread *thread) {
    pthread_mutex_destroy(&thread->mutex);
    pthread_cond_destroy(&thread->cond);
    pthread_attr_destroy(&thread->attr);
}
void Thread_start(Thread *thread, void *(*start_routine)(void *), void *arg) {
    pthread_create(&thread->id, &thread->attr, start_routine, arg);
}
void Thread_join(Thread *thread)    { pthread_join(thread->id, nullptr); }
void Thread_signal(Thread *thread) { pthread_cond_signal(&thread->cond); }
void Thread_wait(Thread *thread)  { pthread_cond_wait(&thread->cond, &thread->mutex); }
void Thread_lock(Thread *thread) { pthread_mutex_lock(&thread->mutex); }
void Thread_unlock(Thread *thread) { pthread_mutex_unlock(&thread->mutex); }

void *thread_test_worker(void *arg) {
    Thread *thread = (Thread *) arg;
    Thread_lock(thread);
    log_message(LL_INFO, sMSG("Worker thread started."));
    Thread_wait(thread);
    log_message(LL_INFO, sMSG("Worker thread signaled."));
    Thread_unlock(thread);
    return nullptr;
}

void thread_test_main(void) {
    Thread thread;
    Thread_init(&thread);
    Thread_start(&thread, thread_test_worker, &thread);
    sleep(1);
    Thread_lock(&thread);
    log_message(LL_INFO, sMSG("Main thread signaling worker."));
    Thread_signal(&thread);
    Thread_unlock(&thread);
    Thread_join(&thread);
    Thread_destroy(&thread);
}

#pragma endregion

#pragma region TracebackImpl

#define Tracer_print_last(ts) (Tracer_print_stack(ts, 16))

void Tracer_print_stack(CallStack *ts, size_t depth) {
    log_message(LL_INFO, s("Traceback (most recent call last):\n"));
    for (int i = ts->top - 1; i >= 0 && depth--; i--) {
        FixStr arg = FixStr_is_empty(ts->data[i].argument) ?
            s("(nullptr)") : ts->data[i].argument;

        log_message(LL_INFO, s("   %.*s:%d\t\tat %.*s(' %.*s ')"),
            fmt(ts->data[i].file), ts->data[i].line,
            fmt(ts->data[i].fn_name), fmt(arg)
        );
    }

    if (depth > 0) {
        log_message(LL_INFO,  sMSG("   <-- TRACEBACK FINISHED EARLY -->\n"));
    }
}
void Tracer_push(CallStack *traces, FixStr name, FixStr arg, FixStr file, size_t line) {
    if (traces->top >= 512) {
        log_message(LL_INFO, sMSG("Traceback overflow: cycling tracer history!\n"));

        #ifdef TRACEBACK_ABORT_ON_OVERFLOW
            Tracer_print_stack(traces, 16);
            abort();
        #endif

        traces->top = 0;
    }

    traces->data[traces->top].fn_name = name;
    traces->data[traces->top].argument = arg;
    traces->data[traces->top].file = file;
    traces->data[traces->top].line = line;
    traces->top++;
}

// Pop a function from the call stack
void Tracer_pop(CallStack *traces) {
    if (traces->top > 0) {
        traces->top--;
    } else {
        log_message(LL_INFO, sMSG("Traceback underflow: Mis_matched push/pop calls!\n"));
    }
}


void Tracer_test_main(void) {
    printf("Running Traceback Tests...\n");

    CallStack cs = {0};

    // Push some data
    Tracer_push(&cs, s("main"), s("arg1"), s("main.c"), 10);
    Tracer_push(&cs, s("foo"), s("arg2"), s("foo.c"), 20);
    Tracer_push(&cs, s("bar"), s("arg3"), s("bar.c"), 30);

    // Print the stack
    Tracer_print_stack(&cs, 10);

    // Pop an entry
    Tracer_pop(&cs);

    // Print the stack again
    Tracer_print_stack(&cs, 10);

    // Push another entry
    Tracer_push(&cs, s("baz"), s("arg4"), s("baz.c"), 40);

    // Print the stack
    Tracer_print_stack(&cs, 10);

    printf("Traceback Tests Completed.\n\n");
}


#pragma endregion

#pragma region PerformanceImpl



typedef struct TimePerfMetric {
    struct timespec start;
    struct timespec end;
} TimePerfMetric;

struct GlobalPerfMetrics {
    TimePerfMetric last_time;
    size_t memory;
    size_t cpu;
};


#define perf_last_metric ctx().debug.perf_metrics[ctx().debug.num_metrics - 1]

void perf_track_memory(size_t memory) {
    perf_last_metric.memory = memory;
}

void perf_track_cpu(size_t cpu) {
    perf_last_metric.cpu = cpu;
}

void perf_cpu_usage(void) {
    /// @todo
}

void perf_time_start(void) {
    clock_gettime(CLOCK_MONOTONIC, &perf_last_metric.last_time.start);
}

void perf_time_end(void) {
    clock_gettime(CLOCK_MONOTONIC, &perf_last_metric.last_time.end);
}

void perf_time_report(void) {
    struct timespec start = perf_last_metric.last_time.start;
    struct timespec end = perf_last_metric.last_time.end;

    long seconds = end.tv_sec - start.tv_sec;
    long ns = end.tv_nsec - start.tv_nsec;
    if (start.tv_nsec > end.tv_nsec) {
        --seconds;
        ns += 1000000000;
    }

    printf("Time elapsed: %ld.%09ld seconds\n", seconds, ns);
}

#pragma endregion

#pragma region CliImpl


void cli_parse_args(int argc, char *argv[], CliOption *out_opts, size_t num_opts) {
    /// @note the first entry in out_opts will be the argv data up until the first option
    /// ... defined by a leading `-` or `--`
    /// ... this is to allow for positional arguments to be parsed
    /// ... before the named options

    // Parse positional arguments
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') break;
        out_opts[0].value = FixStr_from_cstr(argv[i]);
        out_opts[0].is_set = true;
    }

    for (int i = 1; i < argc; i++) {
        for (size_t j = 0; j < num_opts; j++) {
            if (FixStr_eq(s(argv[i]), out_opts[j].name)) {
                out_opts[j].is_set = true;
                if (i + 1 < argc) {
                    out_opts[j].value = FixStr_from_cstr(argv[i + 1]);
                }
            }
        }
    }
}

void cli_print_opts(const CliOption options[], int num_options) {
    printf("Command-Line Options:\n");
    printf("----------------------------\n");
    for (int i = 0; i < num_options; i++) {
        printf("Option: %-10.*s | Set: %-5s | Value: %.*s | Default: %.*s\n",
               fmt(options[i].name),
               options[i].is_set ? "Yes" : "No",
               fmt(options[i].value),
               fmt(options[i].or_else));
    }
    printf("----------------------------\n");
}


#pragma endregion

#pragma region TestImpl


void TestReport_push(TestReport *report, bool result, FixStr message) {
    if (report->num_results > 32) {
        log_message(LL_ERROR, sMSG("Warning: TestSuite capacity exceeded."));
        return;
    }

    report->results[report->num_results] = result;
    report->messages[report->num_results] = FixStr_copy(message);
    report->num_results++;
}


void TestSuite_register(TestSuite *ts, UnitTestFnPtr fnptr) {
    if (ts->num_tests > 32) {
        log_message(LL_ERROR, sMSG("Warning: TestSuite capacity exceeded."));
        return;
    }

    ts->tests[ts->num_tests] = fnptr;
    ts->num_tests++;
}

void TestSuite_run(TestSuite *ts) {
    Arena_init(&ctx().arenas.compiler, ARENA_1MB);
    ctx_change_arena(&ctx().arenas.compiler);

    if (ts->num_tests > 32) {
        log_message(LL_ERROR, sMSG("Warning: TestSuite capacity exceeded."));
        return;
    }

    for (size_t i = 0; i < ts->num_tests; ++i) {
        ts->tests[i](&ts->reports[i]);
    }
}

void TestSuite_print(TestSuite *ts) {
    printf("=== Test Suite Report ===\n");

    for (size_t i = 0; i < ts->num_tests; ++i) {
        printf("Test #%zu:\n", i + 1);
        for (size_t j = 0; j < ts->reports[i].num_results; ++j) {
            printf("  [%s] %.*s\n",
                ts->reports[i].results[j] ? "PASS" : "FAIL",
                fmt(ts->reports[i].messages[j])
            );
        }
    }

    printf("=========================\n");
}


void unittest_sample_test_1(TestReport *r) {
    test_assert(r, 1 + 1 == 2, "Basic arithmetic works");
    test_assert(r, 2 * 2 == 5, "Deliberate failure example");
}

void unittest_sample_test_2(TestReport *r) {
    test_assert(r, strlen("test") == 4, "FixString size is correct");
}

int unittest_test_main(void) {
    TestSuite suite = test_suite(
        unittest_sample_test_1,
        unittest_sample_test_2
    );

    TestSuite_run(&suite);
    TestSuite_print(&suite);

    return 0;
}


#pragma endregion

#pragma region TestMain

int internal_test_main(void) {
    logging_test_main();
    Arena_test_main();
    string_test_main();
    // cli_test_main();
    // Box_test_main();
    Heap_test_main();
    Tracer_test_main();
    unittest_test_main();
    return 0;
}

#pragma endregion

//#endregion
///---------- ---------- ---------- IMPL: PARSING ---------- ---------- ---------- ///
//#region Implementation_Parsing


#pragma region ParsingAstImpl
#pragma endregion

#pragma region ParserLexerRules

#define lex_if_wordlike(c)\
        (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||\
        (c >= '0' && c <= '9') || (c == '_')

#define lex_if_numberlike(c)\
    (c >= '0' && c <= '9')

#define lex_comment_rule(c, d) \
    (c == '/') && (d == '/')

#define lex_word_rule(c, d) \
    ((c >= 'a' && c <= 'z') || (c == '_' && d != ' '))

#define lex_type_rule(c) \
    (c >= 'A' && c <= 'Z' && c != '_')

#define lex_tag_rule(c) \
    (c == '#')

/// lines ending `;` are discards:
///     they do not define a variable
#define lex_discard_rule(c) \
    (c == ';')

#define lex_rule_ref(c) \
    (c == '.')

#define lex_ignore_rule(c, d, e) \
    ((c == '-') && (d == '-') && (e == '-'))

#define lex_annotation_rule(c)\
    (c == ':')

#define lex_number_rule(c, d) \
    (c >= '0' && c <= '9') || (c == '.' && (d >= '0' && d <= '9'))

#define lex_bra_open_rule(c) \
    (c == '{' || c == '(' || c == '[' )

#define lex_bra_close_rule(c) \
    ( c == '}' || c == ')' || c == ']')

#define lex_string_rule(c) \
    (c == '"' || c == '\'')

#define lex_assign_rule(c, d) \
    (c == ':' && d == '=') || c == '=' ||\
    (c == '<' && d == '-') || (c == '-' && d == '>')

#define lex_op_rule(c) \
    (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || \
    c == '<' || c == '>' || c == '!' || c == '#' || c == '=')


/// @todo -- split `:` away from sep... does it ever emit anyway?
#define lex_sep_rule(c) \
    (c == ',' || c == ':')



#pragma endregion

#pragma region ParsingLexerImpl


void lex_init(ParseContext *pctx, size_t capacity, FixStr indent) {
    require_not_null(pctx);
    require_positive(capacity);
    require_positive(indent.size);

    pctx->lexer.indent = indent;

    if(pctx->lexer.data) {
        log_message(LL_WARNING, sMSG("Lexer already initialized, skipping."));
        return;
    }

    Arena_MetaValue_init_new(&pctx->lexer, capacity);
}


/// @todo think about turning off FixStr_copy()
///          by assumption that the source is in the same arena
///          and that we keep it around
///          ... and then consider parser<->lexer sharing

/// @todo this assumes `indent` is spaces, and only really uses its size
///         and so needs to be revised to handle tabs
/// @todo check for overflow & resize

void lex_source(ParseContext *pctx, FixStr source, FixStr indent) {
    require_not_null(pctx);
    ctx_change_arena(&ctx().arenas.compiler);
    lex_init(pctx, 1024, indent);
    pctx->source.lines = FixStr_lines(source, &pctx->source.size);
    lex(&pctx->lexer, &pctx->source);
}

void lex_source_file(ParseContext *pctx, FixStr filename) {
    ///@todo
}


Token* lex_record_token(struct Lexer *lex, TokenType type, size_t line, size_t col, FixStr value) {
    require_not_null(lex);
    require_not_null(lex->data);
    require_positive(capacity_ref(lex) - len_ref(lex));

    Token *tkn = &lex->data[incr_len_ref(lex)];
    tkn->type = type;
    tkn->line = line;
    tkn->col = col;
    tkn->value = value;
    return tkn;
}



/// @brief
//  If we keep the lexer arena around, and it holds the source
///    then we dont need to copy into the arena.
/// This is, by default, assumed:
///     would only change if mem perf of holding source seems OTT.
#ifdef LEX_IS_DYNAMIC
    #define emit(tt_type) \
        lex_record_token(lex, tt_type, at_line, start_col, FixStr_ncopy(line, start_col, at_col))

    #define emit_copy(tt_type, frm_col, value) \
        lex_record_token(lex, tt_type, at_line, at_col, FixStr_copy(value))
#else
    #define emit(tt_type) \
        lex_record_token(lex, tt_type, at_line, start_col, FixStr_part(line, start_col, at_col))

    #define emit_copy(tt_type, frm_col, value) \
        lex_record_token(lex, tt_type, at_line, at_col, FixStr_copy(value))
#endif


/// @brief the lexer considers up to 3 chars per token to classify
#define fst() FixStr_chr_at(line, at_col + 0)
#define snd() FixStr_chr_at(line, at_col + 1)
#define thd() FixStr_chr_at(line, at_col + 2)

void lex(struct Lexer *lex, struct  Source *src) {
    require_not_null(lex->data);
    require_positive(capacity_ref(lex));
    require_not_null(src->lines);
    require_positive(src->size);
    require_not_null(lex->indent.cstr);
    require_positive(lex->indent.size);



    bool    supress_nl = false;
    size_t  at_line = 0;
    size_t  at_col = 0;
    size_t  num_indents = 1;
    uint8_t indent_stack[64] = {0};
    #define last_indent_level() indent_stack[num_indents - 1]

    for(at_line = 0; at_line < src->size; at_line++, at_col = 0) {
        FixStr line = src->lines[at_line];

        /// @note and emits indent/dedent data per `lex->indent.size`
        while(fst() == ' ' || fst() == '\t') at_col++;
        size_t indents = at_col/lex->indent.size;

        while (indents < last_indent_level()) {
            num_indents--;
            emit_copy(TT_DEDENT, at_col, lex->indent);
        }

        while (indents > last_indent_level()) {
            if (num_indents >= 64) {
                log_message(LL_ERROR, sMSG("Max indent depth of 64, found: %d"), indents);
            }
            indent_stack[num_indents++] = indents;
            emit_copy(TT_INDENT, at_col, lex->indent);
        }

        /// @note skip empty lines
        if(line.size <= at_col) continue;

        #define eol_check() (at_col < line.size)

        /// @note process one character at a time until EOL
        while(at_col < line.size) {
            size_t start_col = at_col;

            if(lex_comment_rule(fst(), snd())) {
                /// @todo process comments by emitting a comment token
                /// ... this will require support in the parser...
                /// ... ie., we'll need to allow doc comments in specific places

                at_col = line.size - 1;
                supress_nl = true;
                break;
                // emit_copy(TT_DOC_COMMENT, 0, s("//")); break;
            }
            else if(lex_tag_rule(fst())) {
                at_col += 1;
                while(lex_if_wordlike(fst()) && eol_check()) at_col += 1;
                emit(TT_TAG);
            }
            else if(lex_type_rule(fst())) {
                while(lex_if_wordlike(fst())&& eol_check()) at_col += 1;
                emit(TT_TYPE);
            }
            else if (lex_word_rule(fst(), snd())) {
                while(lex_if_wordlike(fst())&& eol_check()) at_col += 1;
                emit(
                    lex_is_keyword(FixStr_part(line, start_col, at_col))
                    ? TT_KEYWORD
                    : TT_IDENTIFIER
                );
            }
            else if(lex_number_rule(fst(), snd())) {
                /// @todo parse more variations
                ///  ie., TT_FLOAT, TT_INT, TT_FLOAT_EXP, TT_UINT, TT_DOUBLE
                ///         0.123,   123,     1.2E3,      1234u, 0.123d

                while(lex_if_numberlike(fst())&& eol_check()) at_col += 1;

                if(fst() == '.') {
                    at_col += 1;
                    while(lex_if_numberlike(fst())&& eol_check()) at_col += 1;
                    emit(TT_DOUBLE);
                } else {
                    emit(TT_INT);
                }

            }
            else if(lex_bra_open_rule(fst())) {
                at_col += 1;
                emit(TT_BRA_OPEN);
            }
            else if(lex_bra_close_rule(fst())) {
                at_col += 1;
                emit(TT_BRA_CLOSE);
            }
            else if(lex_discard_rule(fst())) {
                at_col += 1;
                emit(TT_DISCARD);
            }
            else if(lex_string_rule(fst())) {
                /// @brief
                /// @todo This is only performs simple single-quote matching
                ///        needs to be expanded to handle triple quotes
                ///         and other string literal syntax
                /// @todo   handle lack of string termination

                const char quote = FixStr_chr_at(line, start_col);

                at_col += 1;
                while(FixStr_chr_at(line, at_col++) != quote ) {
                    log_assert(at_col < line.size, sMSG("Unterminated string"));
                }

                emit(TT_STRING);
            }
            else if(lex_assign_rule(fst(), snd())) {
                if(fst() == ':' && snd() == '=') { at_col += 1; supress_nl = true; }
                else if(fst() == '-' && snd() == '>') { at_col += 1; }
                else if(fst() == '<' && snd() == '-') { at_col += 1; }

                at_col += 1;
                emit(TT_ASSIGN);
            }
            else if(lex_ignore_rule(fst(), snd(), thd())) {
                at_col += 3;
                emit(TT_IGNORE);
            }
            else if(lex_op_rule(fst())) {
                while(lex_op_rule(fst())&& eol_check()) at_col += 1;
                emit(TT_OP);
            }
            else if(lex_rule_ref(fst())) {
                at_col +=1;
                emit(TT_DEREF);
            }
            else if (lex_sep_rule(fst())) {
                at_col += 1;
                emit(TT_SEP);
            } else if(fst() == '\n') {
                emit(TT_END);
            } else {
                at_col++;
            }
        }

        if(!supress_nl) emit_copy(TT_END, 0, s("\n"));
        supress_nl = false;
    }

    // After processing all lines,
    // emit DEDENT data until indent_stack is cleared
    while (num_indents > 1) {
        num_indents--;
        emit_copy(TT_DEDENT, 0, lex->indent);
    }

    /// @note emit final newline token
    /// @todo consider removing this / parsing implications
    emit_copy(TT_END, 0, s("\n"));
}

bool lex_is_keyword(FixStr str) {
    static const char *kws[] = {
        "const", "fn", "for", "yield", "loop",
        "in", "if", "else", "done", "let",
        "mut*", "mut", "pub", "priv", "mod",
        "try", "return", "match", "mod",
        "where", "use", "as", "dyn",
        "trait", "struct", "macro"
    };

    size_t num = sizeof(kws)/sizeof(kws[0]);
    for (size_t i = 0; i < num; i++) {
        if (FixStr_eq_cstr(str, kws[i])) return true;
    }
    return false;
}


void lex_print_data(Token *data, size_t start, size_t end, size_t marker) {
    printf("\n Tokens (%ld printed):", end - start);
    for (size_t i = start; i < end; i++) {
        printf("\n   %3ld: ", i + 1);
        printf("%.*s", fmt(tt_nameof(data[i].type)));
        if (data[i].type == TT_END) {
            printf(" `\\n`");
        } else if (!FixStr_is_empty(data[i].value)) {
            printf(" `%.*s`", fmt(data[i].value));
        }
        printf("@ %zu:%zu", data[i].line + 1, data[i].col);
        if (i == marker) printf("\t\t<--- HERE");
    }

    puts("\n");
}



/// @brief Helper function to map TokenType to ANSI color codes
const char* tt_colof(TokenType tt) {
    switch(tt) {
        case TT_KEYWORD:
            return ANSI_COL_BLUE;
        case TT_IDENTIFIER:
            return ANSI_COL_RESET;
        case TT_STRING:
            return ANSI_COL_BRIGHT_YELLOW;
        case TT_INT:
        case TT_FLOAT:
        case TT_DOUBLE:
            return ANSI_COL_BRIGHT_GREEN;
        case TT_TYPE:
            return ANSI_COL_CYAN;
        case TT_OP:
        case TT_ASSIGN:
            return ANSI_COL_RED;
        case TT_TAG:
            return ANSI_COL_BRIGHT_MAGENTA;
        case TT_BRA_OPEN:
        case TT_BRA_CLOSE:
            return ANSI_COL_BRIGHT_BLUE;
        case TT_DOC_COMMENT:
            return ANSI_COL_BRIGHT_BLACK;
        case TT_END:
            // Reset color for end data (like newlines)
            return ANSI_COL_RESET;
        default:
            // Default color for other data
            return ANSI_COL_WHITE;
    }
}
FixStr data_to_ansi_new(struct Lexer lexed) {
    FixStr result = FixStr_empty();
    FixStr reset_str = FixStr_from_cstr(ANSI_COL_RESET);
    size_t indent_level = 0;
    bool after_nl = true;

    for (size_t i = 0; i < len(lexed); i++) {
        Token tok = lexed.data[i];
        if(!after_nl && (tok.type != TT_SEP  && tok.type != TT_BRA_OPEN))
            result = FixStr_glue_new(result, FixStr_from_cstr(" "));

        switch (tok.type) {
            case TT_INDENT:
                indent_level++;
                break;

            case TT_DEDENT:
                if (indent_level > 0) {
                    indent_level--;
                }
                if(indent_level == 0) {
                    result = FixStr_glue_new(result, FixStr_from_cstr("\n"));
                }
                break;

            case TT_END:
                // Append newline and reset ANSI colors
                result = FixStr_glue_new(result, FixStr_from_cstr("\n"));
                result = FixStr_glue_new(result, reset_str);
                after_nl = true;
                break;

            case TT_ASSIGN:
                if(FixStr_chr_at(tok.value, 0) == ':') {
                    result = FixStr_glue_new(result, FixStr_from_cstr(":=\n"));
                    after_nl = true;
                } else {
                    result = FixStr_glue_new(result, tok.value);
                }
                break;
            case TT_IGNORE:
            case TT_DISCARD:
                // Append these data directly without coloring
                if (!FixStr_is_empty(tok.value)) {
                    result = FixStr_glue_new(result, tok.value);
                }
                break;

            default:
                if (after_nl) {
                    FixStr indent = FixStr_repeat_new(lexed.indent, indent_level);
                    result = FixStr_glue_new(result, indent);
                    after_nl = false;
                }

                if (!FixStr_is_empty(tok.value)) {
                    FixStr colored = FixStr_col_new(tt_colof(tok.type), tok.value);
                    result = FixStr_glue_new(result, colored);
                }
                break;
        }
    }

    return FixStr_glue_new(result, reset_str);
}



void lex_test_main(void) {
    FixStr source = s(
        "const test = 1234\n\n"
        "fn main() :=\n"
        "    log(test)\n"
        "\n"
    );

    FixStr_println(sMSG("Lexer Tests:"));
    lex_source(ctx_parser(), source, s("    "));
    lex_print_all(ctx_parser()->lexer);
}

#pragma endregion

#pragma region ParsingAstApiImpl


// Assume FixAst_new and Ast are defined elsewhere
Ast *FixAst_new(AstType t, size_t line, size_t col) {
    Ast *node = Arena_alloc(sizeof(Ast));
    if (!node) { error_oom(); return nullptr; }
    node->type = t;
    node->line = line;
    node->col = col;
    return node;
}

// static inlined constructors


/// @todo revise to construct from args
/// @brief Creates a new AST node for a type annotation.
/// @param token The token associated with the type annotation (for location tracking).
/// @return A pointer to the newly created Ast.
Ast *Ast_type_annotation_new(Token *token) {
    Ast *node = Arena_alloc(sizeof(Ast));
    node->type = AST_TYPE_ANNOTATION;
    node->line = token->line;
    node->col = token->col;
    node->type_annotation.qualifier = FixStr_empty();
    node->type_annotation.base_type = FixStr_empty();

    clib_memset_zero_safe(&node->type_annotation.type_params,
        sizeof(node->type_annotation.type_params),
        sizeof(node->type_annotation.type_params)
    );

    node->type_annotation.num_type_params = 0;
    node->type_annotation.size_constraint = FixStr_empty();
    return node;
}

/// @brief Creates a new AST node for a destructuring assignment.
Ast *Ast_destructure_assign_new(Token *token, Ast **variables, size_t num_variables, Ast *expression) {
    Ast *node = Arena_alloc(sizeof(Ast));
    node->type = AST_DESTRUCTURE_ASSIGN;
    node->line = token->line;
    node->col = token->col;
    node->destructure_assign.variables = variables;
    node->destructure_assign.num_variables = num_variables;
    node->destructure_assign.expression = expression;
    return node;
}


/// @brief Creates a new AST node for a generic type.
Ast *Ast_generic_type_new(Token *token, FixStr base_type) {
    return nullptr;
    // Ast *node = Arena_alloc(sizeof(Ast));
    // node->type = AST_GENERIC_TYPE;
    // node->line = token->line;
    // node->col = token->col;
    // node->generic_type.base_type = base_type;
    // node->generic_type.type_params = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(Ast *));
    // node->generic_type.num_type_params = 0;
    // node->generic_type.constraints = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(struct TypeConstraint));
    // node->generic_type.num_constraints = 0;
    // node->generic_type.type_params_capacity = ARRAY_SIZE_SMALL;
    // return node;
}

// Identifier
static inline Ast *Ast_identifier_new(Token *head, FixStr name) {
    Ast *node = FixAst_new(AST_ID, head->line, head->col);
    node->id.name = name;
    return node;
}

// Double Value
static inline Ast *Ast_double_new(Token *head, double double_val) {
    Ast *node = FixAst_new(AST_DOUBLE, head->line, head->col);
    node->dble.value = double_val;
    return node;
}


static inline Ast *Ast_int_new(Token *head, int integer) {
    Ast *node = FixAst_new(AST_INT, head->line, head->col);
    node->integer.value = integer;
    return node;
}

static inline Ast *Ast_discard_new(Token *head) {
    return FixAst_new(AST_DISCARD, head->line, head->col);
}

//// @todo: uint, etc.?

// FixString Value
static inline Ast *Ast_string_new(Token *head, FixStr value) {
    Ast *node = FixAst_new(AST_STR, head->line, head->col);
    node->str.value = value;
    return node;
}

// Tag
static inline Ast *Ast_tag_new(Token *head, FixStr name) {
    Ast *node = FixAst_new(AST_TAG, head->line, head->col);
    node->tag.name = name;
    return node;
}

// Binary Operation
static inline Ast *Ast_bop_new(Token *head, FixStr op, Ast *left, Ast *right) {
    Ast *node = FixAst_new(AST_BOP, head->line, head->col);
    node->bop.op = op;
    node->bop.left = left;
    node->bop.right = right;
    return node;
}

// Unary Operation
static inline Ast *Ast_uop_new(Token *head, FixStr op, Ast *operand) {
    Ast *node = FixAst_new(AST_UOP, head->line, head->col);
    node->uop.op = op;
    node->uop.operand = operand;
    return node;
}

// Function Call
static inline Ast *Ast_fn_call_new(Token *head, Ast *callee, Ast **args, size_t num_args) {
    Ast *node = FixAst_new(AST_FN_DEF_CALL, head->line, head->col);
    node->call.callee = callee;
    node->call.args = args;
    node->call.num_args = num_args;
    return node;
}

// Method Call
static inline Ast *Ast_method_call_new(Token *head,
     Ast *target, FixStr method, Ast *call) {
    Ast *node = FixAst_new(AST_METHOD_CALL, head->line, head->col);
    node->method_call.target = target;
    node->method_call.method = method;
    node->method_call.args = call->call.args;
    node->method_call.num_args = call->call.num_args;
    return node;
}

// Member Access
static inline Ast *Ast_member_access_new(Token *head,
     Ast *target, FixStr property) {
    Ast *node = FixAst_new(AST_MEMBER_ACCESS, head->line, head->col);
    node->member_access.target = target;
    node->member_access.property = property;
    return node;
}

// Block
static inline Ast *Ast_block_new(Token *head,
     Ast **statements, size_t num_statements, bool transparent) {
    Ast *node = FixAst_new(AST_BLOCK, head->line, head->col);
    node->block.statements = statements;
    node->block.num_statements = num_statements;
    node->block.transparent = transparent;
    return node;
}

// Function
static inline Ast *Ast_fn_def_new(Token *head,
     FixStr name, struct FnParam *params, size_t num_params, Ast *body,
     bool is_macro, bool is_generator) {
    Ast *node = FixAst_new(AST_FN_DEF, head->line, head->col);
    node->fn.name = name;
    node->fn.params = params;
    node->fn.num_params = num_params;
    node->fn.body = body;
    node->fn.is_macro = is_macro;
    node->fn.is_generator = is_generator;
    return node;
}

/// @todo unused -- handled by Ast_fn_def_new
// Anonymous Function
// static inline Ast *Ast_fn_anon_def_new(Token *head,
//      struct AnonFnParam *params, size_t num_params, Ast *body) {
//     Ast *node = FixAst_new(AST_FN_DEF_ANON, head->line, head->col);
//     node->fn_anon.params = params;
//     node->fn_anon.num_params = num_params;
//     node->fn_anon.body = body;
//     return node;
// }

// FixStruct Literal
static inline Ast *Ast_object_literal_new(Token *head,
     FixStr struct_name, struct AstObjectField *fields, size_t num_fields) {
    Ast *node = FixAst_new(AST_OBJECT_LITERAL, head->line, head->col);
    node->object_literal.struct_name = struct_name;
    node->object_literal.fields = fields;
    node->object_literal.num_fields = num_fields;
    return node;
}

// If
static inline Ast *Ast_if_new(Token *head,
     Ast *condition, Ast *body, Ast *else_body) {
    Ast *node = FixAst_new(AST_IF, head->line, head->col);
    node->if_stmt.condition = condition;
    node->if_stmt.body = body;
    node->if_stmt.else_body = else_body;
    return node;
}

/// @brief Creates a new AST node for a documentation comment.
/// @todo
Ast *Ast_doc_comment_new(Token *token, FixStr comment) {
    return nullptr;
    // Ast *node = Arena_alloc(sizeof(Ast));
    // node->type = AST_DOC_COMMENT;
    // node->line = token->line;
    // node->col = token->col;
    // node->doc_comment.comment = comment;
    // return node;
}


// Loop
/// @todo: consider whether this should be a different AST structure
static inline Ast *Ast_loop_if_new(Token *head,
    Ast *condition, Ast *body) {
    Ast *node = FixAst_new(AST_LOOP, head->line, head->col);
    node->loop.bindings = nullptr;
    node->loop.num_bindings = 0;
    node->loop.body = body;

    /// @todo: given condition != yields.. are both needed?
    ///...... is this assertion correct?
    node->loop.condition = condition;
    node->loop.yields = false;
    return node;
}


static inline Ast *Ast_loop_new(Token *head,
    Ast **binds, size_t num_bindings, Ast *body) {
    Ast *node = FixAst_new(AST_LOOP, head->line, head->col);
    node->loop.bindings = binds;
    node->loop.num_bindings = num_bindings;
    node->loop.body = body;
    node->loop.condition = nullptr;
    node->loop.yields = true;
    return node;
}

// Let
static inline Ast *Ast_let_new(Token *head, Ast **bindings, size_t num_bindings, Ast *body) {
    Ast *node = FixAst_new(AST_LEF_DEF, head->line, head->col);
    node->let_stmt.bindings = bindings;
    node->let_stmt.num_bindings = num_bindings;
    node->let_stmt.body = body;
    return node;
}

// Binding
static inline Ast *Ast_binding_new(Token *head,
     FixStr name, Ast *expression, FixStr assign_op) {
    Ast *node = FixAst_new(AST_BINDING, head->line, head->col);
    node->binding.identifier = name;
    node->binding.expression = expression;
    node->binding.assign_op = assign_op;
    return node;
}

// Return
static inline Ast *Ast_return_new(Token *head,
     Ast *value) {
    Ast *node = FixAst_new(AST_RETURN, head->line, head->col);
    node->return_stmt.value = value;
    return node;
}

// Match
static inline Ast *Ast_match_new(Token *head,
     Ast *expr, struct MatchCase *cases, size_t num_cases) {
    Ast *node = FixAst_new(AST_MATCH, head->line, head->col);
    node->match.expression = expr;
    node->match.cases = cases;
    node->match.num_cases = num_cases;
    return node;
}

// Dictionary
static inline Ast *Ast_dict_new(Token *head,
     struct AstDictEntry *data, size_t size) {
    Ast *node = FixAst_new(AST_DICT, head->line, head->col);
    node->dict.data = data;
    node->dict.size = size;
    return node;
}

// Vector
static inline Ast *Ast_vec_new(Token *head,
     Ast **data, size_t size) {
    Ast *node = FixAst_new(AST_VEC, head->line, head->col);
    node->vec.data = data;
    node->vec.size = size;
    return node;
}

// Use
static inline Ast *Ast_use_new(Token *head,
     FixStr module_path, FixStr alias, bool wildcard) {
    Ast *node = FixAst_new(AST_USE, head->line, head->col);
    node->use_stmt.module_path = module_path;
    node->use_stmt.alias = alias;
    node->use_stmt.wildcard = wildcard;
    return node;
}

// Mutation
static inline Ast *Ast_mutation_new(Token *head,
     Ast *target, FixStr op, Ast *value, bool is_broadcast) {
    Ast *node = FixAst_new(AST_MUTATION, head->line, head->col);
    node->mutation.target = target;
    node->mutation.op = op;
    node->mutation.value = value;
    node->mutation.is_broadcast = is_broadcast;
    return node;
}

// Constant
static inline Ast *Ast_const_new(Token *head,
     FixStr name, Ast *value) {
    Ast *node = FixAst_new(AST_CONST_DEF, head->line, head->col);
    node->const_stmt.name = name;
    node->const_stmt.value = value;
    return node;
}

// Yield
static inline Ast *Ast_yield_new(Token *head,
     Ast *value) {
    Ast *node = FixAst_new(AST_YIELD, head->line, head->col);
    node->yield_stmt.value = value;
    return node;
}

// Ignore
static inline Ast *Ast_ignore_new(Token *head) {
    Ast *node = FixAst_new(AST_IGNORE, head->line, head->col);
    // No additional fields to set
    return node;
}


/// @todo -- unused?
// Expression
// static inline Ast *Ast_expression_new(Token *head,
//      Ast *expression) {
//     Ast *node = FixAst_new(AST_EXPRESSION, head->line, head->col);
//     node->exp_stmt.expression = expression;
//     return node;
// }


/// @todo -- to implement
// Module
// static inline Ast *Ast_module_new(Token *head, FixStr name) {
// }




/// @todo
static inline Ast *Ast_struct_new(Token *head, FixStr name, struct FixStructField *fields, size_t num_fields) {
    Ast *node = Arena_alloc(sizeof(Ast));
    node->type = AST_STRUCT_DEF;
    node->line = head->line;
    node->col = head->col;
    node->struct_def.name = name;
    node->struct_def.fields = fields;
    node->struct_def.num_fields = num_fields;
    return node;
}

// static inline Ast *Ast_trait_new(Token *head, FixStr name, struct TraitMethod *methods, size_t num_methods) {
//     Ast *node = Arena_alloc(sizeof(Ast));
//     node->type = AST_TRAIT;
//     node->line = head->line;
//     node->col = head->col;
//     node->trait.name = name;
//     node->trait.methods = methods;
//     node->trait.num_methods = num_methods;
//     return node;
// }



/// @todo -- unused
// static inline struct MatchCase *Ast_match_arm_new(Token *head, Ast *pattern, Ast *expression) {
//     return nullptr;
// }


#pragma endregion

#pragma region ParsingAstFixStrImpl




FixStr Ast_to_FixStr(Ast *node) {
    if (node == nullptr) {
        return FixStr_empty();
    }
    return Ast_to_json(node, 0);
}

void Ast_print(Ast *node) {
    FixStr_puts(Ast_to_FixStr(node));
}

FixStr Ast_to_json(Ast *node, size_t indent) {
    if (node == nullptr) {
        return FixStr_repeat_new(s(" "), indent);
    }
    return FixAst_to_json(node, indent);
}

FixStr FixAst_to_json(Ast *node, size_t indent) {
    if(node == nullptr) return s("nullptr");

    switch (node->type) {
        case AST_BLOCK:
            return Ast_block_to_json(node, indent);
        case AST_IF:
            return Ast_if_to_json(node, indent);
        case AST_FN_DEF:
            return Ast_fn_def_to_json(node, indent);
        case AST_FN_DEF_CALL:
            return Ast_fn_call_to_json(node, indent);
        case AST_BOP:
            return Ast_binary_op_to_json(node, indent);
        case AST_UOP:
            return Ast_unary_op_to_json(node, indent);
        case AST_FN_DEF_ANON:
            return Ast_fn_anon_to_json(node, indent);
        case AST_METHOD_CALL:
            return Ast_method_call_to_json(node, indent);
        case AST_MEMBER_ACCESS:
            return Ast_member_access_to_json(node, indent);
        case AST_CONST_DEF:
            return Ast_const_to_json(node, indent);
        case AST_LEF_DEF:
            return Ast_let_to_json(node, indent);
        case AST_DICT:
            return Ast_dict_to_json(node, indent);
        case AST_VEC:
            return Ast_vec_to_json(node, indent);
        case AST_USE:
            return Ast_use_to_json(node, indent);
        case AST_INT:
        case AST_FLOAT:
        case AST_DOUBLE:
        case AST_STR:
            return Ast_literal_to_json(node, indent);
        case AST_ID:
            return Ast_identifier_to_json(node, indent);
        case AST_LOOP:
            return Ast_loop_to_json(node, indent);
        case AST_FOR:
            return Ast_for_to_json(node, indent);
        case AST_BINDING:
            return Ast_binding_to_json(node, indent);
        case AST_RETURN:
            return Ast_return_to_json(node, indent);
        case AST_MATCH:
            return Ast_match_to_json(node, indent);
        case AST_MUTATION:
            return Ast_mutation_to_json(node, indent);
        case AST_YIELD:
            return Ast_yield_to_json(node, indent);
        case AST_IGNORE:
            return Ast_ignore_to_json(node, indent);
        case AST_EXPRESSION:
            return Ast_expression_to_json(node, indent);
        case AST_MODULE:
            return Ast_module_to_json(node, indent);
        case AST_STRUCT_DEF:
            return Ast_struct_def_to_json(node, indent);
        case AST_OBJECT_LITERAL:
            return Ast_object_literal_to_json(node, indent);
        case AST_TRAIT:
            return Ast_trait_to_json(node, indent);
        case AST_TAG:
            return Ast_literal_to_json(node, indent);
        default:
            // Wrap in an object
            return FixStr_fmt_new(
                s("%s{\n%s\"unknown_node\": {\"type\": \"%.*s\"}\n%s}"),
                cstr_repeat_new(" ", indent),
                cstr_repeat_new(" ", indent + 1),
                fmt(Ast_nameof(node->type)),
                cstr_repeat_new(" ", indent)
            );
    }
}

FixStr Ast_dict_to_json(Ast *node, size_t indent) {
    // { "dict": { "key": value, ... } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"dict\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    for (size_t i = 0; i < node->dict.size; i++) {
        // Each key, value pair
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->dict.data[i].key, indent + 2)
        );
        result = FixStr_glue_new(result, s(": "));
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->dict.data[i].value, indent + 2)
        );
        if (i < node->dict.size - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent))
    );
    return result;
}

FixStr Ast_vec_to_json(Ast *node, size_t indent) {
    // { "vec": [ data... ] }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"vec\": [\n"), cstr_repeat_new(" ", indent + 1))
    );
    for (size_t i = 0; i < node->vec.size; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->vec.data[i], indent + 2)
        );
        if (i < node->vec.size - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent))
    );
    return result;
}

FixStr Ast_use_to_json(Ast *node, size_t indent) {
    // { "use": {"module_path": "...", "alias": "...", "wildcard": true/false } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"use\": {\"module_path\": \"%.*s\", \"alias\": \"%.*s\", \"wildcard\": %s}\n"),
            cstr_repeat_new(" ", indent + 1),
            fmt(node->use_stmt.module_path),
            fmt(node->use_stmt.alias),
            node->use_stmt.wildcard ? "true" : "false"
        )
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_ignore_to_json(Ast *node, size_t indent) {
    // { "ignore": {} }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"ignore\": {}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_yield_to_json(Ast *node, size_t indent) {
    // { "yield": {} } – if there's no sub-data
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"yield\": {}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_expression_to_json(Ast *node, size_t indent) {
    // { "expression": <child> }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"expression\": "), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->exp_stmt.expression, indent + 2));
    result = FixStr_glue_new(result, s("\n"));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent))
    );
    return result;
}

FixStr Ast_module_to_json(Ast *node, size_t indent) {
    // { "module": {"name": "..."} }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"module\": {\"name\": \"%.*s\"}\n"),
            cstr_repeat_new(" ", indent + 1),
            fmt(node->mod_stmt.name)
        )
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_struct_def_to_json(Ast *node, size_t indent) {
    // { "struct": { "name": "...", "fields": [ ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"struct\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"name\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->struct_def.name)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"fields\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->struct_def.num_fields; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->struct_def.fields[i].type, indent + 3)
        );
        if (i < node->struct_def.num_fields - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s]\n%s}\n"),
            cstr_repeat_new(" ", indent + 2),
            cstr_repeat_new(" ", indent + 1)
        )
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_const_to_json(Ast *node, size_t indent) {
    // { "const": { "name": "...", "value": <child> } }
    FixStr value = Ast_to_json(node->const_stmt.value, indent + 2);
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"const\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"name\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->const_stmt.name)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"value\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, value);
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_trait_to_json(Ast *node, size_t indent) {
    // { "trait": {"name": "...", "methods": [ ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"trait\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"name\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->trait.name)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"methods\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->trait.num_methods; i++) {
        // Example placeholder
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s\"TODO\""), cstr_repeat_new(" ", indent + 3))
        );
        if (i < node->trait.num_methods - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_object_literal_to_json(Ast *node, size_t indent) {
    // { "object_literal": { "struct_name": "...", "fields": [ ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"object_literal\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"struct_name\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->object_literal.struct_name)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"fields\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->object_literal.num_fields; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->object_literal.fields[i].value, indent + 3)
        );
        if (i < node->object_literal.num_fields - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_block_to_json(Ast *node, size_t indent) {
    // { "block": { "statements": [ ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"block\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"statements\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->block.num_statements; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->block.statements[i], indent + 3)
        );
        if (i < node->block.num_statements - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_if_to_json(Ast *node, size_t indent) {
    // { "if": { "condition": <child>, "then": <child>, "else": <child>? } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"if\": {\n"), cstr_repeat_new(" ", indent + 1))
    );

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"condition\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->if_stmt.condition, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"then\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->if_stmt.body, indent + 3));

    if (node->if_stmt.else_body) {
        result = FixStr_glue_new(result, s(",\n"));
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s\"else\": "), cstr_repeat_new(" ", indent + 2))
        );
        result = FixStr_glue_new(result, Ast_to_json(node->if_stmt.else_body, indent + 3));
    }

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_fn_def_to_json(Ast *node, size_t indent) { // NOLINT(*-no-recursion)
    // { "function_definition": { "name": "...", "parameters": [...], "body": <child> } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"function_definition\": {\n"), cstr_repeat_new(" ", indent + 1))
    );

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"name\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->fn.name)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"parameters\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->fn.num_params; i++) {
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(
                s("%s\"%.*s\""),
                cstr_repeat_new(" ", indent + 3),
                fmt(node->fn.params[i].name)
            )
        );
        if (i < node->fn.num_params - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s],\n"), cstr_repeat_new(" ", indent + 2))
    );

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"body\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->fn.body, indent + 2));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_fn_call_to_json(Ast *node, size_t indent) {
    // { "function_call": { "callee": <child>, "arguments": [ ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"function_call\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"callee\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->call.callee, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"arguments\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->call.num_args; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->call.args[i], indent + 3)
        );
        if (i < node->call.num_args - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_binary_op_to_json(Ast *node, size_t indent) {
    // { "binary_operation": { "operator": "+", "left": <child>, "right": <child> } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"binary_operation\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"operator\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->bop.op)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"left\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->bop.left, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"right\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->bop.right, indent + 3));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_unary_op_to_json(Ast *node, size_t indent) {
    // { "unary_operation": { "operator": "-", "operand": <child> } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"unary_operation\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"operator\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->uop.op)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"operand\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->uop.operand, indent + 3));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_literal_to_json(Ast *node, size_t indent) {
    // Wrap each literal in { "integer": 42 }, { "float": 3.14 }, etc.
    switch (node->type) {
        case AST_INT:
            return FixStr_fmt_new(
                s("%s{\"integer\": %lld}"),
                cstr_repeat_new(" ", indent),
                node->integer.value
            );
        case AST_FLOAT:
        case AST_DOUBLE:
            return FixStr_fmt_new(
                s("%s{\"float\": %f}"),
                cstr_repeat_new(" ", indent),
                node->dble.value
            );
        case AST_STR:
            return FixStr_fmt_new(
                s("%s{\"string\": \"%.*s\"}"),
                cstr_repeat_new(" ", indent),
                fmt(node->str.value)
            );
        case AST_TAG:
            return FixStr_fmt_new(
                s("%s{\"tag\": \"%.*s\"}"),
                cstr_repeat_new(" ", indent),
                fmt(node->tag.name)
            );
        default:
            return FixStr_fmt_new(
                s("%s{\"unknown_literal\": {\"type\": \"%s\"}}"),
                cstr_repeat_new(" ", indent),
                Ast_nameof(node->type)
            );
    }
}

FixStr Ast_identifier_to_json(Ast *node, size_t indent) {
    // { "identifier": "someName" }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"identifier\": \"%.*s\"\n"),
            cstr_repeat_new(" ", indent + 1),
            fmt(node->id.name)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent))
    );
    return result;
}

FixStr Ast_fn_anon_to_json(Ast *node, size_t indent) {
    // { "anonymous_function": { "parameters": [...], "body": <child> } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"anonymous_function\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"parameters\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->fn_anon.num_params; i++) {
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(
                s("%s\"%.*s\""),
                cstr_repeat_new(" ", indent + 3),
                fmt(node->fn_anon.params[i].name)
            )
        );
        if (i < node->fn_anon.num_params - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s],\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"body\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->fn_anon.body, indent + 2));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_method_call_to_json(Ast *node, size_t indent) {
    // { "method_call": { "target": <child>, "method": "...", "arguments": [ ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"method_call\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"target\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->method_call.target, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"method\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->method_call.method)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"arguments\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->method_call.num_args; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->method_call.args[i], indent + 3)
        );
        if (i < node->method_call.num_args - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_member_access_to_json(Ast *node, size_t indent) {
    // { "member_access": { "target": <child>, "property": "..." } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"member_access\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"target\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->member_access.target, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"property\": \"%.*s\"\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->member_access.property)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_let_to_json(Ast *node, size_t indent) {
    // { "let": { "bindings": [...], "body": <child> } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"let\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"bindings\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->let_stmt.num_bindings; i++) {
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->let_stmt.bindings[i], indent + 3)
        );
        if (i < node->let_stmt.num_bindings - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s],\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"body\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->let_stmt.body, indent + 2));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_loop_to_json(Ast *node, size_t indent) {
    // { "loop": { "condition": <child>?, "bindings": [...], "body": <child>, "yields": bool } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"loop\": {\n"), cstr_repeat_new(" ", indent + 1))
    );

    if (node->loop.condition) {
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s\"condition\": "), cstr_repeat_new(" ", indent + 2))
        );
        result = FixStr_glue_new(result, Ast_to_json(node->loop.condition, indent + 3));
        result = FixStr_glue_new(result, s(",\n"));
    }

    if (node->loop.num_bindings > 0) {
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s\"bindings\": [\n"), cstr_repeat_new(" ", indent + 2))
        );
        for (size_t i = 0; i < node->loop.num_bindings; i++) {
            result = FixStr_glue_new(
                result,
                Ast_to_json(node->loop.bindings[i], indent + 3)
            );
            if (i < node->loop.num_bindings - 1) {
                result = FixStr_glue_new(result, s(",\n"));
            } else {
                result = FixStr_glue_new(result, s("\n"));
            }
        }
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s],\n"), cstr_repeat_new(" ", indent + 2))
        );
    }

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"body\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->loop.body, indent + 2));
    result = FixStr_glue_new(result, s(",\n"));

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"yields\": %s\n"),
            cstr_repeat_new(" ", indent + 2),
            node->loop.yields ? "true" : "false"
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_for_to_json(Ast *node, size_t indent) {
    // For now, we treat 'for' the same as 'loop'.
    return Ast_loop_to_json(node, indent);
}

FixStr Ast_binding_to_json(Ast *node, size_t indent) {
    // { "binding": { "identifier": "...", "assign_op": "...", "expression": <child> } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"binding\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"identifier\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->binding.identifier)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"assign_op\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->binding.assign_op)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"expression\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->binding.expression, indent + 3));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("\n%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_return_to_json(Ast *node, size_t indent) {
    // { "return": <child or nullptr> }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"return\": "), cstr_repeat_new(" ", indent + 1))
    );
    if (node->return_stmt.value) {
        result = FixStr_glue_new(result, Ast_to_json(node->return_stmt.value, indent + 2));
    } else {
        result = FixStr_glue_new(result, s("nullptr"));
    }
    result = FixStr_glue_new(result, s("\n"));
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_match_to_json(Ast *node, size_t indent) {
    // { "match": { "expression": <child>, "cases": [ { "condition": <child>?, "expression": <child> }, ... ] } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"match\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"expression\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->match.expression, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));

    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"cases\": [\n"), cstr_repeat_new(" ", indent + 2))
    );
    for (size_t i = 0; i < node->match.num_cases; i++) {
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent + 3))
        );
        if (node->match.cases[i].condition) {
            result = FixStr_glue_new(
                result,
                FixStr_fmt_new(s("%s\"condition\": "), cstr_repeat_new(" ", indent + 4))
            );
            result = FixStr_glue_new(
                result,
                Ast_to_json(node->match.cases[i].condition, indent + 5)
            );
            result = FixStr_glue_new(result, s(",\n"));
        }
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("%s\"expression\": "), cstr_repeat_new(" ", indent + 4))
        );
        result = FixStr_glue_new(
            result,
            Ast_to_json(node->match.cases[i].expression, indent + 5)
        );
        result = FixStr_glue_new(
            result,
            FixStr_fmt_new(s("\n%s}"), cstr_repeat_new(" ", indent + 3))
        );
        if (i < node->match.num_cases - 1) {
            result = FixStr_glue_new(result, s(",\n"));
        } else {
            result = FixStr_glue_new(result, s("\n"));
        }
    }
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s]\n"), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

FixStr Ast_mutation_to_json(Ast *node, size_t indent) {
    // { "mutation": { "target": <child>, "operator": "...", "value": <child>, "broadcast": bool } }
    FixStr result = FixStr_fmt_new(s("%s{\n"), cstr_repeat_new(" ", indent));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"mutation\": {\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"target\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->mutation.target, indent + 3));
    result = FixStr_glue_new(result, s(",\n"));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s("%s\"operator\": \"%.*s\",\n"),
            cstr_repeat_new(" ", indent + 2),
            fmt(node->mutation.op)
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s\"value\": "), cstr_repeat_new(" ", indent + 2))
    );
    result = FixStr_glue_new(result, Ast_to_json(node->mutation.value, indent + 3));
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(
            s(",\n%s\"broadcast\": %s\n"),
            cstr_repeat_new(" ", indent + 2),
            node->mutation.is_broadcast ? "true" : "false"
        )
    );
    result = FixStr_glue_new(
        result,
        FixStr_fmt_new(s("%s}\n"), cstr_repeat_new(" ", indent + 1))
    );
    result = FixStr_glue_new(result, FixStr_fmt_new(s("%s}"), cstr_repeat_new(" ", indent)));
    return result;
}

#pragma endregion

#pragma region ParsingParserImpl

#define data_remain() (p->parser.current_token < p->lexer.meta.size)
#define depth_is_bounded() (p->parser.depth++ <= 256)
#define token_null(t) (t.value.size > 0)

#define peek() parse_peek(p, 0)
#define peek_ahead() parse_peek(p, 1)
#define peek_eq(val) (data_remain() && FixStr_eq(parse_peek(p, 0)->value, val))
#define peek_eq_chr(cvar) (data_remain() && FixStr_eq_chr(parse_peek(p, 0)->value, cvar))
#define peek_is(tkt) (data_remain() && (parse_peek(p, 0)->type == tkt))
#define consume(type, err)  parse_consume(p, type, FixStr_empty(), err)
#define consume_specific(type, value, errmsg) parse_consume(p, type, value, errmsg)
#define consume_expected(tkt) parse_consume(p, tkt, FixStr_empty(), FixStr_empty())


#define parse_not_implemented()\
    parse_error(p, peek(), sMSG("Parser: this syntax is not yet implemented!"));\
    consume_expected(peek()->type);\
    return nullptr

static inline Token *parse_peek(ParseContext *p, size_t offset) {
    if(data_remain() && token_null(p->lexer.data[p->parser.current_token + offset])) {
        return &p->lexer.data[p->parser.current_token + offset];
    } else return nullptr;
}

static inline Token *parse_consume(ParseContext *p, TokenType type, FixStr value, FixStr errmsg) {
    if((value.size && peek_eq(value) && peek_is(type)) || peek_is(type)) {
        return &p->lexer.data[p->parser.current_token++];
    } else {
        parse_error(p, peek(), errmsg);
        return nullptr;
    }
}





void parse(ParseContext *p) {
    size_t count = 0;
    Ast **statements = Arena_alloc(ARRAY_SIZE_MEDIUM * sizeof(Ast *));
    Ast *result = nullptr;

    while(data_remain() && depth_is_bounded()) {
        pctx_trace(tt_nameof(peek()->type));
        result = parse_statement(p);
        if(result != nullptr) {
            statements[count++] = result;
        }
        p->parser.depth = 0;
        require_medium_array(count);
    }

    p->parser.current_token = 0;
    p->parser.depth = 0;
    ctx_parser_append(Ast_block_new(peek(), statements, count, true));
}


void parse_print_FixAst(Ast *ast, size_t depth);

void parse_error(ParseContext *pctx, Token *token, FixStr message) {
    FixStr_println(s("Parser Traceback:"));
    Tracer_print_last(&pctx->parser.traces);

    log_message(LL_INFO, s("Current Token %d: %.*s %.*s\n"),
        pctx->parser.current_token, fmt(tt_nameof(token->type)), fmt(token->value)
    );

    log_message(LL_ERROR,
        FixStr_col_new(ANSI_COL_BRIGHT_YELLOW, s("Parse Error: %.*s\n")), fmt(message)
    );

    require_resources_cleanup(pctx);
}



// #define trace(moreinfo) /// @todo

// Token *parse_consume_value(ParseContext *p, FixStr value, TokenType type, FixStr error_message);

/// @brief
/// @example
///     log("Hello World"); // discards return value
///     const result = 10 * 2
///
/// @param p
/// @return
Ast *parse_statement(ParseContext *p) {
    log_assert(depth_is_bounded(), sMSG("Beyond max depth"));
    log_assert(data_remain(), sMSG("No data remain!"));
    require_not_null(peek());


    if(peek_is(TT_END)) {
        pctx_trace(tt_nameof(peek()->type));
        consume_expected(TT_END);
        return nullptr;
    }

    pctx_trace(peek()->value);
    if(peek_is(TT_KEYWORD)) {
        return parse_keyword_statement(p, false);
    } else if(peek_is(TT_INDENT)) {
        return parse_block(p);
    } else if(peek_is(TT_DISCARD)) {
        consume_expected(TT_DISCARD);
        return Ast_discard_new(peek());
    } else {
        /// @todo: expr statements should end on a nl
        return parse_expression(p, 0);
    }
}

static inline uint8_t parse_peek_precedence(ParseContext *p) {
    if (!peek()) return 0;

    if (peek_is(TT_ASSIGN)) return 1;
    if (peek_eq_chr('|')) return 1;
    if (peek_eq_chr('&')) return 2;
    if (peek_eq_chr('=') || peek_eq_chr('!')) return 4;
    if (peek_eq_chr('<') || peek_eq_chr('>')) return 5;
    if (peek_eq_chr('+') || peek_eq_chr('-')) return 6;
    if (peek_eq_chr('*') || peek_eq_chr('/') || peek_eq_chr('%')) return 7;
    if (peek_eq_chr('.')) return 8;
    if (peek_is(TT_SEP)) return 9;
    if (peek_is(TT_DISCARD)) return 10;

    parse_error(p, peek(), sMSG("Operator precedence not found."));
    return 99;
}



/// @brief
/// @example
///     g(10 * (2/3) + 300)
/// @param p
/// @param precedence
/// @return
Ast *parse_expression(ParseContext *p, size_t precedence) {
    #define peek_is_expression_end() (peek_eq_chr(',') || peek_eq_chr(':') ||\
    peek_is(TT_BRA_CLOSE) || peek_is(TT_END) || peek_is(TT_ASSIGN) || peek_is(TT_DISCARD))

    log_assert(depth_is_bounded(), sMSG("Beyond max depth"));
    log_assert(data_remain(), sMSG("No data remain!"));
    pctx_trace(peek()->value);

    if (peek_is_expression_end()) return nullptr;

    Ast *left = parse_expression_part(p);
    while (data_remain() && !peek_is_expression_end()) {
        size_t current_prec = parse_peek_precedence(p);
        if (current_prec <= precedence) break;

        if (peek_is(TT_DEREF)) {
            consume(TT_DEREF, sMSG("Expected '.' for member access"));
            Token *right = consume(TT_IDENTIFIER, sMSG("Expected member name after '.'"));
            left = Ast_member_access_new(peek(), left, right->value);

            if (peek_eq_chr('(')) {
                // If '(' follows, it's a method call
                left = parse_post_call(p, left);
            }
        }
        else if (peek_is(TT_OP)) {
            Token *op = consume(TT_OP, sMSG("Expected a binary operator."));
            Ast *right = parse_expression(p, current_prec);
            if(right != nullptr) {
                left = Ast_bop_new(op, op->value, left, right);
            }
        } else {
            parse_error(p, peek(), sMSG("Expected an operator."));
            break;
        }
    }

    return left;
}



/// @brief
/// @example
///     10 + x
/// @param p
/// @return
Ast *parse_expression_part(ParseContext *p) {
    pctx_trace(peek()->value);

    switch(peek()->type) {
        case TT_KEYWORD:
            return parse_keyword_statement(p, true);
        case TT_TAG: case TT_DOUBLE: case TT_INT: case TT_STRING:
            return parse_literal(p);
        // case TT_DEREF:
        //     return parse_dereference(p);
        case TT_IDENTIFIER:
            return parse_identifier(p);
        case TT_TYPE:
            return parse_object_literal(p);
        case TT_DISCARD:
            return nullptr;
        case TT_BRA_OPEN:
            return parse_bracketed(p);
        case TT_OP:
            return parse_unary(p);
        default:
            parse_error(p, peek(), sMSG("Unknown token type in an expression."));
            return nullptr;
    }
}


/// @brief
/// @example
///     const result = for(x <- range(10)) x * 20
///
/// @param p
/// @param in_expr
/// @return
Ast *parse_keyword_statement(ParseContext *p, bool in_expr) {
    pctx_trace(peek()->value);

    if(!peek_is(TT_KEYWORD)) {
        parse_error(p, peek(), sMSG("Expected a keyword."));
        return nullptr;
    }

    #define perror_no_expr() if(in_expr)\
        parse_error(p, peek(), sMSG("Keyword not allowed in an expression."))

    if(peek_eq(s("use"))) {
        perror_no_expr();
        return parse_use(p);
    } else if(peek_eq(s("trait"))) {
        perror_no_expr();
        return parse_trait(p);
    } else if(peek_eq(s("struct"))) {
        perror_no_expr();
        return parse_struct(p);
    } else if(peek_eq(s("const"))) {
        perror_no_expr();
        return parse_const(p);
    } else if(peek_eq(s("fn"))) {
        return parse_function(p, false);
    } else if(peek_eq(s("for"))) {
        return parse_for(p);
    } else if(peek_eq(s("if"))) {
        return parse_if(p);
    } else if(peek_eq(s("let"))) {
        return parse_let(p);
    } else if(peek_eq(s("mut"))) {
        return parse_mutation(p);
    } else if(peek_eq(s("match"))) {
        return parse_match(p);
    } else if(peek_eq(s("yield"))) {
        return parse_yield(p);
    } else if(peek_eq(s("return"))) {
        return parse_return(p);
    } else if(peek_eq(s("loop"))) {
        return parse_loop(p);
    } else if(peek_eq(s("mod"))) {
        parse_not_implemented();
        // return parse_m(p);
    } else if(peek_eq(s("macro"))) {
        return parse_macro_def(p);
    } else {
        parse_error(p, peek(), sMSG("Unknown keyword."));
        return nullptr;
    }
}

/// @brief Parses a block of statements,
//      typically following keywords like `if`, `for`, etc.
/// @example
///     fn main() :=
///         log("Condition met")
///         execute()
/// @param p The parsing context.
/// @return An AST node representing the block.
Ast *parse_block(ParseContext *p) {
    pctx_trace(peek()->value);

    // Expecting an indentation to start the block
    consume_specific(TT_INDENT, s("INDENT"), sMSG("Expected an indentation to start the block."));

    size_t count = 0;
    Ast **statements = Arena_alloc(ARRAY_SIZE_MEDIUM * sizeof(Ast *));
    Ast *result = nullptr;

    while(data_remain() && !peek_is(TT_DEDENT)) {
        result = parse_statement(p);
        if(result != nullptr) {
            statements[count++] = result;
        }
        p->parser.depth = 0;
        require_medium_array(count);
    }

    // Consume the dedent token
    consume_specific(TT_DEDENT, s("DEDENT"), sMSG("Expected a dedent to end the block."));

    return Ast_block_new(peek(), statements, count, false);
}


/// @brief
/// @example
///
/// @param p
/// @return
Ast *parse_identifier(ParseContext *p) {
    pctx_trace(peek()->value);

    Token *t = consume_expected(TT_IDENTIFIER);
    Ast *id = Ast_identifier_new(t, t->value);

    if(peek_eq_chr('(')) {
        return parse_post_call(p, id);
    } else {
        return id;
    }
}


/// @brief
/// @note the `loop if` syntax is equivalent to `while()`
/// @note the `loop ... in` syntax is a loop over streams
/// @example
///     loop if len(vec) > 0
///         log(pop(vec))
///
///     loop
///         x <- xs
///         y = x * 2
///     in match y
///         if 10 -> log("Yes")
///         else  -> log("No")
/// @param p
/// @return
Ast *parse_loop(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("loop"), sMSG("Expected `loop`."));

    if(peek_eq(s("fn"))) return parse_function(p, true);

    if(peek_eq(s("if"))) {
        consume_expected(TT_KEYWORD);
        return Ast_loop_if_new(peek(), parse_expression(p, 0), parse_block(p));
    } else {
        size_t out_num_bindings = 0;
        Ast **binds = parse_bindings(p, &out_num_bindings);


        /// @todo think about collapsing `in` with one-liners
        /// ....    idea: move `in`-checking to parse_block, which can detect bracketing
        consume_specific(TT_KEYWORD, s("in"), sMSG("`in` missing: loop should have `in` scope."));
        consume_expected(TT_END);

        Ast *body = parse_block(p);

        return Ast_loop_new(peek(), binds, out_num_bindings, body);
    }
}
/// @brief
/// @note the `for` comprehension is a collecting operation across a stream
/// @note it can be a single-line with parentheses, or per in-block
/// @example
///     for(x <- xs, y <- ys, z = x * y) z + x + y
///
///     for
///        x <- xs
///        y <- ys
///     in
///         x * y
///
/// @param p
/// @return
Ast *parse_for(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("for"), sMSG("Keyword `for` expected."));

    size_t out_num_bindings = 0;
    Ast **binds = parse_bindings(p, &out_num_bindings);
    Ast *body = peek_is(TT_INDENT) ? parse_block(p) : parse_expression(p, 0);
    return Ast_loop_new(peek(), binds, out_num_bindings, body);
}

/// @brief
/// @example
///     if something
///         log(something)
///     else
////        log("nothing")
///
///     if something == 1
///         log(something)
///     else if something == 2
///         log("nothing")
///
///     const result = if something then 1 else 2
/// @param p
/// @return
/// @todo -- support for else if
Ast *parse_if(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("if"), sMSG("Keyword `for` expected."));

    Ast *cond = parse_expression(p, 0);
    Ast *body = peek_is(TT_INDENT) ? parse_block(p) : parse_expression(p, 0);
    Ast *elif = nullptr;

    if(peek_eq(s("else"))) {
        consume_specific(TT_KEYWORD, s("else"), sMSG("Keyword `else` expected"));
        elif = peek_is(TT_INDENT) ? parse_block(p) : parse_expression(p, 0);
    }

    return Ast_if_new(peek(), cond, body, elif);
}


/// @brief
/// @note The main kind of block, to be used in binding variables
/// @note Can be abbreviated to a one-liner with parentheses after `let`
/// @note All terms are immutable by default, but `let mut` changes this
/// @example
///     let
///         x = 10
///         mut y = 5
///     in
///         log(x)
///
///     let( x = 10 ) in log(x)
///
///     let mut
///         y = 5
///     in
///         log(take(10, for(ever) mut! y += 1))
///         log(y)
///
/// @param p
/// @return
Ast *parse_let(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("let"), sMSG("Keyword `let` expected."));

    size_t out_num_bindings = 0;
    Ast **binds = parse_bindings(p, &out_num_bindings);

    /// @todo -- consider whether this should lie with parse_block()
    consume_specific(TT_KEYWORD, s("in"), sMSG("Keyword `in` expected."));
    consume_expected(TT_END);

    Ast *body = peek_is(TT_INDENT) ? parse_block(p) : parse_expression(p, 0);
    return Ast_let_new(peek(), binds, out_num_bindings, body);
}


/// @brief
/// @note Defines global constants, therefore never has an `in` block
/// @example
///     const my_constant = 10
/// @param p
/// @return
Ast *parse_const(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("const"), sMSG("Keyword `for` expected."));

    if(peek_eq(s("fn"))) {
        /// @todo placeholder for `const fn` impl
        return parse_function(p, false);
    }

    Token *name = consume(TT_IDENTIFIER, sMSG("Expected an identifier for const name."));
    Token *op = consume(TT_ASSIGN, sMSG("Expected `=` or `:=` to define consts."));
    if(FixStr_chr_at(op->value, 1) == ':')  {
        consume(TT_INDENT, sMSG("Expected an indent after `:=`"));
    }
    Ast *expr = parse_expression(p, 0);
    if(FixStr_chr_at(op->value, 1) == ':')  {
        consume(TT_DEDENT, sMSG("Expected a dedent after `:=` block."));
    }

    return Ast_const_new(peek(), name->value, expr);
}


/// @brief
/// @note Defines functions, use `:=` for a block def
/// @example
///     fn main() = log("Hello World")
///
///     fn main(args) :=
///         log("This is a block: " ++ args[0])
///         10 // last line is return value
/// @param p
/// @return
Ast *parse_function(ParseContext *p,bool is_loop) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("fn"), sMSG("Keyword `for` expected."));

    if(peek_eq_chr('(')) {
        return parse_post_anon(p);
    }

    FixStr name = consume(TT_IDENTIFIER, sMSG("Expected function name"))->value;

    consume_specific(TT_BRA_OPEN, s("("), sMSG("Expected an opening parenthesis."));
    struct FnParam *params = Arena_alloc(sizeof(struct FnParam *) * ARRAY_SIZE_SMALL);
    size_t count = 0;
    while(data_remain()) {
        if(peek_eq_chr(')')) break;
        if(peek_eq_chr(',')) consume_expected(TT_SEP);

        pctx_trace(peek()->value);
        params[count++].name = consume(TT_IDENTIFIER, sMSG("Expected parameter name"))->value;
    }
    consume_specific(TT_BRA_CLOSE, s(")"), sMSG("Expected a closing parenthesis."));

    Token *op = consume(TT_ASSIGN, sMSG("Expected `=` or `:=`"));

    return Ast_fn_def_new(
        peek(), name, params, count,
        FixStr_eq_chr(op->value, '=') ? parse_expression(p, 0) : parse_block(p),
        false, is_loop
    );
}


/// @brief Parses a generic type with constraints.
//// @todo not yet implemented
/// @example
///     fn foo<T: Trait1, Trait2>(param: T)
/// @param p The parsing context.
/// @return An AST node representing the generic type.
Ast *parse_generic_type(ParseContext *p) {
    pctx_trace(peek()->value);
    parse_not_implemented();

    // // Parse base type name
    // Token *base_type_token = consume(TT_TYPE, sMSG("Expected base type name for generic type."));

    // Ast *generic_type = Ast_generic_type_new(peek(), base_type_token->value);

    // // Check for type parameters enclosed in '<' and '>'
    // if (peek_eq_chr('<')) {
    //     consume_specific(TT_BRA_OPEN, s("<"), sMSG("Expected '<' to start generic type parameters."));

    //     while (data_remain() && !peek_eq_chr('>')) {
    //         // Parse each type parameter
    //         Ast *type_param = parse_type_annotation(p);
    //         generic_type->generic_type.type_params[generic_type->generic_type.num_type_params++] = type_param;

    //         // Handle comma-separated type parameters
    //         if (peek_eq_chr(',')) {
    //             consume_specific(TT_SEP, s(","), sMSG("Expected ',' between type parameters."));
    //         } else {
    //             break;
    //         }

    //         require_small_array(generic_type->generic_type.type_params);
    //     }

    //     consume_specific(TT_BRA_OPEN, s(">"), sMSG("Expected '>' to end generic type parameters."));
    // }

    // // Check for type constraints after ':'
    // if (peek_eq_chr(':')) {
    //     consume_specific(TT_OP, s(":"), sMSG("Expected ':' to start type constraints."));

    //     while (data_remain() && !peek_eq_chr(';') && !peek_eq_chr(')')) {
    //         // Parse each constraint
    //         FixStr constraint_name = consume(TT_TYPE, sMSG("Expected trait name for type constraint.")).->value;
    //         generic_type->generic_type.constraints[generic_type->generic_type.num_constraints++] = Ast_type_constraint_new(peek(), constraint_name);

    //         // Handle comma-separated constraints
    //         if (peek_eq_chr(',')) {
    //             consume_specific(TT_SEP, s(","), sMSG("Expected ',' between type constraints."));
    //         } else {
    //             break;
    //         }
    //     }
    // }

    // return generic_type;
}


/// @brief Parses a macro def.
/// @example
///     macro fn create_struct($name, $$fields) :=
///             emit `struct $name :=`
///                 emit* `$$fields`
/// @param p The parsing context.
/// @return An AST node representing the macro def.
Ast *parse_macro_def(ParseContext *p) {
    pctx_trace(peek()->value);
    parse_not_implemented();

    // // Consume 'macro_rules!' keyword
    // consume_specific(TT_KEYWORD, s("macro"), sMSG("Expected 'macro' keyword."));
    // consume_specific(TT_KEYWORD, s("fn"), sMSG("Expected 'fn' keyword."));

    // // Parse macro name
    // Token *macro_name = consume(TT_IDENTIFIER, sMSG("Expected macro name."));

    // // Expecting ':=' to start macro def
    // consume_specific(TT_ASSIGN, s(":="), sMSG("Expected ':=' to start macro def."));

    // // Parse macro rules as a block
    // Ast *macro_body = parse_block(p);

    // // Create and return macro def AST node
    // return Ast_macro_def_new(peek(), macro_name->value, macro_body->macro_def.rules, macro_body->macro_def.num_rules);
}

/// @brief Parses a documentation comment.
/// @example
///     /// This function adds two numbers.
///     fn add(a: Int, b: Int): Int :=
///         a + b
/// @param p The parsing context.
/// @return An AST node representing the documentation comment.
Ast *parse_doc_comment(ParseContext *p) {
    pctx_trace(peek()->value);

    // Consume documentation comment token
    Token *comment_token = consume(TT_DOC_COMMENT, sMSG("Expected documentation comment."));

    // Create and return documentation comment AST node
    return Ast_doc_comment_new(peek(), comment_token->value);
}


/// @brief Parses anonymous functions or lambda expressions.
/// @example
///     fn(x, y) -> x + y
/// @param p The parsing context.
/// @return An AST node representing the anonymous function.
Ast *parse_post_anon(ParseContext *p) {
    // Parse parameters
    consume_specific(TT_BRA_OPEN, s("("), sMSG("Expected '(' after 'fn' for anonymous function parameters."));

    struct FnParam *params = Arena_alloc(sizeof(struct FnParam) * ARRAY_SIZE_SMALL);
    size_t count = 0;

    while(data_remain() && !peek_eq_chr(')')) {
        // require_small_array(count);
        if(count >= ARRAY_SIZE_SMALL) {
            parse_error(p, peek(),
                sMSG("Too many parameters for anonymous function."
                    "The maximum number of parameters is 16."
                    "Consider using a struct or collection to group parameters."
                )
            );
            break;
        }

        if(peek_eq_chr(',')) {
            consume_specific(TT_SEP, s(","), sMSG("Expected a comma between parameters."));
        }

        pctx_trace(peek()->value);
        params[count].name = consume(TT_IDENTIFIER, sMSG("Expected parameter name"))->value;
        params[count].type = nullptr;
        params[count].default_value = nullptr;  /// @todo

        count++;
    }

    consume_specific(TT_BRA_CLOSE, s(")"), sMSG("Expected ')' after anonymous function parameters."));
    consume(TT_ASSIGN, sMSG("Expected '->' after anonymous function parameters."));

    Ast *body = peek_is(TT_INDENT) ? parse_block(p) : parse_expression(p, 0);

    return Ast_fn_def_new(peek(), s(""), params, count, body, false, false);
}


/// @brief Parses a destructuring assignment.
/// @example
///     let [x, y] = get_coordinates() in x
/// @param p The parsing context.
/// @return An AST node representing the destructuring assignment.
Ast *parse_destructure_assignment(ParseContext *p) {
    pctx_trace(peek()->value);

    // Consume 'let' keyword
    consume_specific(TT_KEYWORD, s("let"), sMSG("Expected 'let' keyword for destructuring assignment."));

    /// @todo handle indent

    // Expecting '(' to start variable list
    consume_specific(TT_BRA_OPEN, s("["), sMSG("Expected '(' to start variable list in destructuring assignment."));

    // Parse variables
    Ast **variables = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(Ast *));
    size_t num_variables = 0;

    while (data_remain() && !peek_eq_chr(']')) {
        if (peek_eq_chr(',')) {
            consume_specific(TT_SEP, s(","), sMSG("Expected ',' between variables in destructuring assignment."));
        }

        // Parse variable (identifier)
        Token *var_token = consume(TT_IDENTIFIER, sMSG("Expected variable name in destructuring assignment."));
        Ast *var_node = Ast_identifier_new(var_token, var_token->value);
        variables[num_variables++] = var_node;
        require_small_array(num_variables);
    }

    // Consume ')' to end variable list
    consume_specific(TT_BRA_CLOSE, s("]"), sMSG("Expected ')' to end variable list in destructuring assignment."));

    // Consume '=' operator
    consume_specific(TT_ASSIGN, s("="), sMSG("Expected '=' in destructuring assignment."));

    // Parse the expression being destructured
    Ast *expr = parse_expression(p, 0);

    // Create and return destructure assignment AST node
    return Ast_destructure_assign_new(peek(), variables, num_variables, expr);
}


/// @brief Parses an asynchronous function def.
/// @example
///     async fn fetch_data(url: FixString) :=
///         // asynchronous operations
/// @param p The parsing context.
/// @return An AST node representing the async function def.
Ast *parse_async_function(ParseContext *p) {
    pctx_trace(peek()->value);
    parse_not_implemented();
}

/// @brief
/// @note parses groups of bindings (assignment: =  :=  streaming: <-)
/// @example
///     (x <- range(2, 100, 2), y <- repeat(1), z = x * y)
///
///         x <- range(2, 100, 2)
///         y <- repeat(1)
///         z = x * y
///
/// @param p
/// @return
Ast **parse_bindings(ParseContext *p, size_t *out_num_bindings) {
    pctx_trace(peek()->value);

    bool is_bracketed = peek_eq_chr('(');

    if(is_bracketed) {
        consume_specific(TT_BRA_OPEN, s("("), sMSG("A bracket expected."));
    } else {
        consume(TT_END, sMSG("Bindings blocks follow keywords on their own line."));
        consume(TT_INDENT, sMSG("Bindings blocks should be indented or bracketed."));
    }

    Ast **bindings = Arena_alloc(ARRAY_SIZE_SMALL* sizeof(Ast *));
    *out_num_bindings = 0;
    while(data_remain()) {
        if(peek_eq_chr(')') || peek_is(TT_DEDENT)) break;

        FixStr id = consume(TT_IDENTIFIER, sMSG("Expected an identifier"))->value;
        FixStr op = consume(TT_ASSIGN, sMSG("Expected an assignment operator"))->value;
        Ast *expr = parse_expression(p, 0);
        if(is_bracketed && *out_num_bindings >= 1) {
            consume_specific(TT_SEP, s(","), sMSG("Bracketed bindings are comma separated"));
        } else if(!is_bracketed) {
            consume(TT_END, s("Block bindings should each end on a new line"));
        }
        bindings[*out_num_bindings] = Ast_binding_new(peek(), id, expr, op);
        *out_num_bindings += 1;

        // require_small_array(*out_num_bindings);

        if(*out_num_bindings >= ARRAY_SIZE_SMALL) {
            parse_error(p, peek(),
                sMSG("Too many bindings. The maximum number of bindings is 16."
                    "You can nest binding scopes."
                    "Or, consider using a struct or collection to group bindings."
                )
            );
            break;
        }
    }

    if(is_bracketed) {
        consume_specific(TT_BRA_CLOSE, s(")"), sMSG("Expected a closing bracket."));
    } else {
        consume(TT_DEDENT, sMSG("Expected a dedent."));
    }
    return bindings;
}

Ast *parse_bracketed(ParseContext *p) {
    pctx_trace(peek()->value);

    if(peek_eq_chr('(')) {
        consume_expected(TT_BRA_OPEN);
        Ast *expr = parse_expression(p, 0);
        consume_expected(TT_BRA_CLOSE);
        return expr;
    } else if(peek_eq_chr('[')) {
        return parse_vec(p);
    } else if(peek_eq_chr('{')) {
        return parse_dict(p);
    } else {
        parse_error(p, peek(), sMSG("Unknown bracket type"));
        return nullptr;
    }
}

/// @brief Parses a set or dictionary literal.
/// @note dictionaries cannot have sets as keys, so any `:` is dispositive
/// @example
///     {1, 2, 3} -> set literal
///     {1: 2, 3: 4} -> dictionary literal
/// @param p The parse context.
/// @return An AST node representing the parsed set or dictionary.
Ast *parse_set_or_dict(ParseContext *p) {
    pctx_trace(peek()->value);

    // Consume the opening `{`
    consume_specific(TT_BRA_OPEN, s("{"), sMSG("Expected `{` to start set or dictionary."));

    // Prepare containers for keys and values
    Ast **keys = Arena_alloc(sizeof(Ast *) * ARRAY_SIZE_SMALL);
    Ast **values = Arena_alloc(sizeof(Ast *) * ARRAY_SIZE_SMALL);
    size_t count_keys = 0, count_values = 0;

    // Parse data
    bool is_dict = false;
    while (!peek_eq(s("}"))) {
        Ast *key = parse_expression(p, 0);

        if (peek_eq(s(":"))) {
            // Colon detected, it's a dictionary
            is_dict = true;
            consume_specific(TT_SEP, s(":"), sMSG("Expected `:` in dictionary literal."));
            Ast *value = parse_expression(p, 0);
            keys[count_keys++] = key;
            values[count_values++] = value;
        } else if (is_dict) {
            // Error: mixing dictionary and set syntax
            parse_error(p, peek(), sMSG("Mixed set and dictionary syntax is not allowed."));
        } else {
            // No colon, assume it's a set element
            keys[count_keys++] = key;
        }

        // Handle commas or the closing `}`
        if (peek_eq(s(","))) {
            consume_specific(TT_SEP, s(","), sMSG("Expected `,` or `}`."));
        } else if (!peek_eq(s("}"))) {
            parse_error(p, peek(), sMSG("Expected `,` or `}` in set or dictionary literal."));
        }
    }

    // Consume the closing `}`
    consume_specific(TT_BRA_CLOSE, s("}"), sMSG("Expected `}` to close set or dictionary."));

    // Construct the appropriate AST node
    if (is_dict) {

        /// @todo
        struct AstDictEntry *kv_pairs =  Arena_alloc(sizeof(*kv_pairs) * count_keys);
        for(size_t i = 0; i < count_keys; i++) {
            kv_pairs[i].key = keys[i];
            kv_pairs[i].value = values[i];
        }

        return Ast_dict_new(peek(), kv_pairs, count_keys);
    } else {
        parse_not_implemented();
        return nullptr;
        // return Ast_set_new(peek(), keys, count_keys);
    }
}


/// @brief Parses a type annotation.
/// @todo Implement parsing for complex type annotations as per language specifications.
/// @example
///     const dv : Vec(Int; 32) = range(32)
///     const dd : Dict(Int, Int; 32) = for(v1 <- dv, v2 <- dv) {v1: v2}
///     const stt : Vec(Char; ?) = "Hello World" // static inference
///     const ref st1 : Vec(Char; dyn) = "Hello World"
///     const ref st2 : Vec? = dyn "Hello World"
///     const ref st3 = dyn "Hello World" // infers FixString(Char; dyn)
///     const dc : Dict(?, ?; ?) = {ch(c): "Hello"}
///     const dcc : Dict? = dc // infer all
/// @param p The parsing context.
/// @return An AST node representing the type annotation.
Ast *parse_type_annotation(ParseContext *p) {
    pctx_trace(peek()->value);

    // Initialize a new AST node for type annotation
    Ast *type_ann = Ast_type_annotation_new(peek());

    // Check for qualifiers like 'ref' or 'mut'
    if (peek_eq(s("ref")) || peek_eq(s("mut"))) {
        // Consume the qualifier
        Token *qual_token = consume_expected(TT_KEYWORD);
        type_ann->type_annotation.qualifier = qual_token->value;
    } else {
        // No qualifier present
        type_ann->type_annotation.qualifier = FixStr_empty();
    }

    // Parse the base type name
    Token *base_type_token = consume(TT_TYPE, sMSG("Expected a base type identifier."));
    type_ann->type_annotation.base_type = base_type_token->value;

    // Initialize type parameters
    // type_ann->type_annotation.type_params = nullptr;
    type_ann->type_annotation.num_type_params = 0;

    // Check for generic type parameters enclosed in '('
    if (peek_eq_chr('(')) {
        consume_specific(TT_BRA_OPEN, s("("), sMSG("Expected '(' to start type parameters."));

        // Initialize type parameters array
        size_t params_capacity = 0;
        // type_ann->type_annotation.type_params = Arena_alloc(params_capacity * sizeof(Ast *));
        type_ann->type_annotation.num_type_params = 0;

        while (data_remain() && !peek_eq_chr(')')) {
            // Parse each type parameter (recursive call)
            Ast *param = parse_type_annotation(p);
            if (param != nullptr) {
                type_ann->type_annotation.type_params[type_ann->type_annotation.num_type_params++] = param;
            }

            // Handle comma-separated type parameters
            if (peek_eq_chr(',')) {
                consume_specific(TT_SEP, s(","), sMSG("Expected ',' between type parameters."));
            } else {
                break;
            }

            // Ensure capacity
            require_small_array(params_capacity++);
        }

        consume_specific(TT_BRA_CLOSE, s(")"), sMSG("Expected ')' to end type parameters."));
    }

    // Check for size constraint after ';'
    if (peek_eq_chr(';')) {
        consume_specific(TT_SEP, s(";"), sMSG("Expected ';' before size constraint."));

        // Size constraint can be an identifier (e.g., 'dyn') or a number (e.g., '32') or '?'
        if (peek_is(TT_IDENTIFIER)) {
            Token *size_token = consume(TT_IDENTIFIER, sMSG("Expected size constraint identifier or value."));
            type_ann->type_annotation.size_constraint = size_token->value;
        }
        else if (peek_eq_chr('?')) {
            // Handle '?' as a size constraint
            consume_expected(TT_OP);
            type_ann->type_annotation.size_constraint = s("?");
        }
        else {
            parse_error(p, peek(), sMSG("Invalid size constraint in type annotation."));
            type_ann->type_annotation.size_constraint = s("?");
        }
    } else {
        type_ann->type_annotation.size_constraint = FixStr_empty();
    }

    // Check for optional '?', indicating optional type
    if (peek_eq_chr('?')) {
        consume_specific(TT_OP, s("?"), sMSG("Expected '?' indicating an optional type."));
        /// @todo: probably make this a specific flat
        ///     here we use nullptr as an indicative value
        type_ann->type_annotation.type_params[0] = nullptr; /// @todo [] check
        type_ann->type_annotation.num_type_params = 1;
    }

    return type_ann;
}



/// @brief
/// @todo -- is this required? should DEREF always be part of another rule?
/// @note to support universal function calling syntax (monomorphic)
/// @note and field access on structs
/// @example
///     vec.pop()       // becomes typeof(vec)##_pop(vec)
///     record.field
///
///     /// explicit runtime polymorphic resolution
///     for(x <- [letters, dictionary]) of(x).pop()
///
///     /// becomes
///     for(x <- [letters, dictionary]) call(methodof(typeof(x))[#pop], [x])
/// @param p s
/// @return
// Ast *parse_dereference(ParseContext *p) {
//     pctx_trace(peek()->value);

//     consume_expected(TT_DEREF);



//     Ast *target = parse_expression(p, 0);
//     FixStr attr = s("debug_placeholder_attr");
//     // FixStr attr = consume(TT_IDENTIFIER, sMSG("Expected identifier"))->value;

//     if(peek_eq_chr('(')) {
//         return Ast_method_call_new(peek(), target, attr, parse_post_call(p, nullptr));
//     } else {
//         return Ast_member_access_new(peek(), target, attr);
//     }
// }

Ast *parse_unary(ParseContext *p) {
    pctx_trace(peek()->value);

    if(!(peek_eq_chr('#') || peek_eq_chr('!') || peek_eq_chr('-'))) {
        parse_error(p, peek(), sMSG("Unexpected operator."));
        return nullptr;
    }

    uint8_t prec = parse_peek_precedence(p);
    Token *t = consume_expected(TT_OP);
    return Ast_uop_new(peek(), t->value, parse_expression(p, prec));
}


/// @brief
/// @note vectors are fixed-sized (16 data)
///         and statically allocated to local arena by default
/// @note if they exceed 16, they will be placed on heap
/// @note use `dyn` to place on to GC'd heap directly
///         marking the variable as `ref` to note it now does not copy when reassigned
///         and `ref` variables are reference counted
///
/// @example
///     let( xs = [2, 4, 6] ) in log(xs)
///     let( ref xs = dyn [2, 4, 6] ) in log(xs.append(10))
/// @param p
/// @return
Ast *parse_vec(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_BRA_OPEN, s("["), sMSG("Expected a `[` bracket for vectors."));

    size_t count = 0;
    Ast **items = Arena_alloc(ARRAY_SIZE_SMALL* sizeof(Ast *));

    while(data_remain() && !peek_eq_chr(']')) {
        Ast *item = parse_expression(p, 0);
        items[count++] = item;

        if(peek_eq_chr(',')) {
            consume_specific(TT_SEP, s(","), sMSG("Expected a comma here"));
        }

        require_small_array(count);
        if(count >= ARRAY_SIZE_SMALL) {
            parse_error(p, peek(),
                sMSG("Too many items in vector literal."
                    "The maximum number of items in a literal is 16."
                    "You can create a vector and use `vec.append()` to add more items."
                    "Or, if you want to import data use load_data_file() and a datafile"
                )
            );
            break;
        }
        /// @todo -- consider reallocation or limiting literals
    }

    consume_specific(TT_BRA_CLOSE, s("]"), sMSG("Expected a closing `]` here."));

    return Ast_vec_new(peek(), items, count);

}

/// @brief
/// @note dicts are fixed-sized (16 data)
///         and statically allocated to local arena by default
/// @note if they exceed 16, they will be placed on heap
/// @note use `dyn` to place on to GC'd heap directly
///         marking the variable as `ref` to note it now does not copy when reassigned
///         and `ref` variables are reference counted
///
/// @example
///     let( xs = {"a": "10", "b": "20"} ) in log(xs["a"])
///     let( ref xs = dyn {"a": "10", "b": "20"} ) in log(xs - "a")
/// @param p
/// @return
Ast *parse_dict(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_BRA_OPEN, s("{"), sMSG("Expected a `{` bracket for dictionaries."));

    size_t count = 0;
    struct AstDictEntry *items =
        /// @todo: why is this broken, should we move all the subfields of Ast out?
        ///.... or perhaps move the property parsing into Ast_* ?
        Arena_alloc(ARRAY_SIZE_SMALL* sizeof(struct AstDictEntry));

    while(data_remain() && !peek_eq_chr('}')) {
        /// @todo: this expects dict keys to be ...?
        ///     ... but we should generalize this to keys involving ..?
        items[count].key = parse_expression(p, 0);

        consume_specific(TT_SEP, s(":"), sMSG("Expected a colon."));

        items[count].value = parse_expression(p, 0);

        if(peek_eq_chr(',')) {
            consume_specific(TT_SEP, s(","), sMSG("Expected a comma here"));
        }

        if(count >= ARRAY_SIZE_SMALL) {
            parse_error(p, peek(),
                sMSG("Too many items in dictionary literal."
                    "The maximum number of items in a literal is 16."
                    "You can create a dictionary and use `dict.append()` to add more items."
                    "Or, if you want to import data use load_data_file() and a datafile"
                )
            );
            break;
        }
        require_small_array(count);
    }

    consume_specific(TT_BRA_CLOSE, s("}"), sMSG("Expected a closing `}` here."));
    return Ast_dict_new(peek(), items, count);
}


/// @brief
/// @example
/// @param p
/// @return
Ast *parse_literal(ParseContext *p) {
    pctx_trace(peek()->value);

    if(peek_is(TT_TAG)) {
        return Ast_tag_new(peek(), consume_expected(TT_TAG)->value);
    }
    else if(peek_is(TT_DOUBLE)) {
        return Ast_double_new(peek(), FixStr_to_double(consume_expected(TT_DOUBLE)->value));
    }
    else if(peek_is(TT_INT)) {
        return Ast_int_new(peek(), FixStr_to_int(consume_expected(TT_INT)->value));
    }
    else if(peek_is(TT_STRING)) {
        return Ast_string_new(peek(), FixStr_trim_delim(consume_expected(TT_STRING)->value));
    }
    else {
        parse_error(p, peek(), sMSG("No such kind of literal."));
        return nullptr;
    }
}



/// @brief
/// @todo -- consider strategy for detecting unmatched brackets
/// @param p
/// @param callee
/// @return
Ast *parse_post_call(ParseContext *p, Ast *callee) {
    pctx_trace(peek()->value);

    consume_specific(TT_BRA_OPEN, s("("), sMSG("Expected `(` for function call."));

    Ast **args = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(Ast *));
    size_t count = 0;

    while (data_remain() && !peek_eq_chr(')')) {
        if(count >= 1) {
            consume_specific(TT_SEP, s(","), sMSG("Expected `,` between function arguments. Perhaps missing a closing bracket?"));
        }
        args[count++] = parse_expression(p, 0);
        require_not_null(args[count-1]);
        require_small_array(count);
    }

    consume_specific(TT_BRA_CLOSE, s(")"), sMSG("Expected `)` for function call."));

    if (callee->type == AST_MEMBER_ACCESS) {
        // It's a method call
        return Ast_method_call_new(peek(), callee->member_access.target, callee->member_access.property, Ast_fn_call_new(peek(), nullptr, args, count));
    } else {
        // It's a regular function call
        return Ast_fn_call_new(peek(), callee, args, count);
    }
}




/// @brief Parses struct literals (object initialization).
/// @example
///     Person { .name = "Alice", .age = 30 }
/// @param p The parsing context.
/// @return An AST node representing the struct literal.
Ast *parse_object_literal(ParseContext *p) {
    pctx_trace(peek()->value);

    // Parse struct type name
    FixStr type_name = consume(TT_TYPE, sMSG("FixStruct literals must begin with a type name."))->value;

    // Expecting opening brace
    consume_specific(TT_BRA_OPEN, s("{"), sMSG("Expected '{' to start struct literal."));

    size_t count = 0;
    struct AstObjectField *fields = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(struct AstObjectField));

    while(data_remain() && !peek_eq_chr('}')) {
        /// @todo consider whether we want fields to start with `.`, may need to modify lexer
        // consume_specific(TT_DEREF, s("."), sMSG("Expected '.' before struct field name."));
        FixStr field_name = consume(TT_IDENTIFIER, sMSG("Expected field name in struct literal."))->value;

        consume_specific(TT_ASSIGN, s("="), sMSG("Expected '=' after struct field name."));
        Ast *field_value = parse_expression(p, 0);
        fields[count].name = field_name;
        fields[count].value = field_value;
        count++;
        require_small_array(count);
        if(count >= ARRAY_SIZE_SMALL) {
            parse_error(p, peek(),
                sMSG("Too many fields in object literal."
                    "The maximum number of fields in a literal is 16."
                    "You can create an object and set fields one-by-one."
                    "Or, if you want to import data use load_data_file() with hydrate()"
                )
            );
            break;
        }

        // Optional comma
        if(peek_eq_chr(',')) {
            consume_specific(TT_SEP, s(","), sMSG("Expected ',' between struct fields."));
        }
    }

    consume_specific(TT_BRA_CLOSE, s("}"), sMSG("Expected '}' to close struct literal."));

    return Ast_object_literal_new(peek(), type_name, fields, count);
}


/// @brief Parses `match` expressions for pattern matching.
/// @example
///     match value
///         if 1 -> "One"
///         else -> "Other"
/// @param p The parsing context.
/// @return An AST node representing the match expression.
Ast *parse_match(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("match"), sMSG("Expected 'match' keyword."));

    // Parse the expression to match against
    Ast *matched_expr = parse_expression(p, 0);
    consume_expected(TT_END);
    consume_expected(TT_INDENT);

    // Expecting the start of match arms
    struct MatchCase *arms = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(struct MatchCase));
    size_t arm_count = 0;

    while(data_remain() && !peek_is(TT_DEDENT)) {
        require_small_array(arm_count);

        // Parse 'if' or 'else' keyword
        if(peek_eq(s("if"))) {
            consume_specific(TT_KEYWORD, s("if"), sMSG("Expected 'if' in match arm."));
            arms[arm_count].condition = parse_expression(p, 0);
            consume_specific(TT_ASSIGN, s("->"), sMSG("Expected '->' after match condition."));
            arms[arm_count].expression = parse_expression(p, 0);
            arm_count++;
            consume_expected(TT_END);
        } else if(peek_eq(s("else"))) {
            consume_specific(TT_KEYWORD, s("else"), sMSG("Expected 'else' in match arm."));
            consume_specific(TT_ASSIGN, s("->"), sMSG("Expected '->' after 'else' in match arm."));
            arms[arm_count].condition =  nullptr;
            arms[arm_count].expression = parse_expression(p, 0);
            arm_count++;
            consume_expected(TT_END);
        } else {
            parse_error(p, peek(), sMSG("Unexpected token in match expression."));
            return nullptr;
        }

        /// @todo -- something about an optional comma if we allow collapsing match
        ///...... eg., match(x) if T -> 1, if F -> 2
        // if(peek_eq_chr(',')) {
        //     consume_specific(TT_SEP, s(","), sMSG("Expected ',' between match arms."));
        // }
    }

    consume_expected(TT_DEDENT);
    return Ast_match_new(peek(), matched_expr, arms, arm_count);
}



/// @brief Parses `return` statements for early exits from functions.
/// @example
///     return x + y
/// @param p The parsing context.
/// @return An AST node representing the return statement.
Ast *parse_return(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("return"), sMSG("Expected 'return' keyword."));

    // Optionally parse the return expression
    Ast *ret_expr = nullptr;
    if(!peek_is(TT_END) && !peek_eq_chr('\n')) {
        ret_expr = parse_expression(p, 0);
    }

    return Ast_return_new(peek(), ret_expr);
}


/// @brief Parses `yield` statements used in generator functions.
/// @example
///     yield value
/// @param p The parsing context.
/// @return An AST node representing the yield statement.
Ast *parse_yield(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("yield"), sMSG("Expected 'yield' keyword."));

    // Parse the yield expression
    Ast *yield_expr = parse_expression(p, 0);

    return Ast_yield_new(peek(), yield_expr);
}



/// @brief Parses `ignore` statements indicating that a block or region should be empty.
/// @example
///     ignore
/// @param p The parsing context.
/// @return An AST node representing the ignore statement.
Ast *parse_ignore(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_expected(TT_IGNORE);

    // Typically, 'ignore' doesn't have an associated expression or block
    return Ast_ignore_new(peek());
}



/// @brief Parses `trait` defs for defining interfaces or behavior contracts.
/// @example
///     trait FixStringy :=
///         fn to_FixString()
/// @param p The parsing context.
/// @return An AST node representing the trait def.
Ast *parse_trait(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("trait"), sMSG("Expected 'trait' keyword."));

    // Parse trait name
    //FixStr trait_name = ... ->value;
    consume(TT_IDENTIFIER, sMSG("Expected trait name."));

    // Expecting ':=' to start trait body
    consume_specific(TT_ASSIGN, s(":="), sMSG("Expected ':=' to start trait body."));



    parse_not_implemented();
    return nullptr;
//
    // return Ast_trait_new(peek(), trait_name, trait_methods, num_methods);

    // return Ast_trait_new(peek(), trait_name, body);
}


/// @brief Parses `struct` defs for defining new data types.
/// @example
///     struct Person :=
///         .name : FixString = "Default"
///         .age : Int = 0
/// @param p The parsing context.
/// @return An AST node representing the struct def.
Ast *parse_struct(ParseContext *p) {
    pctx_trace(peek()->value);

    consume_specific(TT_KEYWORD, s("struct"), sMSG("Expected 'struct' keyword."));

    // Parse struct name
    FixStr struct_name = consume(TT_TYPE, sMSG("Expected struct name."))->value;

    // Expecting ':=' to start struct body
    consume_specific(TT_ASSIGN, s(":="), sMSG("Expected ':=' to start struct body."));
    consume_expected(TT_INDENT);

    // parse struct bindings
    // parse_not_implemented();
    // return nullptr;

    struct FixStructField *fields = Arena_alloc(ARRAY_SIZE_SMALL * sizeof(struct FixStructField));
    size_t num_fields = 0;

    while(data_remain() && !peek_is(TT_DEDENT)) {
        // require_small_array(num_fields);

        if(num_fields >= ARRAY_SIZE_SMALL) {
            parse_error(p, peek(),
                sMSG("Too many fields in struct definition (max 16)."
                    "Group fields into sub-structs using indented struct definitions."
                    "Or consider using a collection type."
                )
            );

            return nullptr;
        }

        // @todo -- consider whether `.` should prefix fields -- if so, modify lexer
        // consume_specific(TT_DEREF, s("."), sMSG("Expected '.' before struct field name."));
        FixStr field_name = consume(TT_IDENTIFIER, sMSG("Expected field name in struct def."))->value;
        Ast *field_type = nullptr;
        Ast *field_value = nullptr;


        if (peek_eq_chr(':')) {
            consume_specific(TT_SEP, s(":"), sMSG("Expected ':' after struct field name."));
            field_type = parse_type_annotation(p);
        }

        if(peek_eq_chr('=')) {
            consume_specific(TT_ASSIGN, s("="), sMSG("Expected '=' after struct field type."));
            field_value = parse_expression(p, 0);
        }

        consume_expected(TT_END);

        fields[num_fields].name = field_name;
        fields[num_fields].type = field_type;
        fields[num_fields].default_value = field_value;
        num_fields++;
    }

    consume_expected(TT_DEDENT);
    return Ast_struct_new(peek(), struct_name, fields, num_fields);
}



/// @brief Parses `use` statements for importing modules or specific items.
/// @example
///     use core.collections.*
///     use core.collections as c
/// @param p The parsing context.
/// @return An AST node representing the use statement.
Ast *parse_use(ParseContext *p) {
    pctx_trace(peek()->value);

    parse_not_implemented();
    return nullptr;

    consume_specific(TT_KEYWORD, s("use"), sMSG("Expected 'use' keyword."));

    // Parse the module path
    FixStr module_path = consume(TT_IDENTIFIER, sMSG("Expected module name."))->value;

    // Handle possible sub-paths (e.g., core.collections)
    while(peek_eq_chr('.')) {
        consume_specific(TT_OP, s("."), sMSG("Expected '.' in module path."));
        FixStr sub_module = consume(TT_IDENTIFIER, sMSG("Expected sub-module name."))->value;
        module_path = FixStr_fmt_new(s("%.*s.%.*s"), fmt(module_path), fmt(sub_module));
    }

    // Optional alias
    FixStr alias = FixStr_empty();
    if(peek_eq(s("as"))) {
        consume_specific(TT_KEYWORD, s("as"), sMSG("Expected 'as' keyword for alias."));
        alias = consume(TT_IDENTIFIER, sMSG("Expected alias name."))->value;
    }

    // Optional wildcard import
    bool wildcard = false;
    if(peek_eq_chr('*')) {
        consume_specific(TT_OP, s("*"), sMSG("Expected '*' for wildcard import."));
        wildcard = true;
    }

    return Ast_use_new(peek(), module_path, alias, wildcard);
}



/// @brief Parses mutation operations, marking variables as mutable.
/// @example
///     mut! x += 1
///     mut* y += 2  // Vectorized mutation
/// @param p The parsing context.
/// @return An AST node representing the mutation operation.
Ast *parse_mutation(ParseContext *p) {
    pctx_trace(peek()->value);

    parse_not_implemented();
    return nullptr;

    consume_specific(TT_KEYWORD, s("mut"), sMSG("Expected 'mut' keyword."));

    // Determine mutation type: single or vectorized
    bool is_vector = false;
    if(peek_eq_chr('!')) {
        consume_specific(TT_OP, s("!"), sMSG("Expected '!' for vectorized mutation."));
        is_vector = true;
    }
    else if(peek_eq_chr('*')) {
        consume_specific(TT_OP, s("*"), sMSG("Expected '*' for vectorized mutation."));
        is_vector = true;
    }

    // Parse the target of mutation (e.g., variable or field)
    Ast *target = parse_expression(p, 0);

    // Parse the mutation operator (e.g., +=, -=)
    Token *op = consume(TT_OP, sMSG("Expected mutation operator (e.g., '+=', '-=')."));

    // Parse the mutation expression
    Ast *value = parse_expression(p, 0);

    return Ast_mutation_new(peek(), target, op->value, value, is_vector);
}


void parse_test_main(void) {
    FixStr source = s(
        "const test = 1234\n\n"
        "fn main() :=\n"
        "    log(test)\n"
        "\n"
    );

    FixStr_println(sMSG("Parse Tests:"));
    lex_source(ctx_parser(), source, s("    "));
    parse(ctx_parser());
    Ast_print(ctx_parser()->parser.data);
    Tracer_print_last(&ctx_parser()->parser.traces);

}

// void source_init(Source *src, FixStr source) {
//     src->lines = FixStr_lines(source, &src->size);

//     require_not_null(src->lines);
//     require_positive(src->size);
// }

// void parse(ParseContext *pctx, FixStr source, FixStr indent) {
//     source_init(&pctx->source, source);

//     require_not_null(pctx->lexer.data);
//     require_positive(pctx->lexer.size);
// }

#pragma endregion

#pragma region TestMain
void parsing_test_main(void) {
    lex_test_main();
    parse_test_main();
}
#pragma endregion

//#endregion
///---------- ---------- ---------- IMPL: NATIVE ---------- ---------- ----------- ///
//#region Implementation_NativesAndBoxes


#pragma region NativeErrorImpl

#ifdef DEBUG_REQUIRE_ASSERTS
    #define require_scope_has(scope, key) assert(FixScope_has(scope, key))
#else
    /// @todo swap out all checks
    #define require_not_null(expr)
#endif

// #define NATIVE_LOG_CALLS
// #define NATIVE_DEBUG_VERBOSE

#ifdef NATIVE_DEBUG_VERBOSE
    #define native_log_debug(...)\
        log_message(LL_DEBUG, __VA_ARGS__)
#else
    #define native_log_debug(...) NOOP
#endif

#ifdef NATIVE_LOG_CALLS
    #define native_log_call(self, args)\
        log_message(LL_INFO, s("Native fn: %.*s(...)"), fmt(self->name));\
        FixArray_debug_print(s("Args"), &args);
    #define native_log_call0(self, one)\
        log_message(LL_INFO, s("Native fn: %.*s()"), fmt(self->name));
    #define native_log_call1(self, one)\
        log_message(LL_INFO, s("Native fn: %.*s(%.*s)"),\
            fmt(self->name), fmt(Box_to_FixStr(one)));
    #define native_log_call2(self, one, two)\
        log_message(LL_INFO, s("Native fn: %.*s(%.*s, %.*s)"),\
            fmt(self->name), fmt(Box_to_FixStr(one)), fmt(Box_to_FixStr(two)));
#else
    #define native_log_call(self, args)
    #define native_log_call1(self, one)
    #define native_log_call2(self, one, two)
#endif

#define native_return_error_if(test, ...) \
    if (test) return Box_wrap_FixError(native_error(__VA_ARGS__));\

#pragma endregion

#pragma region BoxGenericImpl

#define Box_return_noimpl()\
    native_return_error_if(true, sMSG("Not yet implemented"));


bool Box_eq(Box a, Box b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case UBX_INT:
            return Box_unwrap_int(a) == Box_unwrap_int(b);
        case UBX_BOOL:
            return Box_unwrap_bool(a) == Box_unwrap_bool(b);
        case UBX_FLOAT:
            return Box_unwrap_float(a) == Box_unwrap_float(b);
        default:
            return a.payload == b.payload;
    }

}


FixStr Unboxed_to_FixStr(Box box) {
    switch (box.type) {
        case UBX_INT:
            return FixStr_fmt_new(s("%d"), Box_unwrap_int(box));
        case UBX_BOOL:
            return FixStr_fmt_new(s("%s"), Box_unwrap_bool(box) ? s("true") : s("false"));
        case UBX_FLOAT:
            return FixStr_fmt_new(s("%f"), Box_unwrap_float(box));
        case UBX_TAG:
            return FixStr_fmt_new(s("Tag(%d)"), Box_unwrap_tag(box));
        default:
            return FixStr_fmt_new(s("Box(%d)"), box.type);
    }
}

/// @brief
/// @param boxed
/// @return
FixStr Boxed_to_FixStr(Boxed *boxed) {

    MetaType type = boxed->meta.type;
    /* MetaType:
    BXD_FIX_STR BXD_FIX_ARRAY BXD_FIX_DICT BXD_FIX_KVPAIR BXD_FIX_SCOPE BXD_FIX_AST BXD_FIX_DISTRIBUTION
    BXD_FIX_SYSTEM_INFO BXD_FIX_MODULE BXD_FIX_STRUCT BXD_FIX_ENUM BXD_FIX_TRAIT BXD_FIX_TYPE BXD_FIX_OBJECT
    BXD_FIX_ARRAY_VIEW BXD_FIX_VEC_TAGS BXD_FIX_NUM_128BIT BXD_FIX_NUM_DOUBLE
    BXD_FIX_VEC_DOUBLE_1 BXD_FIX_VEC_DOUBLE_2 BXD_FIX_VEC_DOUBLE_3 BXD_FIX_VEC_DOUBLE_4
    BXD_FLX_OBJECT BXD_FLX_STR BXD_FLX_ARRAY BXD_FLX_QUEUE BXD_FLX_QUEUE_PRIORITY BXD_FLX_STACK BXD_FLX_DICT BXD_FLX_SET
    BXD_FLX_GRAPH_DIRECTED_WEIGHTED BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED
    BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED BXD_FLX_VEC_DOUBLE_N BXD_FLX_MATRIX BXD_FLX_MATRIX_DOUBLE
    BXD_FLX_TENSOR BXD_FLX_TENSOR_DOUBLE BXD_FLX_DATAFRAME BXD_FLX_VEC_BOOL BXD_FLX_VEC_INT BXD_FLX_VEC_TAGS
    BXD_FLX_VEC_STR BXD_FLX_VEC_OBJS
    */

   /// @note `Boxed` is a parent struct of Fix,Boxed types
   /// so we can extract their fields with:
   /// Boxed_as(FixStr, boxed)
    switch (type) {
        case BXD_FIX_STR:
        case BXD_FLX_STR:
            return FixStr_fmt_new(s("'%.*s'"), fmt_ref(Boxed_as(FixStr, boxed)));
        case BXD_FIX_ARRAY:
            return FixArray_to_FixStr(Boxed_as(FixArray, boxed));
        case BXD_FIX_DICT:
            return FixDict_to_FixStr(Boxed_as(FixDict, boxed));
        case BXD_FIX_KVPAIR:
            return FixKvPair_to_FixStr(Boxed_as(FixKvPair, boxed));
        case BXD_FIX_SCOPE:
            return FixScope_to_FixStr(Boxed_as(FixScope, boxed));
        case BXD_FIX_STRUCT:
            return FixStruct_to_FixStr(Boxed_as(FixStruct, boxed));
        case BXD_FLX_DICT:
            return FlxDict_to_FixStr(Boxed_as(FlxDict, boxed));
        default:
            return FixStr_fmt_new(s("Boxed(%d)"), type);
    }
        // case BXD_FIX_AST:
            // return Ast_to_FixStr(Boxed_as(FixAst *, boxed));
        // case BXD_FIX_DISTRIBUTION:
        //     return FixFixDist_to_FixStr(Boxed_as(FixFixDist, boxed));
        // case BXD_FIX_SYSTEM_INFO:
        //     return FixSystemInfo_to_FixStr(Boxed_as(FixSystemInfo, boxed));
        // case BXD_FIX_MODULE:
            // return FixModule_to_FixStr(Boxed_as(FixModule, boxed));
        // case BXD_FIX_ENUM:
        //     return FixEnum_to_FixStr(Boxed_as(FixEnum, boxed));
        // case BXD_FIX_TRAIT:
        //     return FixTrait_to_FixStr(Boxed_as(FixTrait, boxed));
        // case BXD_FIX_TYPE:
        //     return FixType_to_FixStr(Boxed_as(FixType, boxed));
        // case BXD_FIX_OBJECT:
        //     return FixObject_to_FixStr(Boxed_as(FixObject, boxed));
        // case BXD_FIX_ARRAY_VIEW:
        //     return FixArrayView_to_FixStr(Boxed_as(FixArrayView, boxed));
        // case BXD_FIX_VEC_TAGS:
        //     return FixVecTags_to_FixStr(Boxed_as(FixVecTags, boxed));
        // case BXD_FIX_NUM_128BIT:
        //     return FixNum128Bit_to_FixStr(Boxed_as(FixNum128Bit, boxed));
        // case BXD_FIX_NUM_DOUBLE:
        //     return FixNumDouble_to_FixStr(Boxed_as(FixNumDouble, boxed));
        // case BXD_FIX_VEC_DOUBLE_1:
        //     return FixVecDouble1_to_FixStr(Boxed_as(FixVecDouble1, boxed));
        // case BXD_FIX_VEC_DOUBLE_2:
        //     return FixVecDouble2_to_FixStr(Boxed_as(FixVecDouble2, boxed));
        // case BXD_FIX_VEC_DOUBLE_3:
        //     return FixVecDouble3_to_FixStr(Boxed_as(FixVecDouble3, boxed));
        // case BXD_FIX_VEC_DOUBLE_4:
        //     return FixVecDouble4_to_FixStr(Boxed_as(FixVecDouble4, boxed));
        // case BXD_FLX_OBJECT:
        //     return FlxObject_to_FixStr(Boxed_as(FlxObject, boxed));
        // case BXD_FLX_STR:
        //     return FlxStr_to_FixStr(Boxed_as(FlxStr, boxed));
        // case BXD_FLX_ARRAY:
        //     return FixArray_to_FixStr(Boxed_as(FlxArray, boxed));
        // case BXD_FLX_QUEUE:
        //     return FlxQueue_to_FixStr(Boxed_as(FlxQueue, boxed));
        // case BXD_FLX_QUEUE_PRIORITY:
        //     return FlxQueuePriority_to_FixStr(Boxed_as(FlxQueuePriority, boxed));
        // case BXD_FLX_STACK:
        //     return FlxStack_to_FixStr(Boxed_as(FlxStack, boxed));
        // case BXD_FLX_SET:
        //     return FlxSet_to_FixStr(Boxed_as(FlxSet, boxed));
        // case BXD_FLX_VEC_DOUBLE_N:
        //     return FlxVecDoubleN_to_FixStr(Boxed_as(FlxVecDoubleN, boxed));
        // case BXD_FLX_MATRIX:
        // case BXD_FLX_MATRIX_DOUBLE:
        //     return FlxMatrix_to_FixStr(Boxed_as(FlxMatrix, boxed));
        // case BXD_FLX_TENSOR:
        // case BXD_FLX_TENSOR_DOUBLE:
        //     return FlxTensor_to_FixStr(Boxed_as(FlxTensor, boxed));
        // case BXD_FLX_DATAFRAME:
        //     return FlxDataFrame_to_FixStr(Boxed_as(FlxDataFrame, boxed));
        // case BXD_FLX_VEC_BOOL:
        //     return FlxVecBool_to_FixStr(Boxed_as(FlxVecBool, boxed));
        // case BXD_FLX_GRAPH_DIRECTED_WEIGHTED:
        // case BXD_FLX_GRAPH_DIRECTED_UNWEIGHTED:
        // case BXD_FLX_GRAPH_UNDIRECTED_WEIGHTED:
        // case BXD_FLX_GRAPH_UNDIRECTED_UNWEIGHTED:
        //     return FlxGraph_to_FixStr(Boxed_as(FlxGraph, boxed));


}
FixStr Box_to_FixStr(Box b) {
    /// @todo refactor as _to_FixStr for all types
    switch (b.type) {
        case UBX_INT:
            return FixStr_fmt_new(s("%lld"), Box_unwrap_int(b));
        case UBX_BOOL:
            return FixStr_fmt_new(s("%s"), Box_unwrap_bool(b) ? "true" : "false");
        case UBX_TAG:
            return FixStr_fmt_new(s("Tag(%.*s)"), fmt(Box_unwrap_tag(b)));
        case UBX_FLOAT:
        // case UBX_DYN_DOUBLE: /// @todo impl doubles
            return FixStr_fmt_new(s("%.6f"), Box_unwrap_float(b));
        // case UBX_PTR:
            // return FixStr_fmt_new(s("BoxPtr(%p)"), Box_unwrap_Box_ptr(b));
        case UBX_PTR_ERROR:
            return FixStr_fmt_new(s("Error(%.*s)"), fmt(Box_unwrap_FixError(b)->message));

        case UBX_PTR_ARENA:
        case UBX_PTR_HEAP:
            return Boxed_to_FixStr(Box_unwrap_Boxed(b));
        // // case UBX_PTR_ARENA :
        //     FixStr str = Box_unwrap_FixStr(b);
        //     return str;
        //     // return FixStr_fmt_new(s("'%.*s'"), fmt(str));
        // case UBX_PTR_ARENA:
        //     FixFn *f = Box_unwrap_FixFn(b);
        //     return FixStr_fmt_new(s("Fn(%.*s)"), fmt(f->name));
        // case UBX_PTR_ARENA:
        //     return FixArray_to_FixStr(Box_unwrap_FixArray(b));

        //     /// @todo URGENT -- implement repr for all types
        // // case UBX_PTR_ARENA:
        // //     return FixDict_to_FixStr(Box_unwrap_FixDict(b));
        // case UBX_PTR_ARENA:
        //     return Boxed_to_FixStr(Box_unwrap_Boxed(b));
        // // case UBX_PTR_HEAP :
        // //     return FlxObject_to_FixStr(Box_unwrap_FixObject(b));
        // case UBX_PTR_ARENA:
        //     return sMSG("struct-todo");
        default:
            return FixStr_fmt_new(s("Box?(%d)"), b.type);
    }
}

bool Box_try_numeric(Box box, double *out_value) {
    switch(box.type) {
        case UBX_INT:
            *out_value = (double)Box_unwrap_int(box);
            return true;
        case UBX_FLOAT:
            *out_value = (double)Box_unwrap_float(box);
            return true;
        // case UBX_DYN_DOUBLE:
        //     *out_value = (double)Box_unwrap_double(box);
        //     return true;
        default:
            return false;
    }
}


/// @brief Combines MetaType and payload into a single 64-bit key and applies bit mixing
size_t Box_hash(Box box) {
    uint64_t key = (uint64_t)box.type << 60;

    /// @note strings can be hashed on their pointers if they've been interned
    /// ....    ie., they're guaranteed to be pointer-stable
    /// @todo ensure all strings used as keys on Box'd maps are interned
    /// ....    unclear on pro/con for similar on dynamic dicts
    /// ....    rules for interning: (key of fixed dict) || (< 16 char)
    if(box.type == UBX_PTR_ARENA) {
        return FixStr_hash(Box_unwrap_FixStr(box));

        #ifdef INTERP_PRECACHE_INTERNED_STRINGS
        /// @todo
        // FixKvPair out;
        // if(!interp_precache_getstr(Box_unwrap_FixStr(box), &out)) {
        //     /// @note implementing FNV-1a 64-bit hash for the string
        //     log_message(LL_INFO, sMSG("Hashing an non-interned string"));
        //     return FixStr_hash(Box_unwrap_FixStr(box));
        // }
        #endif
    }


    if(box.type == UBX_PTR_ARENA && Box_is_numeric(box)) {
        key |= (uint64_t) Box_force_numeric(box);
    } else {
        /// everything else can be hashed on their pointers
        /// @todo ... this assumes their pointers won't move
        key |= (box.payload & 0x0FFFFFFFFFFFFFFFUL);
    }

    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;

    return (size_t)key;
}




void test_wrap_unwrap_double(void) {
    log_assert(false, sMSG("Not implemented"));
}


void test_wrap_unwrap_float(void) {
    float original = 3.141592653589793;
    Box boxed = Box_wrap_float(original);

    float unboxed = Box_unwrap_float(boxed);
    log_assert(original == unboxed, sMSG("Floats not unpacking"));
}


void Box_test_main(void) {
    test_wrap_unwrap_double();
    test_wrap_unwrap_float();
}

#pragma endregion

#pragma region BoxApiImpl

FlxStr *FixStr_flexible_new(FixStr str) {
    FlxStr *flex_str = gco_new(BXD_FLX_STR);

    if(Arena_is_forever(ctx_current_arena())) {
        flex_str->cstr = (char *) str.cstr;
        flex_str->meta.size = str.size;
    } else {
        FixStr unowned = FixStr_unowned_cnew(str);
        flex_str->cstr = (char *) unowned.cstr;
        flex_str->meta.size = unowned.size;
    }
    return flex_str;
}

/// @brief
/// @note
///     most boxes are going to come from user land, so will arrive as a boxed pointer
///     ... so the API may as well assume everything is a pointer
///     Even though we will create some of these on the stack
/// @todo -- change everything to Box *


void Box_print(Box box) {
    FixStr str = Box_to_FixStr(box);
    log_message(LL_INFO, sMSG("Box: %.*s"), fmt(str));
    // FixStr_aro_free(str);
}



void FixArray_Arena_new_data(FixArray *array, size_t size) {
    require_not_null(array);

    if(size == 0) {
        array->data = nullptr;
        return;
    }

    /// @note supress warning about typeof
    array->meta.alloc_size = size * sizeof(typeof((array)->data[0])); // NOLINT
    array->meta.capacity = size;
    array->meta.size = 0;

    array->data = Arena_alloc(array->meta.alloc_size);
    if(!array->data) { error_oom(); }
    // memset_s(array->data, 0, array->meta.alloc_size);
    require_safe(clib_memset_zero_safe(array->data, array->meta.alloc_size, array->meta.alloc_size));
}


FixStr FixArray_to_FixStr(FixArray *array) {
    if(len_ref(array) == 0) {
        return s("[]");
    }

    FixStr repr = s("[");
    for(size_t i = 0; i < len_ref(array); i++) {
        repr = FixStr_glue_sep_new(repr, Box_to_FixStr(array->data[i]), s(", "));
    }
    repr = FixStr_glue_new(repr, s("]"));
    return repr;
}


/// @brief
/// @todo -- does this make sense, does size change?
/// @param box_array
/// @param element
/// @param index
/// @return
Box FixArray_set(FixArray *box_array, Box element, size_t index) {
    native_return_error_if(index > capacity_ref(box_array), s("FixArray.set(): Index out of bounds."));

    /// @note FixArrays do not resize,
    // nor can we change their size, so we need to know the index
    box_array->data[index] = element;
    return element;
}

/// @brief
/// @example
///     const cheap_set = ["A", "B", "C"]
///     cheap_set.drop("A")
///     cheap_set.drop_at(1)
/// @param array
/// @param index
/// @return
Box FixArray_unordered_remove(FixArray *array, size_t index) {
    native_return_error_if(index >= len_ref(array), s("FixArray.remove(): Index out of bounds."));

    Box removed = array->data[index];
    array->data[index] = array->data[len_ref(array) - 1];
    decr_len_ref(array);
    return removed;
}

/// @brief
///     surfaced as `removing` to suggest the operation involves moving data
/// @example
///     const array = ["A", "B", "C"]
///     array.removing("A")
///     array.removing_at(0)
/// @param array
/// @param index
/// @return
Box FixArray_ordered_remove(FixArray *array, size_t index) {
    native_return_error_if(index >= len_ref(array), s("FixArray.remove(): Index out of bounds."));

    Box removed = array->data[index];
    for(size_t i = index; i < len_ref(array) - 1; i++) {
        array->data[i] = array->data[i + 1];
    }
    decr_len_ref(array);
    return removed;
}

Box FixArray_append(FixArray *array, Box element) {
    require_not_null(array);
    require_positive(capacity_ref(array));

    native_return_error_if(len_ref(array) >= capacity_ref(array), s("FixArray.append(): Array is full."));

    push_ref(array, element);
    return element;
}

/// @brief
/// @todo -- maybe refactor with _to_FixStr
/// @param array
void FixArray_debug_print(FixStr name, FixArray *array) {
    if(array == nullptr) {
        log_message(LL_INFO, s("%.*s: nullptr"), fmt(name));
        return;
    }

    if(len_ref(array) == 0) {
        log_message(LL_INFO, s("%.*s: []"), fmt(name));
        return;
    }

    for(size_t i = 0; i < len_ref(array); i++) {
        FixStr str = Box_to_FixStr(array->data[i]);
        log_message(LL_INFO, s("%.*s[%d]: %.*s"), fmt(name), i, fmt(str));
        // FixStr_aro_free(str);
    }
}

Box FixArray_get(FixArray array, size_t index) {
    native_return_error_if(index >= len(array), s("FixArray.get(): Index out of bounds."));
    return array.data[index];
}

Box FixArray_overwrite(FixArray *array, size_t index, Box element) {
    native_return_error_if(index >= len_ref(array), s("FixArray.overwrite(): Index out of bounds."));
    array->data[index] = element;
    return element;
}

Box FixArray_weak_unset(FixArray *array, size_t index) {
    native_return_error_if(index >= len_ref(array), s("FixArray.weak_unset(): Index out of bounds."));
    array->data[index] = Box_null();
    return Box_null(); // Added return statement
}




/// @brief
/// @example
///     const vec1 = [10, 20, 30]
///     const vec2 = Vector {10, 20, 30}
///     const set1 = {1, 1, 2, 3}
///     const set2 = Set {1, 1, 2, 3}
///     const dict1 = {1: "Hello"}
///     const dict2 = Dict {{1, "Hello"}, {2, "Goodbye"}}
///     const stack1 = Stack {1, 2, 3}
///     const queue1 = Queue {1, 2, 3}
/// @param initial_capacity
/// @return

#define FixArray_new_auto(cap) FixArray_new(cap, LIFETIME_ARENA_AUTO)

FixArray *FixArray_new(size_t initial_capacity, LifetimeType lifetime) {
    if(lifetime == LIFETIME_ARENA_AUTO) {
        lifetime = ctx_current_arena()->lifetime;
    }

    FixArray *array;

    if(lifetime == LIFETIME_HEAP_GC) {
        array = gco_new(BXD_FLX_ARRAY);
        cnew_carray(array, initial_capacity);
    } else {
        array = aro_new(BXD_FLX_ARRAY);
        FixArray_Arena_new_data(array, initial_capacity);
    }

    if(array == nullptr) { error_oom(); return nullptr; }


    // array->meta.capacity = initial_capacity;
    // array->meta.size = 0;
    // array->data = cnew(initial_capacity, sizeof(Box));
    // if(array->data == nullptr) { error_oom(); return nullptr;}


    return array;
}



/// ----- FlxArray ----- ///

/// @brief Append an element to the end of the array.
/// @note Use reallocation and HeapArray_next_capacity
///     ... to resize the array
Box FlxArray_append(FixArray *array, Box element) {
    require_not_null(array);
    require_positive(capacity_ref(array));
    if(is_empty_ref(array)) {
        size_t new_capacity = HeapArray_next_capacity(capacity_ref(array));
        Box *new_data = cextend(array->data, new_capacity * sizeof(Box));
        if(new_data == nullptr) { error_oom(); return Box_error_empty(); }
        array->data = new_data;
        capacity_ref(array) = new_capacity;
    }

    push_ref(array, element);
    return element;
}

Box FlxArray_get(FixArray *array, size_t index) {
    require_not_null(array);
    native_return_error_if(index >= len_ref(array), s("FlxArray.get(): Index out of bounds."));
    return array->data[index];
}



FlxDict *FlxDict_new(size_t initial_capacity) {
    FlxDict *dict = gco_new(BXD_FLX_DICT);
    if(dict == nullptr) { error_oom(); return nullptr; }

    dict->meta.type = BXD_FLX_DICT;
    cnew_carray(dict, initial_capacity);
    return dict;
}


/// @brief  Set a key-value pair in the dictionary.
/// @note   If the key already exists, the value will be updated.
/// @note       uses probing and open addressing to handle collisions
/// @note   struct FlxKvPair { size_t hash; Box key; Box value; bool occupied; }
/// @param FlxDict *dict
/// @param Box key
/// @param Box val
/// @return Box
Box FlxDict_set(FlxDict *dict, Box key, Box val) {
    require_not_null(dict);

    size_t hash = Box_hash(key);
    size_t index = hash % capacity_ref(dict);
    size_t start = index;

    while(dict->data[index].occupied) {
        if(Box_eq(dict->data[index].key, key)) {
            dict->data[index].value = val;
            return val;
        }
        index = (index + 1) % capacity_ref(dict);

        /// @note reallocate
        if(index == start) {
            size_t new_capacity = HeapArray_next_capacity(capacity_ref(dict));
            FlxKvPair *new_data = cextend(dict->data, new_capacity * sizeof(FlxKvPair));
            if(new_data == nullptr) { error_oom(); return Box_error_empty(); }
            dict->data = new_data;
            dict->meta.capacity = new_capacity;
            index = hash % capacity_ref(dict);
        }
    }

    dict->data[index].hash = hash;
    dict->data[index].key = key;
    dict->data[index].value = val;
    dict->data[index].occupied = true;
    incr_len_ref(dict);
    return val;
}

bool FlxDict_find(FlxDict *dict, Box key, FlxKvPair *out) {
    require_not_null(dict);

    size_t hash = Box_hash(key);
    size_t index = hash % capacity_ref(dict);
    size_t start = index;

    while(dict->data[index].occupied) {
        if(Box_eq(dict->data[index].key, key)) {
            *out = dict->data[index];
            return true;
        }
        index = (index + 1) % capacity_ref(dict);
        if(index == start) {
            return false;
        }
    }

    return false;
}

Box FlxDict_get(FlxDict *dict, Box key) {
    require_not_null(dict);

    FlxKvPair entry;
    if(FlxDict_find(dict, key, &entry)) {
        return entry.value;
    }

    return Box_empty();
}



FixStr FlxObject_to_FixStr(FlxObject *obj) {
    FixStr template_name = obj->data->name;
    // FixStr doc_string = obj->data->doc_string;
    FixStr fields = FixDict_to_FixStr(&obj->data->namespace.data);
    FixStr repr = FixStr_fmt_new(s("<%.*s: %.*s>"), fmt(template_name), fmt(fields));
    return repr;
}

// FixStr FixArray_to_FixStr(FixArray *array) {
//     FixStr repr = s("[");
//     for(size_t i = 0; i < dyn_size(array); i++) {
//         repr = FixStr_glue_sep_new(repr, Box_to_FixStr(array->data[i]), s(", "));
//     }
//     repr = FixStr_glue_new(repr, s("]"));
//     return repr;
// }

FixStr FlxDict_to_FixStr(FlxDict *dict) {
    FixStr repr = s("{");
    for(size_t i = 0; i < dyn_size(dict); i++) {
        repr = FixStr_glue_sep_new(repr, Box_to_FixStr(dict->data[i].key), s(": "));
        repr = FixStr_glue_sep_new(repr, Box_to_FixStr(dict->data[i].value), s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}

FixStr FlxSet_to_FixStr(FlxSet *set) {
    FixStr repr = s("{");
    for(size_t i = 0; i <  dyn_size(set); i++) {
        repr = FixStr_glue_sep_new(repr, Box_to_FixStr(set->data[i]), s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}


FixStr FlxVecDouble_to_FixStr(FlxVecDouble *vec) {
    FixStr repr = s("VecDouble {");
    for(size_t i = 0; i < dyn_size(vec); i++) {
        double value = vec->data[i];
        repr = FixStr_glue_sep_new(repr, FixStr_fmt_new(s("%.6f"), value), s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}

FixStr FlxVecBool_to_FixStr(FlxVecBool *vec) {
    FixStr repr = s("VecBool {");
    for(size_t i = 0; i < dyn_size(vec); i++) {
        bool value = vec->data[i];
        repr = FixStr_glue_sep_new(repr, value ? s("true") : s("false"), s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}

FixStr FlxVecInt_to_FixStr(FlxVecInt *vec) {
    FixStr repr = s("VecInt {");
    for(size_t i = 0; i < dyn_size(vec); i++) {
        int value = vec->data[i];
        repr = FixStr_glue_sep_new(repr, FixStr_fmt_new(s("%d"), value), s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}

FixStr FlxVecChar_to_FixStr(FlxVecChar *vec) {
    FixStr repr = s("VecChar {");
    for(size_t i = 0; i < dyn_size(vec); i++) {
        char cstr = vec->data[i];
        repr = FixStr_glue_sep_new(repr, FixStr_fmt_new(s("'%c'"), cstr), s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}

FixStr FlxVecFixStr_to_FixStr(FlxVecFixStr *vec) {
    FixStr repr = s("VecFixStr {");
    for(size_t i = 0; i < dyn_size(vec); i++) {
        FixStr value = vec->data[i];
        repr = FixStr_glue_sep_new(repr, value, s(", "));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}


/// @brief Retrieves an attribute from a FlxObject without specifying the type.
/// @param obj The FlxObject from which to retrieve the attribute.
/// @param name The name of the attribute.
/// @return The Box representing the attribute's value, or Box_error() if not found.
Box FlxObject_getattr(FlxObject *obj, FixStr name) {
    require_not_null(obj);

    // Lookup the attribute in the object's namespace
    Box value = FixDict_get(&obj->data->namespace.data, name);

    native_return_error_if(Box_is_error(value), s("Attribute `%.*s` not found in object"), fmt(name));

    return value;
}

/// @brief Retrieves an attribute from a FlxObject with  type checking.
/// @param obj The FlxObject from which to retrieve the attribute.
/// @param name The name of the attribute.
/// @param expected_type The expected MetaType of the attribute. Use UBX_INVALID if no type checking is needed.
/// @return The Box representing the attribute's value, or Box_error() if not found or type mismatch.
Box FlxObject_getattr_checked(FlxObject *obj, FixStr name, MetaType expected_type) {
    require_not_null(obj);

    Box value = FixDict_get(&obj->data->namespace.data, name);

    native_return_error_if(Box_is_error(value), s("Attribute `%.*s` not found in object"), fmt(name));

    native_return_error_if(value.type != expected_type,
        s("Attribute `%.*s` expected to be of type %s, but got %s"),
        fmt(name), ubx_nameof(expected_type), ubx_nameof(value.type));

    return value;
}


/// @brief
/// @todo -- tidy up
/// @param obj
/// @param scope
/// @param name
/// @return
Box FlxObject_find_method(Box obj, FixScope *scope, FixStr name) {
    require_not_null(scope);

    Box fn;
    if(Box_is_object(obj)) {
        /// @note we first look for a method in the object's namespace
        if(FixScope_lookup(&Box_unwrap_FixObject(obj)->data->namespace, name, &fn)) {
            native_return_error_if(fn.type != UBX_PTR_ARENA, s("Attribute `%.*s` is not a function"), fmt(name));
            Box_unwrap_FixFn(fn)->enclosure.parent = &Box_unwrap_FixObject(obj)->data->namespace;
            return fn;
        }

        /// @note then we look for a global/parent-scope function to support universal calling
        FixStr method_name = FixStr_glue_sep_new(Box_unwrap_FixObject(obj)->data->name, s("_"), name);
        if(FixScope_lookup(scope, method_name, &fn)) {
            native_return_error_if(fn.type != UBX_PTR_ARENA, s("Attribute `%.*s` is not a function"), fmt(method_name));
            Box_unwrap_FixFn(fn)->enclosure.parent = &Box_unwrap_FixObject(obj)->data->namespace;
            return fn;
        }
    } else {
        /// @note we then look for a global/parent-scope generic method
        if(FixScope_lookup(scope, name, &fn)) {
            native_return_error_if(fn.type != UBX_PTR_ARENA, s("Attribute `%.*s` is not a function"), fmt(name));
            return fn;
        }
    }

    native_return_error_if(true, s("Method `%.*s` not found in object"), fmt(name));
}


FixStr FixKvPair_to_FixStr(FixKvPair *pair) {
    FixStr repr = FixStr_fmt_new(s("%.*s: %.*s"), fmt(pair->key), fmt(Box_to_FixStr(pair->value)));
    return repr;
}


FixStr FixStruct_to_FixStr(FixStruct *s) {
    require_not_null(s);

    FixStr repr = s("Struct {");
    for(size_t i = 0; i < len_ref(&s->fields); i++) {
        repr = FixStr_glue_sep_new(repr, s(" "), FixKvPair_to_FixStr(s->fields.data[i]));
    }
    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}

FixStr FixScope_to_FixStr(FixScope *scope) {
    require_not_null(scope);

    FixStr repr = s("{ TODO");

    repr = FixStr_glue_new(repr, s("}"));
    return repr;
}


FixStr FixDict_to_FixStr(FixDict *d) {
    require_not_null(d);

    if(len_ref(d) == 0) {
        return s("{}");
    }

    FixStr repr = s("{");
    for(size_t i = 0; i < capacity_ref(d); i++) {
        FixKvPair *current = d->data[i];
        while(current != nullptr) {
            repr = FixStr_glue_sep_new(repr, s(" "), current->key);
            repr = FixStr_glue_sep_new(repr, s(": "), Box_to_FixStr(current->value));
            current = current->next;

            if(current != nullptr) {
                repr = FixStr_glue_new(repr, s(", "));
            }
        }
    }
    repr = FixStr_glue_new(repr, s(" }"));
    return repr;
}

#define FixDict_bucket_for_key(dict, key) ((dict)->data[FixStr_hash(key) % (dict)->meta.capacity])

/// @brief Initialize a FixDict with a specified number of data for separate chaining.
void FixDict_data_new(FixDict *dict, size_t size) {
    require_not_null(dict);
    if(size == 0) size = ARRAY_SIZE_SMALL;
    Arena_MetaValue_init_new(dict, size);
}



/// @brief Get a value from a FixDict by key, return Box_null() if not found.
Box FixDict_get(FixDict *dict, FixStr key) {
    require_not_null(dict);

    FixKvPair entry;
    if (FixDict_find(dict, key, &entry)) {
        return entry.value;
    } else {
        return Box_empty();
    }
}

FixStr FixDict_keys_to_FixStr(FixDict *dict) {
    require_not_null(dict);

    FixStr result = FixStr_empty();
    for (size_t i = 0; i < capacity_ref(dict); i++) {
        FixKvPair *current = dict->data[i];
        while (current != nullptr) {
            result = FixStr_glue_sep_new(result, s(", "), current->key);
            current = current->next;
        }
    }
    return result;
}

void FixDict_debug_print_entry(FixKvPair *entry) {
    printf("%.*s: %.*s\n", fmt(entry->key), fmt(Box_to_FixStr(entry->value)));
}

void FixDict_debug_print(FixDict dict) {
    printf("FixDict (size: %zu, capacity: %zu):\n", len(dict), cap(dict));
    for (size_t i = 0; i < cap(dict); i++) {
        FixKvPair *current = dict.data[i];
        while (current != nullptr) {
            FixDict_debug_print_entry(current);
            current = current->next;
        }
    }
}

bool FixDict_find(FixDict *dict, FixStr key, FixKvPair *out) {
    require_not_null(dict);
    if(len_ref(dict) == 0) return false;

    // require_positive(dict->meta.capacity);

    size_t hash = FixStr_hash(key);
    size_t index = hash % capacity_ref(dict);

    FixKvPair *current = dict->data[index];
    while (current != nullptr) {
        if (current->hash == hash && FixStr_eq(current->key, key)) {
            *out = *current;
            native_log_debug(sMSG("Found key '%.*s' in bucket %zu"), fmt(key), index);
            return true;
        }
        current = current->next;
    }

    native_log_debug(sMSG("Key '%.*s' not found (looking in bucket %zu)"), fmt(key), index);
    return false;
}


void FixDict_append(FixDict *dict, FixStr key, Box value)  {
    require_not_null(dict);
    require_positive(capacity_ref(dict));

    /// @note we require the key to be unique
    FixKvPair entry = {0};
    if(FixDict_find(dict, key, &entry)) {
        native_error(s("`%.*s` already exists"), fmt(key));
    } else {
        /// @note if it is, we set
        FixDict_set(dict, key, value);
    }

}

FixKvPair *FixKvPair_new(FixStr key, Box value) {
    FixKvPair *pair = Arena_alloc(sizeof(FixKvPair));
    if(pair == nullptr) {
        error_oom();
    }
    pair->key = key;
    pair->value = value;
    pair->hash = FixStr_hash(key);
    pair->next = nullptr;
    return pair;
}

/// @brief initialises a new dict with `pairs` as key-value pairs
///     faster than inserting each pair individually since
///         we don't check for existing keys
void FixDict_from_pairs(FixDict *empty_dict, FixKvPair **pairs, size_t num_pairs) {
    require_not_null(empty_dict);
    require_positive(capacity_ref(empty_dict));
    require_not_null(pairs);

    if(num_pairs == 0) {
        return;
    }

    /// @note we don't check for existing keys, so we don't use FixDict_set
    for(size_t i = 0; i < num_pairs; i++) {
        size_t index = pairs[i]->hash % capacity_ref(empty_dict);
        pairs[i]->next = empty_dict->data[index];
        empty_dict->data[index] = pairs[i];
        len_ref(empty_dict)++;
    }
}

/// @brief Insert or update a key-value pair in FixDict using separate chaining.
void FixDict_set(FixDict *dict, FixStr key, Box value) {
    require_not_null(dict);
    require_positive(capacity_ref(dict));

    #ifdef INTERP_PRECACHE_INTERNED_STRINGS
        /// @note Intern all dict keys for FixDicts
        interp_precache_setstr(key);
    #endif

    size_t hash = FixStr_hash(key);
    size_t index = hash % capacity_ref(dict);

    FixKvPair *current = dict->data[index];
    while (current != nullptr) {
        if (current->hash == hash && FixStr_eq(current->key, key)) {
            // Key exists, update the value
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Key not found, create a new entry and add it to the front of the chain
    FixKvPair *new_entry = (FixKvPair *)Arena_alloc(sizeof(FixKvPair));
    if (new_entry == nullptr) {
        error_oom();
        return;
    }
    new_entry->hash = hash;
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = dict->data[index];
    dict->data[index] = new_entry;
    incr_len_ref(dict);
}


FixKvPair *FixDict_remove(FixDict *dict, FixStr key) {
    require_not_null(dict);
    require_positive(capacity_ref(dict));

    size_t hash = FixStr_hash(key);
    size_t index = hash % capacity_ref(dict);

    FixKvPair *current = dict->data[index];
    FixKvPair *prev = nullptr;
    while (current != nullptr) {
        if (current->hash == hash && FixStr_eq(current->key, key)) {
            if (prev == nullptr) {
                dict->data[index] = current->next;
            } else {
                prev->next = current->next;
            }
            decr_len_ref(dict);
            return current;
        }
        prev = current;
        current = current->next;
    }
    return nullptr;
}


void FixScope_data_new(FixScope *self, FixScope *parent) {
    require_not_null(self);
    require_not_null(parent);

    self->parent = parent;
    FixDict_data_new(&self->data, ARRAY_SIZE_SMALL);
}


void FixScope_sized_new(FixScope *self, FixScope *parent, size_t num_data) {
    self->parent = parent;
    FixDict_data_new(&self->data, num_data);
}

void FixScope_debug_print(FixScope scope) {
    /// @note print all the data and the parent
    log_message(LL_INFO, sMSG("Printing Scope"));
    if(scope.doc_string.size > 0) FixStr_println(scope.doc_string);

    /// @note we will use `FixDict_debug_print_entry` to print each entry
    for(size_t i = 0; i < scope.data.meta.capacity; i++) {
        FixKvPair *current = scope.data.data[i];
        while(current != nullptr) {
            FixDict_debug_print_entry(current);
            current = current->next;
        }
    }

    if(scope.parent != nullptr) {
        printf("Parent Scope:\n");
        FixScope_debug_print(*scope.parent);
    }
}

/// @brief
/// @param dest
/// @param src
/// @todo -- is a parent merge necessary?
void FixScope_merge_local(FixScope *dest, FixScope *src) {
    require_not_null(dest);
    require_not_null(src);

    FixDict_merge(&dest->data, &src->data);
}

void FixDict_merge(FixDict *dest, FixDict *src) {
    require_not_null(dest);
    require_not_null(src);

    for(size_t i = 0; i < capacity_ref(src); i++) {
        FixKvPair *current = src->data[i];
        while(current != nullptr) {
            /// @todo this should be checked?
            FixDict_set(dest, current->key, current->value);
            current = current->next;
        }
    }
}

bool FixScope_lookup(FixScope *scope, FixStr name, Box *out_value) {
    FixKvPair entry;
    while (scope != nullptr) {
        if (FixDict_find(&scope->data, name, &entry)) {
            *out_value = entry.value;
            native_log_debug(sMSG("Found variable '%.*s'"), fmt(name));
            return true;
        }
        scope = scope->parent;
    }

    native_log_debug(sMSG("Variable '%.*s' not found"), fmt(name));
    return false;
}


bool FixScope_has(FixScope *scope, FixStr name) {
    Box value;
    return FixScope_lookup(scope, name, &value);
}

/// @brief
/// @param scope
/// @todo all fixed stuff should be on an arena and freed in bulk?
void FixScope_aro_free(FixScope *scope) {
    //return;
    /// @todo free all the strings in the dict

}

static FixFn *FixFn_new(FixFnType type, FixStr name,
    FixDict args_defaults, FixScope enclosure, void *fnptr, void *code) {

    FixFn *fn = Arena_alloc(sizeof(FixFn));
    if (!fn) {
        error_oom();
    }
    fn->type = type;
    fn->name = name;
    fn->signature = args_defaults;
    fn->enclosure = enclosure;
    fn->fnptr = fnptr;
    fn->code = code;
    return fn;
}


Box FixFn_call(FixFn *ffn, FixScope scope, FixArray args) {
    require_not_null(ffn);
    require_not_null(ffn->fnptr);

    switch(ffn->type) {
        case FN_USER:
            return FixFn_typed_call(FixFnUser, ffn, scope, args);
        case FN_GENERATOR:
            /// @note calling convention for generators requires
            ///     the last argument to be an iterator
            /// this is handled by interpreter when constructing the call
            // FixIter *_iter = Boxed_unwrap(FixIter, last(args));
            return Box_error_empty();
            // return FixFn_typed_call(FixFnGenerator, ffn, scope, args, *_iter);
        case FN_NATIVE:
            return FixFn_typed_call(FixFnNative, ffn, scope, args);
        case FN_NATIVE_0:
            return FixFn_typed_call(FixFnNative0, ffn, scope);
        case FN_NATIVE_1:
            return FixFn_typed_call(FixFnNative1, ffn, scope, at(args, 0));
        case FN_NATIVE_2:
            return FixFn_typed_call(FixFnNative2, ffn, scope, at(args, 0), at(args, 1));
        case FN_NATIVE_3:
            return FixFn_typed_call(FixFnNative3, ffn, scope, at(args, 0), at(args, 1), at(args, 2));
        default:
            native_return_error_if(true, sMSG("Unknown function type: %d"), ffn->type);
    }
}

FixStruct *FixStruct_new(FixStr name, size_t num_fields) {
    FixStruct *s = Arena_alloc(sizeof(FixStruct));
    if (!s) {
        error_oom();
    }
    s->name = name;
    // s->fields;
    FixDict_data_new(&s->fields, num_fields);
    return s;
}

FlxObject *FlxObject_new(FixStruct *def) {
    FlxObject *obj = gco_new(BXD_FLX_OBJECT);
    if (!obj) {
        error_oom();
    }
    obj->data = def;

    /// @todo consider revising struct to have a FixScope
    /// ... and then we can just copy the struct's fields into the object's data?
    /// ... this would also allow for inheritance from struct to object
    FixScope_sized_new(&obj->data->namespace, nullptr, def->fields.meta.size);
    return obj;
}

FlxObject *FlxObject_from_FixDict(FixStruct *def, FixDict fields) {
    FlxObject *obj = FlxObject_new(def);
    FixDict_merge(&obj->data->namespace.data, &fields);
    return obj;
}

void FlxObject_set_field(FlxObject *obj, FixStr field, Box value) {
    FixDict_set(&obj->data->namespace.data, field, value);
}

bool FlxObject_get_field(FlxObject *obj, FixStr field, Box *out_value) {
    return FixScope_lookup(&obj->data->namespace, field, out_value);
}




int FixArray_test_main(void) {
    FixArray array;
    FixArray_Arena_new_data(&array, 10);
    log_assert(array.data != nullptr, sMSG("Failed to initialize FixArray data"));

    Box val = Box_wrap_int(42);
    Box added = FixArray_overwrite(&array, 0, val);
    log_assert(added.type == UBX_INT, sMSG("FixArray_overwrite did not set type to UBX_INT"));
    log_assert(Box_unwrap_int(added) == 42, sMSG("FixArray_overwrite did not set correct value"));

    Box retrieved = FixArray_get(array, 0);
    log_assert(retrieved.type == UBX_INT, sMSG("FixArray_get did not return UBX_INT"));
    log_assert(Box_unwrap_int(retrieved) == 42, sMSG("FixArray_get returned incorrect value"));

    Box unset = FixArray_weak_unset(&array, 0);
    log_assert(Box_is_null(unset), sMSG("FixArray_weak_unset did not set type to UBX_NULL"));

    return 0;
}

int FixDict_test_main(void) {
    FixDict dict;
    FixDict_data_new(&dict, 10);
    log_assert(dict.data != nullptr, sMSG("Failed to initialize FixDict data"));

    FixStr key = s("key1");
    Box val = Box_wrap_int(100);
    FixDict_set(&dict, key, val);
    Box set_result = FixDict_get(&dict, key);

    log_assert(set_result.type == UBX_INT, sMSG("FixDict_set did not return UBX_INT"));
    log_assert(Box_unwrap_int(set_result) == 100, sMSG("FixDict_set returned incorrect value"));

    FixKvPair found_entry = {0};
    bool found = FixDict_find(&dict, key, &found_entry);
    log_assert(found, sMSG("FixDict_find failed to locate existing key"));
    log_assert(found_entry.value.type == UBX_INT, sMSG("FixDict_find returned incorrect type"));
    log_assert(Box_unwrap_int(found_entry.value) == 100, sMSG("FixDict_find returned incorrect value"));

    // Test finding a non-existent key
    FixStr missing_key = s("missing");
    found = FixDict_find(&dict, missing_key, &found_entry);
    log_assert(!found, sMSG("FixDict_find incorrectly found a non-existent key"));

    return 0;
}


int FixScope_test_main(void) {
    FixScope global_scope = FixScope_empty(s("Global scope"));

    FixScope_data_new(&global_scope, nullptr);
    log_assert(global_scope.data.meta.capacity > 0, sMSG("Failed to create global FixScope"));

    FixStr var1 = s("var1");
    Box val1 = Box_wrap_int(500);
    FixScope_define_local(&global_scope, var1, val1);

    log_assert(Box_unwrap_int(FixDict_bucket_for_key(&global_scope.data, var1)->value) == 500, sMSG("FixScope_define_local failed to set value"));

    FixScope_debug_print(global_scope);

    Box retrieved;
    bool found = FixScope_lookup(&global_scope, var1, &retrieved);

    log_assert(found, sMSG("FixScope_lookup failed to find defined variable"));
    log_assert(retrieved.type == UBX_INT && Box_unwrap_int(retrieved) == 500, sMSG("FixScope_lookup returned incorrect value"));

    // Test nested scope
    FixScope child_scope = FixScope_empty(sMSG("$Scope"));;
    FixScope_data_new(&child_scope, &global_scope);
    log_assert(child_scope.data.meta.capacity > 0, sMSG("Failed to create child FixScope"));

    FixStr var2 = s("var2");
    Box val2 = Box_wrap_bool(true);
    FixScope_define_local(&child_scope, var2, val2);

    found = FixScope_lookup(&child_scope, var1, &retrieved);
    log_assert(found, sMSG("FixScope_lookup failed to find variable from parent scope"));
    log_assert(retrieved.type == UBX_INT && Box_unwrap_int(retrieved) == 500, sMSG("FixScope_lookup returned incorrect value from parent scope"));

    found = FixScope_lookup(&child_scope, var2, &retrieved);
    log_assert(found, sMSG("FixScope_lookup failed to find variable in child scope"));
    // log_assert(Box_is_bool(found) && Box_unwrap_bool(retrieved) == true, sMSG("FixScope_lookup returned incorrect value from child scope"));

    // Test lookup failure
    FixStr missing_var = s("missing_var");
    found = FixScope_lookup(&child_scope, missing_var, &retrieved);
    log_assert(!found, sMSG("FixScope_lookup incorrectly found a non-existent variable"));

    // Cleanup
    // FixScope_aro_free(child_scope);
    // FixScope_aro_free(global_scope);
    return 0;
}



Box FixFn_test_native2(FixFn *fn, FixScope parent, Box one, Box two) {
    log_message(LL_INFO, sMSG("Running test_native2!"));

    /// @note print the function name
    log_message(LL_INFO, sMSG("Function name: %.*s"), fmt(fn->name));

    /// @note print the scope
    FixScope_debug_print(parent);

    /// @note print the types of the two boxes
    FixStr a = Box_to_FixStr(one);
    FixStr b = Box_to_FixStr(two);
    printf("    Box 1: %.*s\n", fmt(a));
    printf("    Box 2: %.*s\n", fmt(b));

    return Box_null();
}

int FixFn_test_main(void) {
    // Create a FixFn representing a user-defined function
    FixStr fn_name = s("test_fn");
    FixDict args;
    FixDict_data_new(&args, 2);
    FixDict_set(&args, s("param1"), Box_null());
    FixDict_set(&args, s("param2"), Box_null());

    FixScope enclosure = FixScope_empty(sMSG("$Scope"));
    FixScope_data_new(&enclosure, nullptr);

    FixFn *fn = FixFn_new(FN_NATIVE, fn_name, args, enclosure, (void *) FixFn_test_native2, nullptr);
    log_assert(fn != nullptr, sMSG("Failed to create FixFn"));

    // Create a FixArray with arguments
    FixArray args_array;
    FixArray_Arena_new_data(&args_array, 2);
    Box arg1 = Box_wrap_int(10);
    Box arg2 = Box_wrap_int(20);
    FixArray_append(&args_array, arg1);
    FixArray_append(&args_array, arg2);
    FixFn_call(fn, enclosure, args_array);

    // Cleanup
    // free(fn);
    // FixScope_aro_free(&enclosure);
    return 0;
}


#pragma endregion

#pragma region BoxHelpersAndErrors

FixArray *CliArgs_to_FixArray(int argc, char **argv) {
    FixArray *args = FixArray_new_auto(argc);
    for (int i = 0; i < argc; i++) {
        Box arg = Box_wrap_BoxedArena(&FixStr_from_cstr(argv[i]));
        FixArray_append(args, arg);
    }
    return args;
}

FixError *error_push(FixError error) {
    ctx().debug.errors[ctx().debug.num_errors++] = error;
    return &ctx().debug.errors[ctx().debug.num_errors - 1];
}



void error_print_lastN(size_t n) {
    for(size_t i = 0; i < n; i++) {
        error_print(ctx().debug.errors[ctx().debug.num_errors - i - 1]);
    }
}

void error_print(FixError error) {
    log_message(LL_INFO,  s("Error Code: %d"), error.code);
    log_message(LL_INFO,  s("Message: %.*s"), fmt(error.message));
    log_message(LL_INFO,  s("Location: %.*s"), fmt(error.location));
    if (error.info.user_file.cstr != nullptr) {
        log_message(LL_INFO,  s("User File: %.*s"), fmt(error.info.user_file));
        log_message(LL_INFO,  s("User Line: %zu"), error.info.user_line);
        log_message(LL_INFO,  s("User Column: %zu"), error.info.user_col);
    }
}

#pragma endregion

#pragma region NativeStdLibMath

void native_stdlib_math_test_main(void) {}

double sample(FixDist *dist) {
    return dist->sample_fn_ptr(dist);
}


/**
 * Initializes a probabilistic model with default or specified parameters.
 *
 * @param model   Pointer to the Model structure representing the probabilistic model.
 * @param method  The inference algorithm to use (e.g., MCMC).
 * @return        Pointer to an FixInferResult structure containing posterior samples.
 */
FixInferResult *FixInferResult_new(const FixInferModel *model, InferMethod method) {
    FixInferResult *result = Arena_alloc(sizeof(FixInferResult));
    if (!result) {  error_oom(); return nullptr; }

    result->samples = Arena_alloc(model->num_target_samples * sizeof(double *));
    result->num_samples = Arena_alloc(model->num_target_samples * sizeof(size_t));

    if (!result->samples) {  error_oom(); return nullptr; }
    if (!result->num_samples) {  error_oom(); return nullptr; }

    for (size_t i = 0; i < model->num_target_samples; i++) {
        result->samples[i] = Arena_alloc(model->num_params * sizeof(double));
        if (!result->samples[i]) {
            error_oom();
        }
    }

    return result;
}


/// @brief Performs inference on the given probabilistic model using the specified inference method.

FixInferResult *FixInferResult_infer(const FixInferModel *model, InferMethod method);

/// @brief Initializes a probabilistic model with default or specified parameters.
FixInferModel *Model_new(FixDist *prior, FixDist *likelihood, size_t num_target_samples);

FixInferModel *Model_set_params(FixInferModel *model, double *params, size_t num_params) {
    model->params = params;
    model->num_params = num_params;
    return model;
}

FixInferModel *Model_set_data(FixInferModel *model, double *data, size_t num_data) {
    model->data = data;
    model->num_data = num_data;
    return model;
}


/**
 * Records an observation within the probabilistic model.
 *
 * @param model
 *      includes pointer to a FixDist struct defining the observation's distribution.
 * @param value The observed value to condition on.
 */
void Model_observe(FixInferModel *model, double value) {
    model->data[model->num_data++] = value;
}

/**
 * Initializes the random number generator with a given seed.
 *
 * @param seed  The seed value for the RNG.
 */
void initialize_rng(unsigned int seed);

double lib_rand_0to1(void) {
    return (double) clib_random_improved();
}

double lib_rand_normal(double mean, double stddev) {
    double u1 = lib_rand_0to1();
    double u2 = lib_rand_0to1();
    double z = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
    return mean + z * stddev;
}

/**
 * Computes the Probability Density Function (PDF) of the Normal distribution.
 *
 * @param x       The value at which to evaluate the PDF.
 * @param mean    The mean (μ) of the Normal distribution.
 * @param stddev  The standard deviation (σ) of the Normal distribution.
 * @return        The PDF value at x for N(mean, stddev).
 */
double normal_pdf(double x, double mean, double stddev);
double normal_log_pdf(double x, double mean, double stddev);

/**
 * Computes the Probability Density Function (PDF) of the Gamma distribution.
 *
 * @param x      The value at which to evaluate the PDF.
 * @param shape  The shape parameter (α) of the Gamma distribution.
 * @param rate   The rate parameter (β) of the Gamma distribution.
 * @return       The PDF value at x for Gamma(shape, rate).
 */
double gamma_pdf(double x, double shape, double rate);
double gamma_log_pdf(double x, double shape, double rate);

/**
 * Samples a value from a Normal (Gaussian) distribution.
 *
 * @param mean    The mean (μ) of the distribution.
 * @param stddev  The standard deviation (σ) of the distribution.
 * @return        A random sample from N(mean, stddev).
 */
double sample_normal(double mean, double stddev);

/**
 * Samples a value from a Gamma distribution.
 *
 * @param shape  The shape parameter (α) of the Gamma distribution.
 * @param rate   The rate parameter (β) of the Gamma distribution.
 * @return       A random sample from Gamma(shape, rate).
 */
double sample_gamma(double shape, double rate);


#pragma endregion

#pragma region BoxedNative



// #define native_srcof(i) ctx().info.sources[i].source_code

#define native_track_src_start() \
    ctx().info.sources[ctx().info.num_sources].name = s(__func__); \
    ctx().info.sources[ctx().info.num_sources].source_file = s(__FILE__); \
    ctx().info.sources[ctx().info.num_sources].start_line_num = __LINE__;

#define native_track_src_end() \
    ctx().info.sources[ctx().info.num_sources].end_line_num = __LINE__; \
    ctx().info.num_sources++;

/// @todo consider the *double problem
///     ... ie maybe have a declare_ for a double arg?
///         ... since it would require a deref


#define declare_arity0_to_native(raw_name, return_packer) \
    Box native_##raw_name(FixFn *self, FixScope parent, Box one, Box two) { \
        native_log_call0(self);\
        return return_packer(raw_name()); \
    }

#define declare_arity1_to_native(raw_name, btype_1st, return_packer) \
    Box native_##raw_name(FixFn *self, FixScope parent, Box one) { \
        native_log_call1(self, one);\
        native_return_error_if(one.type != btype_1st, sMSG("Expected argument type %s"), ubx_nameof(btype_1st)); \
        Box_ctypeof(btype_1st) first;\
        require_safe(clib_memcpy_safe(&first, \
            sizeof(Box_ctypeof(btype_1st)), &one, sizeof(Box_ctypeof(btype_1st))));\
        return return_packer(raw_name(first)); \
    }

#define declare_arity2_to_native(raw_name, btype_1st, btype_2nd, return_packer) \
    Box native_##raw_name(FixFn *self, FixScope parent, Box one, Box two) { \
        native_log_call2(self, one, two);\
        native_return_error_if(one.type != btype_1st, sMSG("Expected argument type %s"), ubx_nameof(btype_1st)); \
        native_return_error_if(two.type != btype_2nd, sMSG("Expected argument type %s"), ubx_nameof(btype_2nd)); \
        Box_ctypeof(btype_1st) first;\
        Box_ctypeof(btype_1st) second;\
        require_safe(clib_memcpy_safe(&first, sizeof(Box_ctypeof(btype_1st)), \
                &one, sizeof(Box_ctypeof(btype_1st))));\
        require_safe(clib_memcpy_safe(&second, sizeof(Box_ctypeof(btype_2nd)), \
                &two, sizeof(Box_ctypeof(btype_2nd))));\
        return return_packer(raw_name(first, second));\
    }


#define declare_arity2_to_native_numeric(raw_name, return_packer) \
    Box native_##raw_name(FixFn *self, FixScope parent, Box one, Box two) { \
        native_log_call2(self, one, two);\
        double first, second;\
        native_return_error_if(!Box_try_numeric(one, &first), \
            sMSG("Expected numeric argument type, got %s"), ubx_nameof(one.type)); \
        native_return_error_if(!Box_try_numeric(two, &second), \
            sMSG("Expected numeric argument type, got %s"), ubx_nameof(two.type)); \
        return return_packer(raw_name(first, second));\
    }


declare_arity1_to_native(sqrtf, UBX_FLOAT, Box_wrap_float);
declare_arity2_to_native_numeric(lib_rand_normal, Box_wrap_float);

Box native_getsrc(FixFn *self, FixScope parent, Box one) {
    native_log_call1(self, one); native_track_src_start();

    FixStr native_fn_name = Box_unwrap_FixStr(one);

    static FixDict native_src_dict;
    static bool is_initialized = false;

    if (!is_initialized) {
        FixDict_data_new(&native_src_dict, ctx().info.num_sources);
        for (size_t i = 0; i < ctx().info.num_sources; i++) {
            FixDict_set(&native_src_dict, ctx().info.sources[i].name, Box_wrap_ptr(&ctx().info.sources[i]));
        }
        is_initialized = true;
    }

    Box entry = FixDict_get(&native_src_dict, native_fn_name);
    if (!Box_is_null(entry)) {
        SourceInfo *src_info = Box_unwrap_typed_ptr(SourceInfo, entry);
        if (FixStr_is_empty(src_info->source_code)) {
            src_info->source_code = FixStr_readlines_new(
                src_info->source_file,
                src_info->start_line_num,
                src_info->end_line_num
            );
        }
        return Box_wrap_BoxedArena(&src_info->source_code);
    }

    native_error(sMSG("Could not find source for native function: %.*s"), fmt(native_fn_name));
    Box result = Box_error_empty();
    native_track_src_end();
    return result;
}


/// @brief
/// @typedef log : fn([]str) -> nullptr
/// @param self
/// @param parent
/// @param args
/// @return
Box native_log(FixFn *self, FixScope parent, FixArray args) {
    native_log_call(self, args);
    native_track_src_start();

    for(size_t i = 0; i < len(args); i++) {
        FixStr_puts(Box_to_FixStr(FixArray_get(args, i)));
        putchar(' ');
    }

    puts("\n");
    Box result = Box_null();

    native_track_src_end();
    return result;
}

/// @brief
/// @example
///     range(3) -> [0, 1, 2, 3]
///     range(1, 3) -> [1, 2, 3]
///     range(0, 4, 2) -> [0, 2, 4]
/// @param self
/// @param parent
/// @param args
/// @return FixArray as Box
Box native_range(FixFn *self, FixScope parent, FixArray args) {
    FixArray *result = nullptr;
    double from = 0, to = 0, step = 1;
    switch(args.meta.size) {
        case 3:
            Box_try_numeric(args.data[0], &from);
            Box_try_numeric(args.data[1], &to);
            Box_try_numeric(args.data[2], &step);
            break;
        case 2:
            Box_try_numeric(args.data[0], &from);
            Box_try_numeric(args.data[1], &to);
            break;
        case 1:
            Box_try_numeric(args.data[0], &to);
            break;
        default:
            native_return_error_if(true,
                sMSG("Incorrect number of args (%zu): `range(start, end, step)`"), len(args));
    }

    size_t _len = 1 + (size_t) fabs(ceil((to - from)/step));

    native_return_error_if(_len == 0, sMSG("Cannot create an empty range"));
    native_return_error_if(_len > UINT64_MAX, sMSG("Cannot create a range of this size: %zu"), _len);
    native_return_error_if(step == 0, sMSG("Cannot create a range with step 0"));
    native_return_error_if(step < 0 && from < to, sMSG("Cannot create a range with negative step and positive start"));
    native_return_error_if(_len > ARRAY_SIZE_SMALL, sMSG("TODO: Move to FlxArray. Range too large: %zu"), _len);

    result = FixArray_new_auto(_len);
    for(size_t i = from; i <= to; i++) {
        FixArray_append(result, Box_wrap_int(i));
    }
    return Box_wrap_BoxedArena(result);
}

/// @brief
/// @example
///     log(infer(model(), #MCMC).take(3))
/// @param self
/// @param parent
/// @param loopFn
/// @param strategyTag
/// @return
Box native_infer(FixFn *self, FixScope parent, Box loopFn, Box strategyTag) {
    native_log_call2(self, loopFn, strategyTag);

    // native_return_error_if(loopFn.type != UBX_PTR_ARENA, sMSG("Expected function argument type, got %s"), ubx_nameof(loopFn.type));
    // native_return_error_if(strategyTag.type != UBX_INT, sMSG("Expected int argument type, got %s"), ubx_nameof(strategyTag.type));

    if(Box_tag_eq(strategyTag, s("#MCMC"))) {
        printf("MCMC\n");
        return Box_wrap_int(2);
    } else if(Box_tag_eq(strategyTag, s("#HMC"))) {
        printf("HMC\n");
        return Box_wrap_int(1);
    } else {
        native_error(sMSG("Unknown inference strategy: %.*s"), fmt(Box_to_FixStr(strategyTag)));
        return Box_null();
    }
}

/// @brief
/// @example
///     let
///         m = sample(normal(0, 2))
///     in
///         observe(normal(f(10, m, c), sigma));
///
Box native_sample(FixFn *self, FixScope parent, Box distObject) {
    native_log_call1(self, distObject);

    if(distObject.type == UBX_FLOAT) {
        return distObject;
    }

    // native_return_error_if(distObject.type != UBX_PTR_HEAP,
    //     sMSG("Expected object argument type, got %s"), ubx_nameof(distObject.type));

    Box fn = FlxObject_getattr_checked(
        Box_unwrap_FixObject(distObject), s("sample"), UBX_PTR_ARENA
    );

    native_return_error_if(Box_is_error(fn),
        sMSG("Expected object to have a `sample` method"));

    return FixFn_call(Box_unwrap_FixFn(fn), parent, (FixArray) {
        .meta.capacity = 1,
        .meta.size = 1,
        .data = &distObject
    });
}

Box native_take(FixFn *self, FixScope parent, FixArray args) {
    native_log_call(self, args);

    FixArray *result = FixArray_new_auto(ARRAY_SIZE_SMALL);

    // @todo implement take
    FixArray_append(result, Box_wrap_float(1.1));
    FixArray_append(result, Box_wrap_float(2.2));
    FixArray_append(result, Box_wrap_float(3.3));
    FixArray_append(result, Box_wrap_float(4.4));

    return Box_wrap_BoxedArena(result);
}


Box native_input(FixFn *self, FixScope parent, FixArray args) {
    native_log_call(self, args);

    require_not_null(self);
    require_not_null(parent.data.data);
    require_not_null(args.data);

    native_return_error_if(args.meta.size != 1, sMSG("Expected 1 argument, got %zu"), len(args));
    native_return_error_if(args.data[0].type != UBX_PTR_ARENA, sMSG("Expected string argument type, got %s"), ubx_nameof(args.data[0].type));

    FixStr prompt = Box_unwrap_FixStr(args.data[0]);
    FixStr_puts(prompt);

    return Box_wrap_BoxedHeap(FixStr_flexible_new(FixStr_readline_new(1024)));
}

Box native_len(FixFn *self, FixScope parent, Box arg) {
    native_log_call1(self, arg);

    if(Box_is_Boxed(arg)) {
        return Box_wrap_int(Box_Boxed_meta(arg).size);
    }

    native_error(sMSG("Expected array or string argument type, got %s"), ubx_nameof(arg.type));
    return Box_null();
}

Box native_sum(FixFn *self, FixScope parent, FixArray args) {
    native_log_call(self, args);

    double sum = 0;
    for(size_t i = 0; i < len(args); i++) {
        native_return_error_if(args.data[i].type != UBX_FLOAT, sMSG("Expected float argument type, got %s"), ubx_nameof(args.data[i].type));
        sum += Box_unwrap_float(args.data[i]);
    }

    return Box_wrap_float(sum);
}

Box native_sum_FlxVecInt(FixFn *self, FixScope parent, Box vec) {
    native_log_call1(self, vec);
    FlxVecInt *a = Box_unwrap_typed_ptr(FlxVecInt, vec);

    native_return_error_if(a == nullptr, sMSG("Expected non-nullptr FlxVecInt argument"));
    native_return_error_if(Box_is_Boxed(vec), sMSG("Expected collection argument type, got %s"), ubx_nameof(vec.type));
    native_return_error_if(Box_is_Boxed_type(vec, BXD_FLX_VEC_INT), sMSG("Expected FlxVecInt argument type, got %s"), bt_nameof(a->meta.type));
    // native_return_error_if(len_ref(a) == 0, sMSG("Cannot sum an empty array"));

    int sum = 0;
    for(size_t i = 0; i < len_ref(a); i++) {
        sum += at_ref(a, i);
    }

    return Box_wrap_int(sum);
}

Box native_sum_FlxVecDouble(FixFn *self, FixScope parent, Box vec) {
    native_log_call1(self, vec);
    FlxVecDouble *a = Box_unwrap_typed_ptr(FlxVecDouble, vec);

    native_return_error_if(Box_is_Boxed(vec), sMSG("Expected collection argument type, got %s"), ubx_nameof(vec.type));
    native_return_error_if(a == nullptr, sMSG("Expected non-nullptr FlxVecFloat argument"));
    native_return_error_if(Box_is_Boxed_type(vec, BXD_FLX_VEC_DOUBLE_N), sMSG("Expected FlxVecFloat argument type, got %s"), bt_nameof(a->meta.type));
    native_return_error_if(len_ref(a) == 0, sMSG("Cannot sum an empty array"));

    double sum = 0;
    for(size_t i = 0; i < len_ref(a); i++) {
        sum += at_ref(a, i);
    }

    return Box_wrap_float(sum);
}

Box native_max(FixFn *self, FixScope parent, FixArray args) {
    native_log_call(self, args);

    double max = -INFINITY;
    for(size_t i = 0; i < len(args); i++) {
        native_return_error_if(args.data[i].type != UBX_FLOAT, sMSG("Expected float argument type, got %s"), ubx_nameof(args.data[i].type));
        max = fmax(max, Box_unwrap_float(args.data[i]));
    }

    return Box_wrap_float(max);
}

// Box native_min(FixFn *self, FixScope parent, FixArray args);
// Box native_abs(FixFn *self, FixScope parent, FixArray args);
// Box native_pow(FixFn *self, FixScope parent, FixArray args);
// Box native_sqrt(FixFn *self, FixScope parent, FixArray args);
// Box native_floor(FixFn *self, FixScope parent, FixArray args);
// Box native_ceil(FixFn *self, FixScope parent, FixArray args);
// Box native_round(FixFn *self, FixScope parent, FixArray args);
Box native_log10(FixFn *self, FixScope parent, FixArray args);
Box native_logB(FixFn *self, FixScope parent, FixArray args);
Box native_logE(FixFn *self, FixScope parent, FixArray args);
Box native_exp(FixFn *self, FixScope parent, FixArray args);
Box native_sin(FixFn *self, FixScope parent, FixArray args);
Box native_cos(FixFn *self, FixScope parent, FixArray args);
Box native_tan(FixFn *self, FixScope parent, FixArray args);
Box native_asin(FixFn *self, FixScope parent, FixArray args);
Box native_acos(FixFn *self, FixScope parent, FixArray args);
Box native_atan(FixFn *self, FixScope parent, FixArray args);
Box native_atan2(FixFn *self, FixScope parent, FixArray args);
Box native_sinh(FixFn *self, FixScope parent, FixArray args);
Box native_cosh(FixFn *self, FixScope parent, FixArray args);
Box native_tanh(FixFn *self, FixScope parent, FixArray args);
Box native_asinh(FixFn *self, FixScope parent, FixArray args);
Box native_acosh(FixFn *self, FixScope parent, FixArray args);
Box native_atanh(FixFn *self, FixScope parent, FixArray args);
Box native_rand(FixFn *self, FixScope parent, FixArray args);
Box native_random_int(FixFn *self, FixScope parent, FixArray args);
Box native_random_range(FixFn *self, FixScope parent, FixArray args);
Box native_random_choice(FixFn *self, FixScope parent, FixArray args);
Box native_random_shuffle(FixFn *self, FixScope parent, FixArray args);
Box native_random_sample(FixFn *self, FixScope parent, FixArray args);
Box native_random_seed(FixFn *self, FixScope parent, FixArray args);
Box native_random_normal(FixFn *self, FixScope parent, FixArray args);
Box native_random_int_normal(FixFn *self, FixScope parent, FixArray args);
Box native_random_gamma(FixFn *self, FixScope parent, FixArray args);
Box native_random_beta(FixFn *self, FixScope parent, FixArray args);
Box native_random_poisson(FixFn *self, FixScope parent, FixArray args);
Box native_random_binomial(FixFn *self, FixScope parent, FixArray args);
Box native_random_bernoulli(FixFn *self, FixScope parent, FixArray args);
Box native_random_exponential(FixFn *self, FixScope parent, FixArray args);
Box native_random_uniform(FixFn *self, FixScope parent, FixArray args);
Box native_random_beta_binomial(FixFn *self, FixScope parent, FixArray args);
Box native_random_dirichlet(FixFn *self, FixScope parent, FixArray args);
Box native_random_gamma_poisson(FixFn *self, FixScope parent, FixArray args);
Box native_random_inv_gamma(FixFn *self, FixScope parent, FixArray args);
Box native_random_laplace(FixFn *self, FixScope parent, FixArray args);
Box native_random_log_normal(FixFn *self, FixScope parent, FixArray args);
Box native_random_neg_binomial(FixFn *self, FixScope parent, FixArray args);
Box native_random_pareto(FixFn *self, FixScope parent, FixArray args);
Box native_random_triangular(FixFn *self, FixScope parent, FixArray args);
Box native_random_uniform_int(FixFn *self, FixScope parent, FixArray args);
Box native_random_weibull(FixFn *self, FixScope parent, FixArray args);
Box native_random_boltzmann(FixFn *self, FixScope parent, FixArray args);
Box native_random_gev(FixFn *self, FixScope parent, FixArray args);
Box native_random_gumbel(FixFn *self, FixScope parent, FixArray args);
Box native_random_rayleigh(FixFn *self, FixScope parent, FixArray args);
Box native_random_wald(FixFn *self, FixScope parent, FixArray args);
Box native_random_maxwell(FixFn *self, FixScope parent, FixArray args);
Box native_random_power_law(FixFn *self, FixScope parent, FixArray args);






void native_add_prelude(FixScope scope) {
    static FixFn prelude[] = {
        FixFnFromNative(FN_NATIVE, s("log"), native_log),
        FixFnFromNative(FN_NATIVE, s("observe"), native_log),
        FixFnFromNative(FN_NATIVE, s("range"), native_range),
        FixFnFromNative(FN_NATIVE_1, s("sqrt"), native_sqrtf),
        FixFnFromNative(FN_NATIVE_2, s("infer"), native_infer),
        FixFnFromNative(FN_NATIVE_1, s("sample"), native_sample),
        FixFnFromNative(FN_NATIVE, s("take"), native_take),
        FixFnFromNative(FN_NATIVE_2, s("normal"), native_lib_rand_normal),
        FixFnFromNative(FN_NATIVE_2, s("gamma"), native_lib_rand_normal)
    };

    size_t n = sizeof(prelude) / sizeof(FixFn);
    for(size_t i = 0; i < n; i++) {
        FixScope_define_local(&scope, prelude[i].name, Box_wrap_BoxedArena(&prelude[i]));
    }
}


#pragma endregion

//#endregion
///---------- ---------- ---------- IMPL: INTERPRETER ---------- ---------- ------ ///
//#region Implementation_Interpreter


#pragma region InterpErrorImpl

#define interp_return_if_error(value) \
    if (Box_is_error(value)) {\
        return Box_exit();\
    }

#pragma endregion

#pragma region BoxedPrecacheImpl

void interp_init(void) {

    #ifdef INTERP_PRECACHE_INTERNED_STRINGS
        /// @todo consider this interning approach, and this size
        /// ... empirically how many vars / prog? how useful is it to intern?
        /// @todo presumably this should be a vector of strings, not a FixDict
        FixDict_data_new(&interp_precache().str_precache, ARRAY_SIZE_MEDIUM);
    #endif
}

/// @todo implement this

/// @brief
/// @param str
/// @param out
/// @return
bool interp_precache_getstr(FixStr str, FixKvPair *out) {
    return true;
}

void interp_precache_setstr(FixStr str) {
    return;
}

void interp_precache_test_main(void) {
    #ifndef INTERP_PRECACHE_INTERNED_STRINGS
        log_message(LL_INFO, sMSG("Skipping interned strings test"));
        return;
    #endif

    // Initialize the interpreter pre-cache
    interp_init();
    log_assert(FixStr_is_empty(interp_precache().str_precache[0]), sMSG("Interned strings should be empty"));

    // Test setting and getting a string
    FixStr test_str = s("test_string");
    interp_precache_setstr(test_str);

    FixKvPair entry;
    bool found = interp_precache_getstr(test_str, &entry);
    log_assert(found, sMSG("Failed to find interned string"));
    log_assert(Box_is_null(entry.value), sMSG("Interned string value should be nullptr"));

    // Test retrieving a non-existent string
    FixStr non_existent = s("non_existent");
    found = interp_precache_getstr(non_existent, &entry);
    log_assert(!found, sMSG("Non-existent string should not be found"));
}

#pragma endregion

#pragma region InterpEvalImpl

#define interp_trace() \
    ctx_trace_me(FixStr_firstN(Ast_to_FixStr(node), 20));

#define interp_print_traces() Tracer_print_stack(ctx().debug.callstack, 16);


 ///@todo
#define interp_require_type(node_type)\
    if (node->type != node_type) {\
        assert(false);\
        return Box_exit();\
    }


static inline __attribute__((always_inline))
Box interp_eval_FixAst(Ast *node, FixScope *scope) {
    switch(node->type) {
        case AST_DISCARD:
            return Box_null();
        case AST_IF:
            return interp_eval_if(node, scope);
        case AST_DICT:
            return interp_eval_dict(node, scope);
        case AST_VEC:
            return interp_eval_vec(node, scope);
        case AST_FN_DEF_CALL:
            return interp_eval_call_fn(node, scope);
        case AST_METHOD_CALL:
            return interp_eval_method_call(node, scope);
        case AST_MEMBER_ACCESS:
            return interp_eval_member_access(node, scope);
        case AST_TAG: case AST_INT: case AST_FLOAT: case AST_DOUBLE: case AST_STR:
            return interp_eval_literal(node, scope);
        case AST_ID:
            return interp_eval_identifier(node, scope);
        case AST_MOD:
            return interp_eval_module(node, scope);
        case AST_BLOCK:
            return interp_eval_block(node, scope);
        case AST_FN_DEF:
            return interp_eval_fn_def(node, scope);
        case AST_LOOP:
            return interp_eval_loop(node, scope);
        case AST_FOR:
            return interp_eval_for(node, scope);
        case AST_EXPRESSION:
            return interp_eval_expression(node, scope);
        case AST_FN_DEF_ANON:
            return interp_eval_fn_anon(node, scope);
        case AST_CONST_DEF:
            return interp_eval_const_def(node, scope);
        case AST_LEF_DEF:
            return interp_eval_let_def(node, scope);
        case AST_BOP:
            return interp_eval_binary_op(node, scope);
        case AST_UOP:
            return interp_eval_unary_op(node, scope);
        case AST_USE:
            return interp_eval_use(node, scope);
        case AST_STRUCT_DEF:
            return interp_eval_struct_def(node, scope);
        case AST_OBJECT_LITERAL:
            return interp_eval_object_literal(node, scope);
        case AST_MATCH:
            return interp_eval_match(node, scope);
        case AST_RETURN:
            return interp_eval_return(node, scope);
        case AST_MUTATION:
            return interp_eval_mutation(node, scope);
        default:
            if((node->type > 0) && (node->type < AST_ENUM_SIZE)) {
                interp_error(sMSG("Unknown AST node type (%.*s)"), fmt(Ast_nameof(node->type)));
            } else {
                interp_error(sMSG("Unknown AST node type (%d)"), node->type);
            }
            return Box_exit();
    }
}


Box interp_eval_ast(Ast *node, FixScope *scope) {
    require_not_null(node); require_not_null(scope);
    require_positive(node->type);
    Box result = interp_eval_FixAst(node, scope);

    //|| Box_is_halt(result)
    if(Box_is_error(result)) {
        interp_graceful_exit();
    }

    return result;
}


void interp_graceful_exit(void) {
    /// @note print the stack trace
    log_message(LL_RECOVERABLE, sMSG("Interpreter Error"));
    interp_print_traces();
    error_print_all();

    /// @todo -- this should not be an abort
    /// ... maybe a longjmp or something else
    abort();
    // exit(1);
}



// Implementation of interp_run_from_source
Box interp_run_from_source(ParseContext *p, FixStr source, int argc, char **argv) {
    lex_source(p, source, s("    "));
    FixStr_puts(data_to_ansi_new(p->lexer));

    #ifdef INTERP_SHOW_PARSING
        lex_print_all(p->lexer);
    #endif

    parse(p);
    require_not_null(p->parser.data);

    #ifdef INTERP_SHOW_PARSING
        Ast_print(p->parser.data);
    #endif

    log_assert(p->parser.data->type == AST_BLOCK, sMSG("Top-level node must be a block"));

    /// @todo revise the FixScope new'ing to ensure data are initialized
    FixScope globals = FixScope_empty(s("Global scope"));
    FixScope_data_new(&globals, nullptr);
    native_add_prelude(globals);

    Box result = interp_eval_ast(p->parser.data, &globals);
    interp_return_if_error(result);

    /// @note we find and run the main function
    // FixArray *main_args = CliArgs_to_FixArray(argc, argv);
    FixArray main_args = {0}; //CliArgs_to_FixArray(argc, argv);
    Box main_fn;
    if(FixScope_lookup(&globals, s("main"), &main_fn)) {
        return FixFn_call(Box_unwrap_FixFn(main_fn), globals, main_args);
    } else {
        FixScope_debug_print(globals);
        interp_error(sMSG("No main function found"));
        return Box_exit();
    }
}

Box FixFn_interp_user_fn(FixFn *fn, FixScope function_scope, FixArray args) {
    require_not_null(fn);
    log_assert(fn->type == FN_USER, sMSG("Function must be user-defined!"));

    Ast *body = (Ast *) fn->code;
    require_not_null(body);

    size_t i = 0;
    FixDict_iter_items(fn->signature, arg) {
        Box value = at(args, i++);
        if(Box_is_null(value)) {
            value = arg->value;
        }
        FixScope_define_local(&function_scope, arg->key, value);
    }

    return interp_eval_ast(body, &function_scope);
}



/// @brief
/// interp_eval_call_generator:
///     Creates a new FixIter to keep track of the generator's state.
///     Initializes the iterator with the original function and other necessary information.
///     Creates a new generator function that wraps the original function and the iterator.
///     Appends the iterator to the arguments.
///     Returns the new generator function.
/// FixFn_interp_generator_fn:
///     Extracts the iterator from the last argument.
///     Calls the original function.
///     Checks the result:
///     If it's a yield, updates the iterator state and returns the yielded value.
///     If it's done, returns a "done" signal.
///     If it's unexpected, returns an error.
/// @example
///      loop fn example_generator() :=
///         for(i <- range(3)) yield i
/// @param fn
/// @param function_scope
/// @param args
/// @return
Box interp_eval_call_generator(FixFn *fn, FixScope function_scope, FixArray args) {
    require_not_null(fn);
    require_not_null(fn->fnptr);
    require(fn->type == FN_LOOP);
    return Box_error_empty();

    // FixIter *iter = FixIter_new(fn, args);
    // FixFn *gen_fn = FixFn_new(FN_GENERATOR, fn->name, fn->signature, function_scope,
    //     (void *) FixFn_interp_generator_fn, (void *) iter
    // );

    // FixArray_append(&args, Box_wrap_BoxedArena(iter));
    // return Box_wrap_BoxedArena(gen_fn);
}

Box FixFn_interp_generator_fn(FixFn *fn, FixScope function_scope, FixArray args) {
    require_not_null(fn);
    require_not_null(fn->fnptr);
    require(fn->type == FN_GENERATOR);
    return Box_error_empty();

    // FixIter *iter = Box_unwrap_FixIter(last(args));
    // Box result = FixFn_call(fn, function_scope, args);

    // if(Box_is_state_end(result)) {
    //     return Box_null();
    // } else if(Box_is_state_yield(result)) {
    //     FixIter_update(iter, result);
    //     return FixIter_get(iter);
    // } else {
    //     interp_error(sMSG("Unexpected generator state"));
    //     return Box_exit();
    // }
}

Box interp_eval_if(Ast *node, FixScope *scope) {
    require_not_null(node); require_not_null(scope); interp_trace();
    interp_require_type(AST_IF);

    Ast *condition = node->if_stmt.condition;
    Box cond_val = interp_eval_ast(condition, scope);

    // Assuming Box has a method to evaluate its truthiness
    bool cond_truth = Box_is_truthy(cond_val);

    if (cond_truth) {
        return interp_eval_ast(node->if_stmt.body, scope);
    } else if (node->if_stmt.else_body) {
        return interp_eval_ast(node->if_stmt.else_body, scope);
    } else {
        return Box_null();
    }
}



/// @brief
/// @todo incomplete -- check start, end, step types
/// @param node
/// @param scope
/// @return
Box interp_eval_range_sugar(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_RANGE_SUGAR);

    Box start = interp_eval_ast(node->range_sugar.start, scope);
    Box end = interp_eval_ast(node->range_sugar.end, scope);
    Box step = interp_eval_ast(node->range_sugar.step, scope);

    interp_return_if_error(start);
    interp_return_if_error(end);
    interp_return_if_error(step);

    // if(start.type != UBX_INT) {
    //     interp_error(sMSG("Expected integer start value, got %s"), ubx_nameof(start.type));
    //     return Box_exit();
    // }


    /// @note construct call to native range
    FixArray *args = FixArray_new_auto(3);
    FixArray_append(args, start);
    FixArray_append(args, end);
    FixArray_append(args, step);

    Box range_fn;
    if(FixScope_lookup(scope, s("range"), &range_fn)) {
        return FixFn_call(Box_unwrap_FixFn(range_fn), *scope, *args);
    } else {
        interp_error(sMSG("Range function not found"));
        return Box_exit();
    }
}


Box interp_eval_dict(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_DICT);

    FlxDict *dict = FlxDict_new(node->dict.size);
    for (size_t i = 0; i < node->dict.size; i++) {
        struct AstDictEntry *entry = &node->dict.data[i];

        Box key = interp_eval_ast(entry->key, scope);
        Box val = interp_eval_ast(entry->value, scope);
        FlxDict_set(dict, key, val);
    }

    return Box_wrap_BoxedArena(dict);
}


Box interp_eval_block(Ast *node, FixScope *scope) {

    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_BLOCK);

    FixScope *block_scope;
    FixScope block_local = FixScope_empty(sMSG("$Scope"));

    /// @note if the block is transparent, we use the parent scope
    /// ... otherwise we create a new scope with the parent
    /// ... transparent blocks are used for the global program scope
    /// ... (and maybe other things, eg., modules, some loops, etc.)
    /// ... non-transparent blocks are used for functions and other scopes

    if(node->block.transparent) {
        block_scope = scope;
    } else {
        FixScope_data_new(&block_local, scope);
        block_scope = &block_local;
    }


    Box result = Box_null();
    Box last_result = Box_null();

    for (size_t i = 0; i < node->block.num_statements; i++) {
        Ast *stmt = node->block.statements[i];
        result = interp_eval_ast(stmt, block_scope);
        interp_log_debug(sMSG("Block statement result: %.*s"), fmt(Box_to_FixStr(result)));

        if(Box_is_state_end(result)) {
            return last_result;
        }
        last_result = result;
    }

    return result;
}


Box interp_eval_fn_def(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_FN_DEF);

    FixDict args;
    FixDict_data_new(&args, ARRAY_SIZE_SMALL);

    for (size_t i = 0; i < node->fn.num_params; i++) {
        Box value = (node->fn.params->default_value != nullptr) ?
            interp_eval_ast(node->fn.params->default_value, scope) :
            Box_null();

        FixDict_set(&args, node->fn.params[i].name, value);
    }

    FixFnType fnt = node->fn.is_generator ? FN_GENERATOR : FN_USER;

    FixFn *fn = FixFn_new(fnt, node->fn.name, args, *scope,
        (void *) FixFn_interp_user_fn, (void*) node->fn.body
    );

    FixScope_define_local(scope, node->fn.name, Box_wrap_BoxedArena(fn));
    require_scope_has(scope, node->fn.name);

    interp_log_debug(sMSG("Defined function '%.*s'"), fmt(node->fn.name));

    /// @todo is this correct?
    return Box_wrap_BoxedArena(fn);
}


Box interp_eval_fn_anon(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_FN_DEF_ANON);

    FixDict args;
    FixDict_data_new(&args, ARRAY_SIZE_SMALL);

    for (size_t i = 0; i < node->fn_anon.num_params; i++) {
        Box value = (node->fn_anon.params->default_value != nullptr) ?
            interp_eval_ast(node->fn_anon.params->default_value, scope) :
            Box_null();

        FixDict_set(&args, node->fn_anon.params[i].name, value);
    }

    FixFn *fn = FixFn_new(FN_USER, s("$anon"), args, *scope, (void *) FixFn_interp_user_fn, (void*) node->fn_anon.body);
    return Box_wrap_BoxedArena(fn);
}


Box interp_eval_member_access(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_MEMBER_ACCESS);

    Box object = interp_eval_ast(node->member_access.target, scope);
    interp_return_if_error(object);

    if (!Box_is_object(object)) {
        interp_error(sMSG("Attempted to access member on non-object type: %s"), ubx_nameof(object.type));
        return Box_exit();
    }

    FixStr member_name = node->member_access.property;

    Box member = FlxObject_getattr(Box_unwrap_FixObject(object), member_name);
    return member;
}

Box interp_eval_method_call(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_METHOD_CALL);

    Box object = interp_eval_ast(node->method_call.target, scope);
    // Box_print(object);

    interp_return_if_error(object);

    FixStr method_name = node->method_call.method;
    Box method = FlxObject_find_method(object, scope, method_name);
    interp_return_if_error(method);

    size_t total_args = node->method_call.num_args + 1;
    FixArray *args = FixArray_new_auto(total_args);

    FixArray_append(args, object);
    for (size_t i = 0; i < node->method_call.num_args; i++) {
        Box arg = interp_eval_ast(node->method_call.args[i], scope);
        interp_return_if_error(arg);
        FixArray_append(args, arg);
    }

    return FixFn_call(Box_unwrap_FixFn(method), Box_unwrap_FixFn(method)->enclosure, *args);
}




Box interp_eval_module(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_MOD);
    Box_return_noimpl();
}

Box interp_eval_literal(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();

    switch(node->type) {
        case AST_INT:
            /// @todo this will need atoi or similar
            return Box_wrap_int(node->integer.value);
        case AST_FLOAT:
        case AST_DOUBLE: // @todo proper double handling -- AST_FLOAT, TT_FLOAT aren't impl
            return Box_wrap_float((float) node->dble.value);
        case AST_STR:
            return Box_wrap_BoxedArena(&node->str.value);
        case AST_TAG:
            return Box_wrap_tag(node->tag.name);
        default:
            interp_error(sMSG("interp_eval_literal called with invalid node type: %d"), node->type);
            return Box_exit();
    }
}


/// @brief `loop` repeats a block of code without a result
///         `loop if` is equivalent to `while`
///         `loop` without a condition iterates over streams
///             The `for` keyword yields a value. See `for` for more information.
/// @param node
/// @param scope
/// @return
/// @example
///     loop if x < 10 in
///         x = x + 1
///         log(x)
///
///     Short form:
///         loop(x <- range(1, 10), y <- range(3), z = x*y) log(z)
///
///     loop
///         x <- range(1, 10)
///         y <- range(3)
///         z = x*y
///     in
///         log(z)
Box interp_eval_loop(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_LOOP);

    FixScope loop_scope = FixScope_empty(sMSG("$Scope"));
    FixScope_data_new(&loop_scope, scope);

    if(node->loop.condition) {
        return interp_eval_loop_with_condition(node, &loop_scope);
    } else if(node->loop.bindings) {
        return interp_eval_loop_with_streams(node, &loop_scope);
    } else {
        return interp_eval_loop_with_infinite(node, &loop_scope);
    }
}

Box interp_eval_loop_with_condition(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_LOOP);

    Ast *condition = node->loop.condition;
    Ast *body = node->loop.body;

    Box result = Box_null();
    while(true) {
        Box cond_val = interp_eval_ast(condition, scope);
        interp_return_if_error(cond_val);

        if(!Box_is_truthy(cond_val)) {
            break;
        }

        result = interp_eval_ast(body, scope);
        if(Box_is_state_end(result)) {
            return result;
        }
    }

    return result;
}
Box interp_eval_loop_with_streams(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_LOOP);

    Ast **bindings = node->loop.bindings;
    size_t num_bindings = node->loop.num_bindings;
    Ast *body = node->loop.body;

    // Create a new iterator scope
    FixScope iter_scope = FixScope_empty(sMSG("$Scope"));
    FixScope_data_new(&iter_scope, scope);

    // Iterate over bindings
    for(size_t i = 0; i < num_bindings; i++) {
        Ast *binding = bindings[i];
        // Assuming binding is an AST_BINDING node
        if(binding->type != AST_BINDING) {
            interp_error(sMSG("Expected binding in loop statement."));
            return Box_exit();
        }

        FixStr var_name = binding->binding.identifier;
        Box expr_val = interp_eval_ast(binding->binding.expression, scope);
        interp_return_if_error(expr_val);
        FixScope_define_local(&iter_scope, var_name, expr_val);
    }

    // Execute the loop body
    Box result = interp_eval_ast(body, &iter_scope);

    return result;
    // Cleanup
}

Box interp_eval_loop_with_infinite(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_LOOP);

    Ast *body = node->loop.body;

    Box result = Box_null();
    while(true) {
        result = interp_eval_ast(body, scope);
        if(Box_is_state_end(result)) {
            return result;
        }

    }

    return result;
}



/// @brief
/// @param node
/// @param scope
/// @example
///    for(x <- range(1, 10)) x
///    for(x <- range(1, 10)) yield x
///    for(x <- range(1, 10), y <- range(3)) x * y
///    for(x <- range(1, 10), y <- range(3), z = x*y) z
///    for
///         x <- range(1, 10)
///         y <- range(3)
///         z = x*y
///     in
///         z
///    for
///         x <- range(1, 10)
///         y <- range(3)
///         z = x*y
///     in yield z
/// @return

Box interp_eval_for(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_FOR);

    // Extract bindings and body
    Ast **bindings = node->loop.bindings;
    size_t num_bindings = node->loop.num_bindings;
    Ast *body = node->loop.body;

    // Create a new iterator scope
    FixScope iter_scope = FixScope_empty(sMSG("$Scope"));
    FixScope_data_new(&iter_scope, scope);

    // Iterate over bindings
    for(size_t i = 0; i < num_bindings; i++) {
        Ast *binding = bindings[i];
        // Assuming binding is an AST_BINDING node
        if(binding->type != AST_BINDING) {
            interp_error(sMSG("Expected binding in for-loop."));
            return Box_exit();
        }

        FixStr var_name = binding->binding.identifier;
        Box expr_val = interp_eval_ast(binding->binding.expression, scope);
        interp_return_if_error(expr_val);
        FixScope_define_local(&iter_scope, var_name, expr_val);
    }

    // Execute the loop body
    Box result = interp_eval_ast(body, &iter_scope);

    // Cleanup
    // TODO: Implement scope cleanup if necessary

    return result;
}


Box interp_eval_expression(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_EXPRESSION);

    return interp_eval_ast(node->exp_stmt.expression, scope);
}

Box interp_eval_const_def(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_CONST_DEF);

    FixStr var_name = node->const_stmt.name;
    Ast *expr = node->const_stmt.value;

    Box value = interp_eval_ast(expr, scope);

    interp_return_if_error(value);

    FixScope_define_local(scope, var_name, value);

    interp_log_debug(sMSG("Defined constant '%.*s' with value '%.*s'"), fmt(var_name), fmt(Box_to_FixStr(value)));

    return Box_done();
}


Box interp_eval_vec(Ast *node, FixScope *scope) {
    require_not_null(node); require_not_null(scope); interp_trace();
    interp_require_type(AST_VEC);

    Ast **data = node->vec.data;
    size_t size = node->vec.size;

    FixArray *array = FixArray_new_auto(size);
    for(size_t i = 0; i < size; i++) {
        Box element = interp_eval_ast(data[i], scope);
        interp_return_if_error(element);
        FixArray_append(array, element);
    }

    return Box_wrap_BoxedHeap(array);
}


Box interp_eval_let_def(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_LEF_DEF);

    Ast **bindings = node->let_stmt.bindings;
    size_t num_bindings = node->let_stmt.num_bindings;
    Ast *body = node->let_stmt.body;

    // Evaluate and define each binding
    for(size_t i = 0; i < num_bindings; i++) {
        Ast *binding = bindings[i];
        if(binding->type != AST_BINDING) {
            interp_error(sMSG("Expected binding in let statement."));
            return Box_exit();
        }

        FixStr var_name = binding->binding.identifier;
        Box expr_val = interp_eval_ast(binding->binding.expression, scope);
        interp_return_if_error(expr_val);
        FixScope_define_local(scope, var_name, expr_val);
    }

    // Evaluate the body
    return interp_eval_ast(body, scope);
}

Box interp_eval_binary_op(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_BOP);

    Box left = interp_eval_ast(node->bop.left, scope);
    interp_return_if_error(left);
    Box right = interp_eval_ast(node->bop.right, scope);
    interp_return_if_error(right);

    return interp_eval_bop(node->bop.op, left, right);
}

Box interp_eval_unary_op(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_UOP);

    Box operand = interp_eval_ast(node->uop.operand, scope);
    interp_return_if_error(operand);

    return interp_eval_uop(node->uop.op, operand);
}

Box interp_eval_match(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_MATCH);

    Box expr_val = interp_eval_ast(node->match.expression, scope);
    interp_return_if_error(expr_val);

    for(size_t i = 0; i < node->match.num_cases; i++) {
        Ast *case_node = node->match.cases[i].condition;

        if(case_node) {
            Box test_val = interp_eval_ast(case_node, scope);
            interp_return_if_error(test_val);

            if(Box_eq(expr_val, test_val)) {
                return interp_eval_ast(node->match.cases[i].expression, scope);
            }
        } else {
            /// @todo -- the else branch
            return Box_null();
        }
    }

    interp_error(sMSG("No matching case found in match statement."));
    return Box_exit();
}


/// @brief handles the `return` keyword which can be placed anywhere in a block
///             to short-circuit the block and return a value
/// @example
///    let(x = 10) in return x
///    let
///        x = 10
///        y = 20
///    in
///        return x + y
///        x * y // allowed but unreachable
/// @param node
/// @param scope
/// @return
Box interp_eval_return(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_RETURN);

    Box return_val = interp_eval_ast(node->return_stmt.value, scope);
    interp_return_if_error(return_val);


    /// @todo check if this implementation is correct
    return return_val;
}

Box interp_eval_mutation(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_MUTATION);

    // Evaluate target and value
    Box target = interp_eval_ast(node->mutation.target, scope);
    interp_return_if_error(target);
    Box value = interp_eval_ast(node->mutation.value, scope);
    interp_return_if_error(value);

    /// @todo perform the mutation
    ///     nb. we have broadcast/vector'd mutations and ordinary updates

    // if(target.type != UBX_PTR) {
    //     interp_error(sMSG("Mutation target is not a mutable reference."));
    //     return Box_exit();
    // }

    Box *target_ptr = Box_unwrap_typed_ptr(Box, target);
    *target_ptr = value;

    return value;
}



Box interp_eval_identifier(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_ID);

    Box out;
    if(FixScope_lookup(scope, node->id.name, &out)) {
        return out;
    } else {
        interp_error(sMSG("Undefined identifier: %.*s"), fmt(node->id.name));
        return Box_exit();
    }
}


Box interp_eval_call_fn(Ast *node, FixScope *scope) {
    require_not_null(node); require_not_null(scope); interp_trace();
    interp_require_type(AST_FN_DEF_CALL);

    Box callee_fn = interp_eval_ast(node->call.callee, scope);
    interp_return_if_error(callee_fn);

    if(callee_fn.type != UBX_PTR_ARENA) {
        interp_error(sMSG("Not a function, type is: %.*s"), fmt(ubx_nameof(callee_fn.type)));
        return Box_exit();
    }

    FixFn *fn = Box_unwrap_FixFn(callee_fn);

    FixArray *args = FixArray_new_auto(node->call.num_args);
    for (size_t i = 0; i < node->call.num_args; i++) {
        Box arg = interp_eval_ast(node->call.args[i], scope);
        FixArray_append(args, arg);
    }

    if(len_ref(args) < fn->signature.meta.size) {
        interp_error(sMSG("Function `%.*s` expects at least %zu arguments, but %zu were provided."),
                    fmt(fn->name), fn->signature.meta.size, len_ref(args));
        // FixArray_aro_free(args);
        return Box_exit();
    }

    FixScope fn_scope = FixScope_empty(sMSG("$Scope"));
    FixScope_data_new(&fn_scope, scope);

    return FixFn_call(fn, fn_scope, *args);
}



Box interp_eval_use(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_USE);

    Box _module = interp_eval_load_module(node);

    if(Box_is_error(_module)) {
        interp_error(sMSG("Failed to load module: %.*s"), node->use_stmt.module_path);
        return Box_exit();
    }

    FlxObject *module = Box_unwrap_FixObject(_module);
    FixScope_merge_local(scope, &module->data->namespace);

    return Box_null();
}

Box interp_eval_load_module(Ast *node) {
    /// @todo implement module loading
    return Box_exit();
}

Box interp_eval_struct_def(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_STRUCT_DEF);

    FixStruct *struct_def = FixStruct_new(node->struct_def.name, node->struct_def.num_fields);
    for(size_t i = 0; i < node->struct_def.num_fields; i++) {
        FixDict_set(&struct_def->fields, node->struct_def.fields[i].name, Box_null());
    }

    FixScope_define_local(scope, node->struct_def.name, Box_wrap_BoxedArena(struct_def));

    return Box_done();
}


/// @brief
/// @param node
/// @param scope
/// @return
/// @example
///     const eg = Point { .x = 10, .y = 20 }
///
Box interp_eval_object_literal(Ast *node, FixScope *scope) {
    require_not_null(node);
    require_not_null(scope);
    interp_trace();
    interp_require_type(AST_OBJECT_LITERAL);

    Box struct_obj;

    if(!FixScope_lookup(scope, node->object_literal.struct_name, &struct_obj)) {
        interp_error(sMSG("Undefined struct: %.*s"), node->object_literal.struct_name);
        return Box_exit();
    }



    FixStruct *struct_def = Box_unwrap_FixStruct(struct_obj);
    FlxObject *object = FlxObject_new(struct_def);
    FixDict *fields = &object->data->namespace.data;

    for(size_t i = 0; i < node->object_literal.num_fields; i++) {
        FixStr field_name = node->object_literal.fields[i].name;
        Ast *field_value = node->object_literal.fields[i].value;

        Box value = interp_eval_ast(field_value, scope);
        interp_return_if_error(value);

        FixDict_set(fields, field_name, value);
    }

    return Box_wrap_BoxedHeap(object);
}

int interp_eval_test_main(void) {
    // Create a simple AST representing the expression: 1 + 2
    Ast left = { .type = AST_INT, .integer.value = 1 };
    Ast right = { .type = AST_INT, .integer.value = 2 };
    Ast bop = { .type = AST_BOP, .bop.left = &left, .bop.right = &right, .bop.op = s("+") };

    // Initialize global scope
    FixScope global_scope = FixScope_empty(sMSG("$Scope"));
    FixScope_data_new(&global_scope, nullptr);
    log_assert(global_scope.data.meta.capacity > 0, sMSG("Failed to create global scope"));

    // Evaluate the AST
    Box result = interp_eval_ast(&bop, &global_scope);

    // Assert the result is correct
    log_assert(result.type == UBX_INT, sMSG("interp_eval_ast did not return UBX_INT"));
    log_assert(Box_unwrap_int(result) == 3, sMSG("interp_eval_ast did not compute 1 + 2 correctly"));

    // Cleanup
    // FixScope_aro_free(global_scope);

    return 0;
}


#pragma endregion

#pragma region InterpreterEvaluatorImpl

// Evaluate a function call
Box interp_eval_fn(Box fn_obj, int num_args, Box *args) {
    if(fn_obj.type != UBX_PTR_ARENA) {
        interp_error(sMSG("Attempted to call a non-function."));
        return Box_exit();
    }

    FixFn *fn = Box_unwrap_FixFn(fn_obj);
    FixArray *args_array = FixArray_new_auto(num_args);
    for(int i = 0; i < num_args; i++) {
        args_array->data[i] = args[i];
    }

    return FixFn_call(fn, fn->enclosure, *args_array);
}

// Evaluate a unary operation
Box interp_eval_uop(FixStr op, Box operand) {
    if(FixStr_eq_chr(op, '-')) {
        if(operand.type == UBX_INT) {
            return Box_wrap_int(-Box_unwrap_int(operand));
        } else if(operand.type == UBX_FLOAT) {
            float val = Box_unwrap_float(operand);
            return Box_wrap_float(-val);
        } else {
            interp_error(sMSG("Unsupported operand type for unary '-'."));
            return Box_exit();
        }
    }
    // Add more unary operators as needed
    interp_error(sMSG("Unsupported unary operator: %s"), op);
    return Box_exit();
}

// Evaluate a binary operation
/// @todo make the type jugglery more robust
Box interp_eval_bop(FixStr op, Box left, Box right) {
    if(FixStr_eq_chr(op, '+')) {
        if(left.type == UBX_INT && right.type == UBX_INT) {
            return Box_wrap_int(Box_unwrap_int(left) + Box_unwrap_int(right));
        }

        else if(left.type == UBX_FLOAT && right.type == UBX_FLOAT) {
            return Box_wrap_float(Box_unwrap_float(left) + Box_unwrap_float(right));
        }
        /// @note if either is a float, we'll cast both to float
        else if(left.type == UBX_INT && right.type == UBX_FLOAT) {
            return Box_wrap_float((float) Box_unwrap_int(left) + Box_unwrap_float(right));
        }

        else if(left.type == UBX_FLOAT && right.type == UBX_INT) {
            return Box_wrap_float(Box_unwrap_float(left) + (float) Box_unwrap_int(right));
        }

        else if(left.type == UBX_PTR_ARENA && right.type == UBX_PTR_ARENA) {
            // Concatenate strings
            /// @todo: this can use a fix string since we know the sizes of both operands
        }

        else {
            interp_error(sMSG("Unsupported operand types for '%c': %.*s, %.*s"),
                FixStr_chr_at(op, 0), fmt(ubx_nameof(left.type)), fmt(ubx_nameof(right.type)));
            return Box_exit();
        }
    }
    // Implement other binary operators (e.g., -, *, /, etc.)
    if(FixStr_eq_chr(op, '-')) {
        if(left.type == UBX_INT && right.type == UBX_INT) {
            return Box_wrap_int(Box_unwrap_int(left) - Box_unwrap_int(right));
        } else if(left.type == UBX_FLOAT && right.type == UBX_FLOAT) {
            return Box_wrap_float(Box_unwrap_float(left) - Box_unwrap_float(right));
        } else {
            interp_error(sMSG("Unsupported operand types for '%c': %.*s, %.*s"),
                FixStr_chr_at(op, 0), fmt(ubx_nameof(left.type)), fmt(ubx_nameof(right.type)));
            return Box_exit();
        }
    }
    if(FixStr_eq_chr(op, '*')) {
        if(left.type == UBX_INT && right.type == UBX_INT) {
            return Box_wrap_int(Box_unwrap_int(left) * Box_unwrap_int(right));
        } else if(left.type == UBX_FLOAT && right.type == UBX_FLOAT) {
            return Box_wrap_float(Box_unwrap_float(left) * Box_unwrap_float(right));
        } else {
            interp_error(sMSG("Unsupported operand types for '%c': %.*s, %.*s"),
                FixStr_chr_at(op, 0), fmt(ubx_nameof(left.type)), fmt(ubx_nameof(right.type)));
            return Box_exit();
        }
    }
    if(FixStr_eq_chr(op, '/')) {
        if(left.type == UBX_INT && right.type == UBX_INT) {
            if(Box_unwrap_int(right) == 0) {
                interp_error(sMSG("Division by zero."));
                return Box_exit();
            }
            return Box_wrap_int(Box_unwrap_int(left) / Box_unwrap_int(right));
        } else if(left.type == UBX_FLOAT && right.type == UBX_FLOAT) {
            if(Box_unwrap_float(right) == 0.0f) {
                interp_error(sMSG("Division by zero."));
                return Box_exit();
            }
            return Box_wrap_float(Box_unwrap_float(left) / Box_unwrap_float(right));
        } else {
            interp_error(sMSG("Unsupported operand types for '%c': %.*s, %.*s"),
                FixStr_chr_at(op, 0), fmt(ubx_nameof(left.type)), fmt(ubx_nameof(right.type)));
            return Box_exit();
        }
    }
    // Add more operators as needed

    interp_error(sMSG("Unsupported binary operator: %s"), op);
    return Box_exit();
}


int interp_error_test_main(void) {

    return 0;
}
#pragma endregion

#pragma region InterpreterTestMain

int interpreter_member_access_test(void) {
    // Define a struct Point
    FixScope global_scope = FixScope_empty(s("Global scope"));
    FixScope_data_new(&global_scope, nullptr);

    // Define struct Point
    Ast point_struct = {
        .type = AST_STRUCT_DEF,
        .struct_def = {
            .name = s("Point"),
            .num_fields = 2,
            .fields = (struct FixStructField[]) {
                { .name = s("x"), .type = nullptr, .default_value = nullptr },
                { .name = s("y"), .type = nullptr, .default_value = nullptr }
            }
        }
    };
    Box result = interp_eval_struct_def(&point_struct, &global_scope);
    log_assert(result.type == UBX_BOOL && result.payload == BXS_DONE, sMSG("FixStruct def did not return box_done"));

    // Create a Point instance
    Ast point_instance = {
        .type = AST_OBJECT_LITERAL,
        .object_literal = {
            .struct_name = s("Point"),
            .num_fields = 2,
            .fields = (struct AstObjectField[]) {
                { .name = s("x"), .value = &(Ast){ .type = AST_INT, .integer.value = 10 } },
                { .name = s("y"), .value = &(Ast){ .type = AST_INT, .integer.value = 20 } }
            }
        }
    };
    Box p = interp_eval_object_literal(&point_instance, &global_scope);
    FixScope_define_local(&global_scope, s("p"), p);

    // Access p.x
    Ast access_x = {
        .type = AST_MEMBER_ACCESS,
        .member_access = {
            .target = &(Ast){ .type = AST_ID, .id = { .name = s("p") } },
            .property = s("x")
        }
    };
    Box x_val = interp_eval_ast(&access_x, &global_scope);
    log_assert(x_val.type == UBX_INT && Box_unwrap_int(x_val) == 10, sMSG("Member access p.x failed"));

    // Access p.y
    Ast access_y = {
        .type = AST_MEMBER_ACCESS,
        .member_access = {
            .target = &(Ast){ .type = AST_ID, .id = { .name = s("p") } },
            .property = s("y")
        }
    };
    Box y_val = interp_eval_ast(&access_y, &global_scope);
    log_assert(y_val.type == UBX_INT && Box_unwrap_int(y_val) == 20, sMSG("Member access p.y failed"));

    return 0;
}

int interpreter_method_call_test(void) {
    return 0;
}

int interpreter_scope_tests(void) {
    printf("Running interpreter tests...\n");
    // Test global constant def
    {
        FixScope global_scope = FixScope_empty(sMSG("Global scope"));
        FixScope_data_new(&global_scope, nullptr);

        Ast const_node = {
            .type = AST_CONST_DEF,
            .const_stmt = {
                .name = s("test"),
                .value = &(Ast){ .type = AST_INT, .integer.value = 1234 }
            }
        };

        Box result = interp_eval_const_def(&const_node, &global_scope);
        log_assert(result.type == UBX_BOOL && result.payload == BXS_DONE, sMSG("Const def did not return box_done"));

        Box retrieved;
        bool found = FixScope_lookup(&global_scope, s("test"), &retrieved);
        log_assert(found, sMSG("Global constant 'test' was not found"));
        log_assert(retrieved.type == UBX_INT && Box_unwrap_int(retrieved) == 1234, sMSG("Global constant 'test' has incorrect value"));

        log_message(LL_INFO, sMSG("Global constant 'test' has been defined"));
        log_message(LL_INFO, sMSG("Global constant 'test' has value %lld\n"), Box_unwrap_int(retrieved));
    }

    // Test global function def
    {
        FixScope global_scope = FixScope_empty(sMSG("Global scope"));
        FixScope_data_new(&global_scope, nullptr);

        Ast fn_node = {
            .type = AST_FN_DEF,
            .fn = {
                .name = s("main"),
                .num_params = 0,
                .params = nullptr,
                .body = &(Ast){
                    .type = AST_FN_DEF_CALL,
                    .call = {
                        .callee = &(Ast){
                            .type = AST_ID,
                            .id = { .name = s("log") }
                        },
                        .num_args = 1,
                        .args = Arena_alloc(sizeof(Ast*) * 1)
                    }
                }
            }
        };

        // fn_node.fn.params.args[0] = &(Ast){
        //     .type = AST_ID,
        //     .id = { .name = s("test") }
        // };

        Box result = interp_eval_fn_def(&fn_node, &global_scope);
        log_assert(result.type == UBX_PTR_ARENA, sMSG("Function def did not return UBX_PTR_ARENA"));

        Box retrieved;
        bool found = FixScope_lookup(&global_scope, s("main"), &retrieved);
        log_assert(found, sMSG("Global function 'main' was not found"));
        log_assert(retrieved.type == UBX_PTR_ARENA, sMSG("Global function 'main' has incorrect type"));

        log_message(LL_INFO, sMSG("Global function 'main' has been defined\n"));
    }

    return 0;
}


void interpreter_test_main(void) {
    Box_test_main();

    // Run tests for InterpErrorImpl
    interp_error_test_main();

    // Run tests for BoxedPrecacheImpl
    interp_precache_test_main();

    // Run tests for FlxDataTypesImpl
    FixArray_test_main();
    FixDict_test_main();
    FixArray_test_main();
    // FlxDict_test_main();
    FixScope_test_main();
    FixFn_test_main();

    interpreter_scope_tests();
    interpreter_member_access_test();
    interpreter_method_call_test();
    // Run tests for InterpEvalImpl
    interp_eval_test_main();
}

#pragma endregion

#pragma region InterpreterSetup


#define code_example(n)\
    (FixStr) {.cstr = __glo_example##n, .size = sizeof(__glo_example##n) - 1}


void interpreter_examples_main(int argc, char **argv) {
    interp_run_from_source(ctx_parser(), code_example(1), argc, argv);
}


void interpreter_main(int argc, char **argv) {
    enum {CLI_POSITIONAL = 0, CLI_SOURCE, CLI_HELP, CLI_INDENT, CLI_ENUM_SIZE};

    CliOption options[] = {
        [CLI_POSITIONAL] = cli_opt_default(s("--source"), s("main.doubt")),
        [CLI_HELP]   = cli_opt_flag(s("--help")),
        [CLI_INDENT] = cli_opt_default(s("--indent"), s("    ")),
    };

    cli_print_opts(options, CLI_ENUM_SIZE);
    /// @todo file reading
    // FixStr source = FixStr_read_file_new(cli_opt_get(options, CLI_POSITIONAL).cstr);
    //

    interp_run_from_source(ctx_parser(), code_example(1), argc, argv);
}


#pragma endregion

#pragma region InterpreterMain

int main(int argc, char **argv) {
    GlobalContext_setup();


    #if defined(EXAMPLES)
        interpreter_examples_main(argc, argv);
    #endif

    #if defined(TEST)
        // parsing_test_main();
        // internal_test_main();
        // interpreter_test_main();
    #else
        interpreter_main(argc, argv);
    #endif

    cfree_log_leaks();

    return 0;
}

#pragma endregion

//#endregion
///---------- ---------- ---------- IMPL: /END ---------- ---------- ---------- -- ///
/// @note EOF
