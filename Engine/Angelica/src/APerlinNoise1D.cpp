#include "pch.h"
#include "APerlinNoise1D.h"

bool APerlinNoise1D::Init(int nBufferWidth, float vAmplitude, int nWaveLength, float vPersistence, int nOctaveNum, unsigned int dwRandSeed)
{
	// First try to release old resource
	Release();

	if (nBufferWidth <= 0)
		return false;

	m_dwSeed = dwRandSeed;
	m_nBufferWidth = nBufferWidth;

	// Resize vector to hold buffer
	m_values.assign(m_nBufferWidth, NOISEVALUE{});

	// Temporary buffer for smoothing (one channel at a time)
	std::vector<float> tempBuffer(m_nBufferWidth);

	for (int k = 0; k < 3; ++k)
	{
		// Generate random values
		for (int i = 0; i < m_nBufferWidth; ++i)
			tempBuffer[i] = RandFloat();

		// Smooth: 0.25*prev + 0.5*current + 0.25*next (with wrap-around)
		for (int i = 0; i < m_nBufferWidth; ++i)
		{
			int prev = (i - 1 + m_nBufferWidth) % m_nBufferWidth;
			int next = (i + 1) % m_nBufferWidth;

			m_values[i].v[k] = 0.25f * tempBuffer[prev] +
				0.5f * tempBuffer[i] +
				0.25f * tempBuffer[next];
		}
	}

	return InitParams(vAmplitude, nWaveLength, vPersistence, nOctaveNum);
}

bool APerlinNoise1D::Release()
{
	m_values.clear();
	m_values.shrink_to_fit();

	m_nBufferWidth = 0;

	return true;
}

void APerlinNoise1D::GetValue(float x, float* pvValue, int nNumValue)
{
	if (!pvValue || nNumValue <= 0 || m_values.empty())
		return;

	const int num = std::min(nNumValue, 3);
	float vFinal[3] = {};

	for (int octave = 0; octave < m_nOctaveNum; ++octave)
	{
		float v = static_cast<float>(m_nStartPos[octave]) + x / static_cast<float>(m_nWaveLength[octave]);
		int n1 = static_cast<int>(std::floor(v));
		float s = v - n1;
		int n2 = n1 + 1;

		float value1[3] = {};
		float value2[3] = {};

		GetRandValues(n1, value1, num);
		GetRandValues(n2, value2, num);

		for (int k = 0; k < num; ++k)
		{
			float lerped = LERP(s, value1[k], value2[k]);
			if (m_bTurbulence)
				vFinal[k] += m_vAmplitude[octave] * std::fabs(lerped);
			else
				vFinal[k] += m_vAmplitude[octave] * lerped;
		}
	}

	for (int i = 0; i < num; ++i)
		pvValue[i] = vFinal[i];
}

void APerlinNoise1D::GetRandValues(int n, float* pvValues, int nNumValue)
{
	if (m_values.empty() || nNumValue <= 0)
		return;

	// Clamp nNumValue to [1, 3]
	const int num = std::min(nNumValue, 3);

	// Wrap n into valid range [0, m_nBufferLen)
	n = ((n % m_nBufferWidth) + m_nBufferWidth) % m_nBufferWidth;

	for (int i = 0; i < num; ++i)
		pvValues[i] = m_values[n].v[i];
}
