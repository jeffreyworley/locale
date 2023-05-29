//
// Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/locale/generator.hpp>
#include <locale>
#include <stdexcept>
#include <vector>

#include "boost/locale/std/all_generator.hpp"

namespace boost { namespace locale { namespace impl_std {

    template<typename CharType>
    class std_converter : public converter<CharType> {
    public:
        typedef std::basic_string<CharType> string_type;
        typedef std::ctype<CharType> ctype_type;
        std_converter(const std::locale& base, size_t refs = 0) : converter<CharType>(refs), base_(base) {}
        string_type convert(converter_base::conversion_type how,
                            const CharType* begin,
                            const CharType* end,
                            int /*flags*/ = 0) const override
        {
            switch(how) {
                case converter_base::upper_case:
                case converter_base::lower_case:
                case converter_base::case_folding: {
                    const ctype_type& ct = std::use_facet<ctype_type>(base_);
                    size_t len = end - begin;
                    std::vector<CharType> res(len + 1, 0);
                    CharType* lbegin = res.data();
                    std::copy(begin, end, lbegin);
                    if(how == converter_base::upper_case)
                        ct.toupper(lbegin, lbegin + len);
                    else
                        ct.tolower(lbegin, lbegin + len);
                    return string_type(lbegin, len);
                }
                case converter_base::normalization:
                case converter_base::title_case: break;
            }
            return string_type(begin, end - begin);
        }

    private:
        std::locale base_;
    };

    class utf8_converter : public converter<char> {
    public:
        typedef std::ctype<char> ctype_type;
        typedef std::ctype<wchar_t> wctype_type;
        utf8_converter(const std::locale& base, size_t refs = 0) : converter<char>(refs), base_(base) {}
        std::string convert(converter_base::conversion_type how,
                            const char* begin,
                            const char* end,
                            int /*flags*/ = 0) const override
        {
            switch(how) {
                case upper_case:
                case lower_case:
                case case_folding: {
                    std::wstring tmp = conv::utf_to_utf<wchar_t>(begin, end);
                    const wctype_type& ct = std::use_facet<wctype_type>(base_);
                    wchar_t* lbegin = &tmp.front();
                    const size_t len = tmp.size();
                    if(how == upper_case)
                        ct.toupper(lbegin, lbegin + len);
                    else
                        ct.tolower(lbegin, lbegin + len);
                    return conv::utf_to_utf<char>(lbegin, lbegin + len);
                }
                case title_case:
                case normalization: break;
            }
            return std::string(begin, end - begin);
        }

    private:
        std::locale base_;
    };

    std::locale
    create_convert(const std::locale& in, const std::string& locale_name, char_facet_t type, utf8_support utf)
    {
        switch(type) {
            case char_facet_t::nochar: break;
            case char_facet_t::char_f: {
                if(utf == utf8_support::native_with_wide || utf == utf8_support::from_wide) {
                    std::locale base(std::locale::classic(), new std::ctype_byname<wchar_t>(locale_name));
                    return std::locale(in, new utf8_converter(base));
                }
                std::locale base(std::locale::classic(), new std::ctype_byname<char>(locale_name));
                return std::locale(in, new std_converter<char>(base));
            }
            case char_facet_t::wchar_f: {
                std::locale base(std::locale::classic(), new std::ctype_byname<wchar_t>(locale_name));
                return std::locale(in, new std_converter<wchar_t>(base));
            }
#ifdef BOOST_LOCALE_ENABLE_CHAR16_T
            case char_facet_t::char16_f: {
                std::locale base(std::locale::classic(), new std::ctype_byname<char16_t>(locale_name));
                return std::locale(in, new std_converter<char16_t>(base));
            }
#endif
#ifdef BOOST_LOCALE_ENABLE_CHAR32_T
            case char_facet_t::char32_f: {
                std::locale base(std::locale::classic(), new std::ctype_byname<char32_t>(locale_name));
                return std::locale(in, new std_converter<char32_t>(base));
            }
#endif
        }
        return in;
    }

}}} // namespace boost::locale::impl_std
