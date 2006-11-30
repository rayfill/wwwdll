#include "wwwdll.cpp"
#include <text/LexicalCast.hpp>
#include <iostream>
#include "wwwdll.h"
#include "item.hpp"
#include <Thread/ScopedLock.hpp>
#include <Thread/Mutex.hpp>
#include <map>
#include <stdexcept>
#include <pthread.h>
#include <signal.h>

int usage(int argc, char** argv)
{
	std::cout << argv[0] << " wakeUpThreadCount" << std::endl;
	return -1;
}

void saveItems(std::vector<Item>& items)
{
	std::ofstream ofs("url_checked.txt", std::ios::out | std::ios::trunc);
	ofs << "check_flg	url	url_check	url_open	name"
		"last_updated	last_visited	crc32	content_length	id"
		<< std::endl;

	for (std::vector<Item>::const_iterator itor = items.begin();
		 itor != items.end(); ++itor)
		ofs << itor->toString() << std::endl;
	ofs.close();
}

std::vector<Item> loadItems()
{
	std::ifstream ifs("url.txt");
	if (ifs.fail())
		return std::vector<Item>();

	char lineBuffer[4096];
	// ヘッダ読み飛ばし
	ifs.getline(lineBuffer, sizeof(lineBuffer));

	std::vector<Item> items;

	// read a check list.
	while (!ifs.fail())
	{
		ifs.getline(lineBuffer, sizeof(lineBuffer));
		Item item = Item::parse(lineBuffer);
		if (item.url != "")
			items.push_back(item);
	}

	return items;
}

void resetItems(std::vector<Item>& items)
{
	for (std::vector<Item>::iterator itor = items.begin();
		 itor != items.end(); ++itor)
		itor->check_flg = false;
}

std::vector<Item>::iterator find_uncheck_item(std::vector<Item>& items)
{
	for (std::vector<Item>::iterator itor = items.begin();
		 itor != items.end(); ++itor)
		if (itor->check_flg == false)
			return itor;

	return items.end();
}

std::vector<Item>::iterator find_item_from_url(std::vector<Item>& items,
											   const std::string& url)
{
	for (std::vector<Item>::iterator itor = items.begin();
		 itor != items.end(); ++itor)
	{
		if (itor->url_check == url)
			return itor;
	}

	return items.end();
}

Mutex syncObject;
std::vector<void*> idleThreads;

pthread_t parent;

void handler(int result, void* threadContext)
{
	{
		ScopedLock<Mutex> lock(syncObject);
		idleThreads.push_back(threadContext);
	}

	pthread_kill(parent, SIGUSR1);
}

int main(int argc, char** argv)
{
	if (argc < 2)
		return usage(argc, argv);

	parent = pthread_self();
	sigset_t original, masking;
	sigemptyset(&original);
	sigemptyset(&masking);
	sigaddset(&masking, SIGUSR1);
	pthread_sigmask(SIG_SETMASK, &masking, &original);

	int wakeupThreads = 0;
	try
	{
		wakeupThreads = lexicalCast<int>(argv[argc-1]);
		if (wakeupThreads < 1)
			return usage(argc, argv);

		std::cout << "wakeup threads of " << wakeupThreads << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return usage(argc, argv);
	}

	void* handle = WWWInit();
	std::vector<Item> items = loadItems();
	resetItems(items);
	void* filterManagerContext = FilterManagerCreate();

	std::map<void*, void*> contextMap;
	

	// create threads.
	for (int count = 0; count != wakeupThreads; ++count)
	{
		std::vector<Item>::iterator itor = find_uncheck_item(items);
		if (itor == items.end())
			break;

		itor->check_flg = true;
		void* thread = ThreadCreate();
		void* httpContext = HTTPCreateContext(itor->url_check.c_str(),
											  NULL, NULL, 20);
		contextMap[thread] = httpContext;
		ThreadStart(thread, httpContext, handler);
	}

	try
	{
		std::vector<Item>::iterator itor;
		while ((itor = find_uncheck_item(items)) != items.end())
		{
			itor->check_flg = true;
			
			int signalNum = 0;
			sigwait(&masking, &signalNum);
			if (signalNum == SIGUSR1)
			{
				ScopedLock<Mutex> lock(syncObject);
				assert (idleThreads.size() > 0);
				void* thread = idleThreads.back();
				idleThreads.pop_back();
				ThreadJoin(thread);

				void* httpContext = contextMap[thread];
				assert(httpContext != NULL);
				contextMap[thread] = NULL;

				{
					char url[2048];
					url[HTTPGetURL(httpContext, url, sizeof(url))] = 0;
					std::vector<Item>::iterator item_itor =
						find_item_from_url(items, url);

					if (item_itor == items.end())
						throw std::runtime_error((std::string("unfind URL: ") +
												  url).c_str());

					char date[256];
					date[HTTPGetLastModified(httpContext,
											 date, sizeof(date))] = 0;
					item_itor->last_updated = date;

					item_itor->crc32 = HTTPGetFilteredCRC32(httpContext,
													   filterManagerContext);
				
					item_itor->content_length =
						HTTPGetContentsLength(httpContext);

					HTTPClose(httpContext);
				}
				HTTPCreateContext(itor->url_check.c_str(),
								  NULL, NULL, 20);
				contextMap[thread] = httpContext;
				ThreadStart(thread, httpContext, handler);
			}
		}

		// 稼働中のスレッドがwakeupThreads個存在する
		for (int count = 0; count != wakeupThreads; ++count)
		{
			int signalNum = 0;
			sigwait(&masking, &signalNum);
			ScopedLock<Mutex> lock(syncObject);
			ThreadClose(idleThreads.back());
			idleThreads.pop_back();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "raise exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "unknown exception raised." << std::endl;
	}

	saveItems(items);
	FilterManagerTerminate(filterManagerContext);
	WWWTerminate(handle);
	pthread_sigmask(SIG_SETMASK, &original, NULL);

	return 0;
}
