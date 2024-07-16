#include <Wire.h>
#include "MPU6050_6Axis_MotionApps20.h"

#define EARTH_GRAVITY_MS2 9.80665  // m/s2
#define DEG_TO_RAD        0.017453292519943295769236907684886
#define RAD_TO_DEG        57.295779513082320876798154814105



class omegaMotion
{
public:
bool mpuReady = false;

private:

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 gg;         // [x, y, z]            gyro sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorInt16 ggWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


public:
    void getData(float[3], Quaternion *);
    Quaternion getQuanterion();
    bool initialize();
    omegaMotion(/* args */);
    ~omegaMotion();
    bool getStatus(){return mpuReady;}

};

Quaternion omegaMotion::getQuanterion()
{



    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 

        // display quaternion values in easy matrix form: w x y z
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
    }
    return q;
}

void omegaMotion::getData(float ypr_deg[3], Quaternion *myq)
{

    if (!dmpReady) return;
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 

        // display quaternion values in easy matrix form: w x y z
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        //memcpy(myq, &q, sizeof(Quaternion));

        mpu.dmpGetGravity(&gravity, &q);
        /*
        // display initial world-frame acceleration, adjusted to remove gravity
        // and rotated based on known orientation from quaternion
        mpu.dmpGetAccel(&aa, fifoBuffer);
        mpu.dmpConvertToWorldFrame(&aaWorld, &aa, &q);

        Serial.print(aaWorld.x * mpu.get_acce_resolution() * EARTH_GRAVITY_MS2);
        Serial.print(aaWorld.y * mpu.get_acce_resolution() * EARTH_GRAVITY_MS2);
        Serial.println(aaWorld.z * mpu.get_acce_resolution() * EARTH_GRAVITY_MS2);

        // display initial world-frame acceleration, adjusted to remove gravity
        // and rotated based on known orientation from quaternion
        mpu.dmpGetGyro(&gg, fifoBuffer);
        mpu.dmpConvertToWorldFrame(&ggWorld, &gg, &q);

        Serial.print(ggWorld.x * mpu.get_gyro_resolution() * DEG_TO_RAD);
        Serial.print("\t");
        Serial.print(ggWorld.y * mpu.get_gyro_resolution() * DEG_TO_RAD);
        Serial.print("\t");
        Serial.println(ggWorld.z * mpu.get_gyro_resolution() * DEG_TO_RAD);
        */
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        ypr_deg[0] = ypr[0] * RAD_TO_DEG;
        ypr_deg[1] = ypr[1] * RAD_TO_DEG;   
        ypr_deg[2] = ypr[2] * RAD_TO_DEG;

    }


}


bool omegaMotion::initialize()
{
    //mpu = MPU6050();
    mpu.initialize();
    mpu.dmpInitialize();
    mpu.setXGyroOffset(-156); mpu.setYGyroOffset(-11); mpu.setZGyroOffset(-14);
    mpu.setXAccelOffset(-3699); mpu.setYAccelOffset(-2519); mpu.setZAccelOffset(1391);
    mpu.CalibrateAccel(6); mpu.CalibrateGyro(6);
    mpu.setDMPEnabled(true);
    dmpReady = true;

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {

        mpu.CalibrateAccel(6);
        mpu.CalibrateGyro(6);
        //mpu.PrintActiveOffsets();
        mpu.setDMPEnabled(true);

        mpuIntStatus = mpu.getIntStatus();

        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
        mpuReady = true;
        return true;
    }
    else
    {
        return false; // Setup failed
    }


}

omegaMotion::omegaMotion(/* args */)
{
}

omegaMotion::~omegaMotion()
{
}
