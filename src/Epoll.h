
#ifndef EPOLL_H
#define EPOLL_H

#include"Common.h"


class Epoll
{
	public:
		Epoll(int port);
	  ~Epoll();

		void Start();
		void SetNonBlocking(int fd);
		void OpEvent(int fd, int events, int how);
		void EpollLoop();
		void ConnectEventHandle(int fd);
		void ReadEventHandle(int fd);
		void WriteEventHandle(int fd);
	private:
		int	m_sockfd;//sock文件描述符
		int m_port;  //端口号
		int m_epfd;  //epoll文件描述符
		static const size_t MAX_EVENT;//最大的连接数量
};
#endif //EPOLL_H
