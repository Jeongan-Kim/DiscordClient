#pragma once

// 서버와 클라이언트 간의 통신을 위한 API 클래스(직접 통신은 Client가 하지만 메시지 정보 정리?하는 역할)
// API : Application Programming Interface 어떤 시스템(서버)과 소통하기 위한 인터페이스
// API는 서버와 클라이언트 간의 통신을 위한 규칙과 형식을 정의하는 역할

//로그인 요청 메시지 구성
//서버에 로그인 요청 전송
//응답 수신 및 파싱
//회원가입 요청 처리도 포함 가능
#include <memory>
#include <string>
#include <iostream>
#include "ChatClient.h"
#include "wx/wx.h"

enum class LoginResult {
    LOGIN_SUCCESS,
    LOGIN_NO_ID,
    LOGIN_WRONG_PW,
    LOGIN_FORMAT_ERROR,
    LOGIN_CONNECT_ERROR,
	LOGIN_ALREADY,
	LOGIN_ERROR
};

class ServerAPI
{
public:
    static bool Init();  // 서버 탐색 및 준비

    // 로그인 요청 - 성공 여부 반환
    static LoginResult Login(const std::string& id, const std::string& pw);

    // 회원가입 요청 - 성공 여부 반환
    static bool Register(const std::string& id, const std::string& pw);

private:
    //ChatClient& client;

    static std::string BuildLoginMessage(const std::string& id, const std::string& pw);
    static std::string BuildRegisterMessage(const std::string& id, const std::string& pw);
    static bool SendAndReceive(const std::string& message, std::string& response);

    static std::string serverIP;
    static int serverPort;
};

