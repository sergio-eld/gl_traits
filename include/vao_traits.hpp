#pragma once

#include "buffer_traits.hpp"

namespace glt
{

    class VAO_base
    {
        static VAO_base* active_vao_;

        static void Register(VAO_base* vao = nullptr)
        {
            active_vao_ = vao;
        }

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

        VAO_base(const VAO_base&) = delete;
        VAO_base& operator=(const VAO_base&) = delete;

        VAO_base(VAO_base&& other)
            : handle_(std::move(other.handle_))
        {
            if (other.IsBound())
                Register(this);
        }

        VAO_base& operator=(VAO_base&& other)
        {
            handle_ = std::move(other.handle_);

            if (other.IsBound())
                Register(this);

            return *this;
        }

    public:
        void Bind()
        {
            glBindVertexArray(handle_accessor(handle_));
            Register(this);
        }

        bool IsBound() const
        {
            return this == active_vao_;
        }

        void UnBind()
        {
            assert(IsBound() && "Unbinding non-bound VAO!");
            glBindVertexArray(0);
            Register();
        }

    };


    // has_name_v = false
    // Attrib is not compound!
    template <size_t indx, class Attrib, bool = has_name_v<Attrib>>
    class VAO_attrib
    {
        const VAO_base& rVao_;

    public:
		
        // TODO: track enabled pointers?
        void EnablePointer(tag_s<indx>) const
        {
            assert(rVao_.IsBound());
            glEnableVertexAttribArray((GLuint)indx);
        }

        void AttributePointer(tag_s<indx>, AttribPtr<Attrib>&& attrib, bool normalize = false)
        {
            assert(rVao_.IsBound() &&
                "Setting Vertex Attribute for non-active VAO");

            glVertexAttribPointer((GLuint)indx,
                AttribPtr<Attrib>::size,
                AttribPtr<Attrib>::glType,
                normalize,
                attrib.stride,
                (void*)attrib.offset);
        }

    protected:
        VAO_attrib(const VAO_base& rVao)
            : rVao_(rVao)
        {}
    };

    // named attribute
    template <size_t indx, class Attrib>
    class VAO_attrib<indx, Attrib, true> : // protected virtual VAO_base,
        public VAO_attrib<indx, variable_traits_type<Attrib>>
    {

        using vao_attrib_nameless = VAO_attrib<indx, variable_traits_type<Attrib>>;

    public:
		
        using vao_attrib_nameless::EnablePointer;
        using vao_attrib_nameless::AttributePointer;

        void EnablePointer(tag_t<Attrib>)
        {
            vao_attrib_nameless::EnablePointer(tag_s<indx>());
        }
        void AttributePointer(AttribPtr<Attrib>&& attrib, bool normalize = false)
        {
            vao_attrib_nameless::AttributePointer(tag_s<indx>(),
                std::move(attrib), normalize);
        }

    protected:
        VAO_attrib(const VAO_base& rVao)
            : vao_attrib_nameless(rVao)
        {}
    };

 
    template <class AttribTuple, class = decltype(std::make_index_sequence<std::tuple_size_v<AttribTuple>>())>
    class VAO_packed;

	/*
	Implements glVertexAttribPointer usage

    // TODO: Attributes locations may be undefined in source code. Meaning that 
    attribute location can be changed dynamically  before linkage, several attributes can point to the same location.
    Need to store that information?

    Need to distinct between VAO with statically defined (in-source) and undefined
    attributes' locations?

    For dynamic definitions need to restrict different types for pointing to the same location

	// TODO: exclude compound attributes, VAO is agnostic to how a buffer stores data
	*/
	template <class ... Attribs, size_t ... indx>
	class VAO_packed<std::tuple<Attribs...>, std::index_sequence<indx...>> : public VAO_base,
        VAO_attrib<indx, Attribs> ...
	{

        template <size_t i>
        using VAO_attr = VAO_attrib<i, std::tuple_element_t<i, std::tuple<Attribs...>>>;


		// What about glm::mat????
//		static_assert(!std::disjunction_v<is_compound_seq<Attribs>, ...>,
//			"Compound attributes are not allowed in VAO!");

        static_assert(all_names_unique<Attribs...>(),
            "All attributes in a VAO must have unique aliases!");

		// for run-time wrapper function
		template <size_t i>
		using PtrEnable = void(VAO_packed::*)(tag_s<i>) const;

		template <size_t i>
		constexpr static PtrEnable<i> enable_ptr = &VAO_packed::EnablePointer;

        // dummy to typename to store function pointers
		template <size_t i>
		constexpr static PtrEnable<0> enable_ptr_0 =
			reinterpret_cast<PtrEnable<0>>(enable_ptr<i>);

	public:
        VAO_packed(HandleVAO&& handle = Allocator::Allocate(VAOTarget()))
			: VAO_base(std::move(handle)),
            VAO_attr<indx>(*this)...
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
 
        void EnablePointers()
        {
            (EnablePointer(tag_s<indx>()), ...);
        }

        // TODO: enable vertex attribute pointer
        // - by name
        // - by index
        // - enable all

    private:
   
	};

    template <class ... Attribs>
    using VAO = VAO_packed<std::tuple<Attribs...>>;

}