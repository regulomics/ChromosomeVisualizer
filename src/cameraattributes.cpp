#include "cameraattributes.h"
#include "ui_cameraattributes.h"
#include "treemodel.h"
#include "camera.h"

CameraAttributes::CameraAttributes(QWidget *parent) :
    MetaAttributes(parent),
    ui(new Ui::CameraAttributes),
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

    //connect P.X
    connect(ui->doubleSpinBox, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
        {
            auto pos = c->getPosition();
            pos.setX(val);
            c->setPosition(pos);
        }
        emit modelViewChanged();
    });

    //connect P.Y
    connect(ui->doubleSpinBox_2, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
        {
            auto pos = c->getPosition();
            pos.setY(val);
            c->setPosition(pos);
        }
        emit modelViewChanged();
    });

    //connect P.Z
    connect(ui->doubleSpinBox_3, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
        {
            auto pos = c->getPosition();
            pos.setZ(val);
            c->setPosition(pos);
        }
        emit modelViewChanged();
    });

    //connect R.H
    connect(ui->doubleSpinBox_4, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
        {
            auto ang = c->getRotation().toEulerAngles();
            c->setRotation(QQuaternion::fromEulerAngles(ang[0], val, ang[2]));
        }
        emit modelViewChanged();
    });

    //connect R.P
    connect(ui->doubleSpinBox_5, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
        {
            auto ang = c->getRotation().toEulerAngles();
            c->setRotation(QQuaternion::fromEulerAngles(val, ang[1], ang[2]));
        }
        emit modelViewChanged();
    });

    //connect R.B
    connect(ui->doubleSpinBox_6, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
        {
            auto ang = c->getRotation().toEulerAngles();
            c->setRotation(QQuaternion::fromEulerAngles(ang[0], ang[1], val));
        }
        emit modelViewChanged();
    });

    //connect projection
    connect(ui->comboBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int val) {
        for (auto c : cameras)
            c->setProjectionType(static_cast<Projection>(val));

        switch (val)
        {
        case CP_Perspective:
            ui->widget->setEnabled(true);
            ui->widget_2->setEnabled(false);
            break;
        case CP_Parallel:
            ui->widget->setEnabled(false);
            ui->widget_2->setEnabled(true);
            break;
        }
    });

    //connect focal length
    connect(ui->doubleSpinBox_7, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setFocalLength(val);
        updateProjection();
    });

    //connect sensor size
    connect(ui->doubleSpinBox_8, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setSensorSize(val);
        updateProjection();
    });

    //connect field of view (horizontal)
    connect(ui->doubleSpinBox_9, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setHorizontalAngle(val);
        updateProjection();
    });

    //connect field of view (vertical)
    connect(ui->doubleSpinBox_10, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setVerticalAngle(val);
        updateProjection();
    });

    //connect near clippping
    connect(ui->doubleSpinBox_11, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setNearClipping(val);
    });

    //connect far clippping
    connect(ui->doubleSpinBox_12, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setFarClipping(val);
    });

    //connect zoom
    connect(ui->doubleSpinBox_16, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setZoom(val);
        updateProjection();
    });

    //connect mode
    connect(ui->comboBox_4, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        for (auto c : cameras)
            c->setMode(static_cast<Camera::Mode>(index));
    });

    //connect eye separation
    connect(ui->doubleSpinBox_13, QOverload<double>::of(&DoubleSpinBox::valueChanged), [this](double val) {
        for (auto c : cameras)
            c->setEyeSeparation(val);
    });

    //connect base
    connect(ui->lineEdit_2, &PickWidget::picked, [this](const QPersistentModelIndex& index) {
        for (auto c : cameras)
            c->setBase(index);
    });

    //connect target object
    connect(ui->lineEdit_3, &PickWidget::picked, [this](const QPersistentModelIndex& index) {
        for (auto c : cameras)
            c->setTarget(index);
    });

    //connect up
    connect(ui->lineEdit_4, &PickWidget::picked, [this](const QPersistentModelIndex& index) {
        for (auto c : cameras)
            c->setUp(index);
    });

    //connect item selection
    connect(ui->lineEdit_2, &PickWidget::selected, [this](const QPersistentModelIndex& index) {
        emit selected(index);
    });

    connect(ui->lineEdit_3, &PickWidget::selected, [this](const QPersistentModelIndex& index) {
        emit selected(index);
    });

    connect(ui->lineEdit_4, &PickWidget::selected, [this](const QPersistentModelIndex& index) {
        emit selected(index);
    });

    ui->tabWidget->removeTab(5);
}

CameraAttributes::~CameraAttributes()
{
    delete ui;
}

#include "treeitem.h"

void CameraAttributes::setSelection(TreeModel* selectedModel, const QModelIndexList &selectedRows)
{
    Q_ASSERT(!selectedRows.empty());

    ui->tabWidget->setCurrentIndex(0);

    model = selectedModel;
    rows = selectedRows;

    cameras.clear();
    cameras.reserve(selectedRows.size());

    for (const auto& i : selectedRows)
        cameras.append(reinterpret_cast<CameraItem*>(i.internalPointer())->getCamera());

    connect(model, &TreeModel::attributeChanged, this, &CameraAttributes::updateModelSelection);

    for (const auto c : cameras)
    {
        connect(c, &Camera::modelViewChanged, this, &CameraAttributes::updateModelView);
        connect(c, &Camera::projectionChanged, this, &CameraAttributes::updateProjection);
    }

    // set title
    QString title("Camera object ");

    if (rows.count() > 1)
        title += "(" + QString::number(rows.count()) + " elements) ";

    ui->label->setTitle(title);

    updateModelSelection();

    updateModelView();

    updateProjection();

    auto fst = cameras.first();
    bool multiple;

    //set projection
    Projection p = fst->getProjectionType();
    multiple = false;

    for (const auto& c : cameras)
        if (p != c->getProjectionType())
        {
            multiple = true;
            break;
        }

    if (multiple)
    {
        ui->comboBox_3->setMultipleValues();
        ui->widget->setEnabled(false);
        ui->widget_2->setEnabled(false);
    }
    else
    {
        ui->comboBox_3->setCurrentIndex(p, false);

        switch (p)
        {
        case CP_Perspective:
            ui->widget->setEnabled(true);
            ui->widget_2->setEnabled(false);
            break;
        case CP_Parallel:
            ui->widget->setEnabled(false);
            ui->widget_2->setEnabled(true);
            break;
        }
    }

    //set mode
    Camera::Mode m = fst->getMode();
    multiple = false;

    for (const auto& c : cameras)
        if (m != c->getMode())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->comboBox_4->setMultipleValues();
    else
        ui->comboBox_4->setCurrentIndex(m, false);

    //set eye separation
    qreal e = fst->getEyeSeparation();
    multiple = false;

    for (const auto& c : cameras)
        if (e != c->getEyeSeparation())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_13->setMultipleValues();
    else
        ui->doubleSpinBox_13->setValue(e, false);

    // set base
    QModelIndex b = fst->getBase();
    multiple = false;

    for (const auto& c : cameras)
        if (b != c->getBase())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->lineEdit_2->setMultipleValues();
    else
        ui->lineEdit_2->pick(b, false);

    // set target object
    QModelIndex t = fst->getTarget();
    multiple = false;

    for (const auto& c : cameras)
        if (t != c->getTarget())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->lineEdit_3->setMultipleValues();
    else
        ui->lineEdit_3->pick(t, false);

    // set up
    QModelIndex u = fst->getUp();
    multiple = false;

    for (const auto& c : cameras)
        if (u != c->getUp())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->lineEdit_4->setMultipleValues();
    else
        ui->lineEdit_4->pick(u, false);
}

void CameraAttributes::unsetSelection()
{
    if (model) model->disconnect(this);
    model = nullptr;

    for (const auto c : cameras)
        c->disconnect(this);
}

void CameraAttributes::updateModelSelection()
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

void CameraAttributes::updateModelView()
{
    const auto fst = cameras.first();
    bool multiple;

    // set P.X
    double x = fst->getPosition().x();
    multiple = false;

    for (const auto c : cameras)
        if (x != c->getPosition().x())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox->setMultipleValues();
    else
        ui->doubleSpinBox->setValue(x, false);

    // set P.Y
    double y = fst->getPosition().y();
    multiple = false;

    for (const auto c : cameras)
        if (y != c->getPosition().y())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_2->setMultipleValues();
    else
        ui->doubleSpinBox_2->setValue(y, false);

    // set P.Z
    double z = fst->getPosition().z();
    multiple = false;

    for (const auto c : cameras)
        if (z != c->getPosition().z())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_3->setMultipleValues();
    else
        ui->doubleSpinBox_3->setValue(z, false);

    // set R.H
    double h = fst->getRotation().toEulerAngles()[1];
    multiple = false;

    for (const auto c : cameras)
        if (h != c->getRotation().toEulerAngles()[1])
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_4->setMultipleValues();
    else
        ui->doubleSpinBox_4->setValue(h, false);

    // set R.P
    double p = fst->getRotation().toEulerAngles()[0];
    multiple = false;

    for (const auto c : cameras)
        if (p != c->getRotation().toEulerAngles()[0])
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_5->setMultipleValues();
    else
        ui->doubleSpinBox_5->setValue(p, false);

    // set R.B
    double b = fst->getRotation().toEulerAngles()[2];
    multiple = false;

    for (const auto c : cameras)
        if (b != c->getRotation().toEulerAngles()[2])
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_6->setMultipleValues();
    else
        ui->doubleSpinBox_6->setValue(b, false);
}

void CameraAttributes::updateProjection()
{
    const auto fst = cameras.first();
    bool multiple;

    // set focal length
    double fl = fst->getFocalLength();
    multiple = false;

    for (const auto c : cameras)
        if (fl != c->getFocalLength())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_7->setMultipleValues();
    else
        ui->doubleSpinBox_7->setValue(fl, false);

    // set sensor size
    double ss = fst->getSensorSize();
    multiple = false;

    for (const auto c : cameras)
        if (ss != c->getSensorSize())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_8->setMultipleValues();
    else
        ui->doubleSpinBox_8->setValue(ss, false);

    // set horizontal angle
    double ha = fst->getHorizontalAngle();
    multiple = false;

    for (const auto c : cameras)
        if (ha != c->getHorizontalAngle())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_9->setMultipleValues();
    else
        ui->doubleSpinBox_9->setValue(ha, false);

    // set vertical angle
    double va = fst->getVerticalAngle();
    multiple = false;

    for (const auto c : cameras)
        if (va != c->getVerticalAngle())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_10->setMultipleValues();
    else
        ui->doubleSpinBox_10->setValue(va, false);

    // set near clipping
    double nc = fst->getNearClipping();
    multiple = false;

    for (const auto c : cameras)
        if (nc != c->getNearClipping())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_11->setMultipleValues();
    else
        ui->doubleSpinBox_11->setValue(nc, false);

    // set far clipping
    double fc = fst->getFarClipping();
    multiple = false;

    for (const auto c : cameras)
        if (fc != c->getFarClipping())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_12->setMultipleValues();
    else
        ui->doubleSpinBox_12->setValue(fc, false);

    // set zoom
    qreal z = fst->getZoom();
    multiple = false;

    for (const auto& c : cameras)
        if (z != c->getZoom())
        {
            multiple = true;
            break;
        }

    if (multiple)
        ui->doubleSpinBox_16->setMultipleValues();
    else
        ui->doubleSpinBox_16->setValue(z, false);
}
