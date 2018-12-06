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
        mean({p4, p34, p47}),
        mean({p4, p38, p34}),
        mean({p4, p47, p38}),
        mean({p34, p38, p47})};

double tetrahedron_inradius = abs(tetrahedron_faces[0]);
double tetrahedron_midradius = 1.7321 * tetrahedron_inradius;
double tetrahedron_circumradius = 3 * tetrahedron_inradius;

direction cube_faces[] = {
    mean({p4, p18, p28, p38}),
    mean({p4, p18, p23, p34}),
    mean({p4, p23, p28, p47}),
    mean({p28, p38, p47, p60}),
    mean({p23, p34, p47, p60}),
    mean({p18, p34, p38, p60})};

double cube_inradius = abs(cube_faces[0]);
double cube_midradius = 1.41422 * cube_inradius;
double cube_circumradius = 1.7321 * cube_inradius;

direction octahedron_faces[] = {
    mean({p7, p10, p43}),
    mean({p7, p22, p10}),
    mean({p7, p43, p49}),
    mean({p7, p49, p22}),
    mean({p55, p10, p43}),
    mean({p55, p22, p10}),
    mean({p55, p43, p49}),
    mean({p55, p49, p22})};

double octahedron_inradius = abs(octahedron_faces[0]);
double octahedron_midradius = 1.2248 * octahedron_inradius;
double octahedron_circumradius = 1.7321 * octahedron_inradius;

direction dodecahedron_faces[] = {
    mean({p4, p8, p11, p18, p20}),
    mean({p4, p8, p13, p16, p23}),
    mean({p4, p11, p13, p28, p30}),
    mean({p8, p16, p18, p34, p36}),
    mean({p11, p20, p28, p38, p45}),
    mean({p13, p23, p30, p41, p47}),
    mean({p16, p23, p34, p41, p50}),
    mean({p18, p20, p36, p38, p52}),
    mean({p28, p30, p45, p47, p56}),
    mean({p34, p36, p50, p52, p60}),
    mean({p38, p45, p52, p56, p60}),
    mean({p41, p47, p50, p56, p60})};

double dodecahedron_inradius = abs(dodecahedron_faces[0]);
double dodecahedron_midradius = 1.17557 * dodecahedron_inradius;
double dodecahedron_circumradius = 1.25841 * dodecahedron_inradius;

direction icosahedron_faces[] = {
    mean({p2, p6, p17}),
    mean({p2, p12, p6}),
    mean({p2, p17, p37}),
    mean({p2, p37, p27}),
    mean({p2, p27, p12}),
    mean({p37, p54, p27}),
    mean({p27, p54, p46}),
    mean({p27, p46, p12}),
    mean({p12, p46, p31}),
    mean({p12, p31, p6}),
    mean({p6, p31, p33}),
    mean({p6, p33, p17}),
    mean({p17, p33, p51}),
    mean({p17, p51, p37}),
    mean({p37, p51, p54}),
    mean({p58, p54, p51}),
    mean({p58, p46, p54}),
    mean({p58, p31, p46}),
    mean({p58, p33, p31}),
    mean({p58, p51, p33})};

double icosahedron_inradius = abs(icosahedron_faces[0]);
double icosahedron_midradius = 1.07047 * icosahedron_inradius;
double icosahedron_circumradius = 1.25841 * icosahedron_inradius;
}
#endif

