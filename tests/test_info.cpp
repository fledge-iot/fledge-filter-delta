#include <gtest/gtest.h>
#include <plugin_api.h>
#include <string.h>
#include <string>
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

extern "C" {
    PLUGIN_INFORMATION *plugin_info();
};

TEST(DELTA_INFO, PluginInfo)
{
    PLUGIN_INFORMATION *info = plugin_info();
    ASSERT_STREQ(info->name, "delta");
    ASSERT_STREQ(info->type, PLUGIN_TYPE_FILTER);
}

TEST(DELTA_INFO, PluginInfoConfigParse)
{
    PLUGIN_INFORMATION *info = plugin_info();
    Document doc;
    doc.Parse(info->config);
    ASSERT_EQ(doc.HasParseError(), false);
    ASSERT_EQ(doc.IsObject(), true);
    ASSERT_EQ(doc.HasMember("plugin"), true);
}
