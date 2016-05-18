#ifndef VIZWINDOW_HPP
#define VIZWINDOW_HPP

#include <memory>

#include <QtGui>
#include <QtOpenGL>
#include <QOpenGLFunctions_3_3_Core>

#include "bartekm_code/Simulation.h"
#include "LabelRenderer.hpp"
#include "SelectionRectWidget.hpp"

struct VizVertex
{
    QVector3D position;
    QVector3D normal;

    VizVertex rotated(const QQuaternion & q) const;
};

struct VizBallInstance
{
    QVector3D position;
    unsigned int flags;
    unsigned int atomID;
    unsigned int color;
    unsigned int specularColor;
    float specularExponent;
    float size;
};

struct VizLink
{
    QVector3D position;
    QQuaternion rotation;
    unsigned int color[2];
    unsigned int specularColor[2];
    float specularExponent[2];
    float size[3];

    void update(const QVector3D & p1, const QVector3D & p2);
};

class VizWidget;

class AtomSelection
{
    friend class VizWidget;

public:
    AtomSelection(const AtomSelection &) = default;

    void setColor(QColor color);
    void setAlpha(float alpha);
    void setSpecularColor(QColor color);
    void setSpecularExponent(float exponent);
    void setSize(float size);
    void setLabel(const QString & label);

    unsigned int atomCount() const;
    QVector3D weightCenter() const;
    const QList<unsigned int> & selectedIndices() const;

private:
    AtomSelection(VizWidget * widget);
    AtomSelection(QList<unsigned int> indices, VizWidget * widget);

    QList<unsigned int> selectedIndices_;
    VizWidget * widget_;
};

using VizSegment = QPair<VizVertex, VizVertex>;

class VizWidget :   public QOpenGLWidget,
                    protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
    friend class AtomSelection;

public:
    VizWidget(QWidget *parent = 0);
    virtual ~VizWidget();

    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;

    void setSimulation(std::shared_ptr<Simulation> dp);

public slots:
    void advanceFrame();
    void setFrame(frameNumber_t frame);
    void setFrame(int frame);

    void setModelView(QMatrix4x4 mat);
    void setProjection(QMatrix4x4 mat);

    void setSelectingState(bool flag);

    // quality should be in range [0.f, 1.f]
    void setBallQuality(float quality);

    bool isSelecting() const;
    QList<unsigned int> selectedSphereIndices() const;
    QList<Atom> selectedSpheres() const;
    AtomSelection selectedSpheresObject() const;

    AtomSelection allSelection();
    AtomSelection atomTypeSelection(const char * s);
    AtomSelection atomTypeSelection(const std::string & s);
    AtomSelection customSelection(const QList<unsigned int> & indices);

    void setVisibleSelection(AtomSelection s);

    void setBackgroundColor(QColor color);
    QColor backgroundColor() const;

    void setLabelTextColor(QColor color);
    QColor labelTextColor() const;

    void setLabelBackgroundColor(QColor color);
    QColor labelBackgroundColor() const;

    const QMap<unsigned int, QString> & getLabels() const;

    void setFogDensity(float intensity);
    void setFogContribution(float contribution);
    float fogDensity() const;
    float fogContribution() const;

    const QVector<VizBallInstance> & getBallInstances() const;

signals:
    void selectionChangedIndices(const QList<unsigned int> & selected,
                                 const QList<unsigned int> & deselected);
    void selectionChanged(const QList<Atom> & selected,
                          const QList<Atom> & deselected);
    void selectionChangedObject(const AtomSelection & selection);

protected:
    void paintLabels();

    // Generates vertices for a solid of revolution based on the given outline.
    //   quads - line segments disjoint from the axis of rotation
    //   axis - axis of rotation. Must not be zero.
    //   segments - number of segments to be generated.
    static QVector<VizVertex> generateSolidOfRevolution(
        const QVector<VizSegment> & quads,
        QVector3D axis,
        unsigned int segments
    );

    static QVector<VizVertex> generateSphere(unsigned int rings, unsigned int segments);
    static QVector<VizVertex> generateIcoSphere(unsigned int subdivisions);
    static QVector<VizVertex> generateCylinder(unsigned int segments);

    void setFirstFrame();
    void updateWholeFrameData();

    virtual void mousePressEvent(QMouseEvent * event) override final;
    virtual void mouseMoveEvent(QMouseEvent * event) override final;
    virtual void mouseReleaseEvent(QMouseEvent * event) override final;

private:
    QOpenGLBuffer sphereModel_;
    QOpenGLBuffer atomPositions_;
    QOpenGLVertexArrayObject vaoSpheres_;

    QOpenGLBuffer cylinderModel_;
    QOpenGLBuffer cylinderPositions_;
    QOpenGLVertexArrayObject vaoCylinders_;

    QOpenGLVertexArrayObject planeVAO_;

    QOpenGLShaderProgram sphereProgram_;
    QOpenGLShaderProgram cylinderProgram_;
    QOpenGLShaderProgram pickingProgram_;

    QMatrix4x4 projection_;
    QMatrix4x4 modelViewProjection_;
    QMatrix4x4 modelView_;
    QMatrix3x3 modelViewNormal_;

    float fogDensity_;
    float fogContribution_;

    unsigned int sphereVertCount_;
    unsigned int cylinderVertCount_;
    unsigned int sphereCount_;

    std::shared_ptr<Simulation> simulation_;
    frameNumber_t frameNumber_;
    
    bool needVBOUpdate_;
    QVector<VizBallInstance> frameState_, sortedState_;
    QVector<VizLink> linksState_;
    void generateSortedState();

    bool isSelecting_;
    bool isSelectingState_;
    QPoint selectionPoints_[2];
    std::unique_ptr<QOpenGLFramebufferObject> pickingFramebuffer_;
    AtomSelection currentSelection_;
    QVector<bool> selectedBitmap_;

    QRect selectionRect() const;
    QList<unsigned int> pickSpheres();

    QPair<unsigned int, unsigned int> ballQualityParameters_;

    QMap<unsigned int, QString> atomLabels_;

    LabelRenderer labelRenderer_;
    SelectionRectWidget * selectionRectWidget_;

    QColor backgroundColor_;
    QColor labelTextColor_, labelBackgroundColor_;
};

#endif /* VIZWINDOW_HPP */
