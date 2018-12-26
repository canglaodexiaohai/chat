#ifndef _DEAL_MASSAGE_H
#define _DEAL_MASSAGE_H

#include"Common.h"
class DealMessage
{
  public:
    ssize_t NoBlockRead(int fd, string &str);
    void ParseMessage(int fd, string &str);
  private:
    ssize_t readn(int fd, void *vptr, size_t n);
    ssize_t write(int fd, void *vptr, size_t n);
    int charset_convert(const char *from_charset,const char *to_charset, char *in_buf, size_t in_length, char *out_buf, size_t out_length); 
    void DealName(int fd,  string &str);
    void DealSingle(int fd,  string &str);
    void DealAll(int fd,  string &str);
    void OnLineName(int fd, int group);
    void SendAllMessage(string &str);
    void NoBlockSend(int fd, string &str);
};

#endif  //_DEAL_MESSAGE_H
