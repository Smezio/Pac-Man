# Pac-Man
Pac-Man videogame developed with UE5 as exercise.

## How-to-play:
- Left arrow: Left
- Right arrow: Right
- Upper arrow: Up
- Lower arrow: Down
- Keyboard "P": Pause

## Main Features
The goal of the project is to recreate the Pac-Man video game using Unreal Engine. The following list define all the requirements the product must meet to be considered completed:
Pac-Man is game where a playable character (i.e. Pac-Man) can move around a maze to collect all dots along all paths.

- The paths of the maze are filled with dots that will be eaten by Pac-Man when he enters the dot’s cell. 

- Four special dots (i.e. Power Dots) are placed in the corners of the maze. When Pac-Man eats them, he will obtain the capability to eat the ghosts.

- The maze has two linked paths on the right and left sides. When Pac-Man passes through one of these paths, he will be teleported on the other side of the maze.
  
- To win the game, the player must collect all the dots before losing the provided 2 lives. If the player fails the 3 attempts, the game is lost.

- The player can lose their lives if he dumps into one of the 4 available ghosts that will chase Pac-Man. The ghosts’ behaviour is defined with 6 states:
  - Idle: ghosts are in the initial room, waiting to come into the maze
  - Chasing: ghosts will chase Pac-Man to defeat him. Each ghost carries out a specific technique
  - Scattering: each ghost will move to each corner of the maze.
  - Escaping: when Pac-Man gets a power-up, the ghost will run away on opposite direction and will choose the next directions randomly
  - Eaten: after Pac-Man eats a ghost, the latter will return to the initial room for spawning
  - Walking: when ghosts come into the maze from the room
  Further details are in the dedicated chapter.

- Score points must be implemented, and increase as follows:
  - Dot = 10 pts
  - Power Dot = 50 pts
  - 1st Ghost = 200 pts
  - 2nd Ghost = 400 pts
  - 3rd Ghost = 800 pts
  - 4th Ghost = 1600 pts
  - Fruit = 300 pts

- A special item represented as Fruit can be eaten to increase the score. In this version, two fruits will spawn after 70 and 170 eaten dots, and disappear after 10 seconds

## Ghosts Behaviour
There are several general rules about the behaviour:
- The ghost can’t choose the back direction (i.e. 180 deg rotation) and the direction through a wall as the next direction.
- During escaping, the ghosts slow down, turn around 180 deg and choose the direction on a crossroad randomly.
- When a ghost is eaten, the target cell will be a cell in front of the initial room entry.
- The chasing and scattering modes are alternated in according to specific time intervals.
- In addition to the previous rules, each ghost has specific behaviours:
  - Blinky (red):
    - chasing: follows directly Pac-Man, targeting his current position
    - scattering: target the upper-right corner
    - angry mode: after 20 eaten dots, Blinky will not scatter anymore.
  - Inky (blue):
    - chasing: the target is selected considering the opposite cell of the Blinky position, relative to two cells in front of Pac-Man
    - scattering: target the lower-right corner
  - Pinky (pink):
    - chasing: targets the spot 2 dots in front of Pac-Man.
    - scattering: target the upper-left corner
  - Clyde (orange):
    - chasing: follows Pac-Man directly when he’s distant 8 or more cells from Pac-Man. Otherwise, Clyde targets his dedicated corner of scattering phase.
    - scattering: target the lower-left corner

## Main Design Methods
The movement of the pawns is grid-based, where FMazeNode struct defines a maze's node by the location, the type of the node and the adjacient nodes.

This struct is integrated with FPathNode struct to define a new structure to manage the AI movement of the ghosts. The path is computed using the A* algorithm for the pathfinding.
Everytime a ghost reach the next node (i.e. one of the adjacent), the algorithm retrieve the path to the indicated target node.

For Pac-Man movement, a direction vector with the input controller is used to define the next path to follow.
