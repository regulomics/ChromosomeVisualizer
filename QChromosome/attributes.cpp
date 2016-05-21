#include "attributes.h"
#include "ui_attributes.h"

Attributes::Attributes(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Attributes)
{
    ui->setupUi(this);

    // set label
    connect(ui->lineEdit_2, &QLineEdit::editingFinished, [this] {
        vizWidget_->selectedSpheresObject().setLabel(ui->lineEdit_2->text());
    });

    // set radius
    connect(ui->doubleSpinBox, &QDoubleSpinBox::editingFinished, [this] {
        vizWidget_->selectedSpheresObject().setSize(ui->doubleSpinBox->value());
    });

    // set color
    connect(ui->widget, &Picker::valueChanged, [this] (QColor color) {
        vizWidget_->selectedSpheresObject().setColor(color);
    });

    // set transparency
    connect(ui->doubleSpinBox_2, &QDoubleSpinBox::editingFinished, [this] {
        vizWidget_->selectedSpheresObject().setAlpha(1. - ui->doubleSpinBox_2->value() / 100);
    });

    // set specular width
    connect(ui->doubleSpinBox_3, &QDoubleSpinBox::editingFinished, [this] {
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

    // set coordinates


    // set object properties

    // set color
    //ui->widget->setInitialColor();

    // set specular
    //ui->widget_2->setInitialColor();

    show();
}
