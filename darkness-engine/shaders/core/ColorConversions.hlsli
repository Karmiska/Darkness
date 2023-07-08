// these are all from: http://www.chilliant.com/rgb2hsv.html

// these values are also in Common.hlsli,
// but can't include because of missing support
static const float PI_CON = 3.1415926535897932384626433832795028841971;
static const float FLOAT_EPS = 1e-10;


/*
float colormap_red(float x) {
	return 4.04377880184332E+00 * x - 5.17956989247312E+02;
}

float colormap_green(float x) {
	if (x < (5.14022177419355E+02 + 1.13519230769231E+01) / (4.20313644688645E+00 + 4.04233870967742E+00)) {
		return 4.20313644688645E+00 * x - 1.13519230769231E+01;
	}
	else {
		return -4.04233870967742E+00 * x + 5.14022177419355E+02;
	}
}

float colormap_blue(float x) {
	if (x < 1.34071303331385E+01 / (4.25125657510228E+00 - 1.0)) { // 4.12367649967
		return x;
	}
	else if (x < (255.0 + 1.34071303331385E+01) / 4.25125657510228E+00) { // 63.1359518278
		return 4.25125657510228E+00 * x - 1.34071303331385E+01;
	}
	else if (x < (1.04455240613432E+03 - 255.0) / 4.11010047593866E+00) { // 192.100512082
		return 255.0;
	}
	else {
		return -4.11010047593866E+00 * x + 1.04455240613432E+03;
	}
}

float4 colormap(float x) {
	float t = x * 255.0;
	float r = clamp(colormap_red(t) / 255.0, 0.0, 1.0);
	float g = clamp(colormap_green(t) / 255.0, 0.0, 1.0);
	float b = clamp(colormap_blue(t) / 255.0, 0.0, 1.0);
	return float4(r, g, b, 1.0);
}
*/

float4 colormap_hsv2rgb(float h, float s, float v) {
	float r = v;
	float g = v;
	float b = v;
	if (s > 0.0) {
		h *= 6.0;
		int i = int(h);
		float f = h - float(i);
		if (i == 1) {
			r *= 1.0 - s * f;
			b *= 1.0 - s;
		}
		else if (i == 2) {
			r *= 1.0 - s;
			b *= 1.0 - s * (1.0 - f);
		}
		else if (i == 3) {
			r *= 1.0 - s;
			g *= 1.0 - s * f;
		}
		else if (i == 4) {
			r *= 1.0 - s * (1.0 - f);
			g *= 1.0 - s;
		}
		else if (i == 5) {
			g *= 1.0 - s;
			b *= 1.0 - s * f;
		}
		else {
			g *= 1.0 - s * (1.0 - f);
			b *= 1.0 - s;
		}
	}
	return float4(r, g, b, 1.0);
}

float4 colormap(float x) {
	if (x < 0.0) {
		return float4(0.0, 0.0, 0.0, 1.0);
	}
	else if (1.0 < x) {
		return float4(1.0, 1.0, 1.0, 1.0);
	}
	else {
		float h = clamp(-9.42274071356572E-01 * x + 8.74326827903982E-01, 0.0, 1.0);
		float s = 1.0;
		float v = clamp(4.90125513855204E+00 * x + 9.18879034690780E-03, 0.0, 1.0);
		return colormap_hsv2rgb(h, s, v);
	}
}



float3 HUEtoRGB(in float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R, G, B));
}

float3 RGBtoHCV(in float3 RGB)
{
	// Based on work by Sam Hocevar and Emil Persson
	float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0 / 3.0) : float4(RGB.gb, 0.0, -1.0 / 3.0);
	float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
	float C = Q.x - min(Q.w, Q.y);
	float H = abs((Q.w - Q.y) / (6 * C + FLOAT_EPS) + Q.z);
	return float3(H, C, Q.x);
}

float3 HSVtoRGB(in float3 HSV)
{
	// no idea why this doesn't work
	/*float3 RGB = HUEtoRGB(HSV.x);
	return ((RGB - 1) * HSV.y + 1) * HSV.z;*/

	float3 RGB = HSV.z;

	float h = HSV.x;
	float s = HSV.y;
	float v = HSV.z;

	float i = floor(h);
	float f = h - i;

	float p = (1.0 - s);
	float q = (1.0 - s * f);
	float t = (1.0 - s * (1 - f));

	if (i == 0) { RGB = float3(1, t, p); }
	else if (i == 1) { RGB = float3(q, 1, p); }
	else if (i == 2) { RGB = float3(p, 1, t); }
	else if (i == 3) { RGB = float3(p, q, 1); }
	else if (i == 4) { RGB = float3(t, p, 1); }
	else 
	// i == -1 
	{ RGB = float3(1, p, q); }

	RGB *= v;

	return RGB;

	/*float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(frac(HSV.xxx + K.xyz) * 6.0 - K.www);
	return HSV.z * lerp(K.xxx, saturate(p - K.xxx), HSV.y);*/
}

float3 HSLtoRGB(in float3 HSL)
{
	float3 RGB = HUEtoRGB(HSL.x);
	float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
	return (RGB - 0.5) * C + HSL.z;
}

// The weights of RGB contributions to luminance.
  // Should sum to unity.
float3 HCYtoRGB(in float3 HCY)
{
	float3 HCYwts = float3(0.299, 0.587, 0.114);
	float3 RGB = HUEtoRGB(HCY.x);
	float Z = dot(RGB, HCYwts);
	if (HCY.z < Z)
	{
		HCY.y *= HCY.z / Z;
	}
	else if (Z < 1)
	{
		HCY.y *= (1 - HCY.z) / (1 - Z);
	}
	return (RGB - Z) * HCY.y + HCY.z;
}

float3 HCLtoRGB(in float3 HCL)
{
	float HCLgamma = 3;
	float HCLy0 = 100;
	float HCLmaxL = 0.530454533953517; // == exp(HCLgamma / HCLy0) - 0.5

	float3 RGB = 0;
	if (HCL.z != 0)
	{
		float H = HCL.x;
		float C = HCL.y;
		float L = HCL.z * HCLmaxL;
		float Q = exp((1 - C / (2 * L)) * (HCLgamma / HCLy0));
		float U = (2 * L - C) / (2 * Q - 1);
		float V = C / Q;
		float A = (H + min(frac(2 * H) / 4, frac(-2 * H) / 8)) * PI * 2;
		float T;
		H *= 6;
		if (H <= 0.999)
		{
			T = tan(A);
			RGB.r = 1;
			RGB.g = T / (1 + T);
		}
		else if (H <= 1.001)
		{
			RGB.r = 1;
			RGB.g = 1;
		}
		else if (H <= 2)
		{
			T = tan(A);
			RGB.r = (1 + T) / T;
			RGB.g = 1;
		}
		else if (H <= 3)
		{
			T = tan(A);
			RGB.g = 1;
			RGB.b = 1 + T;
		}
		else if (H <= 3.999)
		{
			T = tan(A);
			RGB.g = 1 / (1 + T);
			RGB.b = 1;
		}
		else if (H <= 4.001)
		{
			RGB.g = 0;
			RGB.b = 1;
		}
		else if (H <= 5)
		{
			T = tan(A);
			RGB.r = -1 / T;
			RGB.b = 1;
		}
		else
		{
			T = tan(A);
			RGB.r = 1;
			RGB.b = -T;
		}
		RGB = RGB * V + U;
	}
	return RGB;
}

float3 RGBtoHSV(in float3 RGB)
{
	/*float3 HCV = RGBtoHCV(RGB);
	float S = HCV.y / (HCV.z + FLOAT_EPS);
	return float3(HCV.x, S, HCV.z);*/

	float r = RGB.x;
	float g = RGB.y;
	float b = RGB.z;

	float minChannel = min(r, min(g, b));
	float maxChannel = max(r, max(g, b));

	float h = 0;
	float s = 0;
	float v = maxChannel;

	float delta = maxChannel - minChannel;

	if (delta != 0)
	{
		s = delta / v;

		if (r == v) h = (g - b) / delta;
		else if (g == v) h = 2 + (b - r) / delta;
		else if (b == v) h = 4 + (r - g) / delta;
	}

	return float3(h, s, v);
}

float3 RGBtoHSL(in float3 RGB)
{
	float3 HCV = RGBtoHCV(RGB);
	float L = HCV.z - HCV.y * 0.5;
	float S = HCV.y / (1 - abs(L * 2 - 1) + FLOAT_EPS);
	return float3(HCV.x, S, L);
}

float3 RGBtoHCY(in float3 RGB)
{
	// Corrected by David Schaeffer
	float3 HCYwts = float3(0.299, 0.587, 0.114);
	float3 HCV = RGBtoHCV(RGB);
	float Y = dot(RGB, HCYwts);
	float Z = dot(HUEtoRGB(HCV.x), HCYwts);
	if (Y < Z)
	{
		HCV.y *= Z / (FLOAT_EPS + Y);
	}
	else
	{
		HCV.y *= (1 - Z) / (FLOAT_EPS + 1 - Y);
	}
	return float3(HCV.x, HCV.y, Y);
}

float3 RGBtoHCL(in float3 RGB)
{
	float HCLgamma = 3;
	float HCLy0 = 100;
	float HCLmaxL = 0.530454533953517; // == exp(HCLgamma / HCLy0) - 0.5

	float3 HCL;
	float H = 0;
	float U = min(RGB.r, min(RGB.g, RGB.b));
	float V = max(RGB.r, max(RGB.g, RGB.b));
	float Q = HCLgamma / HCLy0;
	HCL.y = V - U;
	if (HCL.y != 0)
	{
		H = atan2(RGB.g - RGB.b, RGB.r - RGB.g) / PI;
		Q *= U / V;
	}
	Q = exp(Q);
	HCL.x = frac(H / 2 - min(frac(H), frac(-H)) / 6);
	HCL.y *= Q;
	HCL.z = lerp(-U, V, Q) / (HCLmaxL * 2);
	return HCL;
}
