#pragma once

#include "basic_types.hpp"

namespace glt
{
	// base specialization
	template <TextureTarget target, TexInternFormat format, 
		size_t dims = get_tex_dim<target>(), bool base = !dims>
	struct texture_image
	{
		texture_base::modifier modifier;

		texture_image(texture_base::modifier&& mod)
			: modifier(std::move(mod))
		{}

		void GenerateMipMap()
		{
			assert_bound_init();

			glGenerateMipmap((GLenum)target);
		}

		void assert_bound_init()
		{
			assert(modifier.state_.IsBound() && "Texture is not bound!");
			assert(modifier.state_.Initialized() && "Texture is not initialized!");
		}

	};

	// 1D specialization
	template <TextureTarget target, TexInternFormat intFormat>
	struct texture_image<target, intFormat, 1> : public texture_image<target, intFormat, 0>
	{
		texture_image(texture_base::modifier&& mod)
			: texture_image<target, intFormat, 0>(std::move(mod))
		{}

		// default format and type = red and unsigned byte
		void SetImage(unsigned int level, unsigned int width)//,
		//	TexFormat format, TexType type)
		{
			assert_bound_init();

			assert(!buffer_base::TargetMapped(BufferTarget::pixel_unpack) &&
				"Attempt to set image while buffer is bound to pixel_unpack target!");

			glTexImage1D((GLenum)target, (GLint)level, (GLint)intFormat,
				(GLsizei)width, 0, 
				(GLenum)TexFormat::red, (GLenum)TexType::unsigned_byte, nullptr);

			modifier.SetLOD(level);
			modifier.SetSizes(width);
		}

		void SubImage(unsigned int level, unsigned int width,
			TexFormat format, TexType type, const void* pixels,
			int xoffset = 0)
		{
			assert_bound_init();

			assert(!buffer_base::TargetMapped(BufferTarget::pixel_unpack) &&
				"Attempt to set image while buffer is bound to pixel_unpack target!");

			// TODO: check width range
			glTexSubImage1D((GLenum)target, (GLint)level, (GLint)xoffset,
				(GLsizei)width, (GLenum)format, (GLenum)type, pixels);
		}

	};

	// 2D specialization
	template <TextureTarget target, TexInternFormat intFormat>
	struct texture_image<target, intFormat, 2> : public texture_image<target, intFormat, 0>
	{
		texture_image(texture_base::modifier&& mod)
			: texture_image<target, intFormat, 0>(std::move(mod))
		{}

		// default format and type = red and unsigned byte
		void SetImage(unsigned int level, unsigned int width, unsigned int height)//,
			//TexFormat format, TexType type)
		{
			assert_bound_init();

			assert(!buffer_base::TargetMapped(BufferTarget::pixel_unpack) &&
				"Attempt to set image while buffer is bound to pixel_unpack target!");

			glTexImage2D((GLenum)target, (GLint)level, (GLint)intFormat,
				(GLsizei)width, (GLsizei)height, 0, 
				(GLenum)TexFormat::red, (GLenum)TexType::unsigned_byte, nullptr);
			
			modifier.SetLOD(level);
			modifier.SetSizes(width, height);
		}

		void SubImage(unsigned int level, unsigned int width, unsigned int height,
			TexFormat format, TexType type, const void* pixels,
			int xoffset = 0, int yoffset = 0)
		{
			assert_bound_init();

			assert(!buffer_base::TargetMapped(BufferTarget::pixel_unpack) &&
				"Attempt to set image while buffer is bound to pixel_unpack target!");

			// TODO: check width range
			glTexSubImage2D((GLenum)target, (GLint)level, 
				(GLint)xoffset, (GLint)yoffset,
				(GLsizei)width, (GLsizei)height,
				(GLenum)format, (GLenum)type, 
				pixels);
		}

	};

	// 3D specialization
	template <TextureTarget target, TexInternFormat intFormat>
	struct texture_image<target, intFormat, 3> : public texture_image<target, intFormat, 0>
	{
		texture_image(texture_base::modifier&& mod)
			: texture_image<target, intFormat, 0>(std::move(mod))
		{}

		void SetImage(unsigned int level, 
			unsigned int width, unsigned int height, unsigned int depth)//,
			//TexFormat format, TexType type)
		{
			assert_bound_init();

			assert(!buffer_base::TargetMapped(BufferTarget::pixel_unpack) &&
				"Attempt to set image while buffer is bound to pixel_unpack target!");

			// default format and types = red and unsigned byte
			glTexImage3D((GLenum)target, (GLint)level, (GLint)intFormat,
				(GLsizei)width, (GLsizei)height, (GLsizei)depth,
				0, 
				(GLenum)TexFormat::red, (GLenum)TexType::unsigned_byte, nullptr);

			modifier.SetLOD(level);
			modifier.SetSizes(width, height, depth);
		}

		void SubImage(unsigned int level, 
			unsigned int width, unsigned int height, unsigned int depth,
			TexFormat format, TexType type, const void* pixels,
			int xoffset = 0, int yoffset = 0, int zoffset = 0)
		{
			assert_bound_init();

			assert(!buffer_base::TargetMapped(BufferTarget::pixel_unpack) &&
				"Attempt to set image while buffer is bound to pixel_unpack target!");

			// TODO: check width range
			glTexSubImage3D((GLenum)target, (GLint)level,
				(GLint)xoffset, (GLint)yoffset, (GLint)zoffset,
				(GLsizei)width, (GLsizei)height, (GLsizei)depth,
				(GLenum)format, (GLenum)type,
				pixels);
		}

	};

    template <TextureTarget target, TexInternFormat intFormat>
	class texture_base_target : public texture_base, 
		protected texture_image<target, intFormat>
    {

		using tex_image = texture_image<target, intFormat>;

        void(texture_base_target::*pBind_)() = 
            &texture_base_target::bind_before_init;

        void bind_after_init()
        {
            glBindTexture((GLenum)target, handle_accessor(handle_));
            texture_base::Register<target>(this);
        }

        void bind_before_init()
        {
            bind_after_init();

            texture_base::target_ = target;
            pBind_ = &texture_base_target::bind_after_init;
        }


    public:
        // TODO: make constructors protected since its' an intermediate class
        texture_base_target(HandleTexture&& handle)
            : texture_base(std::move(handle), target),
			texture_image(texture_base::GetModifier())
        {}

        texture_base_target(texture_base_target&& other)
            : texture_base(std::move(other)),
			texture_image(texture_base::GetModifier())
        {
            if (IsBound())
                Register<target>(this);

            other.pBind_ = &texture_base_target::bind_before_init;
        }

        void Bind()
        {
            (this->*pBind_)();
        }

#pragma message("Need to implement check for active GL_TEXTURE_N")
        void UnBind()
        {
            assert(texture_base::IsBound() && "Attempt to unbind non-active texture!");
            glBindTexture((GLenum)target_, 0);
            texture_base::Register<target>();
        }

		using tex_image::SetImage;
		using tex_image::SubImage;

    };

    struct tex_data_loader
    {

    };

}