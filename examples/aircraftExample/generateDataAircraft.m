
clc;
clear;
rng(109);
%%
Beta = 426.4352;

Gamma = 0.01;

Ka = 0.677;
% 
% Kf = -1.746;
% 
% Ki = -3.864;
% Kq = 0.8156;

Md = -6.8847;

Mq = -0.6571;

Mw = -0.00592;

Sa = 0.005236;

Swg = 3;

Ta = 0.05;

Tal = 0.3959;

Ts = 0.1;

Uo = 689.4;

Vto = 690.4;

W1 = 2.971;

W2 = 4.144;

Wa = 10;

Zd = -63.9979;

Zw = -0.6385;

a = 2.5348;

b = 64.13;

cmdgain = 0.034909544727280771;

g = 32.2;

%%
Params = {'Kf','Ki','Kq'};
ParamRanges = [-10 0;...
               -10 0;...
                 0 10];
M = BreachSimulinkSystem('slexAircraftExample', Params);
%%

input_gen.type = 'UniStep';
numCP = 5;
input_gen.cp = numCP;
M.SetInputGen(input_gen);
M.PrintParams;
paramList = {};
for ii=1:numCP
    paramList{end+1} = sprintf('u_u%d',ii-1);
end
inputRanges = [-0.5*ones(numCP,1) 0.5*ones(numCP,1)];

Params = [Params paramList];
ParamRanges = [ParamRanges;inputRanges];

M.SetParamRanges(Params, ParamRanges);
M.PrintParams;


%%
M.QuasiRandomSample(50);
M.Sim;
M.PlotSignals;





















