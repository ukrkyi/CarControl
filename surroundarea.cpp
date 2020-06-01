/* (c) 2020 ukrkyi */
#include "surroundarea.h"

#include "qmath.h"
#include <QPainter>

#include "mainwindow.h"
#include <QMouseEvent>
#include <QToolTip>

SurroundArea::SurroundArea(QWidget *parent) :
	QWidget(parent)
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	setMouseTracking(true);
}

void SurroundArea::paintEvent(QPaintEvent *event)
{
	const int points[] = {
		0, 0,
		(int)(0.55  * dimension), (int)(1.8 * dimension),
		(int)(-0.55 * dimension), (int)(1.8 * dimension),
	};
	const QPolygon car(3, points);
	// paint grid
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	for (unsigned y = 0; y < proc.height; y++)
		for (unsigned x = 0; x < proc.width; x++) {
			DataProcessor::Cell cell = proc.grid[y][x];
			DataProcessor::Rectangle rect = proc.getRealCoordinates(x, y);
			QColor color = QColor(250 - cell, 250 - cell, 250 - cell);
			QRect paintZone = QRect(translate(rect.lowLeft), translate(rect.upRight));
			painter.setBrush(color);
			painter.drawRect(paintZone);
		}

	// paint all car history
	painter.setPen(QPen(QBrush(Qt::blue),1));
	painter.setBrush(QBrush(Qt::blue));
	for (const auto& point: history) {
		QPoint location = translate(point);
		QPolygon triangle = QTransform().
				translate(location.x(), location.y()).
				rotateRadians(point.angle).
				map(car);

		painter.drawPolygon(triangle);
	}
}

QSize SurroundArea::sizeHint() const
{
	return QSize(dimension * 60, dimension * 100);
}

QSize SurroundArea::minimumSizeHint() const
{
	return QSize(dimension * 60, dimension * 100);
}

QPoint SurroundArea::translate(const Position& pos)
{
	DataProcessor::Point pt = proc.translate(pos.x, pos.y, pos.angle);
	return translate(pt);
}

QPoint SurroundArea::translate(const DataProcessor::Point &pt)
{
	return getO() + QPoint(pt.x * dimension / 100, -pt.y * dimension / 100);
}

QPoint SurroundArea::getO()
{
	return QPoint(this->width() / 2, this->height() - 1);
}

bool SurroundArea::updatePosition(float x, float y, float alpha)
{
	Position newPos = {x, y, alpha};
	Position * oldPos = history.empty() ? NULL : &history.back();
	if (oldPos == NULL || newPos != *oldPos) {
		history.push_back(newPos);
		this->update();
		return true;
	} else {
		return false;
	}
}

void SurroundArea::newRange(float distance)
{
	if (distance > min_distance && !history.empty()) {
		if (distance <= 3000)
			proc.processData(history.back(), distance, true);
		else
			proc.processData(history.back(), 3000, false); // do this to clear path if no obstacle is detected

		update();
	}
}

void SurroundArea::clear()
{
	history.clear();
	proc.clear();
}


void SurroundArea::mouseMoveEvent(QMouseEvent *event)
{
	unsigned x = proc.getGridX(float(event->x() - getO().x())*100/dimension),
			y = proc.getGridY(float(getO().y() - event->y())*100/dimension);
	QToolTip::showText(event->globalPos(), QString::number(proc.grid[y][x].getOccupancy()), this);
}
