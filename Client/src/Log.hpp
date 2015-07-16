#ifndef EPOLLLIB_LOG_HPP
#define EPOLLLIB_LOG_HPP

#include <unistd.h> //TODO remove
#include <iostream> //TODO remove

std::string DateTime();

#define TRACE_LOG std::cout << DateTime() << " TYPE: TRACE" << " PID: " << getpid() << " TID: " << pthread_self() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " "
#define ERROR_LOG std::cout << DateTime() << " TYPE: ERROR" << " PID: " << getpid() << " TID: " << pthread_self() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << " "

#endif //EPOLLLIB_LOG_HPP

