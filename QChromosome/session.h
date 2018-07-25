#ifndef SESSION_H
#define SESSION_H


#include <QOpenGLBuffer>

template<typename T>
class GLBuffer : public QVector<T>
{
public:
    void allocate(QOpenGLBuffer& buffer)
    {
        if (resized)
        {
            buffer.bind();
            buffer.allocate(QVector<T>::constData(), QVector<T>::count() * sizeof(T));
            buffer.release();

            resized = false;
            modified = false;
        }

        if (modified)
        {
            buffer.bind();
            buffer.write(0, QVector<T>::constData(), QVector<T>::count() * sizeof(T));
            buffer.release();

            modified = false;
        }
    }

    int emplace_back()
    {
        resized = true;
        QVector<T>::push_back(T());
        return QVector<T>::count() - 1;
    }

    void resize(int size)
    {
        resized = true;
        QVector<T>::resize(QVector<T>::size() + size);
    }

    inline T& operator[](int i)
    {
        modified = true;
        return QVector<T>::operator[](i);
    }

private:
    bool modified;
    bool resized;
};

#include <QAction>
#include "treeitem.h"
#include "../QtChromosomeViz_v2/bartekm_code/Simulation.h"
#include "camera.h"
#include "labelatlas.h"
#include "treeview.h"
#include "namedelegate.h"
#include "visibilitydelegate.h"
#include "tagsdelegate.h"
#include <QFileInfo>
#include "viewport.h"

template class GLBuffer<VizCameraInstance>;
template class GLBuffer<VizBallInstance>;

class Session
{
public:
    Session();
    ~Session();

    QAction *action;
    Simulation *simulation;

    GLBuffer<VizCameraInstance> cameraBuffer;
    GLBuffer<VizBallInstance> atomBuffer;
    QVector<std::pair<int, int>> chainBuffer;

    camera_data_t cameraUniformBuffer;

    LabelAtlas labelAtlas;

    Camera* editorCamera;

    TreeView* treeView;

    Viewport* viewport;

    void setFrame(std::shared_ptr<Frame> frame);

    // Project Settings
    int FPS;
    int documentTime;
    int maximumTime;
    int minimumTime;
    int previewMinTime;
    int previewMaxTime;

    QString author;
    QString info;
    QString fileFormat;
    QString fileVersion;
    QFileInfo filePath;

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;

private:
    NameDelegate *nd;
    VisibilityDelegate *vd;
    TagsDelegate *td;
};

#endif // SESSION_H
