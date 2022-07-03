#pragma once
// MESSAGE FLIGHT_TEST_INPUT PACKING

#define MAVLINK_MSG_ID_FLIGHT_TEST_INPUT 229


typedef struct __mavlink_flight_test_input_t {
 uint64_t timestamp; /*<  time since system start (microseconds)*/
 uint32_t fti_mode; /*<  fti mode set.*/
 uint32_t fti_state; /*<  fti state set.*/
 float fti_sweep_time_segment_pct; /*<  fti sweep percentage.*/
 float fti_sweep_amplitude; /*<  fti sweep amplitude.*/
 float fti_sweep_frequency; /*<  fti sweep amplitude.*/
 float fti_injection_input; /*<  fti injection input.*/
 float fti_injection_output; /*<  fti injection output.*/
 float fti_raw_output; /*<  fti raw output.*/
 float fti_injection_point; /*<  fti injection point.*/
} mavlink_flight_test_input_t;

#define MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN 44
#define MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN 44
#define MAVLINK_MSG_ID_229_LEN 44
#define MAVLINK_MSG_ID_229_MIN_LEN 44

#define MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC 181
#define MAVLINK_MSG_ID_229_CRC 181



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_FLIGHT_TEST_INPUT { \
    229, \
    "FLIGHT_TEST_INPUT", \
    10, \
    {  { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_flight_test_input_t, timestamp) }, \
         { "fti_mode", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_flight_test_input_t, fti_mode) }, \
         { "fti_state", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_flight_test_input_t, fti_state) }, \
         { "fti_sweep_time_segment_pct", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_flight_test_input_t, fti_sweep_time_segment_pct) }, \
         { "fti_sweep_amplitude", NULL, MAVLINK_TYPE_FLOAT, 0, 20, offsetof(mavlink_flight_test_input_t, fti_sweep_amplitude) }, \
         { "fti_sweep_frequency", NULL, MAVLINK_TYPE_FLOAT, 0, 24, offsetof(mavlink_flight_test_input_t, fti_sweep_frequency) }, \
         { "fti_injection_input", NULL, MAVLINK_TYPE_FLOAT, 0, 28, offsetof(mavlink_flight_test_input_t, fti_injection_input) }, \
         { "fti_injection_output", NULL, MAVLINK_TYPE_FLOAT, 0, 32, offsetof(mavlink_flight_test_input_t, fti_injection_output) }, \
         { "fti_raw_output", NULL, MAVLINK_TYPE_FLOAT, 0, 36, offsetof(mavlink_flight_test_input_t, fti_raw_output) }, \
         { "fti_injection_point", NULL, MAVLINK_TYPE_FLOAT, 0, 40, offsetof(mavlink_flight_test_input_t, fti_injection_point) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_FLIGHT_TEST_INPUT { \
    "FLIGHT_TEST_INPUT", \
    10, \
    {  { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_flight_test_input_t, timestamp) }, \
         { "fti_mode", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_flight_test_input_t, fti_mode) }, \
         { "fti_state", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_flight_test_input_t, fti_state) }, \
         { "fti_sweep_time_segment_pct", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_flight_test_input_t, fti_sweep_time_segment_pct) }, \
         { "fti_sweep_amplitude", NULL, MAVLINK_TYPE_FLOAT, 0, 20, offsetof(mavlink_flight_test_input_t, fti_sweep_amplitude) }, \
         { "fti_sweep_frequency", NULL, MAVLINK_TYPE_FLOAT, 0, 24, offsetof(mavlink_flight_test_input_t, fti_sweep_frequency) }, \
         { "fti_injection_input", NULL, MAVLINK_TYPE_FLOAT, 0, 28, offsetof(mavlink_flight_test_input_t, fti_injection_input) }, \
         { "fti_injection_output", NULL, MAVLINK_TYPE_FLOAT, 0, 32, offsetof(mavlink_flight_test_input_t, fti_injection_output) }, \
         { "fti_raw_output", NULL, MAVLINK_TYPE_FLOAT, 0, 36, offsetof(mavlink_flight_test_input_t, fti_raw_output) }, \
         { "fti_injection_point", NULL, MAVLINK_TYPE_FLOAT, 0, 40, offsetof(mavlink_flight_test_input_t, fti_injection_point) }, \
         } \
}
#endif

/**
 * @brief Pack a flight_test_input message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param timestamp  time since system start (microseconds)
 * @param fti_mode  fti mode set.
 * @param fti_state  fti state set.
 * @param fti_sweep_time_segment_pct  fti sweep percentage.
 * @param fti_sweep_amplitude  fti sweep amplitude.
 * @param fti_sweep_frequency  fti sweep amplitude.
 * @param fti_injection_input  fti injection input.
 * @param fti_injection_output  fti injection output.
 * @param fti_raw_output  fti raw output.
 * @param fti_injection_point  fti injection point.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_flight_test_input_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint64_t timestamp, uint32_t fti_mode, uint32_t fti_state, float fti_sweep_time_segment_pct, float fti_sweep_amplitude, float fti_sweep_frequency, float fti_injection_input, float fti_injection_output, float fti_raw_output, float fti_injection_point)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint32_t(buf, 8, fti_mode);
    _mav_put_uint32_t(buf, 12, fti_state);
    _mav_put_float(buf, 16, fti_sweep_time_segment_pct);
    _mav_put_float(buf, 20, fti_sweep_amplitude);
    _mav_put_float(buf, 24, fti_sweep_frequency);
    _mav_put_float(buf, 28, fti_injection_input);
    _mav_put_float(buf, 32, fti_injection_output);
    _mav_put_float(buf, 36, fti_raw_output);
    _mav_put_float(buf, 40, fti_injection_point);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN);
#else
    mavlink_flight_test_input_t packet;
    packet.timestamp = timestamp;
    packet.fti_mode = fti_mode;
    packet.fti_state = fti_state;
    packet.fti_sweep_time_segment_pct = fti_sweep_time_segment_pct;
    packet.fti_sweep_amplitude = fti_sweep_amplitude;
    packet.fti_sweep_frequency = fti_sweep_frequency;
    packet.fti_injection_input = fti_injection_input;
    packet.fti_injection_output = fti_injection_output;
    packet.fti_raw_output = fti_raw_output;
    packet.fti_injection_point = fti_injection_point;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_FLIGHT_TEST_INPUT;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
}

/**
 * @brief Pack a flight_test_input message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param timestamp  time since system start (microseconds)
 * @param fti_mode  fti mode set.
 * @param fti_state  fti state set.
 * @param fti_sweep_time_segment_pct  fti sweep percentage.
 * @param fti_sweep_amplitude  fti sweep amplitude.
 * @param fti_sweep_frequency  fti sweep amplitude.
 * @param fti_injection_input  fti injection input.
 * @param fti_injection_output  fti injection output.
 * @param fti_raw_output  fti raw output.
 * @param fti_injection_point  fti injection point.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_flight_test_input_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint64_t timestamp,uint32_t fti_mode,uint32_t fti_state,float fti_sweep_time_segment_pct,float fti_sweep_amplitude,float fti_sweep_frequency,float fti_injection_input,float fti_injection_output,float fti_raw_output,float fti_injection_point)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint32_t(buf, 8, fti_mode);
    _mav_put_uint32_t(buf, 12, fti_state);
    _mav_put_float(buf, 16, fti_sweep_time_segment_pct);
    _mav_put_float(buf, 20, fti_sweep_amplitude);
    _mav_put_float(buf, 24, fti_sweep_frequency);
    _mav_put_float(buf, 28, fti_injection_input);
    _mav_put_float(buf, 32, fti_injection_output);
    _mav_put_float(buf, 36, fti_raw_output);
    _mav_put_float(buf, 40, fti_injection_point);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN);
#else
    mavlink_flight_test_input_t packet;
    packet.timestamp = timestamp;
    packet.fti_mode = fti_mode;
    packet.fti_state = fti_state;
    packet.fti_sweep_time_segment_pct = fti_sweep_time_segment_pct;
    packet.fti_sweep_amplitude = fti_sweep_amplitude;
    packet.fti_sweep_frequency = fti_sweep_frequency;
    packet.fti_injection_input = fti_injection_input;
    packet.fti_injection_output = fti_injection_output;
    packet.fti_raw_output = fti_raw_output;
    packet.fti_injection_point = fti_injection_point;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_FLIGHT_TEST_INPUT;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
}

/**
 * @brief Encode a flight_test_input struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param flight_test_input C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_flight_test_input_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_flight_test_input_t* flight_test_input)
{
    return mavlink_msg_flight_test_input_pack(system_id, component_id, msg, flight_test_input->timestamp, flight_test_input->fti_mode, flight_test_input->fti_state, flight_test_input->fti_sweep_time_segment_pct, flight_test_input->fti_sweep_amplitude, flight_test_input->fti_sweep_frequency, flight_test_input->fti_injection_input, flight_test_input->fti_injection_output, flight_test_input->fti_raw_output, flight_test_input->fti_injection_point);
}

/**
 * @brief Encode a flight_test_input struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param flight_test_input C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_flight_test_input_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_flight_test_input_t* flight_test_input)
{
    return mavlink_msg_flight_test_input_pack_chan(system_id, component_id, chan, msg, flight_test_input->timestamp, flight_test_input->fti_mode, flight_test_input->fti_state, flight_test_input->fti_sweep_time_segment_pct, flight_test_input->fti_sweep_amplitude, flight_test_input->fti_sweep_frequency, flight_test_input->fti_injection_input, flight_test_input->fti_injection_output, flight_test_input->fti_raw_output, flight_test_input->fti_injection_point);
}

/**
 * @brief Send a flight_test_input message
 * @param chan MAVLink channel to send the message
 *
 * @param timestamp  time since system start (microseconds)
 * @param fti_mode  fti mode set.
 * @param fti_state  fti state set.
 * @param fti_sweep_time_segment_pct  fti sweep percentage.
 * @param fti_sweep_amplitude  fti sweep amplitude.
 * @param fti_sweep_frequency  fti sweep amplitude.
 * @param fti_injection_input  fti injection input.
 * @param fti_injection_output  fti injection output.
 * @param fti_raw_output  fti raw output.
 * @param fti_injection_point  fti injection point.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_flight_test_input_send(mavlink_channel_t chan, uint64_t timestamp, uint32_t fti_mode, uint32_t fti_state, float fti_sweep_time_segment_pct, float fti_sweep_amplitude, float fti_sweep_frequency, float fti_injection_input, float fti_injection_output, float fti_raw_output, float fti_injection_point)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint32_t(buf, 8, fti_mode);
    _mav_put_uint32_t(buf, 12, fti_state);
    _mav_put_float(buf, 16, fti_sweep_time_segment_pct);
    _mav_put_float(buf, 20, fti_sweep_amplitude);
    _mav_put_float(buf, 24, fti_sweep_frequency);
    _mav_put_float(buf, 28, fti_injection_input);
    _mav_put_float(buf, 32, fti_injection_output);
    _mav_put_float(buf, 36, fti_raw_output);
    _mav_put_float(buf, 40, fti_injection_point);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT, buf, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
#else
    mavlink_flight_test_input_t packet;
    packet.timestamp = timestamp;
    packet.fti_mode = fti_mode;
    packet.fti_state = fti_state;
    packet.fti_sweep_time_segment_pct = fti_sweep_time_segment_pct;
    packet.fti_sweep_amplitude = fti_sweep_amplitude;
    packet.fti_sweep_frequency = fti_sweep_frequency;
    packet.fti_injection_input = fti_injection_input;
    packet.fti_injection_output = fti_injection_output;
    packet.fti_raw_output = fti_raw_output;
    packet.fti_injection_point = fti_injection_point;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT, (const char *)&packet, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
#endif
}

/**
 * @brief Send a flight_test_input message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_flight_test_input_send_struct(mavlink_channel_t chan, const mavlink_flight_test_input_t* flight_test_input)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_flight_test_input_send(chan, flight_test_input->timestamp, flight_test_input->fti_mode, flight_test_input->fti_state, flight_test_input->fti_sweep_time_segment_pct, flight_test_input->fti_sweep_amplitude, flight_test_input->fti_sweep_frequency, flight_test_input->fti_injection_input, flight_test_input->fti_injection_output, flight_test_input->fti_raw_output, flight_test_input->fti_injection_point);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT, (const char *)flight_test_input, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
#endif
}

#if MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_flight_test_input_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint64_t timestamp, uint32_t fti_mode, uint32_t fti_state, float fti_sweep_time_segment_pct, float fti_sweep_amplitude, float fti_sweep_frequency, float fti_injection_input, float fti_injection_output, float fti_raw_output, float fti_injection_point)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_uint32_t(buf, 8, fti_mode);
    _mav_put_uint32_t(buf, 12, fti_state);
    _mav_put_float(buf, 16, fti_sweep_time_segment_pct);
    _mav_put_float(buf, 20, fti_sweep_amplitude);
    _mav_put_float(buf, 24, fti_sweep_frequency);
    _mav_put_float(buf, 28, fti_injection_input);
    _mav_put_float(buf, 32, fti_injection_output);
    _mav_put_float(buf, 36, fti_raw_output);
    _mav_put_float(buf, 40, fti_injection_point);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT, buf, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
#else
    mavlink_flight_test_input_t *packet = (mavlink_flight_test_input_t *)msgbuf;
    packet->timestamp = timestamp;
    packet->fti_mode = fti_mode;
    packet->fti_state = fti_state;
    packet->fti_sweep_time_segment_pct = fti_sweep_time_segment_pct;
    packet->fti_sweep_amplitude = fti_sweep_amplitude;
    packet->fti_sweep_frequency = fti_sweep_frequency;
    packet->fti_injection_input = fti_injection_input;
    packet->fti_injection_output = fti_injection_output;
    packet->fti_raw_output = fti_raw_output;
    packet->fti_injection_point = fti_injection_point;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT, (const char *)packet, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_MIN_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_CRC);
#endif
}
#endif

#endif

// MESSAGE FLIGHT_TEST_INPUT UNPACKING


/**
 * @brief Get field timestamp from flight_test_input message
 *
 * @return  time since system start (microseconds)
 */
static inline uint64_t mavlink_msg_flight_test_input_get_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field fti_mode from flight_test_input message
 *
 * @return  fti mode set.
 */
static inline uint32_t mavlink_msg_flight_test_input_get_fti_mode(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  8);
}

/**
 * @brief Get field fti_state from flight_test_input message
 *
 * @return  fti state set.
 */
static inline uint32_t mavlink_msg_flight_test_input_get_fti_state(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  12);
}

/**
 * @brief Get field fti_sweep_time_segment_pct from flight_test_input message
 *
 * @return  fti sweep percentage.
 */
static inline float mavlink_msg_flight_test_input_get_fti_sweep_time_segment_pct(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Get field fti_sweep_amplitude from flight_test_input message
 *
 * @return  fti sweep amplitude.
 */
static inline float mavlink_msg_flight_test_input_get_fti_sweep_amplitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  20);
}

/**
 * @brief Get field fti_sweep_frequency from flight_test_input message
 *
 * @return  fti sweep amplitude.
 */
static inline float mavlink_msg_flight_test_input_get_fti_sweep_frequency(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  24);
}

/**
 * @brief Get field fti_injection_input from flight_test_input message
 *
 * @return  fti injection input.
 */
static inline float mavlink_msg_flight_test_input_get_fti_injection_input(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  28);
}

/**
 * @brief Get field fti_injection_output from flight_test_input message
 *
 * @return  fti injection output.
 */
static inline float mavlink_msg_flight_test_input_get_fti_injection_output(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  32);
}

/**
 * @brief Get field fti_raw_output from flight_test_input message
 *
 * @return  fti raw output.
 */
static inline float mavlink_msg_flight_test_input_get_fti_raw_output(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  36);
}

/**
 * @brief Get field fti_injection_point from flight_test_input message
 *
 * @return  fti injection point.
 */
static inline float mavlink_msg_flight_test_input_get_fti_injection_point(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  40);
}

/**
 * @brief Decode a flight_test_input message into a struct
 *
 * @param msg The message to decode
 * @param flight_test_input C-struct to decode the message contents into
 */
static inline void mavlink_msg_flight_test_input_decode(const mavlink_message_t* msg, mavlink_flight_test_input_t* flight_test_input)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    flight_test_input->timestamp = mavlink_msg_flight_test_input_get_timestamp(msg);
    flight_test_input->fti_mode = mavlink_msg_flight_test_input_get_fti_mode(msg);
    flight_test_input->fti_state = mavlink_msg_flight_test_input_get_fti_state(msg);
    flight_test_input->fti_sweep_time_segment_pct = mavlink_msg_flight_test_input_get_fti_sweep_time_segment_pct(msg);
    flight_test_input->fti_sweep_amplitude = mavlink_msg_flight_test_input_get_fti_sweep_amplitude(msg);
    flight_test_input->fti_sweep_frequency = mavlink_msg_flight_test_input_get_fti_sweep_frequency(msg);
    flight_test_input->fti_injection_input = mavlink_msg_flight_test_input_get_fti_injection_input(msg);
    flight_test_input->fti_injection_output = mavlink_msg_flight_test_input_get_fti_injection_output(msg);
    flight_test_input->fti_raw_output = mavlink_msg_flight_test_input_get_fti_raw_output(msg);
    flight_test_input->fti_injection_point = mavlink_msg_flight_test_input_get_fti_injection_point(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN? msg->len : MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN;
        memset(flight_test_input, 0, MAVLINK_MSG_ID_FLIGHT_TEST_INPUT_LEN);
    memcpy(flight_test_input, _MAV_PAYLOAD(msg), len);
#endif
}
