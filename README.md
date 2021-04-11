# Universe - a physics-based screensaver!!! 

**"Universe"** is a screensaver based on the N-bodies physical interaction. The Barnes-Hut algorithm is used to perform interaction calculations. The project is completely a hobby and is distributed as is. Any improvements and fixes are welcome!

## Dependences
The SFML library is used for parallelizing calculations and rendering results. All necessary files are included in the repository and project, so there is no need to worry about dependencies.

## Files
The source code is divided into modules:
**universe_constants.h** - declaration of the main project parameters;
**universe_world.h, universe_world.cpp** - simulation world, that containing information about bodies and statistics;
**universe_worker.h, universe_worker.cpp** - the worker thread, that performs the calculation of the interaction of bodies, here is the thread that controls the simulation;
**universe_tree.h, universe_tree.cpp** - object used in the Barnes-Hut algorithm;
**universe_render. h, universe_render.cpp** - results rendering thread.

## Video
https://youtu.be/Ho5MrpSYPcc