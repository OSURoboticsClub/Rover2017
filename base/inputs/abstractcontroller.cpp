#include "abstractcontroller.h"

#include <unistd.h>

#include <algorithm>
#include <QDebug>

#include "serial/serialhandler.h"


AbstractController::AbstractController(QFile *file, QObject *parent)
    : QObject(parent),
      m_file(file),
      m_axisTolerance(0),
      m_currentState(new JoystickState())
{
}

AbstractController::~AbstractController()
{
}

AbstractController::JoystickState::JoystickState()
{
    std::fill(axes, axes + 8, 0.f);
    std::fill(buttons, buttons + 32, false);
}


void AbstractController::emitChanges()
{
    while(read(m_file->handle(), &m_jse, sizeof(m_jse)) > 0){
        if((m_jse.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS){
            m_currentState->axes[m_jse.number] = m_jse.value;
            qDebug("Axis: %i, value: %i", m_jse.number, m_jse.value);
            emitAxisChanges(m_jse.number);
        } else if((m_jse.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON){
            qDebug("Button: %i, value: %i", m_jse.number, m_jse.value);
            m_currentState->buttons[m_jse.number] = m_jse.value;
            emitButtonChanges(m_jse.number);
        }
    }
}



void AbstractController::sendArmMotorPower(qint16 motor1, qint16 motor2, qint16 motor3, qint16 motor4, qint16 motor5){
    quint8 conversionFactor = 255;   //positive or negative tbd
    int8_t m1 = static_cast<int8_t>(motor1 / conversionFactor);
    int8_t m2 = static_cast<int8_t>(motor2 / conversionFactor);
    int8_t m3 = static_cast<int8_t>(motor3 / conversionFactor);
    int8_t m4 = static_cast<int8_t>(motor4 / conversionFactor);
    int8_t m5 = static_cast<int8_t>(motor5 / conversionFactor);

    QMetaObject::invokeMethod(SerialHandler::instance()->p(), "writeArmMotors",
                              Q_ARG( signed char, m1 ),
                              Q_ARG( signed char, m2 ),
                              Q_ARG( signed char, m3 ),
                              Q_ARG( signed char, m4 ),
                              Q_ARG( signed char, m5 ));
}

void AbstractController::sendDriveMotorPower(qint16 left, qint16 right){
    quint8 conversionFactor = 255;
    int8_t l_drive = static_cast<int8_t>(left / conversionFactor);
    int8_t r_drive = static_cast<int8_t>(right / conversionFactor);
    QMetaObject::invokeMethod(SerialHandler::instance()->p(), "writeDriveMotorPower",
                              Q_ARG( signed char, l_drive ),
                              Q_ARG( signed char, l_drive ),
                              Q_ARG( signed char, l_drive ),
                              Q_ARG( signed char, r_drive ),
                              Q_ARG( signed char, r_drive ),
                              Q_ARG( signed char, r_drive ));

}

void AbstractController::sendSwerveDriveState(qint16 swerveValue){
    uint8_t swerve_state = static_cast<uint8_t>(swerveValue);
    qDebug("%i",swerve_state);
    QMetaObject::invokeMethod(SerialHandler::instance()->p(), "writeSwerveDriveState",
                              Q_ARG( unsigned char, swerve_state ));
}

void AbstractController::sendPauseState(qint16 pauseValue){
    double conversionFactor = 1;  // change depending on controller mapping of switch
    uint8_t pauseState =  static_cast<uint8_t>(pauseValue * conversionFactor);
    qDebug("Send Pause: %i",pauseState);
    QMetaObject::invokeMethod(SerialHandler::instance()->p(), "writePause",
                              Q_ARG( unsigned char, pauseState ));
}

void AbstractController::sendSelectCamera(qint16 increment){
    //Increments up if true, and down if false.
    //Loops around to other end if necessary.
    if(increment){
        if(m_camera_state < 6)
            m_camera_state++;
        else
            m_camera_state = 1;
    }
    else{
        if(m_camera_state > 1)
            m_camera_state--;
        else
            m_camera_state = 6;

    }
    //option without looping around
    /*
    if(increment && m_camera_state < 6)
        m_camera_state++;
    else if(!increment && m_camera_state > 1)
        m_camera_state--;
    */
    uint8_t selected_camera = static_cast<uint8_t>(m_camera_state);
    QMetaObject::invokeMethod(SerialHandler::instance()->p(), "writeSelectCamera",
                              Q_ARG( signed char, selected_camera ));
}

//sendCameraCommand
//sendServo?
//sendCallsign?
