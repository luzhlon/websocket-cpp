
## websocket-cpp

WebSocket协议的C++实现，多线程同步模型 Simple && Lightweight

## 文档

* Wiki: https://github.com/luzhlon/websocket-cpp/wiki

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

## 客户端

暂未实现
