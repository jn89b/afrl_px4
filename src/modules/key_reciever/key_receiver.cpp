#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <math.h>
#include <uORB/uORB.h>
#include <uORB/topics/key_command.h>

extern "C" __EXPORT int key_receiver_main(int argc, char **argv);

int key_receiver_main(int argc, char **argv)
{
    int key_sub_fd = orb_subscribe(ORB_ID(key_command));
    orb_set_interval(key_sub_fd, 200); // limit the update rate to 200ms

    px4_pollfd_struct_t fds[1];
    fds[0].fd = key_sub_fd, fds[0].events = POLLIN;

    int error_counter = 0;

    while(true)
    {
        int poll_ret = px4_poll(fds, 1, 1000);

        if (poll_ret == 0)
        {
            PX4_ERR("Got no data within a second");
        }

        else if (poll_ret < 0)
        {
            if (error_counter < 10 || error_counter % 50 == 0)
            {
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }

            error_counter++;
        }

        else
        {
            if (fds[0].revents & POLLIN)
            {
                struct key_command_s input;
                orb_copy(ORB_ID(key_command), key_sub_fd, &input);
                PX4_INFO("Recieved Char : %c", input.cmd);
             }
        }
    }
    return 0;
}
