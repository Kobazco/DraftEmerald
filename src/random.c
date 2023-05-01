#include "global.h"
#include "random.h"

EWRAM_DATA static u8 sUnknown = 0;
EWRAM_DATA static u32 sRandCount = 0;

// IWRAM common
u32 gRngValue;
u32 gRng2Value;

u16 Random(void)
{
    gRngValue = ISO_RANDOMIZE1(gRngValue);
    sRandCount++;
    return gRngValue >> 16;
}

void SeedRng(u16 seed)
{
    gRngValue = seed;
    sUnknown = 0;
}

void SeedRng2(u16 seed)
{
    gRng2Value = seed;
}

u16 Random2(void)
{
    gRng2Value = ISO_RANDOMIZE1(gRng2Value);
    return gRng2Value >> 16;
}

u16 PRandom(u32 *state) {
    *state = ISO_RANDOMIZE1(*state);
    return *state >> 16;
}

// Randomizer port
#define I_MAX 5
u16 RandomSeededModulo(u32 value, u16 modulo)
{
    u32 otId;
    u32 RAND_MAX_TX;
    u32 result = 0;
    u8 i = 0;
    otId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    RAND_MAX_TX = 0xFFFFFFFF - (0xFFFFFFFF % modulo);
    gRngValue = ISO_RANDOMIZE1(gRngValue);

    do
    {
        result = ISO_RANDOMIZE1(gRngValue + value + result);
    }
    while ((result >= RAND_MAX_TX) && (++i != I_MAX));

    return (result % modulo);
}