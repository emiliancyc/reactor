#include "Log.hpp"

std::string DateTime()
{
	time_t t = time(0);
	std::string tmp = ctime(&t);
	size_t found = tmp.find_last_of('\n');
	return (found == std::string::npos) ? tmp : tmp.substr(0, found-1);
}

