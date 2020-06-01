/* (c) 2020 ukrkyi */
#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <stdint.h>
#include <math.h>

static inline float sqr(const float x) {return x*x;}

const float min_distance = 10;

struct Position {
	float x;
	float y;
	float angle;

	bool operator ==(const Position& other) {return x == other.x && y == other.y && angle == other.angle;}
	bool operator !=(const Position& other) {return !(*this == other);}
};

class DataProcessor
{
public:
	static const int discretization = 100;
	static const int width = 6000/discretization;
	static const int height = 10000/discretization;
	struct Cell {
		inline float getOccupancy() const { return occupancy / limit; };
		inline operator uint8_t() const {return occupancy; }
		void empty(float probability);
		void occupied(float probability);
		Cell();
	private:
		static const float limit;
		inline void setOccupancy(float val) { occupancy = (uint8_t)(limit * val); };
		uint8_t occupancy;
	};

	DataProcessor();
	Cell grid[height][width];

	inline unsigned getGridX(float x) const { return (x + discretization / 2) / discretization + width / 2; };
	inline unsigned getGridY(float y) const { return y > 0 ? y / discretization : 0; };

	struct Point { float x, y; };
	struct Rectangle { Point upRight, lowLeft; };

	inline Rectangle getRealCoordinates(unsigned x, unsigned y) const {
		Rectangle result;
		result.lowLeft.x = (float(x) - width / 2) * discretization - discretization / 2;
		result.upRight.x = (float(x) - width / 2) * discretization + discretization / 2;
		result.lowLeft.y = y * discretization;
		result.upRight.y = y * discretization + discretization;
		return result;
	};

	void processData(Position position, float distance, bool obstacle);
	void clear();

	Point translate(float x, float y, float angle) const;

private:
	struct CellPosition {
		unsigned x, y;
		float probability;
	};

	struct Arc {
		Point& center;
		float radius;
		float start_angle, end_angle;
	};

	enum CollisionType {OUTSIDE, CROSS, INSIDE};

	CollisionType checkIntersection(const Arc &arc, const Rectangle& rect) const;
	CollisionType intersectPoints(const Arc& arc, const Rectangle& rect) const;
	bool crossArcVertical(const Arc& arc, float x, float low, float high) const;
	bool crossArcHorizontal(const Arc& arc, float y, float left, float right) const;

	inline bool inRange(float x, float from, float to) const { return x >= from && x <= to; }

	inline float getAngle(const Point &point, const Point &origin) const { return atan2f(point.x - origin.x, point.y - origin.y);}
	inline float getDistance(const Point& pt, const Point& origin) const { return sqrtf(sqr(pt.x - origin.x) + sqr(pt.y - origin.y)); }
	inline Point getRectangleCenter(const Rectangle& rect) const { return {(rect.lowLeft.x + rect.upRight.x) / 2, (rect.lowLeft.y + rect.upRight.y) / 2};}
};

#endif // DATAPROCESSOR_H
