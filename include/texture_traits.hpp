#pragma once

#include "basic_types.hpp"

namespace glt
{

    template <TextureTarget target>
    class texture_base_target : public texture_base
    {

        void(texture_base_target::*pBind_)() = &bind_before_init;

        void bind_after_init()
        {
            glBindTexture((GLenum)target_, handle_accessor(handle_));
            texture_base::Register<target_>(this);
        }

        void bind_before_init()
        {
            bind_after_init();

            texture_base::target_ = target;
            pBind_ = &bind_after_init;
        }


    public:
        // TODO: make constructors protected since its' an intermediate class
        texture_base_target(HandleTexture&& handle)
            : texture_base(std::move(handle))
        {}

        texture_base_target(texture_base_target&& other)
            : texture_base(std::move(other))
        {
            if (IsBound())
                Register(this);

            other.pBind_ = &bind_before_init;
        }

        void Bind()
        {
            this->(*pBind_)();
        }

        void UnBind()
        {
            assert(texture_base::IsBound() && "Attempt to unbind non-active texture!");
            glBindTexture((GLenum)target_, 0);
            texture_base::Register<target_>();
        }

    };

    struct tex_data_loader
    {

    };

}