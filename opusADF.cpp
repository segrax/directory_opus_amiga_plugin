// opusADF.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ADFlib/src/adflib.h"


std::vector<std::string> tokenize(const std::string& in, const std::string& delim) {
    std::vector<std::string> tokens;

    std::string::size_type pos_begin, pos_end = 0;
    std::string input = in;

    while ((pos_begin = input.find_first_not_of(delim, pos_end)) != std::string::npos)
    {
        pos_end = input.find_first_of(delim, pos_begin);
        if (pos_end == std::string::npos) pos_end = input.length();

        tokens.push_back(input.substr(pos_begin, pos_end - pos_begin));
    }

    return tokens;
}

cADFPluginData::cADFPluginData() {
    mAdfDevice = 0;
    mAdfVolume = 0;
}

cADFPluginData::~cADFPluginData() {
    adfUnMount(mAdfVolume);
    adfUnMountDev(mAdfDevice);
}

void cADFPluginData::FreeDir() {

}

FILETIME cADFPluginData::GetFileTime(const Entry *pEntry) {
    SYSTEMTIME AmigaTime, TZAdjusted;
    AmigaTime.wDayOfWeek = 0;
    AmigaTime.wMilliseconds = 0;
    AmigaTime.wYear = pEntry->year;
    AmigaTime.wMonth = pEntry->month;
    AmigaTime.wDay = pEntry->days;
    AmigaTime.wHour = pEntry->hour;
    AmigaTime.wMinute = pEntry->mins;
    AmigaTime.wSecond = pEntry->secs;
    TzSpecificLocalTimeToSystemTime(NULL, &AmigaTime, &TZAdjusted);
    FILETIME ft;
    SystemTimeToFileTime(&TZAdjusted, &ft);
    return ft;
}

LPVFSFILEDATAHEADER cADFPluginData::GetFileData(const Entry *pEntry, HANDLE pHeap) {
    LPVFSFILEDATAHEADER lpFDH;

    if (lpFDH = (LPVFSFILEDATAHEADER)HeapAlloc(pHeap, 0, sizeof(VFSFILEDATAHEADER) + sizeof(VFSFILEDATA))) {
        LPVFSFILEDATA lpFD = (LPVFSFILEDATA)(lpFDH + 1);

        lpFDH->cbSize = sizeof(VFSFILEDATAHEADER);
        lpFDH->lpNext = 0;
        lpFDH->iNumItems = 1;
        lpFDH->cbFileDataSize = sizeof(VFSFILEDATA);

        lpFD->dwFlags = 0;
        lpFD->lpszComment = 0;
        lpFD->iNumColumns = 0;
        lpFD->lpvfsColumnData = 0;

        strcpy(lpFD->wfdData.cFileName, pEntry->name);
        lpFD->wfdData.nFileSizeHigh = 0;
        lpFD->wfdData.nFileSizeLow = pEntry->size;

        if (pEntry->type == ST_DIR)
            lpFD->wfdData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        else
            lpFD->wfdData.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

        lpFD->wfdData.dwReserved0 = 0;
        lpFD->wfdData.dwReserved1 = 0;

        lpFD->wfdData.ftCreationTime = {};
        lpFD->wfdData.ftLastAccessTime = {};
        lpFD->wfdData.ftLastWriteTime = GetFileTime(pEntry);
    }

    return lpFDH;
}

std::vector<std::string> cADFPluginData::GetPaths(std::string pPath) {
    std::vector<std::string> Depth;

    if (pPath.find(mPath) != std::string::npos) {
        pPath = pPath.replace(pPath.begin(), pPath.begin() + mPath.length(), "");
        Depth = tokenize(pPath, "\\\\");
    }

    return Depth;
}

bool cADFPluginData::AdfChangeToPath(const std::string& pPath, bool pIgnoreLast) {
    bool result = false;

    if (!LoadAdf(pPath))
        return false;

    std::vector<std::string> Depth = GetPaths(pPath);

    if (pIgnoreLast) {
        if(Depth.size() > 0)
            Depth.erase(Depth.end() - 1);
    }

    if (mAdfVolume) {

        adfToRootDir(mAdfVolume);

        List *head = adfGetDirEnt(mAdfVolume, mAdfVolume->curDirPtr), *cell = head;
        while (cell) {
            struct Entry* entry = (Entry*)cell->content;

            if (!(entry->type == ST_LFILE || entry->type == ST_LDIR || entry->type == ST_LSOFT)) {

                if (Depth.size()) {

                    // Found our sub directory?
                    if (!Depth[0].compare(entry->name)) {

                        // Free current cell, and load the next
                        adfChangeDir(mAdfVolume, entry->name);
                        cell = adfGetDirEnt(mAdfVolume, mAdfVolume->curDirPtr);
                        adfFreeDirList(head);
                        head = cell; 
                        Depth.erase(Depth.begin());
                        continue;
                    }
                } else {
                    result = true;
                    break;
                }
            }

            cell = cell->next;
        }

        adfFreeDirList(head);
    }

    return result;
}

bool cADFPluginData::LoadAdf(const std::string& pAdfPath) {
    std::string AdfPath = pAdfPath;

    std::transform(AdfPath.begin(), AdfPath.end(), AdfPath.begin(), std::tolower);
    size_t EndPos = AdfPath.find(".adf");
    if (EndPos == std::string::npos) {
        EndPos = AdfPath.find(".hdf");
    }

    if (EndPos == std::string::npos)
        return false;

    AdfPath = pAdfPath.substr(0, EndPos + 4);

    if (!mAdfDevice || !mAdfVolume) {
        mAdfDevice = adfMountDev( const_cast<char*>(AdfPath.c_str()), TRUE);

        if (mAdfDevice) {
            mAdfVolume = adfMount(mAdfDevice, 0, TRUE);
            mPath = AdfPath;
            return true;
        }

        return false;
    }

    return true;
}

bool cADFPluginData::ReadDirectory(LPVFSREADDIRDATAA lpRDD) {

    // Free directory if lister is closing (otherwise ignore free command)
    if (lpRDD->vfsReadOp == VFSREAD_FREEDIRCLOSE) {
        FreeDir();
        return true;
    }

    if (lpRDD->vfsReadOp == VFSREAD_FREEDIR)
        return true;

    // Do nothing if we have no path
    if (!lpRDD->lpszPath || !*lpRDD->lpszPath) {
        mLastError = ERROR_PATH_NOT_FOUND;
        return false;
    }

    if (lpRDD->vfsReadOp == VFSREAD_CHANGEDIR)
        return AdfChangeToPath(lpRDD->lpszPath);

    if (!AdfChangeToPath(lpRDD->lpszPath))
        return false;

    List *head = adfGetDirEnt(mAdfVolume, mAdfVolume->curDirPtr), *cell = head;

    LPVFSFILEDATAHEADER lpLastHeader = 0;
    while (cell) {
        struct Entry* entry = (Entry*) cell->content;
        LPVFSFILEDATAHEADER lpFDH;
            
        if (!(entry->type == ST_LFILE || entry->type == ST_LDIR || entry->type == ST_LSOFT)) {

            // Add the entries for this folder
            if (lpFDH = GetFileData(entry, lpRDD->hMemHeap)) {
                if (lpLastHeader)
                    lpLastHeader->lpNext = lpFDH;
                else
                    lpRDD->lpFileData = lpFDH;

                lpLastHeader = lpFDH;
            }
        }

        cell = cell->next;
    }

    adfFreeDirList(head);

    return true;
}

bool cADFPluginData::ReadFile(File* pFile, size_t pBytes, std::uint8_t* pBuffer) {

    if (!pFile)
        return false;

    return (adfReadFile(pFile, pBytes, pBuffer) > 0);
}

File* cADFPluginData::OpenFile(std::string pPath) {
    
    if (AdfChangeToPath(pPath)) {

        auto Paths = GetPaths(pPath);

        return adfOpenFile(mAdfVolume, (char*)Paths[Paths.size() - 1].c_str(), (char*) "r");
    }

    return 0;
}

void cADFPluginData::CloseFile(File* pFile) {

    adfCloseFile(pFile);
}

int cADFPluginData::ContextVerb(LPVFSCONTEXTVERBDATAA lpVerbData) {

    if (AdfChangeToPath(lpVerbData->lpszPath, true)) {
        auto Depth = GetPaths(lpVerbData->lpszPath);

        List *head = adfGetDirEnt(mAdfVolume, mAdfVolume->curDirPtr), *cell = head;

        LPVFSFILEDATAHEADER lpLastHeader = 0;

        while (cell) {
            struct Entry* entry = (Entry*)cell->content;

            if (!Depth[Depth.size() - 1].compare(entry->name)) {

                if (entry->type == ST_FILE)
                    return VFSCVRES_EXTRACT;

                if (entry->type == ST_DIR)
                    return VFSCVRES_DEFAULT;
            }

            cell = cell->next;
        }

        adfFreeDirList(head);
    }

    return VFSCVRES_FAIL;
}

int cADFPluginData::ExtractPath(const std::string& pPath, const std::string& pDest) {
    int result = 0;

    if (AdfChangeToPath(pPath)) {

        List *head = adfGetDirEnt(mAdfVolume, mAdfVolume->curDirPtr), *cell = head;

        while (cell) {
            struct Entry* entry = (Entry*)cell->content;

            std::string FinalPath = pPath;
            if (FinalPath[FinalPath.size() - 1] != '\\') {
                FinalPath += '\\';
            }
            FinalPath += entry->name;

            result |= Extract(FinalPath, pDest);

            cell = cell->next;
        }

        adfFreeDirList(head);
    }

    return result;
}
int cADFPluginData::ExtractFile(const Entry* pEntry, const std::string& pDest) {

    std::string buffer(pEntry->size, 0);

    auto file = adfOpenFile(mAdfVolume, pEntry->name, (char*)"r");
    if (!file)
        return 1;

    auto n = adfReadFile(file, pEntry->size, reinterpret_cast<uint8_t*>(&buffer[0]));
    if (!n)
        return 2;

    std::ofstream ofile(pDest, std::ios::binary);
    ofile.write(buffer.c_str(), buffer.size());
    ofile.close();

    HANDLE filename = CreateFile(pDest.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetFileTime(filename, 0, 0, &GetFileTime(pEntry));
    CloseHandle(filename);

    adfCloseFile(file);

    return 0;
}

int cADFPluginData::Extract(const std::string& pFile, const std::string& pDest) {
    int result = 1;

    if (AdfChangeToPath(pFile, true)) {
        auto Depth = GetPaths(pFile);

        List *head = adfGetDirEnt(mAdfVolume, mAdfVolume->curDirPtr), *cell = head;
        while (cell) {
            struct Entry* entry = (Entry*)cell->content;

            std::string FinalName = pDest;
            if (FinalName[FinalName.size() - 1] != '\\') {
                FinalName += '\\';
            }
            FinalName += entry->name;

            // Entry match?
            if (!Depth[Depth.size() - 1].compare(entry->name)) {

                // Create a directory, or extract a file?
                if (entry->type == ST_DIR) {
                    CreateDirectory(FinalName.c_str(), 0);
                    result = ExtractPath(pFile, FinalName);

                    if (!result) {
                        HANDLE filename = CreateFile(FinalName.c_str(), FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
                        auto aa  = GetLastError();

                        SetFileTime(filename, 0, 0, &GetFileTime(entry));
                        CloseHandle(filename);
                    }
                } else {
                    result = ExtractFile(entry, FinalName);
                }

                break;
            }

            cell = cell->next;
        }

        adfFreeDirList(head);
    }

    return result;
}

UINT cADFPluginData::BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAA lpBatchData) {

    auto result = VFSBATCHRES_COMPLETE;

    for (int i = 0; i < lpBatchData->iNumFiles; ++i) {

        if (lpBatchData->uiOperation == VFSBATCHOP_EXTRACT) {

            lpBatchData->piResults[i] = Extract(&lpBatchData->pszFiles[i], &lpBatchData->pszDestPath[i]);

            if (lpBatchData->piResults[i]) {
                result = VFSBATCHRES_ABORT;
                break;
            }
        }
    }

        return result;
}
