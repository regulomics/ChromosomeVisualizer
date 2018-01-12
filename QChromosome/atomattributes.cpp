#include "atomattributes.h"
#include "ui_atomattributes.h"
#include "treemodel.h"

AtomAttributes::AtomAttributes(QWidget *parent) :
    MetaAttributes(parent),
    ui(new Ui::AtomAttributes),
    model(nullptr)
{
    ui->setupUi(this);

    // connect name
    connect(ui->lineEdit, &QLineEdit::editingFinished, [this] {
        model->setName(rows, ui->lineEdit->text());
    });

    // connect vie
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        model->setVisibility(rows, (Visibility)index, Editor);
    });

    // connect vir
    connect(ui->comboBox_2, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        model->setVisibility(rows, (Visibility)index, Renderer);
    });
}

AtomAttributes::~AtomAttributes()
{
    delete ui;
}

void AtomAttributes::setSelection(TreeModel* selectedModel, const QModelIndexList &selectedRows)
{
    Q_ASSERT(!selectedRows.empty());

    ui->tabWidget->setCurrentIndex(0);

    model = selectedModel;
    rows = selectedRows;

    connect(model, &TreeModel::attributeChanged, this, &AtomAttributes::updateModelSelection);

    // set title
    QString title = QString::number(rows.count()) + " elements ";

    ui->label->setTitle(title);

    updateModelSelection();

    updatePosition();
}

void AtomAttributes::unsetSelection()
{
    if (model) model->disconnect(this);
    model = nullptr;
}

void AtomAttributes::updateModelSelection()
{
    // set elements
    QString list;

    for (const auto& r : rows)
        list += r.data().toString() + ", ";

    list.chop(2);

    ui->label->setElements(list);

    auto fst = rows.first();
    bool multiple;

    // set name
    QString name = fst.data().toString();
    multiple = false;

    for (const auto& r : rows)
        if (name != r.data().toString())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->lineEdit->setMultipleValues();
    else
        ui->lineEdit->setText(name, false);

    // set vie
    int vie = fst.sibling(fst.row(), 3).data().toInt();
    multiple = false;

    for (const auto& r : rows)
        if (vie != r.sibling(r.row(), 3).data().toInt())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->comboBox->setMultipleValues();
    else
        ui->comboBox->setCurrentIndex(vie, false);

    // set vir
    int vir = fst.sibling(fst.row(), 4).data().toInt();
    multiple = false;

    for (const auto& r : rows)
        if (vir != r.sibling(r.row(), 4).data().toInt())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->comboBox_2->setMultipleValues();
    else
        ui->comboBox_2->setCurrentIndex(vir, false);
}

#include "treeitem.h"
#include "camera.h"

void AtomAttributes::updatePosition()
{
    auto fst = rows.first();
    bool multiple;

    // set P.X
    double x = reinterpret_cast<TreeItem*>(fst.internalPointer())->getPosition().x();
    multiple = false;

    for (const auto& r : rows)
        if (x != reinterpret_cast<TreeItem*>(r.internalPointer())->getPosition().x())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox->setMultipleValues();
    else
        ui->doubleSpinBox->setValue(x, false);

    // set P.Y
    double y = reinterpret_cast<TreeItem*>(fst.internalPointer())->getPosition().y();
    multiple = false;

    for (const auto& r : rows)
        if (y != reinterpret_cast<TreeItem*>(r.internalPointer())->getPosition().y())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_2->setMultipleValues();
    else
        ui->doubleSpinBox_2->setValue(y, false);

    // set P.Z
    double z = reinterpret_cast<TreeItem*>(fst.internalPointer())->getPosition().z();
    multiple = false;

    for (const auto& r : rows)
        if (z != reinterpret_cast<TreeItem*>(r.internalPointer())->getPosition().z())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_3->setMultipleValues();
    else
        ui->doubleSpinBox_3->setValue(z, false);

    // set camera origin
    QVector3D g;

    for (const auto& r : rows)
        g += reinterpret_cast<TreeItem*>(r.internalPointer())->getPosition();

    g /= rows.count();

    Camera::setOrigin(g);
}
