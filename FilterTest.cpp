#include <iostream>
#include "Filter.hpp"
#include <fstream>
#include <iterator>
#include <string>


using namespace Filter;

void output(const std::vector<std::string>& vec)
{
	for (std::vector<std::string>::const_iterator itor = vec.begin();
		 itor != vec.end(); ++itor)
		std::cout << *itor << std::endl;
}

namespace Filter
{
	class FilterMatcherTest
	{
	public:
		void main()
		{
			const std::string pattern = "http://*.cool.ne.jp/*";
			Matcher matcher(pattern.c_str(), true);
			
			std::cout << "tokenize of: " << pattern <<
				std::endl;
			output(matcher.tokens);

			std::string target = "http://miumiu-s.cool.ne.jp/";
			std::pair<size_t, size_t> pair =
				matcher.match(target);

			std::cout << "matched is " <<
				target.substr(pair.first, pair.second - pair.first) <<
				std::endl;
		}
	};
};

int main()
{
 	FilterMatcherTest().main();
	
// 	Filter::MatchUtil util("<body", "</body>", true);
	std::ifstream ifs("savedfile.html");
	std::istreambuf_iterator<char> itor(ifs), end;
	std::string file(itor, end);

// 	std::cout << util.remove(file) << std::endl;

	//	std::cout << "-----------" << std::endl;
	//	std::cout << util.extract(file) << std::endl;

// 	std::cout << "-----------" << std::endl;
// 	Filter::MatchUtil util2("<a*", "<br>", true);
// 	std::cout << util2.remove(file) << std::endl;

// 	Filter::Executor executer(Filter::Executor::removeAll,
// 							  "<a*", "<br>", true);
// 	std::cout << executer.execute(file) << std::endl;

	Filter::FilterLoader loader("filter.txt");
	loader.load();
	std::cout << loader.toString() << std::endl;
	std::vector<FilterLoader::Rule> rules = loader.getRules();
	FilterManager manager;
	manager.addRules(rules);

	std::vector<Executor*> executors =
		manager.getExecutors("http://www.geocities.jp/hou_para/top/mainpage.html");

	std::cout << "matched executors: " << executors.size() << std::endl;

	for (std::vector<Executor*>::iterator itor = executors.begin();
		 itor != executors.end(); ++itor)
	{
		std::cout << "file length of " << file.length() << std::endl;
		std::cout << (*itor)->toString() << std::endl;
		file = (*itor)->execute(file);
	}

	std::ofstream procfile("procfile.html", std::ios::binary);
	procfile << file;
	procfile.close();

	return 0;
}
