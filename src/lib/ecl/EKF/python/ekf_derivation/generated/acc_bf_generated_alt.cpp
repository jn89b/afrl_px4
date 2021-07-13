// Sub Expressions
const float HK0 = q2*vd;
const float HK1 = ve - vwe;
const float HK2 = HK1*q3;
const float HK3 = HK0 - HK2;
const float HK4 = 2*Kaccx;
const float HK5 = q3*vd;
const float HK6 = HK1*q2 + HK5;
const float HK7 = q0*vd;
const float HK8 = HK1*q1;
const float HK9 = vn - vwn;
const float HK10 = HK9*q2;
const float HK11 = 2*HK10;
const float HK12 = HK11 + HK7 - HK8;
const float HK13 = q1*vd;
const float HK14 = HK9*q3;
const float HK15 = HK1*q0 + HK13 - 2*HK14;
const float HK16 = 2*powf(q2, 2);
const float HK17 = 2*powf(q3, 2);
const float HK18 = HK17 - 1;
const float HK19 = HK16 + HK18;
const float HK20 = HK19*Kaccx;
const float HK21 = q0*q3;
const float HK22 = q1*q2;
const float HK23 = HK21 + HK22;
const float HK24 = HK23*HK4;
const float HK25 = q0*q2;
const float HK26 = q1*q3;
const float HK27 = HK25 - HK26;
const float HK28 = 2*HK3;
const float HK29 = 2*HK12;
const float HK30 = 2*HK27;
const float HK31 = 2*HK23;
const float HK32 = 2*HK6;
const float HK33 = 2*HK15;
const float HK34 = HK19*P(0,22) - HK31*P(0,23) + HK31*P(0,5) + HK32*P(0,1) + HK33*P(0,3);
const float HK35 = -HK16 - HK17 + 1;
const float HK36 = -HK0;
const float HK37 = 2*HK2 + 2*HK36;
const float HK38 = -2*HK11 - 2*HK7 + 2*HK8;
const float HK39 = -2*HK25 + 2*HK26;
const float HK40 = HK31*P(5,23);
const float HK41 = HK19*P(22,23) - HK31*P(23,23) + HK32*P(1,23) + HK33*P(3,23) + HK40;
const float HK42 = powf(Kaccx, 2);
const float HK43 = HK31*HK42;
const float HK44 = HK19*P(5,22) + HK31*P(5,5) + HK32*P(1,5) + HK33*P(3,5) - HK40;
const float HK45 = HK19*P(6,22) + HK31*P(5,6) - HK31*P(6,23) + HK32*P(1,6) + HK33*P(3,6);
const float HK46 = HK19*P(1,22) - HK31*P(1,23) + HK31*P(1,5) + HK32*P(1,1) + HK33*P(1,3);
const float HK47 = HK19*P(4,22);
const float HK48 = -HK31*P(4,23) + HK31*P(4,5) + HK32*P(1,4) + HK33*P(3,4) + HK47;
const float HK49 = HK19*HK42;
const float HK50 = HK19*P(22,22) - HK31*P(22,23) + HK31*P(5,22) + HK32*P(1,22) + HK33*P(3,22);
const float HK51 = HK19*P(3,22) - HK31*P(3,23) + HK31*P(3,5) + HK32*P(1,3) + HK33*P(3,3);
const float HK52 = HK19*P(2,22) - HK31*P(2,23) + HK31*P(2,5) + HK32*P(1,2) + HK33*P(2,3);
const float HK53 = Kaccx/(HK28*HK42*(HK34 + HK35*P(0,4) + HK37*P(0,0) + HK38*P(0,2) + HK39*P(0,6)) + HK29*HK42*(HK35*P(2,4) + HK37*P(0,2) + HK38*P(2,2) + HK39*P(2,6) + HK52) + HK30*HK42*(HK35*P(4,6) + HK37*P(0,6) + HK38*P(2,6) + HK39*P(6,6) + HK45) - HK32*HK42*(HK35*P(1,4) + HK37*P(0,1) + HK38*P(1,2) + HK39*P(1,6) + HK46) - HK33*HK42*(HK35*P(3,4) + HK37*P(0,3) + HK38*P(2,3) + HK39*P(3,6) + HK51) + HK43*(HK35*P(4,23) + HK37*P(0,23) + HK38*P(2,23) + HK39*P(6,23) + HK41) - HK43*(HK35*P(4,5) + HK37*P(0,5) + HK38*P(2,5) + HK39*P(5,6) + HK44) - HK49*(HK35*P(4,22) + HK37*P(0,22) + HK38*P(2,22) + HK39*P(6,22) + HK50) + HK49*(HK35*P(4,4) + HK37*P(0,4) + HK38*P(2,4) + HK39*P(4,6) + HK48) - R_ACC);
const float HK54 = HK13 - HK14;
const float HK55 = 2*Kaccy;
const float HK56 = HK10 + HK7 - 2*HK8;
const float HK57 = HK5 + HK9*q1;
const float HK58 = 2*HK2 + HK36 + HK9*q0;
const float HK59 = HK21 - HK22;
const float HK60 = HK55*HK59;
const float HK61 = HK18 + 2*powf(q1, 2);
const float HK62 = HK61*Kaccy;
const float HK63 = q0*q1 + q2*q3;
const float HK64 = 2*HK63;
const float HK65 = 2*HK59;
const float HK66 = 2*HK57;
const float HK67 = 2*HK54;
const float HK68 = 2*HK56;
const float HK69 = 2*HK58;
const float HK70 = HK61*P(0,23) - HK61*P(0,5) + HK64*P(0,6) + HK65*P(0,22) - HK65*P(0,4) + HK66*P(0,2) + HK67*P(0,0) + HK68*P(0,1) - HK69*P(0,3);
const float HK71 = powf(Kaccy, 2);
const float HK72 = -HK61*P(5,6) + HK61*P(6,23) + HK64*P(6,6) - HK65*P(4,6) + HK65*P(6,22) + HK66*P(2,6) + HK67*P(0,6) + HK68*P(1,6) - HK69*P(3,6);
const float HK73 = HK65*P(4,22);
const float HK74 = HK61*P(22,23) - HK61*P(5,22) + HK64*P(6,22) + HK65*P(22,22) + HK66*P(2,22) + HK67*P(0,22) + HK68*P(1,22) - HK69*P(3,22) - HK73;
const float HK75 = HK65*HK71;
const float HK76 = HK61*P(4,23) - HK61*P(4,5) + HK64*P(4,6) - HK65*P(4,4) + HK66*P(2,4) + HK67*P(0,4) + HK68*P(1,4) - HK69*P(3,4) + HK73;
const float HK77 = HK61*P(2,23) - HK61*P(2,5) + HK64*P(2,6) + HK65*P(2,22) - HK65*P(2,4) + HK66*P(2,2) + HK67*P(0,2) + HK68*P(1,2) - HK69*P(2,3);
const float HK78 = HK61*P(5,23);
const float HK79 = HK61*P(23,23) + HK64*P(6,23) + HK65*P(22,23) - HK65*P(4,23) + HK66*P(2,23) + HK67*P(0,23) + HK68*P(1,23) - HK69*P(3,23) - HK78;
const float HK80 = HK61*HK71;
const float HK81 = -HK61*P(5,5) + HK64*P(5,6) - HK65*P(4,5) + HK65*P(5,22) + HK66*P(2,5) + HK67*P(0,5) + HK68*P(1,5) - HK69*P(3,5) + HK78;
const float HK82 = HK61*P(1,23) - HK61*P(1,5) + HK64*P(1,6) + HK65*P(1,22) - HK65*P(1,4) + HK66*P(1,2) + HK67*P(0,1) + HK68*P(1,1) - HK69*P(1,3);
const float HK83 = HK61*P(3,23) - HK61*P(3,5) + HK64*P(3,6) + HK65*P(3,22) - HK65*P(3,4) + HK66*P(2,3) + HK67*P(0,3) + HK68*P(1,3) - HK69*P(3,3);
const float HK84 = Kaccy/(HK64*HK71*HK72 + HK66*HK71*HK77 + HK67*HK70*HK71 + HK68*HK71*HK82 - HK69*HK71*HK83 + HK74*HK75 - HK75*HK76 + HK79*HK80 - HK80*HK81 + R_ACC);


// Observation Jacobians - axis 0
Hfusion.at<0>() = HK3*HK4;
Hfusion.at<1>() = -HK4*HK6;
Hfusion.at<2>() = HK12*HK4;
Hfusion.at<3>() = -HK15*HK4;
Hfusion.at<4>() = HK20;
Hfusion.at<5>() = -HK24;
Hfusion.at<6>() = HK27*HK4;
Hfusion.at<7>() = 0;
Hfusion.at<8>() = 0;
Hfusion.at<9>() = 0;
Hfusion.at<10>() = 0;
Hfusion.at<11>() = 0;
Hfusion.at<12>() = 0;
Hfusion.at<13>() = 0;
Hfusion.at<14>() = 0;
Hfusion.at<15>() = 0;
Hfusion.at<16>() = 0;
Hfusion.at<17>() = 0;
Hfusion.at<18>() = 0;
Hfusion.at<19>() = 0;
Hfusion.at<20>() = 0;
Hfusion.at<21>() = 0;
Hfusion.at<22>() = -HK20;
Hfusion.at<23>() = HK24;


// Kalman gains - axis 0
Kfusion(0) = HK53*(-HK19*P(0,4) - HK28*P(0,0) - HK29*P(0,2) - HK30*P(0,6) + HK34);
Kfusion(1) = HK53*(-HK19*P(1,4) - HK28*P(0,1) - HK29*P(1,2) - HK30*P(1,6) + HK46);
Kfusion(2) = HK53*(-HK19*P(2,4) - HK28*P(0,2) - HK29*P(2,2) - HK30*P(2,6) + HK52);
Kfusion(3) = HK53*(-HK19*P(3,4) - HK28*P(0,3) - HK29*P(2,3) - HK30*P(3,6) + HK51);
Kfusion(4) = HK53*(-HK19*P(4,4) - HK28*P(0,4) - HK29*P(2,4) - HK30*P(4,6) + HK48);
Kfusion(5) = HK53*(-HK19*P(4,5) - HK28*P(0,5) - HK29*P(2,5) - HK30*P(5,6) + HK44);
Kfusion(6) = HK53*(-HK19*P(4,6) - HK28*P(0,6) - HK29*P(2,6) - HK30*P(6,6) + HK45);
Kfusion(7) = HK53*(-HK19*P(4,7) + HK19*P(7,22) - HK28*P(0,7) - HK29*P(2,7) - HK30*P(6,7) + HK31*P(5,7) - HK31*P(7,23) + HK32*P(1,7) + HK33*P(3,7));
Kfusion(8) = HK53*(-HK19*P(4,8) + HK19*P(8,22) - HK28*P(0,8) - HK29*P(2,8) - HK30*P(6,8) + HK31*P(5,8) - HK31*P(8,23) + HK32*P(1,8) + HK33*P(3,8));
Kfusion(9) = HK53*(-HK19*P(4,9) + HK19*P(9,22) - HK28*P(0,9) - HK29*P(2,9) - HK30*P(6,9) + HK31*P(5,9) - HK31*P(9,23) + HK32*P(1,9) + HK33*P(3,9));
Kfusion(10) = HK53*(HK19*P(10,22) - HK19*P(4,10) - HK28*P(0,10) - HK29*P(2,10) - HK30*P(6,10) - HK31*P(10,23) + HK31*P(5,10) + HK32*P(1,10) + HK33*P(3,10));
Kfusion(11) = HK53*(HK19*P(11,22) - HK19*P(4,11) - HK28*P(0,11) - HK29*P(2,11) - HK30*P(6,11) - HK31*P(11,23) + HK31*P(5,11) + HK32*P(1,11) + HK33*P(3,11));
Kfusion(12) = HK53*(HK19*P(12,22) - HK19*P(4,12) - HK28*P(0,12) - HK29*P(2,12) - HK30*P(6,12) - HK31*P(12,23) + HK31*P(5,12) + HK32*P(1,12) + HK33*P(3,12));
Kfusion(13) = HK53*(HK19*P(13,22) - HK19*P(4,13) - HK28*P(0,13) - HK29*P(2,13) - HK30*P(6,13) - HK31*P(13,23) + HK31*P(5,13) + HK32*P(1,13) + HK33*P(3,13));
Kfusion(14) = HK53*(HK19*P(14,22) - HK19*P(4,14) - HK28*P(0,14) - HK29*P(2,14) - HK30*P(6,14) - HK31*P(14,23) + HK31*P(5,14) + HK32*P(1,14) + HK33*P(3,14));
Kfusion(15) = HK53*(HK19*P(15,22) - HK19*P(4,15) - HK28*P(0,15) - HK29*P(2,15) - HK30*P(6,15) - HK31*P(15,23) + HK31*P(5,15) + HK32*P(1,15) + HK33*P(3,15));
Kfusion(16) = HK53*(HK19*P(16,22) - HK19*P(4,16) - HK28*P(0,16) - HK29*P(2,16) - HK30*P(6,16) - HK31*P(16,23) + HK31*P(5,16) + HK32*P(1,16) + HK33*P(3,16));
Kfusion(17) = HK53*(HK19*P(17,22) - HK19*P(4,17) - HK28*P(0,17) - HK29*P(2,17) - HK30*P(6,17) - HK31*P(17,23) + HK31*P(5,17) + HK32*P(1,17) + HK33*P(3,17));
Kfusion(18) = HK53*(HK19*P(18,22) - HK19*P(4,18) - HK28*P(0,18) - HK29*P(2,18) - HK30*P(6,18) - HK31*P(18,23) + HK31*P(5,18) + HK32*P(1,18) + HK33*P(3,18));
Kfusion(19) = HK53*(HK19*P(19,22) - HK19*P(4,19) - HK28*P(0,19) - HK29*P(2,19) - HK30*P(6,19) - HK31*P(19,23) + HK31*P(5,19) + HK32*P(1,19) + HK33*P(3,19));
Kfusion(20) = HK53*(HK19*P(20,22) - HK19*P(4,20) - HK28*P(0,20) - HK29*P(2,20) - HK30*P(6,20) - HK31*P(20,23) + HK31*P(5,20) + HK32*P(1,20) + HK33*P(3,20));
Kfusion(21) = HK53*(HK19*P(21,22) - HK19*P(4,21) - HK28*P(0,21) - HK29*P(2,21) - HK30*P(6,21) - HK31*P(21,23) + HK31*P(5,21) + HK32*P(1,21) + HK33*P(3,21));
Kfusion(22) = HK53*(-HK28*P(0,22) - HK29*P(2,22) - HK30*P(6,22) - HK47 + HK50);
Kfusion(23) = HK53*(-HK19*P(4,23) - HK28*P(0,23) - HK29*P(2,23) - HK30*P(6,23) + HK41);


// Observation Jacobians - axis 1
Hfusion.at<0>() = -HK54*HK55;
Hfusion.at<1>() = -HK55*HK56;
Hfusion.at<2>() = -HK55*HK57;
Hfusion.at<3>() = HK55*HK58;
Hfusion.at<4>() = HK60;
Hfusion.at<5>() = HK62;
Hfusion.at<6>() = -HK55*HK63;
Hfusion.at<7>() = 0;
Hfusion.at<8>() = 0;
Hfusion.at<9>() = 0;
Hfusion.at<10>() = 0;
Hfusion.at<11>() = 0;
Hfusion.at<12>() = 0;
Hfusion.at<13>() = 0;
Hfusion.at<14>() = 0;
Hfusion.at<15>() = 0;
Hfusion.at<16>() = 0;
Hfusion.at<17>() = 0;
Hfusion.at<18>() = 0;
Hfusion.at<19>() = 0;
Hfusion.at<20>() = 0;
Hfusion.at<21>() = 0;
Hfusion.at<22>() = -HK60;
Hfusion.at<23>() = -HK62;


// Kalman gains - axis 1
Kfusion(0) = -HK70*HK84;
Kfusion(1) = -HK82*HK84;
Kfusion(2) = -HK77*HK84;
Kfusion(3) = -HK83*HK84;
Kfusion(4) = -HK76*HK84;
Kfusion(5) = -HK81*HK84;
Kfusion(6) = -HK72*HK84;
Kfusion(7) = -HK84*(-HK61*P(5,7) + HK61*P(7,23) + HK64*P(6,7) - HK65*P(4,7) + HK65*P(7,22) + HK66*P(2,7) + HK67*P(0,7) + HK68*P(1,7) - HK69*P(3,7));
Kfusion(8) = -HK84*(-HK61*P(5,8) + HK61*P(8,23) + HK64*P(6,8) - HK65*P(4,8) + HK65*P(8,22) + HK66*P(2,8) + HK67*P(0,8) + HK68*P(1,8) - HK69*P(3,8));
Kfusion(9) = -HK84*(-HK61*P(5,9) + HK61*P(9,23) + HK64*P(6,9) - HK65*P(4,9) + HK65*P(9,22) + HK66*P(2,9) + HK67*P(0,9) + HK68*P(1,9) - HK69*P(3,9));
Kfusion(10) = -HK84*(HK61*P(10,23) - HK61*P(5,10) + HK64*P(6,10) + HK65*P(10,22) - HK65*P(4,10) + HK66*P(2,10) + HK67*P(0,10) + HK68*P(1,10) - HK69*P(3,10));
Kfusion(11) = -HK84*(HK61*P(11,23) - HK61*P(5,11) + HK64*P(6,11) + HK65*P(11,22) - HK65*P(4,11) + HK66*P(2,11) + HK67*P(0,11) + HK68*P(1,11) - HK69*P(3,11));
Kfusion(12) = -HK84*(HK61*P(12,23) - HK61*P(5,12) + HK64*P(6,12) + HK65*P(12,22) - HK65*P(4,12) + HK66*P(2,12) + HK67*P(0,12) + HK68*P(1,12) - HK69*P(3,12));
Kfusion(13) = -HK84*(HK61*P(13,23) - HK61*P(5,13) + HK64*P(6,13) + HK65*P(13,22) - HK65*P(4,13) + HK66*P(2,13) + HK67*P(0,13) + HK68*P(1,13) - HK69*P(3,13));
Kfusion(14) = -HK84*(HK61*P(14,23) - HK61*P(5,14) + HK64*P(6,14) + HK65*P(14,22) - HK65*P(4,14) + HK66*P(2,14) + HK67*P(0,14) + HK68*P(1,14) - HK69*P(3,14));
Kfusion(15) = -HK84*(HK61*P(15,23) - HK61*P(5,15) + HK64*P(6,15) + HK65*P(15,22) - HK65*P(4,15) + HK66*P(2,15) + HK67*P(0,15) + HK68*P(1,15) - HK69*P(3,15));
Kfusion(16) = -HK84*(HK61*P(16,23) - HK61*P(5,16) + HK64*P(6,16) + HK65*P(16,22) - HK65*P(4,16) + HK66*P(2,16) + HK67*P(0,16) + HK68*P(1,16) - HK69*P(3,16));
Kfusion(17) = -HK84*(HK61*P(17,23) - HK61*P(5,17) + HK64*P(6,17) + HK65*P(17,22) - HK65*P(4,17) + HK66*P(2,17) + HK67*P(0,17) + HK68*P(1,17) - HK69*P(3,17));
Kfusion(18) = -HK84*(HK61*P(18,23) - HK61*P(5,18) + HK64*P(6,18) + HK65*P(18,22) - HK65*P(4,18) + HK66*P(2,18) + HK67*P(0,18) + HK68*P(1,18) - HK69*P(3,18));
Kfusion(19) = -HK84*(HK61*P(19,23) - HK61*P(5,19) + HK64*P(6,19) + HK65*P(19,22) - HK65*P(4,19) + HK66*P(2,19) + HK67*P(0,19) + HK68*P(1,19) - HK69*P(3,19));
Kfusion(20) = -HK84*(HK61*P(20,23) - HK61*P(5,20) + HK64*P(6,20) + HK65*P(20,22) - HK65*P(4,20) + HK66*P(2,20) + HK67*P(0,20) + HK68*P(1,20) - HK69*P(3,20));
Kfusion(21) = -HK84*(HK61*P(21,23) - HK61*P(5,21) + HK64*P(6,21) + HK65*P(21,22) - HK65*P(4,21) + HK66*P(2,21) + HK67*P(0,21) + HK68*P(1,21) - HK69*P(3,21));
Kfusion(22) = -HK74*HK84;
Kfusion(23) = -HK79*HK84;


// Observation Jacobians - axis 2


// Kalman gains - axis 2


