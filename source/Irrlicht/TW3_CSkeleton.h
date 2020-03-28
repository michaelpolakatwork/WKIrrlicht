// Copyright (C) 2002-2020 by Jean-Louis Boudrand
// adapted to irrlicht by vonleebpl(at)gmail.com


#ifndef TW3_CSKELETON_H
#define TW3_CSKELETON_H

#include "ISkinnedMesh.h"

namespace irr
{
    namespace scene
    {

        scene::ISkinnedMesh::SJoint* getJointByName(scene::ISkinnedMesh* mesh, core::stringc name);

        class TW3_CSkeleton
        {
        public:
            TW3_CSkeleton();

            u32 nbBones;
            core::array<core::stringc> names;
            core::array<short> parentId;
            core::array<core::matrix4> matrix;

            core::array<core::vector3df> positions;
            core::array<core::quaternion> rotations;
            core::array<core::vector3df> scales;

            bool applyToModel(scene::ISkinnedMesh* mesh);
        };
    }
}
#endif // TW3_CSKELETON_H
