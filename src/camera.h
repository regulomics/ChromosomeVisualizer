#ifndef CAMERA_H
#define CAMERA_H

#include "interpolator.h"
#include "treeitem.h"

#include <QVector3D>
#include <QMatrix4x4>

class Session;

class Camera : public SplineInterpolator
{
    Q_OBJECT
public:
    explicit Camera(Session* s, QWidget *parent = 0);
    Camera(const Camera& camera);
    ~Camera();

    QVector3D getPosition() const;
    QQuaternion getRotation() const;
    qreal getFocalLength() const;
    qreal getSensorSize() const;
    qreal getHorizontalAngle() const;
    qreal getVerticalAngle() const;
    qreal getNearClipping() const;
    qreal getFarClipping() const;

    void setPosition(const QVector3D& p);
    void setRotation(const QQuaternion& q);
    void setFocalLength(qreal fl);
    void setSensorSize(qreal ss);
    void setHorizontalAngle(qreal ha);
    void setVerticalAngle(qreal va);
    void setNearClipping(qreal nc);
    void setFarClipping(qreal fc);

    void setLookAt(const QVector3D& target);

    static void lockCoordinates(bool x, bool y, bool z);

    enum Action {
        CA_Move,
        CA_Rotate,
        CA_Scale
    };

    static void setCurrentAction(Action ca);

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;

    SplineKeyframe saveFrame() const;
    void loadFrame(const SplineKeyframe &frame);

    void writePOVCamera(QTextStream &stream, bool interpolate) const;

    void setBase(const QModelIndex& index);
    const QModelIndex& getBase() const;

    void setTarget(const QModelIndex& index);
    const QModelIndex& getTarget() const;

    void setUp(const QModelIndex& index);
    const QModelIndex& getUp() const;

    void callibrate(const QModelIndex& index, bool selected, qreal scale = .75);

    enum Mode {
        CM_Mono,
        CM_Symmetrical,
        CM_Left,
        CM_Right
    };

    Mode getMode() const;
    void setMode(Mode cm);

    qreal getEyeSeparation() const;
    void setEyeSeparation(qreal es);

    Projection getProjectionType() const;
    void setProjectionType(Projection p);

    qreal getZoom() const;
    void setZoom(qreal z);

    QAction* action;

public slots:
    /* handles mouse move event */
    void move(int dx, int dy);

    /* handles mouse rotate event */
    void rotate(int dx, int dy);

    /* handles mouse scale event */
    void scale(int dx, int dy);

    /* updates aspect ratio */
    void setAspectRatio(qreal ar);

protected:
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);

    void connectNotify(const QMetaMethod &signal);
    void showEvent(QShowEvent *event);

    void paintEvent(QPaintEvent *event);

private:
    /* eye position */
    QVector3D eye;

    /* Euler angles */
    QQuaternion phb;

    /* moves camera with respect to current coordinates system */
    void move(qreal dx, qreal dy, qreal dz);

    /* rotates camera with respect to current coordinates system */
    void rotate(qreal dp, qreal dh, qreal db);

    qreal focalLength;
    qreal sensorSize;

    qreal horizontalAngle;
    qreal verticalAngle;

    QMatrix4x4 modelView;
    QMatrix4x4 projection;

    /* updates modelview matrix */
    QMatrix4x4& updateModelView();

    /* updates projection matrix */
    QMatrix4x4& updateProjection();

    /* updates field of view */
    void updateAngles();

    static const qreal distanceFactor;
    static const qreal angleFactor;
    static const qreal scaleFactor;
    static const qreal wheelFactor;
    static const qreal zoomFactor;

    qreal aspectRatio;

    qreal nearClipping;
    qreal farClipping;

    static bool lockX, lockY, lockZ;

    static Action currentAction;

    QPersistentModelIndex base, target, up;

    Session *session;
    const int id;

    Mode mode;
    qreal eyeSeparation;

    qreal zoom;

signals:
    void modelViewChanged(QMatrix4x4);
    void projectionChanged(QMatrix4x4);
    void projectionTypeChanged(Projection);

friend class CameraItem;
friend class Session;
};

#endif // CAMERA_H
