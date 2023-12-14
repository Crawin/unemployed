//
//
#include "framework.h"
#include "Application.h"

#ifdef _DEBUG
// PIX 구글 검색 ㄱ
#include <filesystem>
#include <shlobj.h>
#endif

static std::wstring GetLatestWinPixGpuCapturerPath()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
    {
        if (directory_entry.is_directory())
        {
            if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
            {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty())
    {
        // TODO: Error, no PIX installation found
        std::cerr << "ERROR, no PIX\n";
        return L"";
    }

    return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevGetInstance, LPTSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
    {
        //LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
    }
#endif

	SIZE temp = { 1280, 720 };
	if (false == Application::GetInstance().Init(hInstance, temp)) {
		DebugPrint("Application Init failed!!\n");
		system("Pause");
		return 1;
	}



	Application::GetInstance().StartProgram();

}