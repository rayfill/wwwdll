struct Item
{
	bool check_flg;
	std::string url;
	std::string url_check;
	std::string url_open;
	std::string name;
	std::string last_updated;
	std::string last_visited;
	signed int crc32;
	size_t content_length;

	Item():
			check_flg(),
			url(),
			url_check(),
			url_open(),
			name(),
			last_updated(),
			last_visited(),
			crc32(),
			content_length()
	{}

	Item(const Item& rhs):
			check_flg(rhs.check_flg),
			url(rhs.url),
			url_check(rhs.url_check),
			url_open(rhs.url_open),
			name(rhs.name),
			last_updated(rhs.last_updated),
			last_visited(rhs.last_visited),
			crc32(rhs.crc32),
			content_length(rhs.content_length)
	{}

	Item& operator=(const Item& rhs)
	{
		check_flg = rhs.check_flg;
		url = rhs.url;
		url_check = rhs.url_check;
		url_open = rhs.url_open;
		name = rhs.name;
		last_updated = rhs.last_updated;
		last_visited = rhs.last_visited;
		crc32 = rhs.crc32;
		content_length = rhs.content_length;

		return *this;
	}

	static Item parse(const std::string& target)
	{
		RegexCompiler<char> compiler;
		RegexMatch<char> matcher = 
			compiler.compile("([^	]*)	([^	]*)	([^	]*)	([^	]*)	([^	]*)	"
							 "([^	]*)	([^	]*)	([^	]*)	([^	]*)	([^	]*)");

		if (!matcher.match(target))
			return Item();

		Item result;
		result.check_flg = false;
		result.url = matcher.matchedString(target, 2);
		result.url_check = matcher.matchedString(target, 3);
		result.url_open = matcher.matchedString(target, 4);
		result.name = matcher.matchedString(target, 5);
		result.last_updated = matcher.matchedString(target, 6);
		result.last_visited = matcher.matchedString(target, 7);
		result.crc32 =
			matcher.matchedString(target, 8) == "" ?
			0 : lexicalCast<int>(matcher.matchedString(target, 8));
		result.content_length =
			matcher.matchedString(target, 9) == "" ?
			0 : lexicalCast<size_t>(matcher.matchedString(target, 9));
		
		return result;
	}

	std::string toString() const
	{
		const std::string separetor = "\t";

		return std::string("") + separetor +
			url + separetor +
			url_check + separetor +
			url_open + separetor +
			name + separetor +
			last_updated + separetor +
			last_visited + separetor +
			stringCast<int>(crc32) + separetor +
			stringCast<size_t>(content_length) + separetor;
	}


};
