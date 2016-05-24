#include "attributes.h"
#include "ui_attributes.h"

Attributes::Attributes(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Attributes)
{
    ui->setupUi(this);

    // set name

    // set label
    connect(ui->lineEdit_2, &QLineEdit::editingFinished, [this] {
        vizWidget_->selectedSpheresObject().setLabel(ui->lineEdit_2->text());
    });

    // set vie

    // set vir

    // set radius
    connect(ui->doubleSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] () {
        ui->doubleSpinBox->setSpecialValueText("");
    });

    connect(ui->doubleSpinBox, &QDoubleSpinBox::editingFinished, [this] {
        if (ui->doubleSpinBox->specialValueText().isEmpty())
            vizWidget_->selectedSpheresObject().setSize(ui->doubleSpinBox->value());
    });

    // set segments

    // set color
    connect(ui->widget, &Picker::valueChanged, [this] (QColor color) {
        vizWidget_->selectedSpheresObject().setColor(color);
    });

    // set transparency
    connect(ui->doubleSpinBox_2, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] () {
        ui->doubleSpinBox_2->setSpecialValueText("");
    });

    connect(ui->doubleSpinBox_2, &QDoubleSpinBox::editingFinished, [this] {
        if (ui->doubleSpinBox_2->specialValueText().isEmpty())
            vizWidget_->selectedSpheresObject().setAlpha((100. - ui->doubleSpinBox_2->value()) / 100);
    });

    // set specular width
    connect(ui->doubleSpinBox_3, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] () {
        ui->doubleSpinBox_3->setSpecialValueText("");
    });

    connect(ui->doubleSpinBox_3, &QDoubleSpinBox::editingFinished, [this] {
        if (ui->doubleSpinBox_3->specialValueText().isEmpty())
            vizWidget_->selectedSpheresObject().setSpecularExponent(ui->doubleSpinBox_3->value());
    });

    // set specular color
    connect(ui->widget_2, &Picker::valueChanged, [this] (QColor color) {
        vizWidget_->selectedSpheresObject().setSpecularColor(color);
    });
}

Attributes::~Attributes()
{
    delete ui;
}

void Attributes::setVizWidget(VizWidget *vizWidget)
{
    vizWidget_ = vizWidget;
}

void Attributes::handleSelection(const AtomSelection &selection)
{
    if (!selection.atomCount())
        return hide();

    // set title
    QString title("Atom object ");

    if (selection.atomCount() > 1)
        title += "(" + QString::number(selection.atomCount()) + " elements) ";

    title += "[";

    for (auto i : selection.selectedIndices())
        title += "";

    title += "]";

    ui->label_14->setText(title);

    // set name

    // set label
    auto l = selection.getLabel();

    ui->lineEdit_2->setText(l.isValid() ? l.toString() : "<< multiple values >>");

    // set coordinates

    // set radius
    auto r = selection.getSize();

    if (r.isValid())
    {
        ui->doubleSpinBox->setValue(r.toFloat());
        ui->doubleSpinBox->setSpecialValueText("");
    }
    else
    {
        ui->doubleSpinBox->setValue(ui->doubleSpinBox->minimum());
        ui->doubleSpinBox->setSpecialValueText("<< multiple values >>");
    }

    // set segments

    // set color
    ui->widget->setValue(selection.getColor());

    // set transparency
    auto a = selection.getAlpha();

    if (a.isValid())
    {
        ui->doubleSpinBox_2->setValue(a.toFloat());
        ui->doubleSpinBox_2->setSpecialValueText("");
    }
    else
    {
        ui->doubleSpinBox_2->setValue(ui->doubleSpinBox_2->minimum());
        ui->doubleSpinBox_2->setSpecialValueText("<< multiple values >>");
    }

    // set specular width
    auto w = selection.getAlpha();

    if (w.isValid())
    {
        ui->doubleSpinBox_3->setValue(w.toFloat());
        ui->doubleSpinBox_3->setSpecialValueText("");
    }
    else
    {
        ui->doubleSpinBox_3->setValue(ui->doubleSpinBox_3->minimum());
        ui->doubleSpinBox_3->setSpecialValueText("<< multiple values >>");
    }

    // set specular color
    ui->widget_2->setValue(selection.getSpecularColor());

    show();
}
