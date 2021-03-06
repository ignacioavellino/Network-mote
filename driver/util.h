#ifndef UTIL_H
#define UTIL_H

/// use default "pc" serial implementation
/// may be 0 if we are inside tinyos
#ifndef INCLUDE_SERIAL_IMPLEMENTATION
#define INCLUDE_SERIAL_IMPLEMENTATION 1
#endif

/// also, in tinyos malloc is not recommended, so
/// its inclusion and usage is prevented here
#ifndef DYNAMIC_MEMORY
#define DYNAMIC_MEMORY 1
#endif

/// again, in tinyos assertions have no meaning
#ifndef ALLOW_ASSERT
#define ALLOW_ASSERT 1
#endif

#if ALLOW_ASSERT
#include <assert.h>
#else
// fake assertion
#define assert(expr) 
#endif

/// macro to dump a variable
#if LOG_LEVEL&128
#define DUMP_VAR(VAR,TYPE) printf("[%s:%d] "#VAR" == %"#TYPE";\n",__FILE__,__LINE__,VAR)
#else
#define DUMP_VAR(VAR,TYPE)
#endif

/// several macros to ease the usage of DUMP_VAR
#define DUMP_CHAR(VAR) DUMP_VAR((int)VAR,d)
#define DUMP_UCHAR(VAR) DUMP_VAR((unsigned)VAR,u)
#define DUMP_INT(VAR) DUMP_VAR(VAR,d)
#define DUMP_UINT(VAR) DUMP_VAR(VAR,u)
#define DUMP_LONG(VAR) DUMP_VAR(VAR,ld)
#define DUMP_ULONG(VAR) DUMP_VAR(VAR,lu)
#define DUMP_STRING(VAR) DUMP_VAR(VAR,s)
#define DUMP_FLOAT(VAR) DUMP_VAR(VAR,f)

/// terminal colours
#define CONTROL(color) "\033[0;"color"m"
#define RED "31"
#define GREEN "32"
#define YELLOW "33"
#define BLUE "34"
#define MAGENTA "35"
#define CYAN "36"
#define WHITE "37"

/// default mapping of the colours to different log levels
#define ERROR_COLOR RED
#define INFO_COLOR CYAN
#define NOTE_COLOR BLUE
#define WARNING_COLOR MAGENTA
#define DEBUG_COLOR GREEN
#define RESET CONTROL("0")

/// used to colour a string
#define COLOR(color,string) CONTROL(color) string RESET

/// log levels
#include <stdio.h>
#define LOG__PRINT(TYPE,MSG,...) {fprintf(stderr,"%-20s " MSG "\n","<" TYPE ">",##__VA_ARGS__); fflush(stderr); }

#if LOG_LEVEL&1
#define LOG_ERROR(MSG,...)   LOG__PRINT(COLOR(ERROR_COLOR,"ERROR"),MSG,##__VA_ARGS__)
#else
#define LOG_ERROR(MSG,...)
#warning "error messages not compiled"
#endif

#if LOG_LEVEL&2
#define LOG_WARNING(MSG,...) LOG__PRINT(COLOR(WARNING_COLOR,"WARN"),MSG,##__VA_ARGS__)
#else
#define LOG_WARNING(MSG,...)
#endif
#define LOG_WARN LOG_WARNING

#if LOG_LEVEL&4
#define LOG_NOTE(MSG,...)    LOG__PRINT(COLOR(NOTE_COLOR,"NOTE"),MSG,##__VA_ARGS__)
#else
#define LOG_NOTE(MSG,...)
#endif

#if LOG_LEVEL&8
#define LOG_INFO(MSG,...)   LOG__PRINT(COLOR(INFO_COLOR,"INFO"),MSG,##__VA_ARGS__)
#else
#define LOG_INFO(MSG,...)
#endif

#if LOG_LEVEL&16
#define LOG_DEBUG(MSG,...)  LOG__PRINT(COLOR(DEBUG_COLOR,"DEBUG"),MSG,##__VA_ARGS__)
#else
#define LOG_DEBUG(MSG,...)
#endif

#include "structs.h"

#define class(NAME_T,body)                      \
    typedef struct NAME_T NAME_T;               \
    struct NAME_T {                             \
        class_t _class;                         \
        body                                    \
    }


/// used to forward declare a type (specifically a class) you will define later
#define forward(NAME_T) struct NAME_T

/// destructor function pointer type
typedef void (*dtor_t)(void*);

/// macros to make constructors easier
#define CTOR(objPtr) _class_t_ctor((void**)&objPtr,sizeof(*objPtr))
#define DTOR(objPtr) _class_t_dtor((void**)&objPtr)
#define SETDTOR(classPtr) (classPtr)->dtor = (dtor_t)

/// syntactic sugar
#define virtual

/// internal data used to specify classes, this holds meta information and could
/// even be extended to hold a vtable in order to support real polymorphism
typedef struct {
    char ctorAllocd;
    void (*dtor)(void* this);
} class_t;

/**
 * Basic class constructor. Do not call explicitly, use the CTOR define.
 *
 * @param obj Pointer to the variable holding the object. *obj may be NULL, obj may not!
 * @param typesz size of the object crated.
 */
class_t* _class_t_ctor(void** obj, unsigned typesz);

/**
 * Basic class destructor. Do not call explicitly, use the DTOR define.
 *
 * @param obj Pointer to the variable holding the object. Neither *obj nor obj may be NULL.
 */
void _class_t_dtor(void** obj);

#endif
