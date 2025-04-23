#include "Discord.h"
#include <wx/app.h> 
#include "ChatClient.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ws2tcpip.h>
#include <wx/filename.h>
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

void ChatClient::SendFile(const std::string& roomId, const std::string& sender, const std::string& filepath)
{
    if (!isConnected) return;

    // wxFileName 으로 파일명/크기 얻기
    wxFileName fn(filepath);
    std::string filename = fn.GetFullName().ToStdString();
    wxULongLong wxSize = fn.GetSize();
    unsigned long long fileSize = wxSize.GetValue();

    // 파일 전송용 소켓 열기
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9001);         
    addr.sin_addr.s_addr = inet_addr(DiscoverServerIP().c_str());
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
    {
        closesocket(sock);
        return;
    }

    // 헤더 조립: "FILE:room:sender:filename:filesize\n"
    std::ostringstream hdr;
    hdr << "FILE:" << roomId << ':' << sender << ':' << filename << ':' << fileSize << '\n';
    std::string header = hdr.str();

    // 헤더 전송
    send(sock, header.c_str(), header.size(), 0);

    // 5) 본문(바이너리) 전송
    std::ifstream ifs(filepath, std::ios::binary);
    std::vector<char> buffer(8192);
    size_t remaining = fileSize;
    while (remaining > 0 && ifs.good()) {
        size_t chunk = std::min(remaining, buffer.size());
        ifs.read(buffer.data(), chunk);
        send(sock, buffer.data(), chunk, 0);
        remaining -= chunk;
    }

    OutputDebugStringA(("Sent file: " + filename + "\n").c_str());
    closesocket(sock);
}

void ChatClient::StartReceiving()
{
    std::thread([this]() {
        OutputDebugStringA("StartReceiving started\n");
        enum RecvState { READ_HEADER, READ_PAYLOAD };
        RecvState state = READ_HEADER;

        //char buffer[1024];
        //int bytes;
        //std::string recvBuffer;

        std::string headerBuf;
        size_t      bytesToRead = 0;
        std::vector<char> payloadBuf;
        struct { std::string roomId, hour, minute, sender, filename; } meta;

        char rawBuf[4096];
        int  bytes;

        while (isConnected && (bytes = recv(sock, rawBuf, sizeof(rawBuf), 0)) > 0)
        {
            size_t offset = 0;
            while (offset < (size_t)bytes)
            {
                if (state == READ_HEADER)
                {
                    // 1) 개행까지 읽어서 headerBuf에 누적
                    char* nl = (char*)memchr(rawBuf + offset, '\n', bytes - offset);
                    if (nl)
                    {
                        size_t lineLen = nl - (rawBuf + offset) + 1;
                        headerBuf.append(rawBuf + offset, lineLen);
                        headerBuf.pop_back(); // '\n' 제거

                        if (headerBuf.rfind("FILE:", 0) == 0)
                        {
                            // FILE:room:hour:minute:sender:filename:filesize
                            auto trim = headerBuf.substr(5);
                            size_t p1 = trim.find(':');  // roomId 뒤
                            size_t p2 = trim.find(':', p1 + 1); // hour 뒤
                            size_t p3 = trim.find(':', p2 + 1); // minute 뒤
                            size_t p4 = trim.find(':', p3 + 1); // sender 뒤
                            size_t p5 = trim.find(':', p4 + 1); // filename 뒤

                            meta.roomId = trim.substr(0, p1);
                            meta.hour = trim.substr(p1 + 1, p2 - p1 - 1);
                            meta.minute = trim.substr(p2+ 1, p3 - p2 - 1);
                            meta.sender = trim.substr(p3 + 1, p4 - p3 - 1);
                            meta.filename = trim.substr(p4 + 1, p5 - p4 - 1);
                            bytesToRead = std::stoull(trim.substr(p5 + 1));
                            payloadBuf.clear();
                            payloadBuf.reserve(bytesToRead);
                            state = READ_PAYLOAD;
                        }
                        else
                        {
                            // 순수 텍스트 메시지
                            if (onMessageReceived)
                                onMessageReceived(headerBuf);
                        }
                        headerBuf.clear();
                        offset += lineLen;
                    }
                    else
                    {
                        // 개행 못 만났으면 남은 전부 누적
                        headerBuf.append(rawBuf + offset, bytes - offset);
                        offset = bytes;
                    }
                }
                else // READ_PAYLOAD
                {
                    // 2) 파일 본문: bytesToRead 만큼 읽어서 payloadBuf에 담기
                    size_t need = bytesToRead - payloadBuf.size();
                    size_t can = std::min(need, (size_t)bytes - offset);
                    payloadBuf.insert(payloadBuf.end(),
                        rawBuf + offset,
                        rawBuf + offset + can);
                    offset += can;

                    if (payloadBuf.size() == bytesToRead)
                    {
                        // 3) 파일 수신 완료 콜백
                        if (onFileReceived)
                            onFileReceived(
                                meta.roomId,
                                meta.hour,
                                meta.minute,
                                meta.sender,
                                meta.filename,
                                std::move(payloadBuf)
                            );
                        // 다음 메시지로 돌아가기
                        state = READ_HEADER;
                        headerBuf.clear();
                    }
                }
            }
        }
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
