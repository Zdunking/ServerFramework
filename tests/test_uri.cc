#include "../zdunk/uri.h"
#include <iostream>

int main(int argc, char **argv)
{
    zdunk::Uri::ptr uri = zdunk::Uri::Create("http://www.sylar.top/test/uri?id=100&name=zdunk#frg");
    // zdunk::Uri::ptr uri = zdunk::Uri::Create("http://admin@www.sylar.top/test/中文/uri?id=100&name=zdunk&vv=中文#frg中文");
    // zdunk::Uri::ptr uri = zdunk::Uri::Create("http://admin@www.sylar.top");
    // zdunk::Uri::ptr uri = zdunk::Uri::Create("http://www.sylar.top/test/uri");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
    return 0;
}
