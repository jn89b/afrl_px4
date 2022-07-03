/**
 * @file px4_inject_myuorb.c
 * Minimal application example for PX4 autopilot
 *
 * @author Example User <mail@example.com>
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <math.h>

#include <uORB/uORB.h>
#include <uORB/topics/video_monitor.h>

//extern "C" __EXPORT int px4_inject_myUORB_main(int argc, char *argv[]);
__EXPORT int px4_inject_myuorb_main(int argc, char *argv[]);

int px4_inject_myuorb_main(int argc, char *argv[])
{
    PX4_INFO("Hello, I am only a test program able to inject VIDEO_MONITOR messages.");

    // Declare structure to store data that will be sent
    struct video_monitor_s videoMon;

    // Clear the structure by filling it with 0s in memory
    memset(&videoMon, 0, sizeof(videoMon));

    // Create a uORB topic advertisement
    orb_advert_t video_monitor_pub = orb_advertise(ORB_ID(video_monitor), &videoMon);

    for (int i=0; i<40; i++)
        {
        char myStr[]={"Salut !!"}; memcpy(videoMon.info, myStr, 9);
        videoMon.timestamp = hrt_absolute_time();
        videoMon.lat  = i;
        videoMon.lon  = 12345678;
        videoMon.no_people  = i+5;
        videoMon.confidence  = 0.369;

        orb_publish(ORB_ID(video_monitor), video_monitor_pub, &videoMon);

        // //sleep for 2s
        // usleep ();
        }

    PX4_INFO("inject_myUORB finished!");

    return 0;
}
