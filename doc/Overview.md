
websocket-cpp 是websocket协议的C++实现，基于多线程同步模型

## 优点

小巧易用，简单轻量

## 使用

WebSocketHandler对象代表着一个websocket连接，可以使用send()函数发送数据，使用recv()函数接收数据

也可以通过WebSocketServer模板传入一个WebSocketHandler的子类，并在子类中重载onopen等函数，使用多线程进行数据收发

## 服务端示例

```cpp
#include "websocket.hpp"
using namespace websocket;
```

最简单的使用方法:

```cpp
WebSocketServer<WebSocketHandler> server("127.0.0.1", 2048);
auto handler = server.accept();
// receive data
cout << handler.recv().data() << endl;
// send data
handler.send(handler.data());
// close the websocket connection
handler.close();
```

继承WebSocketHandler，重载onopen、onmessage、onclose等函数，调用WebSocketServer的run()函数，自动进行多线程处理

```cpp
class MyHandler : public WebSocketHandler {
public:
    static void onopen(MyHandler& h) {
        cout << "OPEN: " << h.getHost() << endl;
        cout << "GET " << h.getPath() << endl;
        cout << "Host: " << h.getHost() << endl;
        cout << "Sec-WebSocket-Key: " << h.getKey() << endl;
        cout << "Sec-WebSocket-Protocol: " << h.getSubProtocol() << endl;
    }
    // Received a message
    static void onmessage(MyHandler& h) {
        cout << h.data() << endl;
        h.send(h.data());     // send a message to peer
    }
    static void onclose(MyHandler& h) {
        cout << "CLOSED" << endl;
    }
};
int main() {
    WebSocketServer<MyHandler> server("127.0.0.1", 2048);
    server.run();
    return 0;
}
```

