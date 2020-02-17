/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libsolutil/CommonIO.h>
#include <libsolutil/AnsiColorized.h>

#include <memory>
#include <test/Common.h>
#include <test/tools/IsolTestOptions.h>
#include <test/libsolidity/AnalysisFramework.h>
#include <test/InteractiveTests.h>
#include <test/EVMHost.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>
#include <regex>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::util::formatting;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using TestCreator = TestCase::TestCaseCreator;
using TestOptions = solidity::test::IsolTestOptions;

struct TestStats; // forward declaration

class TestFilter
{
public:
	explicit TestFilter(string const& _filter): m_filter(_filter)
	{
		string filter{m_filter};

		boost::replace_all(filter, "/", "\\/");
		boost::replace_all(filter, "*", ".*");

		m_filterExpression = regex{"(" + filter + "(\\.sol|\\.yul))"};
	}

	bool matches(string const& _name) const
	{
		return regex_match(_name, m_filterExpression);
	}

private:
	string m_filter;
	regex m_filterExpression;
};

class TestTool
{
public:
	TestTool(
		TestCreator _testCaseCreator,
		TestOptions const& _options,
		fs::path const& _path,
		string const& _name
	):
		m_testCaseCreator(_testCaseCreator),
		m_options(_options),
		m_filter(TestFilter{_options.testFilter}),
		m_path(_path),
		m_name(_name)
	{}

	enum class Result
	{
		Success,
		Failure,
		Exception,
		Skipped
	};

	Result process();

	static TestStats processPath(
		TestCreator _testCaseCreator,
		TestOptions const& _options,
		fs::path const& _basepath,
		fs::path const& _path
	);

	static string editor;
private:
	enum class Request
	{
		Skip,
		Rerun,
		Quit
	};

	Request handleResponse(bool _exception);

	TestCreator m_testCaseCreator;
	TestOptions const& m_options;
	TestFilter m_filter;
	fs::path const m_path;
	string const m_name;

	unique_ptr<TestCase> m_test;

	static bool m_exitRequested;
};

struct TestStats
{
	std::map<TestTool::Result, std::set<fs::path>> results;
	size_t testCount{0};

	std::set<fs::path> failed()
	{
		std::set<fs::path> result;
		if (results.find(TestTool::Result::Failure) != results.end())
		{
			auto failures{results.find(TestTool::Result::Failure)->second};
			result.insert(failures.begin(), failures.end());
		}
		if (results.find(TestTool::Result::Exception) != results.end())
		{
			auto exceptions{results.find(TestTool::Result::Exception)->second};
			result.insert(exceptions.begin(), exceptions.end());
		}
		if (results.find(TestTool::Result::Skipped) != results.end())
		{
			auto skipped{results.find(TestTool::Result::Skipped)->second};
			result.insert(skipped.begin(), skipped.end());
		}
		return result;
	}

	std::set<fs::path> successful()
	{
		std::set<fs::path> result;
		if (results.find(TestTool::Result::Success) != results.end())
		{
			auto successful{results.find(TestTool::Result::Success)->second};
			result.insert(successful.begin(), successful.end());
		}
		return result;
	}

	[[nodiscard]] size_t successCount() const {
		size_t successCount{0};
		if (results.find(TestTool::Result::Success) != results.end())
			successCount = results.find(TestTool::Result::Success)->second.size();
		return successCount;
	}

	[[nodiscard]] size_t skippedCount() const {
		size_t skippedCount{0};
		if (results.find(TestTool::Result::Skipped) != results.end())
			skippedCount = results.find(TestTool::Result::Skipped)->second.size();
		return skippedCount;
	}

	explicit operator bool() const {
		return successCount() + skippedCount() == testCount;
	}

	TestStats& operator+=(TestStats const& _other) noexcept
	{
		for(auto& it : _other.results)
			results[it.first].insert(it.second.begin(), it.second.end());
		testCount += _other.testCount;
		return *this;
	}
};

string TestTool::editor;
bool TestTool::m_exitRequested = false;

TestTool::Result TestTool::process()
{
	bool formatted{!m_options.noColor};
	std::stringstream outputMessages;

	try
	{
		if (m_filter.matches(m_name))
		{
			(AnsiColorized(cout, formatted, {BOLD}) << m_name << ": ").flush();

			m_test = m_testCaseCreator(TestCase::Config{m_path.string(), m_options.evmVersion()});
			if (m_test->validateSettings(m_options.evmVersion()))
				switch (TestCase::TestResult result = m_test->run(outputMessages, "  ", formatted))
				{
					case TestCase::TestResult::Success:
						AnsiColorized(cout, formatted, {BOLD, GREEN}) << "OK" << endl;
						return Result::Success;
					default:
						AnsiColorized(cout, formatted, {BOLD, RED}) << "FAIL" << endl;

						AnsiColorized(cout, formatted, {BOLD, CYAN}) << "  Contract:" << endl;
						m_test->printSource(cout, "    ", formatted);
						m_test->printUpdatedSettings(cout, "    ", formatted);

						cout << endl << outputMessages.str() << endl;
						return result == TestCase::TestResult::FatalError ? Result::Exception : Result::Failure;
				}
			else
			{
				AnsiColorized(cout, formatted, {BOLD, YELLOW}) << "NOT RUN" << endl;
				return Result::Skipped;
			}
		}
		else
			return Result::Skipped;
	}
	catch (boost::exception const& _e)
	{
		AnsiColorized(cout, formatted, {BOLD, RED}) <<
			"Exception during test: " << boost::diagnostic_information(_e) << endl;
		return Result::Exception;
	}
	catch (std::exception const& _e)
	{
		AnsiColorized(cout, formatted, {BOLD, RED}) <<
			"Exception during test" <<
			(_e.what() ? ": " + string(_e.what()) : ".") <<
			endl;
		return Result::Exception;
	}
	catch (...)
	{
		AnsiColorized(cout, formatted, {BOLD, RED}) <<
			"Unknown exception during test." << endl;
		return Result::Exception;
	}
}

TestTool::Request TestTool::handleResponse(bool _exception)
{
	if (_exception)
		cout << "(e)dit/(s)kip/(q)uit? ";
	else
		cout << "(e)dit/(u)pdate expectations/(s)kip/(q)uit? ";
	cout.flush();

	while (true)
	{
		switch(readStandardInputChar())
		{
		case 's':
			cout << endl;
			return Request::Skip;
		case 'u':
			if (_exception)
				break;
			else
			{
				cout << endl;
				ofstream file(m_path.string(), ios::trunc);
				m_test->printSource(file);
				m_test->printUpdatedSettings(file);
				file << "// ----" << endl;
				m_test->printUpdatedExpectations(file, "// ");
				return Request::Rerun;
			}
		case 'e':
			cout << endl << endl;
			if (system((TestTool::editor + " \"" + m_path.string() + "\"").c_str()))
				cerr << "Error running editor command." << endl << endl;
			return Request::Rerun;
		case 'q':
			cout << endl;
			return Request::Quit;
		default:
			break;
		}
	}
}

TestStats TestTool::processPath(
	TestCreator _testCaseCreator,
	TestOptions const& _options,
	fs::path const& _basepath,
	fs::path const& _path
)
{
	std::queue<fs::path> paths;
	TestStats stats;
	paths.push(_path);

	while (!paths.empty())
	{
		auto currentPath = paths.front();

		fs::path fullpath = _basepath / currentPath;
		if (fs::is_directory(fullpath))
		{
			paths.pop();
			for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
				fs::directory_iterator(fullpath),
				fs::directory_iterator()
			))
				if (fs::is_directory(entry.path()) || TestCase::isTestFilename(entry.path().filename()))
					paths.push(currentPath / entry.path().filename());
		}
		else if (m_exitRequested)
		{
			++stats.testCount;
			paths.pop();
		}
		else
		{
			++stats.testCount;
			TestTool testTool(
				_testCaseCreator,
				_options,
				fullpath,
				currentPath.generic_path().string()
			);
			auto result = testTool.process();

			switch(result)
			{
			case Result::Failure:
				stats.results[Result::Failure].insert(paths.front());
				paths.pop();
				break;
			case Result::Exception:
				stats.results[Result::Exception].insert(paths.front());
				switch(testTool.handleResponse(result == Result::Exception))
				{
				case Request::Quit:
					paths.pop();
					m_exitRequested = true;
					break;
				case Request::Rerun:
					cout << "Re-running test case..." << endl;
					--stats.testCount;
					break;
				case Request::Skip:
					stats.results[Result::Skipped].insert(paths.front());
					paths.pop();
					break;
				}
				break;
			case Result::Success:
				stats.results[Result::Success].insert(paths.front());
				paths.pop();
				break;
			case Result::Skipped:
				stats.results[Result::Skipped].insert(paths.front());
				paths.pop();
				break;
			}
		}
	}

	return stats;

}

namespace
{

void setupTerminal()
{
#if defined(_WIN32) && defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
	// Set output mode to handle virtual terminal (ANSI escape sequences)
	// ignore any error, as this is just a "nice-to-have"
	// only windows needs to be taken care of, as other platforms (Linux/OSX) support them natively.
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return;

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
		return;
#endif
}

std::optional<TestStats> runTestSuite(
	TestCreator _testCaseCreator,
	TestOptions const& _options,
	fs::path const& _basePath,
	fs::path const& _subdirectory,
	string const& _name
)
{
	fs::path testPath{_basePath / _subdirectory};
	bool formatted{!_options.noColor};

	if (!fs::exists(testPath) || !fs::is_directory(testPath))
	{
		cerr << _name << " tests not found. Use the --testpath argument." << endl;
		return std::nullopt;
	}

	TestStats stats = TestTool::processPath(
		_testCaseCreator,
		_options,
		_basePath,
		_subdirectory
	);

	size_t skippedCount{stats.results[TestTool::Result::Skipped].size()};
	size_t successCount{stats.results[TestTool::Result::Success].size()};

	if (stats.results[TestTool::Result::Skipped].size() != stats.testCount)
	{
		cout << endl << _name << " Test Summary: ";
		AnsiColorized(cout, formatted, {BOLD, stats ? GREEN : RED}) <<
			successCount <<
			"/" <<
			stats.testCount;
		cout << " tests successful";
		if (skippedCount > 0)
		{
			cout << " (";
			AnsiColorized(cout, formatted, {BOLD, YELLOW}) << skippedCount;
			cout<< " tests skipped)";
		}
		cout << "." << endl << endl;
	}
	return stats;
}

}

int main(int argc, char const *argv[])
{
	setupTerminal();

	{
		auto options = std::make_unique<solidity::test::IsolTestOptions>(&TestTool::editor);

		try
		{
			if (!options->parse(argc, argv))
				return -1;

			options->validate();
			solidity::test::CommonOptions::setSingleton(std::move(options));
		}
		catch (std::exception const& _exception)
		{
			cerr << _exception.what() << endl;
			return 1;
		}
	}

	auto& options = dynamic_cast<solidity::test::IsolTestOptions const&>(solidity::test::CommonOptions::get());

	bool disableSemantics = !solidity::test::EVMHost::getVM(options.evmonePath.string());
	if (disableSemantics)
	{
		cout << "Unable to find " << solidity::test::evmoneFilename << ". Please provide the path using --evmonepath <path>." << endl;
		cout << "You can download it at" << endl;
		cout << solidity::test::evmoneDownloadLink << endl;
		cout << endl << "--- SKIPPING ALL SEMANTICS TESTS ---" << endl << endl;
	}

	TestStats global_stats;
	cout << "Running tests..." << endl << endl;

	// Actually run the tests.
	// Interactive tests are added in InteractiveTests.h
	for (auto const& ts: g_interactiveTestsuites)
	{
		if (ts.needsVM && disableSemantics)
			continue;

		if (ts.smt && options.disableSMT)
			continue;

		auto stats = runTestSuite(
			ts.testCaseCreator,
			options,
			options.testPath / ts.path,
			ts.subpath,
			ts.title
		);
		if (stats)
			global_stats += *stats;
		else
			return 1;
	}

	cout << endl << "Summary: ";
	AnsiColorized(cout, !options.noColor, {BOLD, global_stats ? GREEN : RED}) <<
		 global_stats.successCount() << "/" << global_stats.testCount;
	cout << " tests successful";
	if (global_stats.skippedCount() > 0)
	{
		cout << " (";
		AnsiColorized(cout, !options.noColor, {BOLD, YELLOW}) << global_stats.skippedCount();
		cout << " tests skipped)";
	}
	cout << "." << endl;

	if (disableSemantics)
		cout << "\nNOTE: Skipped semantics tests because " << solidity::test::evmoneFilename << " could not be found.\n" << endl;

	if (options.showFailed)
		for (auto const& path : global_stats.failed())
			cerr << path.string() << std::endl;

	if (options.showSuccess)
		for (auto const& path : global_stats.successful())
			cerr << path.string() << std::endl;

	return global_stats ? 0 : 1;
}
