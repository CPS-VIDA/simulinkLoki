clc;
clear;
rng(100);
vmax = 120;
Tf = 20;
Ts = 1.0;
tau = 20;
actionLowerLimits = [0 0]';
actionUpperLimits = [100 1]';

stateLowerLimits = [0 0]';
stateUpperLimits = [5500 130]';
save('system_param_settings', 'actionLowerLimits', 'actionUpperLimits', 'stateLowerLimits', 'stateUpperLimits')

%% Define state (observation) and action space
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
%% Make critic network
deno = 1./(obsInfo.UpperLimit-obsInfo.LowerLimit);
invStateScaling = 2.*deno;
invStateBias =  -deno.*(obsInfo.LowerLimit + obsInfo.UpperLimit);

actionDeno = 1./(actInfo.UpperLimit - actInfo.LowerLimit);
invActionScaling = 2.*actionDeno;
invActionBias = -actionDeno.*(actInfo.UpperLimit + actInfo.LowerLimit);

% 'Normalization','rescale-zero-one',...
% 'Min',transpose(obsInfo.LowerLimit),...
% 'Max',transpose(obsInfo.UpperLimit),
% 'Normalization','rescale-zero-one',...
% 'Min',transpose(obsInfo.LowerLimit),...
% 'Max',transpose(obsInfo.UpperLimit),...
% 'Normalization','rescale-zero-one',...
% 'Min',transpose(actInfo.LowerLimit),...
% 'Max',transpose(actInfo.UpperLimit),...
statePath = [
    featureInputLayer(obsInfo.Dimension(1),Name="netObsIn")   
    fullyConnectedLayer(2)
    reluLayer
    fullyConnectedLayer(2,Name="CriticStateFC2")];

actionPath = [
    featureInputLayer(actInfo.Dimension(1),Name="netActIn")   
    fullyConnectedLayer(2,Name="CriticActionFC1")];

commonPath = [
    additionLayer(2,Name="add")
    reluLayer
    fullyConnectedLayer(1,Name="CriticOutput")];

criticNetwork = layerGraph();
criticNetwork = addLayers(criticNetwork,statePath);
criticNetwork = addLayers(criticNetwork,actionPath);
criticNetwork = addLayers(criticNetwork,commonPath);

criticNetwork = connectLayers(criticNetwork, ...
    "CriticStateFC2", ...
    "add/in1");
criticNetwork = connectLayers(criticNetwork, ...
    "CriticActionFC1", ...
    "add/in2");

criticNetwork = dlnetwork(criticNetwork);
summary(criticNetwork);

critic = rlQValueFunction(criticNetwork,obsInfo,actInfo, ...
    ObservationInputNames="netObsIn", ...
    ActionInputNames="netActIn");
% getValue(critic, ...
%  {rand(obsInfo.Dimension)}, ...
%  {rand(actInfo.Dimension)})
%% Make Actor Network
actionScaling = 0.5 *(actInfo.UpperLimit-actInfo.LowerLimit);
actionBias= 0.5*(actInfo.UpperLimit+actInfo.LowerLimit);
actorNetwork = [
    featureInputLayer(obsInfo.Dimension(1))
    fullyConnectedLayer(2)
    tanhLayer
    fullyConnectedLayer(actInfo.Dimension(1))
    ];
actorNetwork = dlnetwork(actorNetwork);
summary(actorNetwork);
actor = rlContinuousDeterministicActor(actorNetwork,obsInfo,actInfo);
a = [getAction(actor,{rand(obsInfo.Dimension)})];
fprintf('%s',mat2str(a{1},3));
%% Setup the RL agent
agentObj = rlDDPGAgent(actor,critic);

agentObj.SampleTime = Ts;

agentObj.AgentOptions.TargetSmoothFactor = 1e-3;
agentObj.AgentOptions.DiscountFactor = 1.0;
agentObj.AgentOptions.MiniBatchSize = 2;
agentObj.AgentOptions.ExperienceBufferLength = 1e6; 

agentObj.AgentOptions.NoiseOptions.Variance = 0.3;
agentObj.AgentOptions.NoiseOptions.VarianceDecayRate = 1e-5;

agentObj.AgentOptions.CriticOptimizerOptions.LearnRate = 1e-03;
agentObj.AgentOptions.CriticOptimizerOptions.GradientThreshold = 1;
agentObj.AgentOptions.ActorOptimizerOptions.LearnRate = 1e-04;
agentObj.AgentOptions.ActorOptimizerOptions.GradientThreshold = 1;
% getAction(agentObj,{rand(obsInfo.Dimension)})
%% Do the training  
trainOpts = rlTrainingOptions(...
    MaxEpisodes=100, ...
    MaxStepsPerEpisode=ceil(Tf/Ts), ...
    ScoreAveragingWindowLength=20, ...
    Verbose=false, ...
    Plots="training-progress",...
    StopTrainingCriteria="AverageReward",...
    StopTrainingValue=800);

%%
doTraining = true;
if doTraining
    % Train the agent.
    trainingStats = train(agentObj,env,trainOpts);
    save('LOKI_autotrans','agentObj');
else
    % Load the pretrained agent for the example.
    load("LOKI_autotrans.mat","agentObj")
end
%% Show the experiences

es = agentObj.ExperienceBuffer.allExperiences;
actions = [es.Action];





