// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

// 8040a29e-0754-4ed7-a532-3688e08fff41
static const GUID GUIDPlugin_ADF = { 0x8040a29e, 0x0754, 0x4ed7,{ 0xa5, 0x32, 0x36, 0x88, 0xe0, 0x8f,0xff, 0x41 } };

HINSTANCE g_hModuleInstance = 0;


extern "C"
{
    __declspec(dllexport) bool VFS_IdentifyW(LPVFSPLUGININFOW lpVFSInfo);
    __declspec(dllexport) bool VFS_ReadDirectoryW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPVFSREADDIRDATAW lpRDD);

    __declspec(dllexport) HANDLE VFS_Create(LPGUID pGuid);
    __declspec(dllexport) void VFS_Destroy(HANDLE hData);

    __declspec(dllexport) int VFS_ContextVerbW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPVFSCONTEXTVERBDATAW lpVerbData);
    __declspec(dllexport) UINT VFS_BatchOperationW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPVFSBATCHDATAW lpBatchData);

    __declspec(dllexport) bool VFS_GetFreeDiskSpaceW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPWSTR lpszPath, unsigned __int64* piFreeBytesAvailable, unsigned __int64* piTotalBytes, unsigned __int64* piTotalFreeBytes);

    __declspec(dllexport) HANDLE VFS_FindFirstFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent);
    __declspec(dllexport) bool VFS_FindNextFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFind, LPWIN32_FIND_DATA lpwfdData);
    __declspec(dllexport) void VFS_FindClose(HANDLE hData, HANDLE hFind);
    __declspec(dllexport) bool VFS_USBSafe(LPOPUSUSBSAFEDATA pUSBSafeData);
    __declspec(dllexport) bool VFS_Init(LPVFSINITDATA pInitData);
    __declspec(dllexport) void VFS_Uninit();
};


extern "C" int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
    g_hModuleInstance = hInstance;
    return 1;
}

// Initialise plugin
bool VFS_Init(LPVFSINITDATA pInitData) {

    adfEnvInitDefault();
    return true;
}

void VFS_Uninit() {
    adfEnvCleanUp();
}

bool VFS_IdentifyW(LPVFSPLUGININFOW lpVFSInfo) {
    // Initialise plugin information
    lpVFSInfo->idPlugin = GUIDPlugin_ADF;
    lpVFSInfo->dwFlags = VFSF_CANCONFIGURE | VFSF_NONREENTRANT;
    lpVFSInfo->dwCapabilities = VFSCAPABILITY_CASESENSITIVE | VFSCAPABILITY_POSTCOPYREREAD | VFSCAPABILITY_READONLY;

    StringCchCopyW(lpVFSInfo->lpszHandleExts, lpVFSInfo->cchHandleExtsMax, L".adf;.hdf");
    StringCchCopyW(lpVFSInfo->lpszName, lpVFSInfo->cchNameMax, L"Amiga ADF/HDF");
    StringCchCopyW(lpVFSInfo->lpszDescription, lpVFSInfo->cchDescriptionMax, L"Amiga Dos disk support");
    StringCchCopyW(lpVFSInfo->lpszCopyright, lpVFSInfo->cchCopyrightMax, L"(c) Copyright 2018 Robert Crossfield");
    StringCchCopyW(lpVFSInfo->lpszURL, lpVFSInfo->cchURLMax, L"github.com/segrax");

    return true;
}

bool VFS_USBSafe(LPOPUSUSBSAFEDATA pUSBSafeData) {
    return true;
}

HANDLE VFS_Create(LPGUID pGuid) {
    return (HANDLE)new cADFPluginData();
}

void VFS_Destroy(HANDLE hData) {
    delete (cADFPluginData*)hData;
}

bool VFS_ReadDirectoryW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPVFSREADDIRDATAW lpRDD) {
    return (hData) ? ((cADFPluginData*)hData)->ReadDirectory(lpRDD) : false;
}

int VFS_ContextVerbW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPVFSCONTEXTVERBDATAW lpVerbData) {

    return (hData) ? ((cADFPluginData*)hData)->ContextVerb(lpVerbData) : VFSCVRES_FAIL;
}

UINT VFS_BatchOperationW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPVFSBATCHDATAW lpBatchData) {
    return (hData) ? ((cADFPluginData*)hData)->BatchOperation(lpszPath, lpBatchData) : VFSCVRES_FAIL;
}

bool VFS_GetFreeDiskSpaceW(HANDLE hData, LPVFSFUNCDATA lpFuncData, LPWSTR lpszPath, unsigned __int64* piFreeBytesAvailable, unsigned __int64* piTotalBytes, unsigned __int64* piTotalFreeBytes) {

    if (!hData)
        return false;

    if(piFreeBytesAvailable)
        *piFreeBytesAvailable = ((cADFPluginData*)hData)->TotalFreeBlocks(lpszPath);

    if(piTotalFreeBytes)
        *piTotalFreeBytes = ((cADFPluginData*)hData)->TotalFreeBlocks(lpszPath);

    if(piTotalBytes)
        *piTotalBytes = ((cADFPluginData*)hData)->TotalDiskBlocks(lpszPath);
    
    return true;
}

HANDLE VFS_FindFirstFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, LPWSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent) {

    return (hData) ? (HANDLE)((cADFPluginData*)hData)->FindFirstFile(lpszPath, lpwfdData, hAbortEvent) : INVALID_HANDLE_VALUE;
}

bool VFS_FindNextFileW(HANDLE hData, LPVFSFUNCDATA lpVFSData, HANDLE hFind, LPWIN32_FIND_DATA lpwfdData) {
    return (hData && hFind) ? ((cADFPluginData*)hData)->FindNextFile((cADFFindData*)hFind, lpwfdData) : false;
}

void VFS_FindClose(HANDLE hData, HANDLE hFind) {
    if (hData && hFind) ((cADFPluginData*)hData)->FindClose((cADFFindData*)hFind);
}
