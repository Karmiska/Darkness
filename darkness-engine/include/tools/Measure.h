#pragma once

#include "tools/Debug.h"
#include "containers/vector.h"
#include <chrono>
#include "containers/string.h"
#include <map>

namespace tools
{
    namespace implementation
    {
        struct Measurement
        {
            const char* msg = nullptr;
            std::chrono::time_point<std::chrono::steady_clock> time;
            engine::string stdMsg;
        };

        class MeasureStorage
        {
        public:
            bool addMeasure(const engine::string& key, uint32_t period, engine::vector<Measurement> measurements);
            void clearMeasure(const engine::string& key);
            engine::vector<double> average(const engine::string& key);
        private:
            std::map<engine::string, engine::vector<engine::vector<Measurement>>> m_storage;
        };

        extern MeasureStorage GlobalMeasurementStorage;
    }

    class Measure
    {
    public:
        Measure();

        void disable();

        void period(uint32_t period);

        void here(const char* msg);
        void here(const engine::string& msg);
        ~Measure();
    private:
        bool m_active;
        uint32_t m_period;

        engine::vector<implementation::Measurement> m_measurements;
    };
}
