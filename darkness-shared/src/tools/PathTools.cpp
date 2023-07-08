#include "tools/PathTools.h"
#include "platform/Platform.h"
#include "platform/Directory.h"

#include <filesystem>

using namespace std;
using namespace engine;

namespace engine
{
    vector<char> pathDelimiters()
    {
        return { '\\','/' };
    }

    char pathPreferredDelim()
    {
        return pathDelimiters()[0];
    }

    char pathPreferredDelim(const string& path)
    {
        auto delims = pathDelimiters();
        char pathDelim = delims[0];
        size_t lowestDelimIndex = path.size();
        for (auto&& delim : delims)
        {
            auto index = path.find(delim);
            if (index != string::npos)
            {
                if (index < lowestDelimIndex)
                {
                    pathDelim = delim;
                    lowestDelimIndex = index;
                }
            }
        }
        return pathDelim;
    }

	std::wstring toWideString(const string& str)
    {
#ifdef _WIN32
        auto lastError = GetLastError();
        // find out the size for target buffer
        SetLastError(0);
        auto wideCharacters = MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            str.data(), -1, nullptr, 0);
        if (wideCharacters == 0)
        {
            auto err = GetLastError();
            switch (err)
            {
            case ERROR_INSUFFICIENT_BUFFER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INSUFFICIENT_BUFFER");
            case ERROR_INVALID_FLAGS: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_FLAGS");
            case ERROR_INVALID_PARAMETER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_PARAMETER");
            case ERROR_NO_UNICODE_TRANSLATION: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_NO_UNICODE_TRANSLATION");
            }
        }

        vector<wchar_t> wstring(wideCharacters);

        // convert
        auto charactersWritten = MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            str.data(),
            -1,  // assuming null terminated string
            wstring.data(),
            static_cast<int>(wstring.size()));
        if (charactersWritten == 0)
        {
            auto err = GetLastError();
            switch (err)
            {
            case ERROR_INSUFFICIENT_BUFFER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INSUFFICIENT_BUFFER");
            case ERROR_INVALID_FLAGS: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_FLAGS");
            case ERROR_INVALID_PARAMETER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_PARAMETER");
            case ERROR_NO_UNICODE_TRANSLATION: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_NO_UNICODE_TRANSLATION");
            }
        }
        SetLastError(lastError);

        return wstring.data();
#else
        ASSERT(false, "toWideString is NOT IMPLEMENTED ON THIS PLATFORM");
#endif
    }

    string toUtf8String(const std::wstring& str)
    {
#ifdef _WIN32
        auto lastError = GetLastError();
        // find out the size for target buffer
        SetLastError(0);
        auto utf8Characters = WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            str.data(), -1,
            nullptr, 0,
            nullptr, nullptr);
        if (utf8Characters == 0)
        {
            auto err = GetLastError();
            switch (err)
            {
            /*case ERROR_INSUFFICIENT_BUFFER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INSUFFICIENT_BUFFER");
            case ERROR_INVALID_FLAGS: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_FLAGS");
            case ERROR_INVALID_PARAMETER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_PARAMETER");
            case ERROR_NO_UNICODE_TRANSLATION: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_NO_UNICODE_TRANSLATION");
            */
            case ERROR_INSUFFICIENT_BUFFER: return "";
            case ERROR_INVALID_FLAGS: return "";
            case ERROR_INVALID_PARAMETER: return "";
            case ERROR_NO_UNICODE_TRANSLATION: return "";
            }
        }

        vector<char> utf8str(utf8Characters);

        // convert
        auto charactersWritten = WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            str.data(), -1,
            utf8str.data(), static_cast<int>(utf8str.size()),
            nullptr, nullptr);
        if (charactersWritten == 0)
        {
            auto err = GetLastError();
            switch (err)
            {
            case ERROR_INSUFFICIENT_BUFFER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INSUFFICIENT_BUFFER");
            case ERROR_INVALID_FLAGS: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_FLAGS");
            case ERROR_INVALID_PARAMETER: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_INVALID_PARAMETER");
            case ERROR_NO_UNICODE_TRANSLATION: ASSERT(false, "Could not convert filepath to widechar. Error: ERROR_NO_UNICODE_TRANSLATION");
            }
        }
        SetLastError(lastError);

        return utf8str.data();
#else
        ASSERT(false, "toUtf8String is NOT IMPLEMENTED ON THIS PLATFORM");
#endif
    }

    bool pathExists(const string& path)
    {
        try
        {
            return std::filesystem::exists(path.c_str());
        }
        catch (std::filesystem::filesystem_error error)
        {
            LOG_ERROR("Filesystem threw error while processing path: %s, error: %s", path.c_str(), error.what());
            return false;
        }
    }

    bool pathIsFolder(const string& path)
    {
#ifdef _WIN32
        if (pathExists(path))
        {
#ifdef UNICODE
            auto wstring = toWideString(path);
            return (GetFileAttributes(wstring.data()) &
                FILE_ATTRIBUTE_DIRECTORY) ==
                FILE_ATTRIBUTE_DIRECTORY;
#else
            return (GetFileAttributes(path.data()) &
                FILE_ATTRIBUTE_DIRECTORY) ==
                FILE_ATTRIBUTE_DIRECTORY;
#endif
        }
        else
        {
            if (pathEndsWithDelimiter(path))
                return true;
            return false;
        }
#else
        ASSERT(false, "pathIsFolder is NOT IMPLEMENTED ON THIS PLATFORM");
#endif
    }

    bool pathIsFile(const string& path)
    {
        return !pathIsFolder(path);
    }

    bool pathStartsWithDelimiter(const string& path)
    {
        for (auto&& delim : pathDelimiters())
        {
            if (path.find(delim) == 0)
                return true;
        }
        return false;
    }

    bool pathEndsWithDelimiter(const string& path)
    {
        for (auto&& delim : pathDelimiters())
        {
            if (path[path.size() - 1] == delim)
                return true;
        }
        return false;
    }

    string pathRemoveTrailingDelimiter(const string& path)
    {
        if (!pathEndsWithDelimiter(path))
            return path;

        auto delim = pathPreferredDelim(path);
        auto tokens = tokenize(path, pathDelimiters());
        if (tokens.size() > 0)
            return pathJoin(tokens, delim);
        else
            return "";
    }

    string pathExtractFolder(const string& filepath, bool withLastDelimiter)
    {
        auto delims = pathDelimiters();
        auto tokens = tokenize(filepath, delims);
        string res = "";

        auto prefDelim = pathPreferredDelim(filepath);

        string joined = "";
        if (pathIsFile(filepath))
            joined = pathJoin(vector<string>(tokens.begin(), tokens.end() - 1), prefDelim);
        else
            joined = pathJoin(vector<string>(tokens.begin(), tokens.end()), prefDelim);

        if (pathStartsWithDelimiter(filepath))
            res += prefDelim + joined;
        else
            res += joined;
        if (withLastDelimiter)
            res += prefDelim;
        return res;
    }

    engine::string pathExtractFolderName(const engine::string& filepath)
    {
        auto folder = pathExtractFolder(filepath);
        return pathSplit(folder).back();
    }

    string pathExtractFilename(const string& filepath)
    {
        auto delims = pathDelimiters();
        auto tokens = tokenize(filepath, delims);
        return tokens.back();
    }

    string pathExtractFilenameWithoutExtension(const string& filepath)
    {
        auto filename = pathExtractFilename(filepath);
        auto ext = pathExtractExtension(filename);
        return filename.substr(0, filename.length() - ext.length() - 1);
    }

    string pathExtractExtension(const string& filepath)
    {
        auto filename = pathExtractFilename(filepath);
        auto tokens = tokenize(filename, { '.' });
        if (tokens.size() == 1)
            return "";
        else
            return tokens.back();
    }

    string pathJoin(const string& pathA, const string& pathB, bool withLastDelimiter)
    {
        auto delim = pathPreferredDelim(pathA);
        return pathJoin(pathA, pathB, delim, withLastDelimiter);
    }

    string pathJoin(const string& pathA, const string& pathB, char delimiter, bool withLastDelimiter)
    {
        auto pathASplit = pathSplit(pathA);
        auto pathBSplit = pathSplit(pathB);

        auto pAjoin = pathJoin(pathASplit, delimiter, withLastDelimiter);
        auto pBjoin = pathJoin(pathBSplit, delimiter, withLastDelimiter);

        if(pathStartsWithDelimiter(pathA))
            return delimiter + pathJoin({ pAjoin, pBjoin }, delimiter, withLastDelimiter);
        else
            return pathJoin({ pAjoin, pBjoin }, delimiter, withLastDelimiter);
    }

    string pathJoin(vector<string> parts, char delimiter, bool withLastDelimiter)
    {
        if (parts.size() == 0)
            return "";

        if (parts.size() == 1)
        {
            if (withLastDelimiter)
                return parts[0] + delimiter;
            else
                return parts[0];
        }

        string result = "";
        if (pathStartsWithDelimiter(parts[0]))
            result += delimiter;

        auto part = parts.begin();
        do
        {
            if (*part != "")
            {
                result += pathRemoveTrailingDelimiter(*part);
                part++;
                if (part != parts.end())
                    result += delimiter;
            }
            else
                part++;
        } while (part != parts.end());
        if (withLastDelimiter)
            result += delimiter;
        return result;
    }

    vector<string> pathSplit(const string& path)
    {
        return tokenize(path, pathDelimiters());
    }

    string pathReplaceExtension(const string& path, const string& newExtension)
    {
        auto folder = pathExtractFolder(path);
        auto filename = pathExtractFilename(path);
        auto extension = pathExtractExtension(filename);
        auto filenameWithoutExt = filename.substr(0, filename.size() - extension.size());
        return pathJoin(folder, filenameWithoutExt + newExtension);
    }

    string pathCommonAncestor(const string& pathA, const string& pathB, bool withLastDelimiter)
    {
        vector<string> common;
        auto a = pathSplit(pathA);
        auto b = pathSplit(pathB);
        int i = 0;
        while (i < a.size() && i < b.size() && a[i] == b[i])
        {
            common.emplace_back(a[i]);
            ++i;
        }
        return pathJoin(common, pathPreferredDelim(pathA), withLastDelimiter);
    }

    engine::string pathSubtractCommon(const engine::string& pathA, const engine::string& pathB)
    {
        vector<string> common;
        auto a = pathSplit(pathA);
        auto b = pathSplit(pathB);
        int i = 0;
        while (i < a.size() && i < b.size() && a[i] == b[i])
        {
            ++i;
        }

        if (a.size() > i)
        {
            vector<string> rest(a.begin() + i, a.end());
            return pathJoin(rest, pathPreferredDelim(pathA));
        }
        if (b.size() > i)
        {
            vector<string> rest(b.begin() + i, b.end());
            return pathJoin(rest, pathPreferredDelim(pathB));
        }
        return "";
    }

    engine::string pathClean(const engine::string& path)
    {
        auto delim = pathPreferredDelim(path);
        auto tokens = tokenize(path, pathDelimiters());
        bool done = false;
        while (!done)
        {
            done = true;
            int removeIndex = -1;
            for (int i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i] == ".." && i > 0)
                {
                    removeIndex = i - 1;
                    break;
                }
            }
            if (removeIndex != -1)
            {
                done = false;
                tokens.erase(tokens.begin() + removeIndex, tokens.begin() + removeIndex + 2);
            }
        }
        return pathJoin(tokens, delim);
    }

    engine::string pathReplaceAllDelimiters(const engine::string& path, const engine::string& delimiter)
    {
        auto tokens = pathSplit(path);
        engine::string result = "";
        for (auto token = tokens.begin(); token != tokens.end(); ++token)
            if (std::next(token) != tokens.end())
                result += *token + delimiter;
            else
                result += *token;
        return result;
    }

    string getWorkingDirectory(bool 
#ifdef _WIN32
        withLastDelimiter
#endif
    )
    {
#ifdef _WIN32
#ifdef UNICODE
        wchar_t path[1024];
        memset(&path[0], 0, 1024 * sizeof(wchar_t));
        DWORD res = GetCurrentDirectoryW(1024, &path[0]);
        ASSERT(res != 0, "Could not get current directory");
		std::wstring temp(path);
        auto pathRes = toUtf8String(temp);
        if (withLastDelimiter && !pathEndsWithDelimiter(pathRes))
            pathRes += pathPreferredDelim(pathRes);
        return pathRes;
#else
        char path[1024];
        memset(&path[0], 0, 1024);
        DWORD res = GetCurrentDirectory(1024, &path[0]);
        ASSERT(res != 0, "Could not get current directory");
        string temp(path);
        if (withLastDelimiter && !pathEndsWithDelimiter(temp))
            temp += pathPreferredDelim(temp);
        return temp;
#endif
#else
        ASSERT(false, "getWorkingDirectory is not implemented on this platform");
#endif
    }

    engine::vector<engine::string> getAllFilesRecursive(const engine::string& path)
    {
        ASSERT(pathIsFolder(path), "Tried to gather from file instead of folder");

        vector<string> result;
        Directory directory(path);
        for (auto&& dir : directory.directories())
        {
            auto contents = getAllFilesRecursive(pathJoin(path, dir));
            result.insert(result.end(), contents.begin(), contents.end());
        }
        auto files = directory.files();
        for (auto&& file : files)
        {
            result.emplace_back(pathJoin(path, file));
        }
        return result;
    }
}
