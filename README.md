# Visualization
Visualizes simple game techniques for 2D.<br>
Although everything is made for 2D, some techniques can be expanded to 3D (i.e. QuadTree to OcTree or 3D flocking with additional axis)

## Engine & Language
Cocos2d-X 3.13
C++

## Platform
Win32
OSX(Planned)
Linux(Planned)

## Features (Press traingle to expand/collapse)
<details>
<summary><b>QuadTree</b></summary>
#### Note
Visualizes 2D space collisions with quad tree. Optimizes number of collision comparison significantly than a bruteforce method (O(n^2)).<br>
Worst query time is O(n)<br>
#### Preview
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

----
**In Progress
### Flocking
Visualizes 2D space boids flocking with flocking algorithm
### Vector Math
Visualizes dot and cross product
