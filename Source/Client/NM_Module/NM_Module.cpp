#include "stdafx.h"
#include "../NM_ClientSDK/NoMercy.h"
#include "../../Common/CompilerSettings.h"

#include "../../Client/NM_Engine/NM_Index.h"
#ifdef _DEBUG
	#pragma comment(lib, "../../Bin/Debug/NM_Engine.lib")
#else
	#pragma comment(lib, "../../Bin/Release/NM_Engine.lib")
#endif

static HMODULE gs_hModule = nullptr;

inline bool CreateInfoData(std::shared_ptr <ANTI_MODULE_INFO> pAntiModuleInfo, HMODULE hModule)
{
#ifdef _M_X64
	auto pPEB = (PPEB)__readgsqword(0x60);
#elif _M_IX86
	auto pPEB = (PPEB)__readfsdword(0x30);
#else
	#error "architecture unsupported"
#endif

	auto CurrentEntry = pPEB->LoaderData->InLoadOrderModuleList.Flink;

	auto Current = PLDR_DATA_TABLE_ENTRY(NULL);
	while (CurrentEntry != &pPEB->LoaderData->InLoadOrderModuleList && CurrentEntry != NULL)
	{
		Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if ((PVOID)hModule == Current->DllBase)
		{
			pAntiModuleInfo.get()->BaseAddress = Current->DllBase;
			pAntiModuleInfo.get()->EntryPoint = Current->EntryPoint;
			pAntiModuleInfo.get()->SizeOfImage = Current->SizeOfImage;

			pAntiModuleInfo.get()->BaseDllName.Buffer = Current->BaseDllName.Buffer;
			pAntiModuleInfo.get()->BaseDllName.Length = Current->BaseDllName.Length;
			pAntiModuleInfo.get()->BaseDllName.MaximumLength = Current->BaseDllName.MaximumLength;

			pAntiModuleInfo.get()->FullDllName.Buffer = Current->FullDllName.Buffer;
			pAntiModuleInfo.get()->FullDllName.Length = Current->FullDllName.Length;
			pAntiModuleInfo.get()->FullDllName.MaximumLength = Current->FullDllName.MaximumLength;

			return true;
		}
		CurrentEntry = CurrentEntry->Flink;
	}
	return false;
}

#pragma optimize("", off)
extern "C" __declspec(dllexport) bool __cdecl Initialize(const char* c_szLicenseCode)
{
	if (!c_szLicenseCode || !*c_szLicenseCode || !strlen(c_szLicenseCode))
		return false;

	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	auto pModuleInfo = std::make_shared<ANTI_MODULE_INFO>();
	if (!pModuleInfo)
		return false;

	auto bCreateModuleInfo = CreateInfoData(pModuleInfo, gs_hModule);
	if (!bCreateModuleInfo)
		return false;

	bool bStandalone = false;
	/*
#ifdef _DEBUG
	auto hBaseModule = GetModuleHandleA(NULL);

	char pszName[MAX_PATH] = { 0 };
	GetModuleFileNameA(hBaseModule, pszName, _countof(pszName));

	std::string szName = pszName;
	std::transform(szName.begin(), szName.end(), szName.begin(), tolower);

	if (strstr(szName.c_str(), "nm_testapp.exe"))
		bStandalone = true;
#endif
	*/

	return nmCommon->InitCore(c_szLicenseCode, pModuleInfo.get(), bStandalone);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeLauncher(HINSTANCE hInstance, const char* c_szLicenseCode, bool bProtected)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	auto pModuleInfo = std::make_shared<ANTI_MODULE_INFO>();
	if (!pModuleInfo)
		return false;

	auto bCreateModuleInfo = CreateInfoData(pModuleInfo, gs_hModule);
	if (!bCreateModuleInfo)
		return false;

	return nmCommon->InitLauncher(hInstance, c_szLicenseCode, pModuleInfo.get(), bProtected);
}

extern "C" __declspec(dllexport) void __cdecl ServiceMessage(int iMessageID)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return;

	nmCommon->OnServiceMessage(iMessageID);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeService(bool bProtected)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	auto pModuleInfo = std::make_shared<ANTI_MODULE_INFO>();
	if (!pModuleInfo)
		return false;

	auto bCreateModuleInfo = CreateInfoData(pModuleInfo, gs_hModule);
	if (!bCreateModuleInfo)
		return false;

	return nmCommon->InitService(bProtected, pModuleInfo.get());
}

extern "C" __declspec(dllexport) bool __cdecl InitializeShadow(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->InitShadow(gs_hModule, hwnd, hinst, lpszCmdLine, nCmdShow);
}

extern "C" __declspec(dllexport) bool __cdecl InitializeHelper()
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	auto pModuleInfo = std::make_shared<ANTI_MODULE_INFO>();
	if (!pModuleInfo)
		return false;

	auto bCreateModuleInfo = CreateInfoData(pModuleInfo, gs_hModule);
	if (!bCreateModuleInfo)
		return false;

	return nmCommon->InitHelper(pModuleInfo.get());
}

extern "C" __declspec(dllexport) bool __cdecl Finalize()
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->Release();
}


extern "C" __declspec(dllexport) bool __cdecl SetupMsgHandler(TNMCallback lpMessageHandler)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->CreateMessageHandler(lpMessageHandler);
}

extern "C" __declspec(dllexport) bool __cdecl MsgHelper(int Code, void* lpMessage)
{
	auto nmCommon = std::make_unique<CNoMercy>();
	if (!nmCommon || !nmCommon.get())
		return false;

	return nmCommon->SendNMMessage(Code, lpMessage);
}
#pragma optimize("", on)


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(ul_reason_for_call);
	UNREFERENCED_PARAMETER(lpReserved);

	if (!gs_hModule)
		gs_hModule = hModule;

	return TRUE;
}

