# Visualization
Visualizes simple game techniques for 2D or anything I found interesting.<br>
**There are preview GIFs for each visualization below on Algorithm section below**

## Notes
- These visualizations are made for 2D, but some can be expanded to 3D (i.e. QuadTree to OcTree or 3D flocking with additional axis).<br>
- All visualizations are coded within about 3 days while my spare time, so I don't expect to work flawlessly and keep up 60 fps for all enviornments. There are definetly some points where I can polish.<br>
- Since all visualizations I made are well known and have tons of articles out there on the internet, I didn't provide any explanation for each algorithm. Search on Google if you need more info.<br>

## Engine & Language
Cocos2d-X 3.13 ~ 3.14.1<br>
> Why Cocos2d-X? Because Cocos2d-X is the engine that I'm most used to it compared to others, works great for 2D, cross-platform and opensource. Also it's free!<br>

C++

## Performance
Performance of each algorithm are different. Some algorithm, Quad Tree for example, limits the size of total entities, but some algorithm can have more than 10000 entities like Circle Packing.<br> 
**I am trying to make the program to run at 60fps, which it does on my machine, but I am more focusing on implementing algorithm than optimization at this moment.<br>**

## Working/Tested Platforms
Windows 10<br>
Mac OS X Sierra<br>
Ubuntu 16.04<br>

## [Wiki](https://github.com/bsy6766/Visualization/wiki)
Checkout wiki page for download link, algorithm details, etc.

----
## Planned
### Optimization
Optimize all visualization.
### Vector Math 
Visualizes dot and cross product (i.e. Patrol detects player if player is in his sight range with angle)
### Audio spectrum
Visualizes music in to several different form of graphs
### Simplex Noise
Ken Perlin's Simplex Noise. Generates noise image and shake.

----
## Font
Used [Rubik](https://www.fontsquirrel.com/fonts/rubik) by Hubert & Fischer.

##Todo List
- Test build on linux
- Replace current ECS (composite) with new ECS (data oriented)


## ChangeLog
v0.10 Added A Star Pathfinding visualization. Memory leak checked with OS X Instrument.<br>
v0.9 Added Ear Clipping visualization.<br>
v0.8 Using new ECS implementation of mine [link](https://github.com/bsy6766/ECS)<br>
v0.7 Added Rect Packing visualization.<br>
v0.6 Refined.<br>
v0.5 Added Circle Packing visualization.<br>
v0.4 Fixed Bugs.<br>
v0.3 Replaced ECS. <br>
v0.2 Added Flocking Algorithm visualization.<br>
v0.1 Added QuadTree visualization.<br>
v0.0 Project started.<br>
