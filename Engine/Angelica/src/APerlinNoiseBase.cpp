#include "pch.h"
#include "APerlinNoiseBase.h"

// Constants for the LCG (Park-Miller minimal standard linear congruential generator)
static constexpr std::uint32_t LCG_MULTIPLIER = 16807U;
static constexpr std::int64_t LCG_MODULUS = 2147483647LL; // 2^31 - 1

APerlinNoiseBase::APerlinNoiseBase()
    : m_bTurbulence(true)
    , m_dwSeed(1U)
    , m_nActiveOctave(-1)
    , m_nBaseWaveLength(0)
    , m_vBaseAmplitude(0.0f)
    , m_vPersistence(0.0f)
    , m_nOctaveNum(0)
    , m_nStartPos{}
    , m_nWaveLength{}
    , m_vAmplitude{}
{}

APerlinNoiseBase::~APerlinNoiseBase() = default;

bool APerlinNoiseBase::InitParams(float vAmplitude, int nWaveLength, float vPersistence, int nOctaveNum)
{
    if (nOctaveNum > MAX_OCTAVE || nOctaveNum <= 0)
        return false;

    m_vBaseAmplitude = vAmplitude;
    m_nBaseWaveLength = nWaveLength;
    m_vPersistence = vPersistence;
    m_nOctaveNum = nOctaveNum;

    // Use standard abs or manual — but avoid std::abs for int/float mix ambiguity
    if (m_vBaseAmplitude < 0.0f)
        m_vBaseAmplitude = -m_vBaseAmplitude;

    if (m_nBaseWaveLength < 0)
        m_nBaseWaveLength = -m_nBaseWaveLength;

    if (m_vPersistence < 0.0f)
        m_vPersistence = -m_vPersistence;

    float vTotalAmplitude = 0.0f;
    float vThisAmplitude = 1.0f;
    int nThisWaveLen = m_nBaseWaveLength;

    int actualOctaves = 0;
    for (int i = 0; i < m_nOctaveNum; ++i)
    {
        vTotalAmplitude += vThisAmplitude;
        m_vAmplitude[i] = vThisAmplitude;
        m_nWaveLength[i] = nThisWaveLen;
        m_nStartPos[i] = static_cast<int>(RandInteger() % 1023);

        vThisAmplitude *= m_vPersistence;
        nThisWaveLen /= 2;

        actualOctaves = i + 1;

        if (nThisWaveLen <= 0)
        {
            m_nOctaveNum = actualOctaves;
            break;
        }
    }

    // Normalize amplitudes
    if (vTotalAmplitude > 0.0f)
    {
        const float scale = m_vBaseAmplitude / vTotalAmplitude;
        for (int i = 0; i < m_nOctaveNum; ++i)
            m_vAmplitude[i] *= scale;
    }

    return true;
}

unsigned int APerlinNoiseBase::RandInteger()
{
    // Use 64-bit arithmetic to avoid overflow, then modify
    const std::int64_t product = static_cast<std::int64_t>(LCG_MULTIPLIER) * static_cast<std::int64_t>(m_dwSeed);
    m_dwSeed = static_cast<unsigned int>(product % LCG_MODULUS);
    return m_dwSeed;
}

float APerlinNoiseBase::RandFloat()
{
    // Range: [-1.0f, 1.0f], step = 0.001f
    const int value = static_cast<int>(RandInteger() % 2001) - 1000;
    return static_cast<float>(value) / 1000.0f;
}
