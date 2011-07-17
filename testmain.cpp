#include "wwwdll.cpp"

const char* userAgent = "Mozilla/5.0 (Windows; U; Windows NT 5.1; ja; rv:1.9.2.2) Gecko/20100316 Firefox/3.6.2";

#include <iostream>

int main()
{
	SocketModule module;
	
	HTTPContext<AfterNotify>* target =
		new HTTPContext<AfterNotify>(
									 "http://yu65026.blog53.fc2.com/?xml",
									 "",
									 userAgent,
									 1000,
									 AfterNotify());

	//target->setProxy("localhost", 6000);
	//target->setProxy("proxy.nttdata.co.jp", 8080);
	//target->setProxyUsername("U034059");
	//target->setProxyPassword("Okazaki5");
	target->run();
	std::cout << "status: " << target->getResponseCode() << std::endl;
	std::cout << "headers: " << target->getResponseHeaders() << std::endl;
	std::cout << "body: " << std::endl;

	std::string result = target->getContentsString();
	std::cout << result << std::endl;
	return 0;
}
