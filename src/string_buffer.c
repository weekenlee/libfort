#include "string_buffer.h"
#include "properties.h"
#include "wcwidth.h"
#include <assert.h>
#include <stddef.h>
#ifdef FT_HAVE_WCHAR
#include <wchar.h>
#endif
#if defined(FT_HAVE_UTF8)
#include "utf8.h"
#endif

static ptrdiff_t str_iter_width(const char *beg, const char *end)
{
    assert(end >= beg);
    return (end - beg);
}


#ifdef FT_HAVE_WCHAR
static ptrdiff_t wcs_iter_width(const wchar_t *beg, const wchar_t *end)
{
    assert(end >= beg);
    return mk_wcswidth(beg, (size_t)(end - beg));
}
#endif /* FT_HAVE_WCHAR */


static size_t buf_str_len(const string_buffer_t *buf)
{
    assert(buf);

    switch (buf->type) {
        case CHAR_BUF:
            return strlen(buf->str.cstr);
#ifdef FT_HAVE_WCHAR
        case W_CHAR_BUF:
            return wcslen(buf->str.wstr);
#endif
#ifdef FT_HAVE_UTF8
        case UTF8_BUF:
            return utf8len(buf->str.u8str);
#endif
    }

    assert(0);
    return 0;
}



FT_INTERNAL
size_t strchr_count(const char *str, char ch)
{
    if (str == NULL)
        return 0;

    size_t count = 0;
    str = strchr(str, ch);
    while (str) {
        count++;
        str++;
        str = strchr(str, ch);
    }
    return count;
}

#ifdef FT_HAVE_WCHAR
FT_INTERNAL
size_t wstrchr_count(const wchar_t *str, wchar_t ch)
{
    if (str == NULL)
        return 0;

    size_t count = 0;
    str = wcschr(str, ch);
    while (str) {
        count++;
        str++;
        str = wcschr(str, ch);
    }
    return count;
}
#endif


// todo: do something with code below!!!!!!!!!!!!1


#if defined(FT_HAVE_UTF8)
FT_INTERNAL
void *ut8next(const void *str)
{
    utf8_int32_t out_codepoint;
    return utf8codepoint(str, &out_codepoint);
}

FT_INTERNAL
size_t utf8chr_count(const void *str, utf8_int32_t ch)
{
    if (str == NULL)
        return 0;

    size_t count = 0;
    str = utf8chr(str, ch);
    while (str) {
        count++;
        str = ut8next(str);
        str = utf8chr(str, ch);
    }
    return count;
}
#endif


FT_INTERNAL
const char *str_n_substring_beg(const char *str, char ch_separator, size_t n)
{
    if (str == NULL)
        return NULL;

    if (n == 0)
        return str;

    str = strchr(str, ch_separator);
    --n;
    while (n > 0) {
        if (str == NULL)
            return NULL;
        --n;
        str++;
        str = strchr(str, ch_separator);
    }
    return str ? (str + 1) : NULL;
}


#ifdef FT_HAVE_WCHAR
FT_INTERNAL
const wchar_t *wstr_n_substring_beg(const wchar_t *str, wchar_t ch_separator, size_t n)
{
    if (str == NULL)
        return NULL;

    if (n == 0)
        return str;

    str = wcschr(str, ch_separator);
    --n;
    while (n > 0) {
        if (str == NULL)
            return NULL;
        --n;
        str++;
        str = wcschr(str, ch_separator);
    }
    return str ? (str + 1) : NULL;
}
#endif /* FT_HAVE_WCHAR */

#if defined(FT_HAVE_UTF8)
FT_INTERNAL
const void *utf8_n_substring_beg(const void *str, utf8_int32_t ch_separator, size_t n)
{
    if (str == NULL)
        return NULL;

    if (n == 0)
        return str;

    str = utf8chr(str, ch_separator);
    --n;
    while (n > 0) {
        if (str == NULL)
            return NULL;
        --n;
        str = ut8next(str);
        str = utf8chr(str, ch_separator);
    }
    return str ? (ut8next(str)) : NULL;
}
#endif


FT_INTERNAL
void str_n_substring(const char *str, char ch_separator, size_t n, const char **begin, const char **end)
{
    const char *beg = str_n_substring_beg(str, ch_separator, n);
    if (beg == NULL) {
        *begin = NULL;
        *end = NULL;
        return;
    }

    const char *en = strchr(beg, ch_separator);
    if (en == NULL) {
        en = str + strlen(str);
    }

    *begin = beg;
    *end = en;
    return;
}


#ifdef FT_HAVE_WCHAR
FT_INTERNAL
void wstr_n_substring(const wchar_t *str, wchar_t ch_separator, size_t n, const wchar_t **begin, const wchar_t **end)
{
    const wchar_t *beg = wstr_n_substring_beg(str, ch_separator, n);
    if (beg == NULL) {
        *begin = NULL;
        *end = NULL;
        return;
    }

    const wchar_t *en = wcschr(beg, ch_separator);
    if (en == NULL) {
        en = str + wcslen(str);
    }

    *begin = beg;
    *end = en;
    return;
}
#endif /* FT_HAVE_WCHAR */

#if defined(FT_HAVE_UTF8)
FT_INTERNAL
void utf8_n_substring(const void *str, utf8_int32_t ch_separator, size_t n, const void **begin, const void **end)
{
    const char *beg = utf8_n_substring_beg(str, ch_separator, n);
    if (beg == NULL) {
        *begin = NULL;
        *end = NULL;
        return;
    }

    const char *en = utf8chr(beg, ch_separator);
    if (en == NULL) {
        en = (const char *)str + strlen(str);
    }

    *begin = beg;
    *end = en;
    return;
}
#endif /* FT_HAVE_UTF8 */



FT_INTERNAL
string_buffer_t *create_string_buffer(size_t number_of_chars, enum str_buf_type type)
{
    size_t sz = 0;

    switch (type) {
        case CHAR_BUF:
            sz = (number_of_chars) * sizeof(char);
            break;
#ifdef FT_HAVE_WCHAR
        case W_CHAR_BUF:
            sz = (number_of_chars) * sizeof(wchar_t);
            break;
#endif
#ifdef FT_HAVE_UTF8
        case UTF8_BUF:
            sz = (number_of_chars) * 4;  // @todo: ahtung replace this 4 with something proper
            break;
#endif
    }

    string_buffer_t *result = (string_buffer_t *)F_MALLOC(sizeof(string_buffer_t));
    if (result == NULL)
        return NULL;
    result->str.data = F_MALLOC(sz);
    if (result->str.data == NULL) {
        F_FREE(result);
        return NULL;
    }
    result->data_sz = sz;
    result->type = type;

    if (sz && type == CHAR_BUF) {
        result->str.cstr[0] = '\0';
#ifdef FT_HAVE_WCHAR
    } else if (sz && type == W_CHAR_BUF) {
        result->str.wstr[0] = L'\0';
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
    } else if (sz && type == UTF8_BUF) {
        ((char*)result->str.u8str)[0] = '\0';
#endif /* FT_HAVE_WCHAR */
    }

    return result;
}


FT_INTERNAL
void destroy_string_buffer(string_buffer_t *buffer)
{
    if (buffer == NULL)
        return;
    F_FREE(buffer->str.data);
    buffer->str.data = NULL;
    F_FREE(buffer);
}

FT_INTERNAL
string_buffer_t *copy_string_buffer(const string_buffer_t *buffer)
{
    assert(buffer);
    string_buffer_t *result = create_string_buffer(buffer->data_sz, buffer->type);
    if (result == NULL)
        return NULL;
    switch (buffer->type) {
        case CHAR_BUF:
            if (FT_IS_ERROR(fill_buffer_from_string(result, buffer->str.cstr))) {
                destroy_string_buffer(result);
                return NULL;
            }
            break;
#ifdef FT_HAVE_WCHAR
        case W_CHAR_BUF:
            if (FT_IS_ERROR(fill_buffer_from_wstring(result, buffer->str.wstr))) {
                destroy_string_buffer(result);
                return NULL;
            }
            break;
#endif /* FT_HAVE_WCHAR */
        default:
            destroy_string_buffer(result);
            return NULL;
    }
    return result;
}

FT_INTERNAL
fort_status_t realloc_string_buffer_without_copy(string_buffer_t *buffer)
{
    assert(buffer);
    char *new_str = (char *)F_MALLOC(buffer->data_sz * 2);
    if (new_str == NULL) {
        return FT_MEMORY_ERROR;
    }
    F_FREE(buffer->str.data);
    buffer->str.data = new_str;
    buffer->data_sz *= 2;
    return FT_SUCCESS;
}


FT_INTERNAL
fort_status_t fill_buffer_from_string(string_buffer_t *buffer, const char *str)
{
    assert(buffer);
    assert(str);

    char *copy = F_STRDUP(str);
    if (copy == NULL)
        return FT_MEMORY_ERROR;

    F_FREE(buffer->str.data);
    buffer->str.cstr = copy;
    buffer->type = CHAR_BUF;

    return FT_SUCCESS;
}


#ifdef FT_HAVE_WCHAR
FT_INTERNAL
fort_status_t fill_buffer_from_wstring(string_buffer_t *buffer, const wchar_t *str)
{
    assert(buffer);
    assert(str);

    wchar_t *copy = F_WCSDUP(str);
    if (copy == NULL)
        return FT_MEMORY_ERROR;

    F_FREE(buffer->str.data);
    buffer->str.wstr = copy;
    buffer->type = W_CHAR_BUF;

    return FT_SUCCESS;
}
#endif /* FT_HAVE_WCHAR */

#ifdef FT_HAVE_UTF8
//FT_INTERNAL
fort_status_t fill_buffer_from_u8string(string_buffer_t *buffer, const void *str)
{
    assert(buffer);
    assert(str);

    void *copy = F_UTF8DUP(str);
    if (copy == NULL)
        return FT_MEMORY_ERROR;

    F_FREE(buffer->str.data);
    buffer->str.u8str = copy;
    buffer->type = UTF8_BUF;

    return FT_SUCCESS;
}
#endif /* FT_HAVE_UTF8 */


FT_INTERNAL
size_t buffer_text_visible_height(const string_buffer_t *buffer)
{
    if (buffer == NULL || buffer->str.data == NULL || buf_str_len(buffer) == 0) {
        return 0;
    }
    if (buffer->type == CHAR_BUF)
        return 1 + strchr_count(buffer->str.cstr, '\n');
#ifdef FT_HAVE_WCHAR
    else if (buffer->type == W_CHAR_BUF)
        return 1 + wstrchr_count(buffer->str.wstr, L'\n');
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
    else if (buffer->type == UTF8_BUF)
        return 1 + utf8chr_count(buffer->str.u8str, '\n');
#endif /* FT_HAVE_WCHAR */

    assert(0);
    return 0;
}

#ifdef FT_HAVE_UTF8
FT_INTERNAL
size_t ut8_width(const void *beg, const void *end)
{
    size_t sz = (size_t)((const char *)end - (const char *)beg);
    char *tmp = F_MALLOC(sizeof(char) * (sz + 1));
    // @todo: add check to tmp
    assert(tmp);

    memcpy(tmp, beg, sz);
    tmp[sz] = '\0';
    size_t result = utf8len(tmp);
    F_FREE(tmp);
    return result;
}
#endif /* FT_HAVE_WCHAR */

FT_INTERNAL
size_t buffer_text_visible_width(const string_buffer_t *buffer)
{
    size_t max_length = 0;
    if (buffer->type == CHAR_BUF) {
        size_t n = 0;
        while (1) {
            const char *beg = NULL;
            const char *end = NULL;
            str_n_substring(buffer->str.cstr, '\n', n, &beg, &end);
            if (beg == NULL || end == NULL)
                return max_length;

            max_length = MAX(max_length, (size_t)(end - beg));
            ++n;
        }
#ifdef FT_HAVE_WCHAR
    } else if (buffer->type == W_CHAR_BUF) {
        size_t n = 0;
        while (1) {
            const wchar_t *beg = NULL;
            const wchar_t *end = NULL;
            wstr_n_substring(buffer->str.wstr, L'\n', n, &beg, &end);
            if (beg == NULL || end == NULL)
                return max_length;

            int line_width = mk_wcswidth(beg, (size_t)(end - beg));
            if (line_width < 0) /* For safety */
                line_width = 0;
            max_length = MAX(max_length, (size_t)line_width);

            ++n;
        }
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
    } else if (buffer->type == UTF8_BUF) {
        size_t n = 0;
        while (1) {
            const void *beg = NULL;
            const void *end = NULL;
            utf8_n_substring(buffer->str.u8str, '\n', n, &beg, &end);
            if (beg == NULL || end == NULL)
                return max_length;

            max_length = MAX(max_length, (size_t)ut8_width(beg, end));
            ++n;
        }
#endif /* FT_HAVE_WCHAR */
    }

    return max_length; /* shouldn't be here */
}


FT_INTERNAL
int buffer_printf(string_buffer_t *buffer, size_t buffer_row, char *buf, size_t total_buf_len,
                  const context_t *context, const char *content_style_tag, const char *reset_content_style_tag)
{
#define CHAR_TYPE char
#define NULL_CHAR '\0'
#define NEWLINE_CHAR '\n'
#define SPACE_CHAR " "
#define SNPRINTF_FMT_STR "%*s"
#define SNPRINTF snprintf
#define BUFFER_STR str.cstr
#define SNPRINT_N_STRINGS  snprint_n_strings
#define STR_N_SUBSTRING str_n_substring
#define STR_ITER_WIDTH str_iter_width

    size_t buf_len = total_buf_len - strlen(content_style_tag) - strlen(reset_content_style_tag);

    if (buffer == NULL || buffer->str.data == NULL
        || buffer_row >= buffer_text_visible_height(buffer) || buf_len == 0) {
        return -1;
    }

    size_t content_width = buffer_text_visible_width(buffer);
    if ((buf_len - 1) < content_width)
        return -1;

    size_t left = 0;
    size_t right = 0;

    switch (get_cell_property_value_hierarcial(context->table_properties, context->row, context->column, FT_CPROP_TEXT_ALIGN)) {
        case FT_ALIGNED_LEFT:
            left = 0;
            right = (buf_len - 1) - content_width;
            break;
        case FT_ALIGNED_CENTER:
            left = ((buf_len - 1) - content_width) / 2;
            right = ((buf_len - 1) - content_width) - left;
            break;
        case FT_ALIGNED_RIGHT:
            left = (buf_len - 1) - content_width;
            right = 0;
            break;
        default:
            assert(0);
            break;
    }

    int set_old_value = 0;
    size_t  written = 0;
    int tmp = 0;
    ptrdiff_t str_it_width = 0;
    const CHAR_TYPE *beg = NULL;
    const CHAR_TYPE *end = NULL;
    CHAR_TYPE old_value = (CHAR_TYPE)0;

    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, left, SPACE_CHAR));

    STR_N_SUBSTRING(buffer->BUFFER_STR, NEWLINE_CHAR, buffer_row, &beg, &end);
    if (beg == NULL || end == NULL)
        return -1;
    old_value = *end;
    *(CHAR_TYPE *)end = NULL_CHAR;
    set_old_value = 1;

    str_it_width = STR_ITER_WIDTH(beg, end);
    if (str_it_width < 0 || content_width < (size_t)str_it_width)
        goto  clear;

    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, 1, content_style_tag));
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINTF(buf + written, total_buf_len - written, SNPRINTF_FMT_STR, (int)(end - beg), beg));
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, 1, reset_content_style_tag));

    *(CHAR_TYPE *)end = old_value;
    set_old_value = 0;
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written,  total_buf_len - written, (content_width - (size_t)str_it_width), SPACE_CHAR));
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, right, SPACE_CHAR));
    return (int)written;

clear:
    if (set_old_value)
        *(CHAR_TYPE *)end = old_value;
    return -1;

#undef CHAR_TYPE
#undef NULL_CHAR
#undef NEWLINE_CHAR
#undef SPACE_CHAR
#undef SNPRINTF_FMT_STR
#undef SNPRINTF
#undef BUFFER_STR
#undef SNPRINT_N_STRINGS
#undef STR_N_SUBSTRING
#undef STR_ITER_WIDTH
}


#ifdef FT_HAVE_WCHAR
FT_INTERNAL
int buffer_wprintf(string_buffer_t *buffer, size_t buffer_row, wchar_t *buf, size_t total_buf_len,
                   const context_t *context, const char *content_style_tag, const char *reset_content_style_tag)
{
#define CHAR_TYPE wchar_t
#define NULL_CHAR L'\0'
#define NEWLINE_CHAR L'\n'
#define SPACE_CHAR " "
#define SNPRINTF_FMT_STR L"%*ls"
#define SNPRINTF swprintf
#define BUFFER_STR str.wstr
#define SNPRINT_N_STRINGS  wsnprint_n_string
#define STR_N_SUBSTRING wstr_n_substring
#define STR_ITER_WIDTH wcs_iter_width

    size_t buf_len = total_buf_len - strlen(content_style_tag) - strlen(reset_content_style_tag);

    if (buffer == NULL || buffer->str.data == NULL
        || buffer_row >= buffer_text_visible_height(buffer) || buf_len == 0) {
        return -1;
    }

    size_t content_width = buffer_text_visible_width(buffer);
    if ((buf_len - 1) < content_width)
        return -1;

    size_t left = 0;
    size_t right = 0;

    switch (get_cell_property_value_hierarcial(context->table_properties, context->row, context->column, FT_CPROP_TEXT_ALIGN)) {
        case FT_ALIGNED_LEFT:
            left = 0;
            right = (buf_len - 1) - content_width;
            break;
        case FT_ALIGNED_CENTER:
            left = ((buf_len - 1) - content_width) / 2;
            right = ((buf_len - 1) - content_width) - left;
            break;
        case FT_ALIGNED_RIGHT:
            left = (buf_len - 1) - content_width;
            right = 0;
            break;
        default:
            assert(0);
            break;
    }

    int set_old_value = 0;
    size_t  written = 0;
    int tmp = 0;
    ptrdiff_t str_it_width = 0;
    const CHAR_TYPE *beg = NULL;
    const CHAR_TYPE *end = NULL;
    CHAR_TYPE old_value = (CHAR_TYPE)0;

    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, left, SPACE_CHAR));

    STR_N_SUBSTRING(buffer->BUFFER_STR, NEWLINE_CHAR, buffer_row, &beg, &end);
    if (beg == NULL || end == NULL)
        return -1;
    old_value = *end;
    *(CHAR_TYPE *)end = NULL_CHAR;
    set_old_value = 1;

    str_it_width = STR_ITER_WIDTH(beg, end);
    if (str_it_width < 0 || content_width < (size_t)str_it_width)
        goto  clear;

    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, 1, content_style_tag));
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINTF(buf + written, total_buf_len - written, SNPRINTF_FMT_STR, (int)(end - beg), beg));
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, 1, reset_content_style_tag));

    *(CHAR_TYPE *)end = old_value;
    set_old_value = 0;
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written,  total_buf_len - written, (content_width - (size_t)str_it_width), SPACE_CHAR));
    CHCK_RSLT_ADD_TO_WRITTEN(SNPRINT_N_STRINGS(buf + written, total_buf_len - written, right, SPACE_CHAR));
    return (int)written;

clear:
    if (set_old_value)
        *(CHAR_TYPE *)end = old_value;
    return -1;

#undef CHAR_TYPE
#undef NULL_CHAR
#undef NEWLINE_CHAR
#undef SPACE_CHAR
#undef SNPRINTF_FMT_STR
#undef SNPRINTF
#undef BUFFER_STR
#undef SNPRINT_N_STRINGS
#undef STR_N_SUBSTRING
#undef STR_ITER_WIDTH
}
#endif /* FT_HAVE_WCHAR */


FT_INTERNAL
size_t string_buffer_width_capacity(const string_buffer_t *buffer)
{
    assert(buffer);

    if (buffer->type == CHAR_BUF)
        return buffer->data_sz;
#ifdef FT_HAVE_WCHAR
    else if (buffer->type == UTF8_BUF)
        return buffer->data_sz / sizeof(wchar_t);
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
    else
        return buffer->data_sz / 4;  /* todo: Maybe better implementation ?? */
#endif /* FT_HAVE_WCHAR */

    assert(0 && "Shouldn't be here");
    return 0;
}


FT_INTERNAL
void *buffer_get_data(string_buffer_t *buffer)
{
    assert(buffer);
    return buffer->str.data;
}
