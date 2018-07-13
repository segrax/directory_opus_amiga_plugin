typedef std::shared_ptr<List> spList;

class cADFPluginData {

    HINSTANCE				mAdfDll;
    size_t                  mLastError;
    std::string             mPath;

    Device                  *mAdfDevice;
    Volume                  *mAdfVolume;

protected:

    LPVFSFILEDATAHEADER GetVFSforEntry(const Entry *pEntry, HANDLE pHeap);
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

    int Import(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pPath);
    int ImportFile(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pPath);
    int ImportPath(LPVFSBATCHDATAA lpBatchData, const std::string& pFile, const std::string& pPath);

    int Extract(const std::string& pFile, const std::string& pDest);
    int ExtractFile(const Entry* pEntry, const std::string& pDest);
    int ExtractPath(const std::string& pPath, const std::string& pDest);

    int ContextVerb(LPVFSCONTEXTVERBDATAA lpVerbData);
    UINT BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAA lpBatchData);
};
