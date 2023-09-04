// KalmanVario.h
//

#ifndef __KALMANVARIO_H__
#define __KALMANVARIO_H__

#include "Task.h"
#include "Sensor_MS5611.h"


////////////////////////////////////////////////////////////////////////////////////
// class KalmanVario

class KalmanVario : public Task
{
public:
	KalmanVario(Sensor_MS5611 & baro);
	
public:
	int						begin(float zVariance = 400.0, float zAccelVariance = 1000.0, float zAccelBiasVariance = 1.0);
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
	
	static KalmanVario *	mActiveVario;
	
	void					TaskProc();
	
private:
	void					init();
	
private:
	// State being tracked
	float					z_;  // position
	float					v_;  // velocity
	float					aBias_;  // acceleration

	// 3x3 State Covariance matrix
	float					Pzz_;
	float					Pzv_;
	float					Pza_;
	float					Pvz_;
	float					Pvv_;
	float					Pva_;
	float					Paz_;
	float					Pav_;
	float					Paa_;
	float					zAccelBiasVariance_; // assumed fixed.
	float					zAccelVariance_;  // dynamic acceleration variance
	float					zVariance_; //  z measurement noise variance fixed
	
	// timestamp
	uint32_t				t_;
	
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
inline float KalmanVario::getPressure()
	{ return baro.getPressure(); }

inline float KalmanVario::getTemperature()
	{ return baro.getTemperature(); }

inline float KalmanVario::getAltitude2()
	{ return z_; }

inline float KalmanVario::getAltitude()
	{ return baroAltitude; }
//	{ return MS5611::getAltitude(baro.getPressure()); }

inline float KalmanVario::getCalibratedAltitude()
//	{ return (z_ + altitudeDrift); }
	{ return (getAltitude() + altitudeDrift); }

inline float KalmanVario::getVelocity()
	{ return v_; }

inline uint32_t KalmanVario::getTimestamp()
	{ return t_; }

inline void KalmanVario::calibrateAltitude(float altitudeRef)
	{ altitudeDrift = altitudeRef - getAltitude(); }


#endif // __KALMANVARIO_H__
