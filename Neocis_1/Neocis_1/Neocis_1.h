#ifndef __NEOCIS_1_H__
#define __NEOCIS_1_H__

#include <QGraphicsRectItem>
#include <QtWidgets/QMainWindow>
#include "ui_Neocis_1.h"
#include "Part_1.h"
#include "Part_2.h"

class Neocis_1 : public QMainWindow {
	Q_OBJECT

public:
	Neocis_1(QWidget* parent = Q_NULLPTR);

private:
	int sceneWidth;
	int sceneHeight;

	Ui::Neocis_1Class ui;

	std::unique_ptr<Part_1> part_1;
	std::unique_ptr<Part_2> part_2;

	// These can be changed, but remember to change the size of the canvas in Neocis_1.ui
	const int SCENE_WIDTH { 840 };
	const int SCENE_HEIGHT{ 840 };

private slots:
	void on_pushButtonOnlineHelp_clicked();

	void on_radioButtonCircle_clicked();
	void on_radioButtonEllipse_clicked();

	void on_pushButtonClear_clicked();

	void on_checkBoxPart2_clicked();
	void on_pushButtonGenerate_clicked();

	void on_pushButtonClose_clicked();
};

#endif
