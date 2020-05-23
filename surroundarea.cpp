/* (c) 2020 ukrkyi */
#include "surroundarea.h"

#include "qmath.h"
#include <QPainter>

SurroundArea::SurroundArea(QWidget *parent) :
	QWidget(parent), pos(0, 0), angle(0)
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

void SurroundArea::paintEvent(QPaintEvent *event)
{
	const int points[] = {
		0, (int)(-0.2 * dimension),
		(int)(0.55  * dimension), (int)(1.6 * dimension),
		(int)(-0.55 * dimension), (int)(1.6 * dimension),
	};
	const QPolygon car(3, points);
	QPoint location = translate(pos);
	QPolygon triangle = QTransform().translate(location.x(), location.y()).rotate(angle).map(car);

	QPainter painter(this);

//	painter.setBrush(Qt::NoBrush);
//	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
	painter.setPen(QPen(QBrush(Qt::blue),1));
	painter.setBrush(QBrush(Qt::blue));
	painter.drawPolygon(triangle);
}

QSize SurroundArea::sizeHint() const
{
	return QSize(dimension * 60, dimension * 100);
}

QSize SurroundArea::minimumSizeHint() const
{
	return QSize(dimension * 60, dimension * 100);
}


QPoint SurroundArea::translate(QPoint point)
{
	return getO() + QPoint(point.y(), -point.x());
}

QPoint SurroundArea::getO()
{
	return QPoint(this->width() / 2, this->height() - 1);
}

void SurroundArea::updatePosition(float x, float y, float alpha)
{
	pos = QPoint(x * dimension / 100, y * dimension / 100);
	angle = alpha;
}
