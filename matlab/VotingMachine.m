classdef VotingMachine < handle
    properties
        registered = 1001:1010
        voted = false(1, 10)
        counts = [0 0 0 0]
        pollOpen = false
        locked = false
        auditLog = {}
    end
    methods
        function accepted = openPoll(obj, pin)
            accepted = ~obj.locked && strcmp(char(pin), '5580');
            if accepted, obj.pollOpen = true; obj.auditLog{end+1} = 'POLL_OPENED';
            else, obj.auditLog{end+1} = 'OFFICER_AUTH_FAIL'; end
        end
        function accepted = closePoll(obj, pin)
            accepted = ~obj.locked && obj.pollOpen && strcmp(char(pin), '5580');
            if accepted, obj.pollOpen = false; obj.auditLog{end+1} = 'POLL_CLOSED'; end
        end
        function accepted = vote(obj, voterId, biometric, candidate, confirm, selections)
            if nargin < 6, selections = 1; end
            index = find(obj.registered == voterId, 1);
            accepted = obj.pollOpen && ~obj.locked && ~isempty(index) && ~obj.voted(index) && biometric && candidate >= 1 && candidate <= 4 && selections == 1 && confirm;
            if accepted
                obj.voted(index) = true; obj.counts(candidate) = obj.counts(candidate) + 1; obj.auditLog{end+1} = 'BALLOT_ACCEPTED';
            elseif isempty(index), obj.auditLog{end+1} = 'UNKNOWN_VOTER';
            elseif ~biometric, obj.auditLog{end+1} = 'BIOMETRIC_FAIL';
            elseif ~obj.pollOpen, obj.auditLog{end+1} = 'POLL_CLOSED_REJECTION';
            elseif obj.voted(index), obj.auditLog{end+1} = 'DUPLICATE_ATTEMPT';
            else, obj.auditLog{end+1} = 'INVALID_SELECTION'; end
        end
        function cancel = cancelBallot(obj)
            cancel = obj.pollOpen && ~obj.locked; if cancel, obj.auditLog{end+1} = 'BALLOT_CANCELLED'; end
        end
        function tamper(obj)
            obj.locked = true; obj.pollOpen = false; obj.auditLog{end+1} = 'LOCKOUT';
        end
    end
end