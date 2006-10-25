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
			const std::string pattern = "HTTP://*.INFOSEEK.co.jp/*a*";
			Matcher matcher(pattern.c_str(), true);
			
			std::cout << "tokenize of: " << pattern <<
				std::endl;
			output(matcher.tokens);

			std::string target =
				"aaaaahttp://www.hp.infoSEEK.co.jp/hogehoge.infoSEEK.co.jp/a";
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
// 	FilterMatcherTest().main();
	
	Filter::MatchUtil util("<body", "</body>", true);
	std::ifstream ifs("teiten.html");
	std::istreambuf_iterator<char> itor(ifs), end;
	std::string file(itor, end);

	std::cout << util.remove(file) << std::endl;

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
		manager.getExecutors("http://www.geocities.jp/hazimes316/top.htm");

	std::cout << "matched executors: " << executors.size() << std::endl;

	return 0;
}
