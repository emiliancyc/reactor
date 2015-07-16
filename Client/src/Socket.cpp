#include "Socket.hpp"
#include "Log.hpp"

#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sstream>
#include <stdexcept>
#include <boost/scoped_array.hpp>
									// 1kB    1MB    1GB    4GB
									// 1024 * 1024 * 1024 * 4
const size_t Socket::MAX_FRAME_LENGTH = 4294967296; //4GB

const int Socket::Address::m_domain = AF_INET;

Socket::Address::Address(int p_Port)
{
	struct sockaddr_in addr;
	::memset(&addr, 0, sizeof addr);

	addr.sin_family = m_domain;
	addr.sin_port = htons(p_Port);
	addr.sin_addr.s_addr = INADDR_ANY;

	::memcpy(&m_address, &addr, sizeof addr);
	m_addressLength = sizeof addr;
}

Socket::Address::Address(const std::string& p_Ip, int p_Port)
{
	if (p_Port < 0 || p_Port > 0xffff) {
		std::stringstream ss;
		ss << "Given port: " << p_Port << " is out of range. Expected range: <0; 65535>.";
		throw runtime_error(ss.str());
	}

	sockaddr_in addr;
	::memset(&addr, 0, sizeof addr);

	addr.sin_family = m_domain;
	addr.sin_port = htons(p_Port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (p_Ip.empty() || !inet_aton(p_Ip.c_str(), &addr.sin_addr)) {
		std::stringstream ss;
		ss << "Given IP address: \"" << p_Ip << "\" is not valid";
		throw runtime_error(ss.str());
	}

	::memcpy(&m_address, &addr, sizeof addr);
	m_addressLength = sizeof addr;
}

Socket::Address::Address(const Address& p_Other) : m_addressLength(p_Other.m_addressLength)
{
	::memcpy(&m_address, &p_Other.m_address, p_Other.m_addressLength);
}

Socket::Address& Socket::Address::operator=(const Address& p_Other)
{
	Socket::Address& result = *this;
	if (this != &p_Other) {
		::memcpy(&m_address, &p_Other.m_address, p_Other.m_addressLength);
		m_addressLength = p_Other.m_addressLength;
	}

	return result;
}

int Socket::Address::Port() const
{
	const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(&m_address);
	return ntohs(addr->sin_port);
}

int Socket::Address::Domain()
{
	return m_domain;
}

Socket::Socket(Type p_Type): m_type(p_Type), m_fd(-1), m_address()
{
	int rawType = -1;
	switch (m_type) {
		case TCP:
			rawType = SOCK_STREAM;
			break;
		case UDP:
			rawType = SOCK_DGRAM;
			break;
		default:
			std::stringstream ss;
			ss << "Socket creation failed: not supported type";
			throw std::runtime_error(ss.str());
	}

	m_fd = ::socket(Address::Domain(), rawType, 0);
	if (m_fd < 0) {
		std::stringstream ss;
		ss << "Socket creation failed: " << strerror(errno);
		throw std::runtime_error(ss.str());
	}

	std::cout << "Otwarto: " << m_fd << std::endl;
}

Socket::~Socket()
{
	std::cout << "Do zamkniecia: " << m_fd << std::endl;
	::close(m_fd);
}

Socket::Socket(Type p_Type, Descriptor p_Fd, Address::Ptr p_Address) : m_type(p_Type), m_fd(p_Fd), m_address(p_Address)
{
	if ( (p_Type != TCP) && (p_Type != UDP) ) {
		std::stringstream ss;
		ss << "Socket creation failed: not supported type";
		throw std::runtime_error(ss.str());
	}

	std::cout << "Utworzono: " << m_fd << std::endl;
}

bool Socket::operator==(Descriptor p_Descriptor)
{
	return (m_fd == p_Descriptor);
}

void Socket::SetNonBlocking()
{
	int flags = fcntl(m_fd, F_GETFL, 0);
	fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
}

Socket::Descriptor Socket::GetDescriptor() const
{
	return m_fd;
}

void Socket::Bind(int p_Port)
{
	m_address.reset(new Socket::Address(p_Port));

	if (::bind(m_fd, rawAddress(m_address), rawAddressLength(m_address)) < 0) {
		std::stringstream ss;
		ss << "Bind socket failed: " << strerror(errno);
		throw runtime_error(ss.str());
	}
}

void Socket::Listen()
{
	if (::listen(m_fd, 1) != 0) {
		std::stringstream ss;
		ss << "Listen socket failed: " << strerror(errno);
		throw runtime_error(ss.str());
	}
}

void Socket::Accept(Socket::Ptr& p_Socket)
{
	Address::Ptr incomingAddress(new Address());
	Descriptor incomingFd = ::accept( m_fd
									, Socket::rawAddress(incomingAddress)
									, Socket::rawAddressLengthPtr(incomingAddress));
	if (incomingFd < 0) {
		std::stringstream ss;
		ss << "Accept socket failed: " << strerror(errno);
		throw runtime_error(ss.str());
	}

	p_Socket = Socket::create(m_type, incomingFd, incomingAddress);
}

void Socket::Connect(const std::string& p_Ip, int p_Port)
{
	m_address.reset(new Socket::Address(p_Ip, p_Port));

	if (::connect(m_fd, rawAddress(m_address), rawAddressLength(m_address)) < 0) {
		std::stringstream ss;
		ss << "Cannot connect address: " << p_Ip << ":" << p_Port << " " << strerror(errno);
		throw runtime_error(ss.str());
	}
}

void Socket::Read(std::string& p_Data)
{
	size_t totalToRead = 0;
	if (::read(m_fd, &totalToRead, sizeof(size_t)) < 0) {
		std::stringstream ss;
		ss << "Socket read failed: " << strerror(errno);
		throw std::runtime_error(ss.str());
	}

	//TRACE_LOG << "Allock reading buffer" << std::endl;
	scoped_array<char> text(new char[totalToRead]);
	//TRACE_LOG << "Zeros reading buffer" << std::endl;
	//::memset(text.get(), 0, (totalToRead-1)*sizeof(char));

	size_t stillToRead = totalToRead;
	while (stillToRead > 0) {
		//TRACE_LOG << "going to read total: " << totalToRead << ", still to read: " << stillToRead << std::endl;
		ssize_t ret = ::read(m_fd, text.get(), stillToRead);
		if (ret > 0) {
			p_Data.append(text.get(), ret);
			//for (int i = 0; i < ret; ++i)
			//	std::cout << "ascii: " << (int)text.get()[i] << std::endl;
			//std::cout << "red: " << ret << " data: \"" << p_Data  << "\"" << std::endl;
			//::memset(text.get(), 0, (ret-1)*sizeof(char));
			stillToRead -= ret;
		} else if (ret == 0) {
			//p_ErrorMessage = "Eof receiveed - socket closed";
			break;
		} else {
			std::stringstream ss;
			ss << "Socket read failed: " << strerror(errno);
			throw std::runtime_error(ss.str());
		}
	} //while(1)
	TRACE_LOG << "red: " << totalToRead
			  //<< " data: \"" << p_Data  << "\""
			  << " real len: " << p_Data.length()
			  << std::endl;
}

void Socket::Write(const std::string& p_Data)
{
	size_t toWrite = p_Data.length();

	if (toWrite > MAX_FRAME_LENGTH) {
		std::stringstream ss;
		ss << "Actuall message length ( " << toWrite << ") exceeds the maximal one (" << MAX_FRAME_LENGTH << ")";
		throw std::runtime_error(ss.str());
	}

	if (::write(m_fd, &toWrite, sizeof(size_t)) < 0) {
		std::stringstream ss;
		ss << "Socket write failed: " << strerror(errno);
		throw std::runtime_error(ss.str());
	}

	if (::write(m_fd, p_Data.c_str(), toWrite) < 0) {
		std::stringstream ss;
		ss << "Socket write failed: " << strerror(errno);
		throw std::runtime_error(ss.str());
	}

	TRACE_LOG << "Written: " << toWrite << " real len: " << p_Data.length() << std::endl;
}

