#include "ServerAPI.h"

// 서버 주소와 포트
std::string ServerAPI::serverIP = "";
int ServerAPI::serverPort = 9000;

bool ServerAPI::Init() 
{
    ChatClient& client = ChatClient::GetInstance();  // 싱글톤 객체 접근, 클라이언트 생성
	serverIP = client.DiscoverServerIP();  // 서버 IP 탐색

    return !serverIP.empty();
}

bool ServerAPI::Login(const std::string& id, const std::string& pw) 
{
    ChatClient& client = ChatClient::GetInstance();

    // Connect 시도
    if (!client.Connect(serverIP, serverPort)) 
    {
        return false;
    }

    // 로그인 요청 메시지 보내기
    std::string loginMsg = "LOGIN:" + id + ":" + pw;
    client.Send(loginMsg);

    // 응답 대기
    char buffer[256];
    int len = recv(client.GetSocket(), buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return false;

    buffer[len] = '\0';
    std::string response = buffer;

    if (response == "LOGIN_SUCCESS")
    {
        client.SetNickname(id);  // 로그인 성공 시 닉네임 저장
        return true;
    }

    return false;
}

bool ServerAPI::Register(const std::string& id, const std::string& pw) 
{
    std::string msg = BuildRegisterMessage(id, pw);
    std::string response;

    return SendAndReceive(msg, response) && response == "REGISTER_OK";
}

// 메시지 포맷 예: LOGIN|아이디|비밀번호
std::string ServerAPI::BuildLoginMessage(const std::string& id, const std::string& pw) {
    return "LOGIN|" + id + "|" + pw;
}

std::string ServerAPI::BuildRegisterMessage(const std::string& id, const std::string& pw) {
    return "REGISTER|" + id + "|" + pw;
}

// 서버와 연결해 메시지를 보내고 응답을 받음
bool ServerAPI::SendAndReceive(const std::string& message, std::string& response) 
{
    ChatClient& client = ChatClient::GetInstance();
    if (!client.Connect(serverIP, serverPort)) return false;

    client.Send(message);

    char buffer[1024];
    int len = recv(client.GetSocket(), buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return false;

    buffer[len] = '\0';
    response = buffer;

    client.Disconnect();
    return true;
}