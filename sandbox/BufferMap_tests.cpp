
#include "helpers.hpp"


using namespace glt;

struct Coord
{
	glm::vec3 xyz;
};

int main()
{
	std::is_aggregate_v<vertex>;
	std::is_constructible_v<vertex, glm::vec3()>;

	is_aggregate_initializable_from_tuple<vertex, std::tuple<glm::vec3, glm::vec2>>::value;
	is_aggregate_initializable_v<vertex, glm::vec3, glm::vec2>;

	SmartGLFW sglfw{ 4, 5 };

	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers' mapping" };
	sglfw.MakeContextCurrent(window);
	sglfw.LoadOpenGL();

	auto namedbufferdata = &glNamedBufferData;



	{
		BufferOld<glm::vec3, glm::vec2> buffer{};

		std::vector<glm::vec3> cube_positions = glm_cube_positions();
		std::vector<glm::vec2> cube_tex_coords = glm_cube_texCoords();

		buffer.Bind(tag_v<BufferTarget::array>());
		buffer.AllocateMemory(cube_positions.size(), cube_tex_coords.size(), BufUsage::static_draw);
		buffer.BufferData<0>(cube_positions.data(), cube_positions.size());

		// testing offset. TODO: move to another unit test
		if (buffer.GetOffset(tag_s<0>()) != 0 ||
			buffer.GetOffset(tag_s<1>()) != sizeof(glm::vec3) * cube_positions.size())
		{
			std::cerr << "Invalid offset recieved!" << std::endl;
			return -1;
		}

		BufferMap<glm::vec3, MapAccess::read_only> mapped =
			buffer.MapBuffer<0, MapAccess::read_only>();

		auto iter_position = cube_positions.cbegin();
		for (auto map_iter : mapped)
			if (map_iter != *iter_position++)
			{
				std::cerr << "Mapped data differs from initial!" << std::endl;
				return -1;
			}

		mapped.UnMap();
		mapped = buffer.MapBuffer<0, MapAccess::read_only>();
	}

	// Loading user-defined vertexes
	{
		using CmpdBuffer = BufferOld<compound<glm::vec3, glm::vec2>>;

		std::vector<vertex> vertices = cube_vertexes();

		CmpdBuffer buffer{};
		buffer.Bind(tag_v<BufferTarget::array>());
		buffer.AllocateMemory(vertices.size(), BufUsage::static_draw);

		// testing offset. TODO: move to another unit test
		if (buffer.GetOffset(tag_s<0>(), tag_s<0>()) != 0 ||
			buffer.GetOffset(tag_s<0>(), tag_s<1>()) != sizeof(glm::vec3))
		{
			std::cerr << "Invalid offset recieved!" << std::endl;
			return -1;
		}


		/*
		static_assert(std::is_same_v<CmpdBuffer::NthType_t<0>, compound<glm::vec3, glm::vec2>>);
		static_assert(glt::is_equivalent_v<compound<glm::vec3, glm::vec2>, vertex>);
		static_assert(glt::is_equivalent_v<CmpdBuffer::NthType_t<0>, vertex>);*/

		buffer.BufferData(vertices.data(), vertices.size());

		// BufferMap<Coord, MapAccess::read_only> mapped = // assertion fails, not equivalent
		//	buffer.MapBuffer<0, MapAccess::read_only>(glt::tag_t<Coord>());

		try 
		{
			BufferMap<vertex, MapAccess::read_only> mapped =
				buffer.MapBuffer<0, MapAccess::read_only>(tag_t<vertex>());

			auto iter_position = vertices.cbegin();
			for (const auto& map_iter : mapped)
				if (map_iter != *iter_position++)
				{
					std::cerr << "Mapped data differs from initial!" << std::endl;
					return -1;
				}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			return -1;
		}
	}

	return 0;

}


////////////////////////////////////////////////////
   ////////////////////////////////////////////////////
   /*
   template <class>
   class MapStatus_;

   // this can be acquired via OpenGL function
   template <BufferTarget ... targets>
   class MapStatus_<std::integer_sequence<BufferTarget, targets...>>
   {
	   inline static std::map<BufferTarget, MapAccess>
		   mapStatus_{ std::pair(targets, (MapAccess)0) ... };

   public:

	   static MapAccess MappedStatus(BufferTarget target)
	   {
		   auto found = mapStatus_.find(target);
		   assert(found != mapStatus_.cend() && "Target is not registered!");
		   // or use glGet ?

		   return found->second;
	   }

	   static bool IsMapped(BufferTarget target)
	   {
		   return (bool)MappedStatus(target);
	   }

	   static void SetMapStatus(BufferTarget target, MapAccess type)
	   {
		   auto found = mapStatus_.find(target);
		   assert(found != mapStatus_.cend() && "Target is not registered!");
		   found->second = type;
	   }
   };

   using MapStatus = MapStatus_<BufferTargetList>;

   // TODO: unite with Buffer and replace with gltMapGuard?
   // or use the same collection of template attribute params:
   //
   template <typename T, MapAccess S = MapAccess::read_write>
   class BufferMap
   {
	   //const IBindable<BufferTarget> *handle_;
	   BufferTarget target_;
	   T *start_,
		   *end_;
	   bool unmapped_ = false;

   public:
	   BufferMap(T* start,
		   T* end,
		   BufferTarget target)
		   : start_(start),
		   end_(end),
		   target_(target)
	   {
		   // maybe this checks should do Buffer class?
		   // Buffer also has to check for mapping when allocating memory or buffering data!
		   //if (!bound_handle<BufferTargetList>::IsBoundRT(target_, handle_))
			   //throw("Attempting to Map a buffer which has not been bound!");

		   assert(end_ > start_ && "Invalid range!");
	   }

	   BufferMap(const BufferMap<T, S>&) = delete;
	   BufferMap& operator=(const BufferMap<T, S>&) = delete;

	   //template <MapAccess S2>
	   BufferMap(BufferMap<T, S>&& other)
		   : target_(other.target_),
		   start_(other.start_),
		   end_(other.end_),
		   unmapped_(other.unmapped_)
	   {
		   other.start_ = nullptr;
		   other.end_ = nullptr;
		   other.unmapped_ = true;
	   }

	   BufferMap& operator=(BufferMap<T, S>&& other)
	   {
		   // two Maps may have different targets, current one must be unmapped
		   if (!unmapped_)
		   {
			   assert(other.unmapped_ || target_ == other.target_ &&
				   "Invalid scenario! Both maps are mapped to the same target!");
			   UnMap();
		   }

		   target_ = other.target_;
		   start_ = other.start_;
		   end_ = other.end_;
		   unmapped_ = other.unmapped_;

		   other.start_ = nullptr;
		   other.end_ = nullptr;
		   other.unmapped_ = true;

		   return *this;
	   }

	   size_t Size() const
	   {
		   return std::distance(start_, end);
	   }

	   class Iterator
	   {
		   T *ptr_;

	   public:
		   Iterator(T *const ptr)
			   : ptr_(ptr)
		   {}

		   bool operator!=(const Iterator& other) const
		   {
			   return ptr_ != other.ptr_;
		   }

		   Iterator& operator++()
		   {
			   ++ptr_;
			   return *this;
		   }

		   Iterator operator++(int)
		   {
			   Iterator tmp{ *this };
			   operator++();
			   return tmp;
		   }

		   T& operator*()
		   {
			   return *ptr_;
		   }
		   T operator*() const
		   {
			   return *ptr_;
		   }
	   };

	   class ConstIterator
	   {
		   T *ptr_;

	   public:
		   ConstIterator(T *const ptr)
			   : ptr_(ptr)
		   {}

		   bool operator!=(const ConstIterator& other) const
		   {
			   return ptr_ != other.ptr_;
		   }
		   ConstIterator& operator++()
		   {
			   ++ptr_;
			   return *this;
		   }

		   ConstIterator operator++(int)
		   {
			   ConstIterator tmp{ *this };
			   operator++();
			   return tmp;
		   }

		   const T& operator*()
		   {
			   return *ptr_;
		   }
		   T operator*() const
		   {
			   return *ptr_;
		   }

	   };



	   Iterator begin()
	   {
		   return Iterator(start_);
	   }

	   ConstIterator begin() const
	   {
		   return Iterator(start_);
	   }

	   Iterator end()
	   {
		   return Iterator(end_);
	   }
	   ConstIterator end() const
	   {
		   return ConstIterator(end_);
	   }

	   void UnMap()
	   {
		   glUnmapBuffer((GLenum)target_);
		   MapStatus::SetMapStatus(target_, (MapAccess)0);
		   unmapped_ = true;
		   start_ = nullptr;
		   end_ = nullptr;
	   }

	   ~BufferMap()
	   {
		   if (!unmapped_)
			   UnMap();
	   }
   };






   */

   /*
   This class is used to Set VertexAttributePointer.
   To set properties of a particular vertex attribute, one must fetch
   an instance of VertexAttrib class from a Buffer object, using
   VertexAttrib<A> Buffer::Attribute(tag_s<indx>) or
   VertexAttrib<A> Buffer::Attribute(tag_s<indx>, tag_s<subIndx>) for accessing
   Attributes withing Compounds:
   A - typename of an attribute,
   indx - Attribute's index (index of a Buffer's template parameter),
   subIndx - Attribute's index within the Compound Attribute
   */
   /*
	   template <class Attrib>
	   class VertexAttrib
	   {

		   std::ptrdiff_t offset_;

		   size_t stride_;

		   // TODO: stride // stride = sizeof(Atrrib)

	   public:

		   // TODO: move to private
		   constexpr VertexAttrib(std::ptrdiff_t offset, size_t stride = 0)
			   : offset_(offset),
			   stride_(stride)
		   {}

		   VertexAttrib(const VertexAttrib<Attrib>&) = delete;
		   VertexAttrib& operator=(const VertexAttrib<Attrib>&) = delete;

		   constexpr VertexAttrib(VertexAttrib<Attrib>&& other)
			   : offset_(other.offset_),
			   stride_(other.stride_)
		   {}

		   VertexAttrib& operator=(VertexAttrib<Attrib>&& other)
		   {
			   offset_ = other.offset_;
			   stride_ = other.stride_;
		   }

		   std::ptrdiff_t Offset() const
		   {
			   return offset_;
		   }
		   size_t Stride() const
		   {
			   return stride_;
		   }

		   ~VertexAttrib()
		   {}
	   private:


	   };

	   */
	   // TODO: add function to return the offset for nth type
		/*template <typename ... Attr>
	   class BufferOld : public IBindable<BufferTarget>
	   {

		   static_assert(!std::disjunction_v<is_compound_attr<Attr>...>,
			   "Only first type may be compound!");

		   constexpr static size_t num_attribs_ = sizeof...(Attr);

		   Handle<BufferTarget> handle_;
		   BufferTarget last_bound_ = BufferTarget::none;
		   std::array<size_t, num_attribs_> inst_allocated_ =
			   init_array<size_t, num_attribs_>()(0);

		   // remove this? Another class is responsible for mapping state check
		   MapAccess mapped_ = MapAccess(0);

	   public:

		   BufferOld(Handle<BufferTarget>&& handle = Allocator::Allocate(BufferTarget()))
			   : handle_(std::move(handle))
		   {
			   if (!handle_)
				   throw std::exception("glt::Buffer::Invalid handle");
		   }

		   template <BufferTarget target>
		   void Bind(tag_v<target> t)
		   {
			   last_bound_ = target;
			   gltActiveBufferTargets::Bind(t, *this);
		   }

		   // RunTime wrapper
		   void Bind(BufferTarget target)
		   {
			   last_bound_ = target;
			   return gltActiveBufferTargets::BindRT(target, *this);
		   }

		   template <BufferTarget target>
		   bool IsBound(tag_v<target>) const
		   {
			   return gltActiveBufferTargets::IsBound(tag_v<target>(), *this);
		   }

		   // RunTime wrapper
		   bool IsBound(BufferTarget target) const
		   {
			   return gltActiveBufferTargets::IsBoundRT(target, *this);
		   }

		   template <BufferTarget target>
		   void UnBind(tag_v<target> t)
		   {
			   last_bound_ = BufferTarget::none;
			   gltActiveBufferTargets::UnBind(t, *this);
		   }

		   // RunTime wrapper
		   void UnBind(BufferTarget target)
		   {
			   last_bound_ = BufferTarget::none;
			   return gltActiveBufferTargets::UnBindRT(target, *this);
		   }

		   const Handle<BufferTarget>& GetHandle() const
		   {
			   return handle_;
		   }

		   operator const Handle<BufferTarget>&() const
		   {
			   return GetHandle();
		   }

		   BufferTarget LastBound() const
		   {
			   return last_bound_;
		   }
		   */


		   /*
		   What to choose? glBufferData or glBufferStorage?
		   BufferStorage size is immutable and memory cannot be reallocated
		   - In case of glBufferData was used, would buffer resizing lead to invalidating
		   the data contained withing? --> Yes
		   - In case of glBufferStorage is used, it is safer to restrict access to AllocateMemory
		   function and call it inside the constructor.
		   - For glBufferStorage does it make sence to make a size-aware class similar to
		   std::array?
		   */
		   // TODO: case with named attributes
void AllocateMemory(convert_to<size_t, Attr> ... instN, BufUsage usage)
{
	ValidateBind();

	size_t total_sz = ((sizeof(Attr) * instN) + ...);// sizeof(First) * inst0; //size_of_N<First>(inst0);
	// if constexpr (sizeof...(Attr))
	//	total_sz += ((sizeof(Attr) * instN) + ...); //(size_of_N<Attr>(instN) + ...);

	glBufferData((GLenum)last_bound_, total_sz, nullptr, (GLenum)usage);
	// TODO: check for OpenGL errors
	modify_array<size_t, num_attribs_>::modify(inst_allocated_, /*inst0,*/ instN...);
}

size_t InstancesAllocated(size_t indx) const
{
	if (indx >= num_attribs_)
		throw std::out_of_range("Buffer's array index is out of range!");
	return inst_allocated_[indx];
}

// default case for Buffer with one template argument
template <size_t indx = 0,
	typename UT = nth_element_t<indx, /*First,*/ Attr...>>
	void BufferData(UT* data, size_t size, size_t offset = 0)
{
	static_assert(is_equivalent_v<nth_element_t<indx, /*First,*/ Attr...>, UT>,
		"Input typename is not equivalent to the buffer's one!");
	size_t total = size + offset;
	if (total > inst_allocated_[indx])
		throw std::range_error("Allocated range exceeded");

	// TODO: choose Named or default
	ValidateBind();
	using DataType = UT;

	GLintptr offset_bytes = GetOffset(tag_s<indx>()) +
		sizeof(UT) * offset,
		size_bytes = sizeof(UT) * size;

	glBufferSubData((GLenum)last_bound_, offset_bytes, size_bytes, data);

}




/*
BufferMap:
- when using glMapBuffer, the whole buffer is mapped.
But in case the Buffer is batched (contains several arrays of different types),
a single pointer can't be used. User must be able to gain access to the
starting pointer of each array within the buffer (or its mapped range).
0. How to ensure that the particular array's range has not been exceeded?
- functions that take data or expose pointers need to take size arguments
and check against range
- do not expose pointers at all?
1. What gl function to use when mapping buffer or its range?
- glMapBuffer always?
- glMapBuffer when accessing the whole buffer and glMapBufferRange for a part?
- Have 2 different functions for glMapBuffer and glMapBufferRange?
2. Who is responsible for exposing mapped pointers or recieving input data?
- BufferMap handles both data input and unmapping
- BufferMap serves as guard for unmapping data, Buffer handles data input
3. Make a separated function that expose pointers to be held during subsequent
drawing operations (glMapBufferRange with persistent bit)?
*/

// TODO: take user-defined type and check for equivalence?
// indx = 0 is default for a Buffer specialization with 1 attribute 
template <size_t indx = 0, MapAccess S = MapAccess::read_write,
	typename UT = nth_element_t<indx, /*First,*/ Attr...>>
	BufferMap<UT, S> MapBuffer(tag_t<UT> = tag_t<UT>())
{
	static_assert(is_equivalent_v<nth_element_t<indx, /*First,*/ Attr...>, UT>,
		"User-defined type is not equivalent to the buffer's one!");

	// TODO: choose Named or default function
	ValidateBind();

	if (MapStatus::IsMapped(last_bound_))
		throw std::exception("Target is already mapped!");

	// TODO: change to glMapBufferRange?
	UT* start = (UT*)glMapBuffer((GLenum)last_bound_, (GLenum)S);
	if (!start)
		throw std::exception("glMapBuffer returned nullptr!");

	UT* end = std::next(start, inst_allocated_[indx]);

	MapStatus::SetMapStatus(last_bound_, S);

	return BufferMap<UT, S>(start, end, last_bound_);
}


// TODO: refactor assertions. Ambiguous errors in wrong order
template <size_t indx, size_t indx_compound = 0>
std::ptrdiff_t GetOffset(tag_s<indx>, tag_s<indx_compound> = tag_s<indx_compound>()) const
{
	using AttrType = nth_element_t<indx, /*First,*/ Attr...>;
	static_assert(indx < num_attribs_, "Index is out of range!");
	constexpr size_t type_sizes[sizeof...(Attr) + 1]{ /*sizeof(First),*/ sizeof(Attr)... };

	std::ptrdiff_t res = 0;

	for (size_t i = 0; i != indx; ++i)
		res += type_sizes[i] * inst_allocated_[i];

	if constexpr (indx_compound)
	{
		static_assert(is_compound_seq_v<AttrType>,
			"Non-compound elements do not support sub-indexes");
		static_assert(indx_compound < compound_attr_count<AttrType>(),
			"Sub-index is out of range!");
		res += get_tuple_member_offset<indx_compound, AttrType>();
	}
	return res;
}

/* Fetch Vertex Attribute (Simple, Named or Compound) by index. */
template <size_t indx, class A =
	std::conditional_t<(indx < num_attribs_), nth_element_t<indx, Attr...>, void>>
	VertexAttrib<A> Attribute(tag_s<indx>) const
{
	static_assert(indx < num_attribs_, "Index is out of range!");

	if constexpr (is_compound_seq_v<A>)
		return VertexAttrib<A>(GetOffset(tag_s<indx>()), sizeof(A));
	else
		return VertexAttrib<A>(GetOffset(tag_s<indx>()));
}

// TODO: remove tag_s with tag taking 2 indices
/* Fetch a subIndex's Attribute from a compound attribute at index */
template <size_t indx, size_t subIndx, class Compound =
	std::conditional_t<(indx < num_attribs_), nth_element_t<indx, Attr...>, void>,
	class A = std::conditional_t<std::is_void_v<Compound>, void, nth_parameter_t<subIndx, Compound>>>
	VertexAttrib<A> Attribute(tag_indx<indx, subIndx>) const
{
	static_assert(indx < num_attribs_, "Index is out of range!");

	return VertexAttrib<A>(GetOffset(tag_s<indx>(), tag_s<subIndx>()), sizeof(Compound));
}

~BufferOld()
{
	// check if is still bound
	if ((bool)last_bound_ && IsBound(last_bound_))
		UnBind(last_bound_);
}

	private:
		void ValidateBind() const
		{
			if (!(bool)last_bound_ || !IsBound(last_bound_))
				throw std::exception("Allocating memory for non-active buffer!");
		}
	};

	template <class AttrList>
	struct BufferListWrapper
	{
		static_assert(is_tuple_v<AttrList>, "Template argument is not an Attribute List");
	};

	template <class ... Attrs>
	struct BufferListWrapper<AttrList<Attrs...>>
	{
		using type = BufferOld<Attrs...>;
	};

	template <class AttrList>
	using BufferL = typename BufferListWrapper<AttrList>::type;


