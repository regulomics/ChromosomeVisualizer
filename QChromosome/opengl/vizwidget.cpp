#include "vizwidget.h"

VizWidget::VizWidget(QWidget *parent)
    : Selection(parent)
{
    setAcceptDrops(true);
}

VizWidget::~VizWidget()
{

}

#include "camera.h"
#include "viewport.h"
#include "session.h"

void VizWidget::initializeGL()
{
    initializeOpenGLFunctions();

    Q_ASSERT(vaoSpheres_.create());

    Q_ASSERT(atomPositions_.create());
    atomPositions_.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    vaoSpheres_.bind();
    atomPositions_.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, position)
    );

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(
        1,
        1,
        GL_INT,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, flags)
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, size)
    );

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, radius)
    );

    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(
        4,
        1,
        GL_INT,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, material)
    );

    atomPositions_.release();
    vaoSpheres_.release();

    Q_ASSERT(vaoCameras_.create());

    Q_ASSERT(cameraPositions_.create());
    cameraPositions_.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    vaoCameras_.bind();
    cameraPositions_.bind();

    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(
            i,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(VizCameraInstance),
            (void*)(offsetof(VizCameraInstance, modelView) + i * sizeof(QVector4D))
        );
    }

    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(i + 4);
        glVertexAttribPointer(
            i + 4,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(VizCameraInstance),
            (void*)(offsetof(VizCameraInstance, projection) + i * sizeof(QVector4D))
        );
    }

    glEnableVertexAttribArray(8);
    glVertexAttribIPointer(
        8,
        1,
        GL_INT,
        sizeof(VizCameraInstance),
        (void*)offsetof(VizCameraInstance, flags)
    );

    cameraPositions_.release();
    vaoCameras_.release();

    Q_ASSERT(vaoLabels_.create());

    vaoLabels_.bind();
    atomPositions_.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, position)
    );

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(
        1,
        1,
        GL_INT,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, flags)
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, size)
    );

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(
        3,
        4,
        GL_INT,
        sizeof(VizBallInstance),
        (void*)offsetof(VizBallInstance, label)
    );

    atomPositions_.release();
    vaoLabels_.release();

    // Shaders
    Q_ASSERT(sphereProgram_.create());
    sphereProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/sphere/vertex.glsl");
    sphereProgram_.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/sphere/geometry.glsl");
    sphereProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/sphere/fragment.glsl");
    Q_ASSERT(sphereProgram_.link());

    Q_ASSERT(cylinderProgram_.create());
    cylinderProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/cylinder/vertex.glsl");
    cylinderProgram_.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/cylinder/geometry.glsl");
    cylinderProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/cylinder/fragment.glsl");
    Q_ASSERT(cylinderProgram_.link());

    Q_ASSERT(cameraProgram_.create());
    cameraProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/camera/vertex.glsl");
    cameraProgram_.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/camera/geometry.glsl");
    cameraProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/camera/fragment.glsl");
    Q_ASSERT(cameraProgram_.link());

    Q_ASSERT(labelsProgram_.create());
    labelsProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/labels/vertex.glsl");
    labelsProgram_.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/labels/geometry.glsl");
    labelsProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/labels/fragment.glsl");
    Q_ASSERT(labelsProgram_.link());

    Q_ASSERT(pickingProgram_.create());
    pickingProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/sphere/vertex.glsl");
    pickingProgram_.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/sphere/geometry.glsl");
    pickingProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/picking/fragment.glsl");
    Q_ASSERT(pickingProgram_.link());

    Q_ASSERT(materials_.create());
    materials_.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    glGenBuffers(2, buffers);

    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffers[0]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(camera_data_t), &session->cameraUniformBuffer, GL_DYNAMIC_DRAW);

        const GLuint binding_point_index = 0;
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, buffers[0]);

        unsigned int block_index;

        block_index = glGetUniformBlockIndex(sphereProgram_.programId(), "shader_data");
        glUniformBlockBinding(sphereProgram_.programId(), block_index, binding_point_index);

        block_index = glGetUniformBlockIndex(cylinderProgram_.programId(), "shader_data");
        glUniformBlockBinding(cylinderProgram_.programId(), block_index, binding_point_index);

        block_index = glGetUniformBlockIndex(cameraProgram_.programId(), "shader_data");
        glUniformBlockBinding(cameraProgram_.programId(), block_index, binding_point_index);

        block_index = glGetUniformBlockIndex(labelsProgram_.programId(), "shader_data");
        glUniformBlockBinding(labelsProgram_.programId(), block_index, binding_point_index);

        block_index = glGetUniformBlockIndex(pickingProgram_.programId(), "shader_data");
        glUniformBlockBinding(pickingProgram_.programId(), block_index, binding_point_index);
    }

    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffers[1]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(viewport_data_t), &Viewport::getBuffer(), GL_DYNAMIC_DRAW);

        const GLuint binding_point_index = 1;
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, buffers[1]);

        unsigned int block_index;

        block_index = glGetUniformBlockIndex(sphereProgram_.programId(), "viewport_data");
        glUniformBlockBinding(sphereProgram_.programId(), block_index, binding_point_index);

        block_index = glGetUniformBlockIndex(cylinderProgram_.programId(), "viewport_data");
        glUniformBlockBinding(cylinderProgram_.programId(), block_index, binding_point_index);
    }

    {
        const GLuint binding_point_index = 2;
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, materials_.bufferId());

        unsigned int block_index;

        block_index = glGetUniformBlockIndex(sphereProgram_.programId(), "material_data");
        glUniformBlockBinding(sphereProgram_.programId(), block_index, binding_point_index);

        block_index = glGetUniformBlockIndex(cylinderProgram_.programId(), "material_data");
        glUniformBlockBinding(cylinderProgram_.programId(), block_index, binding_point_index);
    }

    glGenFramebuffers(1, picking);
    glGenTextures(2, texture);
}

void VizWidget::paintGL()
{
    // Ensure taht buffers are up to date
    allocate();

    // Enable culling
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    auto& buffer = Viewport::getBuffer();
    QColor color = buffer.ucBackgroundColor;

    if (buffer.ubEnableFog)
    {
        float p = buffer.ufFogStrength;
        float q = 1. - p;

        QColor fog = buffer.ucFogColor;

        color.setRedF(p * fog.redF() + q * color.redF());
        color.setGreenF(p * fog.greenF() + q * color.greenF());
        color.setBlueF(p * fog.blueF() + q * color.blueF());
    }

    glClearColor(color.redF(), color.greenF(), color.blueF(), 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // If there are no spheres, my driver crashes
    if (!session->atomBuffer.empty())
    {
        vaoSpheres_.bind();
        cylinderProgram_.bind();

        glMultiDrawArrays(GL_LINE_STRIP, session->chainBuffer[0].data(), session->chainBuffer[1].data(), session->chainBuffer[0].count());

        cylinderProgram_.release();
        sphereProgram_.bind();

        glDrawArrays(GL_POINTS, 0, session->atomBuffer.count());

        sphereProgram_.release();
        vaoSpheres_.release();
    }

    if (session->cameraBuffer.count() > 1)
    {
        vaoCameras_.bind();
        cameraProgram_.bind();

        glDrawArrays(GL_POINTS, 1, session->cameraBuffer.count() - 1);

        cameraProgram_.release();
        vaoCameras_.release();
    }

    // If there are no spheres, my driver crashes
    if (!session->atomBuffer.empty())
    {
        vaoLabels_.bind();
        labelsProgram_.bind();

        labelsProgram_.setUniformValue("uvTextureSize", QSizeF(session->labelAtlas.size()));
        labelsProgram_.setUniformValue("SampleTexture", 0);

        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, session->labelAtlas.textureId());

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDrawArrays(GL_POINTS, 0, session->atomBuffer.count());

        labelsProgram_.release();
        vaoLabels_.release();

        glDisable(GL_BLEND);
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

QPersistentModelIndex VizWidget::pick(const QPoint &pos)
{
    pickSpheres();

    auto color = image.pixel(pos);

    return color != 0xFFFFFFFFU ? session->indices[color] : QPersistentModelIndex();
}

void VizWidget::setSession(Session *s)
{
    session = s;
}

void VizWidget::allocate()
{
    session->cameraBuffer.allocate(cameraPositions_);
    session->atomBuffer.allocate(atomPositions_);
    session->labelAtlas.allocate();

    if (Viewport::modified)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffers[1]);
        GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
        memcpy(p, &Viewport::getBuffer(), sizeof(viewport_data_t));
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        Viewport::modified = false;
    }

    Material::getBuffer().allocate(materials_);

    glBindBuffer(GL_UNIFORM_BUFFER, buffers[0]);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(p, session->cameraUniformBuffer, sizeof(camera_data_t));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void VizWidget::pickSpheres()
{
    makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, picking[0]);

    // Ensure taht buffers are up to date
    allocate();

    // Enable culling
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    int color = -1;
    glClearColor(reinterpret_cast<GLfloat&>(color), 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // If there are no spheres, my driver crashes
    if (!session->atomBuffer.empty())
    {
        vaoSpheres_.bind();
        pickingProgram_.bind();

        glDrawArrays(GL_POINTS, 0, session->atomBuffer.count());

        pickingProgram_.release();
        vaoSpheres_.release();
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    auto array = new uchar[sizeof(GLint) * width() * height()];

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, width(), height(), GL_RED_INTEGER, GL_INT, array);

    image = QImage(array, width(), height(), QImage::Format_ARGB32).mirrored();

    delete[] array;
}

void VizWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (qobject_cast<Material*>(event->source()))
    {
        event->acceptProposedAction();
        pickSpheres();
    }
}

void VizWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (image.pixel(event->pos()) != 0xFFFFFFFF)
        event->acceptProposedAction();
    else
        event->ignore();
}

void VizWidget::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    auto index = session->indices[image.pixel(event->pos())];
    auto material = qobject_cast<Material*>(event->source());
    auto model_ = session->simulation->getModel();
    auto selectionModel_ = session->treeView->selectionModel();

    if (selectionModel_ && selectionModel_->isSelected(index))
        for (auto i : selectionModel_->selectedRows())
            model_->setMaterial(i, material);
    else
        model_->setMaterial(index, material);
}

void VizWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Selection::mouseReleaseEvent(event);

    pickSpheres();

    auto mask = QPainterPath();
    mask.addRect(rect());
    mask = mask.subtracted(getSelectionPath());

    QPainter(&image).fillPath(mask, QColor(-1));

    auto first = (int*)image.constBits();
    auto last = first + width() * height();

    std::sort(first, last);
    last = std::unique(first, last);

    if (*first == -1)
        first++;

    QMap<QPersistentModelIndex, QVector<int>> tmp;

    for (int* i = first; i != last; i++)
    {
        auto& item = session->indices[*i];
        tmp[item.parent()].append(item.row());
    }

    QItemSelection selected;

    for (auto i = tmp.begin(); i != tmp.end(); i++)
    {
        auto& parent = i.key();
        auto& list = i.value();

        qSort(list.begin(), list.end());

        int first = 0;
        int last = -1;

        for (auto j : list)
        {
            if (j != last + 1)
            {
                selected.select(parent.child(first, 0), parent.child(last, 0));
                first = j;
            }

            last = j;
        }
        selected.select(parent.child(first, 0), parent.child(last, 0));
    }

    QItemSelectionModel::SelectionFlags flags;

    auto m = event->modifiers();

    const bool ctrl = m & Qt::ControlModifier;
    const bool shift = m & Qt::ShiftModifier;

    if (!(ctrl || shift))
        flags.setFlag(QItemSelectionModel::Clear);

    flags.setFlag(ctrl ? QItemSelectionModel::Deselect : QItemSelectionModel::Select);

    emit selectionChanged(selected, flags);
}

void VizWidget::resizeEvent(QResizeEvent* event)
{
    QOpenGLWidget::resizeEvent(event);

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width(), height(), 0, GL_RED_INTEGER, GL_INT, 0);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width(), height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, picking[0]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[0], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture[1], 0);
}