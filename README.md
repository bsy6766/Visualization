# Visualization
Visualizes simple game techniques for 2D or anything I found interesting.<br>
Although these are made for 2D, some techniques can be expanded to 3D (i.e. QuadTree to OcTree or 3D flocking with additional axis)<br>
Project is in progress.

## Engine & Language
Cocos2d-X 3.13<br>
> Why Cocos2d-X? Because Cocos2d-X is the engine that I'm most used to it compared to others, works great for 2D, cross-platform and opensource. Also it's free!<br>

C++

## Platform
Win32<br>
OSX(Planned)<br>
Linux(Planned)<br>

## How to run on your machine
<details>
<summary><b>Download Link (Win32)</b></summary>
Executable are available [**here**](https://drive.google.com/open?id=0BxL3wp7rb67tNmNvdXZ1emJXMTg). Download the zip file and open it with your archieve tool. Then run Visualization.exe.<br>
**Note: For Windows, you need Visual c++ Redistributable to run the program. Error message will tell you which DLL you are missing. Google the name of DLL for solution. Also, program might won't open if anti-virus blocks it.**
</details>

## How to build on your machine
<details>
<summary><b>Steps</b></summary>
**Note: I didn't upload entire Cocos2d-X project due to huge size (<200mb initial, <4Gb after building). If you know how Cocos2d-X works, then skip below steps and do it your way. Sources and resources can be found easily in repo folder.**<br>
- 1. Create new Cocos2d-X project. Version 3.13 version is preferred but any version after that will work (I hope).
- 2. Copy Classes folder and Releases folder in repo folder.
- 3. Paste to new Cocos2d-X project folder (where default Classes and Resources folder exists).
- 4. Overwrite if needed.
- 5. Open up the project and build.
</details>


## Features (Press traingle to expand/collapse)
<details>
<summary><b>QuadTree</b></summary>
#### Note
Visualizes 2D space collisions with quad tree. Optimizes number of collision comparison significantly than a bruteforce method (O(n^2)).<br>
Worst query time is O(n)<br>
#### Preview (Expand/Collapse)
<details> 
  <summary>QuadTree preview gif</summary>
   ![QuadTree Preview](https://github.com/bsy6766/Visualization/blob/master/gifs/QuadTree.gif)
</details>
#### Entities
Program only handles 1000 entities due to small screen and over 1000 entities did not seem necessary for demonstration purpose.

##### Modification
To add entity, LEFT CLICK any area in the orange box to add single entity on clicked position or press A to add 10 entities on random position.<br>
To remove entity, RIGHT CLICK on the entity to remove single entity or press E to remove first 10 entities on the entity list (FIFO).<br>
If entity is too small to remove, pause the simulation by pressing SPACE.<br>
To remove all entities, press C.

#### Tracking
To track single entity, click the entity (it's small so I receommend to pause the simulation by SPACE key and then click) you want to track. Blue entity will be the one you track and green entity will be the near entities which can possibly collide with blue one.<br>

#### Duplication Check
If duplication check is enabled, it avoids checking collision with entities that were already checked before.<br>
For this, I used fixed size of vector<int> look up table instead of std::unordered_map<int, bool> because map was very slow comapred to vector.<br>
Toggle this option by pressing D. 

#### Collision Resolution
If collision resolution is enabled, entity will kind of 'bounce off' from collided entity instead of passing by.<br>
Collidided/Colliding entities are shown as red on the screen.<br>
Toggle this option by pressing R.

#### Grid
If grid is enabled, you can see the sub division of QuadTree in the system. 
Toggle this option by pressing G.

#### QuadTree Level
You can increase of decrease QuadTree's maximum level of subdivision. <br>
This feature is limited between 5 and 10.<br>
Since simulation area is limited, it's hard to see QuadTree subdividing more than level 5.<br>

#### Numbber Count
This program will count how many collision check was performed on every frame. You can also check the current number of entities in the orange box. <br>
Numbers are displayed on right top of window.
</details>

<details>
<summary><b>Flocking</b></summary>
#### Note
Visualizes 2D space boids flocking. 

#### Preview (Expand/Collapse)
<details> 
  <summary>Flocking preview gif</summary>
   ![Flocking Preview](https://github.com/bsy6766/Visualization/blob/master/gifs/Flocking.gif)
</details>

#### Boids
Boids are entity that has direction and move on constant speed. Every frame(tick) it updates direction vector based on flocking algorithm.<br>
Maximum boids are limited to 400.

##### Modification
To add boid, LEFT CLICK any area in the orange box to add single entity on clicked position or press A to add 10 entities on random position.<br>
To remove entity, RIGHT CLICK on the entity to remove single entity or press E to remove first 10 entities on the entity list (FIFO).<br>
To remove all entities, press C.

#### Obstacle (See Avoid)
Obstacle is a circle object that can be placed in simulation world. Boids will try to avoid the obstacle in all cases.
##### Modification
To add obstacle, MIDDLE CLICK any area in the orange box to add single obstacle.<br>
To remove obstacle, MIDDLE CLICK on the obstacle.

#### Algorithm
Every boid follows these simple three steering behavior. <br>[Reference](http://www.red3d.com/cwr/boids/)

#### Alignment
Each boid steer towards the average heading of local boidmates.

#### Cohesion
Each boid steer to move toward the average position of local boidmates.

#### Separation
Each boid steer to void crowding local boidmates.

#### Avoid
This isn't one of the three steering behavior. I added this to make boids to avoid obstacle in the world.

#### Weights
Each steering behavior, including Avoid, has weight. Weight determines how much each behavior affects the final direction of each boid.

#### Tracking
To track single boid, LEFT CLICK the boid you want to track. Blue boid will be the one you track and green entity will be the near boids that are in tracking boid's sight range. Yellow sight range checker will be appear on tracking boid.<br>
To stop tracking, LEFT CLICK the boid you are tracking. 

#### QuadTree
This program uses Quad Tree like the QuadTree project in this repo to optimize comparison.

</details>

----
##In Progress
### Animated Circle Packing
### Vector Math
Visualizes dot and cross product

----
## ChangeLog
v0.2 Added Flocking Algorithm visualization.<br>
v0.1 Added QuadTree visualization.<br>
v0.0 Project started.<br>
