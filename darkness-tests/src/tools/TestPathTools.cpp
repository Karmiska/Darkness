#include "gtest/gtest.h"
#include "tools/PathTools.h"
#include "platform/Environment.h"

using namespace engine;

TEST(TestPathTools, PathDelimiters)
{
    auto delims = pathDelimiters();
    EXPECT_GT(delims.size(), 0);
}

TEST(TestPathTools, PathExtractFolder)
{
    // workingDir = "C:\\work\\darkness\\darkness-tests\\bin\\win64\\debug\\darknesstests.exe"
    auto workingDir = engine::getExecutableDirectory();
    workingDir = workingDir.substr(0, workingDir.rfind("\\"));
    workingDir += "\\..\\..\\..\\data";
    static engine::string ValidFilePath = workingDir + "\\test.txt";
    static engine::string ValidFileName = "test.txt";
    static engine::string ValidFolderPathWithoutTD = workingDir + "";
    static engine::string ValidFolderPathWithTD = workingDir + "\\";

    // this needs to point
    string path1 = ValidFilePath;
    string path1A = ValidFolderPathWithoutTD;
    string path1B = ValidFolderPathWithTD;

    string path2 = "C:\\work\\text";
    string path3 = "C:\\work\\text\\";
    string path4 = "C:/work/test";
    string path5 = "C:/work/test";
    string path6 = "C:/work/test/";
    string path7 = "C:/work/test\\";
    string path8 = "..\\detail\\path";
    string path9 = "detail\\path";
    string path10 = "detail/path\\";
    string path11 = "\\detail/path\\..";

    EXPECT_EQ(pathExtractFolder(path1, false), ValidFolderPathWithoutTD);
    EXPECT_EQ(pathExtractFolder(path1, true), ValidFolderPathWithTD);

    EXPECT_EQ(pathExtractFolder(path1A, false), ValidFolderPathWithoutTD);
    EXPECT_EQ(pathExtractFolder(path1A, true), ValidFolderPathWithTD);

    EXPECT_EQ(pathExtractFolder(path1B, false), ValidFolderPathWithoutTD);
    EXPECT_EQ(pathExtractFolder(path1B, true), ValidFolderPathWithTD);

    EXPECT_EQ(pathExtractFolder(path2, false), "C:\\work");
    EXPECT_EQ(pathExtractFolder(path2, true), "C:\\work\\");

    EXPECT_EQ(pathExtractFolder(path3, false), "C:\\work\\text");
    EXPECT_EQ(pathExtractFolder(path3, true), "C:\\work\\text\\");

    EXPECT_EQ(pathExtractFolder(path4, false), "C:/work");
    EXPECT_EQ(pathExtractFolder(path4, true), "C:/work/");

    EXPECT_EQ(pathExtractFolder(path5, false), "C:/work");
    EXPECT_EQ(pathExtractFolder(path5, true), "C:/work/");

    EXPECT_EQ(pathExtractFolder(path6, false), "C:/work/test");
    EXPECT_EQ(pathExtractFolder(path6, true), "C:/work/test/");

    EXPECT_EQ(pathExtractFolder(path7, false), "C:/work/test");
    EXPECT_EQ(pathExtractFolder(path7, true), "C:/work/test/");

    EXPECT_EQ(pathExtractFolder(path8, false), "..\\detail");
    EXPECT_EQ(pathExtractFolder(path8, true), "..\\detail\\");

    EXPECT_EQ(pathExtractFolder(path9, false), "detail");
    EXPECT_EQ(pathExtractFolder(path9, true), "detail\\");

    EXPECT_EQ(pathExtractFolder(path10, false), "detail/path");
    EXPECT_EQ(pathExtractFolder(path10, true), "detail/path/");

    EXPECT_EQ(pathExtractFolder(path11, false), "\\detail\\path");
    EXPECT_EQ(pathExtractFolder(path11, true), "\\detail\\path\\");
}

TEST(TestPathTools, PathExtractFilename)
{
    string path1 = "C:\\work\\test.txt";
    string path2 = "C:\\work\\text.s";
    string path3 = "C:\\work\\text\\t.exts";
    string path4 = "C:/work/test..";
    string path5 = "C:/work\\..test";
    string path6 = "C:/work/test/empty";
    string path7 = "C:/work/test\\empty";
    string path8 = "..\\detail\\path";
    string path9 = "detail\\path";
    string path10 = "detail/path\\..";

    EXPECT_EQ(pathExtractFilename(path1), "test.txt");
    EXPECT_EQ(pathExtractFilename(path2), "text.s");
    EXPECT_EQ(pathExtractFilename(path3), "t.exts");
    EXPECT_EQ(pathExtractFilename(path4), "test..");
    EXPECT_EQ(pathExtractFilename(path5), "..test");
    EXPECT_EQ(pathExtractFilename(path6), "empty");
    EXPECT_EQ(pathExtractFilename(path7), "empty");
    EXPECT_EQ(pathExtractFilename(path8), "path");
    EXPECT_EQ(pathExtractFilename(path9), "path");
    EXPECT_EQ(pathExtractFilename(path10), "..");
}

TEST(TestPathTools, PathExtractExtension)
{
    string path1 = "some\\path\\file.ext";
    string path2 = "some\\path\\file.bat.ext.";
    string path3 = "some\\path\\file.bat.ext";
    string path4 = "some\\path\\file.batext";
    string path5 = "some\\path\\filebatext";

    EXPECT_EQ(pathExtractExtension(path1), "ext");
    EXPECT_EQ(pathExtractExtension(path2), "ext");
    EXPECT_EQ(pathExtractExtension(path3), "ext");
    EXPECT_EQ(pathExtractExtension(path4), "batext");
    EXPECT_EQ(pathExtractExtension(path5), "");
}

TEST(TestPathTools, PathJoinDefaultDelimiter)
{
    string pathA1 = "part_one";
    string pathB1 = "part_two";

    EXPECT_EQ(pathJoin(pathA1, pathB1, false), "part_one\\part_two");
    EXPECT_EQ(pathJoin(pathA1, pathB1, true), "part_one\\part_two\\");

    string pathA2 = "part_one\\";
    string pathB2 = "/part_two";

    EXPECT_EQ(pathJoin(pathA2, pathB2, false), "part_one\\part_two");
    EXPECT_EQ(pathJoin(pathA2, pathB2, true), "part_one\\part_two\\");

    string pathA3 = "/part_one";
    string pathB3 = "/part_two/three\\four\\";

    EXPECT_EQ(pathJoin(pathA3, pathB3, false), "/part_one/part_two/three/four");
    EXPECT_EQ(pathJoin(pathA3, pathB3, true), "/part_one/part_two/three/four/");
}

TEST(TestPathTools, PathJoinSetDelimiter)
{
    string pathA1 = "part_one";
    string pathB1 = "part_two";

    EXPECT_EQ(pathJoin(pathA1, pathB1, '/', false), "part_one/part_two");
    EXPECT_EQ(pathJoin(pathA1, pathB1, '/', true), "part_one/part_two/");

    string pathA2 = "part_one\\";
    string pathB2 = "/part_two";

    EXPECT_EQ(pathJoin(pathA2, pathB2, '/', false), "part_one/part_two");
    EXPECT_EQ(pathJoin(pathA2, pathB2, '/', true), "part_one/part_two/");

    string pathA3 = "/part_one";
    string pathB3 = "/part_two/three\\four\\";

    EXPECT_EQ(pathJoin(pathA3, pathB3, '\\', false), "\\part_one\\part_two\\three\\four");
    EXPECT_EQ(pathJoin(pathA3, pathB3, '\\', true), "\\part_one\\part_two\\three\\four\\");
}

TEST(TestPathTools, PathJoinList)
{
	engine::vector<string> parts{ "one", "two", "three" };

    EXPECT_EQ(pathJoin(parts), "one\\two\\three");
    EXPECT_EQ(pathJoin(parts, '/'), "one/two/three");
    EXPECT_EQ(pathJoin(parts, '\\', true), "one\\two\\three\\");
    EXPECT_EQ(pathJoin(parts, '\\', false), "one\\two\\three");
    EXPECT_EQ(pathJoin(parts, '\\', true), "one\\two\\three\\");
    EXPECT_EQ(pathJoin(parts, '/', false), "one/two/three");
    EXPECT_EQ(pathJoin(parts, '/', true), "one/two/three/");
}

TEST(TestPathTools, PathSplit)
{
    string path = "/../test/blaa/just\\jeah\\";

    auto split = pathSplit(path);
    EXPECT_EQ(split.size(), 5);
    EXPECT_EQ(split[0], "..");
    EXPECT_EQ(split[1], "test");
    EXPECT_EQ(split[2], "blaa");
    EXPECT_EQ(split[3], "just");
    EXPECT_EQ(split[4], "jeah");
}
