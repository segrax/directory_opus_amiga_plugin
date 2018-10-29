typedef std::shared_ptr<List>   spList;
typedef std::shared_ptr<Device> spDevice;
typedef std::shared_ptr<Volume> spVolume;

typedef std::weak_ptr<Device>   wpDevice;
typedef std::weak_ptr<Volume>   wpVolume;

class cADFFindData {

public:

    spList mHead;
    List*  mCell;
    std::regex mFindMask;

};

class cADFPluginData {

    HINSTANCE				mAdfDll;
    size_t                  mLastError;
    std::wstring             mPath;

    spDevice                 mAdfDevice;
    spVolume                 mAdfVolume;

protected:

    LPVFSFILEDATAHEADER GetVFSforEntry(const Entry *pEntry, HANDLE pHeap);
    void GetWfdForEntry(const Entry *pEntry, LPWIN32_FIND_DATA pData);

    FILETIME            GetFileTime(const Entry *pEntry);
    void                SetEntryTime(File *pFile, FILETIME pFT);

    bool LoadFile(const std::wstring& pAfPath);
    spList GetCurrentDirectoryList();

public:
    cADFPluginData();
    ~cADFPluginData();

    std::vector<std::wstring> GetPaths(std::wstring pPath);

    bool AdfChangeToPath(const std::wstring& pPath, bool pIgnoreLast = false);

    bool ReadDirectory(LPVFSREADDIRDATAW lpRDD);
    bool ReadFile(File* pFile, size_t pBytes, std::uint8_t* pBuffer, LPDWORD pReadSize );

    File* OpenFile(std::wstring pPath);
    void CloseFile(File* pFile);
 
    size_t TotalFreeBlocks(const std::wstring& pFile);
    size_t TotalDiskBlocks(const std::wstring& pFile);

    int Delete(LPVFSBATCHDATAW lpBatchData, const std::wstring& pPath, const std::wstring& pFile, bool pAll = false);

    cADFFindData *FindFirstFile(LPTSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent);
    bool FindNextFile(cADFFindData* lpRAF, LPWIN32_FIND_DATA lpwfdData);
    void FindClose(cADFFindData* lpRAF);

    int Import(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath);
    int ImportFile(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath);
    int ImportPath(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pPath);

    int Extract(LPVFSBATCHDATAW lpBatchData, const std::wstring& pFile, const std::wstring& pDest);
    int ExtractFile(LPVFSBATCHDATAW lpBatchData, const Entry* pEntry, const std::wstring& pDest);
    int ExtractPath(LPVFSBATCHDATAW lpBatchData, const std::wstring& pPath, const std::wstring& pDest);

    int ContextVerb(LPVFSCONTEXTVERBDATAW lpVerbData);
    UINT BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAW lpBatchData);
    bool PropGet(vfsProperty propId, LPVOID lpPropData, LPVOID lpData1, LPVOID lpData2, LPVOID lpData3);

};
