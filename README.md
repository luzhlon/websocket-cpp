
C++实现的WebSocket协议，但采用的同步通信模型，不是异步的

## 依赖项目

* [tstream](https://github.com/luzhlon/tstream)

## 服务端

```cpp
#include "websocket.h"

int main {
    tstream::server ser(5333);
    WebSocketHandler h = ser.accept();
    if (h) {    // Open the connection successfully
        cout << "GET " << h.getPath() << endl;
        cout << "Host: " << h.getHost() << endl;
        cout << "Sec-WebSocket-Key: " << h.getKey() << endl;
        cout << "Sec-WebSocket-Protocol: " << h.getSubProtocol() << endl;
        while (1) {
            if (h.recv())
                cout << h.data() << endl,
                h.send(h.data());       // send string data
                h.send(h.data(), true); // send binary data
            getchar();
        }
    }
    return 0;
}
```

## 客户端

未实现
