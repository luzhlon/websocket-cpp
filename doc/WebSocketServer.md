
使用WebSocketServer模板需要传入一个WebSocketHandler的子类(HANDLER_T)进行实例化。

## 构造函数

继承自tstream::server

## 成员函数

* HANDLER_T accept()
  - 功能: 监听指定的端口，接受websocket连接
  - 返回值: WebSocketHandler(或其子类)的实例
  - 说明: 返回的实例不一定已经成功建立websocket连接，需要调用实例的isConnected()函数判断

* bool run()
  - 功能: 在当前线程循环调用accept()函数接受websocket连接，然后启动一个新的线程并传入此HANDLER_T对象来处理新的连接。新的线程首先会调用HANDLER_T的onopen函数，然后循环接收消息并调用onmessage函数，期间发生错误时调用onerror函数，发现连接关闭后调用onclose函数并结束线程
  - 返回值: 布尔值，表明是否成功。失败的原因可能是端口被占用，监听失败；若不失败的话，此函数会阻塞当前线程，一直运行下去

## 代码示例

```cpp
#include "websocket.hpp"

using namespace websocket;

class MyHandler : public WebSocketHandler {
public:
    static void onopen(MyHandler& h) {
        cout << "OPEN: " << h.getHost() << endl;
        cout << "GET " << h.getPath() << endl;
        cout << "Host: " << h.getHost() << endl;
        cout << "Sec-WebSocket-Key: " << h.getKey() << endl;
        cout << "Sec-WebSocket-Protocol: " << h.getProtocol() << endl;
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
    WebSocketServer<MyHandler>
            ser("127.0.0.1", 5333);
    ser.run();
    return 0;
}
```
