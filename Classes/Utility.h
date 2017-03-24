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

		// brief Check if point p is in or on triangle(a, b, c)
		static const bool isPointInOrOnTriangle(const cocos2d::Vec2& a, const cocos2d::Vec2& b, const cocos2d::Vec2& c, const cocos2d::Vec2& p)
		{

			return Utility::Math::determinant(p - a, b - a) >= 0 &&
				Utility::Math::determinant(p - b, c - b) >= 0 &&
				Utility::Math::determinant(p - c, a - c) >= 0;
		}

		// Check if point is in polygon.
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

		// Check if polygon is counter clock wise(true). Else, it's clock wise(false)
		static const bool isCounterClockWise(const std::list<cocos2d::Vec2>& verticies)
		{
			int total = 0;
			int index = 0;
			auto it = verticies.begin();
			auto next = verticies.begin();
			std::advance(next, 1);

			for (; next != verticies.end(); )
			{
				total += ((next->x - it->x) * (next->y + it->y));
				index++;
				it++;
				next++;

				if (next == verticies.end())
				{
					next = verticies.begin();
					total += ((next->x - it->x) * (next->y + it->y));
					break;
				}
			}

			if (total >= 0)
			{
				// clock wise
				return false;
			}
			else
			{
				// counter clock wise
				return true;
			}
		}
	}

	class SimplexNoise
	{
	private:
		// Pre calculated values
		static const float F2;	//F2 = 0.5f * (sqrt(3.0f) - 1.0f)) = 0.366025403f
		static const float G2;	//G2 = (3.0f - sqrt(3.0f)) / 6.0f = 0.211324865f

		// Permutation table. This is random numbers between 0 - 255 and it's fixed. 
		// All Simplex Noise implementation uses same table.
		// Originally the size of table is 255, but doubled the size by repeating
		// sequence of numbers to avoid wrapping index at 255 for each look up
		static int perm[512];

		// Gradient direction
		static const int grad3[12][3];

		// Fast floor. Faster than math.floor
		static float fastFloor(float value)
		{
			return value > 0 ? static_cast<int>(value) : static_cast<int>(value) - 1;
		}

		// Dot product
		static float dot(const int* grad, const float x, const float y)
		{
			return grad[0] * x + grad[1] * y;
		}
	public:
		// Simplex Noise 2D
		static float noise(const cocos2d::Vec2& v)
		{
			// Reference: http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf

			// Noise contributions from the three corners
			float n0, n1, n2;

			// Skew the input space to determine which simplex cell we're in
			float s = (v.x + v.y) * F2; // Hairy factor for 2D
			int i = fastFloor(v.x + s);
			int j = fastFloor(v.y + s);

			float t = static_cast<float>(i + j) * G2;
			float X0 = i - t;	// Unskew the cell origin back to (x,y) space
			float Y0 = j - t;
			float x0 = v.x - X0; // The x,y distances from the cell origin
			float y0 = v.y - Y0;

			// For the 2D case, the simplex shape is an equilateral triangle.
			// Determine which simplex we are in.
			int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
			if (x0 > y0) 
			{ 
				// lower triangle, XY order: (0,0)->(1,0)->(1,1)
				i1 = 1; 
				j1 = 0; 
			} 
			else 
			{ 
				// upper triangle, YX order: (0,0)->(0,1)->(1,1)
				i1 = 0; 
				j1 = 1; 
			} 

			// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
			// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
			// c = (3-sqrt(3))/6

			float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
			float y1 = y0 - j1 + G2;
			float x2 = x0 - 1.0f + 2.0f * G2; // Offsets for last corner in (x,y) unskewed coords
			float y2 = y0 - 1.0f + 2.0f * G2;

			// Work out the hashed gradient indices of the three simplex corners
			int ii = i & 255;
			int jj = j & 255;
			int gi0 = perm[ii + perm[jj]] % 12;
			int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
			int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

			// Calculate the contribution from the three corners
			float t0 = 0.5f - (x0 * x0) - (y0 * y0);
			if (t0 < 0.0f)
			{
				n0 = 0.0f;
			}
			else 
			{
				t0 *= t0;
				n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
			}

			float t1 = 0.5f - (x1 * x1) - (y1 * y1);
			if (t1 < 0)
			{
				n1 = 0.0;
			}
			else 
			{
				t1 *= t1;
				n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
			}

			float t2 = 0.5f - (x2 * x2) - (y2 * y2);
			if (t2 < 0)
			{
				n2 = 0.0;
			}
			else 
			{
				t2 *= t2;
				n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
			}

			// Add contributions from each corner to get the final noise value.
			// The result is scaled to return values in the interval [-1,1].
			return 70.0f * (n0 + n1 + n2);
		}

		// Reset perm table
		static void resetSeed()
		{
			int defaultPerm[512] = { 151,160,137,91,90,15,
				131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
				190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
				88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
				77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
				102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
				135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
				5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
				223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
				129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
				251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
				49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
				138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
				151,160,137,91,90,15,
				131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
				190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
				88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
				77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
				102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
				135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
				5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
				223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
				129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
				251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
				49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
				138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

			std::copy(std::begin(defaultPerm), std::end(defaultPerm), std::begin(perm));
		}
		
		// Set seed and randomize perm table
		static void setSeed(const size_t seed)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			gen.seed(seed);
			std::uniform_int_distribution<int> distribution(1, 255);

			for (int i = 0; i < 256; i++)
			{
				perm[i] = perm[i + 256] = distribution(gen);
			}
		}

		static void setSeed(const std::string& seed)
		{
			return setSeed(std::hash<std::string>{}(seed));
		}
	};
}
#endif