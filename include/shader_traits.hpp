#pragma once

#include "basic_types.hpp"

// what to do with this????
#include "ParseAlgorithm.h"

#include <algorithm>

namespace glt
{
	struct shader_traits
	{
		static inline ParseAlgorithm<ShaderFileInfo::text_source> algorithm;

		template <class = void, class = void>
		struct source_checker
		{
			static bool check_source(std::string_view)
			{
				return true;
			}
		};

		template <typename ... VarsIn, typename ... VarsOut>
		struct source_checker<std::tuple<VarsIn...>, std::tuple<VarsOut...>>
		{
			static inline ParseAlgorithm<ShaderFileInfo::text_source> algorithm;

			static bool check_source(std::string_view source)
			{
				std::vector<Variable> vars = algorithm.Parse(source);
				std::remove_if(vars.begin(), vars.end(),
					[](const Variable& var)
				{
					return var.type != Variable::var_in &&
						var.type != Variable::vertex_in &&
						var.type != Variable::var_out;
				});
				return true;
			}
		};

		template <class TupleIn = void, class TupleOut = void>
		static bool check_source(std::string_view source)
		{
			return source_checker<TupleIn, TupleOut>::check_source(source);
		}

		static bool Compile(const HandleShader& handle, const char *source, size_t length)
		{
			assert(handle && "Shader::Invalid shader handle!");

			glShaderSource(handle_accessor(handle), 1, &source, (GLint*)&length);
			glCompileShader(handle_accessor(handle));
			GLint res = false;
			glGetShaderiv(handle_accessor(handle), GL_COMPILE_STATUS, &res);
			return (bool)res;
		}
	};


	template <ShaderTarget target>
	class Shader
	{
		HandleShader handle_;
		bool compiled_ = false;

	public:

		// default case
		Shader(HandleShader&& handle = Allocator::Allocate(target))
			: handle_(std::move(handle))
		{}

		template <size_t length>
		Shader(const char(&source)[length],
			HandleShader&& handle = Allocator::Allocate(target))
			: handle_(std::move(handle)),
			compiled_(shader_traits::Compile(handle_, source, length))
		{
			assert(IsValid() && "Shader::Failed to compile shader!");
		}

		Shader(const char* source, size_t length,
			HandleShader&& handle = Allocator::Allocate(target))
			: handle_(std::move(handle)),
			compiled_(shader_traits::Compile(handle_, source, length))
		{
			assert(IsValid() && "Shader::Failed to compile shader!");
		}

		Shader(const std::string& source,
			HandleShader&& handle = Allocator::Allocate(target))
			: handle_(std::move(handle)),
			compiled_(shader_traits::Compile(handle_, source.data(), source.size()))
		{
			assert(IsValid() && "Shader::Failed to compile shader!");
		}

		// shaders actually may be allowed to be copied, but what about the handle?
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		bool Compile(const std::string& source)
		{
			compiled_ = shader_traits::Compile(handle_, source.data(), source.length());
			return IsValid();
		}

		bool Compile(const char *source, size_t length)
		{
			compiled_ = shader_traits::Compile(handle_, source, length);
			return IsValid();
		}

		template <size_t length>
		bool Compile(const char(&source)[length])
		{
			compiled_ = shader_traits::Compile(handle_, source, length);
			return IsValid();
		}

		bool IsValid() const
		{
			return compiled_;
		}

		bool operator!() const
		{
			return !IsValid();
		}

		const HandleShader& GetHandle() const
		{
			return handle_;
		}



	};

	using VertexShader = Shader<ShaderTarget::vertex>;
	using FragmentShader = Shader<ShaderTarget::fragment>;
}