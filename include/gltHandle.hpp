#pragma once

#include "gltEnums.hpp"

template <auto target, class = typename decltype(target)>
class gltHandle;

template <auto target>
class gltHandle<target, typename decltype(target)>
{
    //TODO: add Glsync handle type
    GLuint handle_ = 0;

public:

    using Target = typename decltype(target);

    gltHandle() = default;
    gltHandle(GLuint handle)
        : handle_(handle)
    {
        //ownership?
        //handle = 0;
    }

    gltHandle(const gltHandle<target>& other) = delete;
    gltHandle<target>& operator=(const gltHandle<target>& other) = delete;

    gltHandle(gltHandle<target>&& other)
        : handle_(other.handle_)
    {
        other.handle_ = 0;
    }
    gltHandle<target>& operator=(gltHandle<target>&& other)
    {
        if (handle_)
            DestroyHandle();
        handle_ = other.handle_;
        other.handle_ = 0;
        return *this;
    }

    bool operator==(const gltHandle<target>& other) const
    {
        return handle_ == other.handle_;
    }

    bool operator!=(const gltHandle<target>& other) const
    {
        return !operator==(other);
    }

    bool IsValid() const
    {
        return (bool)handle_;
    }

    bool operator!() const
    {
        return !IsValid();
    }

    constexpr operator GLuint() const
    {
        return handle_;
    }

    ~gltHandle()
    {
        if (handle_)
            handle_deleter<decltype(target)>()(handle_);
    }

};