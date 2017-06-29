%% convert baro data
clear baro_data;
last_time = 0;
output_index = 1;
for source_index = 1:length(BARO)
    if (BARO(source_index,2) ~= last_time)
        baro_data.time_us(output_index,1) = BARO(source_index,2);
        baro_data.height(output_index) = BARO(source_index,3);
        last_time = BARO(source_index,2);
        output_index = output_index + 1;
    end
end

%% extract IMU delta angles and velocity data
clear imu_data;
imu_data.time_us = IMT(:,2);
imu_data.gyro_dt = IMT(:,5);
imu_data.del_ang = IMT(:,6:8);
imu_data.accel_dt = IMT(:,4);
imu_data.del_vel = IMT(:,9:11);

%% convert magnetomer data
clear mag_data;
last_time = 0;
output_index = 1;
for source_index = 1:length(MAG)
    mag_timestamp = MAG(source_index,2);
    if (mag_timestamp ~= last_time)
        mag_data.time_us(output_index,1) = mag_timestamp;
        mag_data.field_ga(output_index,:) = 0.001*[MAG(source_index,3),MAG(source_index,4),MAG(source_index,5)];
        last_time = mag_timestamp;
        output_index = output_index + 1;
    end
end

%% save GPS daa
clear gps_data;
gps_data.time_us = GPS(:,2);
gps_data.pos_error = GPA(:,4);
gps_data.spd_error = GPA(:,6);
gps_data.hgt_error = GPA(:,5);

% set reference point used to set NED origin when GPS accuracy is sufficient
gps_data.start_index = max(find(gps_data.pos_error < 5.0, 1 ),find(gps_data.spd_error < 1.0, 1 ));
gps_data.refLLH = [GPS(gps_data.start_index,8);GPS(gps_data.start_index,9);GPS(gps_data.start_index,10)];

% convert GPS data to NED
deg2rad = pi/180;
for index = 1:length(GPS)
    if (GPS(index,3) >= 3)
        gps_data.pos_ned(index,:) = LLH2NED([GPS(index,8);GPS(index,9);GPS(index,10)],gps_data.refLLH);
        gps_data.vel_ned(index,:) = [GPS(index,11).*cos(deg2rad*GPS(index,12)),GPS(index,11).*sin(deg2rad*GPS(index,12)),GPS(index,13)];
    else
        gps_data.pos_ned(index,:) = [0,0,0];
        gps_data.vel_ned(index,:) = [0,0,0];
    end
end

%% save range finder data
clear rng_data;
rng_data.time_us = RFND(:,2);
rng_data.dist = RFND(:,3);

%% save optical flow data
clear flow_data;
flow_data.time_us = OF(:,2);
flow_data.qual = OF(:,3)/255; % scale quality from 0 to 1
flow_data.flowX = OF(:,4); % optical flow rate about the X body axis (rad/sec)
flow_data.flowY = OF(:,5); % optical flow rate about the Y body axis (rad/sec)
flow_data.bodyX = OF(:,6); % angular rate about the X body axis (rad/sec)
flow_data.bodyY = OF(:,7); % angular rate about the Y body axis (rad/sec)

%% save visual odometry data
clear viso_data;
viso_data.time_us = VISO(:,2);
viso_data.dt = VISO(:,3); % time period the measurement was sampled across (sec)
viso_data.dAngX = VISO(:,4); % delta angle about the X body axis
viso_data.dAngY = VISO(:,5); % delta angle about the Y body axis
viso_data.dAngZ = VISO(:,6); % delta angle about the Z body axis
viso_data.dVelX = VISO(:,7); % delta velocity along the X body axis
viso_data.dVelY = VISO(:,8); % delta velocity along the Y body axis
viso_data.dVelZ = VISO(:,9); % delta velocity along the Z body axis
viso_data.qual = VISO(:,10)/100; % quality from 0 - 1

%% save data and clear workspace
clearvars -except baro_data imu_data mag_data gps_data rng_data flow_data viso_data;

save baro_data.mat baro_data;
save imu_data.mat imu_data;
save mag_data.mat mag_data;
save gps_data.mat gps_data;
save rng_data.mat rng_data;
save flow_data.mat flow_data;
save viso_data.mat viso_data;
