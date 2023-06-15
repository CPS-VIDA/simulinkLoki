files = {dir().name};

B1 = BreachTraceSystem({'x'});
B2 = BreachTraceSystem({'x'});

for ii=3:numel(files)       
    fName = files{ii};
    Array = csvread(files{ii}, 2);
    times = Array(:,1);
    xx = Array(:,2);
    if contains(files{ii},'discord')
        B1.AddTrace([times xx]);
    else 
        B2.AddTrace([times xx]);
    end
end
% col1 = Array(:, 1);
% col2 = Array(:, 2);
% plot(col1, col2)

B1.PlotSignals();
figure
B2.PlotSignals();
