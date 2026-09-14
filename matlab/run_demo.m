function run_demo
machine = VotingMachine(); machine.openPoll('5580');
for voter = 1001:1010
    machine.vote(voter, true, mod(voter - 1001, 4) + 1, true);
end
machine.closePoll('5580');
labels = {'Candidate A'; 'Candidate B'; 'Candidate C'; 'Candidate D'};
resultTable = table(labels, machine.counts', 'VariableNames', {'Candidate', 'Votes'});
disp(resultTable);
bar(machine.counts); set(gca, 'XTick', 1:4, 'XTickLabel', labels); ylabel('Votes');
title('Digital voting machine reference results');
if ~exist('../evidence/matlab', 'dir'), mkdir('../evidence/matlab'); end
writetable(resultTable, '../evidence/matlab/simulation-results.csv');
fid = fopen('../evidence/matlab/audit-log.txt', 'w'); fprintf(fid, '%s\n', machine.auditLog{:}); fclose(fid);
summary = table(10, sum(machine.voted), sum(machine.counts), sum(machine.voted) / 10, 'VariableNames', {'Registered', 'Accredited', 'VotesCast', 'Turnout'});
writetable(summary, '../evidence/matlab/summary.csv');
end