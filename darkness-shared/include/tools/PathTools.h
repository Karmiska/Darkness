#pragma once

#include "tools/StringTools.h"
#include "tools/Debug.h"

namespace engine
{
    bool pathExists(const engine::string& path);
    bool pathIsFolder(const engine::string& path);
    bool pathIsFile(const engine::string& path);
    bool pathStartsWithDelimiter(const engine::string& path);
    bool pathEndsWithDelimiter(const engine::string& path);
    engine::string pathRemoveTrailingDelimiter(const engine::string& path);

    engine::vector<char> pathDelimiters();
    char pathPreferredDelim();
    char pathPreferredDelim(const engine::string& path);
    engine::string pathExtractFolder(const engine::string& filepath, bool withLastDelimiter = false);
    engine::string pathExtractFolderName(const engine::string& filepath);
    engine::string pathExtractFilename(const engine::string& filepath);
    engine::string pathExtractFilenameWithoutExtension(const engine::string& filepath);
    engine::string pathExtractExtension(const engine::string& filepath);
    engine::string pathJoin(const engine::string& pathA, const engine::string& pathB, char delimiter, bool withLastDelimiter = false);
    engine::string pathJoin(const engine::string& pathA, const engine::string& pathB, bool withLastDelimiter = false);
    engine::string pathJoin(engine::vector<engine::string> parts, char delimiter = pathDelimiters()[0], bool withLastDelimiter = false);
    engine::vector<engine::string> pathSplit(const engine::string& path);
    engine::string pathReplaceExtension(const engine::string& path, const engine::string& newExtension);
    engine::string pathCommonAncestor(const engine::string& pathA, const engine::string& pathB, bool withLastDelimiter = false);
    engine::string pathSubtractCommon(const engine::string& pathA, const engine::string& pathB);
    engine::string pathClean(const engine::string& path);
    engine::string pathReplaceAllDelimiters(const engine::string& path, const engine::string& delimiter);

    engine::string getWorkingDirectory(bool withLastDelimiter = false);

    engine::vector<engine::string> getAllFilesRecursive(const engine::string& path);
}

