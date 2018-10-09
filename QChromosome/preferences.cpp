#include "preferences.h"
#include "ui_preferences.h"

#include <QProxyStyle>
#include <QPainter>

class MyProxyStyle : public QProxyStyle
{
public:
    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const
    {
        QRect _rect(QPoint(), rect.size().transposed());

        painter->save();

        painter->translate(rect.center());
        painter->rotate(90);
        painter->translate(-_rect.center());

        QProxyStyle::drawItemText(painter, _rect, flags, pal, enabled, text, textRole);

        painter->restore();
    }
};

#include <QSettings>
#include <QStandardPaths>

Preferences::Preferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    ui->TabWidget->tabBar()->setStyle(new MyProxyStyle);

    connect(ui->lineEdit_2, &QLineEdit::editingFinished, [this]() {
        QSettings().setValue("locallib", ui->lineEdit_2->text());
    });

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        ui->stackedWidget->setCurrentIndex(index);
    });

    QSettings settings;
    ui->lineEdit_2->setText(settings.value("locallib", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString());
}

Preferences::~Preferences()
{
    delete ui;
}

#include "treemodel.h"

PointerToMemberFunction Preferences::coloringMethod() const
{
    switch (ui->comboBox->currentIndex())
    {
    case 0:
        return &TreeModel::colorByResidue;
    case 1:
        return &TreeModel::colorByChain;
    }
}
