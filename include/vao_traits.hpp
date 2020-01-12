#pragma once

#include "glslt_traits.hpp"

namespace glt
{
	class ActiveVAO
	{
		inline static GLuint vao_ = 0;

	public:

		static bool IsBound(const HandleVAO& handle)
		{
			return handle == vao_;
		}

		static void Bind(const HandleVAO& handle)
		{
			vao_ = handle_accessor(handle);
			glBindVertexArray(vao_);
		}

		static void UnBind()
		{
			vao_ = 0;
			glBindVertexArray(0);
		}

	};

    class VAO_base
    {
    protected:
        HandleVAO handle_;

        VAO_base(HandleVAO&& handle)
            : handle_(std::move(handle))
        {
            assert(handle_ && "Invalid VAO handle!");
        }

        VAO_base()
            : handle_(Allocator::Allocate(VAOTarget()))
        {
            assert(false && "VAO_base default constructor called!");
        }
    };

    // has_name_v = false
    // Attrib is not compound!
    template <size_t indx, class Attrib, bool = has_name_v<Attrib>>
    class VAO_attrib : protected virtual VAO_base
    {
    public:
        void EnablePointer(tag_s<indx>) const
        {
            ActiveVAO::IsBound(handle_);

            assert(ActiveVAO::IsBound(handle_));
            glEnableVertexAttribArray((GLuint)indx);
        }

        void AttributePointer(tag_s<indx>, VertexAttrib<variable_traits_type<Attrib>>&& attrib, bool normalize = false)
        {
            assert(ActiveVAO::IsBound(handle_) &&
                "Setting Vertex Attribute for non-active VAO");

            auto sz = (GLint)vao_attrib_size<Attrib>()();
            GLenum glType = (GLenum)c_to_gl_v<variable_traits_type<Attrib>>;
            GLsizei stride = (GLsizei)attrib.Stride();
            void *voffset = (void*)attrib.Offset();

            glVertexAttribPointer((GLuint)indx,
                sz,
                glType,
                normalize,
                stride,
                voffset);
        }

    protected:
        VAO_attrib() = default;

        // dummy type
        constexpr static auto GetPtrEnablePointer(tag_s<indx>)
        {
            return reinterpret_cast<void(VAO_attrib<0, void, false>::*)(tag_s<0>)>(&EnablePointer);
        }
    };

    template <size_t indx, class Attrib>
    class VAO_attrib<indx, Attrib, true> : protected virtual VAO_base,
        VAO_attrib<indx, Attrib, false> 
    {
    public:
        using VAO_attrib<indx, Attrib, false>::EnablePointer;
        using VAO_attrib<indx, Attrib, false>::AttributePointer;

        void EnablePointer(tag_t<Attrib>)
        {
            VAO_attrib<indx, Attrib, false>::EnablePointer(tag_s<indx>());
        }
        void AttributePointer(VertexAttrib<Attrib>&& attrib, bool normalize = false)
        {
            VAO_attrib<indx, Attrib, false>::AttributePointer(tag_s<indx>(), 
                reinterpret_cast<VertexAttrib<variable_traits_type<Attrib>>&&>(std::move(attrib)), normalize);
        }


    protected:
        VAO_attrib() = default;
       // using VAO_attrib<indx, Attrib, false>::GetPtrEnablePointer;



        constexpr static auto GetPtrEnablePointer(tag_s<indx>)
        {
           // constexpr void(VAO_attrib::*ptr)(tag_s<indx>) = &EnablePointer;
            return reinterpret_cast<void(VAO_attrib<0, void, false>::*)(tag_s<0>)>(&VAO_attrib<indx, Attrib, false>::EnablePointer);
        }
    };

 
    template <class AttribTuple, class = decltype(std::make_index_sequence<std::tuple_size_v<AttribTuple>>())>
    class VAO_packed;

	/*
	Implements glVertexAttribPointer usage

    // TODO: Attributes locationes may be undefined in source code. Meaning that 
    attribute location can be changed dynamically, several attributes can point to the same location.
    Need to store that information.
    Need to distinct between VAO with statically defined (in-source) and undefined
    attributes' locations.
    For dynamic definitions need to restrict different types for pointing to the same location

	// TODO: exclude compound attributes, VAO is agnostic to how a buffer stores data
	*/
	template <class ... Attribs, size_t ... indx>
	class VAO_packed<std::tuple<Attribs...>, std::index_sequence<indx...>> : VAO_attrib<indx, Attribs> ...
	{
        template <size_t i>
        using VAO_attr = VAO_attrib<i, std::tuple_element_t<i, std::tuple<Attribs...>>>;

        template <size_t i>
        using VAO_attr_f = VAO_attrib<i, std::tuple_element_t<i, std::tuple<Attribs...>>, false>;


		// What about glm::mat????
		static_assert(!std::disjunction_v<is_compound_attr<Attribs>...>,
			"Compound attributes are not allowed in VAO!");

        static_assert(all_names_unique<Attribs...>(),
            "All attributes in a VAO must have unique aliases!");
		// HandleVAO handle_;

		/*
		glVertexAttributePointer(
		0. GLuint index,
		1. GLenum type,
		2. GLboolean normalized,
		3. GLsizei stride,
		4. const void* offset
		)

		const GLvoid *offset - is the offset from the start of the buffer
			object CURRENTLY bound.

		What about Normalized parameter?????
		*/
        using DummyPtrEnablePointer = void(VAO_attrib<0, void, false>::*)(tag_s<0>);

        using VAO_attr<indx>::GetPtrEnablePointer...;



	public:
        VAO_packed(HandleVAO&& handle = Allocator::Allocate(VAOTarget()))
			: VAO_base(std::move(handle))
		{}

        using VAO_attr<indx>::EnablePointer...;
        using VAO_attr<indx>::AttributePointer...;

        template <size_t i>
        using D = void(VAO_packed<std::tuple<Attribs...>, std::index_sequence<indx...>>::*)(tag_s<i>);

        template <size_t i, auto p>
        struct p_enable
        {};



        // TODO: run-time pointer activation
        void EnablePointer(size_t indx)
        {

            using ThisPtr = void(VAO_packed::*)(tag_s<0>);

           // p_func_enable<0, &VAO_attr<0>::EnablePointer>::pFunc;

            const auto& table = enableTable;
            ThisPtr ptr = reinterpret_cast<ThisPtr>(enableTable[indx]);
            (this->*ptr)(tag_s<0>());
        }


		void Bind() const
		{
			ActiveVAO::Bind(handle_);
		}

		void UnBind() const
		{
			ActiveVAO::UnBind();
		}

        // TODO: enable vertex attribute pointer
        // - by name
        // - by index
        // - enable all

    private:

        constexpr static DummyPtrEnablePointer enableTable[]{ (GetPtrEnablePointer(tag_s<indx>())) ...};
	};

    template <class ... Attribs>
    using VAO = VAO_packed<std::tuple<Attribs...>>;

}