#include "material.h"

Material* Material::dm = nullptr;

Material::Material(QString n, QColor c, float t, QColor sc, float se, QWidget *parent) :
    QWidget(parent),
    clicked(false),
    id(QUuid::createUuid()),
    name(n),
    color(c),
    transparency(t),
    specularColor(sc),
    specularExponent(se)
{
    setFixedSize(45, 45);
    updateIcon();
}

Material::~Material()
{

}

QUuid Material::getId() const
{
    return id;
}

QString Material::getName() const
{
    return name;
}

void Material::setName(QString n)
{
    name = n;
}

QColor Material::getColor() const
{
    return color;
}

void Material::setColor(QColor c)
{
    color = c;
    updateIcon();
}

float Material::getTransparency() const
{
    return transparency;
}

void Material::setTransparency(float t)
{
    transparency = t;
    updateIcon();
}

QColor Material::getSpecularColor() const
{
    return specularColor;
}

void Material::setSpecularColor(QColor c)
{
    specularColor = c;
    updateIcon();
}

float Material::getSpecularExponent() const
{
    return specularExponent;
}

void Material::setSpecularExponent(qreal e)
{
    specularExponent = e;
    updateIcon();
}

int Material::getFinish() const
{
    return finish;
}

void Material::setFinish(int f)
{
    finish = f;
    updateIcon();
}

Material* Material::getDefault()
{
    return dm ? dm : (dm = new Material);
}

#include <QPainter>

void Material::paint(QPainter *painter, QRect bounds)
{
    if (p.state() != QProcess::NotRunning)
        updates[dynamic_cast<QWidget*>(painter->device())] = bounds;

    icon.paint(painter, bounds, Qt::AlignCenter, mode);
}

void Material::assign(const QPersistentModelIndex &ix, bool b)
{
    if (b)
        assignment.append(ix);
    else
        assignment.removeOne(ix);
}

const QList<QPersistentModelIndex>& Material::getAssigned() const
{
    return assignment;
}

#include <QJsonObject>

void Material::read(const QJsonObject& json)
{
    id = json["UUID"].toString();
    name = json["Name"].toString();

    const QJsonObject color_ = json["Color"].toObject();
    color = color_["Color"].toString();
    transparency = color_["Transparency"].toDouble();

    const QJsonObject specular = json["Specular"].toObject();
    specularExponent = specular["Shininess exponent"].toDouble();
    specularColor = specular["Specular color"].toString();
}

void Material::write(QJsonObject& json) const
{
    json["UUID"] = id.toString();
    json["Name"] = name;

    QJsonObject color_;
    color_["Color"] = color.name();
    color_["Transparency"] = transparency;
    json["Color"] = color_;

    QJsonObject specular;
    specular["Shininess exponent"] = specularExponent;
    specular["Specular color"] = specularColor.name();
    json["Specular"] = specular;
}

void Material::mousePressEvent(QMouseEvent *event)
{
    clicked = true;

    QWidget::mousePressEvent(event);
}

#include <QMimeData>
#include <QDrag>

void Material::mouseMoveEvent(QMouseEvent *event)
{
    if (clicked)
    {
        clicked = false;

        QDrag *drag = new QDrag(this);
        drag->setMimeData(new QMimeData);
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
    else
        QWidget::mouseMoveEvent(event);
}

void Material::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    paint(&p, rect());
}

#include <QTemporaryFile>
#include <fstream>

void Material::updateIcon()
{
    p.disconnect();
    p.close();

    mode = QIcon::Disabled;

    QTemporaryFile* f = new QTemporaryFile;
    f->open();

    std::ofstream file(f->fileName().toStdString());

    file << *this
         << "plane {z, 1000 texture{pigment {gradient <1,1,0> color_map {[0.0 color rgb<0.4, 0.4, 0.4>] [0.5 color rgb<0.4, 0.4, 0.4>] [0.5 color rgb<0.6, 0.6, 0.6>  ] [1.0 color rgb<0.6, 0.6, 0.6>  ] } scale 500 translate 0}}}\n"
         << "camera {perspective location <0, 0, -5> look_at <0, 0, 0>}\n"
         << "sphere {<0, 0, 0>, 2 material { " << this << " }}\n"
         << "light_source {<1, 1, -2> color rgb<1, 1, 1> parallel point_at <0,0,0>}\n";

    file.close();

    QStringList argv;
    argv << "+W256"
         << "+H256"
         << "-GA"
         << "-D"
         << "-O-"
         << "+I\"" + f->fileName() + "\"";

    p.start("povray", argv);

    connect(&p, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [f, this](int exitCode){
        if (exitCode == 0)
        {
            QImage img;
            img.loadFromData(p.readAllStandardOutput(), "PNG");

            icon = QIcon(QPixmap::fromImage(img));
            mode = QIcon::Normal;

            for (auto i = updates.begin(); i != updates.end(); i++)
                if (i.key()) i.key()->update(i.value());

            updates.clear();
        }

        f->deleteLater();
    });
}

#include "rendersettings.h"

std::ostream &Material::operator<<(std::ostream &stream) const
{
    auto renderSettings = RenderSettings::getInstance();

    switch (finish)
    {
    case 0://custom
        stream << "#declare " << this << " =\n"
               << "material{texture {\n"
               << " pigment {\n"
               << "  color rgb<" << color.redF() << ", " << color.greenF() << ", " << color.blueF() << ">\n"
               << "  transmit " << 1. - (1. - transparency) * (1. - transparency) << "\n"
               << " }\n"
               << " finish {\n"
               << "  ambient " << renderSettings->ambient().toStdString() << "\n"
               << "  diffuse " << renderSettings->diffuse().toStdString() << "\n"
               << "  phong " << renderSettings->phong().toStdString() << "\n"
               << "  phong_size " << renderSettings->phongSize().toStdString() << "\n"
               << "  metallic " << renderSettings->metallic().toStdString() << "\n"
               << "  irid { " << renderSettings->iridescence().toStdString() << "\n"
               << "   thickness " << renderSettings->iridescenceThickness().toStdString() << "\n"
               << "   turbulence " << renderSettings->iridescenceTurbulence().toStdString() << "\n"
               << "  }\n"
               << "  specular 1.0\n"
               << "  roughness 0.02\n"
               << " }\n"
               << "}}\n";
        break;
    case 1://metal
        break;
    case 2://glass
        stream << "#declare Glass_Interior" << this << " = interior {ior 1.5}\n"
               << "#declare Glass3" << this << " = \n"
               << "texture {\n"
               << "pigment { rgbf <" << color.redF() << ", " << color.greenF() << ", " << color.blueF() << ", " << 1. - (1. - transparency) * (1. - transparency) << "> }\n"
               << "finish  { ambient 0.1 diffuse 0.1 reflection 0.1 specular 0.8 roughness 0.0003 phong 1 phong_size 400}\n"
               << "}\n"
               << "#declare " << this << " = material {texture {Glass3" << this << "} interior {Glass_Interior" << this << "}}\n";
        break;
    }

    return stream;
}

std::ostream &operator<<(std::ostream &stream, const Material &mat)
{
    return mat << stream;
}

std::ostream &operator<<(std::ostream &stream, const Material *mat)
{
    return stream << "m" << QString::number(quintptr(mat), 16).toStdString();
}
