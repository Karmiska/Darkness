#include "tools/CompressedFileZstd.h"
#include "tools/Debug.h"
#include "zstd.h"

using namespace engine;

CompressedFileZstd::CompressedFileZstd()
    : m_filename{ "" }
    , m_mode{ 0 }
    , m_eof{ false }
    , m_filePosition{ 0 }
    , m_memoryFile{ false }
{
}

void CompressedFileZstd::open(const string& filename, int mode)
{
    m_fileContents = engine::make_unique<engine::vector<char>>();
    m_fileContentsActive = m_fileContents.get();
    m_filename = filename;
    m_mode = mode;
    m_file.open(filename.c_str(), mode);
    if (((mode & std::ios::in) == std::ios::in) && m_file.is_open())
    {
		std::ifstream file;
        file.open(filename.c_str(), mode);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            m_fileContentsActive->resize(static_cast<size_t>(file.tellg()));
            file.seekg(0, std::ios::beg);
            file.read(m_fileContentsActive->data(), static_cast<std::streamsize>(m_fileContentsActive->size()));
            file.close();
        }

        decode();
    }
}

void CompressedFileZstd::open(engine::vector<char>& memory, int mode)
{
    m_mode = mode;
    m_memoryFile = true;
    m_fileContentsActive = &memory;
    if (((mode & std::ios::in) == std::ios::in) && memory.size() > 0)
    {
        decode();
    }
}

bool CompressedFileZstd::is_open() const
{
    if (m_memoryFile)
        return true;
    return m_file.is_open();
}

void CompressedFileZstd::read(char* buffer, std::streamsize count)
{
	std::streamsize bytesToRead = count;
    if (count > static_cast<std::streamsize>(m_fileContentsActive->size() - m_filePosition))
    {
        m_eof = true;
        bytesToRead = static_cast<std::streamsize>(m_fileContentsActive->size() - m_filePosition);
    }
    if (bytesToRead > 0)
        memcpy(buffer, &(*m_fileContentsActive)[m_filePosition], static_cast<size_t>(bytesToRead));
    m_filePosition += static_cast<size_t>(bytesToRead);
}

void CompressedFileZstd::write(const char* buffer, std::streamsize count)
{
    if (count == 0)
        return;

    if (m_fileContentsActive->size() < m_filePosition + count)
    {
        m_fileContentsActive->resize(m_filePosition + static_cast<size_t>(count));
    }
    memcpy(&(*m_fileContentsActive)[m_filePosition], buffer, static_cast<size_t>(count));
    m_filePosition += static_cast<size_t>(count);
}

void CompressedFileZstd::seekg(std::streampos off, std::ios_base::seekdir way)
{
    if (way == std::ios::cur)
        m_filePosition += static_cast<size_t>(off);
    else if (way == std::ios::beg)
        m_filePosition = static_cast<size_t>(off);
    else if (way == std::ios::end)
    {
        if (static_cast<size_t>(off) < m_fileContentsActive->size())
            m_filePosition = m_fileContentsActive->size() - static_cast<size_t>(off);
        else
            m_filePosition = 0;
    }
}

size_t CompressedFileZstd::tellg()
{
    return m_filePosition;
}

void CompressedFileZstd::clear()
{
    m_eof = false;
}

void CompressedFileZstd::close()
{
    if ((m_mode & std::ios::out) == std::ios::out)
    {
        encode();
    }
    if(!m_memoryFile)
        m_file.close();
}

bool CompressedFileZstd::eof() const
{
    return m_eof;
}

void CompressedFileZstd::decode()
{
    unsigned long long const rSize = ZSTD_getDecompressedSize(m_fileContentsActive->data(), m_fileContentsActive->size());
	engine::vector<char> eBuff(static_cast<size_t>(rSize), 0);

    size_t const dSize = ZSTD_decompress(eBuff.data(), static_cast<size_t>(rSize), m_fileContentsActive->data(), m_fileContentsActive->size());

    ASSERT(dSize == rSize);

    m_fileContentsActive->swap(eBuff);
}

void CompressedFileZstd::encode()
{
    size_t const cBuffSize = ZSTD_compressBound(m_fileContentsActive->size());
	engine::vector<char> cBuff(cBuffSize, 0);

    size_t const cSize = ZSTD_compress(&cBuff[0], cBuffSize, &(*m_fileContentsActive)[0], m_fileContentsActive->size(), 9);
    ASSERT(!ZSTD_isError(cSize));
    if (!m_memoryFile)
        m_file.write(&cBuff[0], static_cast<std::streamsize>(cSize));
    else
    {
        m_fileContentsActive->resize(cSize);
        memcpy(&(*m_fileContentsActive)[0], &cBuff[0], cSize);
    }
}
