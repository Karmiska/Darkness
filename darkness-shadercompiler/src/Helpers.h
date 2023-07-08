#pragma once

#include "containers/string.h"
#include <mutex>
#include "containers/unordered_map.h"
#include "containers/vector.h"
#include "tools/Debug.h"

namespace shadercompiler
{
    bool compareAndReplace(const engine::string& temporaryPath, const engine::string& targetPath);

    class FileAccessSerializer
    {
    public:
        FileAccessSerializer();
        ~FileAccessSerializer();

        engine::vector<char> readFile(const engine::string& file, bool clean = false);
        void writeFile(const engine::string& file, const engine::vector<char>& data);
        void updateFile(const engine::string& file, const engine::vector<char>& data);
    public:
        static FileAccessSerializer& instance()
        {
            static FileAccessSerializer _instance;
            return _instance;
        };

    private:
        struct FileLock
        {
            engine::string file;
            engine::vector<char> data;
            std::mutex mutex;
        };
        std::mutex m_mapMutex;
        engine::unordered_map<engine::string, FileLock*> m_locks;

        engine::vector<char> internalReadFile(const engine::string& file, bool clean);
        void internalWriteFile(const engine::string& file, const engine::vector<char>& data);
    };
}
