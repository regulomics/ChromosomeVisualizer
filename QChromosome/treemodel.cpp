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

int TreeModel::columnCount(const QModelIndex &parent) const
{
    return 7;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

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
    case Qt::UserRole:
        item->selected_children_count = value.toInt();
        ans = true;
        break;
    case Qt::UserRole + 1:
        item->selected_tag_index = value.toInt();
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

void appendSubmodel(std::pair<int, int> range, const std::vector<Atom>& atoms, unsigned int n, unsigned int offset, TreeItem *parent, Session *s)
{
    auto root = new ChainItem(QString("Chain") + (n ? QString(".") + QString::number(n) : ""), {range.first + offset, range.second + offset}, s, parent);

    QMap<int, TreeItem*> types;

    for (auto atom = atoms.begin() + range.first; atom != atoms.begin() + range.second; atom++)
    {
        int t = atom->type;

        if (!types.contains(t))
            types[t] = new ResidueItem(t, root);

        new AtomItem(*atom, atom->id - 1 + offset, s, types[t]);
    }
}

#include <QBitArray>
#include <QFileInfo>
#include "session.h"

void TreeModel::setupModelData(std::shared_ptr<SimulationLayerConcatenation> slc)
{
    unsigned int offset = session->atomBuffer.size();
    auto f = slc->getFrame(0);
    auto& atoms = f->atoms;
    auto& connectedRanges = f->connectedRanges;

    QBitArray used(atoms.size(), false);

    auto root = new LayerItem(QFileInfo(QString::fromStdString(slc->getSimulationLayerConcatenationName())).fileName(), slc, session);

    session->atomBuffer.resize(atoms.size());

    unsigned int i = 0;

    for (auto range : connectedRanges)
    {
        range.first--;
        used.fill(true, range.first, range.second);
        appendSubmodel(range, atoms, i++, offset, root, session);
    }

    QMap<int, TreeItem*> types;

    for (auto i = 0; i < atoms.size(); i++)
        if (!used.testBit(i))
        {
            int t = atoms[i].type;

            if (!types.contains(t))
                types[t] = new ResidueItem(t, root);

            new AtomItem(atoms[i], atoms[i].id - 1 + offset, session, types[t]);
        }

    beginInsertRows(QModelIndex(), 0, 0);
    header->prependChild(root);
    endInsertRows();

    session->indices.resize(offset + atoms.size());
    dumpModel1(index(0, 0), session->indices);
}

void TreeModel::colorByResidue(const QModelIndex& root)
{
    dumpModel2(root, Material::getDefault());
}

#include "defaults.h"

void TreeModel::dumpModel2(const QModelIndex& root, Material* m)
{
    // update current tags
    if (root.sibling(root.row(), 1).data().toInt() == ResidueObject)
    {
        int type = root.sibling(root.row(), 2).data().toInt();
        auto defl = Defaults::typename2color(type).toList();

        for (auto mat : defl)
        {
            m = qobject_cast<Material*>(mat.value<QObject*>());
            m->assign(root.sibling(root.row(), 5));

            if (!materials.contains(m))
            {
                qobject_cast<MaterialListModel*>(session->listView->model())->prepend(m);
                materials.insert(m);
            }
        }

        auto list = root.sibling(root.row(), 5).data().toList();
        setData(root.sibling(root.row(), 5), list + defl);
    }

    // update current material
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
        reinterpret_cast<AtomItem*>(root.internalPointer())->setMaterial(m);

    for (int r = 0; r < rowCount(root); r++)
        dumpModel2(index(r, 0, root), m);
}

void TreeModel::colorByChain(const QModelIndex& root)
{
    dumpModel3(root, Material::getDefault());
}

#include "defaults1.h"

void TreeModel::dumpModel3(const QModelIndex& root, Material* m)
{
    // update current tags
    if (root.sibling(root.row(), 1).data().toInt() == ChainObject)
    {
        int num = root.sibling(root.row(), 2).data().toInt();
        auto defl = Defaults1::chainnumber2color(num).toList();

        for (auto mat : defl)
        {
            m = qobject_cast<Material*>(mat.value<QObject*>());
            m->assign(root.sibling(root.row(), 5));

            if (!materials.contains(m))
            {
                qobject_cast<MaterialListModel*>(session->listView->model())->prepend(m);
                materials.insert(m);
            }
        }

        auto list = root.sibling(root.row(), 5).data().toList();
        setData(root.sibling(root.row(), 5), list + defl);
    }

    // update current material
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
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
    TreeItem* root = new CameraItem(next_name(), camera, session, header);

    beginInsertRows(QModelIndex(), 0, 0);
    header->prependChild(root);
    endInsertRows();

    session->userCameras.append(camera);
}

void TreeModel::setCurrentCamera(QModelIndex index)
{
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

void TreeModel::propagateMaterial(const QModelIndex &root, const Material* m)
{
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
        reinterpret_cast<AtomItem*>(root.internalPointer())->setMaterial(m);

    for (int r = 0; r < rowCount(root); r++)
    {
        auto c = index(r, 0, root);

        if (c.sibling(c.row(), 5).data().toList().isEmpty())
            propagateMaterial(c, m);
    }
}

void TreeModel::setMaterial(const QModelIndex &root, Material *m, int position)
{
    auto index = root.sibling(root.row(), 5);

    auto list = index.data().toList();
    list.insert(position, QVariant::fromValue(m));

    setData(index, list);

    if (position >= list.length() - 1)
        propagateMaterial(root.sibling(root.row(), 0), m);

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

        propagateMaterial(root.sibling(root.row(), 0), m);
    }

    m->assign(index, false);

    emit propertyChanged();

    return m;
}

void TreeModel::updateMaterial(const QModelIndex &root, const Material* m)
{
    auto list = root.sibling(root.row(), 5).data().toList();

    if (!list.isEmpty())
    {
        auto model = qobject_cast<MaterialListModel*>(session->listView->model());

        QVariantList u;

        for (auto i : list)
            u.append(QVariant::fromValue(model->getMaterialById(i.toString())));

        setData(root.sibling(root.row(), 5), u);

        auto n = u.last().value<Material*>();
        n->assign(root);
        m = n;
    }

    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
        reinterpret_cast<AtomItem*>(root.internalPointer())->setMaterial(m);

    for (int r = 0; r < rowCount(root); r++)
        updateMaterial(index(r, 0, root), m);
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

void TreeModel::updateVisibility(const QModelIndex &root, QPair<bool, bool> v)
{
    if (root.isValid())
    {
        Visibility w;

        w = (Visibility)root.sibling(root.row(), Editor).data().toInt();

        if (w != Default)
            v.first = w == On;

        w = (Visibility)root.sibling(root.row(), Renderer).data().toInt();

        if (w != Default)
            v.second = w == On;

        if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
        {
            AtomItem* a = reinterpret_cast<AtomItem*>(root.internalPointer());
            a->setFlag(VisibleInEditor, v.first);
            a->setFlag(VisibleInRenderer, v.second);
        }
    }

    for (int r = 0; r < rowCount(root); r++)
        updateVisibility(index(r, 0, root), v);
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

void TreeModel::pickSelected(const QModelIndex &root, QVector<VizBallInstance> &buffer) const
{
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
    {
        auto atom = reinterpret_cast<AtomItem*>(root.internalPointer());

        if (atom->isSelected())
            buffer.append(atom->getInstance());
    }

    for (int r = 0; r < rowCount(root); r++)
        pickSelected(index(r, 0, root), buffer);
}

void TreeModel::propagateSelected(const QModelIndex &root, bool s)
{
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
        reinterpret_cast<AtomItem*>(root.internalPointer())->setFlag(Selected, s);

    for (int r = 0; r < rowCount(root); r++)
        propagateSelected(index(r, 0, root), s);
}

QVector<VizBallInstance> TreeModel::getSelected() const
{
    QVector<VizBallInstance> ans;

    pickSelected(QModelIndex(), ans);

    return ans;
}

void TreeModel::setSelected(const QModelIndexList &indices, bool s)
{
    for (const auto& i : indices)
        if (i.column() == 0)
            propagateSelected(i, s);

    emit propertyChanged();
}

void TreeModel::propagateOrigin(const QModelIndex &root, QVector3D &s, int& c, bool selected) const
{
    if (root.sibling(root.row(), 1).data().toInt() == AtomObject)
    {
        auto atom = reinterpret_cast<AtomItem*>(root.internalPointer());

        if (atom->isSelected() || !selected)
        {
            s += atom->getPosition();
            c += 1;
        }
    }

    for (int r = 0; r < rowCount(root); r++)
        propagateOrigin(index(r, 0, root), s, c, selected);
}

QVector3D TreeModel::getOrigin(bool selected) const
{
    QVector3D s;
    int c = 0;

    propagateOrigin(QModelIndex(), s, c, selected);

    return s / c;
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
#include <QJsonArray>

void TreeModel::read(const QJsonObject &json)
{
    const QJsonObject children = json["Descendants"].toObject();

    for (auto child = children.end() - 1; child != children.begin() - 1; child--)
    {
        const QJsonObject object = child.value().toObject()["Object"].toObject();

        if (object["class"] == "Layer")
        {
            auto simulationLayer = std::make_shared<SimulationLayerConcatenation>();
            simulationLayer->read(object["paths"].toArray());

            session->simulation->addSimulationLayerConcatenation(simulationLayer);
        }

        if (object["class"] == "Camera")
            qobject_cast<TreeModel*>(session->treeView->model())->addCamera(new Camera(session));
    }

    header->read(json);

    updateMaterial(QModelIndex(), Material::getDefault());
    updateVisibility(QModelIndex(), {true, true});
}

void TreeModel::write(QJsonObject &json) const
{
    header->write(json);
}

void TreeModel::writePOVFrame(QTextStream &stream, std::shared_ptr<Frame> frame)
{
    header->writePOVFrame(stream, frame);
}

void TreeModel::writePOVFrames(QTextStream &stream, frameNumber_t fbeg, frameNumber_t fend)
{
    header->writePOVFrames(stream, fbeg, fend);
}
