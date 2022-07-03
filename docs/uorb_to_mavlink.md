# How to send Custom UORB topics to MAVLINK protocol

This example assumes you already made a UORB topic ready to be published, currently using PX4 Version 1.13 for this example

## Generating MAVLINK 2.0 Messages
In `src/modules/mavlink/message_definitions/v1.0` add your custom message in this repo , for this application we added flight_test_input.xml `src/modules/mavlink/mavlink/message_definitions/v1.0/keyboard_command.xml`

Open up a terminal into `src/modules/pymavlink/` and create a folder repo related to your .xml file, for our case it was flight_test_input, in this terminal run the following command:

```
python3 mavgenerate.py
```
When this GUI pops up for the input select the your xml file `src/modules/mavlink/mavlink/message_definitions/v1.0/keyboard_command.xml` and for your ouput select the folder repo you just made, for our case it was `src/modules/mavlink/mavlink/pymavlink/flight_test_input`

Doing this will generate a bunch of header files and 3 repos consisting of *common*, *minimal*, and *xml_name* the only folder you will care about inside is the the xml_name one. For our case it was `src/modules/mavlink/mavlink/pymavlink/flight_test_input/flight_test_input` (I deleted the other files and folders to save space for the push of this repo)


## Copy Pasta to streaming and making the header file
In `src/modules/mavlink/streams` copy paste that folder repo with the xml name, generated from the mavgenerate.py into this directory

Now create a your_xml_name.hpp file, for our application it was `src/modules/mavlink/streams/FLIGHT_TEST_INPUT.hpp`

Follow this structure but change the message as you see fit

```cpp
/****************************************************************************
 *
 *   Copyright (c) 2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef FLIGHT_TEST_INPUT_HPP
#define FLIGHT_TEST_INPUT_HPP

#include <uORB/topics/flight_test_input.h>
// #include "flight_test_input/mavlink.h"
// include "flight_test_input/flight_test_input.h"

//THIS IS IMPORTANT//
#include "flight_test_input/mavlink_msg_flight_test_input.h"

class MavlinkStreamFlightTestInput : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamFlightTestInput(mavlink); }

	static constexpr const char *get_name_static() { return "FLIGHT_TEST_INPUT"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_FLIGHT_TEST_INPUT;}

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }

	//change _att_sub
	unsigned get_size() override
	{
		return _flight_test_input_sub.advertised() ? MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES : 0;
	}

private:
    explicit MavlinkStreamFlightTestInput(Mavlink *mavlink) : MavlinkStream(mavlink) {}

    uORB::Subscription _flight_test_input_sub{ORB_ID(flight_test_input)};

    bool send() override
    {

        struct flight_test_input_s _flight_test_input;  //make sure ca_traj_struct_s is the definition of your uORB topic
        _flight_test_input = {};

        if (_flight_test_input_sub.update(&_flight_test_input))
	{
            __mavlink_flight_test_input_t _msg_flight_test_input;  //make sure mavlink_ca_trajectory_t is the definition of your custom MAVLink message

            _msg_flight_test_input.timestamp = _flight_test_input.timestamp;
            _msg_flight_test_input.fti_mode = _flight_test_input.mode;
            _msg_flight_test_input.fti_state  = _flight_test_input.state;

            _msg_flight_test_input.fti_sweep_time_segment_pct =_flight_test_input.sweep_time_segment_pct;
            _msg_flight_test_input.fti_sweep_frequency = _flight_test_input.sweep_frequency;
	        _msg_flight_test_input.fti_sweep_amplitude = _flight_test_input.sweep_amplitude;

            _msg_flight_test_input.fti_injection_input = _flight_test_input.injection_input;
            _msg_flight_test_input.fti_injection_output = _flight_test_input.injection_output;
            _msg_flight_test_input.fti_raw_output = _flight_test_input.raw_output;
            _msg_flight_test_input.fti_injection_point = _flight_test_input.injection_point;

            // double val = _msg_flight_test_input.fti_injection_point;

            mavlink_msg_flight_test_input_send_struct(_mavlink->get_channel(), &_msg_flight_test_input);

            // PX4_WARN("Accelerometer:\t%.6f", val);
            return true;
        }

        // PX4_WARN("no bueno!!!!");
        return false;
    }
};


#endif // FLIGHT_TEST_INPUT_HPP
```

**MAKE SURE YOU INCLUDE THE DIRECTORY OF WHERE THE CUSTOM MAVLINK MESSAGE IS** For us it was `#include "flight_test_input/mavlink_msg_flight_test_input.h"`


## Add to mavlink_messages.cpp
Now in `src/modules/mavlink/mavlink_messages.cpp` add the following into your code,
```cpp
//line 56 to include your uorb topics
#include <uORB/topics/flight_test_input.h>


//line 83 where you include your hpp file to convert uorb to mavlink messages
#include "streams/FLIGHT_TEST_INPUT.hpp"


....

//line 563
#if defined(FLIGHT_TEST_INPUT_HPP)
	create_stream_list_item<MavlinkStreamFlightTestInput>()
#endif // FLIGHT_TEST_INPUT_HPP
};
```


## Add to mavlink_main.cpp
Now go to `src/modules/mavlink/mavlink_main.cpp` add the following stuff to allow the messages to be streamed out to different protocols

```cpp

case MAVLINK_MODE_NORMAL:
	configure_stream_local("FLIGHT_TEST_INPUT", 10.0f);


case MAVLINK_MODE_ONBOARD:
	configure_stream_local("FLIGHT_TEST_INPUT", 10.0f);


case MAVLINK_MODE_OSD:
	configure_stream_local("FLIGHT_TEST_INPUT", 10.0f);


case MAVLINK_MODE_CONFIG:
	configure_stream_local("FLIGHT_TEST_INPUT", 10.0f);


case MAVLINK_MODE_ONBOARD_LOW_BANDWIDTH:
	configure_stream_local("FLIGHT_TEST_INPUT", 10.0f);

```


After that go you cd back into the top level directory and `make` to compile the code
