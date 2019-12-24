
#include <iostream>

#include <string_view>
#include <optional>

#include <string>
#include <vector>
#include <map>

constexpr const char tagSource[] = "-S",
tagOutput[] = "-B",
tagConfig[] = "-A",
tagThreads[] = "-T",
tagHelp1[] = "-H",
tagHelp2[] = "-Help";

using TagStr = std::string_view;

// it will sort tags in alphabetic order
std::map<TagStr, std::string> clArgs
{
		{tagSource, std::string()},
		{tagOutput, std::string()},
		{tagConfig, std::string("x32")},
		{tagThreads, std::string("1")}
};

using RefString = std::reference_wrapper<std::string>;

std::optional<RefString> GetValue(const TagStr& tag);

bool IsHelp(std::string_view tag);
void PrintHelp();

bool ArgValidConfig(const std::string& val);
bool ArgValidThreads(const std::string& val);

int main(int argc, const char *argv[])
{
	std::vector<std::string_view> commands{ argv, std::next(argv, argc) };

	using CIter = std::vector<std::string_view>::const_iterator;
	CIter iCommand = std::next(commands.cbegin()); // 0 arg is exe path

	if (iCommand == commands.cend() ||
		IsHelp(iCommand->data()))
	{
		PrintHelp();
		return -1;
	}


	// break loop if starting conditions have not been met for new iteration
	auto IsArgName = [](std::string_view str, bool * loop_broken = nullptr)
	{
		bool res = str[0] == '-';;
		if (loop_broken)
			*loop_broken = !res;
		return res;
	};

	bool loop_broken = false;

	// starting conditions: first command is an arg name
	while (IsArgName(*iCommand, &loop_broken) &&
		++iCommand != commands.cend())
	{
		static std::string_view tag;
		static std::optional<RefString> refargVal = std::nullopt;

		// next command is also an arg name. Leaving current arg's value empty/default
		if (IsArgName(*iCommand))
			continue;

		tag = *std::prev(iCommand);
		refargVal = GetValue(tag);

		if (!refargVal.has_value())
		{
			std::cerr << "Error: Unrecognized tag \"" << tag << "\"" << std::endl;
			PrintHelp();
			return -1;
		}

		std::string& argVal = *refargVal;
		argVal = *iCommand;
		if (++iCommand == commands.cend())
			break;
	}

	if (loop_broken)
	{
		std::cerr << "Error: expected argument name. Recieved: "
			<< *iCommand << std::endl;
		PrintHelp();
		return -1;
	}
	
	bool validInput = true;

	for (const auto& t : clArgs)
	{
		static std::string msg{ "Error: invalid value for argument " },
			msgVal{ "Expected value: " };

		if (t.second.empty())
		{
			std::cerr << "Error: argument " << t.first << " is empty!" << std::endl;
			validInput = false;
			continue;
		}

		if (t.first == tagConfig && 
			!ArgValidConfig(t.second))
		{
			std::cerr << msg << t.first << ". Recieved: " << t.second <<
				std::endl << msgVal << "[x32|x64]" << std::endl;
			validInput = false;
			continue;
		}

		if (t.first == tagThreads &&
			!ArgValidThreads(t.second))
		{
			std::cerr << msg << t.first << ". Recieved: " << t.second <<
				std::endl << msgVal << "[1..10]" << std::endl;
			validInput = false;
			continue;
		}

	}

	if (!validInput)
		return -1;

	for (const auto& a : clArgs)
		std::cout << a.first << " " << a.second << std::endl;

	return 0;
}

std::optional<RefString> GetValue(const TagStr & tag)
{
	using Iter = std::map<TagStr, std::string>::iterator;
	if (tag.empty())
		return std::nullopt;

	Iter found = clArgs.find(tag);
	if (found == clArgs.cend())
		return std::nullopt;

	return std::reference_wrapper(found->second);
}

bool IsHelp(std::string_view tag)
{
	static std::string_view help1{ tagHelp1 },
		help2{ tagHelp2 };

	return tag == help1 ||
		tag == help2;
}

void PrintHelp()
{
	std::cout << "Usage: ./app.exe " << tagSource << " [source path] " <<
		tagOutput << " [output path] " << tagConfig << " [configuration = x32] " <<
		tagThreads << " [threads = 1]\n" <<
		"./app.exe -h\nor\n./app.exe --help\nto print this message." << std::endl;
}


bool ArgValidConfig(const std::string & val)
{
	static std::string x32{ "x32" },
		x64{ "x64" };
	return val == x32 ||
		val == x64;
}

bool ArgValidThreads(const std::string & val)
{
	static std::string msg{ "Invalid value for \"threads\" parameter: " };

	try
	{
		int threads = std::stoi(val);
		return threads >= 1 && threads < 11;
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << msg << e.what() << std::endl;
		return false;
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << msg << e.what() << std::endl;
		return false;
	}
	catch (...)
	{
		std::cerr << msg << "unknown error." << std::endl;
		return false;
	}
}