#pragma once

/*MIT License

Copyright (c) 2019 ElDesalmado

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
SOFTWARE.*/

#ifndef _TYPE_TRAITS_
#include <type_traits>
#endif

#ifndef _TUPLE_
#include <tuple>
#endif

//tag for typenames
template <class T>
struct type_t
{
    using type = T;
};

//tag for autos
template <auto val>
using auto_t = std::integral_constant<decltype(val), val>;

//this is a workaround for vs express 17 (because const char* fails for auto_t)
template <const char* val>
using char_t = std::integral_constant<const char*, val>;

//generic pair
template <typename key_tag, typename val_tag>
struct cexpr_pair
{
	//need to assert that key_tag and val_tag have members "key" and "val"
    using key = key_tag;
    using value = val_tag;
};

template <class ... cexpr_pairs>
class cexpr_generic_map
{
    template <typename cexpr_tag_key, size_t iter = 0>
    constexpr static auto Find()
    {
        //failed to find by key
        if constexpr (iter == sizeof...(cexpr_pairs))
            return cexpr_pair<cexpr_tag_key, void>();
        else
        {
            typedef std::tuple_element_t<iter, std::tuple<cexpr_pairs...>> cur_pair;
            if constexpr (std::is_same_v<cexpr_tag_key, cur_pair::key>)
                return cur_pair();
            else 
                return Find<cexpr_tag_key, iter + 1>();
        }
    }

public:

    template <typename tag_key>
    using found_pair = decltype(Find<tag_key>());
};


template <typename T, size_t sz, class iseq = decltype(std::make_index_sequence<sz>())>
struct gen_tuple;

template <typename T, size_t sz, size_t ... indx>
struct gen_tuple<T, sz, std::index_sequence<indx...>>
{
    template <size_t>
    using T_ = T;

    using tuple = std::tuple<T_<indx>...>;
};

template <auto val, auto ... compare>
inline constexpr bool is_any = ((val == compare) || ...);