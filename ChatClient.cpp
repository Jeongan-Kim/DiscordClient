#include "Discord.h"
#include <wx/app.h> 
#include "ChatClient.h"
#include <iostream>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

ChatClient::ChatClient() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    srand(time(NULL)); // 랜덤 시드 설정
}

ChatClient::~ChatClient() {
    closesocket(sock);
    WSACleanup();
}

std::string ChatClient::DiscoverServerIP()
{
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "UDP 소켓 생성 실패\n";
        return "";
    }

    // 브로드캐스트 허용 옵션 설정
    BOOL broadcastOpt = TRUE;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcastOpt, sizeof(broadcastOpt));

    sockaddr_in broadcastAddr = {};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(50505);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;  // 255.255.255.255

    // 서버 찾는 메시지 보내기
    std::string discoverMsg = "DISCOVER_SERVER";
    sendto(udpSocket, discoverMsg.c_str(), discoverMsg.size(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    // 수신 대기 (타임아웃 설정)
    timeval timeout = { 2, 0 }; // 2초
    setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // 서버로부터 메시지 받을 준비
    char buffer[512];
    sockaddr_in senderAddr;
    int senderLen = sizeof(senderAddr);

    int len = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&senderAddr, &senderLen);
    closesocket(udpSocket);

    if (len > 0) {
        buffer[len] = '\0';
        if (std::string(buffer) == "SERVER_HERE") 
        {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
            return std::string(ipStr);
        }
    }

    return "";
}

bool ChatClient::Connect(const std::string& ip, int port) 
{
    if (IsConnected()) return true;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "IP 주소 변환 실패\n";
        return false;
    }

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) 
    {
        return false;
    }

    isConnected = true;

    // 서버에 닉네임 먼저 전송
    send(sock, nickname.c_str(), nickname.length(), 0);

    return true;
}

void ChatClient::Disconnect()
{
    if (isConnected) // 연결 중일때만 종료하도록 체크
    {

        std::string exitMsg = "__DISCONNECT__";
        send(sock, exitMsg.c_str(), exitMsg.size(), 0);
        closesocket(sock);
        isConnected = false;

    }
}

void ChatClient::JoinRoom(const std::string& roomId)
{
    if (sock != INVALID_SOCKET) 
    {
        std::string msg = "JOIN_ROOM:" + roomId;
        send(sock, msg.c_str(), msg.size(), 0);
    }
}

void ChatClient::Send(const std::string& message) 
{
    if (isConnected) 
    {
        OutputDebugStringA((message + "\n").c_str());
        send(sock, message.c_str(), message.size(), 0);
    }
}

void ChatClient::StartReceiving()
{
    std::thread([this]() {
        OutputDebugStringA("StartReceiving started\n");
        char buffer[1024];
        int bytes;

        while (isConnected && (bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) 
        {
            OutputDebugStringA("Received message\n");
            buffer[bytes] = '\0';
            std::string msg = buffer;

            // UI 쓰레드 접근 전에 isConnected 상태 다시 확인
            if (onMessageReceived && isConnected) 
            {
                wxTheApp->CallAfter([=]
                    {
                        onMessageReceived(msg);      
                    });
            }
        }
        OutputDebugStringA("Receiving thread ended\n");
        }).detach();
}

void ChatClient::SetNickname(const std::string name)
{
    nickname = name;
    OutputDebugStringA("닉네임 설정 완료\n");  
}

ChatClient& ChatClient::GetInstance()
{
    static ChatClient instance;
    return instance;
}
