#pragma once

template <int i>
using gl_value = std::integral_constant<int, i>;

//pnames
struct gl_depth_stencil_texture_mode : public gl_value<GL_DEPTH_STENCIL_TEXTURE_MODE> {};
struct gl_texture_base_level : public gl_value<GL_TEXTURE_BASE_LEVEL> {};
struct gl_texture_compare_func : public gl_value<GL_TEXTURE_COMPARE_FUNC> {};
struct gl_texture_compare_mode : public gl_value<GL_TEXTURE_COMPARE_MODE> {};
struct gl_texture_lod_bias : public gl_value<GL_TEXTURE_LOD_BIAS> {};
struct gl_texture_min_filter : public gl_value<GL_TEXTURE_MIN_FILTER> {};
struct gl_texture_mag_filter : public gl_value<GL_TEXTURE_MAG_FILTER> {};
struct gl_texture_min_lod : public gl_value<GL_TEXTURE_MIN_LOD> {};
struct gl_texture_max_lod : public gl_value<GL_TEXTURE_MAX_LOD> {};
struct gl_texture_max_level : public gl_value<GL_TEXTURE_MAX_LEVEL> {};
struct gl_texture_swizzle_r : public gl_value<GL_TEXTURE_SWIZZLE_R> {};
struct gl_texture_swizzle_g : public gl_value<GL_TEXTURE_SWIZZLE_G> {};
struct gl_texture_swizzle_b : public gl_value<GL_TEXTURE_SWIZZLE_B> {};
struct gl_texture_swizzle_a : public gl_value<GL_TEXTURE_SWIZZLE_A> {};
struct gl_texture_wrap_s : public gl_value<GL_TEXTURE_WRAP_S> {};
struct gl_texture_wrap_t : public gl_value<GL_TEXTURE_WRAP_T> {};
struct gl_texture_wrap_r : public gl_value<GL_TEXTURE_WRAP_R> {};
struct gl_texture_border_color : public gl_value<GL_TEXTURE_BORDER_COLOR> {};
struct gl_texture_swizzle_rgba : public gl_value<GL_TEXTURE_SWIZZLE_RGBA> {};


//predefined values
struct gl_depth_component : public gl_value<GL_DEPTH_COMPONENT> {};
struct gl_stencil_components : public gl_value<GL_STENCIL_COMPONENTS> {};

struct gl_compare_ref_to_texture : public gl_value<GL_COMPARE_REF_TO_TEXTURE> {};

struct gl_nearest : public gl_value<GL_NEAREST> {};
struct gl_linear : public gl_value<GL_LINEAR> {};

struct gl_nearest_mipmap_nearest : public gl_value<GL_NEAREST_MIPMAP_NEAREST> {};
struct gl_linear_mipmamp_nearest : public gl_value<GL_LINEAR_MIPMAP_NEAREST> {};
struct gl_nearest_mipmap_linear : public gl_value<GL_NEAREST_MIPMAP_LINEAR> {};
struct gl_linear_mipmap_linear : public gl_value<GL_LINEAR_MIPMAP_LINEAR> {};

struct gl_clamp_to_edge : public gl_value<GL_CLAMP_TO_EDGE> {};
struct gl_clamp_to_border : public gl_value<GL_CLAMP_TO_BORDER> {};
struct gl_mirrored_repeat : public gl_value<GL_MIRRORED_REPEAT> {};
struct gl_repeat : public gl_value<GL_REPEAT> {};
struct gl_mirror_clamp_to_edge : public gl_value<GL_MIRROR_CLAMP_TO_EDGE> {};

class gltActiveTexture
{
#ifndef MAX_GL_TEXTURES
	constexpr static size_t max_gl_textures = 80;
#else
	constexpr static size_t max_gl_textures = MAX_GL_TEXTURES;
#endif

	static std::array<std::pair<glTargetTex, const void*>, max_gl_textures> textureUnits_;
	static size_t currentUnit_;

public:

	constexpr static size_t MaxGLTextures()
	{
		return max_gl_textures;
	}

	static size_t ActiveTexUnit()
	{
		return currentUnit_;
	}

	static void ActiveTextue(size_t texUnit) noexcept
	{
		assert(texUnit < max_gl_textures && "Texture unit out of bounds!");
		currentUnit_ = texUnit;
		glActiveTexture((GLenum)texUnit + GL_TEXTURE0);
	}


	template <glTargetTex target>
	static void BindTexture(const gltHandle<target>& hTexture, size_t texUnit = ActiveTexUnit())
	{
		if (texUnit != ActiveTexUnit())
			ActiveTextue(texUnit);
		glBindTexture((GLenum)target, hTexture);
		textureUnits_[texUnit] = std::make_pair(target, (const void*)&hTexture);
	}

	static std::pair<glTargetTex, const void*> CurrentTexUnit()
	{
		return textureUnits_[ActiveTexUnit()];
	}

	//ISSUE
	//TODO: check implementation!
	template <glTargetTex target>
	static bool IsCurrentTexture(const gltHandle<target>& hTexture)
	{
		assert(hTexture.IsValid() && "hTexture is nullptr!");
		std::pair<glTargetTex, const void*> curUnit = CurrentTexUnit();

		if (*(const gltHandle<target>*)curUnit.second != hTexture)
			return false;

		assert(curUnit.first == target &&
			"IsCurrentTexture false output. Invalid target type returned!");

		return true;
	}

};

  /*
class gltTexParam_traits
{
	// Targets validation 
	using gl_allowed_targets_1d = std::integer_sequence<glTargetTex,
		glTargetTex::texture_1d,
		glTargetTex::proxy_texture_1d>;

	using gl_allowed_targets_2d = std::integer_sequence<glTargetTex,
		glTargetTex::texture_2d,
		glTargetTex::proxy_texture_2d,
		glTargetTex::texture_1d_array,
		glTargetTex::proxy_texture_1d_array,
		glTargetTex::texture_rectangle,
		glTargetTex::proxy_texture_rectangle,
		glTargetTex::texture_cube_map_positive_x,
		glTargetTex::texture_cube_map_negative_x,
		glTargetTex::texture_cube_map_positive_y,
		glTargetTex::texture_cube_map_negative_y,
		glTargetTex::texture_cube_map_positive_z,
		glTargetTex::texture_cube_map_negative_z,
		glTargetTex::proxy_texture_cube_map>;

	using gl_allowed_targets_2d_multisample = std::integer_sequence<glTargetTex,
		glTargetTex::texture_2d_multisample,
		glTargetTex::proxy_texture_2d_multisample>;

	using gl_allowed_targets_3d = std::integer_sequence<glTargetTex,
		glTargetTex::texture_3d,
		glTargetTex::proxy_texture_3d,
		glTargetTex::texture_2d_array,
		glTargetTex::proxy_texture_2d_array>;

	using gl_allowed_targets_3d_multisample = std::integer_sequence<glTargetTex,
		glTargetTex::texture_2d_multisample_array,
		glTargetTex::proxy_texture_2d_multisample_array
	>;


	//TODO: write unit tester
	using map_allowed_targets = cexpr_generic_map<
		//1d
		cexpr_pair<auto_t<glTargetTex::texture_1d>, gl_allowed_targets_1d>,

		//2d
		cexpr_pair<auto_t<glTargetTex::texture_2d>, gl_allowed_targets_2d>,
		cexpr_pair<auto_t<glTargetTex::texture_1d_array>, gl_allowed_targets_2d>,
		cexpr_pair<auto_t<glTargetTex::texture_rectangle>, gl_allowed_targets_2d>,

		//2d_multisample
		cexpr_pair<auto_t<glTargetTex::texture_2d_multisample>, gl_allowed_targets_2d_multisample>,


		//3d
		cexpr_pair<auto_t<glTargetTex::texture_3d>, gl_allowed_targets_3d>,
		cexpr_pair<auto_t<glTargetTex::texture_2d_array>, gl_allowed_targets_3d>,

		//3d_multisample
		cexpr_pair<auto_t<glTargetTex::texture_2d_multisample_array>, gl_allowed_targets_3d_multisample>
	>;

	template <glTargetTex target>
	using allowed_targets_ = typename map_allowed_targets::found_pair<auto_t<target>>::value;

	template<glTargetTex target, glTargetTex texStorage, class allowed = typename allowed_targets_<target>>
	struct target_validate;

	// values validation
	using values_depth_stencil_texture_mode = std::tuple<gl_depth_component,
		gl_stencil_components>;
  
	using values_texture_compare_func = std::tuple<gl_lequal,
		gl_gequal,
		gl_less,
		gl_greater,
		gl_equal,
		gl_notequal,
		gl_always,
		gl_never
	>;

	using values_texture_compare_mode = std::tuple<
		gl_compare_ref_to_texture,
		gl_none
	>;

	using values_texture_min_filter = std::tuple<
		gl_linear,
		gl_nearest,
		gl_nearest_mipmap_nearest,
		gl_nearest_mipmap_linear,
		gl_linear_mipmamp_nearest,
		gl_linear_mipmap_linear
	>;

	using values_texture_mag_filter = std::tuple<
		gl_linear,
		gl_nearest
	>;

	using values_wrap = std::tuple<
		gl_clamp_to_edge,
		gl_clamp_to_border,
		gl_mirrored_repeat,
		gl_repeat,
		gl_mirror_clamp_to_edge
	>;

	using pname_values = cexpr_generic_map<
		cexpr_pair<gl_depth_stencil_texture_mode, values_depth_stencil_texture_mode>,
		cexpr_pair<gl_texture_compare_func, values_texture_compare_func>,
		cexpr_pair<gl_texture_compare_mode, values_texture_compare_mode>,
		cexpr_pair<gl_texture_min_filter, values_texture_min_filter>,
		cexpr_pair<gl_texture_mag_filter, values_texture_mag_filter>,
		cexpr_pair<gl_texture_wrap_s, values_wrap>,
		cexpr_pair<gl_texture_wrap_r, values_wrap>,
		cexpr_pair<gl_texture_wrap_t, values_wrap>

	>;

	// basic class for static usage
	//for static use
	template <class gl_param_name, class gl_param_v>
	struct glTexParam_base
	{
		//TODO: static_assert param_name and param_v pair
		constexpr static void TexParameter(glTargetTex target, gl_param_name&& pname, gl_param_v&& val) noexcept
		{
			glTexParameteri((int)target, pname, val);
		}
	};

	//intermediate helper
	template <class ... bases>
	struct glTexParam_impl : public bases...
	{
		using bases::TexParameter...;
	};

	template <class gl_param_name, class allowed_vals = typename pname_values::found_pair<gl_param_name>::value>
	struct glTexParamCollect;

	template <class gl_param_name, class ... allowed_vals>
	struct glTexParamCollect<gl_param_name, std::tuple<allowed_vals...>> :
		public glTexParam_impl<glTexParam_base<gl_param_name, allowed_vals>...> {};


public:

	// with handle
	template <glTargetTex target>
	class glTexParam_base_target_virtual
	{
	protected:
		const gltHandle<target>* rhandle_ = nullptr;

	public:
		glTexParam_base_target_virtual()
		{
			assert(false && "Default constructor called!");
		}
		glTexParam_base_target_virtual(const gltHandle<target> * handle)
			: rhandle_(handle)
		{}

	};

private:

	template <glTargetTex target, class gl_param_name, class gl_param_v>
	struct glTexParam_base_target : virtual protected glTexParam_base_target_virtual<target>
	{
		//TODO: static_assert param_name and param_v pair
		constexpr void TexParameter(gl_param_name&& pname, gl_param_v&& val) noexcept
		{
			// validate currently active handle
			assert(gltActiveTexture::IsCurrentTexture(*rhandle_) &&
				"Trying to modify parameters of non-active texture!");
			glTexParameteri((int)target, pname, val);
		}
	};

	template <glTargetTex target, class gl_param_name, class allowed_vals = typename pname_values::found_pair<gl_param_name>::value>
	struct glTexParamCollect_target;

	template <glTargetTex target, class gl_param_name, class ... allowed_vals>
	struct glTexParamCollect_target<target, gl_param_name, std::tuple<allowed_vals...>> :
		public glTexParam_impl<glTexParam_base_target<target, gl_param_name, allowed_vals>...> {};

public:

	template <class ... gl_param_names>
	struct glTexParametersPredefined : public glTexParam_impl<glTexParamCollect<gl_param_names> ...>
	{};

	template <glTargetTex target, class ... gl_param_names>
	struct glTexParametersPredefined_target : public glTexParam_impl<glTexParamCollect_target<target, gl_param_names> ...>
	{};

public:

	template<glTargetTex target, glTargetTex texStorage, glTargetTex ... allowed>
	struct target_validate<target, texStorage, std::integer_sequence<glTargetTex, allowed...>>
	{
		constexpr static bool value = is_any<texStorage, allowed...>;
	};


};

using gltTexParam = gltTexParam_traits::glTexParametersPredefined<gl_depth_stencil_texture_mode,
	gl_texture_compare_func,
	gl_texture_compare_mode,
	gl_texture_min_filter,
	gl_texture_mag_filter,
	gl_texture_wrap_s,
	gl_texture_wrap_r,
	gl_texture_wrap_t>;


// questinable. What do I gain?
template <glTargetTex target>
using gltTexParam_target = gltTexParam_traits::glTexParametersPredefined_target<target, gl_depth_stencil_texture_mode,
	gl_texture_compare_func,
	gl_texture_compare_mode,
	gl_texture_min_filter,
	gl_texture_mag_filter,
	gl_texture_wrap_s,
	gl_texture_wrap_r,
	gl_texture_wrap_t>;


//template <size_t textr_count>
class texture_traits
{


	
public:

	template <glTargetTex target>
	static gltHandle<target> GenTexture()
	{
		GLuint handle = 0;
		glGenTextures(1, &handle);
		assert(handle && "Failed to generate texture!");
		return gltHandle<target>(handle);
	}

	template <glTargetTex target, size_t ... indx>
	static std::array<gltHandle<target>, sizeof...(indx)> GenTextures(std::index_sequence<indx...>&&)
	{
		constexpr size_t sz = sizeof...(indx);
		GLuint handle[sz];
		glGenTextures(sz, handle);
		assert((handle[indx] && ...) && "Failed to generate texture");

		return std::array<gltHandle<target>, sz>{handle[indx] ...};
	}


	

	//temporary
	template <glTargetTex texStorage, glTargetTex target>
	static void TexImage2D(const gltHandle<target>& handle, size_t detailLevel,
		GLint internalFormat,
		size_t width,
		size_t height,
		GLint border ,
		GLenum format,
		GLenum type,
		const GLvoid * data)
	{
		static_assert(gltTexParam_traits::target_validate<target, texStorage>::value,
			"Invalid texture storage!");

		//assert that handle is active or current
		assert(gltActiveTexture::IsCurrentTexture(handle) && 
			"Loading data using non-active texture");

		glTexImage2D((GLint)texStorage, detailLevel, internalFormat, width, height, border,
			format, type, data);
	}


	template <glTargetTex target>
	static void GenerateMipMap() noexcept
	{
		glGenerateMipmap((GLenum)target);
	}

};


template <glTargetTex target, glTargetTex texStorage = target>
class gltTexture : virtual public gltTexParam_traits::glTexParam_base_target_virtual<target>, 
	public gltTexParam_target<target>
{
	using tex_base = gltTexParam_traits::glTexParam_base_target_virtual<target>;
	static_assert(gltTexParam_traits::target_validate<target, texStorage>::value,
		"Invalid texture storage!");

	gltHandle<target> handle_ = texture_traits::GenTexture<target>();

	mutable size_t texUnit_ = std::numeric_limits<size_t>::max();
public:

	gltTexture()
		: tex_base(&handle_)
	{
	}

	//to use when array of handles is created

	template <class other>
	gltTexture(other) = delete;
	template <class other>
	gltTexture(const other&) = delete;
	template <class other>
	gltTexture(other&&) = delete;

	gltTexture(gltHandle<target>&& handle)
		: tex_base(&handle_),
		handle_(std::move(handle))
	{}

	const gltHandle<target>& GetHandle() const
	{
		return handle_;
	}

	operator const gltHandle<target>&() const
	{
		return handle_;
	}

	bool IsCurrent() const
	{
		bool res = gltActiveTexture::IsCurrentTexture(handle_);
		if (!res)
			texUnit_ = std::numeric_limits<size_t>::max();
		return res;
	}

	void BindTexture(size_t texUnit = gltActiveTexture::ActiveTexUnit())
	{
		gltActiveTexture::BindTexture(handle_, texUnit);
		texUnit_ = texUnit;
	}

	//unbind changing active unit!
	void UnBind()
	{
		assert(IsCurrent() && "Unbind texture wich is not currently active!");
		texture_traits::BindTexture(gltHandle<target>(0), texUnit_);
	}

	void GenerateMipMap()
	{
		assert(IsCurrent() && "Updating parameters of non-active texture!");
		texture_traits::GenerateMipMap<target>();
	}
};
*/