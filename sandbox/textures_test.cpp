#include "Texture.hpp"

#include <array>

template <class TexTarSeq>
struct get_tar_indx;

template <glt::TextureTarget ... tar>
struct get_tar_indx<glt::values_list<glt::TextureTarget, tar...>>
{
    constexpr static std::array<glt::TextureTarget, sizeof...(tar)> targets{ tar... };
};

int main()
{

    constexpr auto& targets = get_tar_indx<glt::TextureTargetList>::targets;

    return 0;
}
