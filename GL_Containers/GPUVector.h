#pragma once

template <typename T>
struct GPUVector
{
    gl::Buffer buffer;
    std::size_t capacity = 0;
    std::size_t size = 0;
    const GLenum usage;

    const static inline std::size_t DefaultCapacity = 64u;

    GPUVector(std::size_t capacityIn = DefaultCapacity, GLenum usageIn = GL_STATIC_DRAW) :
        capacity(capacityIn),
        usage(usageIn)
    {
        buffer.Data(capacity * sizeof(T), nullptr, usage);
    }

    void clear()
    {
        size = 0;
    }

    void reserve(std::size_t capacityIn)
    {
        if (capacityIn > capacity)
        {
#if _DEBUG
            //std::clog << "GPUBuffer Reserve " << capacity << " from " << capacityIn<< std::endl;
            //std::clog << "\t buffer was " << buffer.name() << std::endl;
#endif

            capacity = capacityIn;
            gl::Buffer b;

            b.Data(capacity * sizeof(T), nullptr, usage);

            buffer.CopySubData(b, 0u, 0u, size * sizeof(T));
            buffer = std::move(b);

            //std::clog << "\t buffer is " << buffer.name() << std::endl;
        }
    }

    void push_back(const T& t)
    {
        if (size >= capacity)
        {
            reserve(capacity * 2);
            assert(size < capacity);
        }

        buffer.SubData((size++) * sizeof(T), sizeof(T), &t);
    }

    T operator[] (const std::size_t i) const
    {
        assert(i < size);

        T rval;
        buffer.GetSubData(i * sizeof(T), sizeof(T), &rval);
        return rval;
    }

    void write(const T& value, const std::size_t& i)
    {
        assert(i < size);
        buffer.SubData(i * sizeof(T), sizeof(T), &value);
    }
};


