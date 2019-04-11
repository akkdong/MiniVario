// KalmanVario.cpp
//

#include "KalmanVario.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


////////////////////////////////////////////////////////////////////////////////////
// class KalmanVario

KalmanVario::KalmanVario(Sensor_MS5611 & _baro) 
	: Task("Kalman", 1024, 2)
	, baro(_baro)
	, mTimer(NULL)
	, mSemaphore(xSemaphoreCreateBinary())
	, mMux(portMUX_INITIALIZER_UNLOCKED)
{
}

KalmanVario * KalmanVario::mActiveVario = NULL;


int KalmanVario::begin(float zVariance, float zAccelVariance, float zAccelBiasVariance)
{
	//
	baro.begin();
	
	// read dummy data 10 times
	// it may be stabilize data...
	baro.startConvert();
	for (int i = 0; i < 10; i++)
	{
		delay(10);
		baro.convertNext();
		delay(10);
		baro.convertNext();

		float p, t;
		baro.read(&p, &t);
		Serial.print("Pressure = "); Serial.print(p); Serial.print("Temperature = "); Serial.println(t);
	}
	
	//
	float prs, va;

	// init values
	zAccelVariance_ = zAccelVariance;
    zAccelBiasVariance_ = zAccelBiasVariance;
	zVariance_ = zVariance;

	z_ = Sensor_MS5611::getAltitude(baro.getPressure());
	v_ = 0.0f; // vInitial;
	aBias_ = 0.0f; // aBiasInitial;
	Pzz_ = 1.0f;
	Pzv_ = 0.0f;
	Pza_ = 0.0f;
	
	Pvz_ = 0.0f;
	Pvv_ = 1.0f;
	Pva_ = 0.0f;
	
	Paz_ = 0.0f;
	Pav_ = 0.0;
	Paa_ = 100000.0f;	
	
	baroAltitude = z_;
	altitudeDrift = 0.0;
	
	//
	varioUpdated = false;
	t_ = millis();
	
	//
	Task::begin();
	
	//
	mActiveVario = this;
	mTimer = timerBegin(0, 80, true); // ESP32 Counter: 80 MHz, Prescaler: 80 --> 1MHz timer
	timerAttachInterrupt(mTimer, TimerProc, true);
	timerAlarmWrite(mTimer, 1000000 / 100, true); // 100Hz -> alarm very 10msec, 118Hz -> 8.4746 msec  :  the measure period need to be greater than 8.22 msec
	timerAlarmEnable(mTimer);
	
	//
	baro.startConvert();
}

void IRAM_ATTR KalmanVario::TimerProc() 
{
	if (KalmanVario::mActiveVario != NULL)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(KalmanVario::mActiveVario->mSemaphore, &xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken == pdTRUE) 
			portYIELD_FROM_ISR(); // this wakes up task immediately instead of on next FreeRTOS tick
	}
}

void KalmanVario::TaskProc() 
{
	while (1)
	{
		//
		xSemaphoreTake(mSemaphore, portMAX_DELAY);
		
		//
		baro.convertNext();
		
		if (baro.available())
		{
			// update vertical velocity
			update();
		}
	}
}

void KalmanVario::end()
{
	//
	if (mTimer != NULL)
	{
		timerAlarmDisable(mTimer);
		timerDetachInterrupt(mTimer);
		timerEnd(mTimer);
		
		mTimer = NULL;
		mActiveVario = NULL;
	}
	
	//
	Task::end();
	
	//
	baro.end();
}

int	KalmanVario::available()
{
	return varioUpdated;
}

void KalmanVario::flush()
{
	varioUpdated = false;
}

void KalmanVario::update()
{
	//if (baro.available())
	{
		// read pressure & vertical acceleration
		float prs, va = 0;

		baro.read(&prs, 0);
		
		//
		float altitude = Sensor_MS5611::getAltitude(prs);
		baroAltitude = baroAltitude * 0.9 + altitude * 0.1;

		// delta time
		unsigned long deltaTime = 20; // millis() - t_;
		float dt = ((float)deltaTime) / 1000.0;
		t_ = millis();

		//
		// prediction
		//
		float accel = va - aBias_;
		v_ += accel * dt;
		z_ += v_ * dt;

		// Predict State Covariance matrix
		float t00,t01,t02;
		float t10,t11,t12;
		float t20,t21,t22;
		
		float dt2div2 = dt * dt / 2.0f;
		float dt3div2 = dt2div2 * dt;
		float dt4div4 = dt2div2 * dt2div2;
		
		t00 = Pzz_ + dt * Pvz_ - dt2div2 * Paz_;
		t01 = Pzv_ + dt * Pvv_ - dt2div2 * Pav_;
		t02 = Pza_ + dt * Pva_ - dt2div2 * Paa_;

		t10 = Pvz_ - dt * Paz_;
		t11 = Pvv_ - dt * Pav_;
		t12 = Pva_ - dt * Paa_;

		t20 = Paz_;
		t21 = Pav_;
		t22 = Paa_;
		
		Pzz_ = t00 + dt * t01 - dt2div2 * t02;
		Pzv_ = t01 - dt * t02;
		Pza_ = t02;
		
		Pvz_ = t10 + dt * t11 - dt2div2 * t12;
		Pvv_ = t11 - dt * t12;
		Pva_ = t12;
		
		Paz_ = t20 + dt * t21 - dt2div2 * t22;
		Pav_ = t21 - dt * t22;
		Paa_ = t22;

		Pzz_ += dt4div4 * zAccelVariance_;
		Pzv_ += dt3div2 * zAccelVariance_;

		Pvz_ += dt3div2 * zAccelVariance_;
		Pvv_ += dt * dt * zAccelVariance_;

		Paa_ += zAccelBiasVariance_;

		// Error
		float innov = altitude - z_; 
		float sInv = 1.0f / (Pzz_ + zVariance_);  

		// Kalman gains
		float kz = Pzz_ * sInv;  
		float kv = Pvz_ * sInv;
		float ka = Paz_ * sInv;

		// Update state 
		z_ += kz * innov;
		v_ += kv * innov;
		aBias_ += ka * innov;
		
		//*pZ = z_;
		//*pV = v_;

		// Update state covariance matrix
		Pzz_ -= kz * Pzz_;
		Pzv_ -= kz * Pzv_;
		Pza_ -= kz * Pza_;
		
		Pvz_ -= kv * Pzz_;
		Pvv_ -= kv * Pzv_;
		Pva_ -= kv * Pza_;
		
		Paz_ -= ka * Pzz_;
		Pav_ -= ka * Pzv_;
		Paa_ -= ka * Pza_;
		
		//
		varioUpdated = true;		
	}
}
