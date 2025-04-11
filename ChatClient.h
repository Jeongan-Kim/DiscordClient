#pragma once
#include <winsock2.h>
#include <thread>
#include <string>
#include <functional>
#include <map>

class ChatFrame;

// 서버랑 실제 통신하는 클래스(TCP연결, Send, recv 등), 소켓을 직접 열고 대화하는 클래스
// Client 는 싱글톤으로 1개만 존재하게 어디서든 접근할 수 있게
class ChatClient {
public:
    ChatClient();
    ~ChatClient();

    std::string DiscoverServerIP(); // 서버 IP 찾는 함수, 서버가 열려있는지 묻는 함수


    bool Connect(const std::string& ip, int port);
    void Disconnect();
	bool IsConnected() const { return isConnected; } // 연결 상태 확인하는 함수

	void JoinRoom(const std::string& roomId); // 방에 입장하는 함수

    void Send(const std::string& message);
    void StartReceiving();
    void SetNickname(const std::string name);
    std::function<void(const std::string&)> onMessageReceived;  //GUI에서 메시지 받았을 때 해야 할 행동, 콜백함수

	static ChatClient& GetInstance(); // 어디서든 접근할 수 있도록 싱글톤 패턴으로 구현
    std::string GetNickname() const { return nickname; }
	SOCKET GetSocket() const { return sock; } // 소켓을 외부에서 접근할 수 있도록 getter 제공

private:
    SOCKET sock;
    bool isConnected = false;
    //std::map<std::string, ChatFrame*> chatFrames; // 열려있는 채팅방들
    std::string nickname;
};
