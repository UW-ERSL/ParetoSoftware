syms Kuu Kuv Kuw Kvu Kvv Kvw Kwu Kwv Kww real
syms x1 y1 z1 x2 y2 z2 real

KE = [Kuu Kuv Kuw; Kvu Kvv Kvw; Kwu Kwv Kww];

method = 3;

if (method == 1)
    KW = KE;
    
elseif (method == 2)
    R1 = [0 z1 -y1; -z1 0 x1; y1 -x1 0];
    R2 = [0 z2 -y2; -z2 0 x2; y2 -x2 0];
    W1 = [eye(3,3) R1];
    W2 = [eye(3,3) R2];
    KW = W1'*KE*W2;
elseif (method == 3)
    R1 = [0 z1 -y1; -z1 0 x1; y1 -x1 0];
    R2 = [0 z2 -y2; -z2 0 x2; y2 -x2 0];

    A1 = [x1 0 0; 0 y1 0; 0 0 z1];
    A2 = [x2 0 0; 0 y2 0; 0 0 z2];

    W1 = [eye(3,3) R1 A1];
    W2 = [eye(3,3) R2 A2 ];
    KW = W1'*KE*W2;

elseif (method == 4)
    R1 = [0 z1 -y1; -z1 0 x1; y1 -x1 0];
    R2 = [0 z2 -y2; -z2 0 x2; y2 -x2 0];

    A1 = [x1 0 0; 0 y1 0; 0 0 z1];
    A2 = [x2 0 0; 0 y2 0; 0 0 z2];

    S1 = [y1/2 z1/2 0; x1/2 0 z1/2; 0 x1/2 y1/2];
    S2 = [y2/2 z2/2 0; x2/2 0 z2/2; 0 x2/2 y2/2];

   
    W1 = [eye(3,3) R1 A1 S1];
    W2 = [eye(3,3) R2 A2 S2];
    KW = W1'*KE*W2;
end
ccode(KW)