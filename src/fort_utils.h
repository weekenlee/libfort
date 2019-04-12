#ifndef FORT_IMPL_H
#define FORT_IMPL_H

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS /* To disable warnings for unsafe functions */
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "fort.h"

/* Define FT_INTERNAL to make internal libfort functions static
 * in the result amalgamed source file.
 */
#ifdef FT_AMALGAMED_SOURCE
#define FT_INTERNAL static
#else
#define FT_INTERNAL
#endif /* FT_AMALGAMED_SORCE */


#define FORT_DEFAULT_COL_SEPARATOR '|'
extern char g_col_separator;

#define FORT_COL_SEPARATOR_LENGTH 1

#define FORT_UNUSED  __attribute__((unused))

#define F_MALLOC fort_malloc
#define F_FREE fort_free
#define F_CALLOC fort_calloc
#define F_REALLOC fort_realloc
#define F_STRDUP fort_strdup
#define F_WCSDUP fort_wcsdup
/* @todo: replace with custom impl !!!*/
#define F_UTF8DUP utf8dup

#define F_CREATE(type) ((type *)F_CALLOC(sizeof(type), 1))

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


enum PolicyOnNull {
    Create,
    DoNotCreate
};


enum F_BOOL {
    F_FALSE = 0,
    F_TRUE = 1
};

enum str_buf_type {
    CHAR_BUF,
#ifdef FT_HAVE_WCHAR
    W_CHAR_BUF,
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
    UTF8_BUF,
#endif /* FT_HAVE_WCHAR */
};

enum table_char_type {
    FT_UNDEF_CHAR,
    FT_CHAR,
#ifdef FT_HAVE_WCHAR
    FT_W_CHAR
#endif /* FT_HAVE_WCHAR */
};

#define FT_STR_2_CAT_(arg1, arg2) \
    arg1##arg2
#define FT_STR_2_CAT(arg1, arg2) \
    FT_STR_2_CAT_(arg1, arg2)

#define UNIQUE_NAME_(prefix) \
    FT_STR_2_CAT(prefix,__COUNTER__)
#define UNIQUE_NAME(prefix) \
    UNIQUE_NAME_(prefix)



/*****************************************************************************
 *               LOGGER
 *****************************************************************************/
#define SYS_LOG_ERROR(...)



/*****************************************************************************
 *               DEFAULT_SIZES
 * ***************************************************************************/
#define DEFAULT_STR_BUF_SIZE 1024
#define DEFAULT_VECTOR_CAPACITY 10



struct fort_table_properties;
struct fort_column_properties;
struct fort_row;
struct vector;
struct fort_cell;
struct string_buffer;
struct separator {
    int enabled;
};

typedef struct fort_table_properties fort_table_properties_t;
struct fort_context {
    fort_table_properties_t *table_properties;
    size_t row;
    size_t column;

    char *buf;
    size_t width_available_in_cntx;
    size_t width_written;
    size_t raw_bytes_available;
    size_t raw_bytes_written;
};
#define INIT_CONTEXT(cntx, buf, wid_avail, raw_bytes_avail) \
    (cntx)->buf = (buf); \
    (cntx)->width_available = (wid_avail); \
    (cntx)->width_written = 0; \
    (cntx)->raw_bytes_available = (raw_bytes_avail); \
    (cntx)->raw_bytes_written = 0

typedef struct fort_context context_t;
typedef struct fort_column_properties fort_column_properties_t;
typedef struct vector vector_t;
typedef struct fort_cell fort_cell_t;
typedef struct string_buffer string_buffer_t;
typedef struct fort_row fort_row_t;
/*typedef struct ft_table ft_table_t;*/
typedef struct separator separator_t;

enum CellType {
    CommonCell,
    GroupMasterCell,
    GroupSlaveCell
};

enum request_geom_type {
    VISIBLE_GEOMETRY,
    INTERN_REPR_GEOMETRY
};

/*****************************************************************************
 *               LIBFORT helpers
 *****************************************************************************/

extern void *(*fort_malloc)(size_t size);
extern void (*fort_free)(void *ptr);
extern void *(*fort_calloc)(size_t nmemb, size_t size);
extern void *(*fort_realloc)(void *ptr, size_t size);

FT_INTERNAL
void set_memory_funcs(void *(*f_malloc)(size_t size), void (*f_free)(void *ptr));

FT_INTERNAL
char *fort_strdup(const char *str);

FT_INTERNAL
size_t number_of_columns_in_format_string(const char *fmt);

#if defined(FT_HAVE_WCHAR)
FT_INTERNAL
wchar_t *fort_wcsdup(const wchar_t *str);

FT_INTERNAL
size_t number_of_columns_in_format_wstring(const wchar_t *fmt);
#endif

FT_INTERNAL
int snprint_n_strings(char *buf, size_t length, size_t n, const char *str);

FT_INTERNAL
int new_snprint_n_strings(context_t *cntx, size_t length, size_t n, const char *str);

#if defined(FT_HAVE_WCHAR)
FT_INTERNAL
int wsnprint_n_string(wchar_t *buf, size_t length, size_t n, const char *str);
#endif


#define CHCK_RSLT_ADD_TO_WRITTEN(statement) \
    do { \
        tmp = statement; \
        if (tmp < 0) {\
            goto clear; \
        } \
        written += (size_t)tmp; \
    } while(0)

#define CHCK_RSLT_ADD_TO_INVISIBLE_WRITTEN(statement) \
    do { \
        tmp = statement; \
        if (tmp < 0) {\
            goto clear; \
        } \
        invisible_written += (size_t)tmp; \
    } while(0)


#define CHECK_NOT_NEGATIVE(x) \
    do { if ((x) < 0) goto fort_fail; } while (0)

#endif /* FORT_IMPL_H */
