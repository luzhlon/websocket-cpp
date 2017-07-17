
#include "websocket.h"

#include <string>
#include <Windows.h>

const char *opcodes[] = {
    "Additional",           // 0
    "Text",                 // 1
    "Binary",               // 2
    "", "", "", "", "",     // 3-7
    "Close",                // 8
    "Ping",                 // 9
    "Pong",                 // 0xA
    "", "", "", "", ""      // 0xB-0xF
};

using namespace std;
using namespace websocket;

int main(int argc, char **argv) {
	tstream::server ser(5333);
	auto ts = ser.accept();
	WebSocketHandler h = move(ts);
	if (h.open())
	while (1) {
		cout << h.getHost() << endl;
		cout << h.getKey() << endl;
		cout << h.getPath() << endl;
		cout << h.getSubProtocol() << endl;
		if (h.recv())
			cout << h.data() << endl,
			h.send(h.data(), true);
		getchar();
	}
    return 0;
}
