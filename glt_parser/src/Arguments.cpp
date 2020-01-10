#include "IParser.h"

#include <type_traits>
#include <string>
#include <functional>
#include <algorithm>

constexpr const char descr_none[] = "Unknown parameter",
descr_source_dir[] = "Path (absolute or relative) to a directory"
" that contains shader source files",

descr_outout_dir[] = "Path (absolute or relative) to a directory"
" for writing generated files",

descr_file_name[] = "Filename(s) of shader source file(s)",
descr_name_pred[] = "Name predicates that shader source files contain."
" Will be used for shader program output header",

descr_extension[] = "Shader source files' Extensions to be considered.",
descr_severity[] = "Error severity level";

class Argument : public IArgumentOld
{
    std::string tag_;
	std::string name_;
	std::string description_;

protected:

	std::string value_;
	bool valid_ = false;

public:

	Argument(const std::string& tag,
        const std::string& name,
		const std::string& descr)
		: tag_(tag),
        name_(name),
		description_(descr)
	{}

	// Inherited via IArgumentOld
	const std::string & Name() const override
	{
		return name_;
	}
    const std::string & Tag() const override
    {
        return tag_;
    }


	virtual const std::string & Value() const override
	{
		return value_;
	}

	const std::string& Description() const override
	{
		return description_;
	}

	bool IsValid() const override
	{
		return valid_;
	}

	virtual bool SetValue(std::string_view value) override
	{
		value_ = value;
		valid_ = true;
		return IsValid();
	}

};

class PathArgumentOld : public Argument
{
	std::regex regPath{ R"(((?:[a-zA-Z]:)|.*)?([\\|/][\w|\s]*)+)" };
    Severity sev_ = none;

public:
	PathArgumentOld(const std::string& tag,
        const std::string& name,
		const std::string& descr)
		: Argument(tag, name, descr)
	{}

	bool SetValue(std::string_view val) override
	{
		//std::string value{ val };
		std::match_results<std::string_view::iterator> sm;
		if (!std::regex_match(val.begin(), val.end(), sm, regPath) ||
            !fsys::exists(val))
		{
			valid_ = false;
            sev_ = error;

			return IsValid();
		}

        sev_ = none;
		valid_ = true;
		value_ = fsys::path(val).generic_string();

		return IsValid();
	}

    std::string Pattern() const override
    {
        return std::string("(" + Tag() + ")" + " \"F:/ull/or/relative/path/\"");
    }

    std::string GetError() const
    {
        if (!sev_)
            return std::string();

        return std::string("Invalid or non-existing path!");
    }
};

class WarningsArgument : public Argument
{
	std::regex regSev{ R"(([0-2])|(none)|(error)|(warnings))", std::regex::icase };

	// set flag if error occures
	Severity sev_ = error;
public:

    // only one instance would be created
	WarningsArgument()
		: Argument("-w", "Warnings Level", descr_severity)
	{
		valid_ = true;
        value_ = "1";
	}

	bool SetValue(std::string_view val) override
	{
		std::match_results<std::string_view::iterator> sm;
		if (!std::regex_match(val.cbegin(), val.cend(), sm, regSev))
		{
			valid_ = false;
			sev_ = error;
			return IsValid();
		}
		
		valid_ = true;

        auto sub = ++sm.cbegin();

        if (sub->matched)
            value_ = *sub;
        else if ((++sub)->matched)
            value_ = "0";
        else if ((++sub)->matched)
            value_ = "1";
        else
            value_ = "2";

		return IsValid();
	}

    std::string Pattern() const override
    {
        return std::string(Tag() + " [0-2|none|error|warnings]");
    }

    std::string GetError() const
    {
        if (!sev_)
            return std::string();

        return std::string("Invalid warnings level!");
    }
};



template <class ... Args>
std::vector<std::unique_ptr<IArgumentOld>> init_args(std::unique_ptr<Args>&& ... args)
{
	static_assert((std::is_base_of_v<IArgumentOld, Args> && ...), 
		"Args must inherit from IArgument");

	std::vector<std::unique_ptr<IArgumentOld>> out{ sizeof...(args) };
	size_t indx = 0;
	((out[indx++] = std::move(args)),...);

	return out;
}

const std::vector<std::unique_ptr<IArgumentOld>> IArgumentOld::defaultArgs =
	init_args(std::make_unique<PathArgumentOld>("-s", "Source Directory", descr_source_dir),
		std::make_unique<PathArgumentOld>("-d", "Output Directory", descr_outout_dir),
		//std::make_unique<Argument>("-n", descr_file_name),
		std::make_unique<Argument>("-p", "Name predicates", descr_name_pred),
		std::make_unique<Argument>("-e", "Shader source file extensions", descr_extension),
		std::make_unique<WarningsArgument>());


std::optional<rIArgumentOld> IArgumentOld::Find(const std::string& tag)
{
	auto found = std::find_if(IArgumentOld::defaultArgs.cbegin(), IArgumentOld::defaultArgs.cend(),
		[&](const std::unique_ptr<IArgumentOld>& ptr)
	{
		return ptr->Tag() == tag;
	});

	if (found == IArgumentOld::defaultArgs.cend())
		return std::nullopt;

	return rIArgumentOld(*found->get());
}

////////////////////////////////////////////////////////
//// new impl
////////////////////////////////////////////////////////

#include <map>

#include "IDataType.h"

class PathArgument : public IArgument
{
	static inline std::regex regPath{ R"(((?:[a-zA-Z]:)|.*)?([\\|/][\w|\s]*)+)" };

public:

	static bool ValidatePath(std::string_view p)
	{
		std::match_results<std::string_view::iterator> sm;

		// TODO: set corresponding error messages
		if (!std::regex_match(p.begin(), p.end(), sm, regPath) ||
			!fsys::exists(p))
			return false;

		return true;
	}

	PathArgument(const char *tag,
		const char *name,
		const char *descr)
		: IArgument(tag, name)
	{
		IArgument::SetDescription(descr);
		IArgument::SetValiator(std::bind(&ValidatePath, std::placeholders::_1));
	}

	std::any SpecificValue() const override
	{
		return fsys::path(IArgument::Value());
	}

};

class ArgShaderExtensions : public IArgument
{
	ShaderFileInfo::ShaderType type_;
	std::set<std::string_view> extensions_;

	static inline std::regex extPat{ R"((.\w+)+)" };

public:

	ArgShaderExtensions(const char *tag,
		const char *name,
		const char *descr, ShaderFileInfo::ShaderType type, bool validByDefault = false)
		: IArgument(tag, name, validByDefault),
		type_(type)
	{
		IArgument::SetDescription(descr);
	}

	virtual bool SetValue(std::string_view s) override
	{
		extensions_.clear();

		static std::match_results<std::string_view::const_iterator> sm;

		if (s.empty() || 
			!std::regex_match(s.cbegin(), s.cend(), sm, extPat))
		{
			valid_ = false;
			return IsValid();
		}

		auto iter = std::next(sm.cbegin());

		while (iter != sm.cend())
			extensions_.emplace(iter->str());

		valid_ = true;
		return IsValid();
	}

	std::pair<ShaderFileInfo::ShaderType,
		const std::set<std::string_view>&> SpecValue() const
	{
		return std::pair<ShaderFileInfo::ShaderType,
			const std::set<std::string_view>&>(type_, extensions_);
	}

	std::any SpecificValue() const
	{
		return SpecValue();
	}
};

/// Set arguments

bool argsset = ComLineParser::parser->SetArguments(
	std::make_unique<PathArgument>("-s", "Source Directory", descr_source_dir),
	std::make_unique<PathArgument>("-d", "Output Directory", descr_outout_dir),
	std::make_unique<ArgShaderExtensions>("--vert", "Vertex Shader extensions", 
		"Source file extension for vertex shaders", ShaderFileInfo::shader_vertex),
	std::make_unique<ArgShaderExtensions>("--frag", "Fragment Shader extensions",
		"Source file extension for fragment shaders", ShaderFileInfo::shader_fragment),
	std::make_unique<ArgShaderExtensions>("--geom", "Geometry Shader extensions",
		"Source file extension for geometry shaders", ShaderFileInfo::shader_geometry),
	std::make_unique<ArgShaderExtensions>("--comp", "Compute Shader extensions",
		"Source file extension for compute shaders", ShaderFileInfo::shader_compute)
);

