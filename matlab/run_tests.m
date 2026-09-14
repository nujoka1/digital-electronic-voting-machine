function run_tests
machine = VotingMachine();
assert(~machine.openPoll('0000'));                         % invalid officer PIN
assert(~machine.openPoll('0000'));                         % second invalid PIN
assert(~machine.openPoll('0000'));                         % persistent lockout threshold
assert(machine.locked && ~machine.openPoll('5580'));       % locked machine cannot open

machine = VotingMachine();
assert(machine.openPoll('5580'));                         % opening
assert(isempty(machine.results()));                        % active totals remain hidden
assert(~machine.vote(2001, true, 1, true));               % unknown voter
assert(~machine.vote(1001, false, 1, true));              % biometric mismatch
assert(machine.cancelBallot());                            % cancellation
assert(machine.accredited(1));                             % accreditation survives cancellation
assert(machine.vote(1001, true, 1, true));                % accepted vote
assert(~machine.vote(1001, true, 2, true));               % duplicate
assert(~machine.vote(1002, true, 1, true, 2));            % multiple selection
assert(~machine.closePoll('0000'));                        % invalid closure
assert(machine.closePoll('5580'));                         % final closure
assert(~machine.vote(1002, true, 2, true));               % after closure
assert(all(machine.counts == [1 0 0 0]));
assert(isequal(machine.results(), [1 0 0 0]));             % results available after closure
assert(~machine.openPoll('5580'));                         % final closure is permanent
machine.tamper(); assert(machine.locked);                 % tamper lockout
fprintf('MATLAB reference tests: PASS\n');
end