#pragma once

#include <string_view>

// for check for uniqueness
#include <algorithm>

namespace glt
{
	/////////////////////////////////////
	// common traits
	/////////////////////////////////////

	




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
	//////////////////////////////////////////////////////////

	



    /*
	template <class R, typename ... FeedCompound>
	struct is_equivalent<R, compound<FeedCompound...>> :
		is_equivalent<R, compound<FeedCompound...>, FeedCompound...> {};

    template <class L, typename ... FeedCompound>
    struct is_equivalent<compound<FeedCompound...>, L> :
        is_equivalent<compound<FeedCompound...>, L, FeedCompound...> {};*/


    // 2 empty names are considered equal. Shader must not be provided with non-named variables
	template <class ... Types>
	constexpr bool all_names_unique()
	{
#pragma message("glt::all_names_unique is not implemented!")
        /* //This version will take an eternity to compile and eventually fail
        if constexpr ((bool)sizeof...(Types))
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
            return true;*/

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


}