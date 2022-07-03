#ifndef VIDEO_MON_HPP
#define VIDEO_MON_HPP

#include <uORB/topics/video_monitor.h>  //placed in: build/nxp_fmuk66-v3_default/uORB/topics
#include "video_monitor/mavlink.h"
#include "video_monitor/mavlink_msg_video_monitor.h"

class MavlinkStreamVideoMonitor : public MavlinkStream
{
public:
    static MavlinkStream *new_instance(Mavlink *mavlink)
    { return new MavlinkStreamVideoMonitor(mavlink); }

    // In a member function declaration or definition, override specifier ensures that
    // the function is virtual and is overriding a virtual function from a base class.
    const char*get_name() const override
    { return MavlinkStreamVideoMonitor::get_name_static(); }

    // The constexpr specifier declares that it is possible to
    // evaluate the value of the function or variable at compile time.
    static constexpr const char *get_name_static()
    { return "VIDEO_MONITOR";  }

    uint16_t get_id() override { return get_id_static(); }

    static constexpr uint16_t get_id_static()
    { return MAVLINK_MSG_ID_VIDEO_MONITOR; }

    unsigned get_size() override
    { return MAVLINK_MSG_ID_VIDEO_MONITOR_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES; }

private:
    uORB::Subscription _sub{ORB_ID(video_monitor)};

    /* do not allow top copying this class */
    MavlinkStreamVideoMonitor(MavlinkStreamVideoMonitor &);
    MavlinkStreamVideoMonitor& operator = (const MavlinkStreamVideoMonitor &);

protected:
    explicit MavlinkStreamVideoMonitor(Mavlink *mavlink) : MavlinkStream(mavlink)
    {}

    bool send() override
    {
        struct video_monitor_s _video_monitor;  //make sure video_monitor_s is the
                                                //definition of your uORB topic

        if (_sub.update(&_video_monitor))
        {
        mavlink_video_monitor_t _msg_video_monitor;  // mavlink_video_monitor_t is the
                                                     // definition of your custom
                                                     // MAVLink message
        _msg_video_monitor.timestamp  = _video_monitor.timestamp;
        _msg_video_monitor.lat  = _video_monitor.lat;
        _msg_video_monitor.lon  = _video_monitor.lon;
        _msg_video_monitor.no_people  = _video_monitor.no_people;
        _msg_video_monitor.confidence  = _video_monitor.confidence;

	PX4_WARN("sending message");

        for(int i=0; i<11; i++)
            _msg_video_monitor.info[i] = _video_monitor.info[i];

        mavlink_msg_video_monitor_send_struct(_mavlink->get_channel(),
                                              &_msg_video_monitor);

        PX4_WARN("uorb => mavlink - message was sent !!!!");

        return true;
        }

    return false;
    }
};
#endif // VIDEO_MON_HPP
