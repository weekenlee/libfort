#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include "fort_utils.h"

struct string_buffer {
    union {
        char *cstr;
#ifdef FT_HAVE_WCHAR
        wchar_t *wstr;
#endif
#ifdef FT_HAVE_UTF8
        void *u8str;
#endif
        void *data;
    } str;
    size_t data_sz;
    enum str_buf_type type;
};

FT_INTERNAL
string_buffer_t *create_string_buffer(size_t number_of_chars, enum str_buf_type type);

FT_INTERNAL
void destroy_string_buffer(string_buffer_t *buffer);

FT_INTERNAL
string_buffer_t *copy_string_buffer(const string_buffer_t *buffer);

FT_INTERNAL
fort_status_t realloc_string_buffer_without_copy(string_buffer_t *buffer);

FT_INTERNAL
fort_status_t fill_buffer_from_string(string_buffer_t *buffer, const char *str);

#ifdef FT_HAVE_WCHAR
FT_INTERNAL
fort_status_t fill_buffer_from_wstring(string_buffer_t *buffer, const wchar_t *str);
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
//FT_INTERNAL
fort_status_t fill_buffer_from_u8string(string_buffer_t *buffer, const void *str);
#endif /* FT_HAVE_UTF8 */


FT_INTERNAL
size_t buffer_text_visible_height(const string_buffer_t *buffer);

FT_INTERNAL
size_t string_buffer_width_capacity(const string_buffer_t *buffer);

#define string_buffer_raw_bytes_capacity(buffer) \
    buffer->data_sz

FT_INTERNAL
void *buffer_get_data(string_buffer_t *buffer);

FT_INTERNAL
size_t buffer_text_visible_width(const string_buffer_t *buffer);

FT_INTERNAL
int buffer_printf(string_buffer_t *buffer, size_t buffer_row, char *buf, size_t total_buf_len,
                  const context_t *context, const char *content_style_tag, const char *reset_content_style_tag);

#ifdef FT_HAVE_WCHAR
FT_INTERNAL
int buffer_wprintf(string_buffer_t *buffer, size_t buffer_row, wchar_t *buf, size_t total_buf_len,
                   const context_t *context, const char *content_style_tag, const char *reset_content_style_tag);
#endif /* FT_HAVE_WCHAR */
#ifdef FT_HAVE_UTF8
//FT_INTERNAL
//int buffer_u8printf(string_buffer_t *buffer, size_t buffer_row, void *buf, size_t total_buf_len,
//                   const context_t *context, const char *content_style_tag, const char *reset_content_style_tag) __attribute__((unused));
#endif /* FT_HAVE_UTF8 */

#endif /* STRING_BUFFER_H */
