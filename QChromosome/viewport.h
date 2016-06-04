#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#include "../QtChromosomeViz_v2/VizWidget.hpp"
#include "blind.h"
#include "axis.h"

namespace Ui {
    class Viewport;
}

class Viewport : public QWidget
{
    Q_OBJECT
public:
    explicit Viewport(QWidget *parent = 0);
    ~Viewport();

    void setVizWidget(VizWidget* vizWidget);
    void setBlind(Blind *blind);
    void setAxis(Axis *axis);

signals:

public slots:

private:
    Ui::Viewport *ui;

    VizWidget* vizWidget_;
    Blind *blind_;
    Axis *axis_;
};

#endif // VIEWPORT_H
