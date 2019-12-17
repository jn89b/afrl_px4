#include "sensor_simulator.h"


SensorSimulator::SensorSimulator(std::shared_ptr<Ekf> ekf):
_ekf{ekf},
_imu(ekf),
_mag(ekf),
_baro(ekf),
_gps(ekf),
_flow(ekf),
_rng(ekf)
{
	setSensorDataToDefault();
	setSensorRateToDefault();
	startBasicSensor();
}

SensorSimulator::~SensorSimulator()
{

}

void SensorSimulator::setSensorDataToDefault()
{
	_imu.setRateHz(250);
	_mag.setRateHz(80);
	_baro.setRateHz(80);
	_gps.setRateHz(5);
	_flow.setRateHz(50);
	_rng.setRateHz(30);
}
void SensorSimulator::setSensorRateToDefault()
{
	_imu.setData(Vector3f{0.0f,0.0f,-CONSTANTS_ONE_G},
		     Vector3f{0.0f,0.0f,0.0f});
	_mag.setData(Vector3f{0.2f, 0.0f, 0.4f});
	_baro.setData(122.2f);
	_gps.setData(_gps.getDefaultGpsData());
	_flow.setData(_flow.dataAtRest());
	_rng.setData(0.2f, 100);
}
void SensorSimulator::startBasicSensor()
{
	_imu.start();
	_mag.start();
	_baro.start();
}

void SensorSimulator::runSeconds(float duration_seconds)
{
	runMicroseconds( uint32_t(duration_seconds * 1e6f) );
}

void SensorSimulator::runMicroseconds(uint32_t duration)
{
	// simulate in 1000us steps
	const uint32_t start_time = _time;

	for(;_time < start_time + duration; _time+=1000)
	{
		_imu.update(_time);
		_mag.update(_time);
		_baro.update(_time);
		_gps.update(_time);
		_flow.update(_time);
		_rng.update(_time);

		_ekf->update();
	}
}

void SensorSimulator::setImuBias(Vector3f accel_bias, Vector3f gyro_bias)
{
	_imu.setData(Vector3f{0.0f,0.0f,-CONSTANTS_ONE_G} + accel_bias,
		     Vector3f{0.0f,0.0f,0.0f} + gyro_bias);
}
