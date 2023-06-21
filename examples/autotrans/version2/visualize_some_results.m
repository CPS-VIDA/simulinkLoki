%% Load the agent
load("LOKI_autotrans.mat","agentObj")

% Load the system settings
load('system_param_settings.mat')
vmax = 120;
Tf = 20;
Ts = 1.0;
tau = 20;
initrpm=1000   % Init states for RPM, 600 is the lower bound. 

% State sampling
% state_sample = unifrnd(stateLowerLimits,stateUpperLimits)

%% init env
obsInfo = rlNumericSpec([2 1],...
    LowerLimit=stateLowerLimits,...
    UpperLimit=stateUpperLimits);
obsInfo.Name="observations";
obsInfo.Description="rpm, speed";

actInfo=rlNumericSpec([2 1],...
    LowerLimit=actionLowerLimits,...
    UpperLimit=actionUpperLimits); % brake upper limit should be 325
actInfo.Name="throttle, brake";

env=rlSimulinkEnv("LOKI_autotrans2","LOKI_autotrans2/RL Agent",...
    obsInfo,actInfo);

%% define sim options
simOpts = rlSimulationOptions(...
    MaxSteps=30,...
    NumSimulations=1)

experience = sim(env,agentObj,simOpts)


blah = load('600_1.mat')
y3 = blah.ans.Data
plot(ts,y1(:,2),'-r.',ts,y2(:,2),'-b.',ts,y3(:,2),'-m.')
ts = blah.ans.Time
