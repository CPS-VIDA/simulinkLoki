function [tt,xx] = getExperienceNum(agentObj, num, ts, tf)
    experiences = agentObj.ExperienceBuffer.allExperiences;
    numExpPerSim = tf/ts;
    tt = 0:ts:tf;
    dim = length(experiences(1).Observation{:});
    xx = zeros(dim,length(tt));
    for ii = 1 : numExpPerSim
        xx(:,ii) = experiences((num-1)*numExpPerSim + ii).Observation{:};
    end
end