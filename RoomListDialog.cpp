#include "RoomListDialog.h"

wxBEGIN_EVENT_TABLE(RoomListDialog, wxFrame)
EVT_BUTTON(wxID_OK, RoomListDialog::OnJoinClicked)
EVT_CLOSE(RoomListDialog::OnClose)
wxEND_EVENT_TABLE()


RoomListDialog::RoomListDialog(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "채팅방 선택", wxDefaultPosition, wxSize(300, 200))
{
    wxPanel* panel = new wxPanel(this);  // ✅ 패널을 만들어서 그 위에 컨트롤 배치

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    roomListBox = new wxListBox(panel, wxID_ANY);
    //roomListBox = new wxListCtrl(panel, wxID_ANY);
	joinButton = new wxButton(panel, wxID_OK, "입장");    //panel은 부모, wxID_OK는 버튼 ID
	createButton = new wxButton(panel, wxID_ANY, "방 생성");
	refreshButton = new wxButton(panel, wxID_ANY, "새로고침");

    createButton->Bind(wxEVT_BUTTON, &RoomListDialog::OnRoomCreateButtonClicked, this);

	refreshButton->Bind(wxEVT_BUTTON, &RoomListDialog::OnRefreshButtonClicked, this);

    //// ✅ 방 목록 생성 및 서버에 CREATE_ROOM 전송
    //wxArrayString rooms = { "방1", "방2", "방3" };
    //ChatClient& client = ChatClient::GetInstance();

    //for (const auto& room : rooms)
    //{
    //    roomListBox->Append(room);

    //    // 서버에 방 생성 요청 전송
    //    std::string msg = "CREATE_ROOM:" + std::string(room.mb_str());
    //    client.Send(msg);
    //}

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(joinButton, 1, wxALL | wxALIGN_CENTER, 10);
    btnSizer->Add(createButton, 1, wxALL | wxALIGN_CENTER, 10);
    btnSizer->Add(refreshButton, 1, wxALL | wxALIGN_CENTER, 10);

    sizer->Add(roomListBox, 1, wxALL | wxEXPAND, 10);
	sizer->Add(btnSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);
    panel->SetSizer(sizer);

    this->SetSize(wxSize(300, 250));
    this->Centre(); // 화면 중앙 정렬
    this->Show();
}

void RoomListDialog::OnJoinClicked(wxCommandEvent& event)
{
    int sel = roomListBox->GetSelection();
    if (sel != wxNOT_FOUND) 
    {
        selectedRoom = roomListBox->GetString(sel);
        OutputDebugStringA("방 선택 완료\n");

        ChatRoomManager& roomManager = ChatRoomManager::GetInstance();

        roomManager.OpenRoom(selectedRoom.ToStdString());
        
    }
    else 
    {
        wxMessageBox("채팅방을 선택해주세요.", "알림", wxOK | wxICON_INFORMATION);
    }
}

void RoomListDialog::OnRoomCreateButtonClicked(wxCommandEvent& event)
{
    OutputDebugStringA("방 생성 버튼 클릭됨\n");

	RoomCreateDialog* roomCreateDialog = new RoomCreateDialog(this);
}

void RoomListDialog::OnRefreshButtonClicked(wxCommandEvent& event)
{
	OutputDebugStringA("새로고침 버튼 클릭됨\n");

	// 서버에 roomsInfo 요청 전송
	ChatClient& client = ChatClient::GetInstance();
	std::string msg = "ROOMLIST_REFRESH";
	client.Send(msg);
}


wxString RoomListDialog::GetSelectedRoom() const
{
    return selectedRoom;
}

void RoomListDialog::RefreshRoomList(std::unordered_map<std::string, std::string> roomsInfo)
{
    OutputDebugStringA("새로고침 시작\n");
	roomListBox->Clear(); // 기존 방 목록 초기화
	for (const auto& room : roomsInfo)
	{
		wxString roomName = room.first;
		//wxString password = room.second;

		// 방 이름과 비밀번호를 사용하여 방 목록에 추가
		//if (password.empty())
		//	roomListBox->Append(roomName);
		//else
		roomListBox->Append(roomName);
	}
	this->Refresh();
	OutputDebugStringA("방 목록 갱신 완료\n");
}

void RoomListDialog::OnClose(wxCloseEvent& event)
{
    ChatClient::GetInstance().Disconnect(); // 프로그램 종료 시에만

    event.Skip();
}

void RoomListDialog::CreateRoom(wxString roomName, wxString password, bool isPasswordProtected)
{
    OutputDebugStringA(("방 [" + roomName + "] 생성 시작\n").c_str());

	// 방 생성 로직을 여기에 추가
	if (roomName.IsEmpty())		return;
	
    OutputDebugStringA("방 리스트에 추가\n");
    roomListBox->AppendString(roomName);
    this->Refresh();

    std::string msg;
	if (isPasswordProtected)
	{
		// 비밀번호 설정 로직 추가
		OutputDebugStringA("비밀번호 있음\n");
		msg = std::string("CREATE_ROOM:PASSWORD_TRUE:") + roomName.ToStdString() + ":" + password.ToStdString();
	}
    else
    {
        OutputDebugStringA("비밀번호 없음\n");
        msg = std::string("CREATE_ROOM:PASSWORD_FALSE:") + roomName.ToStdString();
    }

	// 서버에 방 생성 요청 전송

	ChatClient::GetInstance().Send(msg);
}
