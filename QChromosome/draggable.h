#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include <QWidget>

class Draggable : public QWidget
{
    Q_OBJECT

public:
    explicit Draggable(QWidget *parent = 0);
    virtual ~Draggable();

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

    void paintEvent(QPaintEvent*);

    static Qt::MouseButton pressedButton();

private:
    QPoint initial;
    QPoint center;

    static Qt::MouseButton pressed;

signals:
    void delta(int, int);

public slots:
    void updateScreenGeometry(QRect);
};

#endif // DRAGGABLE_H
