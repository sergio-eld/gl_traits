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
	template <class ... Attribs>
	class VAO
	{
		// What about glm::mat????
		static_assert(!std::disjunction_v<is_compound_attr<Attribs>...>,
			"Compaund attributes are not allowed in VAO!");
		HandleVAO handle_;

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

	public:
		VAO(HandleVAO&& handle = Allocator::Allocate(VAOTarget()))
			: handle_(std::move(handle))
		{
			if (!handle_)
				throw std::exception("Invalid VAO handle");
		}

		void Bind() const
		{
			ActiveVAO::Bind(handle_);
		}

		void UnBind() const
		{
			ActiveVAO::UnBind();
		}

		// TODO: generate overloaded functions for all the Attributes?
		// No runtime safety:
		// - VAO does not know if it is bound or not;
		// - VAO does not know if a Vertex Attribute belongs to a bound Buffer
		template <size_t indx, typename A>
		void AttributePointer(VertexAttrib<A>&& attrib, tag_s<indx>, bool normalize = false)
		{
			static_assert(std::is_same_v<A, nth_element_t<indx, Attribs...>>,
				"AttributePointer::Invalid Attribute type for provided index!");
			assert(ActiveVAO::IsBound(handle_) &&
				"Setting Vertex Attribute for non-active VAO");

			auto sz = (GLint)vao_attrib_size<A>()();
			GLenum glType = (GLenum)c_to_gl_v<A>;
			GLsizei stride = (GLsizei)attrib.Stride();
			void *voffset = (void*)attrib.Offset();

			glVertexAttribPointer((GLuint)indx,
				sz,
				glType,
				normalize,
				stride,
				voffset);
		}

		void EnableVertexAttribPointer(size_t indx) const
		{
			assert(ActiveVAO::IsBound(handle_));

			// TODO: move to another class
			glEnableVertexAttribArray((GLuint)indx);
		}

		/*
		Defines a vertex attribute of Name or at Index
		1. by index
		- Need to provide an index
		- compare the Input type for equivalence with the type at Index
		- using name
		*/
		template <size_t indx, typename Attr>
		void AttributePointer(const Buffer<Attr>& buffer, bool normalized = false)
		{

		}
	};
}