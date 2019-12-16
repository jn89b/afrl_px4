#include "mag.h"

namespace sensor_simulator
{
namespace sensor
{

Mag::Mag(std::shared_ptr<Ekf> ekf):Sensor(ekf)
{
}

Mag::~Mag()
{
}

void Mag::send(uint32_t time)
{
	float mag[3];
	_mag_data.copyTo(mag);
	_ekf->setMagData(time,mag);
	_time_last_data_sent = time;
}

void Mag::setData(const Vector3f& mag)
{
	_mag_data = mag;
}

} // namespace sensor
} // namespace sensor_simulator::sensor
