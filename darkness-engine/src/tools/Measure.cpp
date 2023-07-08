#include "tools/Measure.h"

using namespace engine;

namespace tools
{
    namespace implementation
    {
        MeasureStorage GlobalMeasurementStorage;

        bool MeasureStorage::addMeasure(const engine::string& key, uint32_t period, vector<Measurement> measurements)
        {
            vector<vector<Measurement>>& list = m_storage[key];
            list.emplace_back(measurements);
            return list.size() >= period;
        }
        void MeasureStorage::clearMeasure(const engine::string& key)
        {
            m_storage.erase(key);
        }
        vector<double> MeasureStorage::average(const engine::string& key)
        {
            auto& list = m_storage[key];

            if (list.size() == 0)
                return {};

            vector<double> averages(list.front().size(), 0.0);

            for (int i = 0; i < list.size(); ++i)
            {
                for (int a = 0; a < list[i].size() - 1; ++a)
                {
                    auto duration = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                        list[i][a + 1].time - list[i][a].time).count() / 1000000.0;
                    averages[a] += duration;
                }
                averages.back() += (double)std::chrono::duration_cast<std::chrono::nanoseconds>(list[i].back().time - list[i][0].time).count() / 1000000.0;
            }
            for (int i = 0; i < averages.size(); ++i)
            {
                averages[i] /= list[0].size();
            }
            return averages;
        }
    }

    Measure::Measure()
        : m_active{ true }
        , m_period{ 1 }
    {}

    void Measure::disable()
    {
        m_active = false;
    }

    void Measure::period(uint32_t period)
    {
        m_period = period;
    }

    void Measure::here(const char* msg)
    {
        if (m_active)
            m_measurements.emplace_back(implementation::Measurement{ msg, std::chrono::high_resolution_clock::now() });
    }
    void Measure::here(const engine::string& msg)
    {
        if (m_active)
            m_measurements.emplace_back(implementation::Measurement{ nullptr, std::chrono::high_resolution_clock::now(), msg });
    }
    Measure::~Measure()
    {
        here("end");
        if (m_active)
        {
            if (m_period != 1)
            {
                engine::string key = m_measurements[0].msg ? m_measurements[0].msg : m_measurements[0].stdMsg;
                if (implementation::GlobalMeasurementStorage.addMeasure(
                    key,
                    m_period,
                    m_measurements))
                {
                    auto averages = implementation::GlobalMeasurementStorage.average(key);
                    for (int i = 0; i < m_measurements.size() - 1; ++i)
                    {
                        if (m_measurements[i].msg)
                            LOG_INFO("Measure: %s = %05.3f ms", m_measurements[i].msg,
                                averages[i]);
                        else
                            LOG_INFO("Measure: %s = %05.3f ms", m_measurements[i].stdMsg.c_str(),
                                averages[i]);
                    }
                    LOG_WARNING("Measure range: %05.3f ms", averages.back());
                    implementation::GlobalMeasurementStorage.clearMeasure(key);
                }
            }
            else
            {
                for (int i = 0; i < m_measurements.size() - 1; ++i)
                {
                    if (m_measurements[i].msg)
                        LOG_INFO("Measure: %s = %05.3f ms", m_measurements[i].msg,
                        (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                            m_measurements[i + 1].time - m_measurements[i].time).count() / 1000000.0);
                    else
                        LOG_INFO("Measure: %s = %05.3f ms", m_measurements[i].stdMsg.c_str(),
                        (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                            m_measurements[i + 1].time - m_measurements[i].time).count() / 1000000.0);
                }
                LOG_WARNING("Measure range: %05.3f ms", (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                    m_measurements[m_measurements.size() - 1].time - m_measurements[0].time).count() / 1000000.0);
            }
        }
    }
}
