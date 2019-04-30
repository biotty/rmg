//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef SOLIDS_HPP
#define SOLIDS_HPP

#include "world_gen.hpp"

namespace model::solids {

constexpr double g = 1.61803398875;  // golden-ratio
constexpr double g2 = 2.618033988750235;  // golden-ratio ^ 2
constexpr double g3 = 4.236067977500615;  // golden-ratio ^ 3

//  selection of vertices in 120-polyhedron
constexpr direction p2 { g2, 0 , g3};
constexpr direction p4 { 0 , g , g3};
constexpr direction p6 {-g2, 0 , g3};
constexpr direction p7 {-g ,-g2, g3};
constexpr direction p8 { 0 ,-g , g3};
constexpr direction p10{ g3, g , g2};
constexpr direction p11{ g2, g2, g2};
constexpr direction p12{ 0 , g3, g2};
constexpr direction p13{-g2, g2, g2};
constexpr direction p16{-g2,-g2, g2};
constexpr direction p17{ 0 ,-g3, g2};
constexpr direction p18{ g2,-g2, g2};
constexpr direction p20{ g3, 0 , g };
constexpr direction p22{-g2, g3, g };
constexpr direction p23{-g3, 0 , g };
constexpr direction p27{ g3, g2, 0 };
constexpr direction p28{ g , g3, 0 };
constexpr direction p30{-g , g3, 0 };
constexpr direction p31{-g3, g2, 0 };
constexpr direction p33{-g3,-g2, 0 };
constexpr direction p34{-g ,-g3, 0 };
constexpr direction p36{ g ,-g3, 0 };
constexpr direction p37{ g3,-g2, 0 };
constexpr direction p38{ g3, 0 ,-g };
constexpr direction p41{-g3, 0 ,-g };
constexpr direction p43{ g2,-g3,-g };
constexpr direction p45{ g2, g2,-g2};
constexpr direction p46{ 0 , g3,-g2};
constexpr direction p47{-g2, g2,-g2};
constexpr direction p49{-g3,-g ,-g2};
constexpr direction p50{-g2,-g2,-g2};
constexpr direction p51{ 0 ,-g3,-g2};
constexpr direction p52{ g2,-g2,-g2};
constexpr direction p54{ g2, 0 ,-g3};
constexpr direction p55{ g , g2,-g3};
constexpr direction p56{ 0 , g ,-g3};
constexpr direction p58{-g2, 0 ,-g3};
constexpr direction p60{ 0 ,-g ,-g3};

direction mean(std::initializer_list<direction> list)
{
    point mean = o;
    for (direction d : list) {
        mean += d;
    }
    return direction_cast(mean) * (1.0 / list.size());
}

direction tetrahedron_faces[] = {
        unit(mean({p4, p34, p47})),
        unit(mean({p4, p38, p34})),
        unit(mean({p4, p47, p38})),
        unit(mean({p34, p38, p47}))};

double tetrahedron_mr = 1.7321;
double tetrahedron_cr = 3;

direction cube_faces[] = {
    unit(mean({p4, p18, p28, p38})),
    unit(mean({p4, p18, p23, p34})),
    unit(mean({p4, p23, p28, p47})),
    unit(mean({p28, p38, p47, p60})),
    unit(mean({p23, p34, p47, p60})),
    unit(mean({p18, p34, p38, p60}))};

double cube_mr = 1.41422;
double cube_cr = 1.7321;

direction octahedron_faces[] = {
    unit(mean({p7, p10, p43})),
    unit(mean({p7, p22, p10})),
    unit(mean({p7, p43, p49})),
    unit(mean({p7, p49, p22})),
    unit(mean({p55, p10, p43})),
    unit(mean({p55, p22, p10})),
    unit(mean({p55, p43, p49})),
    unit(mean({p55, p49, p22}))};

double octahedron_mr = 1.2248;
double octahedron_cr = 1.7321;

direction dodecahedron_faces[] = {
    unit(mean({p4, p8, p11, p18, p20})),
    unit(mean({p4, p8, p13, p16, p23})),
    unit(mean({p4, p11, p13, p28, p30})),
    unit(mean({p8, p16, p18, p34, p36})),
    unit(mean({p11, p20, p28, p38, p45})),
    unit(mean({p13, p23, p30, p41, p47})),
    unit(mean({p16, p23, p34, p41, p50})),
    unit(mean({p18, p20, p36, p38, p52})),
    unit(mean({p28, p30, p45, p47, p56})),
    unit(mean({p34, p36, p50, p52, p60})),
    unit(mean({p38, p45, p52, p56, p60})),
    unit(mean({p41, p47, p50, p56, p60}))};

double dodecahedron_mr = 1.17557;
double dodecahedron_cr = 1.25841;

direction icosahedron_faces[] = {
    unit(mean({p2, p6, p17})),
    unit(mean({p2, p12, p6})),
    unit(mean({p2, p17, p37})),
    unit(mean({p2, p37, p27})),
    unit(mean({p2, p27, p12})),
    unit(mean({p37, p54, p27})),
    unit(mean({p27, p54, p46})),
    unit(mean({p27, p46, p12})),
    unit(mean({p12, p46, p31})),
    unit(mean({p12, p31, p6})),
    unit(mean({p6, p31, p33})),
    unit(mean({p6, p33, p17})),
    unit(mean({p17, p33, p51})),
    unit(mean({p17, p51, p37})),
    unit(mean({p37, p51, p54})),
    unit(mean({p58, p54, p51})),
    unit(mean({p58, p46, p54})),
    unit(mean({p58, p31, p46})),
    unit(mean({p58, p33, p31})),
    unit(mean({p58, p51, p33}))};

double icosahedron_mr = 1.07047;
double icosahedron_cr = 1.25841;
}
#endif

