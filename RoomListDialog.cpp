#include "RoomListDialog.h"
#include "resource.h"

wxBEGIN_EVENT_TABLE(RoomListDialog, wxFrame)
EVT_BUTTON(wxID_OK, RoomListDialog::OnJoinClicked)
EVT_CLOSE(RoomListDialog::OnClose)
wxEND_EVENT_TABLE()


RoomListDialog::RoomListDialog(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "채팅방 선택", wxDefaultPosition, wxSize(300, 200))
{
    wxPanel* panel = new wxPanel(this);  // ✅ 패널을 만들어서 그 위에 컨트롤 배치

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // 비밀방 이미지 아이콘
	passwordIconList = new wxImageList(16, 16);
    wxImage noPasswordIcon = LoadPngFromResource(IDB_PNG_PASSWORDICON1); // 0
	wxImage passwordIcon = LoadPngFromResource(IDB_PNG_PASSWORDICON2); // 1  

    passwordIcon.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);
    noPasswordIcon.Rescale(16, 16, wxIMAGE_QUALITY_HIGH);

    passwordIconList->Add(wxBitmap(passwordIcon, 32)); // 비밀번호 아이콘 추가
    passwordIconList->Add(wxBitmap(noPasswordIcon, 32)); // 비밀번호 없는 아이콘 추가


    //roomListBox = new wxListBox(panel, wxID_ANY);
	roomListBox = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);     // wxLC_REPORT : 첫번째 열의 항목 옆에 작은 아이콘이 표시됨.

	//roomListBox->AssignImage(roomListBox, wxIMAGE_LIST_SMALL);
	roomListBox->AssignImageList(passwordIconList, wxIMAGE_LIST_SMALL);
    roomListBox->InsertColumn(0, "방 목록", wxLIST_FORMAT_LEFT, roomListBox->GetClientSize().GetWidth());

	joinButton = new wxButton(panel, wxID_OK, "입장");    //panel은 부모, wxID_OK는 버튼 ID
	createButton = new wxButton(panel, wxID_ANY, "방 생성");
	refreshButton = new wxButton(panel, wxID_ANY, "새로고침");

    createButton->Bind(wxEVT_BUTTON, &RoomListDialog::OnRoomCreateButtonClicked, this);

	refreshButton->Bind(wxEVT_BUTTON, &RoomListDialog::OnRefreshButtonClicked, this);

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
    long selIndex = -1;
	selIndex = roomListBox->GetNextItem(selIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); // 선택된 방의 인덱스 가져오기
    //int sel = roomListBox->GetSelection();
    if (selIndex != -1)
    {
        selectedRoom = roomListBox->GetItemText(selIndex);
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
	roomListBox->DeleteAllItems(); // 기존 방 목록 초기화

    int count = 0;
	for (const auto& room : roomsInfo)
	{
		wxString roomName = room.first;
		//wxString password = room.second;

		// 방 이름과 비밀번호를 사용하여 방 목록에 추가
		if (room.second.empty())       
        {
            OutputDebugStringA(("비밀번호 없음\n"));
            roomListBox->InsertItem(count, roomName, 0); // 비밀번호 없음
        }
		else
        {
            OutputDebugStringA(("비밀번호 있음 [" + room.second + "]\n").c_str());
            roomListBox->InsertItem(count, roomName, 1); // 비밀번호 있음
        }
        count++;
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
	//int count = roomListBox->GetItemCount(); // 방 개수 확인
    //roomListBox->InsertItem(count, roomName, isPasswordProtected);

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
