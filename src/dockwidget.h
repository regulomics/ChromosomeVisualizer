#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>
#include <QAction>

namespace Ui
{
    class DockWidget;
}

class DockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DockWidget(QWidget *parent = 0);
    ~DockWidget();

    QAction* recentlyClosedAction();

private:
    QAction *recclo;

signals:

public slots:

private:
    Ui::DockWidget *ui;
};

#endif // DOCKWIDGET_H
