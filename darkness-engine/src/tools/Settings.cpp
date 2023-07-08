#include "tools/Settings.h"
#include "tools/Debug.h"
#include "containers/unordered_map.h"
#include <algorithm>
#include <fstream>

#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using namespace tools::implementation;
using namespace rapidjson;
using namespace engine;

namespace tools
{
    namespace implementation
    {
        SettingsContainer::SettingsContainer(const string& groupName)
            : m_parent{ nullptr }
            , m_groupName{ groupName }
        {}

        engine::vector<string> SettingsContainer::keys() const
        {
			engine::vector<string> res;
            for (auto&& keyValPair : m_ints) { res.emplace_back(keyValPair.first); }
            for (auto&& keyValPair : m_intVectors) { res.emplace_back(keyValPair.first); }
            for (auto&& keyValPair : m_floats) { res.emplace_back(keyValPair.first); }
            for (auto&& keyValPair : m_floatVectors) { res.emplace_back(keyValPair.first); }
            for (auto&& keyValPair : m_strings) { res.emplace_back(keyValPair.first); }
            for (auto&& keyValPair : m_stringVectors) { res.emplace_back(keyValPair.first); }
            return res;
        }

		engine::vector<string> SettingsContainer::groups() const
        {
			engine::vector<string> res;
            for (auto&& child : m_childs) { res.emplace_back(child.second->m_groupName); }
            return res;
        }

        void SettingsContainer::readJson(void* _obj)
        {
            rapidjson::Value& doc = *static_cast<rapidjson::Value*>(_obj);
            if (!doc.IsObject())
            {
                LOG("Settings container was malformed");
                return;
            }

            if (doc.HasMember("groupName") && doc["groupName"].IsString())
            {
                m_groupName = string(doc["groupName"].GetString(), doc["groupName"].GetStringLength());
            }

            for (auto member = doc.MemberBegin(); member != doc.MemberEnd(); ++member)
            {
                engine::string name(member->name.GetString(), member->name.GetStringLength());
                if (name != "groupName" &&
                    name != "childs")
                {
                    const Value& list = doc[name.data()];
                    for (SizeType i = 0; i < list.Size(); ++i)
                    {
                        auto obj = list[i].GetObject();
                        for (auto objmember = obj.MemberBegin(); objmember != obj.MemberEnd(); ++objmember)
                        {
                            auto memberName = engine::string(objmember->name.GetString(), objmember->name.GetStringLength());
                            if (name == "bool") m_bools[memberName] = static_cast<bool>(objmember->value.GetBool());
                            if (name == "char") m_chars[memberName] = static_cast<char>(objmember->value.GetInt());
                            if (name == "short") m_shorts[memberName] = static_cast<short>(objmember->value.GetInt());
                            if (name == "int") m_ints[memberName] = objmember->value.GetInt();
                            if (name == "int64") m_bigints[memberName] = objmember->value.GetInt64();
                            if (name == "uchar") m_uchars[memberName] = static_cast<unsigned char>(objmember->value.GetUint());
                            if (name == "ushort") m_ushorts[memberName] = static_cast<unsigned short>(objmember->value.GetUint());
                            if (name == "uint") m_uints[memberName] = objmember->value.GetUint();
                            if (name == "uint64") m_biguints[memberName] = objmember->value.GetUint64();
                            if (name == "float") m_floats[memberName] = objmember->value.GetFloat();
                            if (name == "double") m_doubles[memberName] = objmember->value.GetDouble();
                            if (name == "string") { m_strings[memberName] = engine::string(objmember->value.GetString(), objmember->value.GetStringLength()); }
                        }
                    }
                }
            }

            for (auto member = doc.MemberBegin(); member != doc.MemberEnd(); ++member)
            {
                engine::string name(member->name.GetString(), member->name.GetStringLength());
                if (name != "groupName" &&
                    name != "childs")
                {
                    const Value& list = doc[name.data()];
                    for (SizeType i = 0; i < list.Size(); ++i)
                    {
                        auto obj = list[i].GetObject();
                        for (auto objmember = obj.MemberBegin(); objmember != obj.MemberEnd(); ++objmember)
                        {
                            if (objmember->value.IsArray())
                            {
                                auto memberName = engine::string(objmember->name.GetString(), objmember->name.GetStringLength());
                                auto arr = objmember->value.GetArray();
                                if (name == "vector_bool") { engine::vector<bool> cur;                for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetBool()); } m_boolVectors[memberName] = cur; }
                                if (name == "vector_char") { engine::vector<char> cur;                for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(static_cast<char>(arr[a].GetInt())); } m_charVectors[memberName] = cur; }
                                if (name == "vector_short") { engine::vector<short> cur;            for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(static_cast<short>(arr[a].GetInt())); } m_shortVectors[memberName] = cur; }
                                if (name == "vector_int") { engine::vector<int> cur;                for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetInt()); } m_intVectors[memberName] = cur; }
                                if (name == "vector_int64") { engine::vector<int64_t> cur;            for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetInt64()); } m_bigintVectors[memberName] = cur; }
                                if (name == "vector_uchar") { engine::vector<unsigned char> cur;    for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(static_cast<unsigned char>(arr[a].GetUint())); } m_ucharVectors[memberName] = cur; }
                                if (name == "vector_ushort") { engine::vector<unsigned short> cur;    for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(static_cast<unsigned short>(arr[a].GetUint())); } m_ushortVectors[memberName] = cur; }
                                if (name == "vector_uint") { engine::vector<unsigned int> cur;        for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetUint()); } m_uintVectors[memberName] = cur; }
                                if (name == "vector_uint64") { engine::vector<uint64_t> cur;            for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetUint64()); } m_biguintVectors[memberName] = cur; }
                                if (name == "vector_float") { engine::vector<float> cur;            for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetFloat()); } m_floatVectors[memberName] = cur; }
                                if (name == "vector_double") { engine::vector<double> cur;            for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(arr[a].GetDouble()); } m_doubleVectors[memberName] = cur; }
                                if (name == "vector_string") { engine::vector<engine::string> cur;        for (SizeType a = 0; a < arr.Size(); ++a) { cur.emplace_back(engine::string(arr[a].GetString(), arr[a].GetStringLength())); } m_stringVectors[memberName] = cur; }
                            }
                        }
                    }
                }
            }

            if (doc.HasMember("childs"))
            {
                const Value& list = doc["childs"];
                for (SizeType i = 0; i < list.Size(); ++i)
                {
                    if (list[i].HasMember("groupName") && list[i]["groupName"].IsString())
                    {
                        auto groupName = string(list[i]["groupName"].GetString(),
                                                     list[i]["groupName"].GetStringLength());
                        m_childs[groupName] = engine::make_shared<SettingsContainer>(groupName);
                        m_childs[groupName]->m_parent = shared_from_this();
                        m_childs[groupName]->readJson(&const_cast<Value&>(list[i]));
                    }
                }
            }
        }

        void SettingsContainer::writeJson(void* _writer)
        {
            rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer = *static_cast<rapidjson::PrettyWriter<rapidjson::StringBuffer>*>(_writer);
            writer.StartObject();

            writer.Key("groupName");
            writer.String(m_groupName.data());

            if (m_bools.size() > 0)     { writer.Key("bool");    writer.StartArray(); for (auto&& value : m_bools)   { writer.StartObject(); writer.Key(value.first.data()); writer.Bool(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_chars.size() > 0)        { writer.Key("char");    writer.StartArray(); for (auto&& value : m_chars)    { writer.StartObject(); writer.Key(value.first.data()); writer.Int(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_shorts.size() > 0)    { writer.Key("short");    writer.StartArray(); for (auto&& value : m_shorts)    { writer.StartObject(); writer.Key(value.first.data()); writer.Int(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_ints.size() > 0)        { writer.Key("int");    writer.StartArray(); for (auto&& value : m_ints)    { writer.StartObject(); writer.Key(value.first.data()); writer.Int(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_bigints.size() > 0)    { writer.Key("int64");    writer.StartArray(); for (auto&& value : m_bigints)    { writer.StartObject(); writer.Key(value.first.data()); writer.Int64(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_uchars.size() > 0)    { writer.Key("uchar");    writer.StartArray(); for (auto&& value : m_uchars)    { writer.StartObject(); writer.Key(value.first.data()); writer.Uint(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_ushorts.size() > 0)    { writer.Key("ushort"); writer.StartArray(); for (auto&& value : m_ushorts) { writer.StartObject(); writer.Key(value.first.data()); writer.Uint(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_uints.size() > 0)        { writer.Key("uint");    writer.StartArray(); for (auto&& value : m_uints)    { writer.StartObject(); writer.Key(value.first.data()); writer.Uint(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_biguints.size() > 0)    { writer.Key("uint64");    writer.StartArray(); for (auto&& value : m_biguints){ writer.StartObject(); writer.Key(value.first.data()); writer.Uint64(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_floats.size() > 0)    { writer.Key("float");    writer.StartArray(); for (auto&& value : m_floats)    { writer.StartObject(); writer.Key(value.first.data()); writer.Double(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_doubles.size() > 0)    { writer.Key("double"); writer.StartArray(); for (auto&& value : m_doubles) { writer.StartObject(); writer.Key(value.first.data()); writer.Double(value.second); writer.EndObject(); } writer.EndArray(); }
            if (m_strings.size() > 0)    { writer.Key("string"); writer.StartArray(); for (auto&& value : m_strings) { writer.StartObject(); writer.Key(value.first.data()); writer.String(value.second.data()); writer.EndObject(); } writer.EndArray(); }

            if (m_boolVectors.size() > 0)
            {
                writer.Key("vector_bool"); writer.StartArray();
                for (auto&& vec : m_boolVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Bool(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_charVectors.size() > 0)
            {
                writer.Key("vector_char"); writer.StartArray();
                for (auto&& vec : m_charVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Int(value); }
                    writer.EndArray(); 
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_shortVectors.size() > 0)
            {
                writer.Key("vector_short"); writer.StartArray();
                for (auto&& vec : m_shortVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Int(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_intVectors.size() > 0)
            {
                writer.Key("vector_int"); writer.StartArray();
                for (auto&& vec : m_intVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Int(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_bigintVectors.size() > 0)
            {
                writer.Key("vector_int64"); writer.StartArray();
                for (auto&& vec : m_bigintVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Int64(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_ucharVectors.size() > 0)
            {
                writer.Key("vector_uchar"); writer.StartArray();
                for (auto&& vec : m_ucharVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Uint(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_ushortVectors.size() > 0)
            {
                writer.Key("vector_ushort"); writer.StartArray();
                for (auto&& vec : m_ushortVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Uint(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_uintVectors.size() > 0)
            {
                writer.Key("vector_uint"); writer.StartArray();
                for (auto&& vec : m_uintVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Uint(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_biguintVectors.size() > 0)
            {
                writer.Key("vector_uint64"); writer.StartArray();
                for (auto&& vec : m_biguintVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Uint64(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_floatVectors.size() > 0)
            {
                writer.Key("vector_float"); writer.StartArray();
                for (auto&& vec : m_floatVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Double(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_doubleVectors.size() > 0)
            {
                writer.Key("vector_double"); writer.StartArray();
                for (auto&& vec : m_doubleVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.Double(value); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            if (m_stringVectors.size() > 0)
            {
                writer.Key("vector_string"); writer.StartArray();
                for (auto&& vec : m_stringVectors)
                {
                    writer.StartObject(); writer.Key(vec.first.data());
                    writer.StartArray();
                    for (auto&& value : vec.second) { writer.String(value.data()); }
                    writer.EndArray();
                    writer.EndObject();
                }
                writer.EndArray();
            }

            writer.Key("childs");
            writer.StartArray();
            for (auto&& child : m_childs)
            {
                child.second->writeJson(_writer);
            }
            writer.EndArray();

            writer.EndObject();
        }

        void SettingsContainer::writeBinary()
        {
        }

        template<> void SettingsContainer::set(const string& key, const bool& value)            { m_bools[key] = value; }
        template<> void SettingsContainer::set(const string& key, const char& value)            { m_chars[key] = value; }
        template<> void SettingsContainer::set(const string& key, const short& value)            { m_shorts[key] = value; }
        template<> void SettingsContainer::set(const string& key, const int& value)                { m_ints[key] = value; }
        template<> void SettingsContainer::set(const string& key, const int64_t& value)            { m_bigints[key] = value; }
        template<> void SettingsContainer::set(const string& key, const unsigned char& value)    { m_uchars[key] = value; }
        template<> void SettingsContainer::set(const string& key, const unsigned short& value)    { m_ushorts[key] = value; }
        template<> void SettingsContainer::set(const string& key, const unsigned int& value)    { m_uints[key] = value; }
        template<> void SettingsContainer::set(const string& key, const uint64_t& value)        { m_biguints[key] = value; }
        template<> void SettingsContainer::set(const string& key, const float& value)            { m_floats[key] = value; }
        template<> void SettingsContainer::set(const string& key, const double& value)            { m_doubles[key] = value; }
        template<> void SettingsContainer::set(const string& key, const string& value)            { m_strings[key] = value; }

        template<> void SettingsContainer::set(const string& key, const engine::vector<bool>& value)            { m_boolVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<char>& value)            { m_charVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<short>& value)            { m_shortVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<int>& value)                { m_intVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<int64_t>& value)            { m_bigintVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<unsigned char>& value)    { m_ucharVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<unsigned short>& value)    { m_ushortVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<unsigned int>& value)    { m_uintVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<uint64_t>& value)        { m_biguintVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<float>& value)            { m_floatVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<double>& value)            { m_doubleVectors[key] = value; }
        template<> void SettingsContainer::set(const string& key, const engine::vector<string>& value)          { m_stringVectors[key] = value; }

        template<> bool                SettingsContainer::get<bool>(const string& key) const { return m_bools[key]; }
        template<> char                SettingsContainer::get<char>(const string& key) const { return m_chars[key]; }
        template<> short            SettingsContainer::get<short>(const string& key) const { return m_shorts[key]; }
        template<> int                SettingsContainer::get<int>(const string& key) const { return m_ints[key]; }
        template<> int64_t            SettingsContainer::get<int64_t>(const string& key) const { return m_bigints[key]; }
        template<> unsigned char    SettingsContainer::get<unsigned char>(const string& key) const { return m_uchars[key]; }
        template<> unsigned short    SettingsContainer::get<unsigned short>(const string& key) const { return m_ushorts[key]; }
        template<> unsigned int        SettingsContainer::get<unsigned int>(const string& key) const { return m_uints[key]; }
        template<> uint64_t            SettingsContainer::get<uint64_t>(const string& key) const { return m_biguints[key]; }
        template<> float            SettingsContainer::get<float>(const string& key) const { return m_floats[key]; }
        template<> double            SettingsContainer::get<double>(const string& key) const { return m_doubles[key]; }
        template<> string            SettingsContainer::get<string>(const string& key) const { return m_strings[key]; }

        template<> engine::vector<bool> SettingsContainer::get<engine::vector<bool>>(const string& key) const { return m_boolVectors[key]; }
        template<> engine::vector<char> SettingsContainer::get<engine::vector<char>>(const string& key) const { return m_charVectors[key]; }
        template<> engine::vector<short> SettingsContainer::get<engine::vector<short>>(const string& key) const { return m_shortVectors[key]; }
        template<> engine::vector<int> SettingsContainer::get<engine::vector<int>>(const string& key) const { return m_intVectors[key]; }
        template<> engine::vector<int64_t> SettingsContainer::get<engine::vector<int64_t>>(const string& key) const { return m_bigintVectors[key]; }
        template<> engine::vector<unsigned char> SettingsContainer::get<engine::vector<unsigned char>>(const string& key) const { return m_ucharVectors[key]; }
        template<> engine::vector<unsigned short> SettingsContainer::get<engine::vector<unsigned short>>(const string& key) const { return m_ushortVectors[key]; }
        template<> engine::vector<unsigned int> SettingsContainer::get<engine::vector<unsigned int>>(const string& key) const { return m_uintVectors[key]; }
        template<> engine::vector<uint64_t> SettingsContainer::get<engine::vector<uint64_t>>(const string& key) const { return m_biguintVectors[key]; }
        template<> engine::vector<float> SettingsContainer::get<engine::vector<float>>(const string& key) const { return m_floatVectors[key]; }
        template<> engine::vector<double> SettingsContainer::get<engine::vector<double>>(const string& key) const { return m_doubleVectors[key]; }
        template<> engine::vector<string> SettingsContainer::get<engine::vector<string>>(const string& key) const { return m_stringVectors[key]; }

        template<typename T>
        bool hasKeyGeneric(engine::unordered_map<string, T> map, const string& key)
        {
            for (auto&& keyVal : map)
            {
                if (keyVal.first == key)
                    return true;
            }
            return false;
        }

        template<> bool SettingsContainer::hasKey<bool>(const string& key) const { return hasKeyGeneric(m_bools, key); }
        template<> bool SettingsContainer::hasKey<char>(const string& key) const { return hasKeyGeneric(m_chars, key); }
        template<> bool SettingsContainer::hasKey<short>(const string& key) const { return hasKeyGeneric(m_shorts, key); }
        template<> bool SettingsContainer::hasKey<int>(const string& key) const { return hasKeyGeneric(m_ints, key); }
        template<> bool SettingsContainer::hasKey<int64_t>(const string& key) const { return hasKeyGeneric(m_bigints, key); }
        template<> bool SettingsContainer::hasKey<unsigned char>(const string& key) const { return hasKeyGeneric(m_uchars, key); }
        template<> bool SettingsContainer::hasKey<unsigned short>(const string& key) const { return hasKeyGeneric(m_ushorts, key); }
        template<> bool SettingsContainer::hasKey<unsigned int>(const string& key) const { return hasKeyGeneric(m_uints, key); }
        template<> bool SettingsContainer::hasKey<uint64_t>(const string& key) const { return hasKeyGeneric(m_biguints, key); }
        template<> bool SettingsContainer::hasKey<float>(const string& key) const { return hasKeyGeneric(m_floats, key); }
        template<> bool SettingsContainer::hasKey<double>(const string& key) const { return hasKeyGeneric(m_doubles, key); }
        template<> bool SettingsContainer::hasKey<string>(const string& key) const { return hasKeyGeneric(m_strings, key); }

        template<> bool SettingsContainer::hasKey<engine::vector<bool>>(const string& key) const { return hasKeyGeneric(m_boolVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<char>>(const string& key) const { return hasKeyGeneric(m_charVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<short>>(const string& key) const { return hasKeyGeneric(m_shortVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<int>>(const string& key) const { return hasKeyGeneric(m_intVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<int64_t>>(const string& key) const { return hasKeyGeneric(m_bigintVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<unsigned char>>(const string& key) const { return hasKeyGeneric(m_ucharVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<unsigned short>>(const string& key) const { return hasKeyGeneric(m_ushortVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<unsigned int>>(const string& key) const { return hasKeyGeneric(m_uintVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<uint64_t>>(const string& key) const { return hasKeyGeneric(m_biguintVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<float>>(const string& key) const { return hasKeyGeneric(m_floatVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<double>>(const string& key) const { return hasKeyGeneric(m_doubleVectors, key); }
        template<> bool SettingsContainer::hasKey<engine::vector<string>>(const string& key) const { return hasKeyGeneric(m_stringVectors, key); }
    }

    Settings::Settings()
        : m_rootNode{ engine::make_shared<SettingsContainer>("root") }
        , m_currentNode{ m_rootNode }
        , m_settingsPath{ }
        , m_backend{ SettingsBackend::Json }
        , m_changed{ false }
    {}

    Settings::Settings(const string& settingsPath, SettingsBackend backend)
        : m_rootNode{ engine::make_shared<SettingsContainer>("root") }
        , m_currentNode{ m_rootNode }
        , m_settingsPath{ settingsPath }
        , m_backend{ backend }
        , m_changed{ false }
    {
        switch (m_backend)
        {
        case SettingsBackend::Json: { readJson(); break; }
        case SettingsBackend::Binary: { readBinary(); break; }
        default: ASSERT(false, "Settings destructor did not handle backend type");
        }
    }

    Settings::~Settings()
    {
        if(m_changed)
            save();
    }

    void Settings::save()
    {
        switch (m_backend)
        {
        case SettingsBackend::Json: { writeJson(); break; }
        case SettingsBackend::Binary: { writeBinary(); break; }
        default: ASSERT(false, "Settings destructor did not handle backend type");
        }
    }

    void Settings::writeJson()
    {
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        m_currentNode->writeJson(&writer);

		std::ofstream out;
        out.open(m_settingsPath.c_str());
        out.write(buffer.GetString(), buffer.GetSize());
        out.close();
    }

    void Settings::writeBinary()
    {}

    void Settings::readJson()
    {
		engine::vector<char> buffer;
		std::ifstream in;
        in.open(m_settingsPath.c_str());
        if (in.is_open())
        {
            in.seekg(0, std::ios::end);
            buffer.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(buffer.data(), buffer.size());
            in.close();

            Document doc;
            doc.Parse(buffer.data(), buffer.size());

            //auto obj = doc.GetObject();
            m_currentNode->readJson(&doc);
        }
    }

    void Settings::readBinary()
    {}

	engine::vector<string> Settings::keys() const
    {
        return m_currentNode->keys();
    }

	engine::vector<string> Settings::groups() const
    {
        return m_currentNode->groups();
    }

    void Settings::beginGroup(const string& group)
    {
        auto& childs = m_currentNode->m_childs;
        for(auto&& child : childs)
        {
            if (child.first == group)
            {
                m_currentNode = m_currentNode->m_childs[group];
                return;
            }
        }

        m_currentNode->m_childs[group] = engine::make_shared<SettingsContainer>(group);
        m_currentNode->m_childs[group]->m_parent = m_currentNode;
        m_currentNode = m_currentNode->m_childs[group];
    }

    void Settings::endGroup()
    {
        m_currentNode = m_currentNode->m_parent;
        ASSERT(m_currentNode, "Settings tried to end root group");
    }

}
