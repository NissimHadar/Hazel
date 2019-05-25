#include "Neocis_1.h"

#include <QDesktopServices>
#include <QUrl>

Neocis_1::Neocis_1(QWidget* parent) : 
	QMainWindow{ parent }
{
	// This should always be called first
	ui.setupUi(this);

	installEventFilter(this);
	
	// Set up size of the scene
	sceneWidth  = SCENE_WIDTH;
	sceneHeight = SCENE_HEIGHT;

	// Disable resizing of window
	setFixedSize(geometry().width(), geometry().height());

	// Set screens to size of view
	part_1 = std::make_unique<Part_1>(0, 0, sceneWidth, sceneHeight);
	part_2 = std::make_unique<Part_2>(0, 0, sceneWidth, sceneHeight);

	// Select and show part1
	ui.graphicsView->setScene(part_1.get());
	ui.graphicsView->show();
}

void Neocis_1::on_pushButtonOnlineHelp_clicked() {
	QDesktopServices::openUrl(QUrl("https://github.com/highfidelity/hifi/blob/master/tools/nitpick/README.md"));
}

void Neocis_1::on_radioButtonCircle_clicked() {
	part_1->setMode(CIRCLE);
}

void Neocis_1::on_radioButtonEllipse_clicked() {
	part_1->setMode(ELLIPSE);
}

void Neocis_1::on_pushButtonClear_clicked() {
	part_1->clear();
}

// Part2
// This checkbox is used to select the "Part 2 program"
void Neocis_1::on_checkBoxPart2_clicked() {
	if (ui.checkBoxPart2->isChecked()) {
		ui.radioButtonCircle->setEnabled(false);
		ui.radioButtonEllipse->setEnabled(false);
		ui.pushButtonClear->setEnabled(false);

		ui.pushButtonGenerate->setEnabled(true);
		ui.graphicsView->setScene(part_2.get());
	} else {
		ui.radioButtonCircle->setEnabled(true);
		ui.radioButtonEllipse->setEnabled(true);
		ui.pushButtonClear->setEnabled(true);

		ui.pushButtonGenerate->setEnabled(false);
		ui.graphicsView->setScene(part_1.get());
	}
}

// The generate button is also used to clear the points and circle
void Neocis_1::on_pushButtonGenerate_clicked() {
	static bool readyToGenerate{ true };
	if (readyToGenerate) {
		ui.pushButtonGenerate->setText("Clear");
		part_2->generate();
	} else {
		ui.pushButtonGenerate->setText("Generate");
		part_2->clear();
	}

	readyToGenerate = !readyToGenerate;
}

// Exit when closed
void Neocis_1::on_pushButtonClose_clicked() {
	exit(0);
}
