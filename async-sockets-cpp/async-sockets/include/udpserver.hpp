#pragma once

#include "udpsocket.hpp"
#include <thread>

template <uint16_t BUFFER_SIZE = AS_DEFAULT_BUFFER_SIZE>
class UDPServer : public UDPSocket<BUFFER_SIZE>
{
public:
    // Bind the custom address & port of the server.
    void Bind(const char* address, std::uint16_t port, const char *multicast_address = NULL, FDR_ON_ERROR)
    {
        int status = inet_pton(AF_INET, address, &this->address.sin_addr);
        switch (status) {
            case -1:
                onError(errno, "Invalid address. Address type not supported.");
                return;
            case 0:
                onError(errno, "AF_INET is not supported. Please send message to developer.");
                return;
            default:
                break;
        }

        this->address.sin_family = AF_INET;
        this->address.sin_port = htons(port);

        if (multicast_address != NULL) {
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(multicast_address);
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            int status = setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
            if (status == -1)
            {
                onError(errno, "setsockopt(IP_ADD_MEMBERSHIP) failed.");

                return;
            }
        }

        status = bind(this->sock, (const sockaddr*)&this->address, sizeof(this->address));
        if (status == -1)
        {
            onError(errno, "Cannot bind the socket.");
            return;
        }
    }
    
    // Bind the address(0.0.0.0) & port of the server.
    void Bind(uint16_t port, const char *multicast_address = NULL, FDR_ON_ERROR)
    {
        this->Bind("0.0.0.0", port, multicast_address, onError);
    }

    // Enable or disable the SO_BROADCAST flag
    void setBroadcast(bool value, FDR_ON_ERROR)
    {
        int broadcast = static_cast<int>(value);
        int status = setsockopt(this->sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);
        if (status == -1)
        {
            onError(errno, "setsockopt(SO_BROADCAST) failed.");
            return;
        }
    }
};