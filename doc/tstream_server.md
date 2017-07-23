
tstream::sever代表了一个用于监听端口的socket

## 构造函数

* server(unsigned short port) 构造时监听端口，在所有的网卡上
* server(const char *addr, unsigned short port) 构造时监听端口，在指定网卡上

## 成员函数

* tstream accept()
  - 功能: 接受一个连接，返回TCP流
* bool bind(const char *addr, unsigned short port)
  - 功能: 绑定指定的网卡上的端口port
* bool bind(unsigned short port)
  - 功能: 绑定所有的网卡上的端口port
* bool listen(int backlog = 5)
  - 功能: 监听端口
* int close()
  - 功能: 关闭内部的socket
