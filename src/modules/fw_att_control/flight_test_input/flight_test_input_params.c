#include <px4_platform_common/px4_config.h>
#include <parameters/param.h>
/**
 * @file flight_test_input_params.c
 */

/**
 * Test Input Mode
 *
 * 0 for injecting frequency sweeps, 1 for doublet
 *
 * @min 0
 * @max 1
 * @group Flight Test Input
 */
PARAM_DEFINE_INT32(FTI_MODE, 0);

/**
 * Test input Enable/Disable
 *
 * Frequency sweep enable flag
 *
 * @min 0
 * @max 1
 * @group Flight Test Input
 */
PARAM_DEFINE_INT32(FTI_ENABLE, 0);

// /**
//  * Test input injection point
//  *
//  * 0 - none
//  * ACTUATORS: 1 - roll, 2 - pitch, 3 - yaw
//  * RATE CMD: 4 - roll, 5 - pitch, 6 - yaw
//  * ATTITUDE CMD: 7 - roll, 8 - pitch, 9 - yaw
//  *
//  * @unit enum
//  * @min 0
//  * @max 9
//  * @value 0 None
//  * @value 1 Actuators: roll
//  * @value 2 Actuators: pitch
//  * @value 3 Actuators: yaw
//  * @value 4 Rate cmd: roll
//  * @value 5 Rate cmd: pitch
//  * @value 6 Rate cmd: yaw
//  * @value 7 Attitude cmd: roll
//  * @value 8 Attitude cmd: pitch
//  * @value 9 Attitude cmd: yaw
//  * @min 0
//  * @max 9
//  * @group Flight Test Input
//  */
PARAM_DEFINE_INT32(FTI_INJXN_POINT, 0);

/**
 * Test input frequency sweep duration
 *
 * Total length of frequency sweep
 *
 * @unit s
 * @min 1.0
 * @max 200.0
 * @decimal 2
 * @group Flight Test Input
 */
PARAM_DEFINE_FLOAT(FTI_FS_DURATION, 10.0f);

/**
 * Test input frequency sweep starting frequency
 *
 * Start frequency of sweep
 *
 * @unit Hz
 * @min 0.01
 * @max 50.0
 * @decimal 2
 * @group Flight Test Input
 */
PARAM_DEFINE_FLOAT(FTI_FS_FRQ_BEGIN, 0.1f);

/**
 * Test input frequency sweep ending frequency
 *
 * End frequency of sweep
 *
 * @unit Hz
 * @min 0.01
 * @max 100.0
 * @decimal 2
 * @group Flight Test Input
 */
PARAM_DEFINE_FLOAT(FTI_FS_FRQ_END, 3.0f);

/**
 * Test input frequency sweep starting amplitude
 *
 * Start amplitude of sweep
 * @decimal 2
 * @group Flight Test Input

 */
PARAM_DEFINE_FLOAT(FTI_FS_AMP_BEGIN, 0.0f);

/**
 * Test input frequency sweep ending amplitude
 *
 * End amplitude of sweep
 * @decimal 2
 * @group Flight Test Input
 */
PARAM_DEFINE_FLOAT(FTI_FS_AMP_END, 0.0f);

/**
 * Test input frequency sweep ramp
 * @group Flight Test Input
 */
PARAM_DEFINE_FLOAT(FTI_FS_FRQ_RAMP, 3.0f);

/**
 * Test input for FTI Loop gain
 *
 * Set Loop gain for FTI.
 * 
 * @min 0.5
 * @max 2
 * @decimal 2
 * @group Flight Test Input
 */
PARAM_DEFINE_FLOAT(FTI_LOOP_GAIN, 1.0f);

// /**
//  * Test input doublet pulse length
//  *
//  * Length of each doublet pulse in seconds
//  *
//  * @unit s
//  * @min 0.0
//  * @max 25.0
//  * @group Flight Test Input
//  */
// PARAM_DEFINE_FLOAT(FTI_PULSE_LEN, 3.0f);

// /**
//  * Test input doublet pulse amplitude
//  *
//  * Amplitude of each doublet pulse
//  *
//  * @unit deg/%
//  * @min 0.0
//  * @max 250.0
//  * @group Flight Test Input
//  */
// PARAM_DEFINE_FLOAT(FTI_PULSE_AMP, 0.0f);
