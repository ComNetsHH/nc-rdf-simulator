#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gym
import argparse
import ns3gym
from ns3gym import ns3env

startSim = False
iterationNum = 1
port = 5555
simTime = 5 
stepTime = 0.5
seed = 0
simArgs = {"--simTime": simTime,
           "--stepTime": stepTime,
           "--testArg": 123}
debug = False

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0

try:
    obs = env.reset()
    print("Step: ", stepIdx)
    print("---obs: ", obs)

    while True:
        stepIdx += 1

        action = env.action_space.sample()
        print("---action: ", action)
        obs, reward, done, info = env.step(action)

        print("Step: ", stepIdx)
        print("---obs, reward, done, info: ", obs, reward, done, info)

        if done:
            break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")