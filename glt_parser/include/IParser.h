#pragma once

#include <iostream>
#include <regex>

#include <string_view>

#include <filesystem>

namespace fsys = std::filesystem;

// TODO: add print format. (-n "name predicates list")
struct IArgument
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

	static const std::vector<std::unique_ptr<IArgument>> defaultArgs;
    static IArgument *Find(const std::string& tag);
};



class ComLineParser
{
	std::vector<std::string_view> cl_args_;
	inline static std::regex regArgName_{ R"(-\D)" };
	
public:

	ComLineParser(int argc, const char** argv);
    
    bool EmptyArgs() const;
	bool Success() const;

    void PrintUsage() const;
	void PrintErrors() const;

private:

	void Parce();
	static bool IsArgName(std::string_view val);
	// static IArgument* Find(const std::string& name);
};
