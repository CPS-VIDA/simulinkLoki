filename = './results/LOKI_PTC_DDPG_Prop26_Tf50_1000.mat'
results = load(filename);


episodeRewards = [results.trainingStats.EpisodeReward];
% x = episodeRewards(471);
% episodeRewards(471) = x / 1000;
averageRewards = [results.trainingStats.AverageReward];
% averageRewards = averageRewards(1:470);
Q0 = [results.trainingStats.EpisodeQ0];

% Create the reward plot
figure;
plot(episodeRewards(1:1000), '.-', color="b");
hold on;
plot(averageRewards(1:1000), '.-', color="#4DBEEE");
hold on;
plot(Q0(1:1000), '.-',Color="#EDB120");
hold on; 
% Add red line at 0
yline(0, 'r');

hold on; 

% Find the episodes where the reward was greater than 0
positiveRewardIndices = find(episodeRewards > 0);

% Plot these episodes with bigger red dots
plot(positiveRewardIndices, episodeRewards(positiveRewardIndices), '.', 'Color', 'r', 'MarkerSize', 15);

legend('Episode reward', 'Average reward', 'Episode Q0');

% Add title and labels
title('Reward Plot');
xlabel('Episode Number');
ylabel('Episode Reward');
