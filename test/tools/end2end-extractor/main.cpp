#include "End2EndTests.h"
#include <iostream>

int main()
{
	auto options = std::make_unique<solidity::test::CommonOptions>();
	solidity::test::CommonOptions::setSingleton(std::move(options));
	solidity::test::End2EndExtractor extractor;
	for (auto &task : extractor.testsuite())
	{
		std::cout << task.second.name() << "..." << std::endl;
		task.second.analyse();

		if (task.second.extractable())
			task.second.extract();
		else {
			std::cout << task.second.name() << " could not be extracted, because" << std::endl;
			for (auto const&reason :  task.second.reasons()) {
				std::cout << "   - " << reason << std::endl;
			}
			std::cout << std::endl;
		}
		std::cout << task.second.name() << "... done" << std::endl;
	}
	return 0;
}
