#pragma once

#include "ISourceFile.h"

#include <set>

class IHeaderGenerator
{
	static bool compare_less(std::reference_wrapper<const Variable> w1,
		std::reference_wrapper<const Variable> w2);

	using comp_less = std::integral_constant<decltype(&compare_less),
		&compare_less>;

	constexpr static const char *commonName = "glt_Common.h";

public:
	using Container = std::set<std::reference_wrapper<const Variable>, comp_less>;

private:
	fsys::path outputFolder_;
	Container vars_;

public:
	IHeaderGenerator(fsys::path&& outputFolder);

	void CollectVariables(const ISourceFile& sf);

	void GenerateCommonHeader();

	size_t VarsCollected() const;

	Container::const_iterator cbegin() const;
	Container::const_iterator cend() const;

};
