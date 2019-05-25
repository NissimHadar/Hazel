#ifndef __PART_1_H__
#define __PART_1_H__

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <vector>
#include <set>
enum Mode {
	CIRCLE,
	ELLIPSE
};
class Part_1 : public QGraphicsScene {
public:
	Part_1(int x, int y, int width, int height, QObject* parent = nullptr);

	void setMode(Mode mode);

	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

	void clear();
	void drawGrid();
	void drawCentreMarker(double x, double y);

	void removeCentreMarker();
	void removeEllipse(bool all = false);
	void markSquares();
	void drawEllipses();

private:
	int sceneWidth;
	int sceneHeight;

	int numPointsWide;
	int numPointsHigh;

	int squareSize;

	double centreLineLength;

	double centreX;
	double centreY;

	Mode mode;

	std::vector<std::shared_ptr<QGraphicsRectItem>> squares;
	std::set<std::shared_ptr<QGraphicsRectItem>> markedSquares;

	std::unique_ptr<QGraphicsRectItem> verticalMarkerLine;
	std::unique_ptr<QGraphicsRectItem> horizontalMarkerLine;
	
	std::unique_ptr<QGraphicsEllipseItem> ellipse;
	
	std::vector<std::unique_ptr<QGraphicsEllipseItem>> nearEllipses;
	std::vector<std::unique_ptr<QGraphicsEllipseItem>> farEllipses;

	double gridSpacingX;
	double gridSpacingY;
};

#endif
