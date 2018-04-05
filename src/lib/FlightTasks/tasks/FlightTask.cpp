#include "FlightTask.hpp"
#include <mathlib/mathlib.h>

constexpr uint64_t FlightTask::_timeout;


bool FlightTask::initializeSubscriptions(SubscriptionArray &subscription_array)
{
	if (!subscription_array.get(ORB_ID(vehicle_local_position), _sub_vehicle_local_position)) {
		return false;
	}

	return true;
}

bool FlightTask::activate()
{
	_time_stamp_activate = hrt_absolute_time();
	return true;
}

bool FlightTask::updateInitialize()
{
	_time_stamp_current = hrt_absolute_time();
	_time = (_time_stamp_current - _time_stamp_activate) / 1e6f;
	_deltatime  = math::min((_time_stamp_current - _time_stamp_last), _timeout) / 1e6f;
	_time_stamp_last = _time_stamp_current;
	return _evaluateVehiclePosition();
}

bool FlightTask::_evaluateVehiclePosition()
{
	if ((_time_stamp_current - _sub_vehicle_local_position->get().timestamp) < _timeout) {
		_position = matrix::Vector3f(&_sub_vehicle_local_position->get().x);
		_velocity = matrix::Vector3f(&_sub_vehicle_local_position->get().vx);
		_yaw = _sub_vehicle_local_position->get().yaw;
		return true;

	} else {
		_velocity.zero(); /* default velocity is all zero */
		return false;
	}
}
