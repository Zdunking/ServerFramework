#pragma once
#include "../stream.h"
#include "../socket.h"
#include "../thread.h"
#include "../iomanager.h"

namespace zdunk
{
    class SocketStream : public Stream
    {
    public:
        typedef std::shared_ptr<SocketStream> ptr;

        SocketStream(Socket::ptr sock, bool owner = true);

        ~SocketStream();

        virtual int read(void *buffer, size_t length) override;

        virtual int read(ByteArray::ptr ba, size_t length) override;

        virtual int write(const void *buffer, size_t length) override;

        virtual int write(ByteArray::ptr ba, size_t length) override;

        virtual void close() override;

        Socket::ptr getSocket() const { return m_socket; }

        bool isConnected() const;

        Address::ptr getRemoteAddress();
        Address::ptr getLocalAddress();
        std::string getRemoteAddressString();
        std::string getLocalAddressString();

    protected:
        /// Socket类
        Socket::ptr m_socket;
        /// 是否主控
        bool m_owner;
    };

}
