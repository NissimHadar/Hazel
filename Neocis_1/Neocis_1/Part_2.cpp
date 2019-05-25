#include "Part_2.h"

#include <QGraphicsRectItem>
#include <QFile>
#include <QMessageBox>

#include <limits>

Part_2::Part_2(int x, int y, int width, int height, QObject* parent) :
	QGraphicsScene(x, y, width, height),

	sceneWidth { width },
	sceneHeight{ height },
	numPointsWide{ 20 },
	numPointsHigh{ 20 },
	squareSize{ 12 },
	circle{ nullptr }
{
	// Note that 1.0 is used to coerce double division
	gridSpacingX = sceneWidth / (numPointsWide + 1.0);
	gridSpacingY = sceneHeight / (numPointsHigh + 1.0);

	drawGrid();
}

// Draws a rectangle of squares, evenly divided over the scene
void Part_2::drawGrid() {
	squares.clear();
	for (int row = 1; row <= numPointsWide; ++row) {
		for (int col = 1; col <= numPointsHigh; ++col) {
			int squareXCentre = row * gridSpacingX;
			int squareYCentre = col * gridSpacingY;

			std::unique_ptr<QGraphicsRectItem> square = std::make_unique<QGraphicsRectItem>(
				squareXCentre - squareSize / 2.0,
				squareYCentre - squareSize / 2.0,
				squareSize,
				squareSize
			);

			square->setBrush(QBrush(Qt::gray));
			addItem(square.get());

			squares.emplace_back(std::move(square));
		}
	}
}
void Part_2::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	double x = event->scenePos().x();
	double y = event->scenePos().y();

	// If mouse is hovering over a square that isn't the active square then toggle this square's selection
	// else, de-activate any active square
	for (auto& square : squares) {
		if (inSquare(x, y, square)) {
			if (selectedSquares.find(square) == selectedSquares.end()) {
				selectedSquares.insert(square);
				square->setBrush(QBrush(Qt::green));
			}
			else {
				selectedSquares.erase(square);
				square->setBrush(QBrush(Qt::gray));
			}
		}
	}
}

// Returns true iff (x, y) is in the square
bool Part_2::inSquare(double x, double y, std::shared_ptr<QGraphicsRectItem> square) {
	double centreX = square->rect().center().x();
	double centreY = square->rect().center().y();

	return (fabs(x - centreX) < squareSize / 2.0 && fabs(y - centreY) < squareSize / 2.0);
}

// This algorithm generates a circle from a set of points.
// The algorithm proceeds in 2 stages:
//
//		1 - For every (non-colinear) subset of 3 points - compute the unique circle passing between these 3 points
//			Compute an "average" circle as an initial guess
//		3 - Improve the result by using a least square estimator
//
//	The code is based on the following paper - http://www.spaceroots.org/documents/circle/circle-fitting.pdf
//	(paper has been included with code
void Part_2::generate() {
	// Need at least 3 points to define a circle
	if (selectedSquares.size() < 3) {
		QMessageBox::information(0, "No circle defined", "At least 3 points are needed");
		return;
	}

	// It is clearer to work here with a vector of points rather than a set of squares
	points.clear();
	for (auto square : selectedSquares) {
		points.emplace_back(square->rect().center().x(), square->rect().center().y());
	}

	bool circleFound{ false };
	if (selectedSquares.size() == 3) {
		circleFound = computeAccurateFit();
	} else {
		circleFound = KasaCircleFit();
	}

	if (circleFound) {
		circle = std::make_unique<QGraphicsEllipseItem>(circleCentre.x() - circleRadius, circleCentre.y() - circleRadius, 2.0 * circleRadius, 2.0 * circleRadius);

		QPen pen;
		pen.setBrush(QBrush(Qt::blue));
		circle->setPen(pen);

		addItem(circle.get());
	}
}

void Part_2::clear() {
	for (auto square : squares) {
		square->setBrush(QBrush(Qt::gray));
	}

	selectedSquares.clear();

	removeItem(circle.get());
}

bool Part_2::computeAccurateFit() {
	// For brevity
	double xi = points[0].x();
	double yi = points[0].y();
	double xj = points[1].x();
	double yj = points[1].y();
	double xk = points[2].x();
	double yk = points[2].y();

	// Compute determinant
	double determinant = (xk - xj) * (yj - yi) - (xj - xi) * (yk - yj);

	if (determinant == 0.0) {
		QMessageBox::information(0, "No circle defined", "Points cannot be on a straight line");
		return false;
	}

	// The 3 points create 2 segments ij and jk
	// The centre of the induced circle is the intersection of the 2 perpendicular bisectors to these segments
	circleCentre.setX(
		((yk - yj) * (xi * xi + yi * yi) + (yi - yk) * (xj * xj + yj * yj) + (yj - yi) * (xk * xk + yk * yk))
		/
		( 2.0 * determinant)
	);

	circleCentre.setY(
		-((xk - xj) * (xi * xi + yi * yi) + (xi - xk) * (xj * xj + yj * yj) + (xj - xi) * (xk * xk + yk * yk))
		/
		(2.0 * determinant)
	);

	// The radius is just the distance from any of the points to the circle centre
	double dx = circleCentre.x() - xi;
	double dy = circleCentre.y() - yi;

	circleRadius = sqrt(dx * dx + dy * dy);

	return true;
}

//		  Circle fit to a given set of data points (in 2D)
//
//		  This is an algebraic fit, disovered and rediscovered by many people.
//		  One of the earliest publications is due to Kasa:
//
//		  I. Kasa, "A curve fitting procedure and its error analysis",
//		  IEEE Trans. Inst. Meas., Vol. 25, pages 8-14, (1976)
//
//		 The method is based on the minimization of the function
//
//					 F = sum [(x-a)^2 + (y-b)^2 - R^2]^2
//
//		 This is perhaps the simplest and fastest circle fit.
//
//		 It works well when data points are sampled along an entire circle
//		 or a large part of it (at least half circle).
//
//		 It does not work well when data points are sampled along a small arc
//		 of a circle. In that case the method is heavily biased, it returns
//		 circles that are too often too small.
//
//		 It is the oldest algebraic circle fit (first published in 1972?).
//		 For 20-30 years it has been the most popular circle fit, at least
//		 until the more robust Pratt fit (1987) and Taubin fit (1991) were invented.
//
//		   Nikolai Chernov  (September 2012)
bool Part_2::KasaCircleFit() {
	// Compute x- and y- sample means
	double accumulatorX{ 0.0 };
	double accumulatorY{ 0.0 };
	int counter{ 0 };
	// We are assured there are at least 3 points
	for (int i = 0; i < points.size(); ++i) {
		accumulatorX += points[i].x();
		accumulatorY += points[i].y();

		++counter;
	}

	double meanX{ accumulatorX / counter };
	double meanY{ accumulatorY / counter };

	// Compute moments 
	double mxx{ 0.0 };
	double myy{ 0.0 };
	double mxy{ 0.0 };
	double mxz{ 0.0 };
	double myz{ 0.0 };

	for (int i = 0; i < points.size(); ++i) {
		double xi = points[i].x() - meanX;   //  centered x-coordinates
		double yi = points[i].y() - meanY;   //  centered y-coordinates
		double zi = xi * xi + yi * yi;

		mxx += xi * xi;
		myy += yi * yi;
		mxy += xi * yi;
		mxz += xi * zi;
		myz += yi * zi;
	}

	mxx /= points.size();
	myy /= points.size();
	mxy /= points.size();
	mxz /= points.size();
	myz /= points.size();

	// Solving system of equations by Cholesky factorization
	double g11 = sqrt(mxx);
	double g12 = mxy / g11;

	double g22 = sqrt(myy - g12 * g12);

	const double EPSILON{ 0.00001 };
	if (g22 < EPSILON) {
		QMessageBox::information(0, "No circle defined", "Points cannot be on a straight line");
		return false;
	}

	double d1 = mxz / g11;
	double d2 = (myz - d1 * g12) / g22;

	// Computing paramters of the fitting circle

	double c = d2 / g22 / 2.0;
	double b = (d1 - g12 * c) / g11 / 2.0;

	// Asssembling the output
	circleCentre.setX(b + meanX);
	circleCentre.setY(c + meanY);
	circleRadius = sqrt(b * b + c * c + mxx + myy);

	return true;
}
