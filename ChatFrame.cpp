#include "ChatFrame.h"
#include "ChatRoomManager.h" 

ChatFrame::ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager)
	: wxFrame(parent, wxID_ANY, roomId), client(clientInst), roomId(roomId), roomManager(manager)
{
    OutputDebugStringA("ChatFrame 생성됨\n");

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "roomManager 유효성: %p\n", roomManager);
    OutputDebugStringA(buffer);


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
}

void ChatFrame::AppendMessage(const std::string& sender, const std::string& text)
{
    if (sender == "SYSTEM")
    {
        wxTextAttr systemStyle = GetStyleForMessage("SYSTEM", client.GetNickname(), "SYSTEM");
        chatDisplay->SetDefaultStyle(systemStyle);
        chatDisplay->AppendText(wxString(text + "\n"));
        return;
    }

    // 예: "[16:24] 정안: 안녕하세요~"
    size_t bracketClose = text.find("]");
    std::string timeStr = text.substr(0, bracketClose + 1);           // "[16:24]"
    std::string remaining = text.substr(bracketClose + 2);            // "정안: 안녕하세요~"
    size_t colonPos = remaining.find(':');
    std::string namePart = remaining.substr(0, colonPos + 1);         // "정안:"
    std::string messagePart = remaining.substr(colonPos + 2);         // "안녕하세요~"

    // 시간 스타일
    wxTextAttr timeStyle = GetStyleForMessage(sender, client.GetNickname(), "TIME");
    chatDisplay->SetDefaultStyle(timeStyle);
    chatDisplay->AppendText(wxString(timeStr + " "));

    // 이름 스타일
    wxTextAttr nameStyle = GetStyleForMessage(sender, client.GetNickname(), "NAME");
    chatDisplay->SetDefaultStyle(nameStyle);
    chatDisplay->AppendText(wxString(namePart + " "));

    // 메시지 스타일
    wxTextAttr msgStyle = GetStyleForMessage(sender, client.GetNickname(), "MESSAGE");
    chatDisplay->SetDefaultStyle(msgStyle);
    chatDisplay->AppendText(wxString(messagePart + "\n"));
    
}

void ChatFrame::UpdateUserList(const std::vector<std::string>& users) 
{
    participantList->Clear();
    int count = 0;
    for (auto& user : users)
    {
        wxString name(user);
        if (user == client.GetNickname()) name += " (나)";
        participantList->Append(name);
        count++;
    }
    participantCountText->SetLabel(wxString::Format("인원: %d명", count));
}

void ChatFrame::OnSend(wxCommandEvent& event)
{
	// [roomId]:[sender]:[message] 형식으로 메시지 전송
    wxString msg = inputBox->GetValue();
    if (!msg.IsEmpty()) 
    {
		std::string message = roomId + ":" + client.GetNickname() + ":" + std::string(msg.mb_str());
		client.Send(message); // 메시지 전송
        //client.Send(std::string(msg.mb_str()));

        inputBox->Clear();
    }
}


void ChatFrame::OnClose(wxCloseEvent& event)
{
    OutputDebugStringA("OnClose 호출됨\n");
    // 퇴장 메시지만 서버에 보내고, 실제 연결은 유지
    roomManager->LeaveRoom(roomId);

    event.Skip(); // 기본 닫기 동작 계속 진행 (GUI 종료)
}

wxTextAttr ChatFrame::GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part)
{
    wxTextAttr style;

    if (part == "SYSTEM")
    {
        style.SetTextColour(wxColour(150, 150, 150));
        style.SetFontStyle(wxFONTSTYLE_ITALIC);
        return style;
    }

    if (part == "TIME")
    {
        style.SetTextColour(wxColour(180, 180, 180));
        style.SetFontSize(8); // 작게
        return style;
    }

    if (part == "NAME")
    {
        if (sender == myNickname)
        {
            style.SetTextColour(wxColour(0, 180, 0));
            style.SetFontWeight(wxFONTWEIGHT_BOLD);
        }
        else
        {
            style.SetTextColour(wxColour(0, 0, 180));
            style.SetFontWeight(wxFONTWEIGHT_BOLD);
        }
        return style;
    }

    if (part == "MESSAGE")
    {
        style.SetTextColour(wxColour(0, 0, 0));
        style.SetFontWeight(wxFONTWEIGHT_NORMAL);
        return style;
    }

    // fallback (기본값)
    style.SetTextColour(wxColour(0, 0, 0));
    style.SetFontWeight(wxFONTWEIGHT_NORMAL);
    style.SetFontStyle(wxFONTSTYLE_NORMAL);
    return style;
}
