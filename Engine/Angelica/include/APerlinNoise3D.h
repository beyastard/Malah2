#ifndef _APERLINNOISE3D_H_
#define _APERLINNOISE3D_H_

#include "APerlinNoiseBase.h"

class APerlinNoise3D : public APerlinNoiseBase
{
public:
	APerlinNoise3D() = default;
	~APerlinNoise3D() = default;

	// the peroid will be nBufferWidth * nWaveLength and nBufferHeight * nWaveLength
	bool Init(int nBufferWidth, int nBufferHeight, int nBufferDepth, float vAmplitude,
		int nWaveLength, float vPersistence, int nOctaveNum, unsigned int dwRandSeed);

	bool Release();

	// Get a value, containing 1D, 2D or 3D continuous and smooth random data. 
	// input: x, y, z = the position of the point
	void GetValue(float x, float y, float z, float* pvValue, int nNumValue);

protected:
	void GetRandValues(int x, int y, int z, float* pvValues, int nNumValue);

private:
	int m_nBufferWidth;
	int m_nBufferHeight;
	int m_nBufferDepth;
	std::vector<NOISEVALUE> m_values; // the spool buffer containing rand values, maximum contains 3 number value
};

#endif
