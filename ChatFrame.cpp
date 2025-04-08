#include "ChatFrame.h"



ChatFrame::ChatFrame(const std::string& nickname)
    : wxFrame(nullptr, wxID_ANY, "Discord GUI", wxDefaultPosition, wxSize(400, 300)) 
{
    Bind(wxEVT_CLOSE_WINDOW, &ChatFrame::OnClose, this); // 종료 이벤트 바인드


    wxPanel* panel = new wxPanel(this);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);  // 수평 정렬

    // 왼쪽 채팅 영역
    wxBoxSizer* chatSizer = new wxBoxSizer(wxVERTICAL);
    chatDisplay = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(360, 180),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    inputBox = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(260, 30), wxTE_PROCESS_ENTER);
    inputBox->Bind(wxEVT_TEXT_ENTER, &ChatFrame::OnSend, this);
    wxButton* sendBtn = new wxButton(panel, wxID_ANY, "Send", wxDefaultPosition, wxSize(90, 30));

    // 채팅 구성요소 추가
    chatSizer->Add(chatDisplay, 1, wxEXPAND | wxALL, 5);
    chatSizer->Add(inputBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    chatSizer->Add(sendBtn, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);

    // 오른쪽 참가자 리스트
    participantList = new wxListBox(panel, wxID_ANY, wxDefaultPosition, wxSize(100, -1));

    // 참여자 인원수
    participantCountText = new wxStaticText(panel, wxID_ANY, "인원: 0명");

    wxBoxSizer* participantSizer = new wxBoxSizer(wxVERTICAL);

    participantSizer->Add(participantList, 1, wxEXPAND | wxBOTTOM, 5);
    participantSizer->Add(participantCountText, 0, wxALIGN_CENTER);


    // 레이아웃 연결
    mainSizer->Add(chatSizer, 1, wxEXPAND);              // 왼쪽: 채팅
    mainSizer->Add(participantSizer, 0, wxEXPAND | wxALL, 5); // 오른쪽: 참가자 목록 + 참여자 인원 수

    panel->SetSizer(mainSizer);

    sendBtn->Bind(wxEVT_BUTTON, &ChatFrame::OnSend, this);

    // 닉네임 셋팅 -> Client 생성
    client.SetNickname(nickname);

    // 서버 IP 탐색
    std::string serverIp = client.DiscoverServerIP();  // ← 여기서 IP 탐색
    int serverPort = 9000;

    if (serverIp.empty()) {
        wxMessageBox("서버 IP를 찾을 수 없습니다.", "에러", wxICON_ERROR);
        Close(); // 창 종료
        return;
    }

    // 서버에 연결
    if (!client.Connect(serverIp, serverPort))
    {
        wxMessageBox("서버에 연결할 수 없습니다.", "에러", wxICON_ERROR);
        Close(); // 창 종료
        return;
    }

    // 메시지 수신 콜백
    client.onMessageReceived = [this](const std::string& msg) 
    {   
        // 1. [USER_LIST] 처리 먼저!
        if (msg.rfind("[USER_LIST]", 0) == 0) 
        {
            wxTheApp->CallAfter([=] {
                participantList->Clear();
                wxString userList = wxString(msg.substr(11)); // "[USER_LIST]" 제거
                wxArrayString names = wxSplit(userList, ';'); // 유저 리스트 ;기준으로 자르기
                names.Sort(); // 가나다순 정렬

                int count = 0;
                for (auto name : names) {
                    if (name.IsEmpty()) continue;

                    if (name == wxString(client.GetNickname())) {
                        name += " (나)";
                    }

                    participantList->Append(name);
                    count++;
                }

                participantCountText->SetLabel(wxString::Format("인원: %d명", count));
              
                });
            return;  // 유저 리스트 처리만 하고 종료
        }

        // 2. 일반 메시지 처리
        wxTheApp->CallAfter([=] 
        {
             std::string myNickname = client.GetNickname();  // ChatClient에서 닉네임 가져옴

             wxTextAttr style = GetStyleForMessage(msg, myNickname);
             chatDisplay->SetDefaultStyle(style);
             chatDisplay->AppendText(wxString(msg + "\n"));
        });
    };

    client.StartReceiving(); //수신 시작
}

ChatFrame::~ChatFrame()
{
    client.Disconnect();
}

void ChatFrame::OnClose(wxCloseEvent& event)
{

    client.Disconnect(); // 🔌 연결 종료
    

    event.Skip(); // 기본 닫기 동작 계속 진행 (GUI 종료)
}

void ChatFrame::OnSend(wxCommandEvent& event) {
    wxString msg = inputBox->GetValue();
    if (!msg.IsEmpty()) {
        client.Send(std::string(msg.mb_str()));


        inputBox->Clear();
    }
}

wxTextAttr ChatFrame::GetStyleForMessage(const std::string& msg, const std::string& myNickname) 
{
    wxTextAttr style;

    if (msg.find("입장하셨습니다") != std::string::npos || msg.find("퇴장하셨습니다") != std::string::npos)
    {
        style.SetTextColour(wxColour(100, 100, 255)); // 파란색
        style.SetFontWeight(wxFONTWEIGHT_BOLD);
    }
    else if (msg.find("[" + myNickname + "]") != std::string::npos) 
    {
        style.SetTextColour(wxColour(0, 180, 0)); // 초록색 (내 메시지)
    }
    else if (msg.find("[") != std::string::npos) 
    {
        style.SetTextColour(wxColour(0, 0, 0)); // 일반 메시지
    }
    else 
    {
        style.SetTextColour(wxColour(150, 150, 150)); // 시스템 등 기타 메시지
    }

    return style;
}
