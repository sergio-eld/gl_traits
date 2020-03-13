#pragma once

#include "texture_traits.hpp"

namespace glt
{

    template <TextureTarget target, TexInternFormat format>
    class Texture : public texture_base_target<target, format>
    {

        using tex_base = texture_base_target<target, format>;

    public:

        Texture(HandleTexture&& handle = Allocator::Allocate(TextureTarget()))
            : tex_base(std::move(handle))
        {}

        using tex_base::Bind;
        using tex_base::UnBind;
		using tex_base::GenerateMipMap;
//        using tex_base::SetStorage;

        using texture_base::Initialized;

    };

    template <TexInternFormat internalFormat>
    using Texture1D = Texture<TextureTarget::texture_1d, internalFormat>;

    template <TexInternFormat internalFormat>
    using Texture2D = Texture<TextureTarget::texture_2d, internalFormat>;   

    template <TexInternFormat internalFormat>    
    using Texture3D = Texture<TextureTarget::texture_3d, internalFormat>;

	using Texture2Drgba = Texture2D<TexInternFormat::rgba>;

}