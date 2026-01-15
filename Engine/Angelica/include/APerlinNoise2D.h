#ifndef _APERLINNOISE2D_H_
#define _APERLINNOISE2D_H_

#include "APerlinNoiseBase.h"

class APerlinNoise2D : public APerlinNoiseBase
{
public:
	APerlinNoise2D() = default;
	~APerlinNoise2D() = default;

	// the peroid will be nBufferWidth * nWaveLength and nBufferHeight * nWaveLength
	bool Init(int nBufferWidth, int nBufferHeight, float vAmplitude, int nWaveLength,
		float vPersistence, int nOctaveNum, unsigned int dwRandSeed);

	bool Release();

	// Get a value, containing 1D, 2D or 3D continuous and smooth random data. 
	// input: x, y = the position of the point
	void GetValue(float x, float y, float* pvValue, int nNumValue);

protected:
	void GetRandValues(int x, int y, float* pvValues, int nNumValue);

private:
	int m_nBufferWidth;
	int m_nBufferHeight;
	std::vector<NOISEVALUE> m_values; // the spool buffer containing rand values, maximum contains 3 number value;
};

#endif
