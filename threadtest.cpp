#include "wwwdll.h"
#include <Thread/Runnable.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <net/HTTPClient.hpp>

class RunningTarget : public Runnable
{
public:
	int count;

	virtual void prepare() throw()
	{}

	virtual unsigned run() throw(ThreadException)
	{
		std::cout << "thread running. " << count <<  std::endl;
		return 0;
	}

	virtual void dispose() throw()
	{}

};

struct Info
{
	std::string url;
	std::string lastModified;
	unsigned int crc32;
	unsigned int filteredCrc32;

	Info(const std::string url_,
		 const std::string lastModified_,
		 const unsigned int crc32_,
		 const unsigned int filteredCrc32_):
			url(url_),
			lastModified(lastModified_),
			crc32(crc32_),
			filteredCrc32(filteredCrc32_)
	{}
};

const char* urlList[] = {
	"http://www80.sakura.ne.jp/~noantica/index2.htm",
	"http://www.geocities.co.jp/Playtown/3932/",
	"http://www.kowroo.net/birne/index.htm",
	"http://misoodenn.web.fc2.com/main.html",
	"http://www.studiomilk.com/akachanclub/top.html",
	"http://www.studiomilk.com/ichigomilk/top.html",
	"http://www.wao.or.jp/user/rx780079/",
	"http://www.kowroo.net/purplediamond/indexxx.htm",
	"http://chokudokan.com/",
	"http://blog.livedoor.jp/pinaco_ikurumi/",
	"http://anzu.sakura.ne.jp/~hormone/"};

void my_unexpected()
{
	std::cerr << "unexpected exception." << std::endl;
	std::terminate();
}

void my_terminate()
{
	std::cerr << "terminate program." << std::endl;
	abort();
}

int main()
{
	std::set_unexpected(my_unexpected);
	std::set_terminate(my_terminate);

	const int contentsCount = sizeof(urlList) / sizeof(char*);

	SocketModule module;
// 	void* threadContext = ThreadCreate();
// 	std::vector<Info> items;

// 	void* target = NULL;
		
// 	char url[4096];
// 	char modified[256];
// 	unsigned int crc32;
// 	unsigned int filtered;

// 	void* wwwContext = WWWInit();
// 	void* filterManager = FilterManagerCreate();

	for (int i = 0; i < contentsCount; ++i)
	{
		HTTPClient client;
		client.setKeepAliveTime(300);
		client.setUserAgent("Mozilla/4.0 (compatible; MSIE 6.0; "
							"Windows NT 5.1; SV1; .NET CLR 1.0.3705; "
							".NET CLR 1.1.4322)");
		client.setAcceptEncoding("");
		client.addAcceptLanguage("ja");
		client.addAcceptLanguage("en");
		client.setTimeout(30);

		std::cout << i << ": " << urlList[i] << std::endl;
		try 
		{
			URL url(urlList[i]);
			client.getResource(url);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
//		HTTPResult<> result;
	}

	// for (int i = 0; i < contentsCount; ++i)
// 	{
// 		target = HTTPCreateContext(urlList[i], NULL, NULL, 30);
// 		ThreadStart(threadContext, target, NULL, 0);
// 		ThreadJoin(threadContext);
		
// 		url[HTTPGetURL(target, url, sizeof(url))] = 0;
// 		modified[HTTPGetLastModified(target, modified, sizeof(modified))] = 0;
// 		crc32 = HTTPGetCRC32(target);
// 		filtered = HTTPGetFilteredCRC32(target, filterManager);

// 		items.push_back(Info(url, modified, crc32, filtered));
// 		HTTPClose(target);
// 		std::cerr << ".";
// 	}

// 	ThreadClose(threadContext);
// 	FilterManagerTerminate(filterManager);
// 	WWWTerminate(wwwContext);

// 	std::cout << std::endl;
// 	std::cout.fill('0');
// 	for (std::vector<Info>::iterator itor = items.begin();
// 		 itor != items.end(); ++itor)
// 	{
// 		std::cout << itor->url << ", " << itor->lastModified << ", " <<
// 			"0x" << std::setw(8) << std::hex << itor->crc32 << ", " << 
// 			"0x" << std::setw(8) << std::hex << itor->filteredCrc32 << std::endl;
// 	}
	return 0;
}
