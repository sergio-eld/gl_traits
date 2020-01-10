#pragma once

#include <iostream>
#include <regex>

#include <optional>

#include <set>

#include <string_view>
#include <any>
#include <functional>

#include <filesystem>

namespace fsys = std::filesystem;

class IArgument
{
	std::string tag_,
		name_,
		value_,
		description_;

	std::function<bool(std::string_view)> validator_;

public:
	IArgument(const char* tag, const char* name, bool defaultValid = false);
	IArgument(const std::string& tag, const std::string& value, bool defaultValid = false);

	const std::string& Name() const;
	const std::string& Tag() const;
	const std::string& Value() const;
	const std::string& Descritpion() const;
	//const std::string& Value() const;


	bool operator<(const IArgument& other) const;

	bool HasDescr() const;
	bool IsValid() const;

	operator bool() const
	{
		return IsValid();
	}

	virtual std::any SpecificValue() const
	{
		return std::any();
	}

	template <typename T>
	operator T() const
	{
		try
		{
			return std::any_cast<T>(SpecificValue());
		}
		catch (const std::bad_any_cast&  e)
		{
			throw e;
		}
	}

	void SetDescription(const char *descr);
	void SetValiator(std::function<bool(std::string_view)>&& validator);

	virtual bool SetValue(std::string_view value);

protected:

	bool valid_ = false;

};

using RefIArgument = std::reference_wrapper<IArgument>;

struct IArgumentOld;

using rIArgumentOld = std::reference_wrapper<IArgumentOld>;

// TODO: add print format. (-n "name predicates list")
struct IArgumentOld
{
	enum Severity : unsigned char
	{
		none,
		error,
        warning
	};

	virtual const std::string& Name() const = 0;
    virtual const std::string& Tag() const = 0;
    virtual std::string Pattern() const
    {
        return std::string(Tag() + " [\"val\"]");
    }

	virtual const std::string& Value() const = 0;
	virtual const std::string& Description() const = 0;
	virtual bool IsValid() const = 0;
	virtual Severity GetSeverity() const
	{
		return none;
	}
    virtual std::string GetError() const
    {
        switch (GetSeverity())
        {
        case error:
            return std::string("Invalid argument");
        default:
            return std::string();
        }

    }

	virtual bool SetValue(std::string_view) = 0;

	static const std::vector<std::unique_ptr<IArgumentOld>> defaultArgs;
    static std::optional<rIArgumentOld> Find(const std::string& tag);
};



class ComLineParser
{

	static bool CompLess(const std::unique_ptr<IArgument>& a1,
		const std::unique_ptr<IArgument>& a2)
	{
		return *a1 < *a2;
	}

	using comparator = std::integral_constant<decltype(&CompLess),
		&CompLess>;

	std::set<std::unique_ptr<IArgument>, comparator> arguments_;

	//std::vector<std::string_view> cl_args_;
	inline static std::regex regArgName_{ R"(-\D)" };

	std::vector<RefIArgument> defaultOrder_;
	
public:

	ComLineParser() = default;
	ComLineParser(int argc, const char** argv);
    
	void Parce(int argc, const char** argv);

	bool Success() const;
	bool operator()(int argc, const char** argv);
	

    //bool EmptyArgs() const;

    void PrintUsage() const;
	void PrintErrors() const;

	void SetDefaultOrder(const std::vector<std::string_view>& order);

private:

	decltype(arguments_.begin()) FindByTag(std::string_view s)
	{
		return std::find_if(arguments_.begin(), arguments_.end(), 
			[&](const std::unique_ptr<IArgument>& arg)
		{
			return s == arg->Tag();
		});
	}

	decltype(arguments_.cbegin()) FindByTag(std::string_view s) const
	{
		return std::find_if(arguments_.cbegin(), arguments_.cend(),
			[&](const std::unique_ptr<IArgument>& arg)
		{
			return s == arg->Tag();
		});
	}

	static bool IsArgName(std::string_view val);
	// static IArgumentOld* Find(const std::string& name);

	static std::unique_ptr<ComLineParser> TryInit()
	{
		return parser ? std::unique_ptr<ComLineParser>(parser.release()) 
			: std::make_unique<ComLineParser>();
	}

public:

	static inline std::unique_ptr<ComLineParser> parser{ TryInit() };

	template <typename ... Args>
	bool SetArguments(std::unique_ptr<Args>&& ... args)
	{
		static_assert(std::conjunction_v<std::is_base_of<IArgument, Args>...>,
			"Argument types must inherit from IArgument");

		// TODO: check if all arguments have been emplaced
		return (arguments_.emplace(std::move(args)).second && ...);
	}

};
