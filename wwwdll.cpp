#include "filter.hpp"
#include <net/HTTPClient.hpp>
#include <util/CRC.hpp>
#include <Thread/ThreadPool.hpp>
#include <text/regex/RegexCompile.hpp>
#include <cassert>
#include <fstream>
#include "wwwdll.h"
// #include <iostream>

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
//			std::cerr << e.what() << std::endl;
			return false;
		}
		catch (...)
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
		bool result = false;
		try
		{
			result = this->getContents();

			if (hWnd != NULL)
				PostMessage(hWnd, message,
							static_cast<WPARAM>(result),
							reinterpret_cast<LPARAM>(this->getThreadContext()));
		}
		catch(std::exception& e)
		{
			throw ThreadException(e.what());
		}
		catch(...)
		{
			//std::cerr << "unknown exception raised." << std::endl;
		}

		return result;
	}
};

void* __stdcall ThreadCreate()
{
	return new thread_t;
}

void __stdcall ThreadStart(void* context,
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

long __stdcall ThreadJoin(void* threadContext)
{
	thread_t* thread = reinterpret_cast<thread_t*>(threadContext);

	return thread->join();
}

void __stdcall ThreadClose(void* threadContext)
{
	thread_t* thread = reinterpret_cast<thread_t*>(threadContext);

	thread->quit();

	delete thread;
}

void* __stdcall HTTPCreateContext(const char* url,
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

void* __stdcall HTTPGetContentsSync(const char* url,
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

int __stdcall HTTPGetURL(void* httpContext, char* url, const int length)
{
	assert(url != NULL);
	assert(length >= 0);

	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	std::string targetUrl = target->getURL();
	if (targetUrl.length() <= static_cast<size_t>(length))
	{
		std::string::iterator itor = targetUrl.begin();
		while (itor != targetUrl.end())
			*url++ = *itor++;
	}

	return static_cast<int>(targetUrl.length());
}

long __stdcall HTTPGetLastModified(void* httpContext,
								   char* buffer, const int length)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	const std::string lastModified = target->getLastModified();
	if (buffer == NULL ||
		lastModified.size() > static_cast<unsigned int>(length))
		return static_cast<long>(lastModified.size());

	std::string::const_iterator itor = lastModified.begin();
	while (itor != lastModified.end())
		*buffer++ = *itor++;

	return static_cast<long>(lastModified.size());
}

long __stdcall HTTPGetResponseCode(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	return target->getResponseCode();
}

long __stdcall HTTPGetCRC32(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	return target->getCRC32();
}

long __stdcall HTTPGetFilteredCRC32(void* httpContext,
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

long __stdcall HTTPGetCRC32FromString(const char* buffer)
{
	CRC32 digester;

	const std::string crc32String = buffer;
	digester.setSource(crc32String.begin(), crc32String.end());
	return static_cast<long>(digester.getDigest());
}

long __stdcall HTTPContentsSave(void* httpContext, const char* filename)
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

long __stdcall HTTPGetContentsLength(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	return static_cast<long>(target->getContentsString().length());
}

long __stdcall HTTPGetResource(void* httpContext,
							   char* buffer,
							   const int length)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	if (buffer == NULL)
		return static_cast<long>(target->getContentsString().length());

	const std::string contents = target->getContentsString();
	if (contents.length() <= static_cast<size_t>(length))
	{
		std::string::const_iterator itor = contents.begin();
		while (itor != contents.end())
			*buffer++ = *itor++;
	}

	return static_cast<long>(contents.length());
}

void __stdcall HTTPClose(void* httpContext)
{
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	delete target;
}

void* __stdcall RegexCompile(const char* pattern)
{
	RegexCompiler<char> compiler;
	RegexMatch<char>* matcher = NULL;
	try
	{
		RegexMatch<char> match = compiler.compile(pattern);
		matcher = new RegexMatch<char>(match);
	}
	catch (CompileError& /*e*/)
	{
		return NULL;
	}

	return matcher;
}

long __stdcall RegexMatcher(void* regexContext, void* httpContext,
							char* buffer, const int length,
							const int ignoreCase)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);
	HTTPContext* target =
		reinterpret_cast<HTTPContext*>(httpContext);

	std::string source = target->getContentsString();
	if (!matcher->match(source, ignoreCase != 0))
		return 0;

	const std::string matchedStr = matcher->matchedString(source);
	if (buffer == NULL ||
		static_cast<size_t>(length) < matchedStr.length())
		return static_cast<long>(matchedStr.length());
	
	std::string::const_iterator itor = matchedStr.begin();
	while (itor != matchedStr.end())
		*buffer++ = *itor++;

	return static_cast<long>(matchedStr.length());
}

long __stdcall RegexMatchFromString(void* regexContext,
									const char* targetString,
									char* buffer,
									const int length,
									const int ignoreCase)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);

	std::string source = targetString;
	if (!matcher->match(source, ignoreCase != 0))
		return 0;

	const std::string matchedStr = matcher->matchedString(source);
	if (buffer == NULL ||
		static_cast<size_t>(length) < matchedStr.length())
		return static_cast<long>(matchedStr.length());
	
	std::string::const_iterator itor = matchedStr.begin();
	while (itor != matchedStr.end())
		*buffer++ = *itor++;

	return static_cast<long>(matchedStr.length());
}

long __stdcall RegexMatchedString(void* regexContext,
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

	return static_cast<long>(result.size());
}

void __stdcall RegexTerminate(void* regexContext)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);

	delete matcher;
}

using namespace Filter;

void* __stdcall FilterManagerCreate()
{
	FilterLoader loader("filter.txt");
	FilterManager* manager = new FilterManager();

	loader.load();
	manager->addRules(loader.getRules());

	return manager;
}

void* __stdcall FilterGetFilters(void* managerContext, const char* url)
{
	FilterManager* manager = 
		reinterpret_cast<FilterManager*>(managerContext);

	return new std::vector<Executor*>(manager->getExecutors(url));
}

void __stdcall FilterRemoveFilters(void* filterHandle)
{
	std::vector<Executor*>* executors =
		reinterpret_cast<std::vector<Executor*>*>(filterHandle);

	delete executors;
}

long __stdcall FilterApply(void* filterHandle, char* contents)
{
	assert(filterHandle != NULL);
	assert(contents != NULL);
		
	std::string target(contents);

	std::vector<Executor*>* executors =
		reinterpret_cast<std::vector<Executor*>*>(filterHandle);

	for (std::vector<Executor*>::iterator itor = executors->begin();
		 itor != executors->end(); ++itor)
		target = (*itor)->execute(target);

// 	std::string::const_iterator itor = target.begin();
// 	while (itor != target.end())
// 		*contents++ = *itor++;

	/**
	 * どーもiteratorで回すとへんなコード生成するみたい・・・
	 * 勘弁してくれ･･･
	 * んー、_S_createがこけたのに例外握りつぶしてたどり
	 * 着いてるのかなぁ･･･
	 */
	for (size_t offset = 0; offset < target.length(); ++offset)
		contents[offset] = target[offset];

	return static_cast<long>(target.length());
}

void __stdcall FilterManagerTerminate(void* managerContext)
{
	FilterManager* manager = 
		reinterpret_cast<FilterManager*>(managerContext);

	delete manager;
}

void* __stdcall WWWInit()
{
	return new SocketModule();
}

void __stdcall WWWTerminate(void* contextHandle)
{
	SocketModule* handle = reinterpret_cast<SocketModule*>(contextHandle);

	delete handle;
}

#if defined(DEBUGMAIN) || defined(DEBUGWINMAIN)
#	include "wwwdlltest.cpp"
#endif
