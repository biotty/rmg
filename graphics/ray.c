
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

