#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>
#include <QHeaderView>
#include "treemodel.h"
#include "material.h"
#include <functional>

class VizWidget;

enum VisibilityMode
{
    Editor = 3,
    Renderer = 4
};

class HeaderView;

class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = 0);
    ~TreeView();

    void setSelection(const QList<unsigned int>& indexes);
    void setScene(VizWidget* s);

    QVariant getName(const QList<unsigned int>& indexes) const;
    void setName(const QList<unsigned int>& indexes, const QString& name);

    Visibility getVisibility(const QList<unsigned int>& indexes, VisibilityMode m) const;
    void setVisibility(const QList<unsigned int>& indexes, Visibility v, VisibilityMode m);

    Material* getMaterial(const QList<unsigned int>& indexes) const;
    void setMaterial(const QList<unsigned int>& indexes, Material* m);

signals:
    void visibilityChanged(VisibilityMode);

public slots:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    bool event(QEvent *event);

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Visibility cv;
    VisibilityMode vm;

    VizWidget* scene;

    void dumpModel(const QModelIndex& root, QList<unsigned int>& id, std::function<bool(const QModelIndex&)> functor) const;

    Visibility getVisibility(const QModelIndex& root, VisibilityMode m) const;
    void setVisibility(const QModelIndex& root, Visibility v, VisibilityMode m);

    Material* getMaterial(const QModelIndex& root) const;
    void setMaterial(const QModelIndex& root, Material* m);

    Material* takeSelectedMaterial();

    enum TreeViewState
    {
        NoState,
        ResizeSection,
        ChangeVisibility,
        DragTag
    } state;

    HeaderView *hv;

    QModelIndex selectedTag;

    bool clicked = false;
};

class HeaderView : public QHeaderView
{
    Q_OBJECT
    friend class TreeView;
public:
    explicit HeaderView(Qt::Orientation orientation, QWidget *parent = 0);
};

#endif // TREEVIEW_H
