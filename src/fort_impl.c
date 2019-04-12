/*
libfort

MIT License

Copyright (c) 2017 - 2018 Seleznev Anton

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "fort.h"
#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>

#include "vector.h"
#include "fort_utils.h"
#include "string_buffer.h"
#include "table.h"
#include "row.h"
#include "properties.h"


ft_table_t *ft_create_table(void)
{
    ft_table_t *result = (ft_table_t *)F_CALLOC(1, sizeof(ft_table_t));
    if (result == NULL)
        return NULL;

    result->rows = create_vector(sizeof(fort_row_t *), DEFAULT_VECTOR_CAPACITY);
    if (result->rows == NULL) {
        F_FREE(result);
        return NULL;
    }
    result->separators = create_vector(sizeof(separator_t *), DEFAULT_VECTOR_CAPACITY);
    if (result->separators == NULL) {
        destroy_vector(result->rows);
        F_FREE(result);
        return NULL;
    }
    result->properties = NULL;
    result->conv_buffer = NULL;
    result->cur_row = 0;
    result->cur_col = 0;
    result->char_type = FT_UNDEF_CHAR;
    return result;
}


void ft_destroy_table(ft_table_t *table)
{
    size_t i = 0;

    if (table == NULL)
        return;

    if (table->rows) {
        size_t row_n = vector_size(table->rows);
        for (i = 0; i < row_n; ++i) {
            destroy_row(*(fort_row_t **)vector_at(table->rows, i));
        }
        destroy_vector(table->rows);
    }
    if (table->separators) {
        size_t row_n = vector_size(table->separators);
        for (i = 0; i < row_n; ++i) {
            destroy_separator(*(separator_t **)vector_at(table->separators, i));
        }
        destroy_vector(table->separators);
    }
    destroy_table_properties(table->properties);
    destroy_string_buffer(table->conv_buffer);
    F_FREE(table);
}

ft_table_t *ft_copy_table(ft_table_t *table)
{
    if (table == NULL)
        return NULL;

    ft_table_t *result = ft_create_table();
    if (result == NULL)
        return NULL;

    size_t i = 0;
    size_t rows_n = vector_size(table->rows);
    for (i = 0; i < rows_n; ++i) {
        fort_row_t *row = *(fort_row_t **)vector_at(table->rows, i);
        fort_row_t *new_row = copy_row(row);
        if (new_row == NULL) {
            ft_destroy_table(result);
            return NULL;
        }
        vector_push(result->rows, &new_row);
    }

    size_t sep_sz = vector_size(table->separators);
    for (i = 0; i < sep_sz; ++i) {
        separator_t *sep = *(separator_t **)vector_at(table->separators, i);
        separator_t *new_sep = copy_separator(sep);
        if (new_sep == NULL) {
            ft_destroy_table(result);
            return NULL;
        }
        vector_push(result->separators, &new_sep);
    }


    result->properties = copy_table_properties(table->properties);
    if (result->properties == NULL) {
        ft_destroy_table(result);
        return NULL;
    }

    /* todo: copy conv_buffer  ??  */

    result->cur_row = table->cur_row;
    result->cur_col = table->cur_col;
    result->char_type = table->char_type;
    return result;
}


void ft_ln(ft_table_t *table)
{
    assert(table);
    table->cur_col = 0;
    table->cur_row++;
}

size_t ft_cur_row(ft_table_t *table)
{
    assert(table);
    return table->cur_row;
}

size_t ft_cur_col(ft_table_t *table)
{
    assert(table);
    return table->cur_col;
}

void ft_set_cur_cell(ft_table_t *table, size_t row, size_t col)
{
    assert(table);
    table->cur_row = row;
    table->cur_col = col;
}

FT_PRINTF_ATTRIBUTE_FORMAT(3, 0)
static int ft_row_printf_impl(ft_table_t *table, size_t row, const char *fmt, va_list *va)
{
#define CREATE_ROW_FROM_FMT_STRING create_row_from_fmt_string
    size_t i = 0;
    size_t new_cols = 0;

    if (table == NULL)
        return -1;

    fort_row_t *new_row = CREATE_ROW_FROM_FMT_STRING(fmt, va);

    if (new_row == NULL) {
        return -1;
    }

    fort_row_t **cur_row_p = NULL;
    size_t sz = vector_size(table->rows);
    if (row >= sz) {
        size_t push_n = row - sz + 1;
        for (i = 0; i < push_n; ++i) {
            fort_row_t *padding_row = create_row();
            if (padding_row == NULL)
                goto clear;

            if (FT_IS_ERROR(vector_push(table->rows, &padding_row))) {
                destroy_row(padding_row);
                goto clear;
            }
        }
    }
    /* todo: clearing pushed items in case of error ?? */

    new_cols = columns_in_row(new_row);
    cur_row_p = (fort_row_t **)vector_at(table->rows, row);
    swap_row(*cur_row_p, new_row, table->cur_col);

    table->cur_col += new_cols;
    destroy_row(new_row);
    return (int)new_cols;

clear:
    destroy_row(new_row);
    return -1;
#undef CREATE_ROW_FROM_FMT_STRING
}

#ifdef FT_HAVE_WCHAR
static int ft_row_wprintf_impl(ft_table_t *table, size_t row, const wchar_t *fmt, va_list *va)
{
#define CREATE_ROW_FROM_FMT_STRING create_row_from_fmt_wstring
    size_t i = 0;
    size_t new_cols = 0;

    if (table == NULL)
        return -1;

    fort_row_t *new_row = CREATE_ROW_FROM_FMT_STRING(fmt, va);

    if (new_row == NULL) {
        return -1;
    }

    fort_row_t **cur_row_p = NULL;
    size_t sz = vector_size(table->rows);
    if (row >= sz) {
        size_t push_n = row - sz + 1;
        for (i = 0; i < push_n; ++i) {
            fort_row_t *padding_row = create_row();
            if (padding_row == NULL)
                goto clear;

            if (FT_IS_ERROR(vector_push(table->rows, &padding_row))) {
                destroy_row(padding_row);
                goto clear;
            }
        }
    }
    /* todo: clearing pushed items in case of error ?? */

    new_cols = columns_in_row(new_row);
    cur_row_p = (fort_row_t **)vector_at(table->rows, row);
    swap_row(*cur_row_p, new_row, table->cur_col);

    table->cur_col += new_cols;
    destroy_row(new_row);
    return (int)new_cols;

clear:
    destroy_row(new_row);
    return -1;
#undef CREATE_ROW_FROM_FMT_STRING
}
#endif

#if defined(FT_CLANG_COMPILER) || defined(FT_GCC_COMPILER)
#define FT_PRINTF ft_printf
#define FT_PRINTF_LN ft_printf_ln
#else
#define FT_PRINTF ft_printf_impl
#define FT_PRINTF_LN ft_printf_ln_impl
#endif



int FT_PRINTF(ft_table_t *table, const char *fmt, ...)
{
    assert(table);
    va_list va;
    va_start(va, fmt);
    int result = ft_row_printf_impl(table, table->cur_row, fmt, &va);
    va_end(va);
    return result;
}

int FT_PRINTF_LN(ft_table_t *table, const char *fmt, ...)
{
    assert(table);
    va_list va;
    va_start(va, fmt);
    int result = ft_row_printf_impl(table, table->cur_row, fmt, &va);
    if (result >= 0) {
        ft_ln(table);
    }
    va_end(va);
    return result;
}

#undef FT_PRINTF
#undef FT_PRINTF_LN
#undef FT_HDR_PRINTF
#undef FT_HDR_PRINTF_LN

#ifdef FT_HAVE_WCHAR
int ft_wprintf(ft_table_t *table, const wchar_t *fmt, ...)
{
    assert(table);
    va_list va;
    va_start(va, fmt);
    int result = ft_row_wprintf_impl(table, table->cur_row, fmt, &va);
    va_end(va);
    return result;
}

int ft_wprintf_ln(ft_table_t *table, const wchar_t *fmt, ...)
{
    assert(table);
    va_list va;
    va_start(va, fmt);
    int result = ft_row_wprintf_impl(table, table->cur_row, fmt, &va);
    if (result >= 0) {
        ft_ln(table);
    }
    va_end(va);
    return result;
}

#endif

void ft_set_default_printf_field_separator(char separator)
{
    g_col_separator = separator;
}


static int ft_write_impl(ft_table_t *table, const char *cell_content)
{
    assert(table);
    string_buffer_t *str_buffer = get_cur_str_buffer_and_create_if_not_exists(table);
    if (str_buffer == NULL)
        return FT_ERROR;

    int status = fill_buffer_from_string(str_buffer, cell_content);
    if (FT_IS_SUCCESS(status)) {
        table->cur_col++;
    }
    return status;
}


#ifdef FT_HAVE_WCHAR

static int ft_wwrite_impl(ft_table_t *table, const wchar_t *cell_content)
{
    assert(table);
    string_buffer_t *str_buffer = get_cur_str_buffer_and_create_if_not_exists(table);
    if (str_buffer == NULL)
        return FT_ERROR;

    int status = fill_buffer_from_wstring(str_buffer, cell_content);
    if (FT_IS_SUCCESS(status)) {
        table->cur_col++;
    }
    return status;
}

#endif


int ft_nwrite(ft_table_t *table, size_t count, const char *cell_content, ...)
{
    size_t i = 0;
    assert(table);
    int status = ft_write_impl(table, cell_content);
    if (FT_IS_ERROR(status))
        return status;

    va_list va;
    va_start(va, cell_content);
    --count;
    for (i = 0; i < count; ++i) {
        const char *cell = va_arg(va, const char *);
        status = ft_write_impl(table, cell);
        if (FT_IS_ERROR(status)) {
            va_end(va);
            return status;
        }
    }
    va_end(va);
    return status;
}

int ft_nwrite_ln(ft_table_t *table, size_t count, const char *cell_content, ...)
{
    size_t i = 0;
    assert(table);
    int status = ft_write_impl(table, cell_content);
    if (FT_IS_ERROR(status))
        return status;

    va_list va;
    va_start(va, cell_content);
    --count;
    for (i = 0; i < count; ++i) {
        const char *cell = va_arg(va, const char *);
        status = ft_write_impl(table, cell);
        if (FT_IS_ERROR(status)) {
            va_end(va);
            return status;
        }
    }
    va_end(va);

    ft_ln(table);
    return status;
}

#ifdef FT_HAVE_WCHAR

int ft_nwwrite(ft_table_t *table, size_t n, const wchar_t *cell_content, ...)
{
    size_t i = 0;
    assert(table);
    int status = ft_wwrite_impl(table, cell_content);
    if (FT_IS_ERROR(status))
        return status;

    va_list va;
    va_start(va, cell_content);
    --n;
    for (i = 0; i < n; ++i) {
        const wchar_t *cell = va_arg(va, const wchar_t *);
        status = ft_wwrite_impl(table, cell);
        if (FT_IS_ERROR(status)) {
            va_end(va);
            return status;
        }
    }
    va_end(va);
    return status;
}

int ft_nwwrite_ln(ft_table_t *table, size_t n, const wchar_t *cell_content, ...)
{
    size_t i = 0;
    assert(table);
    int status = ft_wwrite_impl(table, cell_content);
    if (FT_IS_ERROR(status))
        return status;

    va_list va;
    va_start(va, cell_content);
    --n;
    for (i = 0; i < n; ++i) {
        const wchar_t *cell = va_arg(va, const wchar_t *);
        status = ft_wwrite_impl(table, cell);
        if (FT_IS_ERROR(status)) {
            va_end(va);
            return status;
        }
    }
    va_end(va);

    ft_ln(table);
    return status;
}
#endif


int ft_row_write(ft_table_t *table, size_t cols, const char *cells[])
{
    size_t i = 0;
    assert(table);
    for (i = 0; i < cols; ++i) {
        int status = ft_write_impl(table, cells[i]);
        if (FT_IS_ERROR(status)) {
            /* todo: maybe current pos in case of error should be equal to the one before function call? */
            return status;
        }
    }
    return FT_SUCCESS;
}

int ft_row_write_ln(ft_table_t *table, size_t cols, const char *cells[])
{
    assert(table);
    int status = ft_row_write(table, cols, cells);
    if (FT_IS_SUCCESS(status)) {
        ft_ln(table);
    }
    return status;
}

#ifdef FT_HAVE_WCHAR
int ft_row_wwrite(ft_table_t *table, size_t cols, const wchar_t *cells[])
{
    size_t i = 0;
    assert(table);
    for (i = 0; i < cols; ++i) {
        int status = ft_wwrite_impl(table, cells[i]);
        if (FT_IS_ERROR(status)) {
            /* todo: maybe current pos in case of error should be equal
             * to the one before function call?
             */
            return status;
        }
    }
    return FT_SUCCESS;
}

int ft_row_wwrite_ln(ft_table_t *table, size_t cols, const wchar_t *cells[])
{
    assert(table);
    int status = ft_row_wwrite(table, cols, cells);
    if (FT_IS_SUCCESS(status)) {
        ft_ln(table);
    }
    return status;
}
#endif



int ft_table_write(ft_table_t *table, size_t rows, size_t cols, const char *table_cells[])
{
    size_t i = 0;
    assert(table);
    for (i = 0; i < rows; ++i) {
        int status = ft_row_write(table, cols, (const char **)&table_cells[i * cols]);
        if (FT_IS_ERROR(status)) {
            /* todo: maybe current pos in case of error should be equal
             * to the one before function call?
             */
            return status;
        }
        if (i != rows - 1)
            ft_ln(table);
    }
    return FT_SUCCESS;
}

int ft_table_write_ln(ft_table_t *table, size_t rows, size_t cols, const char *table_cells[])
{
    assert(table);
    int status = ft_table_write(table, rows, cols, table_cells);
    if (FT_IS_SUCCESS(status)) {
        ft_ln(table);
    }
    return status;
}


#ifdef FT_HAVE_WCHAR
int ft_table_wwrite(ft_table_t *table, size_t rows, size_t cols, const wchar_t *table_cells[])
{
    size_t i = 0;
    assert(table);
    for (i = 0; i < rows; ++i) {
        int status = ft_row_wwrite(table, cols, (const wchar_t **)&table_cells[i * cols]);
        if (FT_IS_ERROR(status)) {
            /* todo: maybe current pos in case of error should be equal
             * to the one before function call?
             */
            return status;
        }
        if (i != rows - 1)
            ft_ln(table);
    }
    return FT_SUCCESS;
}

int ft_table_wwrite_ln(ft_table_t *table, size_t rows, size_t cols, const wchar_t *table_cells[])
{
    assert(table);
    int status = ft_table_wwrite(table, rows, cols, table_cells);
    if (FT_IS_SUCCESS(status)) {
        ft_ln(table);
    }
    return status;
}
#endif



const char *ft_to_string(const ft_table_t *table)
{
    typedef char char_type;
    const enum str_buf_type buf_type = CHAR_BUF;
    const char *space_char = " ";
    const char *new_line_char = "\n";
#define EMPTY_STRING ""
    int (*snprintf_row_)(const fort_row_t *, char *, size_t, size_t *, size_t, size_t, const context_t *) = snprintf_row;
    int (*print_row_separator_)(char *, size_t,
                                const size_t *, size_t,
                                const fort_row_t *, const fort_row_t *,
                                enum HorSeparatorPos, const separator_t *,
                                const context_t *) = print_row_separator;
    int (*snprint_n_strings_)(char *, size_t, size_t, const char *) = snprint_n_strings;
    assert(table);

    /* Determing size of table string representation */
    size_t height = 0;
    size_t width = 0;
    int status = table_geometry(table, &height, &width);
    if (FT_IS_ERROR(status)) {
        return NULL;
    }
    size_t sz = height * width + 1;

    /* Allocate string buffer for string representation */
    if (table->conv_buffer == NULL) {
        ((ft_table_t *)table)->conv_buffer = create_string_buffer(sz, buf_type);
        if (table->conv_buffer == NULL)
            return NULL;
    }
    while (string_buffer_width_capacity(table->conv_buffer) < sz) {
        if (FT_IS_ERROR(realloc_string_buffer_without_copy(table->conv_buffer))) {
            return NULL;
        }
    }
    char_type *buffer = (char_type *)buffer_get_data(table->conv_buffer);


    size_t cols = 0;
    size_t rows = 0;
    size_t *col_width_arr = NULL;
    size_t *row_height_arr = NULL;
    status = table_rows_and_cols_geometry(table, &col_width_arr, &cols, &row_height_arr, &rows, VISIBLE_GEOMETRY);
    if (FT_IS_ERROR(status))
        return NULL;

    if (rows == 0)
        return EMPTY_STRING;

    size_t written = 0;
    int tmp = 0;
    size_t i = 0;
    context_t context;
    context.table_properties = (table->properties ? table->properties : &g_table_properties);
    INIT_CONTEXT(&context, sz, string_buffer_raw_bytes_capacity(table->conv_buffer));

    fort_row_t *prev_row = NULL;
    fort_row_t *cur_row = NULL;
    separator_t *cur_sep = NULL;
    size_t sep_size = vector_size(table->separators);

    /* Print top margin */
    for (i = 0; i < context.table_properties->entire_table_properties.top_margin; ++i) {
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, width - 1/* minus new_line*/, space_char));
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, 1, new_line_char));
    }

    for (i = 0; i < rows; ++i) {
        cur_sep = (i < sep_size) ? (*(separator_t **)vector_at(table->separators, i)) : NULL;
        cur_row = *(fort_row_t **)vector_at(table->rows, i);
        enum HorSeparatorPos separatorPos = (i == 0) ? TopSeparator : InsideSeparator;
        context.row = i;
        CHCK_RSLT_ADD_TO_WRITTEN(print_row_separator_(buffer + written, sz - written, col_width_arr, cols, prev_row, cur_row, separatorPos, cur_sep, &context));
        CHCK_RSLT_ADD_TO_WRITTEN(snprintf_row_(cur_row, buffer + written, sz - written, col_width_arr, cols, row_height_arr[i], &context));
        prev_row = cur_row;
    }
    cur_row = NULL;
    cur_sep = (i < sep_size) ? (*(separator_t **)vector_at(table->separators, i)) : NULL;
    CHCK_RSLT_ADD_TO_WRITTEN(print_row_separator_(buffer + written, sz - written, col_width_arr, cols, prev_row, cur_row, BottomSeparator, cur_sep, &context));

    /* Print bottom margin */
    for (i = 0; i < context.table_properties->entire_table_properties.bottom_margin; ++i) {
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, width - 1/* minus new_line*/, space_char));
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, 1, new_line_char));
    }


    F_FREE(col_width_arr);
    F_FREE(row_height_arr);
    return buffer;

clear:
    F_FREE(col_width_arr);
    F_FREE(row_height_arr);
    return NULL;
#undef EMPTY_STRING
}


#ifdef FT_HAVE_WCHAR

const wchar_t *ft_to_wstring(const ft_table_t *table)
{
    typedef wchar_t char_type;
    const enum str_buf_type buf_type = W_CHAR_BUF;
    const char *space_char = " ";
    const char *new_line_char = "\n";
#define EMPTY_STRING L""
    int (*snprintf_row_)(const fort_row_t *, wchar_t *, size_t, size_t *, size_t, size_t, const context_t *) = wsnprintf_row;
    int (*print_row_separator_)(wchar_t *, size_t,
                                const size_t *, size_t,
                                const fort_row_t *, const fort_row_t *,
                                enum HorSeparatorPos, const separator_t *,
                                const context_t *) = wprint_row_separator;
    int (*snprint_n_strings_)(wchar_t *, size_t, size_t, const char *) = wsnprint_n_string;


    assert(table);

    /* Determing size of table string representation */
    size_t height = 0;
    size_t width = 0;
    int status = table_geometry(table, &height, &width);
    if (FT_IS_ERROR(status)) {
        return NULL;
    }
    size_t sz = height * width + 1;

    /* Allocate string buffer for string representation */
    if (table->conv_buffer == NULL) {
        ((ft_table_t *)table)->conv_buffer = create_string_buffer(sz, buf_type);
        if (table->conv_buffer == NULL)
            return NULL;
    }
    while (string_buffer_width_capacity(table->conv_buffer) < sz) {
        if (FT_IS_ERROR(realloc_string_buffer_without_copy(table->conv_buffer))) {
            return NULL;
        }
    }
    char_type *buffer = (char_type *)buffer_get_data(table->conv_buffer);


    size_t cols = 0;
    size_t rows = 0;
    size_t *col_width_arr = NULL;
    size_t *row_height_arr = NULL;
    status = table_rows_and_cols_geometry(table, &col_width_arr, &cols, &row_height_arr, &rows, VISIBLE_GEOMETRY);

    if (rows == 0)
        return EMPTY_STRING;

    if (FT_IS_ERROR(status))
        return NULL;

    size_t written = 0;
    int tmp = 0;
    size_t i = 0;
    context_t context;
    context.table_properties = (table->properties ? table->properties : &g_table_properties);
    fort_row_t *prev_row = NULL;
    fort_row_t *cur_row = NULL;
    separator_t *cur_sep = NULL;
    size_t sep_size = vector_size(table->separators);

    /* Print top margin */
    for (i = 0; i < context.table_properties->entire_table_properties.top_margin; ++i) {
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, width - 1/* minus new_line*/, space_char));
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, 1, new_line_char));
    }

    for (i = 0; i < rows; ++i) {
        cur_sep = (i < sep_size) ? (*(separator_t **)vector_at(table->separators, i)) : NULL;
        cur_row = *(fort_row_t **)vector_at(table->rows, i);
        enum HorSeparatorPos separatorPos = (i == 0) ? TopSeparator : InsideSeparator;
        context.row = i;
        CHCK_RSLT_ADD_TO_WRITTEN(print_row_separator_(buffer + written, sz - written, col_width_arr, cols, prev_row, cur_row, separatorPos, cur_sep, &context));
        CHCK_RSLT_ADD_TO_WRITTEN(snprintf_row_(cur_row, buffer + written, sz - written, col_width_arr, cols, row_height_arr[i], &context));
        prev_row = cur_row;
    }
    cur_row = NULL;
    cur_sep = (i < sep_size) ? (*(separator_t **)vector_at(table->separators, i)) : NULL;
    CHCK_RSLT_ADD_TO_WRITTEN(print_row_separator_(buffer + written, sz - written, col_width_arr, cols, prev_row, cur_row, BottomSeparator, cur_sep, &context));

    /* Print bottom margin */
    for (i = 0; i < context.table_properties->entire_table_properties.bottom_margin; ++i) {
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, width - 1/* minus new_line*/, space_char));
        CHCK_RSLT_ADD_TO_WRITTEN(snprint_n_strings_(buffer + written, sz - written, 1, new_line_char));
    }

    F_FREE(col_width_arr);
    F_FREE(row_height_arr);
    return buffer;

clear:
    F_FREE(col_width_arr);
    F_FREE(row_height_arr);
    return NULL;
#undef EMPTY_STRING
}

#endif


int ft_add_separator(ft_table_t *table)
{
    assert(table);
    assert(table->separators);

    while (vector_size(table->separators) <= table->cur_row) {
        separator_t *sep_p = create_separator(F_FALSE);
        if (sep_p == NULL)
            return FT_MEMORY_ERROR;
        int status = vector_push(table->separators, &sep_p);
        if (FT_IS_ERROR(status))
            return status;
    }

    separator_t **sep_p = (separator_t **)vector_at(table->separators, table->cur_row);
    if (*sep_p == NULL)
        *sep_p = create_separator(F_TRUE);
    else
        (*sep_p)->enabled = F_TRUE;

    if (*sep_p == NULL)
        return FT_ERROR;
    return FT_SUCCESS;
}



struct ft_border_style *const FT_BASIC_STYLE = (struct ft_border_style *) &FORT_BASIC_STYLE;
struct ft_border_style *const FT_BASIC2_STYLE = (struct ft_border_style *) &FORT_BASIC2_STYLE;
struct ft_border_style *const FT_SIMPLE_STYLE = (struct ft_border_style *) &FORT_SIMPLE_STYLE;
struct ft_border_style *const FT_PLAIN_STYLE = (struct ft_border_style *) &FORT_PLAIN_STYLE;
struct ft_border_style *const FT_DOT_STYLE = (struct ft_border_style *) &FORT_DOT_STYLE;
struct ft_border_style *const FT_EMPTY_STYLE  = (struct ft_border_style *) &FORT_EMPTY_STYLE;
struct ft_border_style *const FT_EMPTY2_STYLE  = (struct ft_border_style *) &FORT_EMPTY2_STYLE;
struct ft_border_style *const FT_SOLID_STYLE  = (struct ft_border_style *) &FORT_SOLID_STYLE;
struct ft_border_style *const FT_SOLID_ROUND_STYLE  = (struct ft_border_style *) &FORT_SOLID_ROUND_STYLE;
struct ft_border_style *const FT_NICE_STYLE  = (struct ft_border_style *) &FORT_NICE_STYLE;
struct ft_border_style *const FT_DOUBLE_STYLE  = (struct ft_border_style *) &FORT_DOUBLE_STYLE;
struct ft_border_style *const FT_DOUBLE2_STYLE  = (struct ft_border_style *) &FORT_DOUBLE2_STYLE;
struct ft_border_style *const FT_BOLD_STYLE  = (struct ft_border_style *) &FORT_BOLD_STYLE;
struct ft_border_style *const FT_BOLD2_STYLE  = (struct ft_border_style *) &FORT_BOLD2_STYLE;
struct ft_border_style *const FT_FRAME_STYLE  = (struct ft_border_style *) &FORT_FRAME_STYLE;



static void set_border_props_for_props(fort_table_properties_t *properties, const struct ft_border_style *style)
{
    if ((const struct fort_border_style *)style == &FORT_BASIC_STYLE
        || (const struct fort_border_style *)style == &FORT_BASIC2_STYLE
        || (const struct fort_border_style *)style == &FORT_SIMPLE_STYLE
        || (const struct fort_border_style *)style == &FORT_DOT_STYLE
        || (const struct fort_border_style *)style == &FORT_PLAIN_STYLE
        || (const struct fort_border_style *)style == &FORT_EMPTY_STYLE
        || (const struct fort_border_style *)style == &FORT_EMPTY2_STYLE
        || (const struct fort_border_style *)style == &FORT_SOLID_STYLE
        || (const struct fort_border_style *)style == &FORT_SOLID_ROUND_STYLE
        || (const struct fort_border_style *)style == &FORT_NICE_STYLE
        || (const struct fort_border_style *)style == &FORT_DOUBLE_STYLE
        || (const struct fort_border_style *)style == &FORT_DOUBLE2_STYLE
        || (const struct fort_border_style *)style == &FORT_BOLD_STYLE
        || (const struct fort_border_style *)style == &FORT_BOLD2_STYLE
        || (const struct fort_border_style *)style == &FORT_FRAME_STYLE) {
        memcpy(&(properties->border_style), (const struct fort_border_style *)style, sizeof(struct fort_border_style));
        return;
    }

    const struct ft_border_chars *border_chs = &(style->border_chs);
    const struct ft_border_chars *header_border_chs = &(style->header_border_chs);

#define BOR_CHARS properties->border_style.border_chars
#define H_BOR_CHARS properties->border_style.header_border_chars
#define SEP_CHARS properties->border_style.separator_chars

    /*
    BOR_CHARS[TL_bip] = BOR_CHARS[TT_bip] = BOR_CHARS[TV_bip] = BOR_CHARS[TR_bip] = border_chs->top_border_ch;
    BOR_CHARS[LH_bip] = BOR_CHARS[IH_bip] = BOR_CHARS[II_bip] = BOR_CHARS[RH_bip] = border_chs->separator_ch;
    BOR_CHARS[BL_bip] = BOR_CHARS[BB_bip] = BOR_CHARS[BV_bip] = BOR_CHARS[BR_bip] = border_chs->bottom_border_ch;
    BOR_CHARS[LL_bip] = BOR_CHARS[IV_bip] = BOR_CHARS[RR_bip] = border_chs->side_border_ch;

    H_BOR_CHARS[TL_bip] = H_BOR_CHARS[TT_bip] = H_BOR_CHARS[TV_bip] = H_BOR_CHARS[TR_bip] = header_border_chs->top_border_ch;
    H_BOR_CHARS[LH_bip] = H_BOR_CHARS[IH_bip] = H_BOR_CHARS[II_bip] = H_BOR_CHARS[RH_bip] = header_border_chs->separator_ch;
    H_BOR_CHARS[BL_bip] = H_BOR_CHARS[BB_bip] = H_BOR_CHARS[BV_bip] = H_BOR_CHARS[BR_bip] = header_border_chs->bottom_border_ch;
    H_BOR_CHARS[LL_bip] = H_BOR_CHARS[IV_bip] = H_BOR_CHARS[RR_bip] = header_border_chs->side_border_ch;
    */

    BOR_CHARS[TT_bip] = border_chs->top_border_ch;
    BOR_CHARS[IH_bip] = border_chs->separator_ch;
    BOR_CHARS[BB_bip] = border_chs->bottom_border_ch;
    BOR_CHARS[LL_bip] = BOR_CHARS[IV_bip] = BOR_CHARS[RR_bip] = border_chs->side_border_ch;

    BOR_CHARS[TL_bip] = BOR_CHARS[TV_bip] = BOR_CHARS[TR_bip] = border_chs->out_intersect_ch;
    BOR_CHARS[LH_bip] = BOR_CHARS[RH_bip] = border_chs->out_intersect_ch;
    BOR_CHARS[BL_bip] = BOR_CHARS[BV_bip] = BOR_CHARS[BR_bip] = border_chs->out_intersect_ch;
    BOR_CHARS[II_bip] = border_chs->in_intersect_ch;

    BOR_CHARS[LI_bip] = BOR_CHARS[TI_bip] = BOR_CHARS[RI_bip] = BOR_CHARS[BI_bip] = border_chs->in_intersect_ch;

    /*
    if (border_chs->separator_ch == '\0' && border_chs->in_intersect_ch == '\0') {
        BOR_CHARS[LH_bip] = BOR_CHARS[RH_bip] = '\0';
    }
    */
    if (strlen(border_chs->separator_ch) == 0 && strlen(border_chs->in_intersect_ch) == 0) {
        BOR_CHARS[LH_bip] = BOR_CHARS[RH_bip] = "\0";
    }


    H_BOR_CHARS[TT_bip] = header_border_chs->top_border_ch;
    H_BOR_CHARS[IH_bip] = header_border_chs->separator_ch;
    H_BOR_CHARS[BB_bip] = header_border_chs->bottom_border_ch;
    H_BOR_CHARS[LL_bip] = H_BOR_CHARS[IV_bip] = H_BOR_CHARS[RR_bip] = header_border_chs->side_border_ch;

    H_BOR_CHARS[TL_bip] = H_BOR_CHARS[TV_bip] = H_BOR_CHARS[TR_bip] = header_border_chs->out_intersect_ch;
    H_BOR_CHARS[LH_bip] = H_BOR_CHARS[RH_bip] = header_border_chs->out_intersect_ch;
    H_BOR_CHARS[BL_bip] = H_BOR_CHARS[BV_bip] = H_BOR_CHARS[BR_bip] = header_border_chs->out_intersect_ch;
    H_BOR_CHARS[II_bip] = header_border_chs->in_intersect_ch;

    H_BOR_CHARS[LI_bip] = H_BOR_CHARS[TI_bip] = H_BOR_CHARS[RI_bip] = H_BOR_CHARS[BI_bip] = header_border_chs->in_intersect_ch;

    /*
    if (header_border_chs->separator_ch == '\0' && header_border_chs->in_intersect_ch == '\0') {
        H_BOR_CHARS[LH_bip] = H_BOR_CHARS[RH_bip] = '\0';
    }
    */
    if (strlen(header_border_chs->separator_ch) == 0 && strlen(header_border_chs->in_intersect_ch) == 0) {
        BOR_CHARS[LH_bip] = BOR_CHARS[RH_bip] = "\0";
    }

    SEP_CHARS[LH_sip] = SEP_CHARS[RH_sip] = SEP_CHARS[II_sip] = header_border_chs->out_intersect_ch;
    SEP_CHARS[TI_sip] = SEP_CHARS[BI_sip] = header_border_chs->out_intersect_ch;
    SEP_CHARS[IH_sip] = style->hor_separator_char;


#undef BOR_CHARS
#undef H_BOR_CHARS
#undef SEP_CHARS
}


int ft_set_default_border_style(const struct ft_border_style *style)
{
    set_border_props_for_props(&g_table_properties, style);
    return FT_SUCCESS;
}

int ft_set_border_style(ft_table_t *table, const struct ft_border_style *style)
{
    assert(table);
    if (table->properties == NULL) {
        table->properties = create_table_properties();
        if (table->properties == NULL)
            return FT_MEMORY_ERROR;
    }
    set_border_props_for_props(table->properties, style);
    return FT_SUCCESS;
}



int ft_set_cell_prop(ft_table_t *table, size_t row, size_t col, uint32_t property, int value)
{
    assert(table);

    if (table->properties == NULL) {
        table->properties = create_table_properties();
        if (table->properties == NULL)
            return FT_MEMORY_ERROR;
    }
    if (table->properties->cell_properties == NULL) {
        table->properties->cell_properties = create_cell_prop_container();
        if (table->properties->cell_properties == NULL) {
            return FT_ERROR;
        }
    }

    if (row == FT_CUR_ROW)
        row = table->cur_row;
    if (row == FT_CUR_COLUMN)
        col = table->cur_col;

    return set_cell_property(table->properties->cell_properties, row, col, property, value);
}

int ft_set_default_cell_prop(uint32_t property, int value)
{
    return set_default_cell_property(property, value);
}


int ft_set_default_tbl_prop(uint32_t property, int value)
{
    return set_default_entire_table_property(property, value);
}

int ft_set_tbl_prop(ft_table_t *table, uint32_t property, int value)
{
    assert(table);

    if (table->properties == NULL) {
        table->properties = create_table_properties();
        if (table->properties == NULL)
            return FT_MEMORY_ERROR;
    }
    return set_entire_table_property(table->properties, property, value);
}

void ft_set_memory_funcs(void *(*f_malloc)(size_t size), void (*f_free)(void *ptr))
{
    set_memory_funcs(f_malloc, f_free);
}

int ft_set_cell_span(ft_table_t *table, size_t row, size_t col, size_t hor_span)
{
    assert(table);
    if (hor_span < 2)
        return FT_EINVAL;

    if (row == FT_CUR_ROW)
        row = table->cur_row;
    if (row == FT_CUR_COLUMN)
        col = table->cur_col;

    fort_row_t *row_p = get_row_and_create_if_not_exists(table, row);
    if (row_p == NULL)
        return FT_ERROR;

    return row_set_cell_span(row_p, col, hor_span);
}
