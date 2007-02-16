#include <net/HTTPClient.hpp>
#include <util/CRC.hpp>
#include <Thread/RerunnableThread.hpp>
#include <text/regex/RegexCompile.hpp>
#include "Filter.hpp"

#include <cassert>
#include <fstream>
#include "wwwdll.h"


typedef RerunnableThread thread_t;


#ifdef WIN32
struct AfterNotify
{
	HWND hWnd;
	unsigned int message;

	AfterNotify():
		hWnd(), message()
	{}

	AfterNotify(HWND hWnd_, unsigned int message_):
		hWnd(hWnd_), message(message_)
	{}

	AfterNotify(const AfterNotify& src):
		hWnd(src.hWnd), message(src.message)
	{}

	AfterNotify& operator=(const AfterNotify& src)
	{
		if (&src != this)
		{
			hWnd = src.hWnd;
			message = src.message;
		}

		return *this;
	}

	void operator()(bool result, thread_t* threadContext)
	{
		if (message == 0)
			return;

		PostMessage(hWnd, message,
					static_cast<WPARAM>(result),
					reinterpret_cast<LPARAM>(threadContext));
	}
};
#else
struct AfterNotify
{
	Callback func;

	AfterNotify():
		func(NULL)
	{}

	AfterNotify(const AfterNotify& src):
		func(src.func)
	{}

	AfterNotify(Callback function):
		func(function)
	{}

	AfterNotify& operator=(const AfterNotify& src)
	{
		if(&src != this)
		{
			func = src.func;
		}

		return *this;
	}

	void operator()(bool result, thread_t* threadContext)
	{
		if (func == NULL)
			return;

		func(result, threadContext);
	}
};
#endif /* WIN32 */

template <typename AfterFunctor>
class HTTPContext : public Runnable
{
private:
	std::string url;
	std::string cookie;
	std::string userAgent;
	const long timeout;
	HTTPResult<> result;
	thread_t* threadContext;
	AfterFunctor functor;

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
		catch (...)
		{
			return false;
		}
		return true;
	}

	HTTPContext(const HTTPContext&);
	HTTPContext& operator=(const HTTPContext&);
public:
	void setFunctor(const AfterFunctor& newFunctor)
	{
		functor = newFunctor;
	}

	std::string getURL() const
	{
		return this->url;
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
				AfterFunctor functor_):
		url(url_),
		cookie(cookie_ == NULL ? "" : cookie_),
		userAgent(userAgent_ == NULL ? "" : userAgent_),
		timeout(timeout_),
		result(),
		threadContext(),
		functor(functor_)
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

			functor(result, this->getThreadContext());
		}
		catch(std::exception& e)
		{
			functor(result, this->getThreadContext());
			throw ThreadException(e.what());
		}
		catch(...)
		{
			functor(result, this->getThreadContext());
		}

		return result;
	}
};

void* CALLDECL ThreadCreate()
{
	return new thread_t;
}

#ifdef WIN32
void CALLDECL ThreadStart(void* context,
						   void* httpContext,
						   HWND hWnd,
						   const unsigned int message)
{
	thread_t* thread = reinterpret_cast<thread_t*>(context);
	HTTPContext<AfterNotify>* httpObject =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);
	
	httpObject->setFunctor(AfterNotify(hWnd, message));
	httpObject->setThreadContext(thread);

	thread->start(httpObject);
}
#else
void CALLDECL ThreadStart(void* context,
						   void* httpContext,
						   Callback function)
{
	thread_t* thread = reinterpret_cast<thread_t*>(context);
	HTTPContext<AfterNotify>* httpObject =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	httpObject->setFunctor(AfterNotify(function));
	httpObject->setThreadContext(thread);

	thread->start(httpObject);
}

#endif /* WIN32 */

long CALLDECL ThreadJoin(void* threadContext)
{
	thread_t* thread = reinterpret_cast<thread_t*>(threadContext);

	return thread->join();
}

void CALLDECL ThreadClose(void* threadContext)
{
	thread_t* thread = reinterpret_cast<thread_t*>(threadContext);

	thread->quit();

	delete thread;
}

void* CALLDECL HTTPCreateContext(const char* url,
								  const char* cookie,
								  const char* userAgent,
								  const long timeout)
{
	assert(url != NULL);

	Runnable* target =
		new HTTPContext<AfterNotify>(url,
									 cookie,
									 userAgent,
									 timeout,
									 AfterNotify());

	return target;
}

void* CALLDECL HTTPGetContentsSync(const char* url,
									const char* cookie,
									const char* userAgent,
									const long timeout,
									int* result)
{
	assert(url != NULL);
	assert(result != NULL);

	Runnable* target =
		new HTTPContext<AfterNotify>(url,
									 cookie,
									 userAgent,
									 timeout,
									 AfterNotify());

	target->prepare();
	*result = target->run();
	return target;
}

int CALLDECL HTTPGetURL(void* httpContext, char* url, const int length)
{
	assert(url != NULL);
	assert(length >= 0);

	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	std::string targetUrl = target->getURL();
	if (targetUrl.length() <= static_cast<size_t>(length))
	{
		std::string::iterator itor = targetUrl.begin();
		while (itor != targetUrl.end())
			*url++ = *itor++;
	}

	return static_cast<int>(targetUrl.length());
}

long CALLDECL HTTPGetLastModified(void* httpContext,
								   char* buffer, const int length)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	const std::string lastModified = target->getLastModified();
	if (buffer == NULL ||
		lastModified.size() > static_cast<unsigned int>(length))
		return static_cast<long>(lastModified.size());

	std::string::const_iterator itor = lastModified.begin();
	while (itor != lastModified.end())
		*buffer++ = *itor++;

	return static_cast<long>(lastModified.size());
}

long CALLDECL HTTPGetResponseCode(void* httpContext)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	return target->getResponseCode();
}

long CALLDECL HTTPGetCRC32(void* httpContext)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	return target->getCRC32();
}

long CALLDECL HTTPGetFilteredCRC32(void* httpContext,
									void* managerContext)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	void* filter =
		FilterGetFilters(managerContext, target->getURL().c_str());

	std::string resource = target->getContentsString();
	char* str = new char[resource.length()+1];
	std::copy(resource.begin(), resource.end(), str);
	str[resource.length()] = 0;

	const long result = FilterApply(filter, str);
	assert(result >= 0 && resource.length() >= static_cast<size_t>(result));
	str[result] = 0;

	long crc32 = HTTPGetCRC32FromString(str);
	delete[] str;

	FilterRemoveFilters(filter);

	return crc32;
}

long CALLDECL HTTPGetCRC32FromString(const char* buffer)
{
	CRC32 digester;

	const std::string crc32String = buffer;
	digester.setSource(crc32String.begin(), crc32String.end());
	return static_cast<long>(digester.getDigest());
}

long CALLDECL HTTPContentsSave(void* httpContext, const char* filename)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	std::ofstream ofs(filename,
					  std::ios::binary | std::ios::out | std::ios::trunc);
	if (ofs.fail())
		return 0;

	ofs << target->getContentsString();
	ofs.close();
	return 1;
}

long CALLDECL HTTPFilteredContentsSave(void* httpContext,
									   void* managerContext,
									   const char* filename)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	std::ofstream ofs(filename,
					  std::ios::binary | std::ios::out | std::ios::trunc);
	if (ofs.fail())
		return 0;

	std::string resource = target->getContentsString();
	char* str = new char[resource.length()+1];
	std::copy(resource.begin(), resource.end(), str);
	str[resource.length()] = 0;

	void* filter =
		FilterGetFilters(managerContext, target->getURL().c_str());
	const long result = FilterApply(filter, str);
	assert(result >= 0 && resource.length() >= static_cast<size_t>(result));
	
	str[result] = 0;

	ofs << str;
	ofs.close();
	FilterRemoveFilters(filter);
	return 1;
}

long CALLDECL HTTPGetContentsLength(void* httpContext)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	return static_cast<long>(target->getContentsString().length());
}

long CALLDECL HTTPGetResource(void* httpContext,
							   char* buffer,
							   const int length)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

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

void CALLDECL HTTPClose(void* httpContext)
{
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

	delete target;
}

void* CALLDECL RegexCompile(const char* pattern)
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

long CALLDECL RegexMatcher(void* regexContext, void* httpContext,
							char* buffer, const int length,
							const int ignoreCase)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);
	HTTPContext<AfterNotify>* target =
		reinterpret_cast<HTTPContext<AfterNotify>*>(httpContext);

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

long CALLDECL RegexMatchFromString(void* regexContext,
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

long CALLDECL RegexMatchedString(void* regexContext,
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

void CALLDECL RegexTerminate(void* regexContext)
{
	RegexMatch<char>* matcher =
		reinterpret_cast<RegexMatch<char>*>(regexContext);

	delete matcher;
}

using namespace Filter;

void* CALLDECL FilterManagerCreate()
{
	FilterLoader loader("filter.txt");
	FilterManager* manager = new FilterManager();

	loader.load();
	manager->addRules(loader.getRules());

	return manager;
}

void* CALLDECL FilterGetFilters(void* managerContext, const char* url)
{
	FilterManager* manager = 
		reinterpret_cast<FilterManager*>(managerContext);

	return new std::vector<Executor*>(manager->getExecutors(url));
}

void CALLDECL FilterRemoveFilters(void* filterHandle)
{
	std::vector<Executor*>* executors =
		reinterpret_cast<std::vector<Executor*>*>(filterHandle);

	delete executors;
}

long CALLDECL FilterApply(void* filterHandle, char* contents)
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

	/**
	 * どーもiteratorで回すとへんなコード生成するみたい・・・
	 * 勘弁してくれ･･･
	 * んー、_S_createがこけたのに例外握りつぶしてたどり
	 * 着いてるのかなぁ･･･
	 */
// 	for (size_t offset = 0; offset < target.length(); ++offset)
// 		contents[offset] = target[offset];

	return static_cast<long>(target.length());
}

void CALLDECL FilterManagerTerminate(void* managerContext)
{
	FilterManager* manager = 
		reinterpret_cast<FilterManager*>(managerContext);

	delete manager;
}

void* CALLDECL WWWInit()
{
	return new SocketModule();
}

void CALLDECL WWWTerminate(void* contextHandle)
{
	SocketModule* handle = reinterpret_cast<SocketModule*>(contextHandle);

	delete handle;
}

#if defined(DEBUGMAIN) || defined(DEBUGWINMAIN)
#	include "wwwdlltest.cpp"
#endif
