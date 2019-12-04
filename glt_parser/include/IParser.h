#pragma once

#include <iostream>
#include <regex>

#include <string_view>

#include <filesystem>

namespace fsys = std::filesystem;

struct IArgument
{
	enum Severity
	{
		none,
		warning,
		error
	};

	virtual const std::string& Name() const = 0;

	virtual const std::string& Value() const = 0;
	virtual const std::string& Description() const = 0;
	virtual bool IsValid() const = 0;
	virtual Severity GetSeverity() const
	{
		return none;
	}

	virtual bool SetValue(std::string_view) = 0;

	static const std::vector<std::unique_ptr<IArgument>> defaultArgs;

};



class ComLineParser
{
	std::vector<std::string_view> cl_args_;
	inline static std::regex regArgName_{ R"(-\D)" };
	
public:

	ComLineParser(int argc, const char** argv);
	
	bool Success() const;
	void PrintErrors() const;

private:

	void Parce();
	static bool IsArgName(std::string_view val);
	static IArgument* Find(const std::string& name);
};
