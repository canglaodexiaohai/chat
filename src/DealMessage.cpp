#include"DealMessage.h"

ssize_t DealMessage::readn(int fd, void *vptr, size_t n)
{
  size_t nleft = n;
  char *ptr =(char *)vptr;
  size_t nread = 0;
  while(nleft > 0)
  {
    nread = read(fd, ptr, nleft);
    if(nread < 0)
    {
      if(errno == EINTR)
      {
        nread = 0;
      }
      else
      {
        return -1;
      }
    }
    else if(nread == 0)
    {
      break;
    }
    nleft -= nread;
    ptr += nread;
  }
  return n-nleft;
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
  size_t nleft = n;
  ssize_t nwritten = 0;
  const char *ptr =(const char *)vptr;
  while(nleft > 0)
  {
    nwritten = write(fd, ptr, nleft);
    if(nwritten <= 0)
    {
      if(nwritten < 0 && errno ==EINTR)
      {
        nwritten = 0;
      }
      else 
      {
        return -1;
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}
/*********************************************************
 * 函数功能：进行非阻塞的读
 * 函数输入：fd 文件描述符
 *           buf 输出型参数，存储读到的内容
 * 函数输出：读取的字节数
 ********************************************************/ 
ssize_t DealMessage::NoBlockRead(int fd, string &str)
{
  ssize_t total_size =0;
  char buf[BUFSIZE]={0};
  Packet readbuf;
  //memset(&readbuf,0,sizeof(readbuf));
  ssize_t ret = readn(fd, &readbuf.n_msgLen, 4);
  if(ret <= 0)
  {
    return ret;
  }
  int n_dataBytes = readbuf.n_msgLen;

  while(0 < n_dataBytes)
  {
    bzero(buf,sizeof(buf));  
    ssize_t cur_size = 0;
    if(n_dataBytes > BUFSIZE)
    {
      cur_size = readn(fd, &readbuf.msg,BUFSIZE);
      total_size += cur_size;
      n_dataBytes -= 1024;
    }
    else 
    {
      cur_size = readn(fd,&readbuf.msg,n_dataBytes);
      total_size += cur_size;
      n_dataBytes = 0;
    }
    if(cur_size > 0)
    {
      char *to_str_utf8 = NULL;
      int len = 0;
      strcpy(buf,readbuf.msg);
      to_str_utf8 = (char*)calloc(1,strlen(buf)*3);
      len = charset_convert("GB2312", "UTF-8", buf, strlen(buf), to_str_utf8,strlen(buf)*3);
      string tmp(to_str_utf8);
      str += tmp;
      free(to_str_utf8);
    }
    if(cur_size < BUFSIZE || errno == EAGAIN)
    {
      break;
    }
    
  }
  //buf[total_size] = '\0';
  
  return total_size;

}

int DealMessage::charset_convert(const char *from_charset, const char *to_charset,char *in_buf, size_t in_length, char *out_buf, size_t out_length)
{
  iconv_t icd = 0;
  char *pin = in_buf;
  char *pout = out_buf;
  size_t out_size = out_length;
  icd = iconv_open(to_charset, from_charset);
  if(icd ==(iconv_t)-1)
  {
    return -1;
  }
  if((size_t)-1 == iconv(icd, &pin, &in_length, &pout, &out_length))
  {
    return -2;
    iconv_close(icd);
  }
  out_buf[out_size - out_length] = '\0';
  iconv_close(icd);
  return (int)out_size - out_length;
}


void DealMessage::ParseMessage(int fd, string &str)
{
  string::size_type i;//用来接收find的返回值
  i = str.find("name+");
	if((i != string::npos) && (i == 0))
	{
		DealName(fd, str);
		return;
	}

	i = str.find("single+");
	if((i != string::npos) && (i == 0))
	{
		DealSingle(fd, str);
		return;
	}

	i = str.find("all+");
	if((i != string::npos) && (i == 0))
	{
		DealAll(fd, str);
		return;
	}

	i = str.find("online_name");
	if((i != string::npos) && (i == 0))
	{
		int group = 0;
		i = str.find("+group");
		if((i != string::npos) && (i == 11))
		{
			group = 1;
		}	
		OnLineName(fd,group);
		return;
	}
}

void DealMessage::NoBlockSend(int fd, string &str)
{
  /*ssize_t total_size = 0;
  while(1)
  {
    ssize_t cur_size = 0;
    cur_size = send(fd, str.c_str() + total_size, str.length()+1, 0);
    total_size += cur_size;
    if((unsigned long)total_size == str.length() + 1)
    {
      break;
    }
  }*/
  Packet writebuf;
  int n_len = str.length()+1;
  writebuf.n_msgLen=n_len;
  if(n_len > BUFSIZE)
  {
    strncpy(writebuf.msg , str.c_str(),BUFSIZE);
    writen(fd, &writebuf,  BUFSIZE+4);
    n_len -= BUFSIZE;
    writen(fd, str.c_str()+BUFSIZE, n_len);
  }
  else 
  {
    strcpy(writebuf.msg, str.c_str());
    writen(fd,&writebuf,n_len+4);
  }
}

void DealMessage::DealName(int fd, string &str)
{
  str = str.c_str() + 5;
  Info info;
  info.name = str;
  info.flag = 0;
  m_match.insert(make_pair(fd, info));
  string message = "欢迎" + str + "加入聊天室";
  NoBlockSend(fd, message);
  TraceLog("向%s发送消息：%s",m_match[fd].name.c_str(),message.c_str());
}

void DealMessage::DealSingle(int fd, string &str)
{
  str = str.c_str() + 7;
  string::size_type i = 0;
  i = str.find("+");
  string recv_name = "";
  string send_name = m_match[fd].name;
  string send_message = send_name + ":";
  int recv_fd = 0;

  recv_name = str.substr(0, i);
  map<int ,Info>::iterator it = m_match.begin();
  while(it != m_match.end())
  {
    if(it->second.name == recv_name)
    {
      recv_fd = it->first;
    }
    ++it;
  }
  send_message += str.c_str() + i +1;
  NoBlockSend(recv_fd, send_message);
  TraceLog("%s向%s发送消息：%s",send_name.c_str(), recv_name.c_str(), send_message.c_str());
}

void DealMessage::DealAll(int fd, string &str)
{
	string::size_type i = 0;
	i = str.find("all+join");
	if((i != string::npos) && (str.length() == 8))//"all+join\n"
	{
		m_match[fd].flag = 1;
		//发送加入群聊
		string message = "欢迎" + m_match[fd].name + "加入群聊！！！";
		SendAllMessage(message);
		return;
	}

	i = str.find("all+quit");
	if((i != string::npos) && (str.length() == 8))
	{
		m_match[fd].flag = 0;
		//发送离开群聊
		string message = m_match[fd].name + "离开群聊！！！";
		SendAllMessage(message);
		return;
	}
	
	str = str.c_str()+4;
	SendAllMessage(str);
}

void DealMessage::SendAllMessage(string &str)
{
	map<int, Info>::iterator it = m_match.begin();
	int curfd = 0;
	while(it != m_match.end())
	{
		if(it->second.flag == 1)
		{
		  curfd = it->first;
      NoBlockSend(curfd, str);
			TraceLog("向%s发送消息：%s", m_match[curfd].name.c_str(), str.c_str());
		}
		++it;
	}
	return ;
}

void DealMessage::OnLineName(int fd, int group)
{
	string str_name = "在线人员名单：\n";

	map<int, Info>::iterator it = m_match.begin();
	while(it != m_match.end())
	{
		if(group == 1)
		{
			if( it->second.flag == 1)
			{
				str_name = str_name + it->second.name + "$#$";
			}
		}
		else
		{
			str_name = str_name + it->second.name + "$#$";
		}
		++it;
	}
	NoBlockSend(fd, str_name);
  TraceLog("向%s发送消息：%s",m_match[fd].name.c_str(), str_name.c_str());	
}

