#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>

#include <sys/types.h>
#include <sys/socket.h>

#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace std;

class Socket {
	public:
		static const size_t MAX_FRAME_LENGTH;
		typedef shared_ptr<Socket> Ptr;
		typedef int Descriptor;

		enum Type {
			TCP,
			UDP
		};

	public:
		class Address {
			public:
				typedef shared_ptr<Address> Ptr;

			public:
				explicit Address(int p_Port=0);
				Address(const string& p_Ip, int p_Port);
				Address(const Address& p_Other);
				Address& operator=(const Address& p_Other);

			public:
				int Port() const;
				static int Domain();

			private:
				friend class Socket;
				static const int m_domain;
				struct sockaddr m_address;
				socklen_t m_addressLength;
		}; //class Socket::Address

	public:
		explicit Socket(Type p_Type = TCP);
		virtual ~Socket();
		bool operator==(Descriptor p_Descriptor);

	private:
		Socket(const Socket&);
		Socket& operator=(const Socket&);

	public:
		void SetNonBlocking();
		Descriptor GetDescriptor() const;
		void Bind(int p_Port);
		void Listen();
		void Accept(Socket::Ptr& p_Socket);
		void Connect(const string& p_Ip, int p_Port);
		void Write(const string& p_Data);
		void Read(string& p_Data);

	protected:
		static inline Ptr create(Type p_Type, Descriptor p_Fd, Address::Ptr p_Address) { //Used just by server socket
			return Ptr(new Socket(p_Type, p_Fd, p_Address));
		}

		static inline sockaddr* rawAddress(Address::Ptr p_Address) {
			return p_Address ? &p_Address->m_address : 0;
		}
		static inline socklen_t rawAddressLength(Address::Ptr p_Address) {
			return p_Address ? p_Address->m_addressLength : 0;
		}
		static inline socklen_t* rawAddressLengthPtr(Address::Ptr p_Address) {
			return p_Address ? &p_Address->m_addressLength : 0;
		}

	private:
		Socket(Type p_Type, Descriptor p_Fd, Address::Ptr p_Address);

	protected:
		Type m_type;
		Descriptor m_fd;
		Address::Ptr m_address;
}; //class Socket

#endif //SOCKET_HPP

