#include "pch.h"
#include "APerlinNoise3D.h"

bool APerlinNoise3D::Init(int nBufferWidth, int nBufferHeight, int nBufferDepth, float vAmplitude,
	int nWaveLength, float vPersistence, int nOctaveNum, unsigned int dwRandSeed)
{
	// First try to release old resource
	Release();

	if (nBufferWidth <= 0 || nBufferHeight <= 0 || nBufferDepth <= 0)
		return false;

	m_dwSeed = dwRandSeed;
	m_nBufferWidth = nBufferWidth;
	m_nBufferHeight = nBufferHeight;
	m_nBufferDepth = nBufferDepth;

	const size_t bufferSize =
		static_cast<size_t>(m_nBufferWidth) *
		static_cast<size_t>(m_nBufferHeight) *
		static_cast<size_t>(m_nBufferDepth);
	m_values.assign(bufferSize, NOISEVALUE{});

	std::vector<float> tempBuffer(bufferSize);

	for (int k = 0; k < 3; ++k)
	{
		// Generate random values
		for (size_t i = 0; i < bufferSize; ++i)
			tempBuffer[i] = RandFloat();

		// Smooth using 3D kernel (6 face neighbors + center)
		for (int z = 0; z < m_nBufferDepth; ++z)
		{
			for (int y = 0; y < m_nBufferHeight; ++y)
			{
				for (int x = 0; x < m_nBufferWidth; ++x)
				{
					auto idx = [&](int cx, int cy, int cz) -> float {
						// Wrap coordinates in all 3 dimensions
						int wx = ((cx % m_nBufferWidth) + m_nBufferWidth) % m_nBufferWidth;
						int wy = ((cy % m_nBufferHeight) + m_nBufferHeight) % m_nBufferHeight;
						int wz = ((cz % m_nBufferDepth) + m_nBufferDepth) % m_nBufferDepth;

						return tempBuffer[
							static_cast<size_t>(wz) * m_nBufferWidth * m_nBufferHeight +
								static_cast<size_t>(wy) * m_nBufferWidth +
								wx];
						};

					float center = idx(x, y, z);
					float nx = idx(x - 1, y, z);
					float px = idx(x + 1, y, z);
					float ny = idx(x, y - 1, z);
					float py = idx(x, y + 1, z);
					float nz = idx(x, y, z - 1);
					float pz = idx(x, y, z + 1);

					// 6 neighbors * 1/8, center * 1/4
					m_values[
						static_cast<size_t>(z) * m_nBufferWidth * m_nBufferHeight +
							static_cast<size_t>(y) * m_nBufferWidth +
							x
					].v[k] =
							(nx + px + ny + py + nz + pz) * (1.0f / 8.0f) +
							center * (1.0f / 4.0f);
				}
			}
		}
	}

	return InitParams(vAmplitude, nWaveLength, vPersistence, nOctaveNum);
}

bool APerlinNoise3D::Release()
{
	m_values.clear();
	m_values.shrink_to_fit();

	m_nBufferWidth = 0;
	m_nBufferHeight = 0;
	m_nBufferDepth = 0;

	return true;
}

void APerlinNoise3D::GetValue(float x, float y, float z, float* pvValue, int nNumValue)
{
	if (!pvValue || nNumValue <= 0 || m_values.empty())
		return;

	const int num = std::min(nNumValue, 3);
	float vFinal[3] = {};

	for (int octave = 0; octave < m_nOctaveNum; ++octave)
	{
		if (m_nActiveOctave != -1 && m_nActiveOctave != octave)
			continue;

		// Compute 3D offset from m_nStartPos
		int startPos = m_nStartPos[octave];
		float offsetX = static_cast<float>(startPos % m_nBufferWidth);
		float offsetY = static_cast<float>((startPos / m_nBufferWidth) % m_nBufferHeight);
		float offsetZ = static_cast<float>(startPos / (m_nBufferWidth * m_nBufferHeight));

		float vx = offsetX + x / static_cast<float>(m_nWaveLength[octave]);
		float vy = offsetY + y / static_cast<float>(m_nWaveLength[octave]);
		float vz = offsetZ + z / static_cast<float>(m_nWaveLength[octave]);

		int x1 = static_cast<int>(std::floor(vx));
		int y1 = static_cast<int>(std::floor(vy));
		int z1 = static_cast<int>(std::floor(vz));

		float sx = S_CURVE(vx - x1);
		float sy = S_CURVE(vy - y1);
		float sz = S_CURVE(vz - z1);

		int x2 = x1 + 1;
		int y2 = y1 + 1;
		int z2 = z1 + 1;

		// Fetch 8 corner values
		float v000[3] = {}, v100[3] = {}, v010[3] = {}, v110[3] = {};
		float v001[3] = {}, v101[3] = {}, v011[3] = {}, v111[3] = {};

		GetRandValues(x1, y1, z1, v000, num);
		GetRandValues(x2, y1, z1, v100, num);
		GetRandValues(x1, y2, z1, v010, num);
		GetRandValues(x2, y2, z1, v110, num);
		GetRandValues(x1, y1, z2, v001, num);
		GetRandValues(x2, y1, z2, v101, num);
		GetRandValues(x1, y2, z2, v011, num);
		GetRandValues(x2, y2, z2, v111, num);

		// Interpolate along X
		float x00[3]{}, x10[3]{}, x01[3]{}, x11[3]{};
		for (int k = 0; k < num; ++k)
		{
			x00[k] = LERP(sx, v000[k], v100[k]);
			x10[k] = LERP(sx, v010[k], v110[k]);
			x01[k] = LERP(sx, v001[k], v101[k]);
			x11[k] = LERP(sx, v011[k], v111[k]);
		}

		// Interpolate along Y
		float y1_interp[3]{};
		float y2_interp[3]{};
		for (int k = 0; k < num; ++k)
		{
			y1_interp[k] = LERP(sy, x00[k], x10[k]);
			y2_interp[k] = LERP(sy, x01[k], x11[k]);
		}

		// Interpolate along Z
		for (int k = 0; k < num; ++k)
		{
			float lerped = LERP(sz, y1_interp[k], y2_interp[k]);
			if (m_bTurbulence)
				vFinal[k] += m_vAmplitude[octave] * std::fabs(lerped);
			else
				vFinal[k] += m_vAmplitude[octave] * lerped;
		}
	}

	for (int i = 0; i < num; ++i)
		pvValue[i] = vFinal[i];
}

void APerlinNoise3D::GetRandValues(int x, int y, int z, float* pvValues, int nNumValue)
{
	if (m_values.empty() || nNumValue <= 0)
		return;

	const int num = std::min(nNumValue, 3);

	// Correct modulo for negative coordinates
	x = ((x % m_nBufferWidth) + m_nBufferWidth) % m_nBufferWidth;
	y = ((y % m_nBufferHeight) + m_nBufferHeight) % m_nBufferHeight;
	z = ((z % m_nBufferDepth) + m_nBufferDepth) % m_nBufferDepth;

	size_t index =
		static_cast<size_t>(z) * m_nBufferWidth * m_nBufferHeight +
		static_cast<size_t>(y) * m_nBufferWidth + x;

	for (int i = 0; i < num; ++i)
		pvValues[i] = m_values[index].v[i];
}
