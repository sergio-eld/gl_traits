#include "IParser.h"

#include <type_traits>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>

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


class Argument : public IArgument
{
	std::string name_;
	std::string description_;

protected:

	std::string value_;
	bool valid_ = false;

public:

	Argument(const std::string& name,
		const std::string& descr)
		: name_(name),
		description_(descr)
	{}

	// Inherited via IArgument
	const std::string & Name() const override
	{
		return name_;
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

class PathArgument : public Argument
{
	std::regex regPath{ R"(((?:[a-zA-Z]:)|.*)?([\\|/][\w|\s]*)+)" };

public:
	PathArgument(const std::string& name,
		const std::string& descr)
		: Argument(name, descr)
	{}

	bool SetValue(std::string_view val) override
	{
		//std::string value{ val };
		std::match_results<std::string_view::iterator> sm;
		if (!std::regex_match(val.begin(), val.end(), sm, regPath))
		{
			valid_ = false;
			return IsValid();
		}

		valid_ = true;
		value_ = fsys::path(val).generic_string();

		return IsValid();
	}
};

class WarningsArgument : public Argument
{
	std::regex regSev{ R"(([0-1])|(none)|(warnings))", std::regex::icase };

	// set flag if error occures
	Severity sev_ = none;
public:
	WarningsArgument(const std::string& name,
		const std::string& descr)
		: Argument(name, descr)
	{
		valid_ = true;
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

		// TODO:
		value_ = val;
		//value_ = fsys::path(val).generic_string();

		return IsValid();
	}
};



template <class ... Args>
std::vector<std::unique_ptr<IArgument>> init_args(std::unique_ptr<Args>&& ... args)
{
	static_assert((std::is_base_of_v<IArgument, Args> && ...), 
		"Args must inherit from IArgument");

	std::vector<std::unique_ptr<IArgument>> out{ sizeof...(args) };
	size_t indx = 0;
	((out[indx++] = std::move(args)),...);

	return out;
}

const std::vector<std::unique_ptr<IArgument>> IArgument::defaultArgs =
	init_args(std::make_unique<PathArgument>("-s", descr_source_dir),
		std::make_unique<PathArgument>("-d", descr_outout_dir),
		//std::make_unique<Argument>("-n", descr_file_name),
		std::make_unique<Argument>("-p", descr_name_pred),
		std::make_unique<Argument>("-e", descr_extension),
		std::make_unique<WarningsArgument>("-w", descr_severity));



ComLineParser::ComLineParser(int argc, const char ** argv)
	: cl_args_(argv, std::next(argv, argc))
{
	Parce();
}

bool ComLineParser::Success() const
{
	constexpr auto fn = [](bool init, const std::unique_ptr<IArgument>& arg)
	{
		return init && arg->IsValid();
	};
	return std::accumulate(IArgument::defaultArgs.cbegin(),
		IArgument::defaultArgs.cend(), true, fn);
}

void ComLineParser::PrintErrors() const
{
	constexpr auto fn = [&](const std::unique_ptr<IArgument>& arg)
	{
		std::cout << "Invalid Argument: " << arg->Name() << ", Description: " <<
			arg->Description() << std::endl;
	};
	std::for_each(IArgument::defaultArgs.cbegin(), IArgument::defaultArgs.cend(), fn);
}

void ComLineParser::Parce()
{
	fsys::path exePath = cl_args_[0];

	auto def_order_iter = IArgument::defaultArgs.begin();
	auto clArg = ++cl_args_.begin();


	bool default_parse_fail = false;

	// parsing by default order
	while (clArg != cl_args_.cend() &&
		def_order_iter != IArgument::defaultArgs.cend())
	{
		if ((!IsArgName(*clArg) || *clArg != (*def_order_iter)->Name()) &&
			!(*def_order_iter++)->SetValue(*clArg))
		{
			default_parse_fail = true;
			break;
		}
		++clArg;
	}

}

bool ComLineParser::IsArgName(std::string_view val)
{
	return std::regex_match(val.begin(), val.end(), regArgName_);
}

IArgument* ComLineParser::Find(const std::string & name)
{
	auto found = std::find_if(IArgument::defaultArgs.cbegin(), IArgument::defaultArgs.cend(),
		[&](const std::unique_ptr<IArgument>& ptr)
	{
		return ptr->Name() == name;
	});

	if (found == IArgument::defaultArgs.cend())
		return nullptr;

	return found->get();
}
