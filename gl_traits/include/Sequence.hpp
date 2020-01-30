#pragma once

#include "basic_types.hpp"
#include "sequence_layout.hpp"


namespace glt
{
	
	template <class ... Attrs>
	class seq_data_input
	{
		buffer_base& buf_;
		const seq_layout_info<Attrs...>& layout_;
	
	public:
		seq_data_input(buffer_base& buf,
			const seq_layout_info<Attrs...>& layout)
			: buf_(buf),
			layout_(layout)
		{}

		void SubData(compound_t<Attrs...> *data, size_t sz, size_t inst_offset = 0)
		{
			assert(!buf_.IsMapped() && "Copying data to mapped buffer!");
			assert(buf_.IsBound() && "Copying data to non-bound buffer!");
			assert(layout_.Allocated() >= sz + inst_offset && "Data exceeds buffer's bounds!");

			glBufferSubData((GLenum)buf_.Bound(),
				TotalOffsetBytes(inst_offset),
				LengthBytes(sz),
				data);
		}

		void MapRange(compound_t<Attrs...> *&data, MapAccessBit access,
			size_t sz = std::numeric_limits<size_t>::max(),
			size_t inst_offset = 0)
		{
			assert(access != MapAccessBit::none && "MapAccessBit is none!");
			sz = (sz == std::numeric_limits<size_t>::max()) ? layout_.Allocated() : sz;

			assert(!buf_.IsMapped() && "Mapping data to mapped buffer!");
			assert(buf_.IsBound() && "Mapping data to non-bound buffer!");
			assert(layout_.Allocated() >= sz + inst_offset && "Map Range exceeds sequence's bounds!");

			data = (compound_t<Attrs...>*)glMapBufferRange((GLenum)buf_.Bound(),
				TotalOffsetBytes(inst_offset),
				LengthBytes(sz),
				(GLbitfield)access);

			assert(data && "Failed to map buffer range!");

			buf_.SetMapAccessBit(access);
		}

        constexpr bool IsMapped() const
        {
            return buf_.IsMapped();
        }

        void UnMap()
        {
            buf_.UnMap();
        }

	private:

		constexpr GLintptr TotalOffsetBytes(size_t inst_offset) const
		{
			return (GLintptr)layout_.BufferOffset() +
				(GLintptr)(layout_.elem_size * inst_offset);
		}

		constexpr GLsizeiptr LengthBytes(size_t elems)
		{
			return layout_.elem_size * elems;
		}
	};

	template <class ... Attrs>
	class Sequence : aggregate_attribs<compound<Attrs...>>
	{
		seq_layout_info<Attrs...> layout_;
		seq_data_input<Attrs...> dataInput_;

		using aggr_attribs = aggregate_attribs<compound<Attrs...>>;

	public:
		Sequence(buffer_base& buf,
			const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: aggr_attribs(layout_),
			layout_(bytes_lbound, bytes_rbound),
			dataInput_(buf, layout_)
		{}

		Sequence(const Sequence&) = delete;
		Sequence& operator=(const Sequence&) = delete;

        constexpr static size_t elem_size = 
            seq_layout_info<Attrs...>::elem_size;

		using aggr_attribs::AttribPointer;
		using aggr_attribs::operator();

		constexpr size_t Stride() const
		{
			return layout_.Stride();
		}

		constexpr size_t Allocated() const
		{
			return layout_.Allocated();
		}

		constexpr std::ptrdiff_t BufferOffset() const
		{
			return layout_.BufferOffset();
		}

		void SubData(compound_t<Attrs...> *data, size_t sz, size_t inst_offset = 0)
		{
			dataInput_.SubData(data, sz, inst_offset);
		}

		void MapRange(compound_t<Attrs...> *&data, MapAccessBit access,
			size_t sz = std::numeric_limits<size_t>::max(),
			size_t inst_offset = 0)
		{
			dataInput_.MapRange(data, access, sz, inst_offset);
		}

		template <class AttrT,
			class = std::enable_if_t<is_equivalent_v<AttrT, compound<Attrs...>>>>
			void SubData(AttrT *data, size_t sz, size_t inst_offset = 0)
		{
			SubData(reinterpret_cast<compound_t<Attrs...>*>(data), sz, inst_offset);
		}

		template <class AttrT,
			class = std::enable_if_t<is_equivalent_v<AttrT, compound<Attrs...>>>>
			void MapRange(AttrT *&data, MapAccessBit access,
				size_t sz = std::numeric_limits<size_t>::max(),
				size_t inst_offset = 0)
		{
			MapRange(reinterpret_cast<compound_t<Attrs...> *&>(data),
				access, sz, inst_offset);
		}

		constexpr bool IsMapped() const
		{
			return dataInput_.IsMapped();
		}

		void UnMap()
		{
			return dataInput_.UnMap();
		}

	};
	
    // template wrapper accessor for template iterators
    template <typename ... attr>
    class SeqIteratorOut
    {
        using cl_tuple = std::tuple<attr...>;

        glt::compound<attr...> *ptr_;

    public:

        constexpr static size_t elem_size = get_class_size_v<attr...>;

        using difference_type = std::ptrdiff_t;

        using value_type = glt::compound<attr...>;
        using pointer = value_type * ;
        using reference = value_type & ;
        using iterator_category = std::random_access_iterator_tag;

        explicit SeqIteratorOut(pointer ptr = nullptr)
            : ptr_(ptr)
        {}

        template <class T, class = std::enable_if_t<is_equivalent_v<T, value_type>>>
        explicit SeqIteratorOut(T *ptr = nullptr)
            : SeqIteratorOut(reinterpret_cast<pointer>(ptr))
        {}

        SeqIteratorOut& operator++()
        {
            assert(*this);
            //std::next(ptr_); // why this doesn't work???
            ++ptr_;
            return *this;
        }

        // does std::next use this?
        SeqIteratorOut& operator+=(difference_type sz)
        {
            assert(*this);
            // std::next(ptr_, sz);
            return *this;
        }

        SeqIteratorOut operator++(int)
        {
            value_type *ptr = ptr_;
            //std::next(ptr_); // why this doesn't work???
            ++ptr_;
            return SeqIteratorOut(ptr);
        }

        operator bool() const
        {
            return (bool)ptr_;
        }

        bool operator==(const SeqIteratorOut& other) const
        {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const SeqIteratorOut& other) const
        {
            return !((*this) == other);
        }

        reference operator*()
        {
            assert(*this);
            return *ptr_;
        }

        const reference operator*() const
        {
            assert(*this);
            return *ptr_;
        }


        template <class T, class = std::enable_if_t<is_equivalent_v<T, attr...>>>
        T& operator*()
        {
            return reinterpret_cast<T&>(**this)
        }

        template <class T, class = std::enable_if_t<is_equivalent_v<T, attr...>>>
        const T& operator*() const
        {
            return reinterpret_cast<const T&>(**this)
        }

    };

    template <typename ... Attrs>
    class MapGuard
    {
    public:
        using iterator_out = SeqIteratorOut<Attrs...>;

    private:

        Sequence<Attrs...>& seq_;
        iterator_out start_,
            end_;

    public:

        MapGuard(Sequence<Attrs...>& seq, MapAccessBit accessBit)
            : seq_(seq)
        {
            glt::compound_t<Attrs...> *start = nullptr,
                *end = nullptr;

            seq_.MapRange(start, accessBit, seq_.Allocated());

            start_ = iterator_out(start);
            end_ = iterator_out(std::next(start, seq_.Allocated()));
        }

    public:

        MapGuard() = delete;
        MapGuard(const MapGuard&) = delete;
        MapGuard(MapGuard&&) = delete;
        MapGuard& operator=(const MapGuard&) = delete;
        MapGuard& operator=(MapGuard&&) = delete;


        // TODO: const_iterator?
        iterator_out begin()
        {
            return start_;
        }

        iterator_out end()
        {
            return end_;
        }

        ~MapGuard()
        {
            assert(seq_.IsMapped() &&
                "Sequence has been unmapped before guard's destruction!");
            seq_.UnMap();
        }
    };

}


