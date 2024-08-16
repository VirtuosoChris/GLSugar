#pragma once

#include "GPUVector.h"

template <typename T>
class GPUSharedVector : public GPUVector<T, true>
{
    using BaseClass = GPUVector<T, true>;

public:

    GPUSharedVector() : BaseClass(BaseClass::DefaultCapacity, BaseClass::PersistentMapCreationDefaultFlags)
    {
        BaseClass::map(BaseClass::PersistentMappingDefaultFlags);
    }
};

template <typename T>
class GPUSharedVectorWritable : public GPUVector<T, true>
{
    using BaseClass = GPUVector<T, true>;
    constexpr static GLenum Flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

public:

    GPUSharedVectorWritable() : BaseClass(BaseClass::DefaultCapacity, Flags)
    {
        BaseClass::map(Flags);
    }
};

