
#include "plane.h"
#include "math.h"
#include <stdio.h>
#include <assert.h>

#define FAR_AWAY (HUGE_REAL / 2)

    int
main(void)
{
    plane plane_ = {{0, 0, 0}, {0, 0, 1}};
    ray down_enter = {{0, 0, 1}, {0, 0, -1}};
    ray down_dive = {{0, 0, -1}, {0, 0, -1}};
    ray para_dive = {{0, 0, -1}, {1, 0, 0}};
    ray up_exit = {{0, 0, -1}, {0, 0, 1}};
    ray up_fly = {{0, 0, 1}, {0, 0, 1}};
    ray para_fly = {{0, 0, 1}, {1, 0, 0}};
    const pair pa = plane_intersection(&down_enter, &plane_);
    assert(is_pretty_near(pa.first, 1));
    assert(pa.second >= FAR_AWAY);
    const pair pb = plane_intersection(&down_dive, &plane_);
    assert(pb.first < 0);
    assert(pb.second >= FAR_AWAY);
    const pair pc = plane_intersection(&para_dive, &plane_);
    assert(pc.first < 0);
    assert(pc.second >= FAR_AWAY);
    const pair pd = plane_intersection(&up_exit, &plane_);
    assert(pd.first < 0);
    assert(is_pretty_near(pd.second, 1));
    const pair pe = plane_intersection(&up_fly, &plane_);
    assert(pe.first < 0);
    assert(pe.second < 0);
    const pair pf = plane_intersection(&para_fly, &plane_);
    assert(pf.first < 0);
    assert(pf.second < 0);
}

