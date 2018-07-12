
class cADFPluginData {
    HINSTANCE				mAdfDll;
    size_t                  mLastError;
    std::string             mPath;

    Device                  *mAdfDevice;
    Volume                  *mAdfVolume;

protected:

    LPVFSFILEDATAHEADER GetFileData(const Entry *pEntry, HANDLE pHeap);
    FILETIME            GetFileTime(const Entry *pEntry);

    bool LoadAdf(const std::string& pAfPath);

public:
    cADFPluginData();
    ~cADFPluginData();

    std::vector<std::string> GetPaths(std::string pPath);

    bool AdfChangeToPath(const std::string& pPath, bool pIgnoreLast = false);

    void FreeDir();
    bool ReadDirectory(LPVFSREADDIRDATAA lpRDD);
    bool ReadFile(File* pFile, size_t pBytes, std::uint8_t* pBuffer );
    File* OpenFile(std::string pPath);
    void CloseFile(File* pFile);
 
    int Extract(const std::string& pFile, const std::string& pDest);
    int ExtractFile(const Entry* pEntry, const std::string& pDest);
    int ExtractPath(const std::string& pPath, const std::string& pDest);
    int ContextVerb(LPVFSCONTEXTVERBDATAA lpVerbData);
    UINT BatchOperation(LPTSTR lpszPath, LPVFSBATCHDATAA lpBatchData);
};
