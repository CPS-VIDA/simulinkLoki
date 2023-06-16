clc;
clear;
rng(100);
Tf = 35;
Ts = 5.0;
specHorizon = 32.7;
stepsize = 0.02;
gearNum = 1;

% STL_ReadFile('AT.stl')
% 
% phi_autotrans = disp(at5);
% max_rob = 10000;

actionLowerLimits = [0 0]';
actionUpperLimits = [100 325]';

stateLowerLimits = [0 0]';
stateUpperLimits = [5000 125]';

criticHiddenLayerSize = 5;
actorHiddenLayerSize = 5;

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

env=rlSimulinkEnv("LOKI_autotrans_AT51","LOKI_autotrans_AT51/RL Agent",...
    obsInfo,actInfo);
%% Make critic network
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
actorNetwork = [
    featureInputLayer(obsInfo.Dimension(1))
    fullyConnectedLayer(actorHiddenLayerSize)
    tanhLayer
    fullyConnectedLayer(actInfo.Dimension(1))
    ];
actorNetwork = dlnetwork(actorNetwork);
%summary(actorNetwork);
actor = rlContinuousDeterministicActor(actorNetwork,obsInfo,actInfo);
%% Setup the RL agent
agentObj = rlDDPGAgent(actor,critic);
agentObj.SampleTime = Ts;

agentObj.AgentOptions.TargetUpdateFrequency = 1;
agentObj.AgentOptions.TargetSmoothFactor = 1.0;
agentObj.AgentOptions.DiscountFactor = 1.0;
agentObj.AgentOptions.MiniBatchSize = 100;
agentObj.AgentOptions.ExperienceBufferLength = 1e6; 


agentObj.AgentOptions.NoiseOptions.Variance = 0.5;
agentObj.AgentOptions.NoiseOptions.VarianceDecayRate = 1e-4;
% agentObj.AgentOptions.NoiseOptions.MeanAttractionConstant = 1e-3;

agentObj.AgentOptions.CriticOptimizerOptions.LearnRate = 1e-03;
agentObj.AgentOptions.CriticOptimizerOptions.GradientThreshold = 10;
agentObj.AgentOptions.ActorOptimizerOptions.LearnRate = 1e-04;
agentObj.AgentOptions.ActorOptimizerOptions.GradientThreshold = 10;
% getAction(agentObj,{rand(obsInfo.Dimension)})

%% Do the training  
% Plots="training-progress",...
trainOpts = rlTrainingOptions(...
    MaxEpisodes=2000, ...
    MaxStepsPerEpisode=ceil(Tf/Ts), ...
    ScoreAveragingWindowLength=20, ...
    Verbose=1, ...
    Plots='none',...
    StopTrainingCriteria="AverageReward",...    
    StopTrainingValue=0.3);
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
save('./results/LOKI_autotrans_DDPG_AT51_5K','agentObj','trainingStats','trainingTime');
% agentObj.generatePolicyFunction('FunctionName','autotrans_DDPG_evaluate_policy_AT2','MATFileName','LOKI_');





