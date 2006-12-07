#include <string>

#ifdef DEBUGMAIN
#include <iostream>
#include <fstream>
#include <iomanip>
#include <text/LexicalCast.hpp>
#include "wwwdll.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "argument error." << std::endl;
		return -1;
	}

	try
	{
		void* handle = WWWInit();
		const long timeout = lexicalCast<long>(argv[1]);
		std::string url = argv[2];

		void* httpContext;
		int result = 0;
		httpContext = HTTPGetContentsSync(url.c_str(),
										  NULL,
										  "Mozilla/5.0 "
										  "(Windows; U; Windows NT 5.1; ja; "
										  "rv:1.8.0.7) Gecko/20060909 "
										  "Firefox/1.5.0.7",
										  timeout,
										  &result);
		if (result == 0)
		{
			std::cout << "content get failed." << std::endl;
			return 0;
		}

		std::cout << "HTTP status:" <<
			HTTPGetResponseCode(httpContext) << std::endl;

		// save file.
		HTTPContentsSave(httpContext, "savedfile.html");

		// last modified
		long length = HTTPGetLastModified(httpContext, NULL, 0);
		char* buffer = new char[length+1];
		buffer[HTTPGetLastModified(httpContext, buffer, length)] = 0;
		std::cout << "last modified: " << buffer << std::endl;
		delete[] buffer;

		// content length
		std::cout << "length: " << HTTPGetContentsLength(httpContext) << std::endl;
		// crc
		std::cout << "crc: " << HTTPGetCRC32(httpContext) << std::endl;

		void* filterManager = FilterManagerCreate();
		std::cout << "filtered crc: " << 
			HTTPGetFilteredCRC32(httpContext, filterManager) << std::endl;
		FilterManagerTerminate(filterManager);

		// regex
		int matchedLength = HTTPGetContentsLength(httpContext);
		char* matchedString = new char[matchedLength + 1];

		void* regexHandle = 
			RegexCompile("<meta[ 	]+name=\"wwwc\"[ 	]+content="
						 "\"([^\"]+)\"[ 	]*>");

		matchedString[RegexMatcher(regexHandle,
								   httpContext,
								   matchedString,
								   matchedLength,
								   1)] = 0;
		std::cout << "WWWC: " << matchedString << std::endl;
		RegexTerminate(regexHandle);

		delete[] matchedString;

		// post process
		HTTPClose(httpContext);

		WWWTerminate(handle);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception raised." << std::endl;
	}

	return 0;
}
#endif

#ifdef DEBUGWINMAIN
#include <text/LexicalCast.hpp>
#include <text/regex/RegexCompile.hpp>
#include <Thread/CriticalSection.hpp>
#include <Thread/ScopedLock.hpp>
#include "item.hpp"
#include <map>
#include <fstream>
#include <stdexcept>
#include "wwwdll.h"
#include <iostream>

class Win32Exception : public std::runtime_error
{
private:
	std::string transException(const unsigned int code,
							   _EXCEPTION_POINTERS* /*exceptionInfo*/)
	{
		switch (code)
		{
			case EXCEPTION_ACCESS_VIOLATION:
				return "AccessViolation";
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				return "DataType Misalignment";
			case EXCEPTION_BREAKPOINT:
				return "break point";
			case EXCEPTION_SINGLE_STEP:
				return "single step";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				return "array bounds exceeded";
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				return "flt denormal operand";
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				return "flt divide by zero";
			case EXCEPTION_FLT_INEXACT_RESULT:
				return "flt inexact result";
			case EXCEPTION_FLT_INVALID_OPERATION:
				return "flt invalid operation";
			case EXCEPTION_FLT_OVERFLOW:
				return "flt overflow";
			case EXCEPTION_FLT_STACK_CHECK:
				return "flt stack check";
			case EXCEPTION_FLT_UNDERFLOW:
				return "flt underflow";
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				return "int divide by zero";
			case EXCEPTION_INT_OVERFLOW:
				return "int overflow";
			case EXCEPTION_PRIV_INSTRUCTION:
				return "priv instruction";
			case EXCEPTION_IN_PAGE_ERROR:
				return "in page error";
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				return "illegal instruction";
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				return "noncontinuable exception";
			case EXCEPTION_STACK_OVERFLOW:
				return "stack overflow";
			case EXCEPTION_INVALID_DISPOSITION:
				return "invalid disposition";
			case EXCEPTION_GUARD_PAGE:
				return "guard page";
			case EXCEPTION_INVALID_HANDLE:
				return "invalid handle";
		}

		return "unknown exception";
	}

public:
	Win32Exception(const unsigned int code,
		_EXCEPTION_POINTERS* exceptionInfo):
			std::runtime_error(transException(code, exceptionInfo))
	{}

	virtual ~Win32Exception() throw()
	{}

};

void translatorFunction(unsigned int exceptionCode,
						_EXCEPTION_POINTERS* exceptionInfo)
{
	throw Win32Exception(exceptionCode, exceptionInfo);
}

void* filterManagerContext;
std::map<void*, void*> contextMapper;
std::vector<Item> items;

void saveItems()
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
	// �w�b�_�ǂݔ�΂�
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

std::vector<Item>::iterator find_uncheck_item()
{
	for (std::vector<Item>::iterator itor = items.begin();
		 itor != items.end(); ++itor)
		if (itor->check_flg == false)
			return itor;

	return items.end();
}

std::vector<Item>::iterator find_item_from_url(const std::string& url)
{
	for (std::vector<Item>::iterator itor = items.begin();
		 itor != items.end(); ++itor)
	{
		if (itor->url_check == url)
			return itor;
	}

	return items.end();
}

static int threadCount = 0;
static int checkCount = 0;

void updateCounter(HWND hWnd)
{
	SetWindowText(hWnd, 
				  (std::string("thread: ") +
				   stringCast<int>(threadCount) +
				   std::string(", check: ") +
				   stringCast<int>(checkCount)).c_str());
}

std::vector<void*> threadHandler;
std::vector<void*> waitableThreads;

CriticalSection section;
CriticalSection debugSec;
static int postCount = 0;

LRESULT CALLBACK WndProc(HWND hWnd,
						 UINT message,
						 WPARAM wParam,
						 LPARAM lParam)
{
	try
	{
		switch (message)
		{
			case WM_CLOSE:
				DestroyWindow(hWnd);
				return 0;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			case WM_KEYUP:
			{
				ScopedLock<CriticalSection> lock(section);
				std::vector<Item>::iterator itor = find_uncheck_item();
				if (itor != items.end() && waitableThreads.size() > 0)
				{
					void* threadContext = waitableThreads.back();
					waitableThreads.pop_back();

					itor->check_flg = true;
					void* httpContext =
						HTTPCreateContext(itor->url_check.c_str(),
										  NULL, NULL, 30);

					contextMapper[threadContext] = httpContext;
					ThreadStart(threadContext, httpContext, hWnd, WM_USER+1);
					++threadCount;

					updateCounter(hWnd);
				}
				break;
			}

			case WM_USER+1:
			{
				{
					ScopedLock<CriticalSection> lock(debugSec);
					++postCount;
//					OutputDebugString((std::string("posted count of ") +
//							stringCast<int>(postCount)).c_str());
				}

				void* threadContext = reinterpret_cast<void*>(lParam);
				ThreadJoin(threadContext);

				void* httpContext = contextMapper[threadContext];
				contextMapper[threadContext] = NULL;

				if (wParam != 0)
				{
					++checkCount;
					updateCounter(hWnd);
					char url[2048];
					url[HTTPGetURL(httpContext, url, sizeof(url))] = 0;

					std::vector<Item>::iterator itor = find_item_from_url(url);

					char date[256];
					date[HTTPGetLastModified(httpContext, date, sizeof(date))] = 0;
					itor->last_updated = date;
				
					itor->crc32 = HTTPGetFilteredCRC32(httpContext,
													   filterManagerContext);
				
					itor->content_length = HTTPGetContentsLength(httpContext);
				}

				HTTPClose(httpContext);
				httpContext = NULL;

				{
					ScopedLock<CriticalSection> lock(section);
					std::vector<Item>::iterator itor = find_uncheck_item();
					if (itor == items.end())
					{
						--threadCount;
						updateCounter(hWnd);
					}
					else
					{
						itor->check_flg = true;
						httpContext =
							HTTPCreateContext(itor->url_check.c_str(),
											  NULL, NULL, 30);

						contextMapper[threadContext] = httpContext;
						ThreadStart(threadContext, httpContext, hWnd, WM_USER+1);
					}
				}		
				break;
			}

			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				BeginPaint(hWnd, &ps);
				const std::string message =
					"�L�[���������Ƃōő�4�X���b�h�܂ő����܂��B";
				TextOut(ps.hdc, 0, 0, message.c_str(), static_cast<int>(message.length()));
				EndPaint(hWnd, &ps);

				return 0;
			}

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	catch (std::exception& e)
	{
//		std::cout << std::endl << "exception: " << e.what() << std::endl;
	}

	return 0;
}

//#include <eh.h>
#include <exception>

void my_terminate()
{
//	std::cout << "terminate program" << std::endl;
	abort();
}

void my_unexpected()
{
//	std::cout << "unexpected exception" << std::endl;
	std::terminate();
}

int WINAPI WinMain(HINSTANCE hInst, 
				   HINSTANCE /*hPrev*/,
				   char* /*pszCmdLine*/,
				   int /*nCmdShow*/)
{
	std::set_unexpected(my_unexpected);
	std::set_terminate(my_terminate);
// 	_set_se_translator((_se_translator_function)translatorFunction);

	void* handle = WWWInit();

	char* className = "windowclass";
	WNDCLASSEX cls;
	ZeroMemory(&cls, sizeof(cls));
	cls.cbSize = sizeof(cls);
	cls.style = 0;
	cls.lpfnWndProc = WndProc;
	cls.hInstance = hInst;
	cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	cls.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	cls.lpszClassName = className;
	RegisterClassEx(&cls);

	items = loadItems();
	HWND hWnd = CreateWindowEx(0, className, "title",
							   WS_OVERLAPPEDWINDOW,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   320, 240,
							   NULL, NULL, hInst, NULL);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	for (int i = 0; i < 4; ++i)
		threadHandler.push_back(ThreadCreate());

	waitableThreads = threadHandler;

	filterManagerContext = FilterManagerCreate();

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}

	FilterManagerTerminate(filterManagerContext);
	WWWTerminate(handle);

	for (std::vector<void*>::iterator itor = threadHandler.begin();
		 itor != threadHandler.end(); ++itor)
		ThreadClose(*itor);

	saveItems();
	return static_cast<int>(msg.wParam);
}

#endif
