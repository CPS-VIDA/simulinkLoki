rng(9);

vmax = 120;
Tf = 20;
Ts = 1.0;
tau = 20;
initrpm = 1000;
actionLowerLimits = [0 0]';
actionUpperLimits = [100 325]';

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

%% Do the training  
trainOpts = rlTrainingOptions(...
    MaxEpisodes=10000, ...
    MaxStepsPerEpisode=ceil(Tf/Ts), ...
    ScoreAveragingWindowLength=20, ...
    Verbose=false, ...
    Plots="training-progress",...
    StopTrainingCriteria="AverageReward",...
    StopTrainingValue=800);

%% load agent

load("LOKI_autotrans_has_falsify.mat","agentObj")  % load the training agent

%% change agent spec

%% agent option
agentObj.SampleTime = Ts;

agentObj.AgentOptions.TargetSmoothFactor = 1e-3;
agentObj.AgentOptions.DiscountFactor = 1.0;
agentObj.AgentOptions.MiniBatchSize = 5000;
agentObj.AgentOptions.ExperienceBufferLength = 1e6; 

agentObj.AgentOptions.NoiseOptions.Variance = 0.3;
agentObj.AgentOptions.NoiseOptions.VarianceDecayRate = 1e-5;

agentObj.AgentOptions.CriticOptimizerOptions.LearnRate = 1e-04;
agentObj.AgentOptions.CriticOptimizerOptions.GradientThreshold = 1;
agentObj.AgentOptions.ActorOptimizerOptions.LearnRate = 1e-04;
agentObj.AgentOptions.ActorOptimizerOptions.GradientThreshold = 1;
% getAction(agentObj,{rand(obsInfo.Dimension)})


%% do training

trainingStats = train(agentObj,env,trainOpts);
save('LOKI_autotrans','agentObj');
save('trainingStats', "trainingStats") % save the training results. 

es = agentObj.ExperienceBuffer.allExperiences;
actions = [es.Action];
