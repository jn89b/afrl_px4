t2 = q0*q3*2.0;
t3 = q1*q2*2.0;
t4 = t2+t3;
t5 = q0*q0;
t6 = q1*q1;
t7 = q2*q2;
t8 = q3*q3;
t9 = t5+t6-t7-t8;
t10 = q0*q2*2.0;
t25 = q1*q3*2.0;
t11 = t10-t25;
t12 = q3*ve*2.0;
t13 = q0*vn*2.0;
t26 = q2*vd*2.0;
t14 = t12+t13-t26;
t15 = q3*vd*2.0;
t16 = q2*ve*2.0;
t17 = q1*vn*2.0;
t18 = t15+t16+t17;
t19 = q0*vd*2.0;
t20 = q2*vn*2.0;
t27 = q1*ve*2.0;
t21 = t19+t20-t27;
t22 = q1*vd*2.0;
t23 = q0*ve*2.0;
t28 = q3*vn*2.0;
t24 = t22+t23-t28;
t29 = P[0][0]*t14;
t30 = P[1][1]*t18;
t31 = P[4][5]*t9;
t32 = P[5][5]*t4;
t33 = P[0][5]*t14;
t34 = P[1][5]*t18;
t35 = P[3][5]*t24;
t79 = P[6][5]*t11;
t80 = P[2][5]*t21;
t36 = t31+t32+t33+t34+t35-t79-t80;
t37 = t4*t36;
t38 = P[4][6]*t9;
t39 = P[5][6]*t4;
t40 = P[0][6]*t14;
t41 = P[1][6]*t18;
t42 = P[3][6]*t24;
t81 = P[6][6]*t11;
t82 = P[2][6]*t21;
t43 = t38+t39+t40+t41+t42-t81-t82;
t44 = P[4][0]*t9;
t45 = P[5][0]*t4;
t46 = P[1][0]*t18;
t47 = P[3][0]*t24;
t84 = P[6][0]*t11;
t85 = P[2][0]*t21;
t48 = t29+t44+t45+t46+t47-t84-t85;
t49 = t14*t48;
t50 = P[4][1]*t9;
t51 = P[5][1]*t4;
t52 = P[0][1]*t14;
t53 = P[3][1]*t24;
t86 = P[6][1]*t11;
t87 = P[2][1]*t21;
t54 = t30+t50+t51+t52+t53-t86-t87;
t55 = t18*t54;
t56 = P[4][2]*t9;
t57 = P[5][2]*t4;
t58 = P[0][2]*t14;
t59 = P[1][2]*t18;
t60 = P[3][2]*t24;
t78 = P[2][2]*t21;
t88 = P[6][2]*t11;
t61 = t56+t57+t58+t59+t60-t78-t88;
t62 = P[4][3]*t9;
t63 = P[5][3]*t4;
t64 = P[0][3]*t14;
t65 = P[1][3]*t18;
t66 = P[3][3]*t24;
t90 = P[6][3]*t11;
t91 = P[2][3]*t21;
t67 = t62+t63+t64+t65+t66-t90-t91;
t68 = t24*t67;
t69 = P[4][4]*t9;
t70 = P[5][4]*t4;
t71 = P[0][4]*t14;
t72 = P[1][4]*t18;
t73 = P[3][4]*t24;
t92 = P[6][4]*t11;
t93 = P[2][4]*t21;
t74 = t69+t70+t71+t72+t73-t92-t93;
t75 = t9*t74;
t83 = t11*t43;
t89 = t21*t61;
t76 = R_VEL+t37+t49+t55+t68+t75-t83-t89;
t77 = 1.0/t76;
A0[0][0] = t77*(t29+P[0][5]*t4+P[0][4]*t9-P[0][6]*t11+P[0][1]*t18-P[0][2]*t21+P[0][3]*t24);
A0[1][0] = t77*(t30+P[1][5]*t4+P[1][4]*t9+P[1][0]*t14-P[1][6]*t11-P[1][2]*t21+P[1][3]*t24);
A0[2][0] = t77*(-t78+P[2][5]*t4+P[2][4]*t9+P[2][0]*t14-P[2][6]*t11+P[2][1]*t18+P[2][3]*t24);
A0[3][0] = t77*(t66+P[3][5]*t4+P[3][4]*t9+P[3][0]*t14-P[3][6]*t11+P[3][1]*t18-P[3][2]*t21);
A0[4][0] = t77*(t69+P[4][5]*t4+P[4][0]*t14-P[4][6]*t11+P[4][1]*t18-P[4][2]*t21+P[4][3]*t24);
A0[5][0] = t77*(t32+P[5][4]*t9+P[5][0]*t14-P[5][6]*t11+P[5][1]*t18-P[5][2]*t21+P[5][3]*t24);
A0[6][0] = t77*(-t81+P[6][5]*t4+P[6][4]*t9+P[6][0]*t14+P[6][1]*t18-P[6][2]*t21+P[6][3]*t24);
A0[7][0] = t77*(P[7][5]*t4+P[7][4]*t9+P[7][0]*t14-P[7][6]*t11+P[7][1]*t18-P[7][2]*t21+P[7][3]*t24);
A0[8][0] = t77*(P[8][5]*t4+P[8][4]*t9+P[8][0]*t14-P[8][6]*t11+P[8][1]*t18-P[8][2]*t21+P[8][3]*t24);
A0[9][0] = t77*(P[9][5]*t4+P[9][4]*t9+P[9][0]*t14-P[9][6]*t11+P[9][1]*t18-P[9][2]*t21+P[9][3]*t24);
A0[10][0] = t77*(P[10][5]*t4+P[10][4]*t9+P[10][0]*t14-P[10][6]*t11+P[10][1]*t18-P[10][2]*t21+P[10][3]*t24);
A0[11][0] = t77*(P[11][5]*t4+P[11][4]*t9+P[11][0]*t14-P[11][6]*t11+P[11][1]*t18-P[11][2]*t21+P[11][3]*t24);
A0[12][0] = t77*(P[12][5]*t4+P[12][4]*t9+P[12][0]*t14-P[12][6]*t11+P[12][1]*t18-P[12][2]*t21+P[12][3]*t24);
A0[13][0] = t77*(P[13][5]*t4+P[13][4]*t9+P[13][0]*t14-P[13][6]*t11+P[13][1]*t18-P[13][2]*t21+P[13][3]*t24);
A0[14][0] = t77*(P[14][5]*t4+P[14][4]*t9+P[14][0]*t14-P[14][6]*t11+P[14][1]*t18-P[14][2]*t21+P[14][3]*t24);
A0[15][0] = t77*(P[15][5]*t4+P[15][4]*t9+P[15][0]*t14-P[15][6]*t11+P[15][1]*t18-P[15][2]*t21+P[15][3]*t24);
A0[16][0] = t77*(P[16][5]*t4+P[16][4]*t9+P[16][0]*t14-P[16][6]*t11+P[16][1]*t18-P[16][2]*t21+P[16][3]*t24);
A0[17][0] = t77*(P[17][5]*t4+P[17][4]*t9+P[17][0]*t14-P[17][6]*t11+P[17][1]*t18-P[17][2]*t21+P[17][3]*t24);
A0[18][0] = t77*(P[18][5]*t4+P[18][4]*t9+P[18][0]*t14-P[18][6]*t11+P[18][1]*t18-P[18][2]*t21+P[18][3]*t24);
A0[19][0] = t77*(P[19][5]*t4+P[19][4]*t9+P[19][0]*t14-P[19][6]*t11+P[19][1]*t18-P[19][2]*t21+P[19][3]*t24);
A0[20][0] = t77*(P[20][5]*t4+P[20][4]*t9+P[20][0]*t14-P[20][6]*t11+P[20][1]*t18-P[20][2]*t21+P[20][3]*t24);
A0[21][0] = t77*(P[21][5]*t4+P[21][4]*t9+P[21][0]*t14-P[21][6]*t11+P[21][1]*t18-P[21][2]*t21+P[21][3]*t24);
A0[22][0] = t77*(P[22][5]*t4+P[22][4]*t9+P[22][0]*t14-P[22][6]*t11+P[22][1]*t18-P[22][2]*t21+P[22][3]*t24);
A0[23][0] = t77*(P[23][5]*t4+P[23][4]*t9+P[23][0]*t14-P[23][6]*t11+P[23][1]*t18-P[23][2]*t21+P[23][3]*t24);
