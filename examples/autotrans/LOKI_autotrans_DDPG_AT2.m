clc;
clear;
rng(100);
rpm_threshold = 4750;
Tf = 30;
Ts = 5.0;
at2_tau = 20;
actionLowerLimits = [0 0]';
actionUpperLimits = [100 325]';

stateLowerLimits = [0 0]';
stateUpperLimits = [5000 125]';

criticHiddenLayerSize = 5;
actorHiddenLayerSize = 3;

%% Define state (observation) and action space
obsInfo = rlNumericSpec([2 1],...
    LowerLimit=[-1 -1]',...
    UpperLimit=[1 1]');
obsInfo.Name="observations";
obsInfo.Description="rpm, speed";

actInfo=rlNumericSpec([2 1],...
    LowerLimit=[-1 -1]',...
    UpperLimit=[1 1]'); %
actInfo.Name="throttle, brake";

env=rlSimulinkEnv("LOKI_autotrans_AT2","LOKI_autotrans_AT2/RL Agent",...
    obsInfo,actInfo);
%% Make critic network
% deno = 1./(obsInfo.UpperLimit-obsInfo.LowerLimit);
% invStateScaling = 2.*deno;
% invStateBias =  -deno.*(obsInfo.LowerLimit + obsInfo.UpperLimit);

% actionDeno = 1./(actInfo.UpperLimit - actInfo.LowerLimit);
% invActionScaling = 2.*actionDeno;
% invActionBias = -actionDeno.*(actInfo.UpperLimit + actInfo.LowerLimit);

% adding scaling layers or normalizing inputs DOES NOT WORK FOR RL AGENTS in MATLAB!
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
    fullyConnectedLayer(2*criticHiddenLayerSize)
    reluLayer
    fullyConnectedLayer(criticHiddenLayerSize,Name="CriticStateFC2")];

actionPath = [
    featureInputLayer(actInfo.Dimension(1),Name="netActIn")   
    fullyConnectedLayer(criticHiddenLayerSize,Name="CriticActionFC1")];

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
%summary(criticNetwork);

critic = rlQValueFunction(criticNetwork,obsInfo,actInfo, ...
    ObservationInputNames="netObsIn", ...
    ActionInputNames="netActIn");
% getValue(critic, ...
%  {rand(obsInfo.Dimension)}, ...
%  {rand(actInfo.Dimension)})
%% Make Actor Network
%actionScaling = 0.5 *(actInfo.UpperLimit-actInfo.LowerLimit);
%actionBias= 0.5*(actInfo.UpperLimit+actInfo.LowerLimit);
actorNetwork = [
    featureInputLayer(obsInfo.Dimension(1))
    fullyConnectedLayer(actorHiddenLayerSize)
    tanhLayer
    fullyConnectedLayer(actInfo.Dimension(1))
    ];
actorNetwork = dlnetwork(actorNetwork);
%summary(actorNetwork);
actor = rlContinuousDeterministicActor(actorNetwork,obsInfo,actInfo);
% a = [getAction(actor,{rand(obsInfo.Dimension)})];
% fprintf('%s.\n',mat2str(a{1},3));
%% Setup the RL agent
agentObj = rlDDPGAgent(actor,critic);
agentObj.SampleTime = Ts;

agentObj.AgentOptions.TargetSmoothFactor = 1e-3;
agentObj.AgentOptions.DiscountFactor = 1.0;
agentObj.AgentOptions.MiniBatchSize = 50;
agentObj.AgentOptions.ExperienceBufferLength = 1e6; 

agentObj.AgentOptions.NoiseOptions.Variance = 0.4;
agentObj.AgentOptions.NoiseOptions.VarianceDecayRate = 1e-4;

agentObj.AgentOptions.CriticOptimizerOptions.LearnRate = 1e-03;
agentObj.AgentOptions.CriticOptimizerOptions.GradientThreshold = 10;
agentObj.AgentOptions.ActorOptimizerOptions.LearnRate = 1e-03;
agentObj.AgentOptions.ActorOptimizerOptions.GradientThreshold = 1;
% getAction(agentObj,{rand(obsInfo.Dimension)})

%% Do the training  
% Plots="training-progress",...
trainOpts = rlTrainingOptions(...
    MaxEpisodes=200, ...
    MaxStepsPerEpisode=ceil(Tf/Ts), ...
    ScoreAveragingWindowLength=50, ...
    Verbose=1, ...
    Plots='none',...
    StopTrainingCriteria="AverageReward",...    
    StopTrainingValue=0.03);
    % UseParallel=true,...
%%
startTime = tic;
% Train the agent.
trainingStats = train(agentObj,env,trainOpts);
trainingTime = toc(startTime);
fprintf('Finished Training. Total Time Taken = %d.\n',trainingTime);
%% Show the experiences
%
% es = agentObj.ExperienceBuffer.allExperiences;
% actions = [es.Action];
save('./results/LOKI_autotrans_DDPG_AT2','agentObj','trainingStats','trainingTime');
% agentObj.generatePolicyFunction('FunctionName','autotrans_DDPG_evaluate_policy_AT2','MATFileName','LOKI_');





