
tstream继承自iostream，表示用于TCP网络连接的流

## 构造函数

* tstream(tstream&& s) 移动构造函数，不允许使用复制构造函数
* tstream(SOCKET sock) 使用指定的socket构造，若传入的sock无效，流会设置badbit
* tstream(char *ip, unsigned short port) 构造时连接服务器，连接失败流会设置failbit

## 成员函数

* int send(const char *buf, size_t len)
  - 功能: 调用原生的send函数
* int recv(char *buf, size_t len)
  - 功能: 调用原生的recv函数
* bool connect(char *ip, unsigned short port)
  - 功能: 连接到指定的服务器

**注意**: recv函数与继承自iostream的read函数的功能是不同的，recv函数不能保证接收到len个字节的数据，read函数会一直接收数据直到len个字节的数据接收完毕
