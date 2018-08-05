#ifndef MEDIAPANEL_H
#define MEDIAPANEL_H

#include <QWidget>
#include <QTimer>
#include <QTime>

namespace Ui {
class MediaPanel;
class MainWindow;
}

class Session;
class Camera;

class MediaPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPanel(Session* s, Ui::MainWindow* ui__, QWidget *parent = nullptr);
    ~MediaPanel();

    void recordActiveObjects();
    void autokeying(bool checked);

    void playForwards(bool checked);
    void playBackwards(bool checked);

    void goToStart();
    void goToEnd();

    void goToNextFrame();
    void goToPreviousFrame();

    void setFPS(int fps);
    void setDocumentTime(int time);
    void setMinimumTime(int time);
    void setMaximumTime(int time);
    void setPreviewMinTime(int time);
    void setPreviewMaxTime(int time);
    void setLastFrame(int time);

    void changeCamera(Camera* camera);

private:
    Ui::MediaPanel *ui;
    Ui::MainWindow *ui_;
    Session* session;

    QTimer timer;
    QTime time;

    int direction;
};

#endif // MEDIAPANEL_H
