#include "gtest/gtest.h"
#include "tools/PathTools.h"
#include "tools/Settings.h"

using namespace engine;
using namespace tools;

TEST(TestSettings, Settings_JsonBackend)
{
    constexpr int TestVectorSize = 10;
    // save some settings
    {
        Settings settings("C:\\work\\SettingsTest.dat");
        settings.set("testInt", 12);
        settings.set("testString", engine::string("test string"));
        settings.set("testFloat", 1.234f);

        engine::vector<int> testVector(10);
        for (int i = 0; i < TestVectorSize; ++i) { testVector[i] = i; }
        settings.set("testVector", testVector);
    }

    // load those settings
    {
        Settings settings("C:\\work\\SettingsTest.dat");
        EXPECT_EQ(settings.get<int>("testInt"), 12);
        EXPECT_EQ(settings.get<engine::string>("testString"), engine::string("test string"));
        EXPECT_EQ(settings.get<float>("testFloat"), 1.234f);

        auto testVector = settings.get<engine::vector<int>>("testVector");
        EXPECT_EQ(testVector.size(), TestVectorSize);
    
        for (int i = 0; i < TestVectorSize; ++i)
        {
            if(i < testVector.size())
                EXPECT_EQ(testVector[i], i);
        }
    }
}

TEST(TestSettings, Settings_Groups)
{
    {
        Settings settings("C:\\work\\SettingsGroupTest.dat");
        
        settings.beginGroup("testGroup1");
        settings.set("group1 value1", 123);
        settings.endGroup();
        
        settings.beginGroup("testGroup2");
        settings.set("group2 value1", engine::string("123"));
        settings.endGroup();

        settings.beginGroup("empty group");
        settings.endGroup();

        settings.beginGroup("deep group level 0");
            settings.beginGroup("deep group level 1");
                settings.beginGroup("deep group level 2");
                    settings.beginGroup("deep group level 3");
                    settings.set("some value", engine::string("abc"));
                    settings.endGroup();
                settings.endGroup();
            settings.endGroup();
        settings.endGroup();
    }

    {
        Settings settings("C:\\work\\SettingsGroupTest.dat");
        
        auto groups = settings.groups();
        EXPECT_EQ(groups.size(), 4);

        settings.beginGroup("testGroup1");
        EXPECT_EQ(settings.get<int>("group1 value1"), 123);
        settings.endGroup();
        
        settings.beginGroup("testGroup2");
        EXPECT_EQ(settings.get<engine::string>("group2 value1"), engine::string("123"));
        settings.endGroup();

        settings.beginGroup("empty group");
        EXPECT_EQ(settings.keys().size(), 0);
        settings.endGroup();

        settings.beginGroup("deep group level 0");
            settings.beginGroup("deep group level 1");
                settings.beginGroup("deep group level 2");
                    settings.beginGroup("deep group level 3");
                    EXPECT_EQ(settings.keys().size(), 1);
                    EXPECT_EQ(settings.get<engine::string>("some value"), engine::string("abc"));
                    settings.endGroup();
                settings.endGroup();
            settings.endGroup();
        settings.endGroup();
    }

}
