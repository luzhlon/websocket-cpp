
WebSocketHandler对象代表着一个websocket连接，你可以使用send()函数发送数据，使用recv()函数接收数据

## 构造函数

* WebSocketHandler(tstream&& ts)
* WebSocketHandler(WebSocketHandler&& other)

WebSocketHandler只接受移动构造，不接受拷贝构造

## 成员函数

* string getPath()
* string getHost()
* string getKey()
* string getProtocol()

建立websocket连接时需要发送一段http协议的请求头，上面四个函数分别用来获取`GET` `Host` `Sec-WebSocket-Key` `Sec-WebSocket-Protocol`字段的内容

* const char *header() 获取建立连接时接收到的header
* bool init(tstream&& ts)
  - 功能: 使用ts建立websocket连接
  - 返回值: 是否成功
  - 说明: 构造WebSocketHandler对象时如果使用的是默认构造函数，之后可以调用此方法来建立连接

* bool send(const char *data, size_t size, bool bin = false)
  - 功能：向对方发送数据
  - 参数：
    * data: 待发送数据的内存地址
    * size: 待发送数据的字节数
    * bin : 待发送的数据是否为二进制数据
  - 返回值: 是否发送成功

* bool send(const string& data, bool bin = false)
  - 功能: 相当于send(data.c_str(), data.size(), bin)

* WebSocketHandler& recv()
  - 功能：接收数据
  - 参数：无
  - 返回值: WebSocketHandler对象本身
  - 说明: 可通过isConnected()函数判断是否接收成功，若接收数据成功了，可通过data()函数获取接收到的数据

* string& data()
  - 功能：返回WebSocketHandler对象内部数据缓冲区的引用，调用完recv()函数之后用于获取接收到的数据，也可以作send()函数的缓冲区使用

* void close() 关闭此websocket连接


## 静态成员

* void onopen(WebSocketHandler& h)
* void onmessage(WebSocketHandler& h)
* void onerror(WebSocketHandler& h)
* void onclose(WebSocketHandler& h)

这四个函数用于WebSocketServer模板，需要时用户继承WebSocketHandler类并在子类中重载这几个函数
