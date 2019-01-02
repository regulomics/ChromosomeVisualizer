#include "treeitem.h"
#include "treemodel.h"

#include <QStringList>

TreeModel::TreeModel(Session *s, QObject *parent)
    : QAbstractItemModel(parent),
      header(new TreeItem({"Name", "Type", "Id", "ViE", "ViR", "Tags", "Camera"})),
      session(s)
{

}

TreeModel::~TreeModel()
{
    delete header;
}

int TreeModel::columnCount(const QModelIndex &) const
{
    return 7;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = reinterpret_cast<TreeItem*>(index.internalPointer());

    switch (role)
    {
    case Qt::DisplayRole:
        return item->data(index.column());
    case Qt::DecorationRole:
        return item->decoration;
    case Qt::UserRole:
        return item->selected_children_count;
    case Qt::UserRole + 1:
        return item->selected_tag_index;
    }

    return QVariant();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    bool ans;

    switch (role)
    {
    case Qt::DecorationRole:
        item->decoration = value;
        ans = true;
        break;
    case Qt::UserRole + 1:
        item->selected_tag_index = value.toInt();
        if (item->selected_tag_index == -1)
            emit tagSelected({});
        else
            emit tagSelected({index.sibling(index.row(), 5).data().toList().at(item->selected_tag_index).value<Material*>()});
        ans = true;
        break;
    default:
        ans = item->setData(index.column(), value);
    }

    if (ans)
        emit dataChanged(index, index, {role});

    return ans;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | (index.column() == 0 ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = header;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == header)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    return (parent.isValid() ? static_cast<TreeItem*>(parent.internalPointer()) : header)->childCount();
}

void TreeModel::dumpModel1(const QModelIndex& root, QVector<QPersistentModelIndex>& id)
{
    // update index buffer
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
        id[root.sibling(root.row(), 2).data().toUInt()] = root;

    for (int r = 0; r < rowCount(root); r++)
        dumpModel1(index(r, 0, root), id);
}

void TreeModel::colorByResidue(const QModelIndex& root)
{
    dumpModel2(root, Material::getDefault());
}

#include "session.h"
#include "preferences.h"

void TreeModel::dumpModel2(const QModelIndex& root, Material* m)
{
    // update current tags
    if (sibling(root.row(), 1, root).data().toInt() == ResidueObject)
    {
        auto resName = sibling(root.row(), 2, root).data().toInt();
        auto mat = Preferences::getInstance()->materialAt(resName);

        m = qobject_cast<Material*>(mat.value<QObject*>());
        m->assign(sibling(root.row(), 5, root));

        if (!materials.contains(m))
        {
            qobject_cast<MaterialListModel*>(session->listView->model())->prepend(m);
            materials.insert(m);
        }

        setData(sibling(root.row(), 5, root), QVariantList({mat}));
    }
    else
        setData(sibling(root.row(), 5, root), QVariant());

    // update current material
    if (sibling(root.row(), 1, root).data().toInt() == AtomObject)
        reinterpret_cast<AtomItem*>(root.internalPointer())->setMaterial(m);

    for (int r = 0; r < rowCount(root); r++)
        dumpModel2(index(r, 0, root), m);
}

void TreeModel::colorByChain(const QModelIndex& root)
{
    dumpModel3(root, Material::getDefault());
}

void TreeModel::dumpModel3(const QModelIndex& root, Material* m)
{
    // update current tags
    if (sibling(root.row(), 1, root).data().toInt() == ChainObject)
    {
        int chainID = sibling(root.row(), 2, root).data().toInt();
        auto mat = Preferences::getInstance()->materialAt(chainID);

        m = qobject_cast<Material*>(mat.value<QObject*>());
        m->assign(sibling(root.row(), 5, root));

        if (!materials.contains(m))
        {
            qobject_cast<MaterialListModel*>(session->listView->model())->prepend(m);
            materials.insert(m);
        }

        setData(sibling(root.row(), 5, root), QVariantList({mat}));
    }
    else
        setData(sibling(root.row(), 5, root), QVariant());

    // update current material
    if (sibling(root.row(), 1, root).data().toInt() == AtomObject)
        reinterpret_cast<AtomItem*>(root.internalPointer())->setMaterial(m);

    for (int r = 0; r < rowCount(root); r++)
        dumpModel3(index(r, 0, root), m);
}

bool TreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    for (int i = 0; i < count; i++)
    {
        switch (index(row, 1, parent).data().toInt())
        {
        case CameraObject:
        case LayerObject:
            beginRemoveRows(parent, row, row);
            (parent.isValid() ? reinterpret_cast<TreeItem*>(parent.internalPointer()) : header)->removeRows(row, 1);
            endRemoveRows();
            break;
        default:
            row++;
        }
    }

    return true;
}

#include <QRegularExpression>
#include <set>

QString TreeModel::next_name() const
{
    static const QRegularExpression re("^Camera(.(?<label>[1-9][0-9]*))?$");

    std::set<int> used;

    for (int r = 0; r < rowCount(); r++)
    {
        QRegularExpressionMatch match = re.match(index(r, 0).data().toString());

        if (match.hasMatch())
            used.insert(match.captured("label").toInt());
    }

    int i = 0;

    for (auto j = used.begin(); j != used.end() && i == *j; i++, j++);

    return i ? QString("Camera.") + QString::number(i) : "Camera";
}

void TreeModel::addCamera(Camera *camera)
{
    TreeItem* root = new CameraItem(next_name(), camera, session);

    beginInsertRows(QModelIndex(), 0, 0);
    header->prependChild(root);
    endInsertRows();

    session->userCameras.append(camera);
    cameras[camera] = index(0, 6);
}

void TreeModel::changeCamera(Camera *camera)
{
    setCurrentCamera(cameras[camera]);
}

void TreeModel::setCurrentCamera(QModelIndex index)
{
    if (currentCamera == index)
        return;

    if (currentCamera.isValid())
        setData(currentCamera, false);

    currentCamera = index;

    if (currentCamera.isValid())
    {
        setData(currentCamera, true);
        emit cameraChanged(reinterpret_cast<CameraItem*>(currentCamera.internalPointer())->getCamera());
    }
    else
        emit cameraChanged(nullptr);
}

void TreeModel::setMaterial(const QModelIndex &root, Material *m, int position)
{
    auto index = root.sibling(root.row(), 5);

    auto list = index.data().toList();
    list.insert(position, QVariant::fromValue(m));

    setData(index, list);

    if (position >= list.length() - 1)
        reinterpret_cast<TreeItem*>(root.internalPointer())->setMaterial(m);

    m->assign(index);

    emit propertyChanged();
}

Material *TreeModel::removeMaterial(const QModelIndex &root, int position)
{
    auto index = root.sibling(root.row(), 5);

    auto list = index.data().toList();
    auto m = list.takeAt(position).value<Material*>();

    setData(index, list);

    if (position == list.length())
    {
        auto index = root.sibling(root.row(), 0);
        auto m = Material::getDefault();

        while (index.isValid())
        {
            auto list = index.sibling(index.row(), 5).data().toList();

            if (!list.isEmpty())
            {
                m = list.last().value<Material*>();
                break;
            }

            index = index.parent();
        }

        reinterpret_cast<TreeItem*>(root.internalPointer())->setMaterial(m);
    }

    m->assign(index, false);

    emit propertyChanged();

    return m;
}

void TreeModel::propagateVisibility(const QModelIndex &root, VisibilityMode m, bool v)
{
    reinterpret_cast<TreeItem*>(root.internalPointer())->setFlag(m == Editor ? VisibleInEditor : VisibleInRenderer, v);

    for (int r = 0; r < rowCount(root); r++)
    {
        auto c = index(r, 0, root);

        if (c.sibling(c.row(), m).data().toInt() == Default)
            propagateVisibility(c, m, v);
    }
}

void TreeModel::setVisibility(const QModelIndex &index, Visibility v, VisibilityMode m)
{
    setVisibility(QModelIndexList({index}), v, m);
}

void TreeModel::setVisibility(const QModelIndexList &indices, Visibility v, VisibilityMode m)
{
    for (const auto& i : indices)
    {
        auto index = i.sibling(i.row(), m);

        setData(index, v);

        index = i.sibling(i.row(), 0);
        auto v = On;

        while (index.isValid())
        {
            Visibility w = (Visibility)index.sibling(index.row(), m).data().toInt();

            if (w != Default)
            {
                v = w;
                break;
            }

            index = index.parent();
        }

        propagateVisibility(i.sibling(i.row(), 0), m, v == On);
    }

    emit propertyChanged();
    emit attributeChanged();
}

void TreeModel::setName(const QModelIndex &index, const QString &name)
{
    setName(QModelIndexList({index}), name);
}

void TreeModel::setName(const QModelIndexList &indices, const QString &name)
{
    if (!name.isEmpty())
        for (const auto& i : indices)
            setData(i.sibling(i.row(), 0), name);

    emit attributeChanged();
}

#include <QJsonObject>
#include "simulation/simulationlayer.h"

void TreeModel::read(const QJsonObject &json)
{
    const QJsonObject children = json["Descendants"].toObject();

    for (auto child = children.end() - 1; child != children.begin() - 1; child--)
    {
        const QJsonObject object = child.value().toObject()["Object"].toObject();

        if (object["class"] == "Layer")
            session->simulation->prepend(SimulationLayer::read(object, session));

        if (object["class"] == "Camera")
            addCamera(new Camera(session));
    }

    header->read(json, reinterpret_cast<MaterialListModel*>(session->listView->model()));
}

void TreeModel::write(QJsonObject &json) const
{
    header->write(json);
}

void TreeModel::prepend(TreeItem* item)
{
    beginInsertRows(QModelIndex(), 0, 0);
    header->prependChild(item);
    endInsertRows();

    session->indices.resize(session->atomBuffer.size());
    dumpModel1(index(0, 0), session->indices);
}