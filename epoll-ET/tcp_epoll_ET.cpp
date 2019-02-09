

#include "Socket.hpp"
#include <functional>
#include <vector>
#include <sys/epoll.h>

typedef std::function<void ()> Handler;

class Epoll
{
  public:
    Epoll()
    {
      _epoll_fd = epoll_create(10);
    }

    ~Epoll()
    {
      close(_epoll_fd);
    }

    bool Add(Socket& sock)
    {
      int fd = sock.GetSock();
      epoll_event ev;
      ev.data.fd = fd;
      ev.events = EPOLLIN;
      int ret = epoll_ctl(_epoll_fd,EPOLL_CTL_ADD,fd,&ev);
      if(ret < 0)
      {
        std::cerr << "epoll_ctl error" << std::endl;
        return false;
      }
      return true;
    }

    bool Delete(Socket& sock)
    {
      int fd = sock.GetSock();
      int ret = epoll_ctl(_epoll_fd,EPOLL_CTL_DEL,fd,NULL);
      if(ret < 0)
      {
        std::cout << "epoll_ctl error" << std::endl;
        return false;
      }
      return true;
    }

    bool Wait(std::vector<Socket>* output)
    {
      output->clear();
      epoll_event events[1024];
      int nfds = epoll_wait(_epoll_fd,events,sizeof(events)/sizeof(events[0]),-1);
      if(nfds < 0)
      {
        std::cerr << "epoll_wait error" << std::endl;
        return false;
      }
      for(int i = 0; i< nfds; i++)
      {
        Socket sock(events[i].data.fd);
        output->push_back(sock);
      }
      return true;
    }
  private:
    int _epoll_fd;
};

class EpollServer 
{
  public:
    EpollServer(std::string& ip,int port):_ip(ip),_port(port)
    {}

    bool Start(Handler handler)
    {
      Socket listen_sock;
      listen_sock.Sock();
      listen_sock.Bind(_ip,_port);
      listen_sock.Listen(5);

      Epoll _epoll;
      _epoll.Add(listen_sock);

      for(;;)
      {
        std::vector<Socket> output;
        if(!_epoll.Wait(&output))
          continue;

        for(size_t i = 0; i<output.size(); i++)
        {
          int ret = output[i].GetSock();
          if(ret == listen_sock.GetSock())
          {
            Socket newsock;
            listen_sock.Accept(&newsock);
            _epoll.Add(newsock);
          }
          else 
          {
            std::string req,resp;
            bool ret = output[i].Recv(&req);
            if(!ret)
            {
              _epoll.Delete(output[i]);
              output[i].Close();
              continue;
            }
            handler();
            output[i].Send(resp);
          }
        }
      }
      return true;
    }

  private:
      std::string _ip;
      int _port;
};
