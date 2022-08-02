#pragma once

#include "SupervisorControl.hpp"
#include <px4_platform_common/module_params.h>
#include <uORB/topics/parameter_update.h>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>

// public ModuleBase<AngularVelocityController>
class SupervisorControl : public ModuleParams
{
public:
	SupervisorControl();
	~SupervisorControl() override;
private:

	/**
	 * Check for parameter changes and update them if needed.
	 */
	void parameters_update();

	DEFINE_PARAMETERS(
		(ParamFloat<px4::params::MAX_ROLL>) _param_max_roll,   /**< example parameter */
		(ParamFloat<px4::params::MAX_PITCH>) _param_max_pitch  /**< another parameter */
	)
	
	// Subscriptions
	uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};
	
};
