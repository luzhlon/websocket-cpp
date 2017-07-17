/*
    sha1.hpp - header of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Grabsch <vog@notjusthosting.com>
    Safety fixes
        -- Eugene Hopkinson <slowriot at voxelstorm dot com>
*/

#ifndef SHA1_HPP
#define SHA1_HPP


#include <cstdint>
#include <iostream>
#include <string>


class SHA1
{
public:
    SHA1();
	SHA1(const std::string &s)
		: SHA1() { update(s); }
    void update(const std::string &s);
    void update(std::istream &is);
	void reset();
    std::string final();
	std::string digest();
    static std::string from_file(const std::string &filename);

private:
	void calc();

    uint32_t _digest[5];
    std::string buffer;
    uint64_t transforms;
};


#endif /* SHA1_HPP */
