#ifndef __POINT_H__
#define __POINT_H__
// This is a very simple 2D point class used for speed

class Point {
public:
	Point() = default;
	Point(double x, double y) : _x(x), _y(y) {}

	double x() const { return _x; }
	double y() const { return _y; }

	void setX(double x) { _x = x; }
	void setY(double y) { _y = y; }

private:
	double _x;
	double _y;
};

#endif
