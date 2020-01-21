#pragma once

#include "equivalence.hpp"

#include "glm/glm.hpp"

#include "enums.hpp"
#include "glslt_traits.hpp"
#include "type_converions.hpp"

#include "gltHandle.hpp"

namespace glt
{
	// TODO: rename to Buffer_state class?
	// TODO: remove Handle object to the buffer class?
	class Buffer_base
	{
		// this may be optimized out in release?
		static std::map<BufferTarget, Buffer_base*> targets_;

		// will unregister previous buffer and register new ptr
		static void Register(BufferTarget target, Buffer_base* ptr = nullptr)
		{
			Buffer_base *&current = targets_[target];
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

		constexpr Buffer_base(HandleBuffer&& handle)
			: handle_(std::move(handle))
		{
			assert(handle_ && "Invalid Handle!");
		}

		Buffer_base(const Buffer_base&) = delete;
		Buffer_base& operator=(const Buffer_base& other) = delete;

		constexpr Buffer_base(Buffer_base&& other)
			: handle_(std::move(other.handle_)),
			mapAccess_(other.mapAccess_),
			mapAccessBit_(other.mapAccessBit_)
		{
			if (other.Bound() != BufferTarget::none)
				Bind(other.Bound());
			other.mapAccess_ = MapAccess::none;
			other.mapAccessBit_ = MapAccessBit::none;
		}

		Buffer_base& operator=(Buffer_base&& other)
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


		~Buffer_base()
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
}
