rng(10);  %was 100

%% Load the agent
load("LOKI_autotrans_trained.mat","agentObj")

% Load the system settings
load('system_param_settings.mat')
vmax = 120;
Tf = 20;
Ts = 1.0;
tau = 20;
initrpm=1000;   % Init states for RPM, 600 is the lower bound. 

% State sampling
% state_sample = unifrnd(stateLowerLimits,stateUpperLimits)

% Training init state (the origin one)
state_sample = [initrpm; 0];   % speed always starts from 0 in this case study.


% Network Actions
critic = getCritic(agentObj);
action = getAction(agentObj,state_sample);
state_value_origin = getValue(critic,{state_sample}, action)


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

%% Get value diff
% Sample different states and get the value diff between two iters
% If only one ini state, then calculate one diff is enough?
agentObj2 = load("LOKI_autotrans_iter2.mat","agentObj").agentObj;
% Network Actions
critic2 = getCritic(agentObj2);
action2 = getAction(agentObj2,state_sample);
state_value_origin_iter2 = getValue(critic2,{state_sample}, action2)
value_diff_between_two_iter = abs(state_value_origin_iter2 - state_value_origin)

%% Test the theorem
gamma = 0.9;  % discount factor of the agent
alpha =  value_diff_between_two_iter * 2 * gamma / (1 - gamma);

initrpm_new = 700;
state_sample_new = [initrpm_new; 0];   % speed always starts from 0 in this case study.

action = getAction(agentObj,state_sample_new);
state_value_new = getValue(critic,{state_sample}, action)
beta = abs(state_value_new - state_value_origin)


%% Sim for on the new init state
initrpm = initrpm_new
simOpts = rlSimulationOptions(...
    MaxSteps=30,...
    NumSimulations=1);

load("LOKI_autotrans_trained.mat","agentObj")
experience = sim(env,agentObj,simOpts);
rho = 7.17129 / 1000;  % the old rho.

rho_thoerem = rho - (2*alpha + beta)/(gamma .^ tau) 

rho_truth = experience.Reward.Data(20)


