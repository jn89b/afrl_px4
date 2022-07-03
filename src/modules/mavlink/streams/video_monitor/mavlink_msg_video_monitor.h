#pragma once
// MESSAGE VIDEO_MONITOR PACKING

#define MAVLINK_MSG_ID_VIDEO_MONITOR 369


typedef struct __mavlink_video_monitor_t {
 uint64_t timestamp; /*<  time since system start (microseconds)*/
 int32_t lat; /*< [degE7] Latitude WGS84 (deg * 1E7). If unknown set to INT32_MAX*/
 int32_t lon; /*< [degE7] Longitude WGS84 (deg * 1E7). If unknown set to INT32_MAX*/
 float confidence; /*<  I'n not sure for what to using it*/
 uint16_t no_people; /*<  number of identified peoples*/
 char info[11]; /*<  General information (11 characters, null terminated, valid characters are A-Z, 0-9, " " only)*/
} mavlink_video_monitor_t;

#define MAVLINK_MSG_ID_VIDEO_MONITOR_LEN 33
#define MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN 33
#define MAVLINK_MSG_ID_369_LEN 33
#define MAVLINK_MSG_ID_369_MIN_LEN 33

#define MAVLINK_MSG_ID_VIDEO_MONITOR_CRC 49
#define MAVLINK_MSG_ID_369_CRC 49

#define MAVLINK_MSG_VIDEO_MONITOR_FIELD_INFO_LEN 11

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_VIDEO_MONITOR { \
    369, \
    "VIDEO_MONITOR", \
    6, \
    {  { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_video_monitor_t, timestamp) }, \
         { "info", NULL, MAVLINK_TYPE_CHAR, 11, 22, offsetof(mavlink_video_monitor_t, info) }, \
         { "lat", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_video_monitor_t, lat) }, \
         { "lon", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_video_monitor_t, lon) }, \
         { "no_people", NULL, MAVLINK_TYPE_UINT16_T, 0, 20, offsetof(mavlink_video_monitor_t, no_people) }, \
         { "confidence", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_video_monitor_t, confidence) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_VIDEO_MONITOR { \
    "VIDEO_MONITOR", \
    6, \
    {  { "timestamp", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_video_monitor_t, timestamp) }, \
         { "info", NULL, MAVLINK_TYPE_CHAR, 11, 22, offsetof(mavlink_video_monitor_t, info) }, \
         { "lat", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_video_monitor_t, lat) }, \
         { "lon", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_video_monitor_t, lon) }, \
         { "no_people", NULL, MAVLINK_TYPE_UINT16_T, 0, 20, offsetof(mavlink_video_monitor_t, no_people) }, \
         { "confidence", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_video_monitor_t, confidence) }, \
         } \
}
#endif

/**
 * @brief Pack a video_monitor message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param timestamp  time since system start (microseconds)
 * @param info  General information (11 characters, null terminated, valid characters are A-Z, 0-9, " " only)
 * @param lat [degE7] Latitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 * @param lon [degE7] Longitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 * @param no_people  number of identified peoples
 * @param confidence  I'n not sure for what to using it
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_video_monitor_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint64_t timestamp, const char *info, int32_t lat, int32_t lon, uint16_t no_people, float confidence)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_VIDEO_MONITOR_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_int32_t(buf, 8, lat);
    _mav_put_int32_t(buf, 12, lon);
    _mav_put_float(buf, 16, confidence);
    _mav_put_uint16_t(buf, 20, no_people);
    _mav_put_char_array(buf, 22, info, 11);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN);
#else
    mavlink_video_monitor_t packet;
    packet.timestamp = timestamp;
    packet.lat = lat;
    packet.lon = lon;
    packet.confidence = confidence;
    packet.no_people = no_people;
    mav_array_memcpy(packet.info, info, sizeof(char)*11);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_VIDEO_MONITOR;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
}

/**
 * @brief Pack a video_monitor message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param timestamp  time since system start (microseconds)
 * @param info  General information (11 characters, null terminated, valid characters are A-Z, 0-9, " " only)
 * @param lat [degE7] Latitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 * @param lon [degE7] Longitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 * @param no_people  number of identified peoples
 * @param confidence  I'n not sure for what to using it
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_video_monitor_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint64_t timestamp,const char *info,int32_t lat,int32_t lon,uint16_t no_people,float confidence)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_VIDEO_MONITOR_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_int32_t(buf, 8, lat);
    _mav_put_int32_t(buf, 12, lon);
    _mav_put_float(buf, 16, confidence);
    _mav_put_uint16_t(buf, 20, no_people);
    _mav_put_char_array(buf, 22, info, 11);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN);
#else
    mavlink_video_monitor_t packet;
    packet.timestamp = timestamp;
    packet.lat = lat;
    packet.lon = lon;
    packet.confidence = confidence;
    packet.no_people = no_people;
    mav_array_memcpy(packet.info, info, sizeof(char)*11);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_VIDEO_MONITOR;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
}

/**
 * @brief Encode a video_monitor struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param video_monitor C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_video_monitor_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_video_monitor_t* video_monitor)
{
    return mavlink_msg_video_monitor_pack(system_id, component_id, msg, video_monitor->timestamp, video_monitor->info, video_monitor->lat, video_monitor->lon, video_monitor->no_people, video_monitor->confidence);
}

/**
 * @brief Encode a video_monitor struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param video_monitor C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_video_monitor_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_video_monitor_t* video_monitor)
{
    return mavlink_msg_video_monitor_pack_chan(system_id, component_id, chan, msg, video_monitor->timestamp, video_monitor->info, video_monitor->lat, video_monitor->lon, video_monitor->no_people, video_monitor->confidence);
}

/**
 * @brief Send a video_monitor message
 * @param chan MAVLink channel to send the message
 *
 * @param timestamp  time since system start (microseconds)
 * @param info  General information (11 characters, null terminated, valid characters are A-Z, 0-9, " " only)
 * @param lat [degE7] Latitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 * @param lon [degE7] Longitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 * @param no_people  number of identified peoples
 * @param confidence  I'n not sure for what to using it
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_video_monitor_send(mavlink_channel_t chan, uint64_t timestamp, const char *info, int32_t lat, int32_t lon, uint16_t no_people, float confidence)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_VIDEO_MONITOR_LEN];
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_int32_t(buf, 8, lat);
    _mav_put_int32_t(buf, 12, lon);
    _mav_put_float(buf, 16, confidence);
    _mav_put_uint16_t(buf, 20, no_people);
    _mav_put_char_array(buf, 22, info, 11);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIDEO_MONITOR, buf, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
#else
    mavlink_video_monitor_t packet;
    packet.timestamp = timestamp;
    packet.lat = lat;
    packet.lon = lon;
    packet.confidence = confidence;
    packet.no_people = no_people;
    mav_array_memcpy(packet.info, info, sizeof(char)*11);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIDEO_MONITOR, (const char *)&packet, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
#endif
}

/**
 * @brief Send a video_monitor message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_video_monitor_send_struct(mavlink_channel_t chan, const mavlink_video_monitor_t* video_monitor)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_video_monitor_send(chan, video_monitor->timestamp, video_monitor->info, video_monitor->lat, video_monitor->lon, video_monitor->no_people, video_monitor->confidence);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIDEO_MONITOR, (const char *)video_monitor, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
#endif
}

#if MAVLINK_MSG_ID_VIDEO_MONITOR_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_video_monitor_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint64_t timestamp, const char *info, int32_t lat, int32_t lon, uint16_t no_people, float confidence)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint64_t(buf, 0, timestamp);
    _mav_put_int32_t(buf, 8, lat);
    _mav_put_int32_t(buf, 12, lon);
    _mav_put_float(buf, 16, confidence);
    _mav_put_uint16_t(buf, 20, no_people);
    _mav_put_char_array(buf, 22, info, 11);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIDEO_MONITOR, buf, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
#else
    mavlink_video_monitor_t *packet = (mavlink_video_monitor_t *)msgbuf;
    packet->timestamp = timestamp;
    packet->lat = lat;
    packet->lon = lon;
    packet->confidence = confidence;
    packet->no_people = no_people;
    mav_array_memcpy(packet->info, info, sizeof(char)*11);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIDEO_MONITOR, (const char *)packet, MAVLINK_MSG_ID_VIDEO_MONITOR_MIN_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN, MAVLINK_MSG_ID_VIDEO_MONITOR_CRC);
#endif
}
#endif

#endif

// MESSAGE VIDEO_MONITOR UNPACKING


/**
 * @brief Get field timestamp from video_monitor message
 *
 * @return  time since system start (microseconds)
 */
static inline uint64_t mavlink_msg_video_monitor_get_timestamp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field info from video_monitor message
 *
 * @return  General information (11 characters, null terminated, valid characters are A-Z, 0-9, " " only)
 */
static inline uint16_t mavlink_msg_video_monitor_get_info(const mavlink_message_t* msg, char *info)
{
    return _MAV_RETURN_char_array(msg, info, 11,  22);
}

/**
 * @brief Get field lat from video_monitor message
 *
 * @return [degE7] Latitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 */
static inline int32_t mavlink_msg_video_monitor_get_lat(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  8);
}

/**
 * @brief Get field lon from video_monitor message
 *
 * @return [degE7] Longitude WGS84 (deg * 1E7). If unknown set to INT32_MAX
 */
static inline int32_t mavlink_msg_video_monitor_get_lon(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  12);
}

/**
 * @brief Get field no_people from video_monitor message
 *
 * @return  number of identified peoples
 */
static inline uint16_t mavlink_msg_video_monitor_get_no_people(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  20);
}

/**
 * @brief Get field confidence from video_monitor message
 *
 * @return  I'n not sure for what to using it
 */
static inline float mavlink_msg_video_monitor_get_confidence(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Decode a video_monitor message into a struct
 *
 * @param msg The message to decode
 * @param video_monitor C-struct to decode the message contents into
 */
static inline void mavlink_msg_video_monitor_decode(const mavlink_message_t* msg, mavlink_video_monitor_t* video_monitor)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    video_monitor->timestamp = mavlink_msg_video_monitor_get_timestamp(msg);
    video_monitor->lat = mavlink_msg_video_monitor_get_lat(msg);
    video_monitor->lon = mavlink_msg_video_monitor_get_lon(msg);
    video_monitor->confidence = mavlink_msg_video_monitor_get_confidence(msg);
    video_monitor->no_people = mavlink_msg_video_monitor_get_no_people(msg);
    mavlink_msg_video_monitor_get_info(msg, video_monitor->info);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_VIDEO_MONITOR_LEN? msg->len : MAVLINK_MSG_ID_VIDEO_MONITOR_LEN;
        memset(video_monitor, 0, MAVLINK_MSG_ID_VIDEO_MONITOR_LEN);
    memcpy(video_monitor, _MAV_PAYLOAD(msg), len);
#endif
}
