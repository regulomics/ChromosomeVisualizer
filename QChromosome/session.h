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

    void setFrame(std::shared_ptr<Frame> frame);
};

#endif // SESSION_H
