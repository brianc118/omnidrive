# 3/4 wheel omnidirection movement library

In late 2014, I spent a long time developing the library for optimal performance (I had previously made several similar libraries that were less efficient). It was used during 2015 on the 3rd generation of the PiBot for Robocup Junior. As such, you will be able to find an earlier version of the code in the team's [libraries repo](https://github.com/brianchen118/Team-PI-Lib).

The original version supported only the 4 wheel configuration, but since I've now made a [ballbot](http://onewaytobalance.blogspot.com.au/search/label/Ballbot) with 3 wheels, I've changed the library to support 3 and 4 wheeled configurations.

To enable 3 wheeled configuration, uncomment TRI_CONFIG in *omnidrive.h*
To enable 4 wheeled configuration, comment out TRI_CONFIG.

If you have any questions, please do not hesitate to contact me.

The source code in this repository is licensed under The MIT License (MIT).
See LICENSE.txt for details.

Copyright (c) 2015 Brian Chen