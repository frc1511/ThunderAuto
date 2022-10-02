# ThunderAuto

FRC Robot Path Planning Software.

## Table of Contents

* [App Usage](#app-usage)
* [Robot Code](#robot-code)
    * [Movement](#movement)
    * [Actions](#actions)
* [Downloading](#downloading)
* [Building](#building)
* [In-Depth Explanation](#in-depth-explanation)
    * [Overview](#overview)
    * [Path Editor](#path-editor)
        * [Curve Equation](#curve-equation)
        * [Segment Length](#segment-length)
        * [Curvature](#curvature)
        * [Velocity and Time](#velocity-and-time)

## App Usage

ThunderAuto is designed to be a comprehensive desktop app used to plan Autonomous Paths for FRC Robots. On launch, the user is prompted to create a new project or open an existing project. When creating a new project, several settings can be configured, such as the max velocity and acceleration, dimensions of the robot, and the field image.

<img src="https://raw.githubusercontent.com/wiki/petelilley/ThunderAuto/web/screenshots/screenshot_3.png" width="500">

When the project is created, a number of windows are presented to the user, including the Path Selector, Path Editor, and Properties Editor.

<img src="https://raw.githubusercontent.com/wiki/petelilley/ThunderAuto/web/screenshots/screenshot_1.png" width="800">

The Path Selector window contains the functionality to select, create, delete, and rename paths. The Path Editor page renders the current path as a Bézier curve on top of the field image. The trajectory is overlayed with a gradient representing the robot's velocity (blue = slow, red/pink = fast). The user can create/delete waypoints and adjust their attributes, such as the robot's desired position, rotation, and heading. Attributes can also be changed manually in the Properties Page under the 'Point' dropdown when a waypoint is selected in the Path Editor.

The robot can be flagged to stop at a waypoint by checking the 'Stop' checkbox in the inspector. This tells the app to calculate the robot's deceleration to a complete stop at the selected point and its subsequent acceleration as it continues the trajectory. When a waypoint is marked as stopped, the path editor de-couples the heading handles so that the robot can resume the path at a different angle from when it impacted the point.

ThunderAuto also has a unique feature called Actions which allows the user to flag specific actions for the robot to execute at points along the trajectory. A dropdown in the Properties window contains tools for the user to create/delete/rename the available Actions and  select whatever Actions they want applied to each waypoint.

When a path has been created to the user's satisfaction, the user can click the 'Export' button in the Properties Page under the Curve dropdown. This exports the selected path to a CSV file in the project's directory. The CSV file contains a long list of timestamps corresponding to the robot's desired position, rotation, velocity, and actions.

## Robot Code

The code required to execute a ThunderAuto trajectory on the robot does not need to be too elaborate because most of the motion calculations are done by the app before execution by the robot.

A simple example codebase can be found [here](https://github.com/petelilley/Homer).

### Movement

To start, the exported CSV file will need to be placed somewhere accessible on the robot, such as the deploy directory. Before the trajectory's execution, the robot program needs to [parse the CSV](https://github.com/petelilley/Homer/blob/main/src/Main/Trajectory/Trajectory.cpp#L43) and create two maps representing the path's States and Actions over time. When executing the trajectory, the robot must record the time elapsed and then [sample](https://github.com/petelilley/Homer/blob/main/src/Main/Trajectory/Trajectory.cpp#L62) the current State from the map of states. [Linear interpolation](https://github.com/petelilley/Homer/blob/main/src/Main/Trajectory/Trajectory.cpp#L94) between States can be added to attempt to smooth the trajectory. Using the current State (from odometry) and desired State, the WPILib [HolonomicDriveController](https://first.wpi.edu/wpilib/allwpilib/docs/release/cpp/classfrc_1_1_holonomic_drive_controller.html) class can be used to adjust for error and produce the optimal axis velocities to move.

### Actions

To handle Actions, the robot program must contain a key of Actions matching the one in the app. During the path's execution, the program must compare the set bits in the current action bit-field with the available actions. When a match occurs, the code can execute any code relevant to that action. "Blocking" actions can be made by stopping the trajectory timer; however, these should only happen at stopped waypoints.

## Downloading

Download the latest release [here](https://github.com/petelilley/ThunderAuto/releases/latest).

## Building

Supported operating systems are Windows and macOS. Make sure to resolve all the git submodules before building!
```bash
git submodule init
git submodule update
```
Build projects can be generated using CMake. Tested targets include Visual Studio 2019 or 2022 on Windows and Xcode or Unix Makefiles on macOS. 
```bash
# Configure Windows
cmake . -Bbuild -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release
```
```bash
# Configure macOS
cmake . -Bbuild -G "Xcode" -DCMAKE_BUILD_TYPE=Release
```
```bash
# Build
cmake --build build
```

All the app's resources (Images, Fonts, etc.) are built into the executable, so there's no need to worry about moving them around once it's built.

## In-Depth Explanation

### Overview

ThunderAuto was written in C++ 17 and utilizes a number of thirdparty libraries:

* [imgui (docking branch)](https://github.com/ocornut/imgui/tree/docking) - GUI stuff
* [glfw](https://github.com/glfw/glfw/tree/master) - Window stuff
* [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) - Loading images
* [glad](https://glad.dav1d.de/) - OpenGL implementation

Windows builds required the Windows SDK, and macOS builds require Foundation and AppKit for platform-specific utilities.

### Path Editor

The path editor is probably the most complex part of the app, mostly due to the amount of math it required.

#### Curve Equation

First and foremost, the editor uses [Cubic Bézier Curves](https://en.m.wikipedia.org/wiki/B%C3%A9zier_curve) to interpolate a curve between each waypoint. An earlier version of the app was written using [Cubic Hermite Splines](https://en.m.wikipedia.org/wiki/Cubic_Hermite_spline), however I felt that bezier curves allowed for better control of the path. 

The bezier curve function for X and Y coordinates using end points P<sub>0</sub> and P<sub>3</sub>, and control points P<sub>1</sub> and P<sub>2</sub>.

$$ B\left(t\right) = \left(1 - t\right)^3 P_{0} + 3\left(1 - t\right)^2 t P_{1} + 3\left(1 - t\right)t^2 P_{2} + t^3 P_{3}, 0 \le t \le 1 $$

#### Segment Length

Next, the length of each curve segment needs to be calculated to determine the number of curve samples to and for velocity and time calculations. To do this the program essentially just uses Pythag's Theorem a bunch of times. The mathy explaination would be the arc length formula,

$$ L=\int_{a}^{b}\sqrt{1\ +\left(\frac{dy}{dx}\right)^{2}}dx $$

#### Curvature

Before calculating the robot's velocity, it is useful to know about intervals of high curvature where the robot should slow down. This is solved by calculating the [Menger curvature](https://en.wikipedia.org/wiki/Menger_curvature) of each sampled point of the curve.

$$ c\left(x, y, z\right) = \frac{1}{R} = \frac{4A}{|x - y||y - z||z - x|} $$

This equation utilizes the area and side lengths of a triangle formed between three points on the curve. The side lengths are determined with the Pythagorean Theorem, and the area using Heron's Formula.

#### Velocity and Time

Calculating the velocity and time of each sampled point takes into account the configured maximum velocity and acceleration, stop points, and invervals with high curvature. When calculating each point, the program determines the desired max velocity at the current point, the next desired max velocity, and the distance to a stop. Then the program calculates the distance needed to decelerate from the last velocity to a complete stop and compares that against the distance to the next stop. If it is greater than or equal to, the robot's deceleration is calculated. Otherwise, the program determines the distance to the next max velocity and checks whether it can reach it in time. If there's distance to spare, the velocity will accelerate to or remain at the current max. Otherwise the robot's deceleration to the next max velocity will be calculated.

To calculate the velocities, the wonderful physics kinematics equations can be used.

$$ v = \frac{x}{t} $$

$$ v = v_{0} + at $$

$$ x = v_{0}t + \frac{1}{2}at^2 $$

$$ v^2 = v_{0}^2 + 2ax $$

Calculating the time of each sampled point comes after. Depending on whether the robot is accelerating, decelerating, or cuising at a constant velocity determines which kinematics equation to use.
