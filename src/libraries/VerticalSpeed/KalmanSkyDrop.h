/**
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <robin.lilja@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. - Robin Lilja
 *
 * @file altitude_kf.h
 * @author Robin Lilja
 * @date 23 Jul 2015
 */

#ifndef _ALTITUDE_KF_H_
#define _ALTITUDE_KF_H_

#include <Arduino.h>
//#include "../common.h"

/**
 * A linear Kalman filter estimator of altitude and vertical velocity.
 */
class KalmanFilter {

public:

	/**
	 * Constructor.
	 * @param Q_accel covariance of acceleration input signal (σ^2).
	 * @param R_altitude covariance of the altitude measurement (σ^2).
	 */
	void Configure(float Q_accel, float R_altitude, float alt);
	void Update_Propagate(float altitude, float accel, float * h, float * v);
	void Reset(float alt);
	void Debug();

	/**
	 * Propagate the state.
	 * @param acceleration vertical acceleration in Earth frame (positive in the zenith direction) [m/s^2].
	 * @param dt update/sampling period [s].
	 */
	void propagate(float acceleration);

	/**
	 * State correction update. Use this method if you use multiple sensors measuring the altitude.
	 * @param altitude measurement of altitude in Earth frame (positive in the zenith direction) [m].
	 */
	void update(float altitude);

	/**
	 * Estimated vertical height or altitude in Earth frame (positive in the zenith direction) [m].
	 */
	float h;

	/**
	 * Estimated vertical velocity (positive in the zenith direction) [m/s].
	 */
	float v;

	/**
	 * Accelerometer covariance.
	 */
	float Q_accel;

	/**
	 * Altitude measurement covariance.
	 */
	float R_altitude;

private:

	/**
	 * Predicted covariance matrix 'P'.
	 */
	float P[2][2] =
	{
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f }
	};

};


///////////////////////////////////////////////////////////////////////////
//

#include "Task.h"
#include "Sensor_MS5611.h"

class KalmanSkyDrop : public Task, public KalmanFilter
{
public:
	KalmanSkyDrop(Sensor_MS5611 & baro);

public:
	int						begin(float Q_accel = 30.0f, float R_altitude = 4.0f);
	void					end();
	
	int						available();
	void					flush();
	
	void					update();
	
	float					getPressure();
	float					getTemperature();
	float					getAltitude2();
	float					getAltitude();
	float					getCalibratedAltitude();
	float					getVelocity();
	
	uint32_t				getTimestamp();
	
	void					calibrateAltitude(float altitudeRef);
	void					calculateSeaLevel(float altitude);
	
protected:
	static void IRAM_ATTR 	TimerProc();
	
	static KalmanSkyDrop *	mActiveVario;
	
	void					TaskProc();

private:
	// timestamp
	uint32_t				t_;
	
	// kalman
	float					altitude;
	float					vario;
	// barometer altitude
	float					baroAltitude;
	// altitude calibration
	float					altitudeDrift;
	
	//
	int						varioUpdated;
	
	//
	Sensor_MS5611 &			baro;
	float					seaLevel;
	
protected:
	hw_timer_t *			mTimer;
	SemaphoreHandle_t		mSemaphore;
	portMUX_TYPE			mMux;
};


// inline members
inline float KalmanSkyDrop::getPressure()
	{ return baro.getPressure(); }

inline float KalmanSkyDrop::getTemperature()
	{ return baro.getTemperature(); }

inline float KalmanSkyDrop::getAltitude2()
	{ return altitude; }

inline float KalmanSkyDrop::getAltitude()
	{ return baroAltitude; }

inline float KalmanSkyDrop::getCalibratedAltitude()
	{ return (getAltitude() + altitudeDrift); }

inline float KalmanSkyDrop::getVelocity()
	{ return vario; }

inline uint32_t KalmanSkyDrop::getTimestamp()
	{ return t_; }

inline void KalmanSkyDrop::calibrateAltitude(float altitudeRef)
	{ altitudeDrift = altitudeRef - getAltitude(); }


#endif /* _ALTITUDE_KF_H_ */
