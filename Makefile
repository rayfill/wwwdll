CXX=/cygdrive/c/MinGW/bin/g++
#CXX=g++
DLLWRAP=/cygdrive/c/MinGW/bin/dllwrap
DLLTOOL=/cygdrive/c/MinGW/bin/dlltool
.PHONY: strip clean depends

test: wwwdll.dll wwwdlltest
	./wwwdlltest.exe 300 http://drag11.sakura.ne.jp/check/

debug: wwwdll.h wwwdll.cpp wwwdlltest.cpp
	$(CXX) -DDEBUGMAIN -g -Wall -I. -Idepends -o debugmain wwwdll.cpp -lwsock32

windebug: wwwdll.h wwwdll.cpp wwwdlltest.cpp
	$(CXX) -DDEBUGWINMAIN -D_REENT -D_WIN32 -D__USE_W32_SOCKETS -mthreads -mwindows -g -Wall -I. -Idepends -o debugwinmain wwwdll.cpp -lwsock32


wwwdll.o: wwwdll.cpp
	$(CXX) -DNDEBUG -Wall -mthreads -I. -Idepends -c wwwdll.cpp

wwwdll.dll: wwwdll.o wwwdll.def
	$(DLLWRAP) -k --def wwwdll.def --driver-name `cygpath -w $(CXX)` -mthreads -o wwwdll.dll wwwdll.o -lwsock32

libwwwdll.a: wwwdll.dll wwwdll.def
	$(DLLTOOL) -k --def wwwdll.def --dllname wwwdll.dll --output-lib libwwwdll.a

clean:
	rm -f wwwdll.dll wwwdll.o libwwwdll.a wwwdlltest.exe debugmain.exe debugwinmain.exe

strip:
	strip wwwdll.dll wwwdlltest.exe

wwwdlltest: wwwdlltest.cpp libwwwdll.a
	$(CXX) -g  -DDEBUGMAIN -I. -Idepends -Wall -o wwwdlltest.exe wwwdlltest.cpp -L. -lwwwdll

wwwdllwintest: libwwwdll.a
	$(CXX) -g  -DDEBUGWINMAIN -mthreads -mwindows -I. -Idepends  -Wall -o wwwdlltest.exe wwwdlltest.cpp -L. -lwwwdll

depends:
	cp ../new_lib/net/HTTPClient.hpp ./depends/net/HTTPClient.hpp
	cp ../new_lib/net/HTTPProperty.hpp ./depends/net/HTTPProperty.hpp
	cp ../new_lib/net/HTTPResult.hpp ./depends/net/HTTPResult.hpp
	cp ../new_lib/net/NetService.hpp ./depends/net/NetService.hpp
	cp ../new_lib/net/URL.hpp ./depends/net/URL.hpp
	cp ../new_lib/Socket/ClientSocket.hpp ./depends/Socket/ClientSocket.hpp
	cp ../new_lib/Socket/IP.hpp ./depends/Socket/IP.hpp
	cp ../new_lib/Socket/NativeSocket.hpp ./depends/Socket/NativeSocket.hpp
	cp ../new_lib/Socket/ServerSocket.hpp ./depends/Socket/ServerSocket.hpp
	cp ../new_lib/Socket/Socket.hpp ./depends/Socket/Socket.hpp
	cp ../new_lib/Socket/SocketException.hpp ./depends/Socket/SocketException.hpp
	cp ../new_lib/text/LexicalCast.hpp ./depends/text/LexicalCast.hpp
	cp ../new_lib/text/regex/RegexCompile.hpp ./depends/text/regex/RegexCompile.hpp
	cp ../new_lib/Thread/CollectableThreadGroup.hpp ./depends/Thread/CollectableThreadGroup.hpp
	cp ../new_lib/Thread/CriticalSection.hpp ./depends/Thread/CriticalSection.hpp
	cp ../new_lib/Thread/Event.hpp ./depends/Thread/Event.hpp
	cp ../new_lib/Thread/Mutex.hpp ./depends/Thread/Mutex.hpp
	cp ../new_lib/Thread/Runnable.hpp ./depends/Thread/Runnable.hpp
	cp ../new_lib/Thread/Thread.hpp ./depends/Thread/Thread.hpp
	cp ../new_lib/Thread/ThreadException.hpp ./depends/Thread/ThreadException.hpp
	cp ../new_lib/Thread/ThreadGroup.hpp ./depends/Thread/ThreadGroup.hpp
	cp ../new_lib/Thread/ThreadPool.hpp ./depends/Thread/ThreadPool.hpp
	cp ../new_lib/util/CRC.hpp ./depends/util/CRC.hpp
	cp ../new_lib/util/hash/SHA1.hpp ./depends/util/hash/SHA1.hpp
	cp ../new_lib/util/Notification.hpp ./depends/util/Notification.hpp
	cp ../new_lib/util/Property.hpp ./depends/util/Property.hpp
	cp ../new_lib/util/Singleton.hpp ./depends/util/Singleton.hpp
	cp ../new_lib/util/SmartPointer.hpp ./depends/util/SmartPointer.hpp
	cp ../new_lib/WinThread/WinCriticalSection.hpp ./depends/WinThread/WinCriticalSection.hpp
	cp ../new_lib/WinThread/WinEvent.hpp ./depends/WinThread/WinEvent.hpp
	cp ../new_lib/WinThread/WinMutex.hpp ./depends/WinThread/WinMutex.hpp
	cp ../new_lib/WinThread/WinThread.hpp ./depends/WinThread/WinThread.hpp
