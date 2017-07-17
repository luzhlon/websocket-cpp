
#include <stdint.h>
#include "base64.h"

// 0x0  0x1  0x2  0x3  0x4  0x5  0x6  0x7  0x8  0x9  0xa  0xb  0xc  0xd  0xe  0xf  0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27  0x28  0x29  0x2a  0x2b  0x2c  0x2d  0x2e  0x2f  0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37  0x38  0x39  0x3a  0x3b  0x3c  0x3d  0x3e  0x3f
// 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16    17    18    19    20    21    22    23    24    25    26    27    28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48    49    50    51    52    53    54    55    56    57    58    59    60    61    62    63
// A    B    C    D    E    F    G    H    I    J    K    L    M    N    O    P    Q     R     S     T     U     V     W     X     Y     Z     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o     p     q     r     s     t     u     v     w     x     y     z     0     1     2     3     4     5     6     7     8     9     +     /

static void encode_unit(const uint8_t *pi, char *po) {
	static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	po[0] = table[pi[0] >> 2];
	po[1] = table[(pi[0] & 0x03) << 4 | pi[1] >> 4];
	po[2] = table[(pi[1] & 0x0F) << 2 | pi[2] >> 6];
	po[3] = table[pi[2] & 0x3F];
}

static void decode_unit(const uint8_t *pi, char *po) {
	static uint8_t table[] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  +,  ,  ,  ,  /,  0 - 9
	    62, 0,0,0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,0,0,0,0,0,0,
	//  A - Z
	    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,0,0,0,0,0,
	//  a - z
	    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};

	uint8_t buf[4] = { table[*pi], table[pi[1]], 0, 0 };
	po[0] = buf[0] << 2 | buf[1] >> 4;
	if (pi[2] != '=')
		buf[2] = table[pi[2]],
		po[1] = buf[1] << 4 | buf[2] >> 2;
	else
		return;
	if (pi[3] != '=')
		buf[3] = table[pi[3]],
		po[2] = buf[2] << 6 | buf[3];
}

namespace base64 {
	string encode(const string& str) {
		auto size = str.size();
		auto q = size / 3, r = size % 3;
		string out(q * 4 + (r ? 4 : 0), '\0');
		auto po = (char *)out.c_str();
		auto pi = (uint8_t *)str.c_str();
		// transform
		for (; q; q--, pi += 3, po += 4)
			encode_unit(pi, po);
		// tail
		uint8_t buf[3] = { *pi, 0, 0 };
		if (r == 1) {
			encode_unit(buf, po);
			po[2] = po[3] = '=';
		} else if (r == 2) {
			buf[1] = pi[1];
			encode_unit(buf, po);
			po[3] = '=';
		}
		return std::move(out);
	}

	string decode(const string& str) {
		auto size = str.size();
		auto q = size / 4;
		auto out_size = q * 3;
		if (str[size - 1] == '=')
			out_size -= str[size - 2] == '=' ? 2 : 1;
		string out(out_size, '\0');
		auto po = (char *)out.c_str();
		auto pi = (uint8_t *)str.c_str();
		// transform
		for (; q; q--, pi += 4, po += 3)
			decode_unit(pi, po);
		return std::move(out);
	}

}
