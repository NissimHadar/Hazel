#ifndef __PART_2_H__
#define __PART_2_H__

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QWidget>

#include <unordered_set>
#include <vector>

#include "Point.h"

class Part_2 : public QGraphicsScene, public QWidget {
public:
	Part_2(int x, int y, int width, int height, QObject* parent = nullptr);

	void drawGrid();

	void mousePressEvent(QGraphicsSceneMouseEvent* event);

	bool inSquare(double x, double y, std::shared_ptr<QGraphicsRectItem> square);
	void generate();
	void clear();
	
	bool computeAccurateFit();

	// Implementation of Kasa's algorithm to find best fitting circle to set of 2D points
	bool KasaCircleFit();

private:
	int sceneWidth;
	int sceneHeight;

	int numPointsWide;
	int numPointsHigh;

	int squareSize;

	double gridSpacingX;
	double gridSpacingY;

	std::vector<std::shared_ptr<QGraphicsRectItem>> squares;

	std::unordered_set<std::shared_ptr<QGraphicsRectItem>> selectedSquares;
	std::vector<Point> points;

	Point circleCentre;
	double circleRadius;

	std::unique_ptr<QGraphicsEllipseItem> circle;
};

#endif
