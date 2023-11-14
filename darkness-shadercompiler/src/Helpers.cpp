#include "Helpers.h"
#include "platform/File.h"
#include "containers/vector.h"

#include <sstream>
#include <fstream>

#include <Windows.h>
#include <strsafe.h>

VOID CALLBACK FileIOCompletionRoutine(
    __in  DWORD dwErrorCode,
    __in  DWORD dwNumberOfBytesTransfered,
    __in  LPOVERLAPPED lpOverlapped
);

VOID CALLBACK FileIOCompletionRoutine(
    __in  DWORD dwErrorCode,
    __in  DWORD dwNumberOfBytesTransfered,
    __in  LPOVERLAPPED lpOverlapped)
{
    //_tprintf(TEXT("Error code:\t%x\n"), dwErrorCode);
    //_tprintf(TEXT("Number of bytes:\t%x\n"), dwNumberOfBytesTransfered);
    //g_BytesTransferred = dwNumberOfBytesTransfered;
}

void DisplayError(LPTSTR lpszFunction)
// Routine Description:
// Retrieve and output the system error message for the last-error code
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);

    lpDisplayBuf =
        (LPVOID)LocalAlloc(LMEM_ZEROINIT,
            (lstrlen((LPCTSTR)lpMsgBuf)
                + lstrlen((LPCTSTR)lpszFunction)
                + 40) // account for format string
            * sizeof(TCHAR));

    if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error code %d as follows:\n%s"),
        lpszFunction,
        dw,
        lpMsgBuf)))
    {
        printf("FATAL ERROR: Unable to output error code.\n");
    }

    LOG("ERROR: %s\n", (LPCTSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

namespace shadercompiler
{
    bool compareAndReplace(const engine::string& temporaryPath, const engine::string& targetPath)
    {
        if (!engine::fileExists(temporaryPath))
            return false;

        auto temporaryFile = FileAccessSerializer::instance().readFile(temporaryPath, true);
        auto targetFile = FileAccessSerializer::instance().readFile(targetPath, true);
        bool different = false;

        // size comparison
        if (temporaryFile.size() != targetFile.size())
            different = true;

        // contents comparison
        if (!different && !std::equal(temporaryFile.begin(), temporaryFile.end(), targetFile.begin()))
            different = true;

        if (different)
        {
            // rename the new one
            //engine::fileRename(temporaryPath, targetPath);
            engine::fileCopy(temporaryPath, targetPath);
            engine::fileDelete(temporaryPath);

            FileAccessSerializer::instance().updateFile(temporaryPath, temporaryFile);
            FileAccessSerializer::instance().updateFile(targetPath, temporaryFile);

            return true;
        }
        return false;
    }

    FileAccessSerializer::FileAccessSerializer()
    {};

    FileAccessSerializer::~FileAccessSerializer()
    {
        for (auto&& fileLock : m_locks)
            delete fileLock.second;
        m_locks.clear();
    };

    engine::vector<char> FileAccessSerializer::readFile(const engine::string& file, bool clean)
    {
        if (!engine::fileExists(file))
            return {};

        engine::vector<char> result;
        FileLock* fileLock = nullptr;
        bool reading = false;

        // try to get the FileLock
        {
            std::lock_guard<std::mutex> lock(m_mapMutex);
            auto f = m_locks.find(file);
            if (f != m_locks.end())
            {
                fileLock = f->second;
            }
            else
            {
                fileLock = new FileLock{ file, {}, {} };
                m_locks[file] = fileLock;
                fileLock->mutex.lock();
                reading = true;
            }
        }

        // either read or fill the file
        {
            if (reading)
            {
                fileLock->data = internalReadFile(file, clean);
                result = fileLock->data;
                fileLock->mutex.unlock();
            }
            else
            {
                std::lock_guard<std::mutex> lock(fileLock->mutex);
                result = fileLock->data;
            }
        }

        return result;
    }

    void FileAccessSerializer::writeFile(const engine::string& file, const engine::vector<char>& data)
    {
        FileLock* fileLock = nullptr;
        bool filling = false;

        // try to get the FileLock
        {
            std::lock_guard<std::mutex> lock(m_mapMutex);
            auto f = m_locks.find(file);
            if (f != m_locks.end())
            {
                fileLock = f->second;
            }
            else
            {
                fileLock = new FileLock{ file, {}, {} };
                m_locks[file] = fileLock;
                fileLock->mutex.lock();
                filling = true;
            }
        }

        // either read or fill the file
        {
            if (filling)
            {
                // new file. never seen
                fileLock->data.resize(data.size());
                memcpy(fileLock->data.data(), data.data(), data.size());
                internalWriteFile(file, data);

                fileLock->mutex.unlock();
            }
            else
            {
                // existing file
                std::lock_guard<std::mutex> lock(fileLock->mutex);

                bool needToWriteToDisk = fileLock->data.size() != data.size();
                if (!needToWriteToDisk)
                    needToWriteToDisk = !std::equal(fileLock->data.begin(), fileLock->data.end(), data.begin());

                fileLock->data.resize(data.size());
                memcpy(fileLock->data.data(), data.data(), data.size());
                if(needToWriteToDisk)
                    internalWriteFile(file, data);
            }
        }
    }

    void FileAccessSerializer::updateFile(const engine::string& file, const engine::vector<char>& data)
    {
        FileLock* fileLock = nullptr;
        bool filling = false;

        // try to get the FileLock
        {
            std::lock_guard<std::mutex> lock(m_mapMutex);
            auto f = m_locks.find(file);
            if (f != m_locks.end())
            {
                fileLock = f->second;
            }
            else
            {
                fileLock = new FileLock{ file, {}, {} };
                m_locks[file] = fileLock;
                fileLock->mutex.lock();
                filling = true;
            }
        }

        // either read or fill the file
        {
            if (filling)
            {
                // new file. never seen
                fileLock->data.resize(data.size());
                memcpy(fileLock->data.data(), data.data(), data.size());

                fileLock->mutex.unlock();
            }
            else
            {
                // existing file
                std::lock_guard<std::mutex> lock(fileLock->mutex);

                fileLock->data.resize(data.size());
                memcpy(fileLock->data.data(), data.data(), data.size());
            }
        }
    }

    engine::vector<char> FileAccessSerializer::internalReadFile(const engine::string& file, bool clean)
    {
        std::fstream filestream;
        filestream.open(file);
        ASSERT(filestream.is_open(), "Failed to open preprocessed file! %s", file.c_str());

        filestream.seekg(0, std::ios::end);
        auto fileSize = static_cast<size_t>(filestream.tellg());
        filestream.seekg(0, std::ios::beg);
        
        if (clean)
        {
            engine::vector<char> data(fileSize);
            filestream.read(data.data(), fileSize);
            filestream.close();
            return data;
        }
        else
        {
            engine::vector<char> data(fileSize + 1);
            filestream.read(data.data(), fileSize);
            filestream.close();
            data[fileSize] = '\0';
            return data;
        }
    }

    void FileAccessSerializer::internalWriteFile(const engine::string& file, const engine::vector<char>& data)
    {
        std::ofstream fileStream;
        fileStream.open(file, std::ios::out);
        fileStream.write(data.data(), data.size());
        fileStream.close();
    }



    /*engine::vector<char> FileAccessSerializer::internalReadFile(const engine::string& file, bool clean)
    {
        HANDLE fileHandle;
        int failures = 0;
        do
        {
            fileHandle = CreateFile(file.c_str(),    // name of the write
                GENERIC_READ,                               // open for writing
                FILE_SHARE_READ | FILE_SHARE_DELETE,                          // delete access is required so renaming works nicely in multithreaded
                NULL,                                       // default security
                OPEN_EXISTING,                              // create new file only
                FILE_ATTRIBUTE_NORMAL,                      // normal file
                NULL);                                      // no attr. template
            if (fileHandle == INVALID_HANDLE_VALUE)
            {
                ++failures;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } while (fileHandle == INVALID_HANDLE_VALUE && failures < 1000);
        
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            DisplayError((LPTSTR)TEXT("CreateFile"));
            ASSERT(false, "Could not open file for writing: %s", file.c_str());
        }

        DWORD fileSize = GetFileSize(fileHandle, nullptr);
        if (clean)
        {
            engine::vector<char> data(fileSize);
            
            OVERLAPPED ol = { 0 };
            BOOL result = ReadFileEx(fileHandle, data.data(), fileSize, &ol, FileIOCompletionRoutine);
            if (result == FALSE)
            {
                ASSERT(false, "Could not read file contents: %s", file.c_str());
            }
            CloseHandle(fileHandle);
            return data;
        }
        else
        {
            engine::vector<char> data(fileSize + 1);

            OVERLAPPED ol = { 0 };
            BOOL result = ReadFileEx(fileHandle, data.data(), fileSize, &ol, FileIOCompletionRoutine);
            if (result == FALSE)
            {
                ASSERT(false, "Could not read file contents: %s", file.c_str());
            }
            data[fileSize] = '\0';

            CloseHandle(fileHandle);
            return data;
        }
    }

    void FileAccessSerializer::internalWriteFile(const engine::string& file, const engine::vector<char>& data)
    {
        HANDLE fileHandle = CreateFile(file.c_str(),    // name of the write
            GENERIC_WRITE,                              // open for writing
            FILE_SHARE_WRITE | FILE_SHARE_DELETE,                          // delete access is required so renaming works nicely in multithreaded
            NULL,                                       // default security
            CREATE_NEW,                                 // create new file only
            FILE_ATTRIBUTE_NORMAL,                      // normal file
            NULL);                                      // no attr. template
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            ASSERT(false, "Could not open file for writing: %s", file.c_str());
        }

        DWORD dwBytesToWrite = (DWORD)data.size();
        DWORD dwBytesWritten = 0;
        BOOL bErrorFlag = WriteFile(
            fileHandle,           // open file handle
            data.data(),      // start of data to write
            dwBytesToWrite,  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);            // no overlapped structure

        if ((FALSE == bErrorFlag) || (dwBytesWritten != dwBytesToWrite))
        {
            ASSERT(false, "Could not write file: %s", file.c_str());
        }

        CloseHandle(fileHandle);
    }
    */
}
