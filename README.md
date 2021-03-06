# RL-Controller-Agents
An application of reinforcement learning in a multi-agent setting. Two teams of agents compete to reach the other team's home zone first.

Created by: Patrick Hartman

-- Developed between May and August 2015

Description:

    A reinforcement learning scenario. For each team (Green/Red), a controller
    learns a policy of actions to sucessfully direct the two agents to discover
    and move to the other team's end zone. The controllers' actions consist of
    pairs of goals given to its respective agents. The goals become a part of
    the agents' state, and their task becomes to learn a policy of actions to
    successfully accomplish the assigned goals.

    The controller's state consists of an array of 9 values indicating the
    status of the individual 'blocks'. A block is a 9x9-sized section of the
    map. The agents's state consists of an array of values indicating the
    contents of the block in which it currently resides as well as the goal that
    has been assigned to it. To decrease the potentially-enormous state space
    for both controllers and agents, many state details deemed less significant
    to the task are omitted.

    Controller actions are: ENGAGE, FINISH, GO_LEFT, GO_RIGHT, GO_UP, GO_DOWN,
    NO_ACTION. Controller actions are agent goals.

    Agent actions are: MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN, FIRE_LEFT,
    FIRE_RIGHT, FIRE_UP, FIRE_DOWN.

    Controllers' receive a reward when an agent of its team reaches an end zone.
    Agents receive a reward when they accomplish an assigned goal, kill an
    enemy, or reach the end zone. Agents receive a punishment when they fail to
    accomplish an assigned goal, kill the other agent on its team, or die.

    Both agents and controllers learn state,actions pairs ( Q(s,a) ). These
    state action pairs and their learned values are stored in agentsValues.csv
    and ctrlrValues.csv. The last value in each row (line) is the number of
    times that the given state, action pair has been updated.

    As the number of games played increases, both controller and agent behaviour
    improves significantly until very capable play is attained.



Dependencies:

    POSIX Threads, SDL-devel



To compile:

    type 'make'



To run:

    type 'make run'



Controls:

    left arrow      - decrease delay length

    right arrow     - increase delay length

    f               - alternate between views (explored-shaded, explored-full,
                      no shade)

    t               - alternate between team views (red, green)

    space           - display green agent 1 (brighter green) state, action,
                      rewards in terminal

    a               - display green agent 1 action values and choice in
                      terminal

    c               - display green/red controller state and green action in
                      terminal

    esc.            - quit



Included files:

    config                  -   configuration file telling program what map to
                                load, and what (state,action) value trees to
                                load

    Makefile                -   Makefile for compiling and running program

    agentValues.csv         -   Agent (state, action) value tree

    ctrlrValues.csv         -   Controller (state, action) value tree


    src/ and include/

        agent.c/.h          -   code for the agents

        bullet.c/.h         -   code for bullet data type

        controller.c/.h     -   code for the controllers

        draw.c/.h           -   code to produce graphics

        env.c/.h            -   code related to the environment (env) structure

        event.c/.h          -   code related to keyboard presses

        LinkedList.c/.h     -   linked list ADT

        main.c/.h           -   major operational functions

        neuralnet.c/.h      -   unused in final code, implementation of a simple
                                neural net trained in GNU Octave

        reward.c/.h         -   code for issuing rewards to agents, controllers

        tree.c/.h           -   code for tree used to store agent, controller
                                (state, action) values

