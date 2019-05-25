#include "Part_1.h"

#include <QGraphicsRectItem>
#include <QMessageBox>
#include <limits>

Part_1::Part_1(int x, int y, int width, int height, QObject* parent) :
	QGraphicsScene(x, y, width, height),

	sceneWidth{ width },
	sceneHeight{ height },
	numPointsWide{ 20 },
	numPointsHigh{ 20 },
	squareSize{ 8 },
	centreLineLength{ 20.0 },
	centreX{ 0 },
	centreY{ 0 },
	mode{ CIRCLE },
	verticalMarkerLine{ nullptr },
	horizontalMarkerLine{ nullptr },
	ellipse(nullptr)
{
	// Note that 1.0 is used to coerce double division
	gridSpacingX = sceneWidth  / (numPointsWide + 1.0);
	gridSpacingY = sceneHeight / (numPointsHigh + 1.0);

	nearEllipses.clear();
	farEllipses.clear();

	drawGrid();
}

void Part_1::setMode(Mode mode) {
	this->mode = mode;
}

void Part_1::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	centreX = event->scenePos().x();
	centreY = event->scenePos().y();

	drawCentreMarker(centreX, centreY);
}

void Part_1::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	removeEllipse();

	double currentX = event->scenePos().x();
	double currentY = event->scenePos().y();

	double deltaX = fabs(currentX - centreX);
	double deltaY = fabs(currentY - centreY);

	if (mode == CIRCLE) {
		double radius = sqrt(deltaX * deltaX + deltaY * deltaY);

		ellipse = std::make_unique<QGraphicsEllipseItem>(centreX - radius, centreY - radius, 2.0 * radius, 2.0 * radius);
	} else {
		ellipse = std::make_unique <QGraphicsEllipseItem>(centreX - deltaX, centreY - deltaY, 2.0 * deltaX, 2.0 * deltaY);
	}

	QPen pen;
	pen.setBrush(QBrush(Qt::green));
	ellipse->setPen(pen);

	addItem(ellipse.get());
}

void Part_1::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	// Protect from the case that the mouse click was released before moving
	if (!ellipse) {
		return;
	}

	markSquares();
	drawEllipses();

	removeCentreMarker();
	removeEllipse();
}

void Part_1::clear() {
	// set all squares to gray
	for (auto& square : squares) {
		square->setBrush(QBrush(Qt::gray));
	}

	// remove centre marker and all ellipses
	removeCentreMarker();
	removeEllipse(true);
}

// Draws a rectangle of squares, evenly divided over the scene
void Part_1::drawGrid() {
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

			squares.emplace_back(move(square));
		}
	}
}

void Part_1::drawCentreMarker(double x, double y) {
	removeCentreMarker();
	removeEllipse();

	verticalMarkerLine   = std::make_unique<QGraphicsRectItem>(x - 1, y - centreLineLength / 2, 2, centreLineLength);
	horizontalMarkerLine = std::make_unique<QGraphicsRectItem>(x - centreLineLength / 2, y - 1, centreLineLength, 2);

	verticalMarkerLine->setBrush(QBrush(Qt::green));
	horizontalMarkerLine->setBrush(QBrush(Qt::green));

	addItem(verticalMarkerLine.get());
	addItem(horizontalMarkerLine.get());
}

void Part_1::removeCentreMarker() {
	if (verticalMarkerLine) {
		removeItem(verticalMarkerLine.get());
	}

	if (horizontalMarkerLine) {
		removeItem(horizontalMarkerLine.get());
	}
}

void Part_1::removeEllipse(bool all) {
	if (ellipse) {
		removeItem(ellipse.get());
	}

	if (all) {
		for (auto& ellipse : farEllipses) {
			removeItem(ellipse.get());
		}
		for (auto& ellipse : nearEllipses) {
			removeItem(ellipse.get());
		}
	}
}

// The algorithm to mark the squares on the ellipse is not trivial.
// The circle is treated as an ellipse, so no special case code is required.
// The algorithm works by scanning the grid from left to right, and then top to bottom
// The scan starts with the first column of squares before the ellipse and ends with the first column after the ellipse.
// Each column is the scanned and the squares that are closest to the ellipse are marked
//
// The algorithm is then repeated from top to bottom
void Part_1::markSquares() {
	double leftMostPointOnEllipse  = ellipse->rect().left();
	double rightMostPointOnEllipse = ellipse->rect().right();

	int leftMostColumn  = std::round(leftMostPointOnEllipse  / gridSpacingX);
	int rightMostColumn = std::round(rightMostPointOnEllipse / gridSpacingX);

	// limit to scene
	leftMostColumn  = std::max(leftMostColumn, 0);
	rightMostColumn = std::min(rightMostColumn, numPointsWide);

	// The ellipse is now scanned left to right, one column at a time
	// For each column, find the x coordinate and then the y coordinates on the ellipse
	// These are computed from  the ellipse equation (x^2 / a^2 + y^2 / b^2 = 1)
	//
	//		y = b * sqrt(1 - x^2 / a^2)
	//
	// Note that this also works for circles
	const double a{ (ellipse->rect().right() - ellipse->rect().left()) / 2.0 };
	const double b{ (ellipse->rect().bottom() - ellipse->rect().top()) / 2.0 };
	
	// Marked squares are stored in a set for later use
	markedSquares.clear();

	for (int col = std::max(1, leftMostColumn); col <= rightMostColumn; ++col) {
		double x = col * gridSpacingX - centreX;

		// If x is outside the ellipse then set both y's to the centre Y
		// else compute using the ellipse equation
		double y;
		if (x < - a || x > a) {
			y = 0.0;
		} else {
			y = b * sqrt(1 - x * x / (a * a));
		}

		// Compute Top and Bottom y values in screen coordinates (note that y increases downward)
		double yTop    = centreY - y;
		double yBottom = centreY + y;

		int rowTop  = std::round(yTop / gridSpacingY) - 1;
		int rowBottom = std::round(yBottom / gridSpacingY) - 1;

		// Don't draw outside of scene
		if (rowTop >= 1 && rowTop < numPointsHigh) {
			int indexTop = (col - 1) * numPointsWide + rowTop;
			squares[indexTop]->setBrush(QBrush(Qt::blue));
			markedSquares.insert(squares[indexTop]);
		}

		if (rowBottom >= 1 && rowBottom < numPointsHigh) {
			int indexBottom = (col - 1) * numPointsWide + rowBottom;
			squares[indexBottom]->setBrush(QBrush(Qt::blue));
			markedSquares.insert(squares[indexBottom]);
		}
	}

	// Now repeat from top to bottom
	double topMostPointOnEllipse = ellipse->rect().top();
	double bottomMostPointOnEllipse = ellipse->rect().bottom();

	int topMostRow = std::round(topMostPointOnEllipse / gridSpacingY);
	int bottomMostRow = std::round(bottomMostPointOnEllipse / gridSpacingY);

	topMostRow = std::max(topMostRow, 0);
	bottomMostRow = std::min(bottomMostRow, numPointsHigh);

	for (int row = std::max(1, topMostRow); row <= bottomMostRow; ++row) {
		double y = row * gridSpacingY - centreY;

		// If y is outside the ellipse then set both x's to the centre X
		// else compute using the ellipse equation
		double x;
		if (y < -b || y > b) {
			x = 0.0;
		}
		else {
			x = a * sqrt(1 - y * y / (b * b));
		}

		// Compute Top and Bottom y values in screen coordinates (note that y increases downward)
		double xLeft  = centreX - x;
		double xRight = centreX + x;

		int colLeft = std::round(xLeft / gridSpacingX);
		int colRight = std::round(xRight / gridSpacingX);

		// Don't draw outside of scene
		if (colLeft >= 1 && colLeft <= numPointsWide) {
			int indexLeft = (colLeft - 1) * numPointsWide + row - 1;
			squares[indexLeft]->setBrush(QBrush(Qt::blue));
			markedSquares.insert(squares[indexLeft]);
		}

		if (colRight >= 1 && colRight <= numPointsWide) {
			int indexRight = (colRight - 1) * numPointsWide + row - 1;
			squares[indexRight]->setBrush(QBrush(Qt::blue));
			markedSquares.insert(squares[indexRight]);
		}
	}
}

// Both ellipses are drawn together as the calculations are similar
// The algorithm loops over all marked squares and computes the distance from the centre to each square
// It then draws ellipses to the nearest and farthest marked squares, keeping the ellipse's aspect ratio
void Part_1::drawEllipses() {
	double maxDistance{ 0 };
	double minDistance{ std::numeric_limits<double>::max() };

	std::shared_ptr<QGraphicsRectItem> farthestSquare{ nullptr };
	std::shared_ptr<QGraphicsRectItem> nearestSquare{ nullptr };

	for (auto& square : markedSquares) {
		double dx = centreX - square->rect().center().x();
		double dy = centreY - square->rect().center().y();

		double distanceToCentre = sqrt(dx * dx + dy * dy);

		if (distanceToCentre > maxDistance) {
			maxDistance = distanceToCentre;
			farthestSquare = square;
		}

		if (distanceToCentre < minDistance) {
			minDistance = distanceToCentre;
			nearestSquare = square;
		}
	}

	if (!farthestSquare || !nearestSquare) {
		QMessageBox::critical(0, "Internal error: " + QString(__FILE__) + ":" + QString::number(__LINE__),
			"Couldn't find farthest or nearest square");
		exit(-1);
	}

	// 
	farthestSquare->setBrush(QBrush(Qt::darkBlue));
	nearestSquare->setBrush(QBrush(Qt::darkBlue));

	// Polar coordinates are used to scale new ellipses
	double farX  = farthestSquare->rect().center().x() - centreX;
	double farY  = farthestSquare->rect().center().y() - centreY;
	double nearX = nearestSquare->rect().center().x()  - centreX;
	double nearY = nearestSquare->rect().center().y()  - centreY;

	double farthestCosTheta = farX  / maxDistance;
	double farthestSinTheta = farY  / maxDistance;
	double nearestCosTheta  = nearX / minDistance;
	double nearestSinTheta  = nearY / minDistance;
	
	// Compute parameters of the actual ellipse near the 2 squares
	const double actualA{ (ellipse->rect().right()  - ellipse->rect().left()) / 2.0 };
	const double actualB{ (ellipse->rect().bottom() - ellipse->rect().top())  / 2.0 };

	double actualEllipseRadiusFarSquare =
		sqrt(1.0 / (((farthestCosTheta * farthestCosTheta) / (actualA * actualA)) + ((farthestSinTheta * farthestSinTheta) / (actualB * actualB))));
	
	double actualEllipseRadiusNearSquare =
		sqrt(1.0 / (((nearestCosTheta * nearestCosTheta) / (actualA * actualA)) + ((nearestSinTheta * nearestSinTheta) / (actualB * actualB))));

	// Compute scaling factors
	double scaleFar  = sqrt(farX  * farX  + farY  * farY ) / actualEllipseRadiusFarSquare;
	double scaleNear = sqrt(nearX * nearX + nearY * nearY) / actualEllipseRadiusNearSquare;

	double farA  = actualA * scaleFar;
	double farB  = actualB * scaleFar;
	double nearA = actualA * scaleNear;
	double nearB = actualB * scaleNear;

	std::unique_ptr<QGraphicsEllipseItem> farEllipse  = std::make_unique<QGraphicsEllipseItem>(centreX - farA,  centreY - farB,  2.0 * farA,  2.0 * farB );
	std::unique_ptr<QGraphicsEllipseItem> nearEllipse = std::make_unique<QGraphicsEllipseItem>(centreX - nearA, centreY - nearB, 2.0 * nearA, 2.0 * nearB);
	
	QPen pen;
	pen.setBrush(QBrush(Qt::red));

	farEllipse->setPen(pen);
	nearEllipse->setPen(pen);

	addItem(farEllipse.get());
	addItem(nearEllipse.get());

	farEllipses.emplace_back(move(farEllipse));
	nearEllipses.emplace_back(move(nearEllipse));
}
