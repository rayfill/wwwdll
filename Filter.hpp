#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <cassert>

namespace Filter
{
	enum EnableMatcher
	{
		both,
		head,
		tail
	};

	class Matcher
	{
		friend class FilterMatcherTest;

	private:
		std::string pattern;
		std::vector<std::string> tokens;
		bool ignoreCase;

		std::string lowerCase(const std::string& target) const
		{
			std::string result = target;
			for (std::string::iterator itor = result.begin();
				 itor != result.end(); ++itor)
			{
				if (*itor >= 'A' && *itor <= 'Z')
					*itor = *itor + ('a' - 'A');
			}

			return result;
		}

		std::vector<std::string> tokenize(const std::string& pattern_,
										  const bool ignoreCase_) const
		{
			std::string::size_type pos = 0;
			std::vector<std::string> result;

			const std::string casedPattern = 
				ignoreCase_ ? lowerCase(pattern_) : pattern_;

			do
			{
				std::string::size_type oldPos = pos;
				pos = casedPattern.find("*", pos);

				if (pos != 0)
					result.push_back(casedPattern.substr(oldPos,
														 pos - oldPos));

				if (pos < casedPattern.length())
				{
					result.push_back("*");
					++pos;
				}

			} while (pos < casedPattern.length());

			return result;
		}

		/**
		 * 
		 * @return マッチ個所終端。std::string::nposだと未マッチ
		 */
		size_t subMatch(const int patternNumber,
						size_t current,
						const std::string& target) const 
		{
			assert(static_cast<unsigned int>(patternNumber) < tokens.size());

			const std::string currentPattern = tokens[patternNumber];

			if (currentPattern == "*")
			{
				if (static_cast<unsigned int>(patternNumber + 1) <
					tokens.size())
					return subMatch(patternNumber + 1, current, target);
				
				return current;
			}

			if (current >= target.length())
				return std::string::npos;

			current = target.find(currentPattern, current);
			if (current == std::string::npos)
				return current;

			if (static_cast<unsigned int>(patternNumber + 1) == tokens.size())
				return current + currentPattern.length();

			do
			{
				size_t subcurrent = subMatch(patternNumber + 1,
								   current + currentPattern.length(),
								   target);
				if (subcurrent != std::string::npos)
					return subcurrent;

			} while ((current =
					  target.find(currentPattern,
								  current + currentPattern.length())) !=
					 std::string::npos);

			return std::string::npos;
		}

	public:
		Matcher(const std::string& pattern_, const bool ignoreCase_):
			pattern(pattern_),
			tokens(tokenize(pattern_, ignoreCase_)),
			ignoreCase(ignoreCase_)
		{}

		Matcher(const Matcher& rhs):
			pattern(rhs.pattern),
			tokens(rhs.tokens),
			ignoreCase(rhs.ignoreCase)
		{}

		Matcher& operator=(const Matcher& rhs)
		{
			pattern = rhs.pattern;
			tokens = rhs.tokens;
			ignoreCase = rhs.ignoreCase;

			return *this;
		}

		std::pair<size_t, size_t>
		match(const std::string& targetStr, const size_t offset = 0) const
		{
			assert(tokens.size() > 0);

			if (!(offset < targetStr.length()))
				return std::make_pair<size_t, size_t>(0, 0);

			size_t first = offset;
			const std::string target =
				ignoreCase ? lowerCase(targetStr) : targetStr;

			if (tokens.front() != "*")
				first = target.find(tokens.front(), offset);

			size_t last = subMatch(0, first, target);
			if (last == std::string::npos)
				return std::make_pair<size_t, size_t>(0, 0);

			return std::make_pair(first, last);
		}

		std::string toString() const
		{
			return pattern + ", ignoreCase: " + (ignoreCase ? "true" : "false");
		}
	};

	class MatchUtil
	{
	public:
		typedef std::pair<size_t, size_t> range_t;

	private:
		Matcher headMatcher;
		Matcher tailMatcher;
		EnableMatcher enables;

		static bool isValidRange(const range_t& head, const range_t& last)
		{
			return ((head.first <= head.second) &&
					(last.first <= last.second) &&
					(head.second <= last.first));
		}
		
	public:
		MatchUtil(const char* head_, const char* tail_,
				  const bool ignoreCase, EnableMatcher enables_ = both):
			headMatcher(head_, ignoreCase), tailMatcher(tail_, ignoreCase),
			enables(enables_)
		{}

		~MatchUtil()
		{}


		range_t makeMatchRange(const Matcher& head_, const Matcher& tail_,
							   const std::string& target,
							   EnableMatcher enabler) const
		{
			range_t headRange;
			range_t tailRange;
			if (enabler != tail)
			{
				headRange = headMatcher.match(target);
				if (headRange.second == 0)
					return range_t(0, 0);
			}
			else
				headRange = range_t(0, 0);

			if (enabler != head)
			{
				tailRange = tailMatcher.match(target, headRange.second);
				if (tailRange.second == 0)
					return range_t(0, 0);
			}
			else
				tailRange = range_t(target.length(), target.length());

			assert(isValidRange(headRange, tailRange));
			
			return range_t(headRange.first, tailRange.second);
		}

		std::string extract(const std::string& target) const
		{
			range_t range = makeMatchRange(headMatcher, tailMatcher,
										   target, enables);
			if (range.first == 0 && range.first == range.second)
				return target;
			return target.substr(range.first, range.second - range.first);
		}

		std::string extractAll(const std::string& target) const
		{
			size_t length;
			std::string result = target;

			do 
			{
				length = result.length();
				result = extract(result);
			} while (length > result.length());

			return result;
		}

		std::string remove(const std::string& target) const
		{
			range_t range = makeMatchRange(headMatcher, tailMatcher,
										   target, enables);
			if (range.first == 0 && range.first == range.second)
				return target;
			return target.substr(0, range.first) + target.substr(range.second);
		}

		std::string removeAll(const std::string& target) const
		{
			size_t length;
			std::string result = target;

			do 
			{
				length = result.length();
				result = remove(result);
			} while (length > result.length());

			return result;
		}

		std::string toString() const
		{
			return std::string("head: ") + headMatcher.toString() + "\n" +
				"tail: " + tailMatcher.toString() + "\n";
		}
	};

	class Executor
	{
	public:
		enum operation {
			undefined,
			extract,
			remove,
			removeAll
		};

	private:
		operation oper;
		MatchUtil util;

	public:
		Executor(operation op,
				 const char* head, const char* last,
				 const bool ignoreCase, EnableMatcher enables = both):
			oper(op), util(head, last, ignoreCase, enables)
		{}

		std::string getOperationMode() const
		{
			switch (oper)
			{
			case extract:
				return "extract";
			case remove:
				return "remove";
			case removeAll:
				return "removeAll";
			default:
				assert(false);
			}

			return "";
		}

		std::string execute(const std::string& target)
		{
			switch (oper)
			{
			case extract:
				return util.extract(target);

			case remove:
				return util.remove(target);
			
			case removeAll:
				return util.removeAll(target);

			default:
				return "";
			}
		}

		std::string toString() const
		{
			std::string op;
			switch (oper)
			{
				case extract:
					op = "extract";
					break;

				case remove:
					op = "remove";
					break;
			
				case removeAll:
					op = "remove all";
					break;

				default:
					op = "unknown";
			}

			return std::string("operation: ") + op + "\n" + util.toString();
		}
	};

	class FilterLoader
	{
	public:
		struct Rule
		{
			std::string rules;
			std::string matchURL;
			std::string head;
			std::string tail;

			std::string toString() const
			{
				return std::string("rules: ") + rules + "\n" +
					"matchURL: " + matchURL + "\n" +
					"head: " + head + "\n" +
					"tail: " + tail + "\n";
			}

			Rule():
				rules(), matchURL(), head(), tail()
			{}

			Rule(const std::string& rules_,
				 const std::string& matchURL_,
				 const std::string& head_,
				 const std::string& tail_):
				rules(rules_),
				matchURL(matchURL_),
				head(head_),
				tail(tail_)
			{}				

			Rule(const Rule& rhs):
				rules(rhs.rules),
				matchURL(rhs.matchURL),
				head(rhs.head),
				tail(rhs.tail)
			{}
				
			Rule& operator=(const Rule& rhs)
			{
				rules = rhs.rules;
				matchURL = rhs.matchURL;
				head = rhs.head;
				tail = rhs.tail;
				
				return *this;
			}
		};

	private:
		const std::string filename;
		std::vector<Rule> rules;

	public:
		FilterLoader(const char* filename_):
			filename(filename_), rules()
		{}

		~FilterLoader()
		{}

		std::vector<Rule> getRules() const
		{
			return rules;
		}

		std::string toString() const
		{
			std::string result;

			for (std::vector<Rule>::const_iterator itor = rules.begin();
				 itor != rules.end(); ++itor)
				result += itor->toString();

			return result;
		}

		void load()
		{
			std::ifstream ifs(filename.c_str());
			char lineBuffer[1024];
			while (!ifs.fail())
			{
				ifs.getline(lineBuffer, sizeof(lineBuffer));
				std::string line = lineBuffer;

				if (line.length() == 0)
					continue;

				if (line[0] == '#')
					continue;
				
				const std::string::size_type rule_term =
					line.find_first_of("	");
				if (rule_term == std::string::npos)
					continue;

				const std::string::size_type url_head =
					line.find_first_not_of("	", rule_term);
				if (url_head == std::string::npos)
					continue;

				const std::string::size_type url_term =
					line.find_first_of("	", url_head);
				if (url_term == std::string::npos)
					continue;

				const std::string::size_type head_head = 
					line.find_first_not_of("	", url_term);
				if (head_head == std::string::npos)
					continue;

				const std::string::size_type head_term =
					line.find_first_of("	", head_head);
				if (head_term == std::string::npos)
					continue;

				const std::string::size_type tail_head =
					line.find_first_not_of("	", head_term);

				if (tail_head == std::string::npos)
					continue;

				rules.push_back(
					Rule(line.substr(0, rule_term),
						 line.substr(url_head, url_term - url_head),
						 line.substr(head_head, head_term - head_head),
						 line.substr(tail_head)));
			}
		}
	};

	class FilterManager
	{
	public:
		typedef std::pair<Matcher, Executor*> pair_t;

	private:
		std::vector<pair_t> rules;

		pair_t createExecutor(const FilterLoader::Rule& rule)
		{
			const std::string opSource = rule.rules;
			const bool ignoreCase = (opSource.find("C") != std::string::npos);

			Executor::operation oper;
			if (opSource.find("D") != std::string::npos)
			{
				if (opSource.find("R") != std::string::npos)
					oper = Executor::removeAll;
				else
					oper = Executor::remove;
			}
			else if (opSource.find("S") != std::string::npos)
				oper = Executor::extract;
			else
				oper = Executor::undefined;

			EnableMatcher enabler;
			if (rule.head == "^")
				enabler = tail;
			else if (rule.tail == "$")
				enabler = head;
			else
				enabler = both;

			Executor* executor = new Executor(oper, 
											  rule.head.c_str(),
											  rule.tail.c_str(),
											  ignoreCase, enabler);

			return std::make_pair(Matcher(rule.matchURL, false), executor);
		}

	public:
		FilterManager():
			rules()
		{}

		~FilterManager()
		{
			for (std::vector<pair_t>::iterator itor = rules.begin();
				 itor != rules.end(); ++itor)
				delete itor->second;
		}
		
		void addRule(const FilterLoader::Rule& rule)
		{
			rules.push_back(createExecutor(rule));
		}

		void addRules(const std::vector<FilterLoader::Rule>& rules)
		{
			for (std::vector<FilterLoader::Rule>::const_iterator itor =
					 rules.begin(); itor != rules.end(); ++itor)
				addRule(*itor);
		}

		std::vector<Executor*> getExecutors(const std::string& url)
		{
			std::vector<Executor*> result;

			for (std::vector<pair_t>::const_iterator itor = rules.begin();
				 itor != rules.end(); ++itor)
				if (itor->first.match(url).second != 0)
					result.push_back(itor->second);

			return result;
		}
	};
};
