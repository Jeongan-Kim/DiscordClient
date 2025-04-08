#pragma once
#include <wx/wx.h>
#include "ChatClient.h"
#include "LoginDialog.h"
#include <windows.h>

class Discord : public wxApp {
public:
    virtual bool OnInit() override;
};

wxDECLARE_APP(Discord);