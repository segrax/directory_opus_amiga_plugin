typedef std::shared_ptr<List>   spList;
typedef std::shared_ptr<Device> spDevice;
typedef std::shared_ptr<Volume> spVolume;

typedef std::weak_ptr<Device>   wpDevice;
typedef std::weak_ptr<Volume>   wpVolume;

extern std::map<std::string, wpDevice>  gOpenFiles;
extern std::map<std::string, wpVolume>     gOpenVolume;

class cADFFindData {

public:

    spList mHead;
    List*  mCell;
    std::regex mFindMask;

};

class cADFPluginData {

    HINSTANCE				mAdfDll;
    size_t                  mLastError;
    std::string             mPath;

    spDevice                 mAdfDevice;
    spVolume                 mAdfVolume;

protected:

    LPVFSFILEDATAHEADER GetVFSforEntry(const Entry *pEntry, HANDLE pHeap);
    void GetWfdForEntry(const Entry *pEntry, LPWIN32_FIND_DATA pData);

    FILETIME            GetFileTime(const Entry *pEntry);
    void                SetEntryTime(File *pFile, FILETIME pFT);

    bool LoadFile(const std::string& pAfPath);
    spList GetCurrentDirectoryList();

public:
    cADFPluginData();
    ~cADFPluginData();

    std::vector<std::string> GetPaths(std::string pPath);

    bool AdfChangeToPath(const std::string& pPath, bool pIgnoreLast = false);

    bool ReadDirectory(LPVFSREADDIRDATAA lpRDD);
    bool ReadFile(File* pFile, size_t pBytes, std::uint8_t* pBuffer );

    File* OpenFile(std::string pPath);
    void CloseFile(File* pFile);
 
    size_t TotalFreeBlocks(const std::string& pFile);
    size_t TotalDiskBlocks(const std::string& pFile);

    int Delete(LPVFSBATCHDATAA lpBatchData, const std::string& pPath, const std::string& pFile, bool pAll = false);

    cADFFindData *FindFirstFile(LPTSTR lpszPath, LPWIN32_FIND_DATA lpwfdData, HANDLE hAbortEvent);
    bool FindNextFile(cADFFindData* lpRAF, LPWIN32_FIND_DATA lpwfdData);
    void FindClose(cADFFindData* lpRAF);

    int Import(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pPath);
    int ImportFile(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pPath);
    int ImportPath(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pPath);

    int Extract(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pDest);
    int ExtractFile(LPVFSBATCHDATAA lpBatchData, const Entry* pEntry, const std::string& pDest);
    int ExtractPath(LPVFSBATCHDATAA lpBatchData, const std::string& pPath, const std::string& pDest);

    int ContextVerb(LPVFSCONTEXTVERBDATAA lpVerbData);
    UINT BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAA lpBatchData);
};
