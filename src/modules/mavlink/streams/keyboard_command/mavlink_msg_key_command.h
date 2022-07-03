#pragma once
// MESSAGE KEY_COMMAND PACKING

#define MAVLINK_MSG_ID_KEY_COMMAND 239


typedef struct __mavlink_key_command_t {
 uint64_t timestamp; /*<   */
 char command; /*<   */
} mavlink_key_command_t;

#define MAVLINK_MSG_ID_KEY_COMMAND_LEN 9
#define MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN 9
#define MAVLINK_MSG_ID_239_LEN 9
#define MAVLINK_MSG_ID_239_MIN_LEN 9

#define MAVLINK_MSG_ID_KEY_COMMAND_CRC 112
#define MAVLINK_MSG_ID_239_CRC 112



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_KEY_COMMAND { \
    239, \
    "KEY_COMMAND", \
    2, \
    {  { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_key_command_t, timestamp) }, \
         { "command", NULL, MAVLINK_TYPE_CHAR, 0, 8, offsetof(mavlink_key_command_t, command) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_KEY_COMMAND { \
    "KEY_COMMAND", \
    2, \
    {  { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_key_command_t, timestamp) }, \
         { "command", NULL, MAVLINK_TYPE_CHAR, 0, 8, offsetof(mavlink_key_command_t, command) }, \
         } \
}
#endif

/**
 * @brief Pack a key_command message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param timestamp
 * @param command
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_key_command_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint64_t timestamp, char command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_KEY_COMMAND_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_char(buf, 8, command);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_KEY_COMMAND_LEN);
#else
    mavlink_key_command_t packet;
    packet.timestamp = timestamp;
    packet.command = command;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_KEY_COMMAND_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_KEY_COMMAND;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
}

/**
 * @brief Pack a key_command message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param timestamp
 * @param command
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_key_command_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint64_t timestamp,char command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_KEY_COMMAND_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_char(buf, 8, command);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_KEY_COMMAND_LEN);
#else
    mavlink_key_command_t packet;
    packet.timestamp = timestamp;
    packet.command = command;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_KEY_COMMAND_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_KEY_COMMAND;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
}

/**
 * @brief Encode a key_command struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param key_command C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_key_command_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_key_command_t* key_command)
{
    return mavlink_msg_key_command_pack(system_id, component_id, msg, key_command->timestamp, key_command->command);
}

/**
 * @brief Encode a key_command struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param key_command C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_key_command_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_key_command_t* key_command)
{
    return mavlink_msg_key_command_pack_chan(system_id, component_id, chan, msg, key_command->timestamp, key_command->command);
}

/**
 * @brief Send a key_command message
 * @param chan MAVLink channel to send the message
 *
 * @param timestamp
 * @param command
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_key_command_send(mavlink_channel_t chan, uint64_t timestamp, char command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_KEY_COMMAND_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_char(buf, 8, command);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_KEY_COMMAND, buf, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
#else
    mavlink_key_command_t packet;
    packet.timestamp = timestamp;
    packet.command = command;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_KEY_COMMAND, (const char *)&packet, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
#endif
}

/**
 * @brief Send a key_command message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_key_command_send_struct(mavlink_channel_t chan, const mavlink_key_command_t* key_command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_key_command_send(chan, key_command->timestamp, key_command->command);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_KEY_COMMAND, (const char *)key_command, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
#endif
}

#if MAVLINK_MSG_ID_KEY_COMMAND_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_key_command_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint64_t timestamp, char command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_char(buf, 8, command);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_KEY_COMMAND, buf, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
#else
    mavlink_key_command_t *packet = (mavlink_key_command_t *)msgbuf;
    packet->timestamp = timestamp;
    packet->command = command;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_KEY_COMMAND, (const char *)packet, MAVLINK_MSG_ID_KEY_COMMAND_MIN_LEN, MAVLINK_MSG_ID_KEY_COMMAND_LEN, MAVLINK_MSG_ID_KEY_COMMAND_CRC);
#endif
}
#endif

#endif

// MESSAGE KEY_COMMAND UNPACKING


/**
 * @brief Get field timestamp from key_command message
 *
 * @return
 */
static inline uint64_t mavlink_msg_key_command_get_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field command from key_command message
 *
 * @return
 */
static inline char mavlink_msg_key_command_get_command(const mavlink_message_t* msg)
{
    return _MAV_RETURN_char(msg,  8);
}

/**
 * @brief Decode a key_command message into a struct
 *
 * @param msg The message to decode
 * @param key_command C-struct to decode the message contents into
 */
static inline void mavlink_msg_key_command_decode(const mavlink_message_t* msg, mavlink_key_command_t* key_command)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    key_command->timestamp = mavlink_msg_key_command_get_timestamp(msg);
    key_command->command = mavlink_msg_key_command_get_command(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_KEY_COMMAND_LEN? msg->len : MAVLINK_MSG_ID_KEY_COMMAND_LEN;
        memset(key_command, 0, MAVLINK_MSG_ID_KEY_COMMAND_LEN);
    memcpy(key_command, _MAV_PAYLOAD(msg), len);
#endif
}
