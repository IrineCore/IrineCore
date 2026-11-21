# Networking Architecture

IrineCore uses TCP for control/social/inter-server traffic and Reliable UDP (`RUDP`) for real-time instance gameplay.

## Transports

- **TCP (`common/networking/IocpManager.*`, `SocketContext.*`, `Connection.*`)**:
  - Handles all inter-server RPC channels and client connections to `DNLoginServer` (TCP `14300`), `DNVillageServer` (TCP `14400`), and `DNGameServer` (TCP `15000`).
  - Uses `WinIocpPoller` (IOCP) on Windows and `LinuxEpollPoller` (`epoll` + `eventfd`) on Linux behind `IPlatformPoller`.
- **Reliable UDP (`common/networking/RUDPSocketFrame.*`, `NetSocketRUDP.*`, `src/server/DNGameServer/DNRUDPGameServer.*`)**:
  - Handles low-latency movement, skill execution, hit detection, and state sync on `DNGameServer` (UDP `15100+`).
  - Implements sequenced reliable/unreliable queues, ACK tracking, RTT calculation, and fragmentation/reassembly over standard UDP sockets.

## Packet Framing & Encryption

- **Base Header (`DNTPacketHeader`, 5 bytes, `#pragma pack(push, 1)`)**:
  - `unsigned short iLen` (total packet length including header)
  - `unsigned char cMainCmd`
  - `unsigned char cSubCmd`
- **Encrypted Client Framing (`DNEncryptPacketSeq`)**:
  - Prepends rolling sequence byte (`cSeq`) and length (`nLen`) around a TEA-encrypted payload (`common/networking/CryptPacket.*`, `common/Protocol/DNSecure.h`).
- **String Encoding**:
  - All packet strings are fixed-size 2-byte UTF-16LE `WCHAR` buffers (`static_assert(sizeof(WCHAR) == 2)`).
