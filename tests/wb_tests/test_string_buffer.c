#include "tests.h"
#include "string_buffer.h"
#include <wchar.h>
#if defined(FT_HAVE_UTF8)
#include "utf8.h"
#endif

size_t strchr_count(const char *str, char ch);
size_t wstrchr_count(const wchar_t *str, wchar_t ch);
size_t utf8chr_count(const void *str, utf8_int32_t ch);


const char *str_n_substring_beg(const char *str, char ch_separator, size_t n);
const wchar_t *wstr_n_substring_beg(const wchar_t *str, wchar_t ch_separator, size_t n);
#ifdef FT_HAVE_UTF8
const void *utf8_n_substring_beg(const void *str, utf8_int32_t ch_separator, size_t n);
#endif

fort_status_t str_n_substring(const char *str, char ch_separator, size_t n, const char **begin, const char **end);
void wstr_n_substring(const wchar_t *str, wchar_t ch_separator, size_t n, const wchar_t **begin, const wchar_t **end);


//size_t buffer_text_visible_width(string_buffer_t *buffer);


void test_strchr_count(void);
void test_str_n_substring(void);
void test_buffer_text_visible_width(void);
void test_buffer_text_visible_height(void);


void test_string_buffer(void)
{
    test_strchr_count();
    test_str_n_substring();
    test_buffer_text_visible_width();
    test_buffer_text_visible_height();
}


void test_strchr_count(void)
{
    assert_true(strchr_count(NULL, '\n') == 0);
    assert_true(strchr_count("", '\n') == 0);
    assert_true(strchr_count("asbd", '\n') == 0);

    assert_true(strchr_count("asbd\n", '\n') == 1);
    assert_true(strchr_count("\nasbd", '\n') == 1);
    assert_true(strchr_count("a\nsbd", '\n') == 1);

    assert_true(strchr_count("\n12\n123", '\n') == 2);
    assert_true(strchr_count("\n12\n123\n", '\n') == 3);
    assert_true(strchr_count("\n\n\n", '\n') == 3);
    assert_true(strchr_count("\n123123\n123123\n\n\n123", '\n') == 5);

    assert_true(strchr_count("1a23123a123123aaa123", 'a') == 5);


    assert_true(wstrchr_count(NULL, L'\n') == 0);
    assert_true(wstrchr_count(L"", L'\n') == 0);
    assert_true(wstrchr_count(L"asbd", L'\n') == 0);

    assert_true(wstrchr_count(L"asbd\n", L'\n') == 1);
    assert_true(wstrchr_count(L"\nasbd", L'\n') == 1);
    assert_true(wstrchr_count(L"a\nsbd", L'\n') == 1);

    assert_true(wstrchr_count(L"\n12\n123", L'\n') == 2);
    assert_true(wstrchr_count(L"\n12\n123\n", L'\n') == 3);
    assert_true(wstrchr_count(L"\n\n\n", '\n') == 3);
    assert_true(wstrchr_count(L"\n123123\n123123\n\n\n123", L'\n') == 5);

    assert_true(wstrchr_count(L"1\xffffy23123\xffffy123123\xffffy\xffffy\xffffy123", L'\xffff') == 5);


#ifdef FT_HAVE_UTF8
    assert_true(utf8chr_count(NULL, '\n') == 0);
    assert_true(utf8chr_count("", '\n') == 0);
    assert_true(utf8chr_count("asbd", '\n') == 0);

    assert_true(utf8chr_count("Chinese L視野無限廣，窗外有藍天", '\n') == 0);
    assert_true(utf8chr_count("Hindi ऋषियों को सताने वाले दुष्ट राक्षसों के राजा रावण का सर्वनाश करने वाले विष्णुवतार भगवान श्रीराम, अयोध्या के महाराज दशरथ के बड़े सपुत्र थे।", '\n') == 0);
    assert_true(utf8chr_count("Portuguese Luís argüia à Júlia que «brações, fé, chá, óxido, pôr, zângão» eram palavras do português.", '\n') == 0);
    assert_true(utf8chr_count("Russian В чащах юга жил бы цитрус? Да, но фальшивый экземпляръ!", '\n') == 0);
    assert_true(utf8chr_count("Spanish La niña, viéndose atrapada en el áspero baúl índigo y sintiendo asfixia, lloró de vergüenza; mientras que la frustrada madre llamaba a su hija diciendo: ¿Dónde estás Waleska?", '\n') == 0);

    assert_true(utf8chr_count("asbd\n", '\n') == 1);
    assert_true(utf8chr_count("\nasbd", '\n') == 1);
    assert_true(utf8chr_count("a\nsbd", '\n') == 1);
    assert_true(utf8chr_count("Chinese L視野無限\n廣，窗外有藍天", '\n') == 1);
    assert_true(utf8chr_count("Hindi ऋषियों को सताने वा\nले दुष्ट राक्षसों के राजा रावण का सर्वनाश करने वाले विष्णुवतार भगवान श्रीराम, अयोध्या के महाराज दशरथ के बड़े सपुत्र थे।", '\n') == 1);
    assert_true(utf8chr_count("Portuguese Luís argüi\na à Júlia que «brações, fé, chá, óxido, pôr, zângão» eram palavras do português.", '\n') == 1);
    assert_true(utf8chr_count("Russian В чащах \nюга жил бы цитрус? Да, но фальшивый экземпляръ!", '\n') == 1);
    assert_true(utf8chr_count("Spanish La niña, vié\nndose atrapada en el áspero baúl índigo y sintiendo asfixia, lloró de vergüenza; mientras que la frustrada madre llamaba a su hija diciendo: ¿Dónde estás Waleska?", '\n') == 1);

    assert_true(utf8chr_count("\n12\n123", '\n') == 2);
    assert_true(utf8chr_count("\n12\n123\n", '\n') == 3);
    assert_true(utf8chr_count("\n\n\n", '\n') == 3);
    assert_true(utf8chr_count("\n123123\n123123\n\n\n123", '\n') == 5);
    assert_true(utf8chr_count("Chinese L視野無限\n廣，窗外有\n藍天", '\n') == 2);
    assert_true(utf8chr_count("Hindi ऋषियों को सताने वा\nले दुष्ट राक्षसों के राजा\n रावण का सर्वना\nश करने वाले विष्णुवतार भगवान श्रीराम, अयोध्या के महाराज दशरथ के बड़े सपुत्र थे।", '\n') == 3);
    assert_true(utf8chr_count("Portuguese Luís argüi\na à Júlia que «brações, fé, chá, óxido, \npôr, zângão» eram pal\navras do portu\nguês.", '\n') == 4);
    assert_true(utf8chr_count("Russian В чащах \nюга жил бы ц\nитрус? Да, но фальшивый экземпляръ!", '\n') == 2);
    assert_true(utf8chr_count("Spanish La niña, vié\nndose atrapada en el \n\n\náspero baúl índigo y \nsintiendo asfixia, lloró de vergüenza; mientras que la frustrada madre llamaba a su hija diciendo: ¿Dónde estás Waleska?", '\n') == 5);

    assert_true(utf8chr_count("1a23123a123123aaa123", 'a') == 5);
#endif
}

void assert_str_equal_strong(const char *str1, const char *str2,
                             const char *file,
                             int line,
                             const char *function)
{
    if (!str1 && !str2)
        return;

    if ((str1 && !str2) || (!str1 && str2)) {
        fprintf(stderr, "%s:%d(%s):Abort! Not equals strings:\n", file, line, function);
        exit(EXIT_FAILURE);
    } else if (strcmp(str1, str2) != 0) {
        fprintf(stderr, "%s:%d(%s):Abort! Not equals strings:\n", file, line, function);
        exit(EXIT_FAILURE);
    }
}

#define ASSERT_STR_EQUAL_STRONG(str1, str2) assert_str_equal_strong(str1, str2, __FILE__,__LINE__, __func__)

void test_str_n_substring(void)
{
    const char *empty_str = "";
    assert_true(str_n_substring_beg(empty_str, '\n', 0) == empty_str);
    assert_true(str_n_substring_beg(empty_str, '\n', 1) == NULL);
    assert_true(str_n_substring_beg(empty_str, '\n', 2) == NULL);

#if defined(FT_HAVE_WCHAR)
    const wchar_t *empty_wstr = L"";
    assert_true(wstr_n_substring_beg(empty_wstr, L'\n', 0) == empty_wstr);
    assert_true(wstr_n_substring_beg(empty_wstr, L'\n', 1) == NULL);
    assert_true(wstr_n_substring_beg(empty_wstr, L'\n', 2) == NULL);
#endif
#ifdef FT_HAVE_UTF8
    const char *utf8_empty_str = "";
    assert_true(utf8_n_substring_beg(utf8_empty_str, '\n', 0) == utf8_empty_str);
    assert_true(utf8_n_substring_beg(utf8_empty_str, '\n', 1) == NULL);
    assert_true(utf8_n_substring_beg(utf8_empty_str, '\n', 2) == NULL);
#endif


    const char *str = "123\n5678\n9";
    assert_true(str_n_substring_beg(NULL, '\n', 0) == NULL);
    assert_true(str_n_substring_beg(str, '\n', 0) == str);
    assert_true(str_n_substring_beg(str, '1', 0) == str);

    assert_true(str_n_substring_beg(str, '\n', 1) == str + 4);
    assert_true(str_n_substring_beg(str, '\n', 2) == str + 9);
    assert_true(str_n_substring_beg(str, '\n', 3) == NULL);

#if defined(FT_HAVE_WCHAR)
    const wchar_t *wstr = L"123\n5678\n9";
    assert_true(wstr_n_substring_beg(NULL, L'\n', 0) == NULL);
    assert_true(wstr_n_substring_beg(wstr, L'\n', 0) == wstr);
    assert_true(wstr_n_substring_beg(wstr, L'1', 0) == wstr);

    assert_true(wstr_n_substring_beg(wstr, L'\n', 1) == wstr + 4);
    assert_true(wstr_n_substring_beg(wstr, L'\n', 2) == wstr + 9);
    assert_true(wstr_n_substring_beg(wstr, L'\n', 3) == NULL);
#endif
#ifdef FT_HAVE_UTF8
    const char *utf8_str = "123\n5678\n9";
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(NULL, '\n', 0), NULL);
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str, '\n', 0), utf8_str);
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str, '1', 0), utf8_str);

    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str, '\n', 1), "5678\n9");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str, '\n', 2), "9");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str, '\n', 3), NULL);
#endif

    const char *str2 = "\n123\n56\n\n9\n";
    assert_true(str_n_substring_beg(str2, '\n', 0) == str2);
    assert_true(str_n_substring_beg(str2, '\n', 1) == str2 + 1);
    assert_true(str_n_substring_beg(str2, '\n', 2) == str2 + 5);
    assert_true(str_n_substring_beg(str2, '\n', 3) == str2 + 8);
    assert_true(str_n_substring_beg(str2, '\n', 4) == str2 + 9);
    assert_true(str_n_substring_beg(str2, '\n', 5) == str2 + 11);
    assert_true(str_n_substring_beg(str2, '\n', 6) == NULL);

#if defined(FT_HAVE_WCHAR)
    const wchar_t *wstr2 = L"\xff0fy23\xff0fy6\xff0f\xff0fy\xff0f";
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 0) == wstr2);
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 1) == wstr2 + 1);
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 2) == wstr2 + 5);
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 3) == wstr2 + 8);
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 4) == wstr2 + 9);
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 5) == wstr2 + 11);
    assert_true(wstr_n_substring_beg(wstr2, L'\xff0f', 6) == NULL);
#endif
#ifdef FT_HAVE_UTF8
    const char *utf8_str2 = "\n123\n56\n\n9\n";
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 0), utf8_str2);
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 1), "123\n56\n\n9\n");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 2), "56\n\n9\n");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 3), "\n9\n");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 4), "9\n");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 5), "");
    ASSERT_STR_EQUAL_STRONG(utf8_n_substring_beg(utf8_str2, '\n', 6), NULL);
#endif

    const char *beg = NULL;
    const char *end = NULL;
    str_n_substring(empty_str, '\n', 0, &beg, &end);
    assert_true(beg == empty_str && end == empty_str + strlen(empty_str));
    str_n_substring(empty_str, '\n', 1, &beg, &end);
    assert_true(beg == NULL && end == NULL);
    str_n_substring(empty_str, '\n', 2, &beg, &end);
    assert_true(beg == NULL && end == NULL);

#if defined(FT_HAVE_WCHAR)
    const wchar_t *wbeg = NULL;
    const wchar_t *wend = NULL;
    wstr_n_substring(empty_wstr, L'\n', 0, &wbeg, &wend);
    assert_true(wbeg == empty_wstr && wend == empty_wstr + wcslen(empty_wstr));
    wstr_n_substring(empty_wstr, L'\n', 1, &wbeg, &wend);
    assert_true(wbeg == NULL && wend == NULL);
    wstr_n_substring(empty_wstr, L'\n', 2, &wbeg, &wend);
    assert_true(wbeg == NULL && wend == NULL);
#endif
#ifdef FT_HAVE_UTF8
    const char *utf8_beg = NULL;
    const char *utf8_end = NULL;
    str_n_substring(utf8_empty_str, '\n', 0, &utf8_beg, &utf8_end);
    assert_true(utf8_beg == utf8_empty_str && utf8_end == utf8_empty_str + strlen(utf8_empty_str));
    str_n_substring(utf8_empty_str, '\n', 1, &utf8_beg, &utf8_end);
    assert_true(utf8_beg == NULL && utf8_end == NULL);
    str_n_substring(utf8_empty_str, '\n', 2, &utf8_beg, &utf8_end);
    assert_true(utf8_beg == NULL && utf8_end == NULL);
#endif


    str_n_substring(NULL, '\n', 0, &beg, &end);
    assert_true(beg == NULL && end == NULL);
    str_n_substring(str, '\n', 0, &beg, &end);
    assert_true(beg == str && end == str + 3);
    str_n_substring(str, '2', 0, &beg, &end);
    assert_true(beg == str && end == str + 1);

#if defined(FT_HAVE_WCHAR)
    wstr_n_substring(NULL, L'\n', 0, &wbeg, &wend);
    assert_true(wbeg == NULL && wend == NULL);
    wstr_n_substring(wstr, L'\n', 0, &wbeg, &wend);
    assert_true(wbeg == wstr && wend == wstr + 3);
    wstr_n_substring(wstr, L'2', 0, &wbeg, &wend);
    assert_true(wbeg == wstr && wend == wstr + 1);
#endif

    str_n_substring(str, '\n', 1, &beg, &end);
    assert_true(beg == str + 4 && end == str + 8);
    str_n_substring(str, '\n', 2, &beg, &end);
    assert_true(beg == str + 9 && end == str + strlen(str));
    str_n_substring(str, '\n', 3, &beg, &end);
    assert_true(beg == NULL && end == NULL);

#if defined(FT_HAVE_WCHAR)
    wstr_n_substring(wstr, L'\n', 1, &wbeg, &wend);
    assert_true(wbeg == wstr + 4 && wend == wstr + 8);
    wstr_n_substring(wstr, L'\n', 2, &wbeg, &wend);
    assert_true(wbeg == wstr + 9 && wend == wstr + wcslen(wstr));
    wstr_n_substring(wstr, L'\n', 3, &wbeg, &wend);
    assert_true(wbeg == NULL && wend == NULL);
#endif

    str_n_substring(str2, '\n', 0, &beg, &end);
    assert_true(beg == str2 && end == str2);
    str_n_substring(str2, '\n', 1, &beg, &end);
    assert_true(beg == str2 + 1 && end == str2 + 4);
    str_n_substring(str2, '\n', 2, &beg, &end);
    assert_true(beg == str2 + 5 && end == str2 + 7);
    str_n_substring(str2, '\n', 3, &beg, &end);
    assert_true(beg == str2 + 8 && end == str2 + 8);
    str_n_substring(str2, '\n', 4, &beg, &end);
    assert_true(beg == str2 + 9 && end == str2 + 10);
    str_n_substring(str2, '\n', 5, &beg, &end);
    assert_true(beg == str2 + 11 && end == str2 + 11);
    str_n_substring(str2, '\n', 6, &beg, &end);
    assert_true(beg == NULL && end == NULL);

#if defined(FT_HAVE_WCHAR)
    wstr_n_substring(wstr2, L'\xff0f', 0, &wbeg, &wend);
    assert_true(wbeg == wstr2 && wend == wstr2);
    wstr_n_substring(wstr2, L'\xff0f', 1, &wbeg, &wend);
    assert_true(wbeg == wstr2 + 1 && wend == wstr2 + 4);
    wstr_n_substring(wstr2, L'\xff0f', 2, &wbeg, &wend);
    assert_true(wbeg == wstr2 + 5 && wend == wstr2 + 7);
    wstr_n_substring(wstr2, L'\xff0f', 3, &wbeg, &wend);
    assert_true(wbeg == wstr2 + 8 && wend == wstr2 + 8);
    wstr_n_substring(wstr2, L'\xff0f', 4, &wbeg, &wend);
    assert_true(wbeg == wstr2 + 9 && wend == wstr2 + 10);
    wstr_n_substring(wstr2, L'\xff0f', 5, &wbeg, &wend);
    assert_true(wbeg == wstr2 + 11 && wend == wstr2 + 11);
    wstr_n_substring(wstr2, L'\xff0f', 6, &wbeg, &wend);
    assert_true(wbeg == NULL && wend == NULL);
#endif
}

void test_buffer_text_visible_width(void)
{
    string_buffer_t *buffer = create_string_buffer(200, CHAR_BUF);
    buffer->type = CHAR_BUF;
    char *old_value = buffer->str.cstr;

    buffer->str.cstr = (char *)"";
    assert_true(buffer_text_visible_width(buffer) == 0);

    buffer->str.cstr = (char *)"\n\n\n\n";
    assert_true(buffer_text_visible_width(buffer) == 0);

    buffer->str.cstr = (char *)"12345";
    assert_true(buffer_text_visible_width(buffer) == 5);

    buffer->str.cstr = (char *)"12345\n1234567";
    assert_true(buffer_text_visible_width(buffer) == 7);

    buffer->str.cstr = (char *)"12345\n1234567\n";
    assert_true(buffer_text_visible_width(buffer) == 7);

    buffer->str.cstr = (char *)"12345\n1234567\n123";
    assert_true(buffer_text_visible_width(buffer) == 7);

#if defined(FT_HAVE_WCHAR)
    buffer->type = W_CHAR_BUF;

    buffer->str.wstr = (wchar_t *)L"";
    assert_true(buffer_text_visible_width(buffer) == 0);

    buffer->str.wstr = (wchar_t *)L"\n\n\n\n";
    assert_true(buffer_text_visible_width(buffer) == 0);

    buffer->str.wstr = (wchar_t *)L"12345";
    assert_true(buffer_text_visible_width(buffer) == 5);

    buffer->str.wstr = (wchar_t *)L"12345\n1234567";
    assert_true(buffer_text_visible_width(buffer) == 7);

    buffer->str.wstr = (wchar_t *)L"12345\n1234567\n";
    assert_true(buffer_text_visible_width(buffer) == 7);

    buffer->str.wstr = (wchar_t *)L"12345\n1234567\n123";
    assert_true(buffer_text_visible_width(buffer) == 7);
#endif

    buffer->type = CHAR_BUF;
    buffer->str.cstr = old_value;
    destroy_string_buffer(buffer);
}


void test_buffer_text_visible_height(void)
{
    string_buffer_t *buffer = create_string_buffer(200, CHAR_BUF);
    buffer->type = CHAR_BUF;
    char *old_value = buffer->str.cstr;

    buffer->str.cstr = (char *)"";
    assert_true(buffer_text_visible_height(buffer) == 0);

    buffer->str.cstr = (char *)"\n";
    assert_true(buffer_text_visible_height(buffer) == 2);

    buffer->str.cstr = (char *)"\n\n";
    assert_true(buffer_text_visible_height(buffer) == 3);

    buffer->str.cstr = (char *)"\n\n\n\n";
    assert_true(buffer_text_visible_height(buffer) == 5);

    buffer->str.cstr = (char *)"12345";
    assert_true(buffer_text_visible_height(buffer) == 1);

    buffer->str.cstr = (char *)"\n12345";
    assert_true(buffer_text_visible_height(buffer) == 2);

    buffer->str.cstr = (char *)"\n12345\n\n2";
    assert_true(buffer_text_visible_height(buffer) == 4);

#if defined(FT_HAVE_WCHAR)
    buffer->type = W_CHAR_BUF;
    buffer->str.wstr = (wchar_t *)L"";
    assert_true(buffer_text_visible_height(buffer) == 0);

    buffer->str.wstr = (wchar_t *)L"\n";
    assert_true(buffer_text_visible_height(buffer) == 2);

    buffer->str.wstr = (wchar_t *)L"\n\n";
    assert_true(buffer_text_visible_height(buffer) == 3);

    buffer->str.wstr = (wchar_t *)L"\n\n\n\n";
    assert_true(buffer_text_visible_height(buffer) == 5);

    buffer->str.wstr = (wchar_t *)L"\xff0fy2345\xff0f";
    assert_true(buffer_text_visible_height(buffer) == 1);

    buffer->str.wstr = (wchar_t *)L"\n12345";
    assert_true(buffer_text_visible_height(buffer) == 2);

    buffer->str.wstr = (wchar_t *)L"\n12345\n\n2";
    assert_true(buffer_text_visible_height(buffer) == 4);
#endif


    buffer->type = CHAR_BUF;
    buffer->str.cstr = old_value;
    destroy_string_buffer(buffer);
}
