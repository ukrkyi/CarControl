/* (c) 2020 ukrkyi */
#ifndef SURROUNDAREA_H
#define SURROUNDAREA_H

#include <QWidget>
#include "dataprocessor.h"

class SurroundArea : public QWidget
{
	Q_OBJECT

	static const int dimension = 7;

	QVector<Position> history;
	DataProcessor proc;

	QPoint translate(const Position &pos);
	QPoint translate(const DataProcessor::Point &pt);
	QPoint getO();

protected:
	void paintEvent(QPaintEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

public:
	explicit SurroundArea(QWidget *parent = nullptr);
	QSize sizeHint() const override;
	QSize minimumSizeHint() const override;

	bool updatePosition(float x, float y, float alpha);
	void newRange(float distance);
	void clear();

};

#endif // SURROUNDAREA_H
