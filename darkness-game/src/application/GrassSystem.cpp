#include "GrassSystem.h"
#include "SolTrack.h"
#include "engine/primitives/Math.h"

namespace game
{


    void GrassBlade::grow(
        float deltaSeconds, 
        float effectiveness,
        const engine::Vector3f& sunDirection)
    {
        if(sunDirection.y > 0.0f)
            m_tip += sunDirection * (GrowthRatePerSecond * deltaSeconds * effectiveness);
    }

    GrassSystem::GrassSystem(size_t widthMeters, size_t heightMeters)
    {
        auto grassBladeArcheType = m_ecs.archeType<Transform, GrassBlade>();
        auto grassBladeArcheTypeId = grassBladeArcheType.id();

        uint64_t bladeCount = widthMeters * heightMeters * BladesPerSquareMeter;
        for (uint64_t i = 0; i < bladeCount; ++i)
        {
            auto blade = m_ecs.createEntity();
            blade.setComponents(grassBladeArcheTypeId);
        }
    }

    engine::Vector3f sunDirection(float azimuth, float altitude)
    {
        // Convert azimuth and altitude to radians
        float azRad = azimuth * DEG_TO_RAD;
        float altRad = altitude * DEG_TO_RAD;

        // Calculate components of the unit vector
        float x = cos(altRad) * cos(azRad);
        float y = cos(altRad) * sin(azRad);
        float z = sin(altRad);

        // Return the unit vector
        return { x, y, z };
    }

    void GrassSystem::grow(float deltaSeconds)
    {
        auto hours = static_cast<uint64_t>(deltaSeconds / 60.0f / 60.0f);
        auto secondsInHour = 60.0f * 60.0f;

        int useDegrees = 1;             // Input (geographic position) and output are in degrees
        int useNorthEqualsZero = 1;     // Azimuth: 0 = South, pi/2 (90deg) = West  ->  0 = North, pi/2 (90deg) = East
        int computeRefrEquatorial = 1;  // Compure refraction-corrected equatorial coordinates (Hour angle, declination): 0-no, 1-yes
        int computeDistance = 1;        // Compute the distance to the Sun in AU: 0-no, 1-yes

        uint64_t hour = 0;
        uint64_t day = 13;
        for(uint64_t i = 0; i < hours; ++i)
        {
            soltrack::Time time;

            // Set (UT!) date and time manually - use the first date from SolTrack_positions.dat:
            time.year = 2023;
            time.month = 6;
            time.day = day;
            time.hour = hour;  // 8h CEST = 6h UT
            ++hour;
            if (time.hour > 24)
            {
                ++day;
                hour = 0;
            }
            time.minute = 2;
            time.second = 49.217348;

            soltrack::Location loc;
            loc.longitude = 24.945831;  // Helsinki, Finland
            loc.latitude = 30.192059;
            loc.pressure = 101.325;     // Atmospheric pressure in kPa
            loc.temperature = 293.15;  // Atmospheric temperature in K (20 c)


            // Compute rise and set times:
            soltrack::Position pos;
            soltrack::RiseSet riseSet;
            SolTrack_RiseSet(time, loc, &pos, &riseSet, 0.0, useDegrees, useNorthEqualsZero);

            // Compute positions:
            SolTrack(time, loc, &pos, useDegrees, useNorthEqualsZero, computeRefrEquatorial, computeDistance);

            //LOG("Position: [time: %i:%i:%i, julianDay: %f, tJD: %f, tJC: %f, tJC2: %f, longitude: %f, distance: %f, obliquity: %f, cosObliquity: %f, nutationLon: %f, rightAscension: %f, declination: %f, hourAngle: %f, agst: %f, altitude: %f, altitudeRefract: %f, azimuthRefract: %f, hourAngleRefract: %f, declinationRefract: %f]",
            //    time.hour, time.minute, time.second,
            //    pos.julianDay, pos.tJD, pos.tJC, pos.tJC2,
            //    pos.longitude, pos.distance,
            //    pos.obliquity, pos.cosObliquity, pos.nutationLon,
            //    pos.rightAscension, pos.declination, pos.hourAngle, pos.agst,
            //    pos.altitude, pos.altitudeRefract, pos.azimuthRefract,
            //    pos.hourAngleRefract, pos.declinationRefract);

            float efficiency = pos.altitude;
            if (efficiency < 0.0f)
                efficiency = 0.0f;
            if (efficiency > 90.0f)
                efficiency = 90.0f;
            efficiency /= 90.0f;

            float distance = pos.distance;
            float effectiveness = efficiency / distance;
            auto _sunDirection = sunDirection(pos.altitude, pos.azimuthRefract);

            m_ecs.query([&secondsInHour, &effectiveness, &_sunDirection](Transform& transform, GrassBlade& blade)
            {
                blade.grow(secondsInHour, effectiveness, _sunDirection);
            });
        }
    }
}
