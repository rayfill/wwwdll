#include "wwwdll.h"
#include <Thread/Runnable.hpp>
#include <iostream>

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

int main()
{
	void* threadContext = ThreadCreate();
	
	RunningTarget target;
	target.count = 0;
	for (int i = 0; i < 10; ++i)
	{
		++target.count;
		ThreadStart(threadContext, &target, NULL, 0);
		ThreadJoin(threadContext);
	}

	ThreadClose(threadContext);

	return 0;
}
