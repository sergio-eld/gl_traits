#pragma once

#include "gltEnums.hpp"

template <typename>//auto target, typename = decltype(target)>
struct gltAllocator;

template <typename eTargetType>//auto target>
struct handle_accessor;

template <typename eTargetType>
class gltHandle//<target, typename decltype(target)>
{
    //TODO: add Glsync handle type
    GLuint handle_;// = 0;

    friend struct gltAllocator<eTargetType>;
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

    bool operator==(GLuint raw_handle) const
    {
        return handle_ == raw_handle;
    }

    bool operator!=(GLuint raw_handle) const
    {
        return !operator==(raw_handle);
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
struct gltAllocator
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
struct gltAllocator<glShaderTarget>
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

template <auto>
struct tag
{};

template <typename eTargetType, eTargetType target>
class gl_current_object_base
{
    static GLuint raw_handle_;

    static_assert(has_func_bind_v<eTargetType>, "Typename doesn't have a glBind function");

    // property cannot be retrieved using glGet(..._BINDING)
    static_assert(has_gl_binding_v<target>, "Target can not be bound");
public:

    constexpr static auto ppBindFunc = pp_gl_binder_v<eTargetType>;
    constexpr static auto binding = get_binding_v<target>;

    static void Bind(tag<target>, const gltHandle<eTargetType>& handle)
    {
        assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

        (*ppBindFunc)((GLenum)target, handle_accessor<eTargetType>()(handle));
        raw_handle_ = handle_accessor<eTargetType>()(handle);
    }

    static void UnBind(tag<target>)
    {
        assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

        (*ppBindFunc)((GLenum)target, handle_accessor<eTargetType>()(handle));
        raw_handle_ = handle_accessor<eTargetType>()(handle);
    }

    static bool IsCurrent(tag<target>, const gltHandle<eTargetType>& handle)
    {
        // TODO: assert that returned by glGet object is equal to raw_handle_;
        // TODO: make an independent wrapper
        GLint res = 0;
        glGetIntegerv((GLenum)binding, &res);
        assert(res == raw_handle_);

        return handle == raw_handle_;
    }
};

template <typename eTargetType, eTargetType target>
GLuint gl_current_object_base<eTargetType, target>::raw_handle_ = 0;

template <typename ... gl_cur_obj>
struct gl_current_object_collection : gl_cur_obj...
{
    using gl_cur_obj::Bind...;
    using gl_cur_obj::UnBind...;
    using gl_cur_obj::IsCurrent...;

};

template <typename>
class gl_current_object;

template <typename eTargetType, eTargetType ... vals>
class gl_current_object<std::integer_sequence<eTargetType, vals...>> :
    public gl_current_object_collection<gl_current_object_base<eTargetType, vals> ...>
{

};

/*
OpenGL:

Shader source contains:
- layout of named attributes with (not necessarily) defined locations
- named uniforms

Buffers can store arrays of attributes:

- one batched attribute array in one VBO (one attribute per array element)
- array of compounds (several attributes per array element)
- several batched arrays of attributes  in one VBO
- first - compound, followed by multiple batched atribute arrays

Each attribute/variable has a glsl Type and a name;

Goal:
1. Provide type info about Buffer Object's data:
    a. types of attributes stored (Type, Batched/Compound)
    b. order of attributes stored
    c. if the first array is compound
    d. (maybe) specific info about the attribute variable name
    e. (maybe) specific info about the attribute location number

Comment on "1.d.". Name is needed to check the attribute location when
VBO is passed to a Shader object to upload data. Would be completely typesafe
in case Shader classes will be generated at precompile step from shader source file.

Comment of "1.e". Not typesafe

2. Accept containers with continious data to buffers:
    a. having the same type as template arguments
        glt_VBO<glm::Vec3> vbo;
        ...
        std::vector<glm::Vec3> data;
        vbo.Load(data);

    b. having type equivalent to that of template arguments
    (check using pod_reflection lib)

        struct Vertex
        {
            glm::Vec3 pos,
                texture;
        }
        ...
        std::vector<Vertex> data;
        glt_VBO<glm::Vec3, glm::Vec2> vbo;
        vbo.Load(data);

    c. for buffers with multiple attributes provide location index
    or deduce:
        - explicitly passing the index number
        - if attribute type is unique
        - for named may use tag dispatching

Cases:
- VBO of unnamed attributes
    glVBO<glm::Vec3, glm::Vec3, int, int>

- VBO with first compound maybe followed by several unnamed attributes
    glVBO<comp_attr<glm::Vec3, glm::Vec2>, int, int>

- VBO with named attributes
    glVBO<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>> (gl_pos = "pos", gl_tex = "tex")
    glVBO<comp_attr<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>>>

TODO: define all attribute validations here (at the beginning)
*/


// attribute traits
template <class T, const char * glslName>
struct glslt
{
    using type = typename T;
    constexpr static const char * name = glslName;
};

template <class ... vAttribs>
using comp_attr = std::tuple<vAttribs...>;

template <class T>
struct is_named_attr : std::bool_constant<false> {};

template <class T, const char * glslName>
struct is_named_attr<glslt<T, glslName>> : std::bool_constant<true> {};

template <class T>
constexpr inline bool is_named_attr_v = is_named_attr<T>();

template <class T>
struct is_compound_attr : std::bool_constant<false> {};

template <class ... T>
struct is_compound_attr<comp_attr<T...>> : std::bool_constant<true> {};

// matrices are compound
template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct is_compound_attr<glm::mat<C, R, T, Q>> : std::bool_constant<true> {};

template <class T>
constexpr inline bool is_compound_attr_v = is_compound_attr<T>();

template <typename T, typename ... N>
class gltBuffer
{
    
};