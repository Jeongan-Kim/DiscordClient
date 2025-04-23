#include "ChatFrame.h"
#include "ChatRoomManager.h" 
#include "ResourceLoader.h"
#include "resource.h"
#include "AudioIO.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <wx/filedlg.h>
#include <wx/regex.h>
#include <wx/popupwin.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/display.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include <wx/artprov.h>
#include <wx/mstream.h>
#include <wx/textctrl.h> 

wxBEGIN_EVENT_TABLE(ChatFrame, wxFrame)
EVT_ACTIVATE(ChatFrame::OnActivate)
wxEND_EVENT_TABLE()

ChatFrame::ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager)
	: wxFrame(parent, wxID_ANY, roomId), client(clientInst), roomId(roomId), roomManager(manager)
{
    OutputDebugStringA("ChatFrame 생성됨\n");
    Bind(wxEVT_CLOSE_WINDOW, &ChatFrame::OnClose, this); // 종료 이벤트 바인드
    Bind(wxEVT_TIMER, &ChatFrame::OnExpireTimer, this);

    SetSize(wxSize(800, 600));

    wxPanel* panel = new wxPanel(this);
    panel->SetSize(wxSize(800, 600));
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);  // 수평 정렬

    // ------------------- 왼쪽: 채팅 구성 ---------------------
    wxBoxSizer* chatSizer = new wxBoxSizer(wxVERTICAL);
    chatDisplay = new wxRichTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(360, 180), wxRE_MULTILINE | wxRE_READONLY);
    chatDisplay->Bind(wxEVT_TEXT_URL, &ChatFrame::OnURLClick, this); // 링크인 경우 누르면 OnURLClick 호출

    wxBoxSizer* inputRowSizer = new wxBoxSizer(wxHORIZONTAL);
    inputBox = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(400, 30), wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    inputBox->Bind(wxEVT_TEXT_ENTER, &ChatFrame::OnSend, this);
    wxButton* sendBtn = new wxButton(panel, wxID_ANY, "Send", wxDefaultPosition, wxSize(90, 30));
    sendBtn->Bind(wxEVT_BUTTON, &ChatFrame::OnSend, this);

    emoticonButton = new wxButton(panel, wxID_ANY, "E", wxDefaultPosition, wxSize(30, 30));
    emoticonButton->Bind(wxEVT_BUTTON, &ChatFrame::OnEmoticonButtonClick, this); // 이모티콘 창 생성

    wxButton* attachmentBtn = new wxButton(panel, wxID_ANY, "+", wxDefaultPosition, wxSize(30, 30));
    attachmentBtn->Bind(wxEVT_BUTTON, &ChatFrame::OnAttachmentButtonClick, this); // 파일 첨부 창 생성


    inputRowSizer->Add(inputBox, 1, wxEXPAND | wxALL, 5);
    inputRowSizer->Add(emoticonButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    inputRowSizer->Add(attachmentBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    inputRowSizer->Add(sendBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    // 채팅 구성요소 추가
    chatSizer->Add(chatDisplay, 1, wxEXPAND | wxALL, 5);
    chatSizer->Add(inputRowSizer, 0, wxEXPAND);

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

    //chatChannelList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &ChatFrame::OnParticipantRightClick, this);

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

    voiceChannelList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &ChatFrame::OnParticipantRightClick, this); // 볼륨 조절 창 생성

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

    chatDisplay->SetBackgroundColour(*wxWHITE);
    chatDisplay->SetOwnBackgroundColour(*wxWHITE);

	//// 참여자 목록에 마우스 오른쪽 클릭 이벤트 바인드(음량 조절)
    //   participantList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &ChatFrame::OnParticipantRightClick, this);
    //Show();
	OnInitLayout(); // 레이아웃 초기화 완료 표시


	// 마이크, 헤드셋 토글 버튼 초기화(꺼놓기)
    UpdateVoiceControlsState(false);

    inputBox->SetFocus();
    // 한영키 토글
    keybd_event(VK_HANGUL, 0, 0, 0);
    keybd_event(VK_HANGUL, 0, KEYEVENTF_KEYUP, 0);
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

void ChatFrame::AppendMessage(const std::string& hour, const std::string& minute, const std::string& sender, const wxString& text)
{
    chatDisplay->Freeze();
    chatDisplay->SetInsertionPointEnd();
    chatDisplay->SetBackgroundColour(*wxWHITE);
    chatDisplay->SetOwnBackgroundColour(*wxWHITE);

    //메시지 스타일(기본)
    wxTextAttr baseMsgStyle = GetStyleForMessage(sender, client.GetNickname(), "MESSAGE");


    if (sender == "SYSTEM")
    {
        wxTextAttr systemStyle = GetStyleForMessage("SYSTEM", client.GetNickname(), "SYSTEM");
        chatDisplay->SetDefaultStyle(systemStyle);
        chatDisplay->WriteText(text + "\n");

        chatDisplay->SetDefaultStyle(baseMsgStyle);  // 기본 스타일 리셋
        chatDisplay->Thaw();
        return;
    }

    // 예: "[16:24] 정안: (이모티콘)안녕하세요~"

    wxString timeStr = "[" + wxString(hour) + ":" + wxString(minute) + "]";           // "[16:24]"
    std::string namePart = sender;

    // 시간 스타일
    wxTextAttr timeStyle = GetStyleForMessage(sender, client.GetNickname(), "TIME");
    chatDisplay->SetDefaultStyle(timeStyle);
    chatDisplay->WriteText(timeStr + " ");

    // 이름 스타일
    wxTextAttr nameStyle = GetStyleForMessage(sender, client.GetNickname(), "NAME");
    chatDisplay->SetDefaultStyle(nameStyle);
    chatDisplay->WriteText(wxString(namePart + " "));




    // URL 또는 :token: 패턴 (wxString에서 바로 쓰는 UTF‑8 리터럴)
    wxRegEx re(
        R"((https?://[^ ]+)|:([A-Za-z0-9_+\-]+):)",
        wxRE_ADVANCED | wxRE_ICASE
    );

    if (!re.IsValid())
    {
        // regex 컴파일 실패 시 그냥 일반 텍스트
        chatDisplay->WriteText(text);
        chatDisplay->Newline();
        chatDisplay->Thaw();
        return;
    }

    // 2) content 파싱
    wxString msg = text;
    size_t globalPos = 0/*, len = text.Length()*/;
    size_t start, len;

    // 2) 이모티콘(:token:) 또는 URL(http:// or https://) 패턴을 하나로 매치
   // re.Matches(text, flags, startpos) 로 루프
    while (re.Matches(msg))
    {
        // 전체 매치 위치 얻기 (index=0)
        re.GetMatch(&start, &len, 0);

        // 매치 전 일반 텍스트
        if (start > 0)
        {
            chatDisplay->SetDefaultStyle(baseMsgStyle);
            chatDisplay->WriteText(text.Mid(0, start));
        }

        // 1) URL 그룹(1) 확인
        wxString url = re.GetMatch(msg, 1);
        wxString token = re.GetMatch(msg, 2);

        if (!url.IsEmpty())
        {
            wxTextAttr urlStyle = GetStyleForMessage(sender, client.GetNickname(), "URL");
            chatDisplay->SetDefaultStyle(urlStyle);

            chatDisplay->BeginURL(url);
            chatDisplay->WriteText(url);
            chatDisplay->EndURL();

            // 반드시 스타일 복구
            chatDisplay->SetDefaultStyle(baseMsgStyle);
        }
        else if(!token.IsEmpty())
        {
            // 2) 이모티콘 그룹(2)

            auto itEmoji = emojiRes.find(std::string(token.utf8_str()));
            if (itEmoji != emojiRes.end())
            {
                wxImage img = LoadPngFromResource(itEmoji->second);
                if (img.IsOk())
                {
                    img.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);                   
                    chatDisplay->WriteImage(wxBitmap(img));
                }
                else
                {
                    chatDisplay->SetDefaultStyle(baseMsgStyle);
                    chatDisplay->WriteText(":" + token + ":");
                }
            }
            else
            {
                chatDisplay->SetDefaultStyle(baseMsgStyle);
                chatDisplay->WriteText(":" + token + ":");
            }
        }

        // 다음 검색 위치로 이동
        msg = msg.Mid(start + len);
        globalPos += start + len;
    }
   

    // 남은 일반 텍스트
    if (!msg.IsEmpty())
    {
        // 메시지 스타일
        chatDisplay->SetDefaultStyle(baseMsgStyle);
        chatDisplay->WriteText(msg);
    }

    chatDisplay->Newline();
    chatDisplay->SetDefaultStyle(baseMsgStyle); // 이걸 호출하면 배경은 컨트롤의 기본(위에서 흰색으로 고정)으로 돌아갑니다
    inputBox->SetFocus();
    inputBox->SetInsertionPointEnd();
    chatDisplay->Thaw();

    if (IsIconized() || !isActive) // 현재 창이 최소화되어 있거나, 활성 상태가 아니면
    {
        // 방 이름(roomId)이나 발신자(sender) 등을 포함해서
        wxString note = wxString::Format( "%s: %s", roomId, text.Left(30) + wxT("…"));
        
        ShowNotification(roomId, sender, text.Left(30));
    }
}

void ChatFrame::AppendFileMessage(const std::string& hour, const std::string& minute, const std::string& sender, const std::string& filename, const std::vector<char>& data)
{
    // 1) Freeze/스타일 기본 설정
    chatDisplay->Freeze();
    chatDisplay->SetInsertionPointEnd();
    wxTextAttr base = GetStyleForMessage(sender, client.GetNickname(), "MESSAGE");

    // 2) 시간+이름 출력 (AppendMessage 와 동일)
    wxString timeStr = "[" + wxString(hour) + ":" + wxString(minute) + "]";
    chatDisplay->SetDefaultStyle(GetStyleForMessage(sender, client.GetNickname(), "TIME"));
    chatDisplay->WriteText(timeStr + " ");
    chatDisplay->SetDefaultStyle(GetStyleForMessage(sender, client.GetNickname(), "NAME"));
    chatDisplay->WriteText(wxString(sender + " "));

    // 3) 파일 UI 삽입
    // 3-1) 이미지 파일이면 썸네일
    wxString ext = wxFileName(filename).GetExt().Lower();
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif")
    {
        // 메모리 스트림에서 로드
        wxMemoryInputStream mis(data.data(), data.size());
        wxImage img(mis, wxBITMAP_TYPE_ANY);
        wxSize previewSz = img.GetSize();
        if (img.IsOk())
        {
            if (previewSz.x > 300 || previewSz.y > 300)
                img.Rescale(
                    previewSz.x > 300 ? 300 : previewSz.x,
                    previewSz.y > 300 ? 300 : previewSz.y,
                    wxIMAGE_QUALITY_HIGH
                );

            wxBitmap bmp(img);

            // ID 생성 & 맵에 저장
            std::string id = std::to_string(wxGetLocalTimeMillis().GetValue());
            pendingFiles[id] = PendingFile{ filename, data };

            chatDisplay->WriteImage(bmp);

            chatDisplay->WriteText(" "); // 약간 여백
            // 그리고 다운로드 링크도 복원
            chatDisplay->BeginURL("img:" + id);
            chatDisplay->WriteText("[보기]");
            chatDisplay->EndURL();
        }
        else
        {
            // 로드 실패 시 아이콘 + 파일명
            wxBitmap icon = wxArtProvider::GetBitmap(
                wxART_NORMAL_FILE, wxART_OTHER, wxSize(24, 24));
            chatDisplay->WriteImage(icon);
            chatDisplay->WriteText(" " + wxString(filename));
        }
    }
    else
    {
        // 3-2) 일반 파일: 아이콘 + 파일명 + 용량 + [다운로드]
        wxBitmap icon = wxArtProvider::GetBitmap(
            wxART_NORMAL_FILE, wxART_OTHER, wxSize(24, 24));
        chatDisplay->WriteImage(icon);

        double sizeKB = data.size() / 1024.0;
        chatDisplay->SetDefaultStyle(base);
        chatDisplay->WriteText(" " + wxString(filename) + wxString::Format(" (%.1f KB) ", sizeKB));

        // 식별자 생성 (예: 메모리 맵 키)
        std::string id = wxString::Format("%lld", wxGetLocalTimeMillis().GetValue()).ToStdString();
        pendingFiles[id] = PendingFile{ filename, data };

        // [다운로드] 링크 삽입
        chatDisplay->BeginURL("dl:" + id);
        chatDisplay->WriteText("[다운로드]");
        chatDisplay->EndURL();
    }

    // 4) 마무리
    chatDisplay->Newline();
    inputBox->SetFocus();
    inputBox->SetInsertionPointEnd();
    chatDisplay->Thaw();
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
        voiceChannelList->SetItemTextColour(count, *wxLIGHT_GREY);
        count++;
    }
    voiceChannelLabel->SetLabel(wxString::Format("음성 채널 (%d명)", count));
}

void ChatFrame::HighlightVoiceUser(std::string talker)
{
    if (IsBeingDeleted() || !voiceChannelList)
        return;

    int idx = FindItemIndex(talker);
    if (idx == wxNOT_FOUND) return;

    // 말하고 있으면 검정
    voiceChannelList->SetItemTextColour(idx, *wxBLACK);
    voiceChannelList->RefreshItem(idx);

    // ) 기존 타이머가 있으면 멈추고 삭제
    auto itOld = _clientTimers.find(talker);
    if (itOld != _clientTimers.end()) {
        itOld->second->Stop();
        delete itOld->second;
        _clientTimers.erase(itOld);
    }

    // 새 타이머 생성 (ID 자동 할당)
    int tid = _nextTimerId++;
    wxTimer* t = new wxTimer(this, tid);
    _clientTimers[talker] = t;
    _timerToClient[tid] = talker;
    t->StartOnce(500);  // 500ms 후에 한 번만 발동
    
    //voiceChannelList->Refresh();
}

void ChatFrame::OnExpireTimer(wxTimerEvent& evt)
{
    int tid = evt.GetId();               // 이 타이머의 ID
    auto it = _timerToClient.find(tid);
    if (it == _timerToClient.end()) return;

    std::string clientId = it->second;
    int idx = FindItemIndex(clientId);
    if (idx != wxNOT_FOUND)
    {
        // 회색으로 돌려놓기
        voiceChannelList->SetItemTextColour(idx, *wxLIGHT_GREY);
        voiceChannelList->RefreshItem(idx);
    }

    // 타이머 정리
    auto itTimer = _clientTimers.find(clientId);
    if (itTimer != _clientTimers.end()) {
        itTimer->second->Stop();
        delete itTimer->second;
        _clientTimers.erase(itTimer);
    }

    _timerToClient.erase(it);
}

int ChatFrame::FindItemIndex(const std::string& clientId) const
{
    for (int i = 0; i < voiceChannelList->GetItemCount(); ++i) 
    {
        if (voiceChannelList->GetItemText(i).ToStdString() == clientId
            || voiceChannelList->GetItemText(i).ToStdString() == clientId + " (나)")
            return i;
    }
    return wxNOT_FOUND;
}

void ChatFrame::UpdateJoinButtons()
{
	chatJoinButton->Enable(isInVoiceChannel);    // 음성 채널에 참가했으면 활성화
	voiceJoinButton->Enable(!isInVoiceChannel);  // 음성 채널에 참가했으면 비활성화
}

void ChatFrame::OnURLClick(wxTextUrlEvent& event)
{
    // 클릭된 “URL” (BeginURL() 에 넘긴 그 문자열)
    wxString url = event.GetString();

    if (url.StartsWith("dl:"))
    {
        // ── 다운로드 링크 처리 ──
        // 기본 동작(브라우저로 열기)은 막고
        event.Skip(false);

        std::string id = std::string(url.Mid(3).utf8_str());
        auto it = pendingFiles.find(id);
        if (it == pendingFiles.end()) return;

        // 원래 파일명과 확장자를 가져오기
        const std::string& origName = it->second.filename;
        wxFileName fn(origName);
        wxString defaultName = fn.GetFullName();            // ex: "image.png"
        wxString wildcard = "*." + fn.GetExt().Lower();   // ex: "*.png"


        wxFileDialog dlg( this, "저장할 위치 선택", wxEmptyString, defaultName, wildcard,  wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() == wxID_OK)
        {
            std::ofstream ofs(dlg.GetPath().ToStdString(), std::ios::binary);
            ofs.write(it->second.data.data(), it->second.data.size());
            ofs.close();
        }
        pendingFiles.erase(it);
    }
    else if (url.StartsWith("img:"))
    {
        ShowImagePreview(url.Mid(4).ToStdString());
    }
    else
    {
        // ── 일반 URL 처리 ──
        // 기본 동작(브라우저로 열기) 대신 직접 실행
        // 그리고 이벤트도 더 이상 처리하지 않음
        event.Skip(false);
        wxLaunchDefaultBrowser(url);
    }
}

void ChatFrame::OnEmoticonButtonClick(wxCommandEvent& event)
{
    wxDialog dlg(this, wxID_ANY, "이모티콘", wxDefaultPosition);
    wxGridSizer* grid = new wxGridSizer(5, 5, 5, 5);
    std::vector<std::pair<std::string, int>> emojis = 
    {
        {"angry", IDB_PNG_ANGRY},
        {"confused", IDB_PNG_CONFUSED},
        {"poop", IDB_PNG_POOP},
        {"kiss", IDB_PNG_KISS},
        {"frowning", IDB_PNG_FROWNING},
        {"grinning", IDB_PNG_GRINNING},
        {"grinningsweat", IDB_PNG_GRINNING_SWEAT},
        {"grinningwink", IDB_PNG_GRINNING_WINK},
        {"rollsmile", IDB_PNG_ROLL_SMILE},
        {"smile", IDB_PNG_SMILE},
        {"smileheart", IDB_PNG_SMILE_HEART},
        {"tired", IDB_PNG_TIRED}
    };

    for (auto& [name, resId] : emojis) {
        wxImage img = LoadPngFromResource(resId);
        img.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);

        wxBitmap bmp(img);

        // 2) 버튼 생성
        wxBitmapButton* btn =
            new wxBitmapButton(&dlg, wxID_ANY, bmp,
                wxDefaultPosition, wxDefaultSize,
                wxBORDER_NONE);

        // 3) 람다에 dlg를 참조 캡처
        btn->Bind(wxEVT_BUTTON, [this, name, &dlg, bmp, img](wxCommandEvent&)
            {                                
                long pos = inputBox->GetInsertionPoint();// 현재 커서 위치
                //wxRichTextImageBlock block;
                //block.MakeImageBlockDefaultQuality(img, wxBITMAP_TYPE_PNG); // 이미지 블록 만들고 삽입
                wxString token = ":" + name + ":";

                inputBox->Freeze();
                //inputBox->WriteImage(block);
                inputBox->WriteText(token);
                inputBox->WriteText(" ");
                inputBox->Thaw();
                
                EmojiInfo info = { pos, 1, name };
                insertEmojis.push_back(info); // 삽입 위치와 길이, 이름을 기록

                // 모달 다이얼로그 닫기
                dlg.EndModal(wxID_OK);
            });

        grid->Add(btn, 0, wxEXPAND);
    }
    dlg.SetSizerAndFit(grid);
    dlg.ShowModal();
}

void ChatFrame::OnAttachmentButtonClick(wxCommandEvent& event)
{
    // 파일 선택 대화상자 열기
    wxFileDialog openFileDialog(this, _("첨부파일을 선택하세요"), "", "", _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() != wxID_OK) return;    

    wxString filePath = openFileDialog.GetPath();// 사용자가 선택한 파일 경로
        
    wxFileName filename(filePath);// 파일 정보 추출
    wxString name = filename.GetFullName();     // 파일 이름
    wxString ext = filename.GetExt().Lower();   // 확장자
    wxULongLong size = filename.GetSize();      // 파일 크기

    bool isImage = ext == "png" || ext == "jpg" || ext == "jpeg";

    if (isImage)
        AskImageAttachment(filePath, name, size);
    else
        AskFileAttachment(filePath, name, size);
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
    std::string sender = client.GetNickname();

    wxString text = inputBox->GetValue(); //순수 텍스트 가져오기(placeholder 공백 들어있음)
    if (text.IsEmpty()) return;

    // UTF‑8 로 변환
    wxCharBuffer buf = text.ToUTF8();
    std::string utf8(buf.data(), buf.length());

    // roomId, nick 은 미리 정의된 std::string
    std::string packet = roomId + ":" + sender + ":" + utf8;
    client.Send(packet);

    inputBox->Clear();

}


void ChatFrame::OnClose(wxCloseEvent& event)
{
    OutputDebugStringA("OnClose 호출됨\n");
    // 모든 타이머 삭제
    for (auto& [clientId, timer] : _clientTimers) {
        timer->Stop();
        delete timer;
    }
    _clientTimers.clear();
    _timerToClient.clear();

	VoiceChannelManager::GetInstance().LeaveVoiceChannel(this, roomId, client.GetNickname()); // 음성채널 나가기 처리
    // 퇴장 메시지 서버에 보내기
    roomManager->LeaveRoom(roomId);

    event.Skip(); // 기본 닫기 동작 계속 진행 (GUI 종료)
}

wxTextAttr ChatFrame::GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part)
{
    wxTextAttr style;
    style.SetBackgroundColour(*wxWHITE);

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
        }
        else
        {
            style.SetTextColour(wxColour(0, 0, 180));
        }
        style.SetFontWeight(wxFONTWEIGHT_BOLD);
        return style;
    }

    if (part == "MESSAGE")
    {
        style.SetTextColour(wxColour(0, 0, 0));
        style.SetFontWeight(wxFONTWEIGHT_NORMAL);
        return style;
    }

    if (part == "URL")
    {
        style.SetTextColour(wxColour(128, 0, 128)); //보라색
        //style.SetTextColour(wxColour(225, 165, 0)); //주황색
        style.SetFontStyle(wxFONTSTYLE_ITALIC);
        style.SetFontWeight(wxFONTWEIGHT_BOLD);
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

void ChatFrame::ShowImagePreview(const std::string& id)
{
    auto it = pendingFiles.find(id);
    if (it == pendingFiles.end()) return;

    // 메모리에서 wxImage 로 로드
    const auto& pf = it->second;
    wxMemoryInputStream mis(pf.data.data(), pf.data.size());
    wxImage orig(mis, wxBITMAP_TYPE_ANY);
    if (!orig.IsOk()) return;

    // 2) 화면 기준 초기 스케일
    wxRect area = wxGetClientDisplayRect();
    int maxW = int(area.GetWidth() * 0.8);
    int maxH = int(area.GetHeight() * 0.8);
    double sx = double(maxW) / orig.GetWidth();
    double sy = double(maxH) / orig.GetHeight();
    double initScale = std::min(1.0, std::min(sx, sy));

    // 스케일과 이미지를 캡처할 shared_ptr
    auto scalePtr = std::make_shared<double>(initScale);
    auto imgPtr = std::make_shared<wxImage>(orig);

    // 3) 다이얼로그 + 스크롤윈도우 + 스태틱비트맵
    wxDialog* dlg = new wxDialog(this, wxID_ANY, pf.filename,
        wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    wxScrolledWindow* scrol = new wxScrolledWindow(dlg, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxHORIZONTAL | wxVERTICAL);
    const int scrollRateX = 10, scrollRateY = 10;
    scrol->SetScrollRate(scrollRateX, scrollRateY);

    // 함수: 스케일에 맞춰 비트맵 재생성하고 레이아웃 갱신
    auto refreshBitmap = [=]() {
        double s = *scalePtr;
        int w = int(imgPtr->GetWidth() * s);
        int h = int(imgPtr->GetHeight() * s);
        wxImage scaled = imgPtr->Copy();
        scaled.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
        wxBitmap bmp(scaled);

        // static bitmap 이 없으면 새로, 있으면 교체
        wxStaticBitmap* sb = scrol->FindWindowById(wxID_ANY)
            ? dynamic_cast<wxStaticBitmap*>(scrol->GetChildren()[0])
            : nullptr;
        if (!sb) {
            sb = new wxStaticBitmap(scrol, wxID_ANY, bmp);
        }
        else {
            sb->SetBitmap(bmp);
        }

        scrol->SetVirtualSize(w, h);
        scrol->Layout();
        scrol->Refresh();
        };

    // 처음 한 번 그려 주기
    refreshBitmap();

    // 4) Ctrl+휠 바인딩
    scrol->Bind(wxEVT_MOUSEWHEEL, [=](wxMouseEvent& evt) {
        if (evt.ControlDown()) {
            int rot = evt.GetWheelRotation();
            double& s = *scalePtr;

            // 확대/축소
            s *= (rot > 0 ? 1.2 : 1 / 1.2);

            // 최소값을 initScale 으로, 최대값은 필요에 따라 (예: 5.0)로 고정
            s = std::clamp(s, initScale, 5.0);

            refreshBitmap();
            return; // 이벤트 소모
        }
        evt.Skip(); // 일반 스크롤은 그대로
        });

    // 5) 다운로드 버튼
    wxButton* btn = new wxButton(dlg, wxID_ANY, "다운로드");

    // 6) 레이아웃
    wxBoxSizer* vs = new wxBoxSizer(wxVERTICAL);
    vs->Add(scrol, 1, wxEXPAND | wxALL, 5);
    vs->Add(btn, 0, wxALIGN_CENTER | wxALL, 5);
    dlg->SetSizerAndFit(vs);

    // 7) 다운로드 바인딩
    btn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        wxFileName fn(pf.filename);
        wxFileDialog filedlg(
            dlg, "저장할 위치 선택",
            wxEmptyString,
            fn.GetFullName(),
            "*." + fn.GetExt().Lower(),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        );
        if (filedlg.ShowModal() == wxID_OK) {
            std::ofstream ofs(filedlg.GetPath().ToStdString(),
                std::ios::binary);
            ofs.write(pf.data.data(), pf.data.size());
        }
        });

    dlg->ShowModal();
    dlg->Destroy();
}

void ChatFrame::AskImageAttachment(const wxString& path, const wxString& name, wxULongLong size)
{
    // 300×300 이하로 축소해서 보여주기
    wxImage img(path, wxBITMAP_TYPE_ANY);
    wxSize maxSz(300, 300);
    if (img.GetWidth() > maxSz.x || img.GetHeight() > maxSz.y)
        img.Rescale
        (
            img.GetWidth() > maxSz.x ? maxSz.x : img.GetWidth(),
            img.GetHeight() > maxSz.y ? maxSz.y : img.GetHeight()
        );

    // 커스텀 다이얼로그
    wxDialog dlg(this, wxID_ANY, _("이미지 첨부 확인"), wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* vs = new wxBoxSizer(wxVERTICAL);

    vs->Add(new wxStaticBitmap(&dlg, wxID_ANY, wxBitmap(img)), 0, wxALIGN_CENTER | wxALL, 10);

    wxString info = wxString::Format(_("파일명: %s\n크기: %.2f KB"), name, size.ToDouble() / 1024.0);
    vs->Add(new wxStaticText(&dlg, wxID_ANY, info), 0, wxALIGN_LEFT | wxALL, 10);

    wxString ask = wxString::Format(_("이 파일을 보내시겠습니까?"));
    vs->Add(new wxStaticText(&dlg, wxID_ANY, ask), 0, wxALIGN_LEFT | wxALL, 10);

    vs->Add(dlg.CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 10);

    dlg.SetSizerAndFit(vs);

    if (dlg.ShowModal() == wxID_OK)
        client.SendFile(roomId, client.GetNickname(), path.ToStdString());
}

void ChatFrame::AskFileAttachment(const wxString& path, const wxString& name, wxULongLong size)
{
    wxDialog dlg(this, wxID_ANY, _("파일 첨부 확인"));
    wxBoxSizer* vs = new wxBoxSizer(wxVERTICAL);

    // 왼쪽 아이콘
    wxBitmap icon = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(32, 32));
    wxBoxSizer* hs = new wxBoxSizer(wxHORIZONTAL);
    hs->Add(new wxStaticBitmap(&dlg, wxID_ANY, icon), 0, wxALL, 10);

    // 오른쪽 텍스트
    wxString info = wxString::Format(_("파일명: %s\n크기: %.2f KB"), name, size.ToDouble() / 1024.0);
    hs->Add(new wxStaticText(&dlg, wxID_ANY, info), 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);

    vs->Add(hs, 0, wxALL, 0);

    wxString ask = wxString::Format(_("이 파일을 보내시겠습니까?"));
    vs->Add(new wxStaticText(&dlg, wxID_ANY, ask), 0, wxALIGN_LEFT | wxALL, 10);
    vs->Add(dlg.CreateSeparatedButtonSizer(wxOK | wxCANCEL),
        0, wxEXPAND | wxALL, 10);

    dlg.SetSizerAndFit(vs);

    if (dlg.ShowModal() == wxID_OK)
        client.SendFile(roomId, client.GetNickname(), path.ToStdString());
}

void ChatFrame::OnParticipantRightClick(wxListEvent& event) {
    long itemIndex = event.GetIndex();
    wxString username = voiceChannelList->GetItemText(itemIndex);
    std::string clientId = voiceChannelList->GetItemText(itemIndex).ToStdString();

    wxDialog* volumeDialog = new wxDialog(this, wxID_ANY, username + " 볼륨 조절", wxDefaultPosition, wxSize(250, 140));
    wxSlider* volumeSlider = new wxSlider(volumeDialog, wxID_ANY, int(VoiceChannelManager::GetInstance().GetParticipantVolume(roomId, clientId) * 100), 0, 100, wxDefaultPosition, wxSize(200, -1), wxSL_HORIZONTAL | wxSL_LABELS);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(volumeSlider, 1, wxALL | wxEXPAND, 10);

    wxCheckBox* chkMute = new wxCheckBox(volumeDialog, wxID_ANY, "음소거");
    chkMute->SetValue(VoiceChannelManager::GetInstance().IsParticipantMuted(roomId, clientId));
    sizer->Add(chkMute, 0, wxALL | wxALIGN_CENTER, 5);

    volumeDialog->SetSizerAndFit(sizer);

    // 2) 슬라이더 이벤트 → 설정 저장
    volumeSlider->Bind(wxEVT_SLIDER, [=](wxCommandEvent& e) {
        float vol = e.GetInt() / 100.0f;
        VoiceChannelManager::GetInstance()
            .SetParticipantVolume(roomId, clientId, vol);
        });

    // 3) 체크박스 이벤트 → 음소거 설정
    chkMute->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& e) {
        bool mute = e.IsChecked();
        VoiceChannelManager::GetInstance()
            .SetParticipantMute(roomId, clientId, mute);
        });

    volumeDialog->ShowModal();



    //long idx = event.GetIndex();
    //std::string clientId = voiceChannelList->GetItemText(idx).ToStdString();
    //std::string title = clientId + " 볼륨/뮤트";

    // 1) 다이얼로그 + 컨트롤 생성
    //wxDialog dialog(this, wxID_ANY, title, wxDefaultPosition, wxSize(250, 140));
    //wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);

    // 볼륨 슬라이더
    //wxSlider* slider = new wxSlider(&dialog, wxID_ANY, int(VoiceChannelManager::GetInstance().GetParticipantVolume(roomId, clientId) * 100), 0, 100, wxDefaultPosition, wxSize(200, -1), wxSL_HORIZONTAL | wxSL_LABELS);
    //s->Add(slider, 0, wxALL | wxALIGN_CENTER, 10);

    // 음소거 체크박스
    //wxCheckBox* chkMute = new wxCheckBox(&dialog, wxID_ANY, "음소거");
    //chkMute->SetValue(VoiceChannelManager::GetInstance().IsParticipantMuted(roomId, clientId));
    //s->Add(chkMute, 0, wxALL | wxALIGN_CENTER, 5);

    // 확인/취소 버튼
    //s->Add(dialog.CreateSeparatedButtonSizer(wxOK | wxCANCEL),  0, wxEXPAND | wxALL, 10);

    //dialog.SetSizerAndFit(s);

    // 2) 슬라이더 이벤트 → 설정 저장
    //slider->Bind(wxEVT_SLIDER, [=](wxCommandEvent& e) {
    //    float vol = e.GetInt() / 100.0f;
    //    VoiceChannelManager::GetInstance()
    //        .SetParticipantVolume(roomId, clientId, vol);
    //    });

    //// 3) 체크박스 이벤트 → 음소거 설정
    //chkMute->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& e) {
    //    bool mute = e.IsChecked();
    //    VoiceChannelManager::GetInstance()
    //        .SetParticipantMute(roomId, clientId, mute);
    //    });

    //dialog.ShowModal();
    //dialog.Destroy();
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

void ChatFrame::OnActivate(wxActivateEvent& evt)
{
    isActive = evt.GetActive();
    evt.Skip();
}

void ChatFrame::ShowNotification(const wxString& roomId, const wxString& sender, const wxString& msg)
{
    // 팝업 윈도우 생성 (포커스를 잃으면 자동 닫힘)
    //wxPopupTransientWindow* popup = new wxPopupTransientWindow(this, wxBORDER_NONE);
    wxFrame* popup = new wxFrame(
        nullptr, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize,
        wxFRAME_SHAPED
        | wxFRAME_NO_TASKBAR
        | wxSTAY_ON_TOP
        | wxBORDER_NONE
    );
    popup->Bind(wxEVT_ACTIVATE, [popup](wxActivateEvent& evt) {
        if (!evt.GetActive())
            popup->Destroy();
        evt.Skip();  // 다음 바인딩에도 이벤트 전달
        });

    // 2) 타이머로 일정 시간 뒤 사라지게 하기 (예: 5초)
    //    wxTimer 의 부모를 popup 으로 두면 popup 파괴시 자동으로 해제됨
    wxTimer* timer = new wxTimer(popup);
    popup->Bind(wxEVT_TIMER, [popup, timer](wxTimerEvent&) {
        if (popup && popup->IsShown())
            popup->Destroy();
        // timer 는 popup 파괴시 함께 사라집니다.
        });
    timer->Start(5000, /*oneShot=*/true);

    // 내용물 패널
    wxPanel* panel = new wxPanel(popup);
    panel->SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);

    // 3) 페인트 핸들러 연결 (rounded bg + 반투명)
    panel->Bind(wxEVT_PAINT, [panel](wxPaintEvent&)
        {
        wxAutoBufferedPaintDC dc(panel);
        dc.SetBackground(*wxTRANSPARENT_BRUSH);
        dc.Clear();
        wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
        if (!gc) return;

        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

        int w = panel->GetSize().x, h = panel->GetSize().y;
        const int r = 12; // 모서리 반경

        // 짙은 회색 90% 불투명
        wxBrush brush(wxColour(30, 30, 30, 230));
        wxPen   pen(wxColour(255, 255, 255, 180), 1.0);

        gc->SetBrush(brush);
        gc->SetPen(pen);
        gc->DrawRoundedRectangle(0, 0, w, h, r); // TODO:모서리 둥글게 처리 안됨.
        
        delete gc;
        });

    // 4) 레이아웃 준비: 헤더(방이름) / 본문(sender+msg)
    wxBoxSizer* mainSz = new wxBoxSizer(wxVERTICAL);

    // 4‑1) 헤더 바
    wxPanel* header = new wxPanel(panel);
    header->SetBackgroundColour(wxColour(40, 120, 220)); // 블루
    wxStaticText* roomLbl = new wxStaticText(header, wxID_ANY, roomId);
    roomLbl->SetForegroundColour(*wxWHITE);
    wxFont hf = roomLbl->GetFont(); hf.SetPointSize(10); hf.SetWeight(wxFONTWEIGHT_BOLD);
    roomLbl->SetFont(hf);

    wxBoxSizer* hs = new wxBoxSizer(wxHORIZONTAL);
    hs->Add(roomLbl, 1, wxALL, 6);
    header->SetSizerAndFit(hs);
    mainSz->Add(header, 0, wxEXPAND);

    // 4‑2) 본문 영역
    wxPanel* body = new wxPanel(panel);
    body->SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);

    wxStaticText* senderLbl = new wxStaticText(body, wxID_ANY, sender);
    senderLbl->SetForegroundColour(wxColour(0, 0, 0));
    wxFont sf = senderLbl->GetFont(); sf.SetPointSize(9); sf.SetWeight(wxFONTWEIGHT_BOLD);
    senderLbl->SetFont(sf);

    wxStaticText* msgLbl = new wxStaticText(body, wxID_ANY, msg);
    msgLbl->SetForegroundColour(wxColour(128, 128, 128));
    wxFont mf = msgLbl->GetFont(); mf.SetPointSize(8);
    msgLbl->SetFont(mf);

    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    bs->Add(senderLbl, 0, wxLEFT | wxRIGHT | wxTOP, 6);
    bs->Add(msgLbl, 0, wxALL, 6);
    body->SetSizerAndFit(bs);
    mainSz->Add(body, 0, wxEXPAND);

    panel->SetSizerAndFit(mainSz);
    popup->SetClientSize(panel->GetSize());
    popup->SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
    panel->SetBackgroundStyle(wxBG_STYLE_PAINT);

    popup->Show();
    popup->Raise();

#ifdef __WXMSW__
    HWND hwnd = (HWND)popup->GetHWND();
    LONG ex = ::GetWindowLong(hwnd, GWL_EXSTYLE);
    ::SetWindowLong(hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
    ::SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
#endif

    // 5) region(rounded rect) 생성
    int w = panel->GetSize().GetWidth();
    int h = panel->GetSize().GetHeight();
    const int r = 12;
#ifdef __WXMSW__
    // CreateRoundRectRgn의 마지막 두 인자는 ellipse width/height,
    // 즉 radius*2 를 넘겨야 진짜 반지름 r 가 됩니다.
    HRGN hRgn = ::CreateRoundRectRgn(0, 0, w, h, r * 2, r * 2);
    wxRegion region(hRgn);  // hrgn 소유권은 wxRegion 이 가져갑니다
#else
    // Windows 가 아니면 일단 사각 region 으로 fallback
    wxRegion region(wxRect(0, 0, w, h));
#endif

    // 4) Apply shape
    popup->SetShape(region);

    // 5) 위치 잡기
    wxSize sz = popup->GetSize();
    wxDisplay disp((unsigned int)0);
    wxRect area = disp.GetClientArea();
    popup->SetPosition(
        { area.x + area.width - sz.GetWidth(),
          area.y + area.height - sz.GetHeight() }
    );

}

