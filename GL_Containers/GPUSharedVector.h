#pragma once

#include "GPUVector.h"

// default is coherent persistent rw shared buffer
template <typename T, GLenum Flags>
class GPUMappedVector : public GPUVector<T, true>
{
    using BaseClass = GPUVector<T, true>;

public:

    GPUMappedVector() : BaseClass(BaseClass::DefaultCapacity, Flags)
    {
        BaseClass::map(Flags);
    }
};

template <typename T>
using GPUSharedVector = GPUMappedVector<T, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT>;

template <typename T>
using GPUSharedVectorWritable = GPUMappedVector<T, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT>;

template <typename T>
using GPUSharedVectorReadable = GPUMappedVector<T, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT>;

// *** need to be manually flushed / synced ***
template <typename T>
using GPUPersistentVectorWritable = GPUMappedVector<T, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT>;

template <typename T>
using GPUPersistentVectorReadable = GPUMappedVector<T, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT>;


