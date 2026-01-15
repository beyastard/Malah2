#include "pch.h"
#include "APerlinNoise2D.h"

bool APerlinNoise2D::Init(int nBufferWidth, int nBufferHeight, float vAmplitude,
	int nWaveLength, float vPersistence, int nOctaveNum, unsigned int dwRandSeed)
{
	// First try to release old resource
	Release();

	if (nBufferWidth <= 0 || nBufferHeight <= 0)
		return false;

	m_dwSeed = dwRandSeed;
	m_nBufferWidth = nBufferWidth;
	m_nBufferHeight = nBufferHeight;

	const size_t bufferSize =
		static_cast<size_t>(m_nBufferWidth) *
		static_cast<size_t>(m_nBufferHeight);
	m_values.assign(bufferSize, NOISEVALUE{});

	std::vector<float> tempBuffer(bufferSize);

	for (int k = 0; k < 3; ++k)
	{
		// Generate random values
		for (size_t i = 0; i < bufferSize; ++i)
			tempBuffer[i] = RandFloat();

		// Smooth using 3x3 kernel (with wrap-around)
		for (int y = 0; y < m_nBufferHeight; ++y)
		{
			for (int x = 0; x < m_nBufferWidth; ++x)
			{
				auto idx = [&](int cx, int cy) -> float {
					// Wrap coordinates
					int wx = ((cx % m_nBufferWidth) + m_nBufferWidth) % m_nBufferWidth;
					int wy = ((cy % m_nBufferHeight) + m_nBufferHeight) % m_nBufferHeight;
					return tempBuffer[static_cast<size_t>(wy) * m_nBufferWidth + wx];
					};

				float center = idx(x, y);
				float left = idx(x - 1, y);
				float right = idx(x + 1, y);
				float top = idx(x, y - 1);
				float bottom = idx(x, y + 1);
				float tl = idx(x - 1, y - 1);
				float tr = idx(x + 1, y - 1);
				float bl = idx(x - 1, y + 1);
				float br = idx(x + 1, y + 1);

				// Same weights as original: corners=1/16, edges=1/8, center=1/4
				m_values[static_cast<size_t>(y) * m_nBufferWidth + x].v[k] =
					(tl + tr + bl + br) * (1.0f / 16.0f) +
					(top + bottom + left + right) * (1.0f / 8.0f) +
					center * (1.0f / 4.0f);
			}
		}
	}

	return InitParams(vAmplitude, nWaveLength, vPersistence, nOctaveNum);
}

bool APerlinNoise2D::Release()
{
	m_values.clear();
	m_values.shrink_to_fit();

	m_nBufferWidth = 0;
	m_nBufferHeight = 0;

	return true;
}

void APerlinNoise2D::GetValue(float x, float y, float* pvValue, int nNumValue)
{
	if (!pvValue || nNumValue <= 0 || m_values.empty())
		return;

	const int num = std::min(nNumValue, 3);
	float vFinal[3] = {};

	for (int octave = 0; octave < m_nOctaveNum; ++octave)
	{
		if (m_nActiveOctave != -1 && m_nActiveOctave != octave)
			continue;

		// Compute offset + scaled position
		float offsetX = static_cast<float>(m_nStartPos[octave] % m_nBufferWidth);
		float offsetY = static_cast<float>(m_nStartPos[octave] / m_nBufferWidth);

		float vx = offsetX + x / static_cast<float>(m_nWaveLength[octave]);
		float vy = offsetY + y / static_cast<float>(m_nWaveLength[octave]);

		int x1 = static_cast<int>(std::floor(vx));
		int y1 = static_cast<int>(std::floor(vy));
		float sx = vx - x1;
		float sy = vy - y1;

		// Apply smoothstep (S_CURVE)
		sx = S_CURVE(sx);
		sy = S_CURVE(sy);

		int x2 = x1 + 1;
		int y2 = y1 + 1;

		float v11[3] = {}, v21[3] = {}, v12[3] = {}, v22[3] = {};

		GetRandValues(x1, y1, v11, num);
		GetRandValues(x2, y1, v21, num);
		GetRandValues(x1, y2, v12, num);
		GetRandValues(x2, y2, v22, num);

		// Interpolate in X
		float x1_interp[3]{};
		float x2_interp[3]{};
		for (int k = 0; k < num; ++k)
		{
			x1_interp[k] = LERP(sx, v11[k], v21[k]);
			x2_interp[k] = LERP(sx, v12[k], v22[k]);
		}

		// Interpolate in Y
		for (int k = 0; k < num; ++k)
		{
			float lerped = LERP(sy, x1_interp[k], x2_interp[k]);
			if (m_bTurbulence)
				vFinal[k] += m_vAmplitude[octave] * std::fabs(lerped);
			else
				vFinal[k] += m_vAmplitude[octave] * lerped;
		}
	}

	for (int i = 0; i < num; ++i)
		pvValue[i] = vFinal[i];
}

void APerlinNoise2D::GetRandValues(int x, int y, float* pvValues, int nNumValue)
{
	if (m_values.empty() || nNumValue <= 0)
		return;

	const int num = std::min(nNumValue, 3);

	// Correct modulo for negative values
	x = ((x % m_nBufferWidth) + m_nBufferWidth) % m_nBufferWidth;
	y = ((y % m_nBufferHeight) + m_nBufferHeight) % m_nBufferHeight;

	size_t index = static_cast<size_t>(y) * m_nBufferWidth + x;
	for (int i = 0; i < num; ++i)
		pvValues[i] = m_values[index].v[i];
}
