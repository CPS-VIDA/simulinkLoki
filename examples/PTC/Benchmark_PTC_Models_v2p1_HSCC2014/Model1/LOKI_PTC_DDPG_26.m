clc;
clear;
rng(100);
Tf = 40;
SampleTime = 5;
fuel_inj_tol = 1.0; 
MAF_sensor_tol = 1.0;
AF_sensor_tol = 1.0;
fault_time = 100; 
en_speed = 1000;

%% Property constants: Always_[Ts,Tf] (abs(mu) < Mulimit)
MuLimit = 0.05;
Ts = 11.0; % Ti + eta_S->N
%%

actionLowerLimits = [0 900]';
actionUpperLimits = [61.1 1100]';

stateLowerLimits = [10 12]';
stateUpperLimits = [19 15]';

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
actInfo.Name="throttle;en_speed";

env=rlSimulinkEnv("AbstractFuelControl_M1_LOKI","AbstractFuelControl_M1_LOKI/RL Agent",...
    obsInfo,actInfo);


%% Make critic network
statePath = [
    featureInputLayer(obsInfo.Dimension(1),Name="netObsIn")   
    fullyConnectedLayer(criticHiddenLayerSize)
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
    reluLayer
    fullyConnectedLayer(actInfo.Dimension(1))
    tanhLayer
    ];
actorNetwork = dlnetwork(actorNetwork);
%summary(actorNetwork);
actor = rlContinuousDeterministicActor(actorNetwork,obsInfo,actInfo);
% a = [getAction(actor,{rand(obsInfo.Dimension)})];
% fprintf('%s.\n',mat2str(a{1},3));

% getAction(agentObj,{rand(obsInfo.Dimension)})

%% Do the training  
% Plots="training-progress",...


    % UseParallel=true,...

    %% Setup the RL agent
criticOptions = rlOptimizerOptions( ...
    LearnRate=1e-1, ...
    GradientThreshold=10, ...
    L2RegularizationFactor=1e-4);
actorOptions = rlOptimizerOptions( ...
    LearnRate=1e-1, ...
    GradientThreshold=10, ...
    L2RegularizationFactor=1e-4);

agentOptions = rlDDPGAgentOptions(...
    TargetSmoothFactor = 1e-3, ...
    SampleTime = SampleTime, ...
    MiniBatchSize = 1000, ...
    DiscountFactor = 1.0,  ...
    ExperienceBufferLength = 1e7, ...
    ActorOptimizerOptions = actorOptions, ...
    CriticOptimizerOptions = criticOptions);

agentOptions.NoiseOptions.Variance = [0.7;0.7];
agentOptions.NoiseOptions.VarianceDecayRate = 1e-4;
agentObj = rlDDPGAgent(actor,critic,agentOptions);
%%
trainOpts = rlTrainingOptions(...
    MaxEpisodes=10000, ...
    MaxStepsPerEpisode=ceil(Tf/SampleTime), ...
    ScoreAveragingWindowLength=50, ...
    Verbose=1, ...   
    Plots='training-progress',...
    StopTrainingCriteria="AverageReward",...    
    StopTrainingValue=0.03);
startTime = tic;
% Train the agent.
trainingStats = train(agentObj,env,trainOpts);
trainingTime = toc(startTime);
fprintf('Finished Training. Total Time Taken = %d.\n',trainingTime);
%% Show the experiences
%
% es = agentObj.ExperienceBuffer.allExperiences;
% actions = [es.Action];
save('./results/LOKI_PTC_DDPG_Prop26_Tf50_1000','agentObj','trainingStats','trainingTime');
% agentObj.generatePolicyFunction('FunctionName','autotrans_DDPG_AT1_evaluate_policy','MATFileName','./results/autotrans_DDPG_AT1');





