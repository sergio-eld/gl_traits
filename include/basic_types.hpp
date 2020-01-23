#pragma once

/*
TODO: rename *classname*_base to *classname*_state (?)

Since glGet() is considered to be slow, the solution might be to store the state.
However, querying state might use 2 implementation, depending on configuration:
1. Using stored state;
2. Using glGet;

TODO: add configuration-driven implementation either using a MACRO of if constexpr.
*/

#include "equivalence.hpp"

#include "glm/glm.hpp"

#include "enums.hpp"
#include "glslt_traits.hpp"
#include "type_converions.hpp"

#include "gltHandle.hpp"

namespace glt
{
	template <class Attr>
	struct sequence_traits
	{
		constexpr static size_t elem_count = 1;
		constexpr static bool is_compound = false;
		constexpr static size_t elem_size = sizeof(Attr);

		using first_type = Attr;
	};

	template <class ... Attrs>
	struct sequence_traits<compound<Attrs...>>
	{
		constexpr static size_t elem_count = sizeof...(Attrs);
		constexpr static bool is_compound = elem_count > 1 ? true : false;
		constexpr static size_t elem_size =
			get_class_size_v<Attrs...>;

		using first_type = std::tuple_element_t<0, std::tuple<Attrs...>>;

	};

	template<glm::length_t R, typename T, glm::qualifier Q>
	struct sequence_traits<glm::vec<R, T, Q>>
	{
		constexpr static size_t elem_count = R;
		constexpr static bool is_compound = false;
		constexpr static size_t elem_size = sizeof(T);

		using first_type = T;

	};

	template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct sequence_traits<glm::mat<C, R, T, Q>>
	{
		constexpr static size_t elem_count = C;
		constexpr static bool is_compound = true;
		constexpr static size_t elem_size = sizeof(glm::vec<R, T, Q>);

		using first_type = glm::vec<R, T, Q>;

	};

	template <class T>
	constexpr inline bool is_compound_seq_v = sequence_traits<T>::is_compound;

	template <class T>
	constexpr inline size_t seq_elem_size = sequence_traits<T>::elem_size;

	template <class T>
	constexpr inline size_t seq_elem_count = sequence_traits<T>::elem_count;

	template <class T>
	using seq_first_type = typename sequence_traits<T>::first_type;

	// TODO: rename to buffer_state class?
	// TODO: remove Handle object to the buffer class?
	class buffer_base
	{
		// this may be optimized out in release?
		static std::map<BufferTarget, buffer_base*> targets_;

		// will unregister previous buffer and register new ptr
		static void Register(BufferTarget target, buffer_base* ptr = nullptr)
		{
			buffer_base *&current = targets_[target];
			if (current)
				current->target_ = BufferTarget::none;

			current = ptr;
		}


	protected:

		HandleBuffer handle_;

		// one buffer can be mapped to multiple targets (?)
		BufferTarget target_ = BufferTarget::none;
		BufUsage currentUsage_ = BufUsage::none;

		MapAccess mapAccess_ = MapAccess::none;
		MapAccessBit mapAccessBit_ = MapAccessBit::none;

		constexpr buffer_base(HandleBuffer&& handle)
			: handle_(std::move(handle))
		{
			assert(handle_ && "Invalid Handle!");
		}

		buffer_base(const buffer_base&) = delete;
		buffer_base& operator=(const buffer_base& other) = delete;

		constexpr buffer_base(buffer_base&& other)
			: handle_(std::move(other.handle_)),
			mapAccess_(other.mapAccess_),
			mapAccessBit_(other.mapAccessBit_)
		{
			if (other.Bound() != BufferTarget::none)
				Bind(other.Bound());
			other.mapAccess_ = MapAccess::none;
			other.mapAccessBit_ = MapAccessBit::none;
		}

		buffer_base& operator=(buffer_base&& other)
		{
			handle_ = std::move(other.handle_);
			mapAccess_ = other.mapAccess_;
			mapAccessBit_ = other.mapAccessBit_;

			if (other.Bound() != BufferTarget::none)
				Bind(other.Bound());
			other.mapAccess_ = MapAccess::none;
			other.mapAccessBit_ = MapAccessBit::none;

			return *this;
		}


		~buffer_base()
		{
			// does opengl auto unmap data?
			if (IsMapped())
				UnMap();

			if (IsBound())
				Register(Bound());

		}
	public:

		// Will register current buffer for the given target and unregister previous if any
		void Bind(BufferTarget target)
		{
			glBindBuffer((GLenum)target, handle_accessor(handle_));
			Register(target, this);
			target_ = target;
		}

		constexpr BufferTarget Bound() const
		{
			return target_;
		}

		constexpr bool IsBound() const
		{
			return Bound() != BufferTarget::none;
		}

		void UnBind()
		{
			assert(IsBound() && "Buffer is alraedy not bound");
			if (Bound() == BufferTarget::none)
				throw std::exception("Trying to unbind non-bound buffer!");
			Register(target_);
		}

		constexpr MapAccess MapAccess() const
		{
			return mapAccess_;
		}

		constexpr MapAccessBit MapAccessBit() const
		{
			return mapAccessBit_;
		}

		constexpr bool IsMapped() const
		{
			return MapAccess() != MapAccess::none ||
				MapAccessBit() != MapAccessBit::none;
		}

		constexpr void SetMapAccess(glt::MapAccess access)
		{
			mapAccess_ = access;
		}

		constexpr void SetMapAccessBit(glt::MapAccessBit access)
		{
			mapAccessBit_ = access;
		}

		void UnMap()
		{
			assert(IsBound() && "Unmapping buffer that is not bound!");
			assert(IsMapped() && "Unmapping buffer that is not mapped!");

			glUnmapBuffer((GLenum)Bound());
			SetMapAccess(glt::MapAccess::none);
			SetMapAccessBit(glt::MapAccessBit::none);
		}
	};

    class vao_base
    {
        static vao_base* active_vao_;



    protected:

		// TODO: move handle to private
        HandleVAO handle_;

        vao_base(HandleVAO&& handle)
            : handle_(std::move(handle))
        {
            assert(handle_ && "Invalid VAO handle!");
        }

        vao_base()
            : handle_(Allocator::Allocate(VAOTarget()))
        {
            assert(false && "vao_base default constructor called!");
        }

        vao_base(const vao_base&) = delete;
        vao_base& operator=(const vao_base&) = delete;

        vao_base(vao_base&& other)
            : handle_(std::move(other.handle_))
        {
            if (other.IsBound())
                Register(this);
        }

        vao_base& operator=(vao_base&& other)
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

	private:

		static void Register(vao_base* vao = nullptr)
		{
			active_vao_ = vao;
		}

    };

	class program_base
	{

		static program_base *active_prog_;

		HandleProg handle_;
		bool linked_ = false;

	protected:

		program_base(HandleProg&& handle)
			: handle_(std::move(handle))
		{
			assert(*this && "Invalid handle!");
		}

		program_base(const program_base&) = delete;
		program_base& operator=(const program_base&) = delete;

		program_base(program_base&& other)
			: handle_(std::move(other.handle_)),
			linked_(other.linked_)
		{
			if (other.IsActive())
				Register(this);

			other.linked_ = false;
		}

		program_base& operator=(program_base&& other)
		{
			handle_ = std::move(other.handle_);
			linked_ = other.linked_;

			if (other.IsActive())
				Register(this);

			other.linked_ = false;

			return *this;
		}

		operator bool() const
		{
			return handle_;
		}

		void SetLinkStatus(bool linked)
		{
			linked_ = linked;
		}

		void Use()
		{
			assert(*this && "Attemplt to use invalid program!");
			glUseProgram(handle_accessor(handle_));
			Register(this);
		}

		void UnUse()
		{
			assert(IsActive() && "Attempt to unuse non-active program!");
			glUseProgram(0);
			Register();
		}

	private:

		static void Register(program_base *prog = nullptr)
		{
			active_prog_ = prog;
		}

	public:

		bool IsActive() const
		{
			return this == active_prog_;
		}

		bool Linked() const
		{
			return linked_;
		}

		const HandleProg& Handle() const
		{
			return handle_;
		}		

	};

	class uniform_base
	{
	protected:

		const program_base& prog_;
		GLint location_ = -1;

		// by default
		uniform_base(const program_base& prog, const char* name)
			: prog_(prog),
			location_(prog_.Linked() ? GetLocation(name) : -1)
		{}

		GLint GetLocation(const char* name)
		{
			assert(prog_.Linked() &&
				"Attempt to get Uniform location of a non-linked program");
			GLint ret = glGetUniformLocation(handle_accessor(prog_.Handle()), name);
			assert(ret != -1 && "Failed to get Unifrom location");
			return ret;
		}

	private:



	};



}
