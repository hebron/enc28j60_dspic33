#include "enc28j60.h"
#include "ipv4.h"
#include "uart.h"
#include "arp.h"

typedef struct Ipv4PacketHandlerInfo_s
{
    bool_t used;
    Ipv4PacketHandler_t handler;
} Ipv4PacketHandlerInfo_t;

Ipv4PacketHandlerInfo_t Ipv4Handlers[IPV4_MAXIMUM_PROTOCOL_HANDLERS];

void ipv4HandlePacket(EthernetFrame_t* frame, bool_t* handled);

void ipv4Init()
{
    enc28j60RegisterTxHandler(ipv4HandlePacket);
}

void ipv4RegisterHandler(Ipv4PacketHandler_t myHandler)
{
    UI08_t i = 0;

    while (i < IPV4_MAXIMUM_PROTOCOL_HANDLERS)
    {
        if (Ipv4Handlers[i].used == FALSE)
        {
            Ipv4Handlers[i].used = TRUE;
            Ipv4Handlers[i].handler = myHandler;
        }
        i++;
    }
}

void ipv4UnregisterHandler(Ipv4PacketHandler_t myHandler)
{
    UI08_t i = 0;

    while (i < IPV4_MAXIMUM_PROTOCOL_HANDLERS)
    {
        if (Ipv4Handlers[i].handler == myHandler)
        {
            Ipv4Handlers[i].used = FALSE;
        }
        i++;
    }
}


void ipv4FireHandlers(EthernetIpv4_t* frame)
{
    UI08_t i = 0;
    bool_t done = FALSE;

    while (i < IPV4_MAXIMUM_PROTOCOL_HANDLERS && done == FALSE)
    {
        if (Ipv4Handlers[i].used == TRUE)
        {
            Ipv4Handlers[i].handler(frame, &done);
        }
        i++;
    }
}

void ipv4HandlePacket(EthernetFrame_t* frame, bool_t* handled)
{
    EthernetIpv4_t* ipv4Header;
    UI08_t headerSize = 0;
    if (frame->type == 0x0800)
    {
        // mark as 'done'
        *handled = TRUE;
        
        // IPV4 packet
        ipv4Header = (EthernetIpv4_t*) frame;
        ipv4Header->header.length = htons(ipv4Header->header.length);

        //if (ipv4Header->version > 4)
        //    return;

        headerSize = 4 * ipv4Header->header.ihl;

#ifdef DEBUG_CONSOLE
        sprintf(debugBuffer, "[ipv4] RX Packet %d bytes, protocol %02X, length %02X sizeof: %d\r\n", headerSize, ipv4Header->header.protocol, ipv4Header->header.length, sizeof(EthernetIpv4_t));
        uartTxString(debugBuffer);
#endif
                
        ipv4FireHandlers(ipv4Header);
        
    }
    
}

UI16_t ipv4Crc(UI08_t* data, UI16_t size)
{
    volatile UI16_t b = 0;
    volatile UI16_t* dataUI16 = (UI16_t*) (data);
    volatile UI32_t crc = 0;
    volatile UI32_t sum = 0;
    UI08_t counts = 0;

    while (b < size/2)
    {
        sum += htons(dataUI16[b]);
        b += 1;
        counts++;

    }

    b = sum >> 16;
    sum = sum & 0xFFFF;
    sum += b;
    crc = ~sum;

    return (UI16_t) crc;
}

void ipv4TxReplyPacket(EthernetIpv4_t* ipv4Packet, UI08_t totalSize)
{
    UI08_t ipTmp[4];
    // Swap source/destination
    memcpy(ipTmp,                               ipv4Packet->header.destinationIp, 4);
    memcpy(ipv4Packet->header.destinationIp,    ipv4Packet->header.sourceIp     , 4);
    memcpy(ipv4Packet->header.sourceIp,         ipTmp                           , 4);

    // Recalculate crc
    ipv4Packet->header.crc          = 0;
    ipv4Packet->frame.type          = htons(ipv4Packet->frame.type);
    ipv4Packet->header.length       = htons(ipv4Packet->header.length);
    ipv4Packet->header.crc          = htons(ipv4Crc((UI08_t*)(&ipv4Packet->header), 4*ipv4Packet->header.ihl ) );

#ifdef DEBUG_CONSOLE
    sprintf(debugBuffer, "[ipv4] TX Packet %d bytes, protocol %02X, length %02X CRC %04X\r\n", 4*ipv4Packet->header.ihl, ipv4Packet->header.protocol, htons(ipv4Packet->header.length), htons(ipv4Packet->header.crc));
    uartTxString(debugBuffer);
#endif
    // Push 1 layer further down
    enc28j60TxReplyFrame((EthernetFrame_t*)ipv4Packet, sizeof(EthernetIpv4Header_t) + totalSize);
}

void ipv4TxPacket(UI08_t* dstIp, UI08_t protocol, UI08_t *data, UI16_t size)
{
    EthernetIpv4_t ipv4Packet;

    ipv4Packet.header.version = 4;
    ipv4Packet.header.ihl = 5;
    ipv4Packet.header.DSCP = 0;
    ipv4Packet.header.length = size;
    ipv4Packet.header.ID = 0x1234;
    ipv4Packet.header.flags = 0;
    ipv4Packet.header.timeToLive = 0x80;
    ipv4Packet.header.protocol = protocol;
    ipv4Packet.header.crc = 0; // calculate?
    memcpy(ipv4Packet.header.destinationIp, dstIp, 4);
    memcpy(ipv4Packet.header.sourceIp, thisIp, 4);

    //enc28j60TxFrame()
}