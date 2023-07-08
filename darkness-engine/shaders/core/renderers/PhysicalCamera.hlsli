
// these have been imported from: https://placeholderart.wordpress.com/2014/11/21/implementing-a-physically-based-camera-manual-exposure/
/*
* Get an exposure using the Saturation-based Speed method.
*/
float getSaturationBasedExposure(float aperture,
    float shutterSpeed,
    float iso)
{
    float l_max = (7800.0f / 65.0f) * sqrt(aperture) / (iso * shutterSpeed);
    return 1.0f / l_max;
}

static const float MIN_ISO = 100.0f;
static const float MAX_ISO = 6400.0f;

static const float MIN_SHUTTER = 1.0f / 4000.0f;
static const float MAX_SHUTTER = 1.0f / 30.0f;

static const float MIN_APERTURE = 1.8f;
static const float MAX_APERTURE = 22.0f;

/*
* Get an exposure using the Standard Output Sensitivity method.
* Accepts an additional parameter of the target middle grey.
*/
float getStandardOutputBasedExposure(float aperture,
    float shutterSpeed,
    float iso,
    float middleGrey = 0.18f)
{
    float l_avg = (1000.0f / 65.0f) * sqrt(aperture) / (iso * shutterSpeed);
    return middleGrey / l_avg;
}

// References:
// http://en.wikipedia.org/wiki/Film_speed
// http://en.wikipedia.org/wiki/Exposure_value
// http://en.wikipedia.org/wiki/Light_meter

// Notes:
// EV below refers to EV at ISO 100

// Given an aperture, shutter speed, and exposure value compute the required ISO value
float ComputeISO(float aperture, float shutterSpeed, float ev)
{
    return (sqrt(aperture) * 100.0f) / (shutterSpeed * pow(2.0f, ev));
}

// Given the camera settings compute the current exposure value
float ComputeEV(float aperture, float shutterSpeed, float iso)
{
    return log2((sqrt(aperture) * 100.0f) / (shutterSpeed * iso));
}

// Using the light metering equation compute the target exposure value
float ComputeTargetEV(float averageLuminance)
{
    // K is a light meter calibration constant
    static const float K = 12.5f;
    return log2(averageLuminance * 100.0f / K);
}

void ApplyAperturePriority(float focalLength,
    float targetEV,
    inout float aperture,
    inout float shutterSpeed,
    inout float iso)
{
    // Start with the assumption that we want a shutter speed of 1/f
    shutterSpeed = 1.0f / (focalLength * 1000.0f);

    // Compute the resulting ISO if we left the shutter speed here
    iso = clamp(ComputeISO(aperture, shutterSpeed, targetEV), MIN_ISO, MAX_ISO);

    // Figure out how far we were from the target exposure value
    float evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);

    // Compute the final shutter speed
    shutterSpeed = clamp(shutterSpeed * pow(2.0f, -evDiff), MIN_SHUTTER, MAX_SHUTTER);
}

void ApplyShutterPriority(float focalLength,
    float targetEV,
    inout float aperture,
    inout float shutterSpeed,
    inout float iso)
{
    // Start with the assumption that we want an aperture of 4.0
    aperture = 4.0f;

    // Compute the resulting ISO if we left the aperture here
    iso = clamp(ComputeISO(aperture, shutterSpeed, targetEV), MIN_ISO, MAX_ISO);

    // Figure out how far we were from the target exposure value
    float evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);

    // Compute the final aperture
    aperture = clamp(aperture * pow(sqrt(2.0f), evDiff), MIN_APERTURE, MIN_APERTURE);
}

void ApplyProgramAuto(float focalLength,
    float targetEV,
    inout float aperture,
    inout float shutterSpeed,
    inout float iso)
{
    // Start with the assumption that we want an aperture of 4.0
    aperture = 4.0f;

    // Start with the assumption that we want a shutter speed of 1/f
    shutterSpeed = 1.0f / (focalLength * 1000.0f);

    // Compute the resulting ISO if we left both shutter and aperture here
    iso = clamp(ComputeISO(aperture, shutterSpeed, targetEV), MIN_ISO, MAX_ISO);

    // Apply half the difference in EV to the aperture
    float evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
    aperture = clamp(aperture * pow(sqrt(2.0f), evDiff * 0.5f), MIN_APERTURE, MIN_APERTURE);

    // Apply the remaining difference to the shutter speed
    evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
    shutterSpeed = clamp(shutterSpeed * pow(2.0f, -evDiff), MIN_SHUTTER, MAX_SHUTTER);
}

float4 applyPhysicalCamera(float4 color)
{
    return color;
    float f = 5.6f;


    float iso = 100.0f;
    float aperture = f / 4.0f;// 16.0f;
    float shutterSpeed = 1.0f / 100.0f;

    return color * getStandardOutputBasedExposure(aperture, shutterSpeed, iso);
}
