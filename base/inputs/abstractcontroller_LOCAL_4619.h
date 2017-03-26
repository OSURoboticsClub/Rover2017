#ifndef ABSTRACTCONTROLLER_H
#define ABSTRACTCONTROLLER_H

#include <QObject>
#include <QFile>
#include <linux/joystick.h>

class AbstractController : public QObject
{
    Q_OBJECT
public:
    explicit AbstractController(QFile *file, QObject *parent = 0);
    ~AbstractController();
    virtual void emitChanges();

signals:



protected:
    virtual void emitAxisChanges(int axisIndex);
    virtual void emitButtonChanges(int buttonIndex);
    void sendArmMotorPower(double m1, double m2, double m3, double m4, double m5);


    QFile *m_file;
    struct js_event m_jse;

    struct JoystickState {
        JoystickState();
        float axes[8];
        bool  buttons[32];
    };
    JoystickState *m_currentState;
    float m_axisTolerance;
    int m_mode = 0;

};

#endif // ABSTRACTCONTROLLER_H
