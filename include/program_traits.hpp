#pragma once

#include "basic_types.hpp"

#include "shader_traits.hpp"
#include "uniform_traits.hpp"
#include "vao_traits.hpp"


namespace glt
{

	template <class vao_t, class unif_collection, class ...>
	class Program;

	template <class ... Attr, class ... GLSL>
	class Program<VAO<Attr...>, uniform_collection<GLSL...>> : public program_base,
		public uniform_collection<GLSL...>
	{

	public:

        using uniform_collection<GLSL...>::Uniform;
        using uniform_collection<GLSL...>::GetLocations;

		// default
		Program(HandleProg&& handle = Allocator::Allocate(ProgramTarget()))
			: program_base(std::move(handle)),
			uniform_collection<GLSL...>(static_cast<const program_base&>(*this))
		{}

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		// TODO: have to provide handle manually >:{
		template <ShaderTarget ... targets>
		Program(HandleProg&& handle, const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders)
			: Program(std::move(handle))
		{
			Link(vShader, fShader, otherShaders...);
            GetLocations();
		}

		void Use()
		{
			program_base::Use();
		}

		void UnUse()
		{
			program_base::UnUse();
		}

		operator bool() const
		{
			return static_cast<const program_base&>(*this) &&
				IsActive();
		}

		template <ShaderTarget ... targets>
		bool Link(const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders)
		{
			SetLinkStatus(Link_(vShader, fShader, otherShaders...));
			return Linked();
		}

	private:

		template <ShaderTarget ... targets>
		bool Link_(const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders)
		{
			assert(program_base::Handle() && "Program::Invalid handle!");

			glAttachShader(handle_accessor(program_base::Handle()),
				handle_accessor(vShader.GetHandle()));

			glAttachShader(handle_accessor(program_base::Handle()),
				handle_accessor(fShader.GetHandle()));

			if constexpr (sizeof...(targets))
				(glAttachShader(handle_accessor(program_base::Handle()),
					handle_accessor(otherShaders.GetHandle())), ...);

			glLinkProgram(handle_accessor(program_base::Handle()));

			GLint res = false;
			glGetProgramiv(handle_accessor(program_base::Handle()), GL_LINK_STATUS, &res);
			assert(res && "Failed to link program!");

			return (bool)res;
		}

	};
}