#pragma once

#include "basic_types.hpp"

namespace glt
{
	/////////////////////////////////////////////////////////////////////////
	///// UNIFORMS
	/////////////////////////////////////////////////////////////////////////

	template <class T>
	struct unwrap_glsl
	{
		using type = T;
	};

	template <class T, const char *name>
	struct unwrap_glsl<glslt<T, name>>
	{
		using type = T;
	};


	template <class T>
	using unwrap_glsl_t = typename unwrap_glsl<T>::type;

	template <class GLSL, class T = variable_traits_type<GLSL>, class =
		decltype(std::make_index_sequence<seq_elem_count<GLSL>>())>
		class unif_data_io;

	/////////////////
	// Public class
	/////////////////
	template <class T, class =
		decltype(std::make_index_sequence<seq_elem_count<T>>())>
		class Uniform;

	// fundamentals and glm::vec type specialization
	template <class T, size_t ... indx>
	class Uniform<T, std::index_sequence<indx...>> : protected uniform_base
	{
	
	protected:
		Uniform(const program_base& prog, const char* name)
			: uniform_base(prog, name)
		{}

        using uniform_base::GetLocation;

	public:

		constexpr static size_t elems_count = sizeof...(indx);
		constexpr static glm::qualifier Q = glm_qualifier_v<T>;
		using c_type = variable_traits_type<T>;
		using glm_type = glm::vec<elems_count, c_type, Q>;
		using ret_type = std::conditional_t<(elems_count > 1), 
			glm_type, c_type>;

		void Set(convert_v_to<c_type, indx> ... val)
		{
			assert(prog_.IsActive() &&
				"Attempting to set uniform for non-active program!");

			p_gl_uniform_t<T, elems_count> pglUniformT =
				get_p_gl_uniform<c_type, elems_count>();

            (*pglUniformT)(location_, val...);
		}

		void Set(const glm_type& val)
		{
			assert(prog_.IsActive() &&
				"Attempting to set uniform for non-active program!");

			p_gl_uniform_t<glm_type> pglUniformT =
				get_p_gl_uniform<glm_type>();

            // count = 1 ??
			(*pglUniformT)(location_, 1, &val);
		}

		void Get(ret_type &ret) const
		{
            using FuncGet = void(*)(GLint, GLint, ret_type&);
            FuncGet ptr = reinterpret_cast<FuncGet>(*pp_gl_get_uniform_map<c_type>());

            (*ptr)(handle_accessor(prog_.Handle()), location_, ret);

		}

		ret_type Get() const
		{
			ret_type ret{};
			Get(ret);

			return ret;
		}

	};


	// matrix specailization
	template <class T, glm::length_t C, glm::length_t R, glm::qualifier Q, 
		size_t ... indx>
	class Uniform<glm::mat<C, R, T, Q>, std::index_sequence<indx...>> : 
		protected uniform_base
	{
	protected:
		Uniform(const program_base& prog, const char* name)
			: uniform_base(prog, name)
		{}
        
        using uniform_base::GetLocation;

	public:

		constexpr static size_t elems_count = sizeof...(indx);
		constexpr static glm::qualifier Q = Q;
		using c_type = T;
		using glm_type = glm::mat<C, R, T, Q>;
		using ret_type = glm::mat<C, R, T, Q>;

		void Set(const glm_type& val, bool transpose = false)
		{
			assert(prog_.IsActive() &&
				"Attempting to set uniform for non-active program!");

			p_gl_uniform_t<glm_type> pglUniformT =
				get_p_gl_uniform<glm_type, 1>();

            // count = 1 ??
			(*pglUniformT)(location_, 1, transpose, val);
		}

		void Get(ret_type &ret) const
		{
            using FuncGet = void(*)(GLint, GLint, ret_type&);
            FuncGet ptr = reinterpret_cast<FuncGet>(*pp_gl_get_uniform_map<c_type>());

			(*ptr)(handle_accessor(prog_.Handle()), location_, ret);
		}

		ret_type Get() const
		{
			ret_type ret{};
			Get(ret);

			return ret;
		}
	};

	template <class GLSL>
	class named_uniform : public Uniform<variable_traits_type<GLSL>>
	{
		static_assert(has_name_v<GLSL>, "Only named unifrom variables are allowed!");

		using unif_type = Uniform<variable_traits_type<GLSL>>;

	protected:

		named_uniform(const program_base& prog)
			: unif_type(prog, variable_traits_name<GLSL>)
		{}

        void GetLocation()
        {
            unif_type::GetLocation(variable_traits_name<GLSL>);
        }

	public:

		unif_type& Uniform(tag_t<GLSL>)
		{
			return *this;
		}

		const unif_type& Uniform(tag_t<GLSL>) const
		{
			return *this;
		}

		void Set(const GLSL& val)
		{
			unif_type::Set(val.glt_value);
		}

	};

	template <class GLSL_tuple, class =
		decltype(std::make_index_sequence<std::tuple_size_v<GLSL_tuple>>())>
		class uniform_collection;

	template <typename ... GLSL, size_t ...indx>
	class uniform_collection<std::tuple<GLSL...>, std::index_sequence<indx...>> :
		public named_uniform<GLSL> ...
	{
		static_assert(all_names_unique<GLSL...>(), 
			"All the uniforms must have unique aliases!");

		template <size_t i>
		using uniform_i = named_uniform<std::tuple_element_t<i, std::tuple<GLSL...>>>;

	protected:

		uniform_collection(const program_base& prog)
			: uniform_i<indx>(prog)...
		{}

        void GetLocations()
        {
            (uniform_i<indx>::GetLocation(), ...);
        }


	public:

		using uniform_i<indx>::Uniform...;
		using uniform_i<indx>::Set...;




	};


}