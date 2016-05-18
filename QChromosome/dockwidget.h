#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>

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

    static QAction* noneClosedAction();

private:
    QAction *recclo;

    static QAction* none;
    static int n;

signals:

public slots:

private:
    Ui::DockWidget *ui;
};

#endif // DOCKWIDGET_H
