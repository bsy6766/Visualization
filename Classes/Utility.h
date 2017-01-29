#ifndef MY_UTILITY
#define MY_UTILITY

#include "cocos2d.h"
#include <random>
#include <string>
#include <sstream>
#include <functional>	// std::hash
#include <chrono>		// time

namespace Utility
{
	class Random
	{
	private:
		static size_t seedNumber;
		static std::string seedString;

		static std::mt19937 generator;

		static bool initialized;

		static inline void generateRandomSeedString()
		{
			std::string randStr = "";
			const int len = 6;

			const char capAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

			for (int i = 0; i < len; i++)
			{
				randStr += capAlphabet[cocos2d::random() % (sizeof(capAlphabet) - 1)];
			}

			cocos2d::log("Created random string seed = %s", randStr.c_str());

			seedString = randStr;
		}

		static inline void generateSeed()
		{
			generateRandomSeedString();

			seedNumber = std::hash<std::string>{}(seedString);

			generator.seed(seedNumber);
		}

	public:
		static inline int randomInt(int min, int max)
		{
			if (seedNumber == 0 || seedString == "")
			{
				generateSeed();
			}

			if (min > max)
			{
				std::swap(min, max);
			}

			std::uniform_int_distribution<int> dist(min, max);
			return dist(generator);
		}

		static inline int randomIntExponential()
		{
			std::exponential_distribution<double> dist(1);
			return static_cast<int>(dist(generator));
		}

		static inline int randomInt100()
		{
			return randomInt(0, 100);
		}

		static inline bool randomIntRollCheck(int chance)
		{
			const int randomInt = randomInt100();
			if (randomInt <= chance)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		template<typename T>
		static inline T randomReal(T min, T max)
		{
			if (seedNumber == 0 || seedString == "")
			{
				generateSeed();
			}

			if (min > max)
			{
				std::swap(min, max);
			}

			std::uniform_real_distribution<T> dist(min, max);
			return dist(generator);
		}

		static inline float random_minus_1_1()
		{
			int rand = randomInt(0, 1);
			return rand == 0 ? 1.0f : -1.0f;
		}

		static inline void init()
		{
			if (Utility::Random::initialized != true)
			{
				seedNumber = 0;
				seedString = "";

				generateSeed();

				Utility::Random::initialized = true;
			}
		}

		static inline void setSeed(const std::string& seed)
		{
			seedString = seed;
			seedNumber = std::hash<std::string>{}(seedString);
			generator.seed(seedNumber);
		}

	};

	class Time
	{
	private:
		static std::chrono::steady_clock::time_point begin;
		static std::chrono::steady_clock::time_point end;
	public:
		static inline void start()
		{
			Time::begin = std::chrono::steady_clock::now();
		}

		static inline void stop()
		{
			Time::end = std::chrono::steady_clock::now();
		}

		static const std::string getElaspedTime()
		{
			std::stringstream ss;
			ss << std::chrono::duration_cast<std::chrono::microseconds>(Time::end - Time::begin).count();
			return ss.str();
		}
	};

	/**
	*	@brief Checks if passed bounding box is inside of boundary.
	*
	*	@retval true Bounding box is completely in the boundary
	*	@retval false Bounding box is not in the boundary. But can be intersect.
	*/
	static inline const bool containsRect(const cocos2d::Rect& boundary, const cocos2d::Rect& target)
	{
		if (target.getMaxX() <= boundary.getMaxX() &&
			target.getMinX() >= boundary.getMinX() &&
			target.getMaxY() <= boundary.getMaxY() &&
			target.getMinY() >= boundary.getMinY())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	/**
	*	@brief Get intersecting cocos2d rect between two rects.
	*	@detail If either width or height is 0 or less than 0, then it means it's not intersecting.
	*/
	static inline const cocos2d::Rect getIntersectingRect(const cocos2d::Rect& first, const cocos2d::Rect& second)
	{
		cocos2d::Rect intersection;
		//get intersection Rect
		intersection.origin.x = (first.origin.x > second.origin.x ? first.origin.x : second.origin.x);
		intersection.origin.y = (first.origin.y > second.origin.y ? first.origin.y : second.origin.y);

		if (first.origin.x + first.size.width < second.origin.x + second.size.width)
		{
			intersection.size.width = first.origin.x + first.size.width - intersection.origin.x;
		}
		else
		{
			intersection.size.width = second.origin.x + second.size.width - intersection.origin.x;
		}

		if (first.origin.y + first.size.height < second.origin.y + second.size.height)
		{
			intersection.size.height = first.origin.y + first.size.height - intersection.origin.y;
		}
		else
		{
			intersection.size.height = second.origin.y + second.size.height - intersection.origin.y;
		}

		return intersection;
	}

	template<typename T>
	static inline const  T min(T first, T second)
	{
		return first < second ? first : second;
	}

	template<typename T>
	static inline const  T max(T first, T second)
	{
		return first > second ? first : second;
	}
}
#endif