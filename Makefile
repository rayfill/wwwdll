CXX=/cygdrive/c/MinGW/bin/g++
#CXX=g++
DLLWRAP=/cygdrive/c/MinGW/bin/dllwrap
DLLTOOL=/cygdrive/c/MinGW/bin/dlltool
DEPENDS_DIR=../cpp_lib
CXXFLAGS +=-g -Wall -D_WIN32_WINNT=0x0500 -D_REENTRANT -mthreads
.PHONY: strip clean depends

test: wwwdll.dll wwwdlltest
	./wwwdlltest.exe 300 http://www.geocities.jp/hou_para/top/mainpage.html

filterTest: FilterTest.cpp Filter.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -o filterTest FilterTest.cpp $(LIBS)

debug: wwwdll.h wwwdll.cpp wwwdlltest.cpp
	$(CXX) -DDEBUGMAIN $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS) -o debugmain wwwdll.cpp -lws2_32 $(LIB_PATH)  $(LIBS)

windebug: wwwdll.h wwwdll.cpp wwwdlltest.cpp
	$(CXX) -DDEBUGWINMAIN $(CXXFLAGS) -mwindows $(INCLUDES) -I$(DEPENDS_DIR) -o debugwinmain wwwdll.cpp -lws2_32 $(LIB_PATH)  $(LIBS)


wwwdll.o: wwwdll.cpp
	$(CXX) -DNDEBUG -Wall $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -c wwwdll.cpp

wwwdll.dll: wwwdll.o wwwdll.def
	$(DLLWRAP) -k --def wwwdll.def --driver-name `cygpath -w $(CXX)`  -mthreads -o wwwdll.dll wwwdll.o -lws2_32 $(LIB_PATH)  $(LIBS)

libwwwdll.a: wwwdll.dll wwwdll.def
	$(DLLTOOL) -k --def wwwdll.def --dllname wwwdll.dll --output-lib libwwwdll.a

clean:
	rm -f wwwdll.dll wwwdll.o libwwwdll.a wwwdlltest.exe debugmain.exe debugwinmain.exe

strip:
	strip wwwdll.dll wwwdlltest.exe

wwwdlltest: wwwdlltest.cpp libwwwdll.a
	$(CXX) -g  -DDEBUGMAIN $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -Wall -o wwwdlltest.exe wwwdlltest.cpp -L. -lwwwdll $(LIB_PATH)  $(LIBS)

wwwdllwintest: libwwwdll.a
	$(CXX) -g  -DDEBUGWINMAIN $(CXXFLAGS) -mwindows $(INCLUDES) -I$(DEPENDS_DIR) -Wall -o wwwdlltest.exe wwwdlltest.cpp -L. -lwwwdll $(LIB_PATH)  $(LIBS)

depends:
	cp ../cpp_lib/net/HTTPClient.hpp ./depends/net/HTTPClient.hpp
	cp ../cpp_lib/net/HTTPProperty.hpp ./depends/net/HTTPProperty.hpp
	cp ../cpp_lib/net/HTTPResult.hpp ./depends/net/HTTPResult.hpp
	cp ../cpp_lib/net/NetService.hpp ./depends/net/NetService.hpp
	cp ../cpp_lib/net/URL.hpp ./depends/net/URL.hpp
	cp ../cpp_lib/Socket/ClientSocket.hpp ./depends/Socket/ClientSocket.hpp
	cp ../cpp_lib/Socket/IP.hpp ./depends/Socket/IP.hpp
	cp ../cpp_lib/Socket/NativeSocket.hpp ./depends/Socket/NativeSocket.hpp
	cp ../cpp_lib/Socket/ServerSocket.hpp ./depends/Socket/ServerSocket.hpp
	cp ../cpp_lib/Socket/Socket.hpp ./depends/Socket/Socket.hpp
	cp ../cpp_lib/Socket/SocketException.hpp ./depends/Socket/SocketException.hpp
	cp ../cpp_lib/text/LexicalCast.hpp ./depends/text/LexicalCast.hpp
	cp ../cpp_lib/text/regex/RegexCompile.hpp ./depends/text/regex/RegexCompile.hpp
	cp ../cpp_lib/Thread/CollectableThreadGroup.hpp ./depends/Thread/CollectableThreadGroup.hpp
	cp ../cpp_lib/Thread/CriticalSection.hpp ./depends/Thread/CriticalSection.hpp
	cp ../cpp_lib/Thread/Event.hpp ./depends/Thread/Event.hpp
	cp ../cpp_lib/Thread/Mutex.hpp ./depends/Thread/Mutex.hpp
	cp ../cpp_lib/Thread/Runnable.hpp ./depends/Thread/Runnable.hpp
	cp ../cpp_lib/Thread/Thread.hpp ./depends/Thread/Thread.hpp
	cp ../cpp_lib/Thread/ThreadException.hpp ./depends/Thread/ThreadException.hpp
	cp ../cpp_lib/Thread/ThreadGroup.hpp ./depends/Thread/ThreadGroup.hpp
	cp ../cpp_lib/Thread/ThreadPool.hpp ./depends/Thread/ThreadPool.hpp
	cp ../cpp_lib/util/CRC.hpp ./depends/util/CRC.hpp
	cp ../cpp_lib/util/hash/SHA1.hpp ./depends/util/hash/SHA1.hpp
	cp ../cpp_lib/util/Notification.hpp ./depends/util/Notification.hpp
	cp ../cpp_lib/util/Property.hpp ./depends/util/Property.hpp
	cp ../cpp_lib/util/Singleton.hpp ./depends/util/Singleton.hpp
	cp ../cpp_lib/util/SmartPointer.hpp ./depends/util/SmartPointer.hpp
	cp ../cpp_lib/WinThread/WinCriticalSection.hpp ./depends/WinThread/WinCriticalSection.hpp
	cp ../cpp_lib/WinThread/WinEvent.hpp ./depends/WinThread/WinEvent.hpp
	cp ../cpp_lib/WinThread/WinMutex.hpp ./depends/WinThread/WinMutex.hpp
	cp ../cpp_lib/WinThread/WinThread.hpp ./depends/WinThread/WinThread.hpp
