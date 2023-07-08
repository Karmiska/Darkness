#pragma once

#include "CompressedFileIf.h"
#include <iostream>
#include <fstream>
#include "containers/string.h"
#include "containers/vector.h"
#include "containers/memory.h"

class CompressedFileZstd : public CompressedFileIf
{
public:
    CompressedFileZstd();

    void open(const engine::string& filename, int mode) override;
    void open(engine::vector<char>& memory, int mode) override;
    bool is_open() const override;

    void read(char* buffer, std::streamsize count) override;
    void write(const char* buffer, std::streamsize count) override;
    
    void close() override;
    void clear() override;

    void seekg(std::streampos off, std::ios_base::seekdir way) override;
    size_t tellg() override;

    bool eof() const override;
private:
    std::fstream m_file;
    engine::string m_filename;
    int m_mode;
    bool m_memoryFile;

    engine::unique_ptr<engine::vector<char>> m_fileContents;
    engine::vector<char>* m_fileContentsActive;
    bool m_eof;
    size_t m_filePosition;

    void encode();
    void decode();
};
