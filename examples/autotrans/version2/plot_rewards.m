filename = 'trainingStats_training_500.mat'
load(filename);


episodeRewards = [trainingStats.EpisodeReward];
% x = episodeRewards(471);
% episodeRewards(471) = x / 1000;
averageRewards = [trainingStats.AverageReward];
averageRewards = averageRewards(1:470);
Q0 = [trainingStats.EpisodeQ0];

% Create the reward plot
figure;
plot(episodeRewards, '.-', color="b");
hold on;
plot(averageRewards, '.-', color="#4DBEEE");
hold on;
plot(Q0, '.-',Color="#EDB120");
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
