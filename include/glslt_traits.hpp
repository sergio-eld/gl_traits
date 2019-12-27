#pragma once

#include <tuple>
#include <string_view>
#include <type_traits>

// for check for uniqueness
#include <algorithm>

namespace glt
{
	constexpr bool compare_const_strings(const char *lhs, const char *rhs)
	{
		while (*lhs || *rhs)
			if (*lhs++ != *rhs++)
				return false;
		return true;
	}

	template <const char *a, const char *b>
	struct str_compare : std::bool_constant<compare_const_strings(a, b)> {};

	/* Wrapper typename to represent named Attribute.
	Can be:
	- A simple glm or basic C type;
	- user-defined structure (defined in shader source)

	User can define his own typenames given he defines:
	- constexpr static const char * glt_name()
	- using glt_type = typename T
	*/
	template <class T, const char * glslName>
	struct glslt
	{
		static_assert((bool)glslName,
			"glsl Name must not be nullptr! Use fundamental or glm types instead");
		constexpr static const char * glt_name() { return glslName; }
		using glt_type = typename T;
	};

	template <typename T, typename = std::void_t<>>
	struct has_name : std::false_type {};

	template <typename T>
	struct has_name<T, std::void_t<decltype(&T::glt_name)>> : std::true_type {};

	template <typename T>
	constexpr inline bool has_name_v = has_name<T>();

	template <typename T, typename = std::void_t<>>
	struct has_type : std::false_type {};

	template <typename T>
	struct has_type<T, std::void_t<typename T::glt_type>> : std::true_type {};

	template <typename T>
	constexpr inline bool has_type_v = has_type<T>();

	/////////////////////////////////////
	// Layout qualifiers
	////////////////////////////////////
	template <int loc>
	struct q_location
	{
		constexpr static int glt_location = loc;
	};

	template <int bind>
	struct q_binding
	{
		constexpr static int glt_binding = bind;
	};

	template <typename VarT, typename ... qualifiers>
	struct add_qualifiers : VarT, public qualifiers ...
	{
		using VarT::glt_name;
		using VarT::glt_type;
	};

	template <typename T, typename = std::void_t<>>
	struct has_location : std::false_type {};

	template <typename T>
	struct has_location<T, std::void_t<decltype(&T::glt_location)>> : std::true_type {};

	template <typename T>
	constexpr inline bool has_location_v = has_location<T>();

	template <class T>
	struct is_glm_vec : std::false_type {};

	template <glm::length_t L, typename T, glm::qualifier Q>
	struct is_glm_vec<glm::vec<L, T, Q>> : std::true_type {};

	template <class T>
	constexpr inline bool is_glm_vec_v = is_glm_vec<T>();

	template <class T>
	struct is_glm_mat : std::false_type {};

	template <class T, glm::length_t C, glm::length_t R, glm::qualifier Q>
	struct is_glm_mat<glm::mat<C, R, T, Q>> : std::true_type {};

	template <class T>
	constexpr inline bool is_glm_mat_v = is_glm_mat<T>();

	template <class T>
	struct is_glm : std::bool_constant<is_glm_mat_v<T> || is_glm_vec_v<T>> {};

	template <class T>
	constexpr inline bool glm_or_fundamental = is_glm<T>() || std::is_fundamental_v<T>;

	template <class T>
	struct variable_traits
	{
		static_assert(glm_or_fundamental<T> || has_name_v<T> && has_type_v<T>,
			"T is not a valid glt variable type!");

		constexpr static const char *get_name_()
		{
			if constexpr (glm_or_fundamental<T>)
				return nullptr;
			else
				return T::glt_name();
		}

		constexpr static int get_location()
		{
			if constexpr (has_location_v<T>)
				return T::glt_location;
			else return -1;
		}

		template <bool glm_or_fund = false>
		struct get_type_ { using type = typename T::glt_type; };

		template <>
		struct get_type_<true> { using type = T; };

		// name and type have different aliases than glt variables (glt_name, glt_type)
		constexpr static const char* name = get_name_();
		using type = typename get_type_<glm_or_fundamental<T>>::type;
		constexpr static int location = get_location();
	};

	template <class T>
	constexpr inline const char *variable_traits_name = variable_traits<T>::name;

	template <class T>
	using variable_traits_type = typename variable_traits<T>::type;

	template <class T>
	constexpr inline int variable_traits_location = variable_traits<T>::location;

	//////////////////////////////////////////////////////////
	// Equivalence traits
	//////////////////////////////////////////////////////////

	// TODO: refactor this

	template <class T>
	struct is_tuple : std::false_type {};

	template <class ... T>
	struct is_tuple<std::tuple<T...>> : std::true_type {};

	template <class T>
	constexpr inline bool is_tuple_v = is_tuple<T>();

	template <class T, typename = std::void_t<>>
	struct defines_tuple : std::false_type {};

	template <class T>
	struct defines_tuple<T, std::void_t<typename T::tuple>> : std::true_type {};

	template <class ... T>
	struct defines_tuple<std::tuple<T...>, std::void_t<std::tuple<T...>>> : std::true_type {};
	
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

	/* Version for variadic input */
	template <class T, class ... From>
	using is_initializable_from = is_initializable<T, std::tuple<From...>>;

	template <class T, class ... From>
	constexpr inline bool is_initializable_from_v =
		is_initializable<T, std::tuple<From...>>::value;

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

	template <class T, class NotTuple>
	struct is_tuple_equivalent : std::false_type
	{
		static_assert(!is_tuple_v<T>, "First argument must not be a tuple!");
		// TODO: warning for 2nd class is not a tuple?
	};

	/* Get if class T (non-tuple) is equivalent to std::tuple<Args...>.
	True if T is initializable from Args and size of T is equal
	to the size of a potential class with identical alignment of Args members */
	template <class T, class ... Args>
	struct is_tuple_equivalent<T, std::tuple<Args...>>
		: std::bool_constant<(class_size_from_tuple_v<std::tuple<Args...>> == sizeof(T)) &&
		is_initializable_from_v<T, Args...>>
	{
		static_assert(!is_tuple_v<T>, "First argument must not be a tuple!");
	};

	template <class T, class Tuple>
	constexpr inline bool is_tuple_equivalent_v = is_tuple_equivalent<T, Tuple>();

	/*
	Recover std::tuple from compound type or user-defined type.
	For user-defined case need to provide InitFrom tuple, that contains
	types to try aggregate initialization from.

	If failed to recover false_type, type = std::tuple<Attr>

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
		: std::bool_constant<(class_size_from_tuple_v<std::tuple<InitList...>> == sizeof(Attr)) &&
		is_initializable_from_v<Attr, InitList...>>
	{
		using type = std::conditional_t<(class_size_from_tuple_v<std::tuple<InitList...>> == sizeof(Attr)) &&
			is_initializable_from_v<Attr, InitList...>,
			std::tuple<InitList...>, Attr>;
	};

	template <class Attr, class InitFrom = std::void_t<>>
	using recover_tuple_t = typename recover_tuple<Attr, InitFrom>::type;


	/*-------------
	Checking Attr and In types for equivalence:
	Two classes are equivalent if their recovered tuples are the same.
	-------------*/
	template <class Attr, class In, class InitAttr = std::void_t<>>
	struct is_equivalent
		: std::bool_constant<
		std::is_same_v<recover_tuple_t<Attr, InitAttr>,
		recover_tuple_t<In, recover_tuple_t<Attr, InitAttr>>>
		> {};

	template <class Attr, class In, class InitAttr = std::void_t<>>
	constexpr inline bool is_equivalent_v = is_equivalent<Attr, In, InitAttr>::value;

	//////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Compound traits
	//////////////////////////////////////////////////////////
	/* In this section:
	- introduce compound type (for buffers and storages)
	- check if all the glsl types in a set are unique (for named types)
	- check if 2 sets of glsl types has zero common subset
	*/

    // 2 empty names are considered equal. Shader must not be provided with non-named variables
	template <class ... Types>
	constexpr bool all_names_unique()
	{
        if constexpr (sizeof...(Types))
        {
            std::string_view strs[]{ variable_traits_name<Types> ... };

            for (size_t i = 0; i + 1 != sizeof...(Types); ++i)
                for (size_t j = i + 1; j != sizeof...(Types); ++j)
                    if (strs[i] == strs[j])
                        return false;
            return true;
        }
        // empty set yields true
        else
            return true;
	}

    template <class Tuple>
    struct tuple_unique_names : std::false_type {};

    template <class ... Vars, template <class ...> class T>
    struct tuple_unique_names<T<Vars...>> 
        : std::bool_constant<all_names_unique<Vars...>()> {};

    template <class Tuple>
    constexpr inline bool tuple_unique_names_v = tuple_unique_names<Tuple>();

    template <class ... Vars1, class ... Vars2>
    constexpr bool get_sets_identical(std::tuple<Vars1...>, std::tuple<Vars2...>)
    {
        return true;
    }

    template <class Set1, class Set2>
    struct identical_sets : std::false_type {};

    template <template <class ...> class S1, class ... Vars1, template <class ...> class S2, class ... Vars2>
    struct identical_sets<S1<Vars1...>, S2<Vars2...>> 
        : std::bool_constant<get_sets_identical(std::tuple<Vars1...>(), 
            std::tuple<Vars2...>())> {};

    template <class Set1, class Set2>
    constexpr inline bool identical_sets_v = identical_sets<Set1, Set2>();

	//////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////

}