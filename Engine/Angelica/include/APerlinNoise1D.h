#ifndef _APERLINNOISE1D_H_
#define _APERLINNOISE1D_H_

#include "APerlinNoiseBase.h"

class APerlinNoise1D : public APerlinNoiseBase
{
public:
	APerlinNoise1D() = default;
	~APerlinNoise1D() = default;

	// The period will be nBufferLen * nWaveLength
	bool Init(int nBufferWidth, float vAmplitude, int nWaveLength, float vPersistence, int nOctaveNum, unsigned int dwRandSeed);

	bool Release();

	// Get a value, containing 1D, 2D or 3D continuous and smooth random data. 
	// input: x = the position of the value in the perlin noise series, can be a value in real situation
	void GetValue(float x, float* pvValue, int nNumValue);

protected:
	void GetRandValues(int n, float* pvValues, int nNumValue);

private:
	int m_nBufferWidth;
	std::vector<NOISEVALUE> m_values; // the spool buffer containing rand values, maximum contains 3 number value
};

#endif
