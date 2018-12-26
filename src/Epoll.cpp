
#include"Epoll.h"
#include"DealMessage.h"
const size_t Epoll::MAX_EVENT = 10000;
//#define SERVER_IP "123.207.83.157"
Epoll::Epoll(int port)
	:m_sockfd(-1)
	,m_port(port)
	,m_epfd(-1)
{}

DealMessage g_DealMsg;
map<int, Info> m_match;

Epoll::~Epoll()
{
	if(m_sockfd != -1)
	{
		close(m_sockfd);
	}
	if(m_epfd != -1)
	{
		close(m_epfd);
	}
}
/*********************************************************************
 * 函数功能：启动服务器
 * 函数输入：
 * 函数输出：
 ********************************************************************/ 
void Epoll::Start()
{
	//创建套接字
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(m_sockfd == -1)
	{
		ErrorLog("create socket");
		return;
	}
	
	SetNonBlocking(m_sockfd);
	//绑定socket
	int opt=1;
	setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_port);
	if(-1 == bind(m_sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		ErrorLog("bind sock");
		return ;
	}

	//监听socket
	if(-1 == listen(m_sockfd, 1000))
	{
		ErrorLog("listen sock");
		return;
	}

	//创建epoll
	m_epfd = epoll_create(MAX_EVENT);
	if(m_epfd == -1)
	{
		ErrorLog("epoll_create");
		return ;
	}

	OpEvent(m_sockfd, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
	 
	EpollLoop();
}

/*********************************************************************
 * 函数功能：将文件描述符设置为非阻塞
 * 函数输入：sfd 文件描述符
 * 函数输出：
 ********************************************************************/ 
void Epoll::SetNonBlocking(int sfd)
{
	int flags,s;
	flags = fcntl(sfd, F_GETFL, 0);
	if(flags == -1)
	{
		ErrorLog("F_GETFL");
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if(s == -1)
	{
		ErrorLog("F_SETFL");
	}
}

/*********************************************************************
 * 函数功能：注册要监听的事件
 * 函数输入：fd  文件描述符
 * 			 events  事件 EPOLLIN/EPOLLOUT...
 * 			 how 动作 EPOLL_CTL_ADD/MOD/DEL
 * 函数输出：
 ********************************************************************/ 
void Epoll::OpEvent(int fd, int events, int how)
{
	struct epoll_event event;
   	event.events = events;
	event.data.fd = fd;
	if(epoll_ctl(m_epfd, how, fd, &event) == -1)
	{
		ErrorLog("epoll_ctl.fd:%d+how:%d",fd, how);
	}
}

/*********************************************************************
 * 函数功能：epoll的处理
 * 函数输入：
 * 函数输出：
 ********************************************************************/ 
void Epoll::EpollLoop()
{
	TraceLog("EpollLoop");
	struct epoll_event events[MAX_EVENT];

	while(1)
	{
		int size = epoll_wait(m_epfd, events, MAX_EVENT, -1);//阻塞等待
		if(size < 0)
		{
			ErrorLog("epoll_wait");
		}
		for(int i = 0; i < size; ++i)
		{
			if(events[i].data.fd == m_sockfd)//新连接
			{
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);
				int client_fd = accept(m_sockfd, (struct sockaddr*)&client_addr,&len);
				if(client_fd == -1)
				{
					ErrorLog("accept");
				}
				TraceLog("client connect");
				ConnectEventHandle(client_fd);
			}
			else if(events[i].events & EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读事件
			{
				ReadEventHandle(events[i].data.fd);
			}
			else if(events[i].events & EPOLLOUT)// 进行写事件
			{
				WriteEventHandle(events[i].data.fd);
			}
			else
			{
				ErrorLog("error event");
			}
		}
	}
}

/*********************************************************************
 * 函数功能：连接事件的处理
 * 函数输入：fd 连接的描述符
 * 函数输出：
 ********************************************************************/ 
void Epoll::ConnectEventHandle(int fd)
{
	OpEvent(fd, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
    SetNonBlocking(fd);	
}



/*********************************************************************
 * 函数功能：读事件的处理
 * 函数输入：fd 文件描述符
 * 函数输出：
 ********************************************************************/ 
void Epoll::ReadEventHandle(int fd)
{
  string str;
  //char  buf[1024]={0};
	int read_size = g_DealMsg.NoBlockRead(fd, str);
	if(read_size < 0)
	{
		ErrorLog("read");
	}	
	else if(read_size == 0)
	{
		close(fd);
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
		map<int, Info>::iterator it;
		it = m_match.find(fd);
		if(it != m_match.end())
		{
			m_match.erase(it);
		}
	    TraceLog("client say: goodbye");
		return;
	}

	TraceLog("client say:%s", str.c_str());
  g_DealMsg.ParseMessage(fd, str);
	// 
	//send(fd, buf, sizeof(buf), 0);
	OpEvent(fd, EPOLLOUT | EPOLLET,  EPOLL_CTL_MOD);
}


/*********************************************************************
 * 函数功能：写事件的处理
 * 函数输入：
 * 函数输出：
 ********************************************************************/ 
void Epoll::WriteEventHandle(int fd)
{
	//ParseMessage(fd);	
	//send(fd, buf, sizeof(buf), 0);
	OpEvent(fd, EPOLLIN | EPOLLET,  EPOLL_CTL_MOD);
}



int main()
{
	Epoll s(8080);
	s.Start();
	return 0;
}
