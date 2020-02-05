#pragma once

#include "sequence_layout.hpp"

namespace glt
{

    // has_name_v = false
    // Attrib is not compound!
    template <size_t indx, class Attrib, bool = has_name_v<Attrib>>
    class vao_attrib_modify
    {
        const vao_base& rVao_;

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

            using unwrapped_type = variable_traits_type<Attrib>;

			// for debug
			auto params = std::make_tuple(indx,
				sequence_traits<unwrapped_type>::elem_count,
				c_to_gl_v<unwrapped_type>,
				normalize,
				attrib.stride,
				attrib.offset);

            glVertexAttribPointer((GLuint)indx,
                (GLint)sequence_traits<unwrapped_type>::elem_count,
                (GLenum)c_to_gl_v<unwrapped_type>,
                normalize,
                (GLsizei)attrib.stride,
                (void*)attrib.offset);
        }
        
        template <typename T, typename = std::enable_if_t<is_equivalent_v<T, Attrib>>>
        void AttributePointer(tag_s<indx>, AttribPtr<T>&& attrib, bool normalize = false)
        {
            AttributePointer(tag_s<indx>(), 
                std::forward<AttribPtr<Attrib>>(reinterpret_cast<AttribPtr<Attrib>&&>(attrib)), normalize);
        }

    protected:
        vao_attrib_modify(const vao_base& rVao)
            : rVao_(rVao)
        {}
    };

    // named attribute
    template <size_t indx, class Attrib>
    class vao_attrib_modify<indx, Attrib, true> : // protected virtual vao_base,
        public vao_attrib_modify<indx, variable_traits_type<Attrib>>
    {
        using vao_attrib_nameless = vao_attrib_modify<indx, variable_traits_type<Attrib>>;

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
        vao_attrib_modify(const vao_base& rVao)
            : vao_attrib_nameless(rVao)
        {}
    };

 
    template <class AttribTuple, class = decltype(std::make_index_sequence<std::tuple_size_v<AttribTuple>>())>
    struct aggregated_vao_attribs;

    template <class ... Attribs, size_t ... indx>
    class aggregated_vao_attribs<std::tuple<Attribs...>, std::index_sequence<indx...>> :
        public vao_attrib_modify<indx, Attribs> ...
    {
        static_assert(all_names_unique<Attribs...>(),
            "All attributes in a VAO must have unique aliases!");

        template <size_t i>
        using vao_attrib_i =
            vao_attrib_modify<i, std::tuple_element_t<i, std::tuple<Attribs...>>>;

        // for run-time wrapper function
        template <size_t i>
        using PtrEnable = void(aggregated_vao_attribs::*)(tag_s<i>) const;

        template <size_t i>
        constexpr static PtrEnable<i> enable_ptr = &aggregated_vao_attribs::EnablePointer;

        // dummy to typename to store function pointers
        template <size_t i>
        constexpr static PtrEnable<0> enable_ptr_0 =
            reinterpret_cast<PtrEnable<0>>(enable_ptr<i>);

    public:

        aggregated_vao_attribs(const vao_base& vao)
            : vao_attrib_i<indx>(vao)...
        {}

        using vao_attrib_i<indx>::EnablePointer...;
        using vao_attrib_i<indx>::AttributePointer...;

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
    };

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

	template <class ... Attribs>
	class VAO : public vao_base, 
        public aggregated_vao_attribs<std::tuple<Attribs...>>
	{
        using aggr_attribs = aggregated_vao_attribs<std::tuple<Attribs...>>;

		// What about glm::mat????
//		static_assert(!std::disjunction_v<is_compound_seq<Attribs>, ...>,
//			"Compound attributes are not allowed in VAO!");

	public:
        VAO(HandleVAO&& handle = Allocator::Allocate(VAOTarget()))
			: vao_base(std::move(handle)),
            aggr_attribs(static_cast<const vao_base&>(*this))
		{}

        using aggr_attribs::EnablePointer;
        using aggr_attribs::EnablePointers;
        using aggr_attribs::AttributePointer;
		   
	};


}