#include "End2EndExtractor.hpp"
#include "End2EndTests.h"
#include "test/Common.h"
#include <iostream>

int main()
{
	auto options = std::make_unique<solidity::test::CommonOptions>();
	solidity::test::CommonOptions::setSingleton(std::move(options));
	End2EndExtractor extractor;
	TestSuite suite = extractor.testsuite();
	int count{1};
	for (auto &a : suite)
	{
		std::cout << count << ": " << a.first << std::endl;
		a.second();
		std::cout << std::endl;
		++count;
	}
	return 0;
}
