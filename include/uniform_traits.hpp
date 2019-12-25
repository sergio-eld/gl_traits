#pragma once

namespace glt
{
	/////////////////////////////////////////////////////////////////////////
	///// UNIFORMS
	/////////////////////////////////////////////////////////////////////////

	template <class glslt_T>
	struct get_uniform_elems_count;

	template <class T, const char *name>
	struct get_uniform_elems_count<glslt<T, name>>
	{
		constexpr static size_t value = 1;
	};

	template <class T, const char *name, glm::length_t L>
	struct get_uniform_elems_count<glslt<glm::vec<L, T>, name>>
	{
		constexpr static size_t value = L;
	};

	template <class T, const char * name, glm::length_t C, glm::length_t R>
	struct get_uniform_elems_count <glslt<glm::mat<C, R, T>, name>>
	{
		constexpr static size_t value = 0;
	};

	template <class T>
	constexpr inline size_t get_uniform_elems_count_v =
		get_uniform_elems_count<T>::value;


	template <typename T, size_t sz = 0,
		class = decltype(std::make_index_sequence<sz>())>
		struct UniformModifier;

	template <typename T, size_t sz, size_t ... indx>
	struct UniformModifier<T, sz, std::index_sequence<indx...>>
	{
		using ret_type = std::conditional_t<(sz > 1), glm::vec<sz, T>, T>;

		// TODO: change GLint to handle
		static void Update(GLint handle,
			convert_v_to<T, indx>... vals)
		{
			p_gl_uniform_t<T, sz> pglUniformT = get_p_gl_uniform<T, sz>();

			(*pglUniformT)(handle, vals...);
		}

		static void Get(const HandleProg& prog, GLint handle, ret_type&ret)
		{
			void(*ptr)(GLint, GLint, ret_type*) = *pp_gl_get_uniform_map<ret_type>();

			(*ptr)(handle_accessor(prog), handle, &ret);
		}


		static ret_type Get(const HandleProg& prog, GLint handle)
		{
			ret_type ret{};
			void(*ptr)(GLint, GLint, ret_type&) =
				reinterpret_cast<decltype(ptr)>(*pp_gl_get_uniform_map<ret_type>());

			(*ptr)(handle_accessor(prog), handle, ret);

			return ret;
		}


	};

	template <typename T, glm::length_t L>
	struct UniformModifier<glm::vec<L, T>>
	{
		// TODO: change GLint to handle
		static void Update(GLint handle, const glm::vec<L, T>& vec)
		{
			p_gl_uniform_t<glm::vec<L, T>> pglUniformT =
				get_p_gl_uniform<glm::vec<L, T>, 1>();

			(*pglUniformT)(handle, L, vec);
		}

		static void Get(const HandleProg& prog, GLint handle, glm::vec<L, T>&ret)
		{
			void(*ptr)(GLint, GLint, glm::vec<L, T>&) =
				reinterpret_cast<decltype(ptr)>(*pp_gl_get_uniform_map<T>());

			(*ptr)(handle_accessor(prog), handle, ret);
		}

		static glm::vec<L, T> Get(const HandleProg& prog, GLint handle)
		{
			glm::vec<L, T> ret;
			void(*ptr)(GLint, GLint, glm::vec<L, T>&) =
				reinterpret_cast<decltype(ptr)>(*pp_gl_get_uniform_map<T>());

			(*ptr)(handle_accessor(prog), handle, ret);

			return ret;
		}

	};

	template <typename T, glm::length_t C, glm::length_t R>
	struct UniformModifier<glm::mat<C, R, T>>
	{
		// TODO: change GLint to handle
		static void Update(GLint handle, const glm::mat<C, R, T>& mat,
			bool transpose = false)
		{
			p_gl_uniform_t<glm::mat<C, R, T >> pglUniformT =
				get_p_gl_uniform<glm::mat<C, R, T>, 1>();

			(*pglUniformT)(handle, 1, transpose, mat);
		}

		static void Get(const HandleProg& prog, GLint handle, glm::mat<C, R, T>&ret)
		{
			void(*ptr)(GLint, GLint, glm::mat<C, R, T>&) =
				reinterpret_cast<decltype(ptr)>(*pp_gl_get_uniform_map<T>());

			(*ptr)(handle_accessor(prog), handle, ret);
		}

		static glm::mat<C, R, T> Get(const HandleProg& prog, GLint handle)
		{
			glm::mat<C, R, T> ret;
			void(*ptr)(GLint, GLint, glm::mat<C, R, T>&) =
				reinterpret_cast<decltype(ptr)>(*pp_gl_get_uniform_map<T>());

			(*ptr)(handle_accessor(prog), handle, ret);

			return ret;
		}

	};

	template <typename T, const char *name>
	class UniformBase
	{
	protected:

		// TODO: make prog a pointer?
		const HandleProg& prog_;
		GLint handle_ = -1;

		UniformBase(const HandleProg& prog)
			: prog_(prog),
			handle_(GetHandle())
		{
			assert(handle_ != -1 && "Uniform::Failed to get Uniform location!");
		}

	private:
		GLint GetHandle()
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to access a uniform when Program is not active!");
			return glGetUniformLocation(handle_accessor(prog_),
				name);
		}

	};


	template <class glslt_T,
		class =
		decltype(std::make_index_sequence<get_uniform_elems_count_v<glslt_T>>())>
		class Uniform;

	// glUniform1* and glUniform1*v
	template <typename T, const char *name>
	class Uniform<glslt<T, name>, std::index_sequence<0>>
		: protected UniformBase<T, name>
	{

	public:

		Uniform(const HandleProg& prog)
			: UniformBase<T, name>(prog)
		{}

		void Update(tag_c<name>, T val) const
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to modify a uniform for a non-active Program!");
			UniformModifier<T, 1>::Update(handle_, val);
		}

		void Update(tag_c<name>, const glm::vec<1, T>& val) const
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to modify a uniform for a non-active Program!");
			UniformModifier<glm::vec<1, T>>::Update(handle_, val);
		}

		void Get(tag_c<name>, T& val) const
		{
			UniformModifier<T, 1>::Get(prog_, handle_, val);
		}

		T Get(tag_c<name>) const
		{
			return UniformModifier<T, 1>::Get(prog_, handle_);
		}

	};

	// glUniform1-4* and glUniform1-4*v
	template <glm::length_t L, typename T, const char *name, size_t ... indx>
	class Uniform<glslt<glm::vec<L, T>, name>,
		std::index_sequence<indx...>>
		: protected UniformBase<glm::vec<L, T>, name>
	{

	public:

		Uniform(const HandleProg& prog)
			: UniformBase<glm::vec<L, T>, name>(prog)
		{}

		void Update(tag_c<name>, convert_v_to<T, indx> ... val) const
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to modify a uniform for a non-active Program!");
			UniformModifier<T, L>::Update(handle_, val ...);
		}

		void Update(tag_c<name>, const glm::vec<L, T>& val) const
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to modify a uniform for a non-active Program!");
			UniformModifier<glm::vec<L, T>>::Update(handle_, val);
		}

	};

	// glUniformMatrix
	template <glm::length_t C, glm::length_t R, typename T,
		const char *name>
		class Uniform<glslt<glm::mat<C, R, T>, name>,
		std::index_sequence<>>
		: protected UniformBase<glm::mat<C, R, T>, name>
	{
	public:

		Uniform(const HandleProg& prog)
			: UniformBase<glm::mat<C, R, T>, name>(prog)
		{
			assert(handle_ != -1 && "Uniform::Failed to get Uniform location!");
		}

		void Update(tag_c<name>, const glm::mat<C, R, T>& val, bool transpose = false) const
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to modify a uniform for a non-active Program!");
			UniformModifier<glm::mat<C, R, T>>::Update(handle_, val, transpose);
		}

		void Get(tag_c<name>, glm::mat<C, R, T>& val) const
		{
			UniformModifier<glm::mat<C, R, T>>::Get(prog_, handle_, val);
		}

		glm::mat<C, R, T> Get(tag_c<name>) const
		{
			return UniformModifier<glm::mat<C, R, T>>::Get(prog_, handle_);
		}

	};


	template <class TupleArgs,
		class = decltype(std::make_index_sequence<std::tuple_size_v<TupleArgs>>())>
		class UniformCollection;

	template <class ... Attribs, size_t ... indx>
	class UniformCollection<std::tuple<Attribs...>, std::index_sequence<indx...>>
		: Uniform<Attribs> ...
	{

		template <size_t i>
		using UnifBase = Uniform<nth_element_t<i, Attribs...>>;

	public:

		UniformCollection(const HandleProg& handle)
			: Uniform<Attribs>(handle)...
		{}

		using UnifBase<indx>::Update...;
		using UnifBase<indx>::Get...;

	};
}