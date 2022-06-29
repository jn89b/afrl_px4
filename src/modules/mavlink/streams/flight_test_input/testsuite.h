/** @file
 *    @brief MAVLink comm protocol testsuite generated from flight_test_input.xml
 *    @see https://mavlink.io/en/
 */
#pragma once
#ifndef FLIGHT_TEST_INPUT_TESTSUITE_H
#define FLIGHT_TEST_INPUT_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL
static void mavlink_test_common(uint8_t, uint8_t, mavlink_message_t *last_msg);
static void mavlink_test_flight_test_input(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_common(system_id, component_id, last_msg);
    mavlink_test_flight_test_input(system_id, component_id, last_msg);
}
#endif

#include "../common/testsuite.h"


static void mavlink_test_flight_test_input(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_FLIGHT_TEST_INPUT >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_flight_test_input_t packet_in = {
        93372036854775807ULL,963497880,963498088,129.0,157.0,185.0,213.0,241.0,269.0,297.0
    };
    mavlink_flight_test_input_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.timestamp = packet_in.timestamp;
        packet1.fti_mode = packet_in.fti_mode;
        packet1.fti_state = packet_in.fti_state;
        packet1.fti_sweep_time_segment_pct = packet_in.fti_sweep_time_segment_pct;
        packet1.fti_sweep_amplitude = packet_in.fti_sweep_amplitude;
        packet1.fti_sweep_frequency = packet_in.fti_sweep_frequency;
        packet1.fti_injection_input = packet_in.fti_injection_input;
        packet1.fti_injection_output = packet_in.fti_injection_output;
        packet1.fti_raw_output = packet_in.fti_raw_output;
        packet1.fti_injection_point = packet_in.fti_injection_point;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_flight_test_input_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_flight_test_input_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_flight_test_input_pack(system_id, component_id, &msg , packet1.timestamp , packet1.fti_mode , packet1.fti_state , packet1.fti_sweep_time_segment_pct , packet1.fti_sweep_amplitude , packet1.fti_sweep_frequency , packet1.fti_injection_input , packet1.fti_injection_output , packet1.fti_raw_output , packet1.fti_injection_point );
    mavlink_msg_flight_test_input_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_flight_test_input_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.timestamp , packet1.fti_mode , packet1.fti_state , packet1.fti_sweep_time_segment_pct , packet1.fti_sweep_amplitude , packet1.fti_sweep_frequency , packet1.fti_injection_input , packet1.fti_injection_output , packet1.fti_raw_output , packet1.fti_injection_point );
    mavlink_msg_flight_test_input_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_flight_test_input_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_flight_test_input_send(MAVLINK_COMM_1 , packet1.timestamp , packet1.fti_mode , packet1.fti_state , packet1.fti_sweep_time_segment_pct , packet1.fti_sweep_amplitude , packet1.fti_sweep_frequency , packet1.fti_injection_input , packet1.fti_injection_output , packet1.fti_raw_output , packet1.fti_injection_point );
    mavlink_msg_flight_test_input_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("FLIGHT_TEST_INPUT") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_FLIGHT_TEST_INPUT) != NULL);
#endif
}

static void mavlink_test_flight_test_input(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_flight_test_input(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // FLIGHT_TEST_INPUT_TESTSUITE_H
