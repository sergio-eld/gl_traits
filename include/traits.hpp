﻿#pragma once

#include <type_traits>
#include <tuple>
#include "glm/glm.hpp"

namespace glt
{
	//////////////////////////////////////////////////
	// common traits
	//////////////////////////////////////////////////

	template <class T>
	struct is_tuple : std::false_type {};

	template <class ... T>
	struct is_tuple<std::tuple<T...>> : std::true_type {};

	template <class T>
	constexpr inline bool is_tuple_v = is_tuple<T>();

	template <size_t n, class ... T>
	using nth_element_t = std::tuple_element_t<n, std::tuple<T...>>;

	template <size_t n, class Tuple>
	struct tuple_nth_element
	{
		static_assert(is_tuple_v<Tuple>, "Only std::tuple is allowed!");
		using type = std::tuple_element_t<n, Tuple>;
	};

	/* Convert one type to another. Used when expanding parameter pack.
	Example: set n = sizeof...(T) arguments in a Foo<T..>::bar;
	template <class ... T>
	class Foo
	{
		void bar(convert_to<bool, T> ... arg) // to invoke, n bool args is needed.
		{}
	};
	*/
	template <typename To, typename From>
	using convert_to = To;

	/*
	Similar to "convert_to", but converting from non-template parameter.
	*/
	template <typename To, auto indx>
	using convert_v_to = To;


	template <auto>
	struct tag_v
	{};

	template <size_t S>
	using tag_s = tag_v<S>;

	template <typename>
	struct tag_t
	{};

	/* get maximum value within given sequence of T */
	template <typename T, T ... vals>
	constexpr T get_max(std::integer_sequence<T, vals...> =
		std::integer_sequence<T, vals...>())
	{
		static_assert(std::is_integral_v<T>, "Only integral types are allowed!");
		T arr[sizeof...(vals)]{ vals... },
			max = 0;
		for (size_t i = 0; i != sizeof...(vals); ++i)
			max = arr[i] > max ? arr[i] : max;
		return max;
	}

	/*
	Extension for std::is_constructible, but using aggregate initialization.
	Get if a pod class T can be initialized from an std::tuple<Types...>
	*/
	template <class T, class Tuple, class = std::void_t<>>
	struct is_initializable : std::false_type {};

	template <class T, class ... Types>
	struct is_initializable < T,
		std::tuple<Types...>,
		std::void_t<decltype(T{ Types()... }) >>
		: std::true_type
	{};

	template <class T, class ... From>
	using is_initializable_from = is_initializable<T, std::tuple<From...>>;

	template <class T, class ... From>
	constexpr inline bool is_initializable_from_v =
		is_initializable<T, std::tuple<From...>>::value;

	//////////////////////////////////////////////////
	// attribute traits
	/////////////////////////////////////////////////

	/* Wrapper typename to represent named Attribute */
	template <class T, const char * glslName>
	struct glslt
	{
		using type = typename T;
		constexpr static const char * name = glslName;
	};

	template <class T>
	struct is_named_attr : std::bool_constant<false> {};

	template <class T, const char * glslName>
	struct is_named_attr<glslt<T, glslName>> : std::bool_constant<true> {};

	template <class T>
	constexpr inline bool is_named_attr_v = is_named_attr<T>();

	/* Alias for compound attributes. vAttribs can be named glslt types */
	template <class ... vAttribs>
	using compound = std::tuple<vAttribs...>;

	template <class T>
	struct is_compound_attr : std::bool_constant<false> {};

	template <class ... T>
	struct is_compound_attr<compound<T...>> : std::bool_constant<(sizeof...(T) > 1)> {};

	// matrices are compound
	template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct is_compound_attr<glm::mat<C, R, T, Q>> : std::bool_constant<true> {};

	// TODO: add is_compound case for glslt?

	template <class T>
	constexpr inline bool is_compound_attr_v = is_compound_attr<T>();

	/* Get number of attributes within the compound one. 
	0 (and warning?) if attribute is not compound */
	template <class Compound>
	struct compound_attr_count : glt_constant<size_t(0)>
	{
		// warning not compound ?
	};

	template <class ... Attr>
	struct compound_attr_count<compound<Attr...>> : glt_constant<sizeof...(Attr)> {};

	template <class Compound>
	constexpr inline size_t compound_attr_count_v =	compound_attr_count<Compound>();

	/* Get attribute's type by indx.
	Typename parameter may be a:
	- tuple or compound (default)
	- Buffer or any class holds attributes
	- glslt compound attribute
	*/
	template <size_t indx, class Tuple>
	struct nth_attribute
	{
		using type = nth_element_t<indx, Tuple>;
	};

	template <size_t indx, template <typename ...> class Holder, typename ... Attr>
	struct nth_attribute<indx, Holder<Attr...>>
	{
		using type = nth_element_t<indx, std::tuple<Attr...>>;
	};

	template <size_t indx, const char * attrName, class ... Attr>
	struct nth_attribute<indx, glslt<compound<Attr...>, attrName>>
	{
		using type = nth_element_t<indx, std::tuple<Attr...>>;
	};


	//////////////////////////////////////////////////
	// vao traits
	/////////////////////////////////////////////////

	/* get offset of member at index "indx", assuming standart memory alignment */
	template <size_t indx, class ... T>
	constexpr std::ptrdiff_t get_member_offset()
	{
		static_assert(indx < sizeof...(T), "Index is out of range!");

		if constexpr (!indx)
			return 0;

		std::ptrdiff_t res = 0,
			sizes[]{ sizeof(T)... };

		for (size_t i = 1; i <= indx; ++i)
		{
			res += sizes[i - 1];
			/*
			if (!(res % 4))
				continue;

			if (sizes[i] > 4 - res % 4)
				res += 4 - res % 4;
				*/

			if ((res % 4) &&
				sizes[i] > 4 - res % 4)
				res += 4 - res % 4;
		}

		return res;
	}

	template <size_t indx, class>
	struct get_tuple_member_offset;

	template <size_t indx, class ... T>
	struct get_tuple_member_offset<indx, std::tuple<T...>>
		: std::integral_constant<std::ptrdiff_t, get_member_offset<indx, T...>()> {};

	template <size_t indx, class Tuple>
	constexpr inline std::ptrdiff_t get_tuple_member_offset_v =
		get_tuple_member_offset<indx, Tuple>();

	/*Get size of a class, assuming it has the same types and order of members as
	a given tuple:
	If template parameter is not an std::tuple, size of a class is returned 
	// TODO: restrict other classes than tuple, return 0 in another function*/
	template <class Tuple>
	constexpr size_t class_size_from_tuple()
	{
		if constexpr (!is_tuple_v<Tuple>)
			// TODO: add warning?
			return sizeof(Tuple);
		else
		{
			constexpr size_t lastIndx = std::tuple_size_v<Tuple> -1;
			using LastType = std::tuple_element_t<lastIndx, Tuple>;

			constexpr size_t offset = get_tuple_member_offset_v<lastIndx, Tuple>,
				res = offset + sizeof(LastType);

			return (sizeof(LastType) % 4) ? res + 4 - sizeof(LastType) % 4 : res;
		}

	}

	template <class Tuple>
	constexpr inline size_t class_size_from_tuple_v =
		class_size_from_tuple<Tuple>();


	//////////////////////////////////////////////////
	// equivalence traits
	/////////////////////////////////////////////////

	/*
	Recover std::tuple from compound type or user-defined type.
	For user-defined case need to provide InitFrom tuple, that contains
	types to try aggregate initialization from

	TODO: Rename?
	*/
	template <class Attr, class InitFrom = std::void_t<>>
	struct recover_tuple : std::false_type
	{
		using type = std::tuple<Attr>;
	};

	template <class ... Attr>
	struct recover_tuple<std::tuple<Attr...>> : std::true_type
	{
		using type = std::tuple<Attr...>;
	};

	// Checks if Attribute is equivalent to std::tuple<InitList...>
	template <class Attr, class ... InitList>
	struct recover_tuple<Attr, std::tuple<InitList...>>
		: std::bool_constant<is_initializable_from_v<Attr, InitList...>>
	{
		using type = std::conditional_t<is_initializable_from_v<Attr, InitList...>,
			std::tuple<InitList...>, Attr>;
	};


	/*-------------
	Checking Attr and In types for equivalence:
	// TODO: wrapp sizeof(Attr) to get the size of a class with same fields
	// as the tuple's parameters
	sizeof(Attr) == sizeof(Attr) &&
	std::is_same_v<recover_tuple_t<A>, recover_tuple_t<I>>
	-------------*/
	template <class Attr, class In, class InitAttr = std::void_t<>>
	struct is_equivalent
		: std::bool_constant<(
			std::is_same_v<Attr, In> ||
			class_size_from_tuple_v<Attr> == sizeof(In) &&
			std::is_same_v<typename recover_tuple<Attr, InitAttr>::type,
			typename recover_tuple<In, typename recover_tuple<Attr, InitAttr>::type>::type>
			)> {};

	template <class Attr, class In>
	constexpr inline bool is_equivalent_v = is_equivalent<Attr, In>::value;

}