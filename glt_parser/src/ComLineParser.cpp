#include "IParser.h"

#include <numeric>
#include <functional>

IArgument::IArgument(const char * tag, const char * name, bool defaultValid)
	: tag_(tag),
	name_(name),
	valid_(defaultValid)
{}

IArgument::IArgument(const std::string & tag, const std::string & name, bool defaultValid)
	: IArgument(tag.c_str(), name.c_str(), defaultValid)
{}

const std::string & IArgument::Name() const
{
	return name_;
}

const std::string & IArgument::Tag() const
{
	return tag_;
}

const std::string & IArgument::Value() const
{
	return value_;
}

const std::string & IArgument::Descritpion() const
{
	return description_;
}

bool IArgument::operator<(const IArgument & other) const
{
	return tag_ < other.tag_;
}

bool IArgument::HasDescr() const
{
	return !description_.empty();
}

bool IArgument::IsValid() const
{
	return valid_;
}

void IArgument::SetDescription(const char * descr)
{
	description_ = descr;
}

void IArgument::SetValiator(std::function<bool(std::string_view)>&& validator)
{
	validator_ = std::move(validator);
}

bool IArgument::SetValue(std::string_view value)
{
	value_ = value_;
	validator_ ? valid_ = validator_(value_) : valid_ = true;
	return IsValid();
}



ComLineParser::ComLineParser(int argc, const char ** argv)
    //: cl_args_(argv, std::next(argv, argc))
{
    Parce(argc, argv);
}

// TODO: need proper implementation!!!!
#pragma message("Warning: ComLineParser::Parce is not fully implemented!")
void ComLineParser::Parce(int argc, const char** argv)
{
	std::vector<std::string_view> cl_args_{ argv, std::next(argv, argc) };


	fsys::path exePath = cl_args_[0];

	auto def_order_iter = IArgumentOld::defaultArgs.begin();

	// set default path as exe directory
	(*def_order_iter)->SetValue(exePath.parent_path().generic_string());

	auto clArg = ++cl_args_.begin(),
		clArg_fail = clArg;

	bool default_parse_fail = false;

	// parsing by default order
	while (clArg != cl_args_.cend() &&
		def_order_iter != IArgumentOld::defaultArgs.cend())
	{
		/* // invalid value is not a reason to stop parsing
		if ((!IsArgName(*clArg) || *clArg != (*def_order_iter)->Name())  &&
			!(*def_order_iter++)->SetValue(*clArg))
			*/

		clArg_fail = clArg;

		// move to next if cl_arg is a name
		if (IsArgName(*clArg) && *clArg++ != (*def_order_iter)->Tag())
		{
			default_parse_fail = true;
			break;
		}

		// set value and move to next cl_arg and arg
		(*def_order_iter++)->SetValue(*clArg++);
	}

	if (clArg == cl_args_.cend())
		return;

	/*
	std::vector<decltype(clArg_fail)> unmatched;

	// TODO: proceed matching, clArg_fail is Named at this point
	IArgumentOld *found = IArgumentOld::Find((clArg_fail++)->data());

	while (clArg_fail != cl_args_.cend())
	{


		if (IsArgName(*clArg_fail) &&
			!(found = IArgumentOld::Find(std::string(*clArg_fail))))
		{
			unmatched.push_back(clArg_fail);
			unmatched
		}
	}
	*/
}

bool ComLineParser::Success() const
{
	// TODO: check warning levels ??

	constexpr auto fn = [](bool init, const std::unique_ptr<IArgumentOld>& arg)
	{
		return init && arg->IsValid();
	};
	return std::accumulate(IArgumentOld::defaultArgs.cbegin(),
		IArgumentOld::defaultArgs.cend(), true, fn);
}

bool ComLineParser::operator()(int argc, const char** argv)
{
	Parce(argc, argv);
	return Success();
}


/*
bool ComLineParser::EmptyArgs() const
{
    if (cl_args_.size() > 1)
        // TODO: other cases?
        return false;
    return true;
}
*/


void ComLineParser::PrintUsage() const
{
    std::cout << "Usage: ./glt_parser";
    for (const auto& pArg : IArgumentOld::defaultArgs)
        std::cout << " " << pArg->Pattern();
    std::cout << std::endl;
}

void ComLineParser::PrintErrors() const
{
	std::optional<rIArgumentOld> wLevelFound = IArgumentOld::Find("-w");
	if (!wLevelFound.has_value())
		throw std::exception("Warning level argument is missing in default args list!");

	IArgumentOld& wLevel = *wLevelFound;    

    IArgumentOld::Severity sev = (IArgumentOld::Severity)std::stoi(wLevel.Value());

    if (sev > IArgumentOld::Severity::warning)
        throw std::exception("Invalid warning level!");

    if (!sev)
        return;


    constexpr auto fn = [](const std::unique_ptr<IArgumentOld>& arg, IArgumentOld::Severity sev)
    {
        if (arg->IsValid() || arg->GetSeverity() > sev)
            return;

        std::cout << "Error. [" << arg->Name() << "] " << arg->GetError() << std::endl;
            // ", Description: " <<            arg->Description() << std::endl;
    };
    std::for_each(IArgumentOld::defaultArgs.cbegin(), IArgumentOld::defaultArgs.cend(), std::bind(fn, std::placeholders::_1, sev));
}

void ComLineParser::SetDefaultOrder(const std::vector<std::string_view>& order)
{
	if (order.empty())
		return;

	for (std::string_view s : order)
	{
		if (s.empty())
			throw std::invalid_argument("ComLineParser::SetDefaultOrder: tag name is empty!");

		auto found = FindByTag(s);
		if (found == arguments_.cend())
			throw std::invalid_argument("ComLineParser::SetDefaultOrder:"
				" tag name is not registered!");
	
		defaultOrder_.emplace_back(**found);
	}
}


bool ComLineParser::IsArgName(std::string_view val)
{
    return std::regex_match(val.begin(), val.end(), regArgName_);
}