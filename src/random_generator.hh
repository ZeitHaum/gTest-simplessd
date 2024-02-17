#pragma once
#include "def.hh"
#include <random>
#include <cstdint>
#include "util/bitset.hh"
#include <cassert>

extern std::random_device rd;
extern std::mt19937 gen;
extern const uint32_t RANDOM_SEED;

template <typename T>
T getRandomInt(T l, T r);

template <typename T>
T getRandomFloating(T l, T r);

void getRandomBitset(SimpleSSD::Bitset& ret);

template <typename T>
void getRamdomVector(std::vector<T>& ret, T l, T r);

uint8_t getRandomByte();
