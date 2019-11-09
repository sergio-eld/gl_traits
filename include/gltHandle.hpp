#pragma once

#include "gltEnums.hpp"

template <typename>//auto target, typename = decltype(target)>
struct handle_allocator;

template <typename eTargetType>//auto target>
struct handle_accessor;

template <typename eTargetType>
class gltHandle//<target, typename decltype(target)>
{
    //TODO: add Glsync handle type
    GLuint handle_;// = 0;

    friend struct handle_allocator<eTargetType>;
    friend struct handle_accessor<eTargetType>;

public:

    // recover pointer to deleter function pointer
    constexpr static auto ppDeleteFunc = pp_gl_deleter_v<eTargetType>;

    gltHandle(const gltHandle<eTargetType>& other) = delete;
    gltHandle<eTargetType>& operator=(const gltHandle<eTargetType>& other) = delete;

    gltHandle(gltHandle<eTargetType>&& other)
        : handle_(other.handle_)
    {
        other.handle_ = 0;
    }
    gltHandle<eTargetType>& operator=(gltHandle<eTargetType>&& other)
    {
        if (handle_)
            DestroyHandle();
        handle_ = other.handle_;
        other.handle_ = 0;
        return *this;
    }

    bool operator==(const gltHandle<eTargetType>& other) const
    {
        return handle_ == other.handle_;
    }

    bool operator!=(const gltHandle<eTargetType>& other) const
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


    ~gltHandle()
    {
        if (handle_)
        {
            // TODO: do i need to check?
            assert(*ppDeleteFunc && "Pointers to OpenGL deleter functions have not been initialiized!");

            // check function signature for arguments
            if constexpr (std::is_same_v<void(APIENTRYP *)(GLsizei, const GLuint*), pp_gl_deleter<eTargetType>::value_type>)
                (*ppDeleteFunc)(1, &handle_);
            else if constexpr (std::is_same_v<void(APIENTRYP *)(GLuint), pp_gl_deleter<eTargetType>::value_type>)
                (*ppDeleteFunc)(handle_);
            else
                static_assert(false, "Unhandled case!");    
        }
    }


protected:

    gltHandle() = default;
    gltHandle(GLuint handle)
        : handle_(handle)
    {
        //ownership?
        //handle = 0;
    }

    constexpr operator GLuint() const
    {
        return handle_;
    }

};

template <typename eTargetType>
struct handle_allocator
{
    // retrieve pointer to allocator function pointer
    constexpr static auto ppAllocFunc = pp_gl_allocator<eTargetType>::value;

    //template <typename = std::enable_if_t<!std::is_same_v<glShaderTarget, decltype(target)>>>
    static gltHandle<eTargetType> Allocate()
    {
        // TODO: also check for deleter function pointers to be loaded
        assert(*ppAllocFunc && "Pointers to OpenGL allocator functions have not been initialiized!");
        if (!*ppAllocFunc)
            throw("Pointers to OpenGL allocator functions have not been initialiized!");

        GLuint h = 0;
        if constexpr (std::is_same_v<void(APIENTRYP *)(GLsizei, GLuint*), pp_gl_allocator<eTargetType>::value_type>)
        {
            (*ppAllocFunc)(1, &h);
        }
        else if constexpr (std::is_same_v<GLuint(APIENTRYP *)(void), pp_gl_allocator<eTargetType>::value_type>)
            h = (*ppAllocFunc)();
        //else if constexpr (std::is_same_v<GLuint(APIENTRYP *)(GLenum), pp_gl_allocator<eTargetType>::value_type>)
        //    return gltHandle<eTargetType>((*ppAllocFunc)((GLenum)target));
        else
            static_assert(false, "Unhandled case!");   

        if (!h)
        {
            assert(false && "Failed to allocate handle!");
            throw("Failed to allocate handle");
        }
        return gltHandle<eTargetType>(h);

    }

    //template <typename = std::enable_if_t<!std::is_same_v<glShaderTarget, decltype(target)>>>
    gltHandle<eTargetType> operator()() const
    {
        return Allocate();
    }

};

template <>
struct handle_allocator<glShaderTarget>
{
    constexpr static auto ppAllocFunc = pp_gl_allocator_v<glShaderTarget>;

    static gltHandle<glShaderTarget> Allocate(glShaderTarget target)
    {
        // TODO: also check for deleter function pointers to be loaded
        assert(*ppAllocFunc && "Pointers to OpenGL allocator functions have not been initialiized!");
        if (!*ppAllocFunc)
            throw("Pointers to OpenGL allocator functions have not been initialiized!");

        return gltHandle<glShaderTarget>((*ppAllocFunc)((GLenum)target));
    }

    gltHandle<glShaderTarget> operator()(glShaderTarget target) const
    {
        return Allocate(target);
    }

};

template <typename eTargetType>
struct handle_accessor
{
    GLuint operator()(const gltHandle<eTargetType>& handle) const
    {
        return handle;
    }
};


template <auto target>
class check_current_handle
{
    // makes no sence since objects are allowed to be moved. Need to checkusing GLuint
    // static const gltHandle<eTargetType> * current_handle_;
    static GLuint raw_handle_;

public:

    static void SetCurrentHandle(const gltHandle<decltype(target)>& handle)
    {
       // glBindBuffer()
        raw_handle_ = handle;
    }
};

// template <typename eTargetType, eTargetType target>
// const gltHandle<eTargetType> * check_current_handle<eTargetType, target>::current_handle_ = nullptr;