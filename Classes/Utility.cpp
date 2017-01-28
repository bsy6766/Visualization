#include "Utility.h"

size_t Utility::Random::seedNumber;
std::string Utility::Random::seedString;
std::mt19937 Utility::Random::generator;
bool Utility::Random::initialized = false;
std::chrono::steady_clock::time_point Utility::Time::begin;
std::chrono::steady_clock::time_point Utility::Time::end;