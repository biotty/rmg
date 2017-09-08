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
inverse_rotation_ray(ray * ray_, real theta, real phi)
{
    ray_->endpoint = inverse_rotation(direction_from_origo(ray_->endpoint), theta, phi);
    // improve: re-use transformation-matrices
    ray_->head = inverse_rotation(ray_->head, theta, phi);
}
