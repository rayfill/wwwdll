#include <wwwdll.h>
#include <filter.hpp>
#include <net/HTTPClient.hpp>
#include <util/CRC.hpp>
#include <Thread/ThreadPool.hpp>
#include <text/regex/RegexCompile.hpp>
#include <cassert>
#include <fstream>

typedef ThreadPool<>::RerunnableThread thread_t;
class HTTPContext : public Runnable
{
private:
	std::string url;
	std::string cookie;
	std::string userAgent;
	const long timeout;
	HWND hWnd;
	unsigned int message;
	HTTPResult<> result;
	thread_t* threadContext;

	bool getContents()
	{
		try 
		{
			HTTPClient client;
			client.setKeepAliveTime(300);
			client.setUserAgent(userAgent == "" ? 
								"Mozilla/4.0 (compatible; MSIE 6.0; "
								"Windows NT 5.1; SV1; .NET CLR 1.0.3705; "
								".NET CLR 1.1.4322)" :
								userAgent.c_str());
			client.setAcceptEncoding("");
			client.addAcceptLanguage("ja");
			client.addAcceptLanguage("en");
			if (cookie != "")
				client.addCookie(cookie.c_str());
			client.setTimeout(timeout);

			result = client.getResource(url.c_str());
		}
		catch (std::exception& e)
		{
			return false;
		}
		return true;
	}

public:
	std::string getURL() const
	{
		return this->url;
	}

	void setHWnd(HWND hWnd_)
	{
		this->hWnd = hWnd_;
	}

	void setMessage(const unsigned int message_)
	{
		this->message = message_;
	}

	std::string getLastModified() const
	{
		return result.getResponseHeaders().get("Last-Modified");
	}

	long getCRC32() const
	{
		CRC32 digester;
		std::vector<unsigned char> resource = result.getResource();

		digester.setSource(resource.begin(), resource.end());
		return static_cast<long>(digester.getDigest());
	}

	std::string getContentsString() const
	{
		std::vector<unsigned char> resource = result.getResource();
		return std::string(resource.begin(), resource.end());
	}

	long getResponseCode() const
	{
		return result.getStatusCode();
	}

	HTTPContext(const char* url_,
					 const char* cookie_,
					 const char* userAgent_,
					 const long timeout_,
					 HWND hWnd_ = NULL,
					 unsigned int message_ = 0):
		url(url_),
		cookie(cookie_ == NULL ? "" : cookie_),
		userAgent(userAgent_ == NULL ? "" : userAgent_),
		timeout(timeout_),
		hWnd(hWnd_),
		message(message_),
		result(),
		threadContext()
	{}

	virtual ~HTTPContext() throw ()
	{}

	virtual void prepare() throw()
	{}

	virtual void dispose() throw()
	{}

	thread_t* getThreadContext() const
	{
		return threadContext;
	}

	void setThreadContext(thread_t* newContext)
	{
		threadContext = newContext;
	}

	virtual unsigned run() throw(ThreadException)
	{
		bool result = this->getContents();

		if (hWnd != NULL)
			PostMessage(hWnd, message,
						static_cast<WPARAM>(result),
						reinterpret_cast<LPARAM>(this->getThreadContext()));

		return result;
	}
};

__stdcall void* ThreadCreate()
{
	return new thread_t;
}

__stdcall void ThreadStart(void* context,
						   void* httpContext,
						   HWND hWnd,
						   const unsigned int message)
{
	thread_t* thread = reinterpret_cast<thread_t*>(context);
	HTTPContext* httpObject = reinterpret_cast<HTTPContext*>(httpContext);
	
	httpObject->setHWnd(hWnd);
	httpObject->setMessage(message);
	httpObject->setThreadContext(thread);

	thread->start(httpObject);
}

__stdcall long ThreadJoin(void* threadContext)
{
	thread_t* thread = reinterpret_cast<thread_t*>(threadContext);

	return thread->join();
}

__stdcall void ThreadClose(void* threadContext)
{
	thread_t* thread = reinterpret_cast<thread_t*>(threadContext);

	thread->quit();

	delete thread;
}


__stdcall void* HTTPCreateContext(const char* url,
								  const char* cookie,
								  const char* userAgent,
								  const long timeout)
{
	assert(url != NULL);

	Runnable* target =
		new HTTPContext(url,
						cookie,
						userAgent,
						timeout);

	return target;
}

__stdcall void* HTTPGetContentsSync(const char* url,
									const char* cookie,
									const char* userAgent,
									const long timeout,
									int* result)
{
	assert(url != NULL);
	assert(result != NULL);

	Runnable* target =
		new HTTPContext(url,
						cookie,
						userAgent,
						timeout,
						NULL,
						0);

	*result = target->run();
	return target;
}

__stdcall long HTTPGetLastModified(void* httpContext,
								   char* buffer, const int length)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	const std::string lastModified = target->getLastModified();
	if (buffer == NULL ||
		lastModified.size() > static_cast<unsigned int>(length))
		return lastModified.size();

	std::string::const_iterator itor = lastModified.begin();
	while (itor != lastModified.end())
		*buffer++ = *itor++;

	return lastModified.size();
}

__stdcall long HTTPGetResponseCode(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	return target->getResponseCode();
}

__stdcall long HTTPGetCRC32(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	return target->getCRC32();
}

__stdcall long HTTPGetFilteredCRC32(void* httpContext,
									void* managerContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	void* filter =
		FilterGetFilters(managerContext, target->getURL().c_str());

	std::string resource = target->getContentsString();
	char* str = new char[resource.length()];

	str[FilterApply(filter, str)] = 0;

	long crc32 = HTTPGetCRC32FromString(str);
	delete[] str;

	FilterRemoveFilters(filter);

	return crc32;
}

__stdcall long HTTPGetCRC32FromString(const char* buffer)
{
	CRC32 digester;

	const std::string crc32String = buffer;
	digester.setSource(crc32String.begin(), crc32String.end());
	return static_cast<long>(digester.getDigest());
}

__stdcall long HTTPContentsSave(void* httpContext, const char* filename)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	std::ofstream ofs(filename,
					  std::ios::binary | std::ios::out | std::ios::trunc);
	if (ofs.fail())
		return 0;

	ofs << target->getContentsString();
	ofs.close();
	return 1;
}

__stdcall long HTTPGetContentsLength(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	return target->getContentsString().length();
}

__stdcall long HTTPGetResource(void* httpContext,
							   char* buffer,
							   const int length)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	if (buffer == NULL)
		return target->getContentsString().length();

	const std::string contents = target->getContentsString();
	if (contents.length() <= static_cast<size_t>(length))
	{
		std::string::const_iterator itor = contents.begin();
		while (itor != contents.end())
			*buffer++ = *itor++;
	}

	return contents.length();
}

__stdcall void HTTPClose(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	delete target;
}

__stdcall void* RegexCompile(const char* pattern)
{
	RegexCompiler<char> compiler;
	RegexMatch<char>* matcher = NULL;
	try
	{
		RegexMatch<char> match = compiler.compile(pattern);
		matcher = new RegexMatch<char>(match);
	}
	catch (CompileError& e)
	{
		return NULL;
	}

	return matcher;
}

__stdcall long RegexMatcher(void* regexContext, void* httpContext,
							char* buffer, const int length,
							const int ignoreCase)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	std::string source = target->getContentsString();
	if (!matcher->match(source, static_cast<bool>(ignoreCase)))
		return 0;

	const std::string matchedStr = matcher->matchedString(source);
	if (buffer == NULL ||
		static_cast<size_t>(length) < matchedStr.length())
		return matchedStr.length();
	
	std::string::const_iterator itor = matchedStr.begin();
	while (itor != matchedStr.end())
		*buffer++ = *itor++;

	return matchedStr.length();
}

__stdcall long RegexMatchFromString(void* regexContext,
									const char* targetString,
									char* buffer,
									const int length,
									const int ignoreCase)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);

	std::string source = targetString;
	if (!matcher->match(source, static_cast<bool>(ignoreCase)))
		return 0;

	const std::string matchedStr = matcher->matchedString(source);
	if (buffer == NULL ||
		static_cast<size_t>(length) < matchedStr.length())
		return matchedStr.length();
	
	std::string::const_iterator itor = matchedStr.begin();
	while (itor != matchedStr.end())
		*buffer++ = *itor++;

	return matchedStr.length();
}

__stdcall long RegexMatchedString(void* regexContext,
								  const char* sourceString,
								  const int groupNumber,
								  char* buffer,
								  const int length)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);

	std::string source = sourceString;
	if (static_cast<unsigned int>(groupNumber) >= matcher->size())
		return -1;

	const std::string result = matcher->matchedString(source, groupNumber);
	if (result.size() <= static_cast<size_t>(length))
	{
		std::string::const_iterator itor = result.begin();
		while (itor != result.end())
			*buffer++ = *itor++;
	}

	return result.size();
}

__stdcall void RegexTerminate(void* regexContext)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);

	delete matcher;
}

using namespace Filter;

__stdcall void* FilterManagerCreate()
{
	FilterLoader loader("filter.txt");
	FilterManager* manager = new FilterManager();

	loader.load();
	manager->addRules(loader.getRules());

	return manager;
}

__stdcall void* FilterGetFilters(void* managerContext, const char* url)
{
	FilterManager* manager = 
		reinterpret_cast<FilterManager*>(managerContext);

	return new std::vector<Executor*>(manager->getExecutors(url));
}

__stdcall void FilterRemoveFilters(void* filterHandle)
{
	std::vector<Executor*>* executors =
		reinterpret_cast<std::vector<Executor*>*>(filterHandle);

	delete executors;
}

__stdcall long FilterApply(void* filterHandle, char* contents)
{
	assert(filterHandle != NULL);
	assert(contents != NULL);
		
	std::string target(contents);

	std::vector<Executor*>* executors =
		reinterpret_cast<std::vector<Executor*>*>(filterHandle);

	for (std::vector<Executor*>::iterator itor = executors->begin();
		 itor != executors->end(); ++itor)
		target = (*itor)->execute(target);

	std::string::const_iterator itor = target.begin();
	while (itor != target.end())
		*contents++ = *itor++;

	return target.length();
}

__stdcall void FilterManagerTerminate(void* managerContext)
{
	FilterManager* manager = 
		reinterpret_cast<FilterManager*>(managerContext);

	delete manager;
}

__stdcall void* WWWInit()
{
	return new SocketModule();
}

__stdcall void WWWTerminate(void* contextHandle)
{
	SocketModule* handle = reinterpret_cast<SocketModule*>(contextHandle);

	delete handle;
}

#if defined(DEBUGMAIN) || defined(DEBUGWINMAIN)
#	include "wwwdlltest.cpp"
#endif
