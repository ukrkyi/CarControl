/* (c) 2020 ukrkyi */
#ifndef SURROUNDAREA_H
#define SURROUNDAREA_H

#include <QWidget>

class SurroundArea : public QWidget
{
	Q_OBJECT

	static const int dimension = 7;

	QPoint pos;
	float angle;

	QPoint translate(QPoint point);
	QPoint getO();

protected:
	void paintEvent(QPaintEvent *event) override;

public:
	explicit SurroundArea(QWidget *parent = nullptr);
	QSize sizeHint() const override;
	QSize minimumSizeHint() const override;

public slots:
	void updatePosition(float x, float y, float alpha);
};

#endif // SURROUNDAREA_H
