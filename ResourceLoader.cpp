#include "ResourceLoader.h"
#include "resource.h"
#include <wx/mstream.h>
#include <windows.h>

// LoadPngFromResource 함수는 리소스에서 PNG 이미지를 로드하는 함수
wxImage LoadPngFromResource(int resID)
{
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resID), _T("PNG"));

    std::string debugMsg = "리소스 ID: " + std::to_string(resID);
    OutputDebugStringA((debugMsg + "\n").c_str());

    if (!hRes)
    {
        OutputDebugStringA("리소스 찾을 수 없음\n");
        return wxImage();
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        OutputDebugStringA("리소스 데이터를 로드할 수 없음\n");
        return wxImage();
    }

    void* pData = LockResource(hData);
    if (!pData) {
        OutputDebugStringA("리소스 데이터를 잠글 수 없음\n");
        return wxImage();
    }

    DWORD size = SizeofResource(NULL, hRes);
    if (size == 0) {
        OutputDebugStringA("리소스 크기가 0입니다\n");
        return wxImage();
    }

    wxMemoryInputStream stream(pData, size);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    if (!img.IsOk())
    {
        OutputDebugStringA("이미지 로드실패\n");
		return wxImage();
    }
    return img;
}
