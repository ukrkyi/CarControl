/* (c) 2020 ukrkyi */
#include "dataprocessor.h"

#include <algorithm>

#define PI M_PIf32

DataProcessor::DataProcessor()
{
}

void DataProcessor::processData(Position position, float distance, bool obstacle)
{
	// Static variables
	static const float wide_angle = 0.4f;
	static CellPosition occ[width * 2];
	// Function variables
	unsigned occ_n = 0;
	float p_sum = 0;
	Point center = translate(position.x, position.y, position.angle);
	float angle = position.angle + 0.014; // mean angle
	unsigned max_y, min_y, max_x, min_x;


	if ((angle - wide_angle <= 0) && (angle + wide_angle >= 0)) {
		max_y = getGridY(center.y + distance);

		min_x = getGridX(center.x + distance * sinf(angle - wide_angle));
		max_x = getGridX(center.x + distance * sinf(angle + wide_angle));
	} else if (angle + wide_angle < 0) {
		max_y = getGridY(center.y + distance * cosf(angle + wide_angle));

		if (angle - wide_angle >= -PI / 2)
			min_x = getGridX(center.x + distance * sinf(angle - wide_angle));
		else
			min_x = getGridX(center.x - distance);
		max_x = getGridX(center.x);
	} else if (angle - wide_angle > 0) {
		max_y = getGridY(center.y + distance * cosf(angle - wide_angle));

		if (angle + wide_angle <= PI / 2)
			max_x = getGridX(center.x + distance * sinf(angle + wide_angle));
		else
			max_x = getGridX(center.x + distance);
		min_x = getGridX(center.x);
	} else {
		while(1); // This should never happen but to silence warning
	}

	if (angle - wide_angle < -PI / 2)
		min_y = getGridY(center.y + distance * cosf(angle - wide_angle));
	else if (angle + wide_angle > PI / 2)
		min_y = getGridY(center.y + distance * cosf(angle + wide_angle));
	else
		min_y = getGridY(center.y);

	for (unsigned y = min_y; y <= max_y; y++)
		for (unsigned x = min_x; x <= max_x; x++) {
			Rectangle rect = getRealCoordinates(x, y);
			CollisionType col = checkIntersection(
				{ center, distance, angle - wide_angle, angle + wide_angle },
				rect);
			if (col == OUTSIDE)
				continue;

			Point rectCenter = getRectangleCenter(rect);
			float angle_d = getAngle(rectCenter, center) - angle;
			float center_d = getDistance(rectCenter, center);
			if (fabsf(angle_d) <= wide_angle) { // Ignore arc end
				if (col == INSIDE) {
					if (!(center_d < distance))
						while (1);

					grid[y][x].empty((1 - sqr(angle_d / wide_angle)) *
							 (1 - sqr((center_d - min_distance) / (distance - min_distance))));
				} else if (col == CROSS && obstacle) {
					float prob = 1 - sqr(angle_d / wide_angle);
					occ[occ_n++] = { x, y, prob };
					p_sum += prob;
				}
			}
		}

	for (unsigned i = 0; i < occ_n; i++)
		grid[occ[i].y][occ[i].x].occupied(occ[i].probability / p_sum);
}

void DataProcessor::clear()
{
	std::fill(&(grid[0][0]), &(grid[0][0]) + sizeof(grid), Cell());
}

DataProcessor::Point DataProcessor::translate(float x, float y, float angle) const
{
	static const float rangeAccelDistance = 20;

	return Point{ .x = y + rangeAccelDistance * sinf(angle), .y = x + rangeAccelDistance * cosf(angle) };
}

DataProcessor::CollisionType DataProcessor::checkIntersection(const Arc &arc, const Rectangle &rect) const
{
	if (inRange(arc.center.y, rect.lowLeft.y, rect.upRight.y) &&
	    inRange(arc.center.x, rect.lowLeft.x, rect.upRight.x)) {
		// center lies inside rectangle
		if (crossArcVertical(arc, rect.lowLeft.x, rect.lowLeft.y, rect.upRight.y) ||
		    crossArcVertical(arc, rect.upRight.x, rect.lowLeft.y, rect.upRight.y) ||
		    crossArcHorizontal(arc, rect.lowLeft.y, rect.lowLeft.x, rect.upRight.x) ||
		    crossArcHorizontal(arc, rect.upRight.y, rect.lowLeft.x, rect.upRight.x))
			return CROSS;
		else
			return INSIDE;
	} else {
		// center lies outside rectangle
		CollisionType pointCollision = intersectPoints(arc, rect);

		if (pointCollision != OUTSIDE) {
			return pointCollision;
		} else if (inRange(arc.center.y, rect.lowLeft.y, rect.upRight.y)) {
			/* center Y is aligned with rectangle y - there might be corner case like this:
			 * -+
			 *  |
			 * (|
			 *  |
			 * -+
			 */
			float nearestX;
			if (arc.center.x > rect.upRight.x)
				nearestX = rect.upRight.x;
			else
				// arc.center.x < rect.lowLeft.x
				nearestX = rect.lowLeft.x;
			if (arc.radius < fabsf(arc.center.x - nearestX) ||
			    !crossArcVertical(arc, nearestX, rect.lowLeft.y, rect.upRight.y))
				return OUTSIDE;
			else
				return CROSS;
		} else if (inRange(arc.center.x, rect.lowLeft.x, rect.upRight.x)) {
			/* center X is aligned with rectangle x - there might be corner case like this:
			 * +---+
			 * | U |
			 */
			float nearestY;
			if (arc.center.y > rect.upRight.y)
				nearestY = rect.upRight.y;
			else
				// arc.center.y < rect.lowLeft.y
				nearestY = rect.lowLeft.y;

			if (arc.radius < fabsf(arc.center.y - nearestY) ||
			    !crossArcHorizontal(arc, nearestY, rect.lowLeft.x, rect.upRight.x))
				return OUTSIDE;
			else
				return CROSS;
		} else
			return OUTSIDE;
	}
}

DataProcessor::CollisionType DataProcessor::intersectPoints(const DataProcessor::Arc &arc, const DataProcessor::Rectangle &rect) const
{
	Point vert[4] = {
		rect.lowLeft,
		{rect.lowLeft.x, rect.upRight.y},
		rect.upRight,
		{rect.upRight.x, rect.lowLeft.y}
	};
	unsigned inCircle = 0, inArc = 0;
	for (const Point& pt: vert){
		float len = getDistance(pt, arc.center);
		if (len >= arc.radius)
			// Point outside
			continue;

		inCircle++;
		if (inRange(atan2f(pt.x - arc.center.x, pt.y - arc.center.y), arc.start_angle, arc.end_angle)) {
			inArc++;
		}
	}

	if (inArc == 0) {
		// None of points lie inside a circle
		return OUTSIDE;
	} else {
		// Points are partly inside partly outside an arc
		if (inCircle != 4) {
			// Rectangle is crosed by an arc
			return CROSS;
		} else {
			/* Whole rectangle lies inside a circle,
			 * so there may be two cases:
			 *	- Whole rectangle lies inside an arc (inArc == 4)
			 *	- Part of rectangle lies inside an arc (inArc < 4)
			 */
			return INSIDE;
		}
	}
}

bool DataProcessor::crossArcVertical(const Arc &arc, float x, float low, float high) const
{
	float angle1 = asinf((x - arc.center.x) / arc.radius);
	float angle2 = copysignf(acosf(-fabsf(x - arc.center.x) / arc.radius), x - arc.center.x);

	// if line is outside a circle, angles should be NaN and following will return false

	if (inRange(angle1, arc.start_angle, arc.end_angle) &&
	    inRange(arc.center.y + cosf(angle1) * arc.radius, low, high))
		return true;
	else if (inRange(angle2, arc.start_angle, arc.end_angle) &&
		 inRange(arc.center.y + cosf(angle2) * arc.radius, low, high))
		return true;
	else
		return false;
}

bool DataProcessor::crossArcHorizontal(const Arc &arc, float y, float left, float right) const
{
	float angle1 = -acosf((y - arc.center.y) / arc.radius);
	float angle2 = acosf((y - arc.center.y) / arc.radius);

	// if line is outside a circle, angles should be NaN and following will return false

	if (inRange(angle1, arc.start_angle, arc.end_angle) &&
	    inRange(arc.center.y + sinf(angle1) * arc.radius, left, right))
		return true;
	else if (inRange(angle2, arc.start_angle, arc.end_angle) &&
		 inRange(arc.center.y + sinf(angle2) * arc.radius, left, right))
		return true;
	else
		return false;
}

const float DataProcessor::Cell::limit = 250;

void DataProcessor::Cell::empty(float probability)
{
	float prob0 = 1 - getOccupancy();
	float result = prob0 + probability - prob0 * probability;

	setOccupancy(1 - result);
}

void DataProcessor::Cell::occupied(float probability)
{
	// NOTE maybe we need to lower probability value if cell was considered empty before
	float prob0 = getOccupancy();
	float result = prob0 + probability - prob0 * probability;

	setOccupancy(result);
}

DataProcessor::Cell::Cell() : occupancy(limit / 2)
{
}
