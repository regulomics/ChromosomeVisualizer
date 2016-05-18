#ifndef MOVIEMAKER_H
#define MOVIEMAKER_H

#include <cstdlib>
#include <fstream>

#include <QVector3D>

#include "../QtChromosomeViz_v2/VizWidget.hpp"
#include "camera.h"
#include "rendersettings.h"

#include <QSettings>
#include <QProcess>

std::ostream& operator<<(std::ostream& out, const QVector3D & vec)
{
    return out << "<" << -vec.x() << ", " << vec.y() << ", " << vec.z() << ">";
}

std::ostream& operator<<(std::ostream& out, const QColor & col)
{
    return out << "rgbt<" << col.redF() << ", " << col.greenF() << ", " << col.blueF() << ", " << 1. - col.alphaF() * col.alphaF() << ">";
}

class MovieMaker
{
public:

    static void inline makeMovie(QString filename, int frames, float framerate, int fps)
    {
        QStringList argv;
        argv << "-framerate " + QString::number(framerate) << "-i " + filename + "%0" + QString::number(QString::number(frames).length()) + ".png" << "-c:v libx264"
             << "-r " + QString::number(fps) << "-pix_fmt yuv420p" << filename + ".mp4";
        //QProcess::execute("ffmpeg", argv);
        QProcess::execute(QString("ffmpeg ") + "-y" + " -framerate " + QString::number(framerate) + " -i " + filename + "%0" + QString::number(QString::number(frames).length()) + "d.png" + " -c:v libx264"
                          + " -r " + QString::number(fps) + " -pix_fmt yuv420p " + filename + ".mp4");
    }

    static void inline captureScene(const VizWidget* scene, const Camera* camera, const RenderSettings * renderSettings, QString number)
    {
        prepareINIFile(renderSettings->outputSize(), true);
        std::ofstream outFile;
        createPOVFile(outFile, (renderSettings->saveFile() + number).toStdString());

        setCamera(outFile, camera, renderSettings->outputSize());
        setBackgroundColor(outFile, scene->backgroundColor());
        //setFog(outFile, backgroundColor, distance); //TODO: dobre rownanie dla ostatniego argumentu

        auto& vizBalls = scene->getBallInstances();

        for (auto b : vizBalls)
            addSphere(outFile, b.position, b.size, QColor::fromRgba(b.color));

        for (int i = 0; i < vizBalls.length() - 1; i++)
        {
            if (((vizBalls[i].color >> 24) == 0xFF) && ((vizBalls[i+1].color >> 24) == 0xFF))
                addCylinder(outFile, vizBalls[i].position, vizBalls[i + 1].position, vizBalls[i].size / 2, QColor::fromRgba(vizBalls[i].color), QColor::fromRgba(vizBalls[i + 1].color));
            else
                break;
        }

        for (auto & key : scene->getLabels().keys())
            addLabel(outFile, "");

        outFile.flush();

        QSettings settings;

#ifdef __linux__
        QStringList argv;
        argv << "povray.ini" << "+L" + settings.value("povraypath", "/usr/local/share/povray-3.7").toString() + "/include/" << renderSettings->saveFile() + number + ".pov";
        QProcess::execute("povray", argv);
#elif _WIN32
        qDebug() << "windows povray photo";
        system((QString(R"~(""C:\Program Files\POV-Ray\v3.7\bin\pvengine64.exe"" povray.ini -D /RENDER )~") + settings.saveFile() + QString(".pov /EXIT")).toUtf8().constData());
#else
        qDebug() << "platform not supported";
#endif
    }

private:

    static void inline prepareINIFile(QSize size, bool aa)
    {
        std::ofstream outFile("povray.ini");
        outFile << "Width=" << size.width() << "\nHeight=" << size.height() << "\nAntialias=" << (aa ? "On" : "Off") << "\n";
    }

    static void inline createPOVFile(std::ofstream& outFile, std::string filename)
    {
        outFile.open(filename + ".pov");
        outFile << "#include \"colors.inc\"\n#include \"stones.inc\"\n"
                << "\n";
    }

    static void inline setCamera(std::ofstream& outFile, const Camera* camera, QSize size)
    {
        outFile << "camera{\n"
                << "right x*" << size.width() << "/" << size.height() << "\n"
                << "location " << camera->position() << "\n"
                << "look_at " << camera->lookAt() << "\n"
                << "angle " << camera->angle() << "\n"
                << "}\n"
                << "\n";

        outFile << "light_source {" << camera->position() << " " << "color " << QColor(Qt::white) << "}\n"
                << "\n";
    }

    static void inline setBackgroundColor(std::ofstream& outFile, const QColor & color)
    {
        outFile << "background{color " << color << "}\n";
    }

    static void inline setFog(std::ofstream& outFile, const QColor & color, const float distance)
    {
        outFile << "fog{color " << color << " distance " << distance << " }\n";
    }

    static void inline addSphere(std::ofstream& outFile, const QVector3D & position, float size, QColor color)
    {
        outFile << "sphere{" << position << ", " << size << " texture{pigment{" << color << "}}}\n";
    }

    static void inline addCylinder(std::ofstream& outFile, const QVector3D & positionA, const QVector3D & positionB, float radius, QColor colorA, QColor colorB)
    {
        QVector3D direction = positionB - positionA;
        qreal dist = qFabs(QVector3D::dotProduct(positionA, direction)) / direction.length(); //distance of positionA from plane defined by normal vector 'direction' and point (0,0,0)
        outFile << "cylinder{" << positionA << ", " << positionB << ", " << radius <<
            " texture{pigment{gradient" << direction << " color_map{[0.0 color " << colorA << "][1.0 color " <<
            colorB << "]} phase " << (dist - static_cast<int>(dist / direction.length()) * direction.length()) / direction.length() <<
            " scale " << direction.length() << "}}}\n";
    }

    static void inline addLabel(std::ofstream& outFile, const QString & label)
    {

    }
};

#endif // MOVIEMAKER_H