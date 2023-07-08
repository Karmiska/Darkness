#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "containers/memory.h"

namespace tools
{
    enum class SettingsBackend
    {
        Binary,
        Json
    };

    class Settings;
    namespace implementation
    {
        class SettingsContainer : public std::enable_shared_from_this<SettingsContainer>
        {
        private:
            mutable engine::unordered_map<engine::string, bool> m_bools;
            mutable engine::unordered_map<engine::string, char> m_chars;
            mutable engine::unordered_map<engine::string, short> m_shorts;
            mutable engine::unordered_map<engine::string, int> m_ints;
            mutable engine::unordered_map<engine::string, int64_t> m_bigints;
            mutable engine::unordered_map<engine::string, unsigned char> m_uchars;
            mutable engine::unordered_map<engine::string, unsigned short> m_ushorts;
            mutable engine::unordered_map<engine::string, unsigned int> m_uints;
            mutable engine::unordered_map<engine::string, uint64_t> m_biguints;
            mutable engine::unordered_map<engine::string, float> m_floats;
            mutable engine::unordered_map<engine::string, double> m_doubles;
            mutable engine::unordered_map<engine::string, engine::string> m_strings;
            
            mutable engine::unordered_map<engine::string, engine::vector<bool>> m_boolVectors;
            mutable engine::unordered_map<engine::string, engine::vector<char>> m_charVectors;
            mutable engine::unordered_map<engine::string, engine::vector<short>> m_shortVectors;
            mutable engine::unordered_map<engine::string, engine::vector<int>> m_intVectors;
            mutable engine::unordered_map<engine::string, engine::vector<int64_t>> m_bigintVectors;
            mutable engine::unordered_map<engine::string, engine::vector<unsigned char>> m_ucharVectors;
            mutable engine::unordered_map<engine::string, engine::vector<unsigned short>> m_ushortVectors;
            mutable engine::unordered_map<engine::string, engine::vector<unsigned int>> m_uintVectors;
            mutable engine::unordered_map<engine::string, engine::vector<uint64_t>> m_biguintVectors;
            mutable engine::unordered_map<engine::string, engine::vector<float>> m_floatVectors;
            mutable engine::unordered_map<engine::string, engine::vector<double>> m_doubleVectors;
            mutable engine::unordered_map<engine::string, engine::vector<engine::string>> m_stringVectors;

        public:
            SettingsContainer(const engine::string& groupName);
            engine::vector<engine::string> keys() const;
            engine::vector<engine::string> groups() const;

            template<typename T>
            void set(const engine::string& key, const engine::vector<T>& value);

            template<typename T>
            void set(const engine::string& key, const T& value);

            template<typename T>
            T get(const engine::string& key) const;

            template<typename T>
            bool hasKey(const engine::string& key) const;

        protected:
            friend class Settings;
            engine::shared_ptr<implementation::SettingsContainer> m_parent;
            engine::string m_groupName;
            engine::unordered_map<engine::string, engine::shared_ptr<implementation::SettingsContainer>> m_childs;

            void readJson(void* _obj);
            void writeJson(void* _writer);

            void writeBinary();
        };
    }

    class Settings
    {
    public:
        Settings();
        Settings(const engine::string& settingsPath, SettingsBackend backend = SettingsBackend::Json);
        ~Settings();

        void save();

        Settings(const Settings&) = default;
        Settings(Settings&&) = default;
        Settings& operator=(const Settings&) = default;
        Settings& operator=(Settings&&) = default;

        engine::vector<engine::string> keys() const;
        engine::vector<engine::string> groups() const;

        void beginGroup(const engine::string& group);
        void endGroup();

        template<typename T>
        void set(const engine::string& key, const T& value)
        {
            m_currentNode->set(key, value);
            m_changed = true;
        }

        template<typename T>
        void set(const engine::string& key, const engine::vector<T>& value)
        {
            m_currentNode->set(key, value);
            m_changed = true;
        }

        template<typename T>
        T get(const engine::string& key) const
        {
            return m_currentNode->get<T>(key);
        }

        template<typename T>
        T get(const engine::string& key, const T& defaultValue) const
        {
            if(m_currentNode->hasKey<T>(key))
                return m_currentNode->get<T>(key);
            else
            {
                m_currentNode->set(key, defaultValue);
                m_changed = true;
                return defaultValue;
            }
        }
    private:
        engine::shared_ptr<implementation::SettingsContainer> m_rootNode;
        engine::shared_ptr<implementation::SettingsContainer> m_currentNode;
        engine::string m_settingsPath;
        SettingsBackend m_backend;
        mutable bool m_changed;

        void readJson();
        void readBinary();

        void writeJson();
        void writeBinary();
    };
}
