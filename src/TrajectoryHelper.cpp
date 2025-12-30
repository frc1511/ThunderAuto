#include <ThunderAuto/TrajectoryHelper.hpp>

const ThunderAutoTrajectorySkeleton kDefaultNewTrajectory{
    {
        ThunderAutoTrajectorySkeletonWaypoint(Point2d(0_m, 0_m),
                                              0_deg,
                                              ThunderAutoTrajectorySkeletonWaypoint::HeadingWeights{}),
        ThunderAutoTrajectorySkeletonWaypoint(Point2d(5_m, 0_m),
                                              0_deg,
                                              ThunderAutoTrajectorySkeletonWaypoint::HeadingWeights{}),
    },
    0_deg,
    0_deg};
