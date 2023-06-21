
%% Load the agent
rng(10);  %was 100

load("LOKI_autotrans_500.mat","agentObj")

% Load the system settings
load('system_param_settings.mat')
vmax = 120;
Tf = 20;
Ts = 1.0;
tau = 20;
initrpm=1000;   % Init states for RPM, 1000 is the training value. 600 is the lower bound. 

% State sampling
% state_sample = unifrnd(stateLowerLimits,stateUpperLimits)

% Training init state (the origin one)
state_sample = [1000; 0];   % speed always starts from 0 in this case study.
state_sample_new = [600; 0];

% Network Actions
critic = getCritic(agentObj);
action = getAction(agentObj,state_sample);
state_value_origin = getValue(critic,{state_sample}, action)

action2 = getAction(agentObj, state_sample_new);
state_value_new = getValue(critic, {state_sample_new}, action2)

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

%% Test the theorem

beta = abs(state_value_new - state_value_origin)


%% Sim for on the new init state
experience = sim(env,agentObj);


rho_truth = experience.Reward.Data(20)


