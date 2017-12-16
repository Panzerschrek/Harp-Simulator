#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSlider>
#include <QtCore/QTimer>

#include "sound_out.h"

class HarpWindow : public QMainWindow
{
	Q_OBJECT
public:
	HarpWindow();
	~HarpWindow();

	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

private slots:
	void onTimer();

private:
	QTimer timer_;
	QSlider slider_;
	SoundOut sound_out_;
};
