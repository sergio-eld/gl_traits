
#include <iostream>

#include <map>
#include <string_view>
#include <string>
#include <optional>

#include <vector>

constexpr const char tagSource[] = "-S",
tagOutput[] = "-B",
tagConfig[] = "-A",
tagThreads[] = "-T",
tagHelp1[] = "-H",
tagHelp2[] = "-Help";

using TagStr = std::string;

// it will sort tags in alphabetic order
std::map<TagStr, std::string> clArgs
{
		{tagSource, std::string()},
		{tagOutput, std::string()},
		{tagConfig, std::string("x32")},
		{tagThreads, std::string("1")}
};

using RefString = std::reference_wrapper<std::string>;

std::optional<RefString> GetValue(const char *tag)
{
	using Iter = std::map<TagStr, std::string>::iterator;
	if (!tag) // is nullptr
		return std::nullopt;

	Iter found = clArgs.find(tag);
	if (found == clArgs.cend())
		return std::nullopt;

	return std::reference_wrapper(found->second);

}

bool ArgValidConfig(const std::string& val)
{
	static std::string x32{ "x32" },
		x64{ "x64" };
	return val == x32 ||
		val == x64;
}

bool ArgValidThreads(const std::string& val)
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

void PrintHelp()
{
	std::cout << "Usage: ./app.exe " << tagSource << " [source path] " <<
		tagOutput << " [output path] " << tagConfig << " [configuration = x32] " <<
		tagThreads << " [threads = 1]\n" <<
		"./app.exe -h\nor\n./app.exe --help\nto print this message." << std::endl;
}

int main(int argc, const char *argv[])
{
	std::vector<std::string_view> commands{ argv, std::next(argv, argc) };

	if (commands.size() <= 1)
	{
		PrintHelp();
		return -1;
	}

	using CIter = std::vector<std::string_view>::const_iterator;


	CIter iCommand = std::next(commands.cbegin()); // 0 arg is exe path

	while (iCommand != commands.cend())
	{
		// is prev/current argument a Name
		static bool prevName = false,
			curName = false;

		curName = (*iCommand)[0] == '-';

		if (prevName && curName)
		{
			// empty value for current argument
			++iCommand;
			continue;
		}

		if (!prevName && !curName)
		{
			std::cerr << "Argument name expected. Recieved:" <<
				*iCommand << std::endl;
			PrintHelp();
			return -1;
		}
		
		if (curName)
		{
			static std::string help1{ tagHelp1 },
				help2{ tagHelp1 };

			std::string tag{ *iCommand };

			bool isHelp = tag == help1 ||
				tag == help2;

			if (isHelp)
			{
				PrintHelp();
				return -1;
			}

			// value for the argument is expected in the next elem
			prevName = true;
			++iCommand;
			continue;
		}

		// get tag for the current argument
		CIter prev = std::prev(iCommand);
		std::optional<RefString> refVal = GetValue(prev->data());

		if (!refVal.has_value())
		{
			std::cerr << "Error: Unrecognized tag \"" << *iCommand << "\"" << std::endl;
			PrintHelp();
			return -1;
		}

		std::string& val = refVal->get();
		val = *iCommand;
		++iCommand;
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