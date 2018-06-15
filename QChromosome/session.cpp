#include "session.h"

Session::Session() :
    action(new QAction),
    simulation(new Simulation(this)),
    editorCamera(new Camera(this)),
    treeView(new TreeView),
    nd(new NameDelegate),
    vd(new VisibilityDelegate),
    td(new TagsDelegate)
{
    treeView->setMouseTracking(true);
    treeView->setFocusPolicy(Qt::NoFocus);
    treeView->setAcceptDrops(true);
    treeView->setAlternatingRowColors(true);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setUniformRowHeights(true);
    treeView->setHeaderHidden(true);

    treeView->setModel(simulation->getModel());

    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(4);
    treeView->hideColumn(6);

    treeView->setItemDelegateForColumn(0, nd);
    treeView->setItemDelegateForColumn(3, vd);
    treeView->setItemDelegateForColumn(5, td);

    auto header = treeView->header();
    header->resizeSection(3, 42);
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    header->setSectionResizeMode(5, QHeaderView::Fixed);
}

Session::~Session()
{
    delete action;
    delete simulation;
    delete nd;
    delete vd;
    delete td;
}

#include <cassert>

void Session::setFrame(std::shared_ptr<Frame> frame)
{
    assert(frame->atoms.size() == atomBuffer.size());

    auto& atoms = frame->atoms;

    for (int i = 0; i < atoms.size(); i++)
    {
        auto& atom = atoms[i];
        atomBuffer[i].position = QVector3D(atom.x, atom.y, atom.z);
    }
}
