
#include "gl_traits.hpp"

#include <iostream>

template <class T>
struct bass
{
	template <class = std::enable_if_t<std::is_same_v<int, T>>>
	void Print(T a)
	{
		std::cout << a << std::endl;
	}

	template <class = std::enable_if_t<std::is_same_v<char, T>>>
	void Print(T a, int times)
	{
		while (times--)
			std::cout << a << std::endl;
	}


};

template <class T>
struct inherit : bass<T>
{
	using bass<T>::Print;
};

int main()
{

	inherit<int>().Print(69);
	inherit<char>().Print('a', 32);

	return 0;
}