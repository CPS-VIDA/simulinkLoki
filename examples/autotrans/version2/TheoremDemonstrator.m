% Load the agent
load("LOKI_autotrans.mat","agentObj")

% Load the system settings
load('system_param_settings.mat')


% State sampling
state_sample = unifrnd(stateLowerLimits,stateUpperLimits)


% Network Actions
action = getAction(agentObj,state_sample);
critic = getCritic(agentObj);
state_value = getValue(critic,{state_sample}, action)
