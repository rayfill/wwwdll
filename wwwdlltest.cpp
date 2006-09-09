#include <string>
#include <wwwdll.h>

#ifdef DEBUGMAIN
#include <iostream>
#include <fstream>
#include <iomanip>
#include <text/LexicalCast.hpp>

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
										   "COOKIE=MONSTER",
										   "TestAgent",
										   timeout,
										   &result);
		if (result == 0)
		{
			std::cout << "content get failed." << std::endl;
			return 0;
		}

		// last modified
		long length = HTTPGetLastModified(httpContext, NULL, 0);
		char* buffer = new char[length+1];
		buffer[HTTPGetLastModified(httpContext, buffer, length)] = 0;
		std::cout << "last modified: " << buffer << std::endl;
		delete[] buffer;

		// crc
		std::cout << "crc: " << HTTPGetCRC32(httpContext) << std::endl;
		void* filterManager = FilterManagerCreate();
		std::cout << "filtered crc: " << 
			HTTPGetFilteredCRC32(httpContext, filterManager) << std::endl;
		FilterManagerTerminate(filterManager);

		// save file.
		HTTPContentsSave(httpContext, "savedfile.html");

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

unsigned long crc;
char dateBuffer[256];
int dateLength;
int responseCode;
void* filterManagerContext;
void* threadContext;
void* httpContext;
const char url[] = "http://puchi-neko.hp.infoseek.co.jp/";

LRESULT CALLBACK WndProc(HWND hWnd,
						 UINT message,
						 WPARAM wParam,
						 LPARAM lParam)
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
			MessageBox(hWnd, "start", "", MB_OK);
			httpContext = HTTPCreateContext(url,
											NULL,
											NULL,
											300);
		
			ThreadStart(threadContext, httpContext, hWnd, WM_USER+1);
			break;
		}

	case WM_USER+1:
		if (wParam != 0)
		{
			std::string message;
			
			// last modified
			long length = HTTPGetLastModified(httpContext, NULL, 0);
			char* buffer = new char[length+1];
			buffer[HTTPGetLastModified(httpContext, buffer, length)] = 0;
			message += std::string("last modified: ") + buffer + "\r\n";
			delete[] buffer;

			// crc
			message += "crc: " +
				stringCast<long>(HTTPGetCRC32(httpContext)) +
				"\r\n";

			// save file.
			HTTPContentsSave(httpContext, "savedfile.html");

			// filter
			void* filter =
				FilterGetFilters(filterManagerContext, url);
			
			const long contentsLength = HTTPGetContentsLength(httpContext);
			buffer = new char[contentsLength+1];
			buffer[HTTPGetResource(httpContext, buffer, contentsLength)] = 0;
			const long strSize = FilterApply(filter, buffer);
			buffer[strSize] = 0;
			std::ofstream ofs("savefile_filterd.txt",
							  std::ios::binary |
							  std::ios::out |
							  std::ios::trunc);
			ofs << buffer;
			ofs.close();
			message += "crc32: " +
				stringCast<long>(HTTPGetCRC32FromString(buffer)) + "\r\n";

			delete buffer;
			FilterRemoveFilters(filter);

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
			message += std::string("WWWC: ") + matchedString + "\r\n";
			delete[] matchedString;

			// post process
			RegexTerminate(regexHandle);
			HTTPClose(httpContext);

			MessageBox(hWnd, message.c_str(), "", MB_OK);
		}
		else
			MessageBox(hWnd, "failed get.", "", MB_OK);
		ThreadJoin(threadContext);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


int WINAPI WinMain(HINSTANCE hInst, 
				   HINSTANCE hPrev,
				   char* pszCmdLine,
				   int nCmdShow)
{
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

	HWND hWnd = CreateWindowEx(0, className, "title",
							   WS_OVERLAPPEDWINDOW,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, hInst, NULL);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	crc = 0;
	dateLength = sizeof(dateBuffer);
	filterManagerContext = FilterManagerCreate();
	threadContext = ThreadCreate();

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}

	ThreadClose(threadContext);
	FilterManagerTerminate(filterManagerContext);
	WWWTerminate(handle);
	
	return msg.wParam;
}

#endif
