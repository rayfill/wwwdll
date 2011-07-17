CXX=g++
#CXX=g++
DLLWRAP=dllwrap
DLLTOOL=dlltool
DEPENDS_DIR=../cpplib
#CXXFLAGS +=-g -Wall -DWINNT_VER=0x0501 -D_WIN32_WINNT=0x0501 -D_REENTRANT -mthreads
CXXFLAGS +=-g -Wall -D_WIN32_WINNT=0x0501 -D_REENTRANT 
LIBS += -lz

.PHONY: strip clean depends

test: wwwdll.dll wwwdlltest
	./wwwdlltest.exe 300 http://www.goo.ne.jp/

filterTest: FilterTest.cpp Filter.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -o filterTest FilterTest.cpp $(LIBS)

debug: wwwdll.h wwwdll.cpp wwwdlltest.cpp
	$(CXX) -DDEBUGMAIN $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -o debugmain wwwdll.cpp wwwdlltest.cpp -lws2_32 $(LIB_PATH)  $(LIBS)

windebug: wwwdll.h wwwdll.cpp wwwdlltest.cpp
	$(CXX) -DDEBUGWINMAIN $(CXXFLAGS) -mwindows $(INCLUDES) -I$(DEPENDS_DIR) -o debugwinmain wwwdll.cpp wwwdlltest.cpp -lws2_32 $(LIB_PATH)  $(LIBS)


wwwdll.o: wwwdll.cpp
	$(CXX) -DNDEBUG -Wall $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -c wwwdll.cpp

wwwdll.dll: wwwdll.o wwwdll.def
	$(DLLWRAP) -k --def wwwdll.def --driver-name $(CXX)  -mthreads -o wwwdll.dll wwwdll.o -lws2_32 $(LIB_PATH)  $(LIBS)

libwwwdll.a: wwwdll.dll wwwdll.def
	$(DLLTOOL) -k --def wwwdll.def --dllname wwwdll.dll --output-lib libwwwdll.a

clean:
	rm -f wwwdll.dll wwwdll.o libwwwdll.a wwwdlltest.exe debugmain.exe debugwinmain.exe

strip:
	strip wwwdll.dll wwwdlltest.exe

wwwdlltest: wwwdlltest.cpp libwwwdll.a
	$(CXX) -g  -DDEBUGMAIN $(CXXFLAGS) $(INCLUDES) -I$(DEPENDS_DIR) -Wall -o wwwdlltest.exe wwwdlltest.cpp -L. -lwwwdll $(LIB_PATH)  $(LIBS)

wwwdllwintest: libwwwdll.a
	$(CXX) -g  -DDEBUGWINMAIN $(CXXFLAGS) -mwindows $(INCLUDES) -I$(DEPENDS_DIR) -Wall --input-charset=UTF-8 --exec-charset=cp932 -o wwwdlltest.exe wwwdlltest.cpp -L. -lwwwdll $(LIB_PATH)  $(LIBS)


