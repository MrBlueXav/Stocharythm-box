constexpr float kPI      = 3.14159265359f;
constexpr float kPI_2    = kPI / 2.0f;
constexpr float kPI_4    = kPI / 4.0f;
constexpr float k2PI     = 2.0f * kPI;

// sin_table[0 ... kSinTableSize-1] contient sin(x) pour x dans [0, π/2]

float FastSin(float x)
{
    // Ramène x dans [0, 2π]
    while(x < 0)      x += k2PI;
    while(x >= k2PI)  x -= k2PI;

    float result;
    if(x <= kPI_2)
    {
        // [0, π/2]
        int idx = static_cast<int>(x * (kSinTableSize - 1) / kPI_2);
        result = sin_table[idx];
    }
    else if(x <= kPI)
    {
        // [π/2, π] : sin(x) = sin(π - x)
        float y = kPI - x;
        int idx = static_cast<int>(y * (kSinTableSize - 1) / kPI_2);
        result = sin_table[idx];
    }
    else if(x <= 3.0f * kPI_2)
    {
        // [π, 3π/2] : sin(x) = -sin(x - π)
        float y = x - kPI;
        int idx = static_cast<int>(y * (kSinTableSize - 1) / kPI_2);
        result = -sin_table[idx];
    }
    else
    {
        // [3π/2, 2π] : sin(x) = -sin(2π - x)
        float y = k2PI - x;
        int idx = static_cast<int>(y * (kSinTableSize - 1) / kPI_2);
        result = -sin_table[idx];
    }
    return result;
}


import math
N = 128
for i in range(N):
    v = math.sin(math.pi/2 * i / (N-1))
    print("{:.6f}f,".format(v))


#include <cmath>
constexpr int kTableSize = 1024;
float sine_table[kTableSize];

// Initialisation (à placer dans une fonction d'init)
void InitSineTable()
{
    for(int i = 0; i < kTableSize; ++i)
    {
        float phase = static_cast<float>(i) / kTableSize; // 0 à 1
        sine_table[i] = sinf(phase * 2.0f * M_PI);
    }
}

// Utilisation
float FastSin(float phase) // phase entre 0 et 1
{
    int idx = static_cast<int>(phase * kTableSize) % kTableSize;
    return sine_table[idx];
}
