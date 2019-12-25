#pragma once

#include "shader_traits.hpp"
#include "uniform_traits.hpp"
#include "vao_traits.hpp"

namespace glt
{
	// Checks and sets an active Shader Program
	class ActiveProgram
	{
		inline static GLuint raw_handle_ = 0;

	public:

		static bool IsActive(const HandleProg& handle)
		{
			return handle == raw_handle_;
		}

		static void Use(const HandleProg& handle)
		{
			glUseProgram(handle_accessor(handle));
			raw_handle_ = handle_accessor(handle);
		}

		static void Reset()
		{
			glUseProgram(0);
			raw_handle_ = 0;
		}

	};

	//template <class VAOdescr, class UniformCollect, class ...>
	//class Program;

	//template <class ... Attribs, class UniformCollect>
	class Program//<VAO<Attribs...>, UniformCollect>
	{
		HandleProg handle_;

		bool linked_ = false;
		// VAO<Attribs> vao_;

	public:
		// default
		Program(HandleProg&& handle = Allocator::Allocate(ProgramTarget()))
			: handle_(std::move(handle))
		{
			assert(handle_.IsValid() && "Program::Invalid handle!");
		}

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		// TODO: fix. will not compile with additional shaders
		template <ShaderTarget ... targets>
		Program(const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders,
			HandleProg&& handle = AllocatorSpecific<ProgramTarget>::Allocate())
			: handle_(std::move(handle)),
			linked_(Link_(vShader, fShader, otherShaders...))
		{
			assert(IsValid() && "Program::Failed to link program!");
		}

		const HandleProg& GetHandle() const
		{
			return handle_;
		}

		bool IsActive() const
		{
			return ActiveProgram::IsActive(handle_);
		}

		void Use() const
		{
			ActiveProgram::Use(handle_);
		}

		void UnUse() const
		{
			if (!IsActive())
				throw std::exception("Program::Deactivating"
					" a program wich is not currently active!");
			ActiveProgram::Reset();
		}

		bool IsValid() const
		{
			return linked_;
		}

		bool operator!() const
		{
			return !IsValid();
		}

		template <ShaderTarget ... targets>
		bool Link(const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders)
		{
			linked_ = Link_(vShader, fShader, otherShaders...);
			return IsValid();
		}

	private:

		template <ShaderTarget ... targets>
		bool Link_(const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders) const
		{
			assert(handle_.IsValid() && "Program::Invalid handle!");

			glAttachShader(handle_accessor(handle_),
				handle_accessor(vShader.GetHandle()));

			glAttachShader(handle_accessor(handle_),
				handle_accessor(fShader.GetHandle()));

			if constexpr (sizeof...(targets))
				(glAttachShader(handle_accessor(handle_),
					handle_accessor(otherShaders.GetHandle())), ...);

			glLinkProgram(handle_accessor(handle_));

			GLint res = false;
			glGetProgramiv(handle_accessor(handle_), GL_LINK_STATUS, &res);
			return (bool)res;
		}

	};
}