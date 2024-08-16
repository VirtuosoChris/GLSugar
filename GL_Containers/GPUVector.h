#pragma once

template <typename T, bool MappedInterface = false>
struct GPUVector
{
    gl::Buffer buffer;

    using value_type = T;

    const static inline std::size_t DefaultCapacity = 64u;

    constexpr static GLenum NonMappedCreationFlagsDefault = GL_DYNAMIC_STORAGE_BIT;
    constexpr static GLenum PersistentMapCreationDefaultFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT;
    constexpr static GLenum PersistentMappingDefaultFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    GPUVector(std::size_t capacityIn = DefaultCapacity, GLenum usageIn = MappedInterface ? PersistentMapCreationDefaultFlags : NonMappedCreationFlagsDefault) :
        capacity(capacityIn),
        usage(usageIn)
    {
        buffer.Storage(capacity * sizeof(T), nullptr, usage);
    }

    void removeUnordered(const std::size_t index)
    {
        removeSwapBack(index);
    }

    /// - remove element at index, swapping with last element to keep data tightly packed
    /// - side effect : order of the elements is changed
    void removeSwapBack(const std::size_t index)
    {
        assert(size > 0);
        assert(index < size);

        std::size_t last = size - 1;

        if (index != last)
        {
            // -- small buffer-buffer copy on gpu / server
            buffer.CopySubData(buffer, last * sizeof(T), index * sizeof(T), sizeof(T));
        }

        size -= 1;
    }

    void clear()
    {
        size = 0;
    }

    void shrink_to_fit()
    {
        capacity = size;
        reallocate();
    }

    void reserve(std::size_t capacityIn)
    {
        if (capacityIn > capacity)
        {
            capacity = capacityIn;
            reallocate();
        }
    }

    void push_back(const T& t)
    {
        if (size >= capacity)
        {
            reserve(capacity * 2);
            assert(size < capacity);
        }

        if (MappedInterface)
        {
            _impl.ptr()[(size++)] = t;
        }
        else
        {
            buffer.SubData((size++) * sizeof(T), sizeof(T), &t);
        }
    }

    template<typename = std::enable_if_t<MappedInterface == true>>
    const T& operator[] (const std::size_t& i) const
    {
        assert(_impl.mappedPtr != nullptr);
        return _impl.mappedPtr[i];
    }

    template<typename = std::enable_if_t<MappedInterface == true>>
    T& operator[] (const std::size_t& i)
    {
        assert(_impl.mappedPtr != nullptr);
        return _impl.mappedPtr[i];
    }

    template<typename = std::enable_if_t<MappedInterface == false>>
    T operator[] (const std::size_t& i) const
    {
        assert(i < size);

        T rval;
        buffer.GetSubData(i * sizeof(T), sizeof(T), &rval);
        return rval;
    }

    //template<typename = std::enable_if_t<MappedInterface == false>>
    void write(const T& value, const std::size_t& i)
    {
        assert(i < size);

        if (MappedInterface)
        {
            _impl.ptr()[i] = value;
        }
        else
        {
            buffer.SubData(i * sizeof(T), sizeof(T), &value);
        }
    }

    template<typename = std::enable_if_t<MappedInterface>>
    void map(GLenum flags = PersistentMappingDefaultFlags)
    {
        assert(_impl.mappedPtr == nullptr);
        _impl.mappedPtr = (T*)buffer.Map(flags);
        assert(_impl.mappedPtr != nullptr);
        _impl.mapFlags = flags;
    }

    template<typename = std::enable_if_t<MappedInterface>>
    void unmap()
    {
        assert(_impl.mappedPtr != nullptr);
        buffer.Unmap();
        _impl.mappedPtr = nullptr;
        _impl.mapFlags = 0;
    }

    std::size_t Size() const
    {
        return size;
    }

    std::size_t SizeBytes() const
    {
        return size * sizeof(T);
    }

    std::size_t Capacity() const
    {
        return capacity;
    }

    std::size_t CapacityBytes() const
    {
        return capacity * sizeof(T);
    }

private:

    std::size_t capacity = 0;
    std::size_t size = 0;
    const GLenum usage;

    template<typename = std::enable_if_t<MappedInterface>>
    bool remap()
    {
        if (_impl.mapFlags)
        {
            map(_impl.mapFlags);
            return _impl.mappedPtr != nullptr;
        }
    }

    template<typename = std::enable_if_t<MappedInterface == false>>
    void remap() { ; }

    void reallocate()
    {
        gl::Buffer b;

        b.Storage(capacity * sizeof(T), nullptr, usage);

        buffer.CopySubData(b, 0u, 0u, size * sizeof(T));
        buffer = std::move(b);

        remap();
    }

    template <bool MappedInterface>
    struct _Impl
    {
        T* ptr() const { return nullptr; }
    };

    template<>
    struct _Impl<true>
    {
        T* mappedPtr = nullptr;
        GLenum mapFlags = 0;

        T* ptr() { return mappedPtr; }
    };

    using Impl = _Impl<MappedInterface>;

    Impl _impl;
};


