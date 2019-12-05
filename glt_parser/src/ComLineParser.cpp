#include "IParser.h"

#include <numeric>
#include <functional>

ComLineParser::ComLineParser(int argc, const char ** argv)
    : cl_args_(argv, std::next(argv, argc))
{
    Parce();
}

bool ComLineParser::EmptyArgs() const
{
    if (cl_args_.size() > 1)
        // TODO: other cases?
        return false;
    return true;
}

bool ComLineParser::Success() const
{
	// TODO: check warning levels ??

    constexpr auto fn = [](bool init, const std::unique_ptr<IArgument>& arg)
    {
        return init && arg->IsValid();
    };
    return std::accumulate(IArgument::defaultArgs.cbegin(),
        IArgument::defaultArgs.cend(), true, fn);
}

void ComLineParser::PrintUsage() const
{
    std::cout << "Usage: ./glt_parser";
    for (const auto& pArg : IArgument::defaultArgs)
        std::cout << " " << pArg->Pattern();
    std::cout << std::endl;
}

void ComLineParser::PrintErrors() const
{
	std::optional<rIArgument> wLevelFound = IArgument::Find("-w");
	if (!wLevelFound.has_value())
		throw std::exception("Warning level argument is missing in default args list!");

	IArgument& wLevel = *wLevelFound;    

    IArgument::Severity sev = (IArgument::Severity)std::stoi(wLevel.Value());

    if (sev > IArgument::Severity::warning)
        throw std::exception("Invalid warning level!");

    if (!sev)
        return;


    constexpr auto fn = [](const std::unique_ptr<IArgument>& arg, IArgument::Severity sev)
    {
        if (arg->IsValid() || arg->GetSeverity() > sev)
            return;

        std::cout << "Error. [" << arg->Name() << "] " << arg->GetError() << std::endl;
            // ", Description: " <<            arg->Description() << std::endl;
    };
    std::for_each(IArgument::defaultArgs.cbegin(), IArgument::defaultArgs.cend(), std::bind(fn, std::placeholders::_1, sev));
}

// TODO: need proper implementation!!!!
#pragma message("Warning: ComLineParser::Parce is not fully implemented!")
void ComLineParser::Parce()
{
    fsys::path exePath = cl_args_[0];

    auto def_order_iter = IArgument::defaultArgs.begin();
    
    // set default path as exe directory
    (*def_order_iter)->SetValue(exePath.parent_path().generic_string());

    auto clArg = ++cl_args_.begin(),
        clArg_fail = clArg;

    bool default_parse_fail = false;

    // parsing by default order
    while (clArg != cl_args_.cend() &&
        def_order_iter != IArgument::defaultArgs.cend())
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
    IArgument *found = IArgument::Find((clArg_fail++)->data());

    while (clArg_fail != cl_args_.cend())
    {
       

        if (IsArgName(*clArg_fail) &&
            !(found = IArgument::Find(std::string(*clArg_fail))))
        {
            unmatched.push_back(clArg_fail);
            unmatched
        }
    }
    */
}

bool ComLineParser::IsArgName(std::string_view val)
{
    return std::regex_match(val.begin(), val.end(), regArgName_);
}