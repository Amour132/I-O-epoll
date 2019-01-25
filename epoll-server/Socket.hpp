#pragma once 

#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
using std::cout;
using std::endl; 

class Socket
{
  public:
    Socket(int sock):_sock(sock)
    {}

    Socket():_sock(-1)
    {}

    bool Sock()
    {
      _sock = socket(AF_INET,SOCK_STREAM,0);
      if(_sock < 0)
      {
        std::cerr << "socket error" << std::endl;
        return false;
      }
      return true;
    }

    bool Bind(std::string& ip,int port)
    {
      sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(ip.c_str());
      addr.sin_port = htons(port);
      int ret = bind(_sock,(sockaddr*)&addr,sizeof(addr));
      if(ret < 0)
      {
        std::cerr << "Bind error" << std::endl;
        return false;
      }
      return true;
    }

    bool Listen(int num)
    {
      int ret = listen(_sock,num); 
      if(ret < 0)
      {
        std::cerr << "listen error" << std::endl;
        return false;
      }
      return true;
    }

    bool Accept(Socket* peer,std::string* ip = NULL,int* port = NULL)
    {
      sockaddr_in peer_addr;
      socklen_t len = sizeof(peer_addr);
      int new_sock = accept(_sock,(sockaddr*)&peer_addr,&len);
      if(new_sock < 0){
        std::cerr << "accept error" << std::endl;
        return false;
      }
      peer->_sock = new_sock;
      if(ip != NULL)
      {
        *ip = inet_ntoa(peer_addr.sin_addr);
      }
      if(port != NULL)
      {
        *port = ntohs(peer_addr.sin_port);
      }
      return true;
    }

    bool Recv(std::string* buf)
    {
      buf->clear();
      char tmp[1024] = {0};
      ssize_t read_size = recv(_sock,tmp,sizeof(tmp),0);
      if(read_size < 0)
      {
        std::cerr << "recv error" << std::endl;
        return false;
      }
      else if(read_size == 0)
      {
        return false;
      }
      else 
      {
        buf->assign(tmp,read_size);
        return true;
      }
    }

    bool Send(const std::string& buf)
    {
      ssize_t write_size = send(_sock,buf.data(),buf.size(),0);
      if(write_size < 0)
      {
        std::cerr << "send error" << std::endl;
        return false;
      }
      return true;
    }

    bool Connect(const std::string& ip,int port)
    {
      sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(ip.c_str());
      addr.sin_port = htons(port);
      int ret = connect(_sock,(sockaddr*)&addr,sizeof(addr));
      if(ret < 0)
      {
        std::cerr << "connect error" << std::endl;
        return false;
      }
      return true;
    }

    int GetSock()
    {
      return _sock;
    }
    
    void Close()
    {
      close(_sock);
    }
    ~Socket()
    {
      Close();
    }
  private:
    int _sock = -1;
};
