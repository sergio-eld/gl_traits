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

        using vao = VAO<Attr...>;
        using uniforms = uniform_collection<GLSL... >;

        using program_base::operator bool;

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
		}

		void Use()
		{
			program_base::Use();
		}

		void UnUse()
		{
			program_base::UnUse();
		}


		template <ShaderTarget ... targets>
		bool Link(const VertexShader& vShader, const FragmentShader& fShader,
			const Shader<targets>& ... otherShaders)
		{
			SetLinkStatus(Link_(vShader, fShader, otherShaders...));
            GetLocations();
           // assert(uniforms::AllValid() && "Failed to get uniforms locations!");
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

    public:

        class ProgGuard
        {
            Program *prog_;

        public:

            ProgGuard(Program& prog)
                : prog_(&prog)
            {
                if (!prog_->IsActive())
                    prog_->Use();
            }

            Program::uniforms& Uniforms()
            {
                return static_cast<Program::uniforms&>(*prog_);
            }

			void DrawTriangles(const Program::vao& vao, size_t first, size_t count)
			{
				assert(count > first &&
					!((count - first) % 3) &&
					"Invalid range for triangles!");

				assert(prog_->IsActive() && "Program is not active during Guard's lifetime!");

				glDrawArrays(GL_TRIANGLES, (GLint)first, (GLint)count);
			}

            template <typename ... attr, class = std::enable_if_t<std::conjunction_v<is_equivalent<Attr, attr>...>>>
			void DrawTriangles(const VAO<attr...>& vao, size_t first, size_t count)
			{
				DrawTriangles(reinterpret_cast<const Program::vao&>(vao), first, count);
			}

			void DrawTriangleFan(const Program::vao& vao, size_t first, size_t count)
			{
				assert(prog_->IsActive() && "Program is not active during Guard's lifetime!");
                assert(vao.IsBound() && "VAO is not bound!");

				glDrawArrays(GL_TRIANGLE_FAN, (GLint)first, (GLint)count);
			}

			template <typename ... attr, class = std::enable_if_t<std::conjunction_v<is_equivalent<Attr, attr>...>>>
			void DrawTriangleFan(const VAO<attr...>& vao, size_t first, size_t count)
			{
				DrawTriangleFan(reinterpret_cast<const Program::vao&>(vao), first, count);
			}
			
            // TODO: add VAO as parameter to ensure that it is active during the drawing call
            void DrawElements(const glt::Buffer<unsigned int>& elemBuffer, RenderMode mode, size_t count, size_t indexStart = 0)
            {
                assert(prog_->IsActive() && "Program is not active during Guard's lifetime!");
                assert(elemBuffer().Allocated() > indexStart + count && "Element indices are out of range!");

                if (!elemBuffer.IsBound())
                    elemBuffer.Bind(BufferTarget::element_array);

                glDrawElements((GLenum)mode, (GLsizei)count, GL_UNSIGNED_INT, (void*)indexStart);
                elemBuffer.UnBind();
            }



            ~ProgGuard()
            {
                assert(prog_ && "Program is not supposed to be nullptr!");
                assert(prog_->IsActive() && "Another program has been activated during Guard's lifetime!");
                prog_->UnUse();
            }
        };

        ProgGuard Guard()
        {
            return ProgGuard(*this);
        }
	};    

}