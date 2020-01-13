#pragma once

#include "buffer_traits.hpp"

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
            assert(ActiveVAO::IsBound(handle_));
            glEnableVertexAttribArray((GLuint)indx);
        }

        void AttributePointer(tag_s<indx>, FetchedAttrib<Attrib>&& attrib, bool normalize = false)
        {
            assert(ActiveVAO::IsBound(handle_) &&
                "Setting Vertex Attribute for non-active VAO");

            glVertexAttribPointer((GLuint)indx,
                FetchedAttrib<Attrib>::size,
                FetchedAttrib<Attrib>::glType,
                normalize,
                attrib.stride,
                attrib.offset);
        }

    protected:
        VAO_attrib() = default;
    };

    // named attribute
    template <size_t indx, class Attrib>
    class VAO_attrib<indx, Attrib, true> : protected virtual VAO_base,
        public VAO_attrib<indx, variable_traits_type<Attrib>>
    {
    public:
		
        using VAO_attrib<indx, variable_traits_type<Attrib>>::EnablePointer;
        using VAO_attrib<indx, variable_traits_type<Attrib>>::AttributePointer;

        void EnablePointer(tag_t<Attrib>)
        {
            VAO_attrib<indx, Attrib, false>::EnablePointer(tag_s<indx>());
        }
        void AttributePointer(VertexAttrib<Attrib>&& attrib, bool normalize = false)
        {
            VAO_attrib<indx, variable_traits_type<Attrib>>::AttributePointer(tag_s<indx>(),
                std::move(attrib), normalize);
        }

    protected:
        VAO_attrib() = default;
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

		// for run-time wrapper function
		template <size_t i>
		using PtrEnable = void(VAO_packed::*)(tag_s<i>) const;

		template <size_t i>
		constexpr static PtrEnable<i> enable_ptr = &VAO_packed::EnablePointer;

		template <size_t i>
		constexpr static PtrEnable<0> enable_ptr_0 =
			reinterpret_cast<PtrEnable<0>>(enable_ptr<i>);

	public:
        VAO_packed(HandleVAO&& handle = Allocator::Allocate(VAOTarget()))
			: VAO_base(std::move(handle))
		{}

        using VAO_attr<indx>::EnablePointer...;
        using VAO_attr<indx>::AttributePointer...;
		
        // run-time pointer activation
        void EnablePointer(size_t i)
        {
			constexpr static PtrEnable<0> enableTable[]{
				enable_ptr_0<indx>...
			};

			(this->*enableTable[i])(tag_s<0>());
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

//        constexpr static DummyPtrEnablePointer enableTable[]{ (GetPtrEnablePointer(tag_s<indx>())) ...};
	};

    template <class ... Attribs>
    using VAO = VAO_packed<std::tuple<Attribs...>>;

}