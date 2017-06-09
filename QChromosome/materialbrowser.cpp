#include "materialbrowser.h"
#include "ui_materialbrowser.h"

MaterialBrowser::MaterialBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MaterialBrowser),
    s(new QSplitter),
    lv(new ListView),
    w(new QWidget)
{
    ui->setupUi(this);

    lv->setFocusPolicy(Qt::ClickFocus);
    lv->setStyleSheet("QLineEdit {\
                      color: white;\
                      border: none;\
                      padding: 1px;\
                      background: #262626;\
                  }");

    lv->setItemDelegate(new MaterialDelegate(this));

    s->addWidget(w);
    s->addWidget(lv);

    layout()->addWidget(s);

    MaterialListModel * m = new MaterialListModel;
    m->insertRows(0,4);
    m->setData(m->index(0), QVariant::fromValue(new Material("", Qt::red)), Qt::DecorationRole);
    m->setData(m->index(1), QVariant::fromValue(new Material("", Qt::green)), Qt::DecorationRole);
    m->setData(m->index(2), QVariant::fromValue(new Material("", Qt::blue)), Qt::DecorationRole);
    m->setData(m->index(3), QVariant::fromValue(new Material("", Qt::white, .5)), Qt::DecorationRole);

    lv->setModel(m);

    connect(lv->selectionModel(), &QItemSelectionModel::currentChanged, [this](const QModelIndex& index, const QModelIndex&) {
        if (index.isValid())
            emit materialsSelected({index.data(Qt::DecorationRole).value<Material*>()});
        update();
    });

    lv->setCurrentIndex(m->index(0));
}

MaterialBrowser::~MaterialBrowser()
{
    delete ui;
}

#include <QPainter>

void MaterialBrowser::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);

    if (lv->selectionModel()->hasSelection())
        lv->selectionModel()->currentIndex().data(Qt::DecorationRole).value<Material*>()->paint(&p, w->geometry().translated(s->pos()).marginsRemoved(QMargins(10, 10, 10, 10)));
}

ListView::ListView(QWidget *parent) : QListView(parent)
{

}

void ListView::mousePressEvent(QMouseEvent *event)
{
    clicked = true;

    QListView::mousePressEvent(event);
}

#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>

void ListView::mouseMoveEvent(QMouseEvent *event)
{
    if (clicked)
    {
        clicked = false;

        auto index = indexAt(event->pos());

        if (index.isValid())
        {
            QDrag *drag = new QDrag(index.data(Qt::DecorationRole).value<Material*>());
            drag->setMimeData(new QMimeData);
            drag->exec(Qt::CopyAction | Qt::MoveAction);
        }
    }
    else
        QListView::mouseMoveEvent(event);
}

void ListView::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);

    if (event->key() == Qt::Key_Delete)
        model()->removeRow(currentIndex().row());
}

int MaterialListModel::rowCount(const QModelIndex &parent) const
{
    return materials.count();
}

QVariant MaterialListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= materials.size())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return materials.at(index.row())->getName();
    case Qt::DecorationRole:
        return QVariant::fromValue(materials.at(index.row()));
    default:
        return QVariant();
    }
}

QVariant MaterialListModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

Qt::ItemFlags MaterialListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool MaterialListModel::setData(const QModelIndex &index,
                              const QVariant &value, int role)
{
    if (index.isValid())
    {
        switch (role)
        {
        case Qt::EditRole:
            materials.at(index.row())->setName(value.toString());
            emit dataChanged(index, index);
            return true;
        case Qt::DecorationRole:
            materials.replace(index.row(), value.value<Material*>());
            emit dataChanged(index, index);
            return true;
        default:
            return false;
        }
    }
}

bool MaterialListModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row)
        materials.insert(position, 0);

    endInsertRows();
    return true;
}

bool MaterialListModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row)
        materials.removeAt(position);

    endRemoveRows();
    return true;
}

MaterialDelegate::MaterialDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

#include <QLineEdit>

void MaterialDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qobject_cast<QLineEdit*>(editor)->setText(index.data().toString());
    editor->setFocus();
}

void MaterialDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QString name = qobject_cast<QLineEdit*>(editor)->text();

    if (!name.isEmpty())
        model->setData(index, name);
}

void MaterialDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->save();
    index.data(Qt::DecorationRole).value<Material*>()->paint(painter, QRect(option.rect.topLeft() + QPoint(10, 1), QSize(30, 30)));
    painter->restore();
}
