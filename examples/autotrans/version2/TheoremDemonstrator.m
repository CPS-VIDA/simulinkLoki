% Load the agent

load("LOKI_autotrans.mat","agentObj")


% predictImNN = predict(agentObj,[100;100]);
action = getAction(agentObj,[100;100]);


critic = getCritic(agentObj);
state_value = getValue(critic,{[100;100]}, action)
