#pragma once

#include "basic_types.hpp"

namespace glt
{
    template <TextureTarget target, TexInternFormat intFormat,
        class = decltype(std::make_index_sequence<get_tex_dim<target>::value>())>
        class texture_base_target;

    template <TextureTarget target, TexInternFormat intFormat, size_t ... indx>
    class texture_base_target<target, intFormat, std::index_sequence<indx...>>
        : public texture_base
    {

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
            : texture_base(std::move(handle), target)
        {}

        texture_base_target(texture_base_target&& other)
            : texture_base(std::move(other))
        {
            if (IsBound())
                Register(this);

            other.pBind_ = &texture_base_target::bind_before_init;
        }

        void Bind()
        {
            (this->*pBind_)();
        }

        void UnBind()
        {
            assert(texture_base::IsBound() && "Attempt to unbind non-active texture!");
            glBindTexture((GLenum)target_, 0);
            texture_base::Register<target_>();
        }


        template <class = std::enable_if_t<get_tex_dim<target>() == 1>>
        void SetStorage(unsigned int levelsOfDetail, unsigned int width)
        {
            assert(IsBound() && "Attempt to set storage for non-bound texture!");
            glTexStorage1D((GLenum)target, (GLsizei)levelsOfDetail, 
                (GLenum)intFormat, (GLsizei)width);

            lod_ = levelsOfDetail;
            width_ = width;

        }

        template <class = std::enable_if_t<get_tex_dim<target>() == 2>>
        void SetStorage(unsigned int levelsOfDetail, unsigned int width,
            unsigned int height)
        {
            assert(IsBound() && "Attempt to set storage for non-bound texture!");
            glTexStorage2D((GLenum)target, (GLsizei)levelsOfDetail,
                (GLenum)intFormat, (GLsizei)width, (GLsizei)height);

            lod_ = levelsOfDetail;
            width_ = width;
            height_ = height;
        }

        template <class = std::enable_if_t<get_tex_dim<target>() == 3>>
        void SetStorage(unsigned int levelsOfDetail, unsigned int width,
            unsigned int height, unsigned int depth)
        {
            assert(IsBound() && "Attempt to set storage for non-bound texture!");
            glTexStorage2D((GLenum)target, (GLsizei)levelsOfDetail,
                (GLenum)intFormat, (GLsizei)width, (GLsizei)height, (GLsizei)depth);

            lod_ = levelsOfDetail;
            width_ = width;
            height_ = height;
            depth_ = depth;
        }


    };

    struct tex_data_loader
    {

    };

}