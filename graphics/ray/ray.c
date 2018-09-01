//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "ray.h"
#include "math.h"

    void
advance(ray * ray_, real r)
{
    direction displacement = ray_->head;
  /*assert(is_near(length(displacement), 1));*/
    scale(&displacement, r);
    move(&ray_->endpoint, displacement);
}

    void
inverse_tilt_ray(ray * ray_, tilt_arg arg)
{
    ray_->endpoint = point_from_origo(
            inverse_tilt(direction_from_origo(ray_->endpoint),
                arg));
    // improve: re-use transformation-matrices
    ray_->head = inverse_tilt(ray_->head, arg);
}

    void
inverse_base_ray(ray * ray_, base_arg arg)
{
    ray_->endpoint = point_from_origo(
            inverse_base(direction_from_origo(ray_->endpoint),
                arg));
    // improve: re-use transformation-matrices
    ray_->head = inverse_base(ray_->head, arg);
}
