#include "ChatFrame.h"
#include "ChatRoomManager.h" 
#include "ResourceLoader.h"
#include "resource.h"
#include <algorithm>
#include <wx/filedlg.h>
#include "AudioIO.h"

ChatFrame::ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager)
	: wxFrame(parent, wxID_ANY, roomId), client(clientInst), roomId(roomId), roomManager(manager)
{
    OutputDebugStringA("ChatFrame 생성됨\n");
    Bind(wxEVT_CLOSE_WINDOW, &ChatFrame::OnClose, this); // 종료 이벤트 바인드

    SetSize(wxSize(800, 600));

    wxPanel* panel = new wxPanel(this);
    panel->SetSize(wxSize(800, 600));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);  // 수평 정렬

    // ------------------- 왼쪽: 채팅 구성 ---------------------
    wxBoxSizer* chatSizer = new wxBoxSizer(wxVERTICAL);
    chatDisplay = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(360, 180),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    inputBox = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(260, 30), wxTE_PROCESS_ENTER);
    inputBox->Bind(wxEVT_TEXT_ENTER, &ChatFrame::OnSend, this);
    wxButton* sendBtn = new wxButton(panel, wxID_ANY, "Send", wxDefaultPosition, wxSize(90, 30));
    sendBtn->Bind(wxEVT_BUTTON, &ChatFrame::OnSend, this);

    // 채팅 구성요소 추가
    chatSizer->Add(chatDisplay, 1, wxEXPAND | wxALL, 5);
    chatSizer->Add(inputBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    chatSizer->Add(sendBtn, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);

    // ------------------- 오른쪽: 참가자 패널 ---------------------
    wxBoxSizer* participantPanelSizer = new wxBoxSizer(wxVERTICAL);

    // 마이크, 헤드셋 아이콘 이미지 리스트 생성
    participantIcons = new wxImageList(32, 16);
    wxImage micOnheadsetOn = LoadPngFromResource(IDB_PNG_MIC_ON_HEADSET_ON);
    wxImage micOffheadsetOn = LoadPngFromResource(IDB_PNG_MIC_OFF_HEADSET_ON);
    wxImage micOnheadsetOff = LoadPngFromResource(IDB_PNG_MIC_ON_HEADSET_OFF);
    wxImage micOffheadsetOff = LoadPngFromResource(IDB_PNG_MIC_OFF_HEADSET_OFF);

    // 이미지 크기 조정
    micOnheadsetOn.Rescale(32, 16, wxIMAGE_QUALITY_HIGH);
    micOffheadsetOn.Rescale(32, 16, wxIMAGE_QUALITY_HIGH);
    micOnheadsetOff.Rescale(32, 16, wxIMAGE_QUALITY_HIGH);
    micOffheadsetOff.Rescale(32, 16, wxIMAGE_QUALITY_HIGH);

    // imageList에 추가 (wxBitmap으로 변환해서 추가)
    participantIcons->Add(wxBitmap(micOnheadsetOn, 32));      // 0 : 마이크 on, 헤드셋 on
    participantIcons->Add(wxBitmap(micOffheadsetOn, 32));     // 1 : 마이크 off, 헤드셋 on
    participantIcons->Add(wxBitmap(micOnheadsetOff, 32));     // 2 : 마이크 on, 헤드셋 off
    participantIcons->Add(wxBitmap(micOffheadsetOff, 32));    // 3 : 마이크 off, 헤드셋 off

    // ----------------------- 채팅 채널 -------------------------------
    wxStaticBoxSizer* chatChannelBox = new wxStaticBoxSizer(wxVERTICAL, panel);
    wxBoxSizer* chatHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
    chatChannelLabel = new wxStaticText(panel, wxID_ANY, "채팅 채널");
    chatHeaderSizer->Add(chatChannelLabel, 1, wxALIGN_CENTER_VERTICAL);
    chatJoinButton = new wxButton(panel, wxID_ANY, "참가");
	chatJoinButton->Bind(wxEVT_BUTTON, &ChatFrame::OnVoiceLeaveButtonClicked, this);

    chatHeaderSizer->Add(chatJoinButton, 0, wxALIGN_RIGHT);

    chatChannelList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(140, 100), wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
    chatChannelList->InsertColumn(0, "참여자", wxLIST_FORMAT_LEFT, 120);

    chatChannelList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &ChatFrame::OnParticipantRightClick, this);

    chatChannelBox->Add(chatHeaderSizer, 0, wxEXPAND | wxBOTTOM, 5);
    chatChannelBox->Add(chatChannelList, 1, wxEXPAND);

    chatJoinButton->Enable(isInVoiceChannel); // 채팅 채널 참가 버튼 비활성화로 초기화(처음엔 채팅 채널에 참가되어있으므로)

    // ---------------------- 채팅 + 음성 채널 -------------------------
    wxStaticBoxSizer* voiceChannelBox = new wxStaticBoxSizer(wxVERTICAL, panel);
    wxBoxSizer* voiceHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
    voiceChannelLabel = new wxStaticText(panel, wxID_ANY, "음성 채널");
    voiceHeaderSizer->Add(voiceChannelLabel, 1, wxALIGN_CENTER_VERTICAL);
    voiceJoinButton = new wxButton(panel, wxID_ANY, "참가");
	voiceJoinButton->Bind(wxEVT_BUTTON, &ChatFrame::OnVoiceJoinButtonClicked, this);

    voiceHeaderSizer->Add(voiceJoinButton, 0, wxALIGN_RIGHT);

    voiceChannelList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(140, 100), wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
    voiceChannelList->AssignImageList(participantIcons, wxIMAGE_LIST_SMALL);
    voiceChannelList->InsertColumn(0, "참여자", wxLIST_FORMAT_LEFT, 120);

    voiceChannelList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &ChatFrame::OnParticipantRightClick, this);

    voiceChannelBox->Add(voiceHeaderSizer, 0, wxEXPAND | wxBOTTOM, 5);
    voiceChannelBox->Add(voiceChannelList, 1, wxEXPAND);

    // -------- 내 정보 (아이디 + 마이크/헤드셋) --------
    wxBoxSizer* userInfoSizer = new wxBoxSizer(wxHORIZONTAL);
    // 프로필 이미지를 리소스에서 불러오도록 수정
    //profilePic = new wxStaticBitmap(panel, wxID_ANY, wxBitmap("icons/profile.png", wxBITMAP_TYPE_PNG)); // 기본 프로필 이미지
	wxImage profileImage = LoadPngFromResource(IDB_PNG_PROFILE);
	profileImage.Rescale(32, 32, wxIMAGE_QUALITY_HIGH); // 이미지 크기 조정
    profilePic = new wxStaticBitmap(panel, wxID_ANY, wxBitmap(profileImage)); // 리소스에서 기본 프로필 이미지 로드
    profilePic->SetSize(32, 32);

    //userInfoSizer->Add(profilePic, 0, wxRIGHT, 5);
    userInfoSizer->Add(profilePic, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);  // 오른쪽에 5px 여백 추가

    wxStaticText* userIdText = new wxStaticText(panel, wxID_ANY, client.GetNickname());
    userInfoSizer->Add(userIdText, 1, wxALIGN_CENTER_VERTICAL);

    // 프로필 이미지 변경 버튼 추가
    wxButton* changeProfilePicBtn = new wxButton(panel, wxID_ANY, "프로필 이미지 변경");
    changeProfilePicBtn->Bind(wxEVT_BUTTON, &ChatFrame::OnSetProfilePic, this);

    userInfoSizer->Add(changeProfilePicBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    // 마이크, 헤드셋 토글 버튼(내꺼)
    wxImage micImage = LoadPngFromResource(IDB_PNG_MIC_ON);
    wxImage headsetImage = LoadPngFromResource(IDB_PNG_HEADSET_ON);

    micImage.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);
    headsetImage.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);

    micToggle = new wxBitmapToggleButton(panel, wxID_ANY, wxBitmap(micImage), wxDefaultPosition, wxSize(32, 32));
    headsetToggle = new wxBitmapToggleButton(panel, wxID_ANY, wxBitmap(headsetImage), wxDefaultPosition, wxSize(32, 32));

    micToggle->Bind(wxEVT_TOGGLEBUTTON, &ChatFrame::OnMicToggle, this);
    headsetToggle->Bind(wxEVT_TOGGLEBUTTON, &ChatFrame::OnHeadsetToggle, this);

    userInfoSizer->Add(micToggle, 0, wxLEFT, 5);
    userInfoSizer->Add(headsetToggle, 0, wxLEFT, 5);

    // 참가자 패널에 전부 추가
    participantPanelSizer->Add(chatChannelBox, 1, wxEXPAND | wxBOTTOM, 10);
    participantPanelSizer->Add(voiceChannelBox, 1, wxEXPAND | wxBOTTOM, 10);
    participantPanelSizer->Add(userInfoSizer, 0, wxEXPAND | wxALL, 5);

    // 메인 레이아웃 조립
    mainSizer->Add(chatSizer, 1, wxEXPAND);
    mainSizer->Add(participantPanelSizer, 0, wxEXPAND | wxALL, 5);
    panel->SetSizer(mainSizer);


	//// 참여자 목록에 마우스 오른쪽 클릭 이벤트 바인드(음량 조절)
    //   participantList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &ChatFrame::OnParticipantRightClick, this);
    //Show();
	OnInitLayout(); // 레이아웃 초기화 완료 표시


	// 마이크, 헤드셋 토글 버튼 초기화(꺼놓기)
    UpdateVoiceControlsState(false);

}

void ChatFrame::MarkInitialized()
{
    isInitialized = true;
    OutputDebugStringA("ChatFrame::MarkInitialized 호출됨\n");
    // 미처 반영하지 못했던 voice update 반영
    //VoiceChannelManager& voiceManager = VoiceChannelManager::GetInstance();
    //if (voiceManager.HasPendingVoiceUpdate(roomId)) 
    //{
    //    OutputDebugStringA("Pending voice update 반영\n");
    //    std::vector<std::string> users = voiceManager.ConsumePendingUpdate(roomId);
    //    voiceManager.UpdateVoiceUserList(roomId, users);
    //}
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
    currentUsers = users;  // 최신 유저 정보 저장

    chatChannelList->DeleteAllItems(); // 기존 참여자 목록 삭제
    int count = 0;

    for (auto& user : users)
    {
        wxString name(user);

        if (user == client.GetNickname())
        {
            name += " (나)";
        }

        chatChannelList->InsertItem(count, name);
        count++;
    }
    chatChannelLabel->SetLabel(wxString::Format("채팅 채널 (%d명)", count));
}

void ChatFrame::OnVoiceChannelJoinedByManager()
{
    OutputDebugStringA("OnVoiceChannelJoinedByManager\n");

    isInVoiceChannel = true;
    UpdateVoiceControlsState(true); // 마이크/헤드셋 버튼 활성화
    //UpdateVoiceParticipantList(); // 참가자 목록 갱신
    UpdateJoinButtons(); // [참가] 버튼 상태 갱신
}

void ChatFrame::OnVoiceChannelLeftByManager()
{
    OutputDebugStringA("OnVoiceChannelLeftByManager\n");

    isInVoiceChannel = false;
    UpdateVoiceControlsState(false); // 마이크/헤드셋 비활성화
    //UpdateVoiceParticipantList(); // 나간 사람 빼고 음성채널 참여자 목록 업데이트
    UpdateJoinButtons(); // [참가] 버튼 상태 갱신

    OutputDebugStringA("기존 참가하던 채널 나가기 완료\n");

}

void ChatFrame::UpdateVoiceControlsState(bool isActive)
{
    if (micToggle) micToggle->Enable(isActive);
    if (headsetToggle) headsetToggle->Enable(isActive);
}

void ChatFrame::UpdateVoiceParticipantList()
{
    OutputDebugStringA("UpdateVoiceParticipantSList\n");
    if (!voiceChannelList) return;

    voiceChannelList->DeleteAllItems();

    auto voiceUsers = VoiceChannelManager::GetInstance().GetUsersInVoice(roomId);
    int count = 0;

    for (const auto& user : voiceUsers)
    {
        wxString name(user.clientId);
        bool micOn = user.micOn;
        bool headsetOn = user.headsetOn;

        if (user.clientId == client.GetNickname())
        {
            name += " (나)";
            micOn = micStatus;
            headsetOn = headsetStatus;
        }

        int imageIndex = 0;
        if (!micOn && headsetOn) imageIndex = 1;
        else if (micOn && !headsetOn) imageIndex = 2;
        else if (!micOn && !headsetOn) imageIndex = 3;

        voiceChannelList->InsertItem(count, name, imageIndex);
        count++;
    }
    voiceChannelLabel->SetLabel(wxString::Format("음성 채널 (%d명)", count));
}

void ChatFrame::UpdateJoinButtons()
{
	chatJoinButton->Enable(isInVoiceChannel);    // 음성 채널에 참가했으면 활성화
	voiceJoinButton->Enable(!isInVoiceChannel);  // 음성 채널에 참가했으면 비활성화
}

void ChatFrame::OnVoiceJoinButtonClicked(wxCommandEvent& event)
{    
    OutputDebugStringA(("OnVoiceJoinButtonClicked" + roomId + "\n").c_str());
    VoiceChannelManager::GetInstance().JoinVoiceChannel(this, roomId, client.GetNickname());
    AudioIO& audioIO = AudioIO::GetInstance();
    audioIO.SetMicMuted(micStatus);
    audioIO.SetHeadsetMuted(headsetStatus);
    
}

void ChatFrame::OnVoiceLeaveButtonClicked(wxCommandEvent& event)
{
    OutputDebugStringA("OnVoiceLeaveButtonClicked\n");
    VoiceChannelManager::GetInstance().LeaveVoiceChannel(this, roomId, client.GetNickname());
}

void ChatFrame::OnInitLayout()
{
    MarkInitialized();
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
	VoiceChannelManager::GetInstance().LeaveVoiceChannel(this, roomId, client.GetNickname()); // 음성채널 나가기 처리
    // 퇴장 메시지 서버에 보내기
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

void ChatFrame::OnMicToggle(wxCommandEvent& event)
{
	micStatus = event.IsChecked(); // 마이크 상태 토글
    AudioIO& audioIO = AudioIO::GetInstance();
    audioIO.SetMicMuted(micStatus);

    wxImage img_on = LoadPngFromResource(IDB_PNG_MIC_ON);
    wxImage img_off = LoadPngFromResource(IDB_PNG_MIC_OFF);

    wxImage img = micStatus ? img_on : img_off;
    img.Rescale(16, 16);
    micToggle->SetBitmap(wxBitmap(img));

    // RoomChannelManager에게 서버에게 나 마이크 토글했다고 알리라고 하기.
    ChatRoomManager::GetInstance().MicSet(roomId, micStatus);
    // 상태 저장 및 참여자 목록 갱신 호출
    UpdateVoiceParticipantList();
}

void ChatFrame::OnHeadsetToggle(wxCommandEvent& event)
{
    headsetStatus = event.IsChecked();
    AudioIO& audioIO = AudioIO::GetInstance();
    audioIO.SetHeadsetMuted(headsetStatus);

    wxImage img_on = LoadPngFromResource(IDB_PNG_HEADSET_ON);
    wxImage img_off = LoadPngFromResource(IDB_PNG_HEADSET_OFF);

	wxImage img = headsetStatus ? img_on : img_off;
    img.Rescale(16, 16);
    headsetToggle->SetBitmap(wxBitmap(img));

    // RoomChannelManager에게 서버에게 나 마이크 토글했다고 알리라고 하기.
    ChatRoomManager::GetInstance().HeadsetSet(roomId, headsetStatus);
    // 상태 저장 및 참여자 목록 갱신 호출
    UpdateVoiceParticipantList();
}

void ChatFrame::OnParticipantRightClick(wxListEvent& event) {
    long itemIndex = event.GetIndex();
    wxString username = voiceChannelList->GetItemText(itemIndex);

    wxDialog* volumeDialog = new wxDialog(this, wxID_ANY, username + " 볼륨 조절", wxDefaultPosition, wxSize(200, 100));
    wxSlider* volumeSlider = new wxSlider(volumeDialog, wxID_ANY, 50, 0, 100);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(volumeSlider, 1, wxALL | wxEXPAND, 10);
    volumeDialog->SetSizerAndFit(sizer);
    volumeDialog->ShowModal();
}

void ChatFrame::OnSetProfilePic(wxCommandEvent& event)
{
    // 파일 선택 대화상자 열기
    wxFileDialog openFileDialog(this, _("프로필 이미지 선택"), "", "", "Image files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_OK)
    {
        // 사용자가 선택한 파일 경로
        wxString filePath = openFileDialog.GetPath();

        // 프로필 이미지를 새 경로로 로드
        wxImage profileImage;
        if (profileImage.LoadFile(filePath, wxBITMAP_TYPE_ANY))
        {
            // 프로필 이미지 로드 성공
            profileImage.Rescale(32, 32, wxIMAGE_QUALITY_HIGH);  // 16x16 사이즈로 리사이즈
            profilePic->SetBitmap(wxBitmap(profileImage));  // wxStaticBitmap에 이미지 업데이트
        }
        else
        {
            wxLogError("프로필 이미지를 불러올 수 없습니다.");
        }
    }
}

