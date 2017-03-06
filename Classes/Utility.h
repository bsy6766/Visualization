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
	namespace Math
	{
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

		static inline const float determinant(const cocos2d::Vec2& a, const cocos2d::Vec2& b)
		{
			return (a.x * b.y) - (a.y * b.x);
		}
	}

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

		static inline std::chrono::steady_clock::time_point now()
		{
			return std::chrono::steady_clock::now();
		}

		static inline const std::string toMicroSecondString(const std::chrono::steady_clock::time_point start, const std::chrono::steady_clock::time_point end)
		{
			std::stringstream ss;
			ss << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			return ss.str();
		}

		static inline const std::string toMilliSecondString(const std::chrono::steady_clock::time_point start, const std::chrono::steady_clock::time_point end)
		{
			std::stringstream ss;
			ss << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			return ss.str();
		}

		static const std::string getElaspedTime()
		{
			std::stringstream ss;
			ss << std::chrono::duration_cast<std::chrono::microseconds>(Time::end - Time::begin).count();
			return ss.str();
		}
	};

	namespace Rect
	{
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
	}

	namespace Polygon
	{
		// Check if two segment is parallel
		static const bool isParallel(const cocos2d::Vec2& a, const cocos2d::Vec2& b, const cocos2d::Vec2& c, const cocos2d::Vec2& d)
		{
			auto p1 = b - a;
			auto p2 = d - c;

			return p1.cross(p2) == 0.0f;
		}

		// Returns positive if vec b is ccw from vec a, returns negative if cw from vec a. 0 if parrellel
		static const float ccw(const cocos2d::Vec2& a, const cocos2d::Vec2& b)
		{
			return a.cross(b);
		}

		// From point p, if vec v is ccw from vec a, return positive. if cw, return negative. 0 if parrellel
		static const float ccw(const cocos2d::Vec2& p, const cocos2d::Vec2& a, const cocos2d::Vec2& b)
		{
			return ccw(a - p, b - p);
		}

		// Check if two segment ((a, b), (c, d)) intersects.
		static const bool doesSegmentIntersects(cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 c, cocos2d::Vec2 d)
		{
			float ab = ccw(a, b, c) * ccw(a, b, d);
			float cd = ccw(c, d, a) * ccw(c, d, b);

			// In case where two lines are aligned in one segment or end point touches
			/*
			if(ab == 0 && cd == 0)
			{
				if (b < a)
				{
					auto t = a;
					a = b;
					b = t;
				}

				if(d < c)
				{
					auto t = c;
					c = d;
					d = c;
				}

				return !(b < c || d < a);
			}
			else
			{
				return ab <= 0 && cd <= 0;
			}
			*/

			// if ab, cd is 0, it means end point touches each other.
			return ab < 0 && cd < 0;
		}

		/**
		*	@brief Check if point p is in or on triangle(a, b, c)
		*/
		static const bool isPointInOrOnTriangle(const cocos2d::Vec2& a, const cocos2d::Vec2& b, const cocos2d::Vec2& c, const cocos2d::Vec2& p)
		{

			return Utility::Math::determinant(p - a, b - a) >= 0 &&
				Utility::Math::determinant(p - b, c - b) >= 0 &&
				Utility::Math::determinant(p - c, a - c) >= 0;
		}

		/**
		*	@brief Check if point is in polygon.
		*/
		static const bool isPointInPolygon(std::list<cocos2d::Vec2>& verticies, const cocos2d::Vec2& point)
		{
			auto c = point;
			auto d = point;
			d.x += 10000.0f;  // Create a long line, think as raycasting to right.

			auto p1 = verticies.begin();
			auto p2 = verticies.begin();
			std::advance(p2, 1);

			int count = 0;
			for (; p2 != verticies.end(); )
			{
				bool intersect = doesSegmentIntersects(*p1, *p2, c, d);
				if (intersect)
				{
					count++;
				}

				p1++;
				p2++;

				if (p2 == verticies.end())
				{
					p2 = verticies.begin();
					bool intersect = doesSegmentIntersects(*p2, *p1, c, d);
					if (intersect)
					{
						count++;
					}
					break;
				}
			}

			if (count % 2 == 0)
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		// Check if point is valid for polygon. This checks if segment of last vertex in polygon and new point intersects any exisiting segments in polygon.
		static const bool doesPointIntersectPolygonSegments(std::list<cocos2d::Vec2>& verticies, const cocos2d::Vec2& point)
		{
			if (verticies.size() == 1)
			{
				// Return false if polygon has only single vertex
				return false;
			}
			else if (verticies.size() == 2)
			{
				// Only case that new segment intersects existing segment is when it's parallel.
				auto iter = verticies.begin();
				auto a = *iter;
				iter++;
				auto b = *iter;
				if (Polygon::isParallel(a, b, b, point))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			auto p1 = verticies.begin();
			auto p2 = verticies.begin();
			std::advance(p2, 1);
			auto end_it = verticies.end();
			std::advance(end_it, -1);
			auto lastVertex = verticies.back();
			for (; p2 != end_it; )
			{
				cocos2d::Vec2 a = *p1;
				cocos2d::Vec2 b = *p2;

				bool intersect = Utility::Polygon::doesSegmentIntersects(a, b, lastVertex, point);
				if (intersect)
				{
					return true;
				}

				p1++;
				p2++;
			}

			return false;
		}
	}


}
#endif