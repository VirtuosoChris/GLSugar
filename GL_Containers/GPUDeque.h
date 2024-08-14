#pragma once

template<typename T, bool MappedInterface = false, std::size_t MinPageSize = 0xffff>
struct GPUDeque
{
    constexpr static std::size_t PageSizeBytes = std::max<std::size_t>(MinPageSize, sizeof(T));
    constexpr static std::size_t CountPerPage = PageSizeBytes / sizeof(T);

    constexpr static GLenum PersistentMapCreationDefaultFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

    // use coherent map bit by default so the user doesn't have to think about explicit sync.  This may be slower!
    constexpr static GLenum PersistentMappingDefaultFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    const GLenum usage;

    using PageIndex = std::pair<std::size_t, std::size_t>;

    PageIndex cursor = { 0u,0u }; // one past the end
    PageIndex begin = { 0u,0u };

    template<typename = std::enable_if_t<MappedInterface>>
    void map(GLenum flags)
    {
        mapped = true;

        mapFlags = flags;

        for (Page& p : pageVector)
        {
            p.mappedPtr = (T*)p.buffer.MapRange(0, PageSizeBytes, flags);
        }
    }

    template<typename = std::enable_if_t<MappedInterface>>
    void unmap()
    {
        mapped = false;
        mapFlags = 0;

        for each (Page & p in pageVector)
        {
            p.mappedPtr = nullptr;
            p.buffer.Unmap();
        }
    }

    std::size_t Size() const
    {
        return cursor - begin;
    }

    std::size_t SizeBytes() const
    {
        return Size() * sizeof(T);
    }

    std::size_t Capacity() const
    {
        return CountPerPage * pageVector.size();
    }

    std::size_t CapacityBytes() const
    {
        return PageSizeBytes * pageVector.size();
    }

    PageIndex end()
    {
        return { pageVector.size(),0u };
    }

    GPUDeque(GLenum usageIn = defaultUsage()) : usage(usageIn)
    {
        pageVector.push_back(allocatePage());
        clear();
    }

    void push_front(const T& t)
    {
        constexpr PageIndex nullCursor = { 0u, 0u };
        if (cursor == nullCursor)
        {
            pageVector.push_front(allocatePage());
            cursor.first++;
            begin.first++;
        }

        setData(t, cursor);

        cursor--;
    }

    void push_back(const T& t)
    {
        if (cursor == end())
        {
            pageVector.push_back(allocatePage() );
        }

        setData(t, cursor);

        cursor++;
    }

    void pop_front()
    {
        assert(Size() != 0);
        begin++;
    }

    void pop_back()
    {
        assert(Size() != 0);
        cursor--;
    }

    void clear()
    {
        cursor = begin = { 0u,0u };
    }

    void shrink_to_fit()
    {
        shrinkBack();
        shrinkFront();
    }

    std::size_t extraPageBytes() const
    {
        std::size_t extraPagesBack = pageVector.size() - (cursor + 1);
        std::size_t extraPagesFront = begin.first;
        return (extraPagesFront + extraPagesBack) * PageSizeBytes;
    }

    template<typename = std::enable_if_t<MappedInterface == true>>
    T& operator[](const std::size_t& i)
    {
        static_assert(MappedInterface == true);
        assert(i < Size());

        PageIndex c = begin + i;
        return pageVector[c.first][c.second];
    }


    template<typename = std::enable_if_t<MappedInterface == false>>
    T operator[](const std::size_t& i) const
    {
        static_assert(MappedInterface == false);
        assert(i < Size());

        PageIndex c = begin + i;

        return getData(c);
    }

    template<typename = std::enable_if_t<MappedInterface == true>>
    const T& operator[](const std::size_t& i) const
    {
        static_assert(MappedInterface == true);
        assert(i < Size());

        PageIndex c = begin + i;
        return pageVector[c.first][c.second];
    }

    template<typename = std::enable_if_t<MappedInterface == false>>
    void write(const T& value, const std::size_t& i)
    {
        static_assert(MappedInterface == false);
        assert(i < Size());

        PageIndex c = begin + i;

        setData(value, c);
    }

private:

    GLenum mapFlags = 0; // valid state is for these to be zero whenever the deque is not mapped
    bool mapped = false;

    static inline GLenum defaultUsage()
    {
       if (!MappedInterface) return GL_DYNAMIC_STORAGE_BIT; // -- needed for subData calls to change the data

       return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
    }

    template<typename T, bool MappedInterface>
    struct PageType
    {
        gl::Buffer buffer;

        PageType(GLenum usage, GLenum x)
        {
            buffer.Data(PageSizeBytes, nullptr, usage);
        }
    };

    template <typename T>
    struct PageType<T, true>
    {
        gl::Buffer buffer;
        T* mappedPtr = nullptr;
        bool dirtyBit = false;

        const T& operator[](const std::size_t idx) const
        {
            assert(mappedPtr != nullptr);
            return *mappedPtr[idx];
        }

        T& operator[](const std::size_t idx)
        {
            assert(mappedPtr != nullptr);
            dirtyBit = true;
            return *mappedPtr[idx];
        }

        PageType(GLenum usage, GLenum mapFlags = 0)
        {
            buffer.Data(PageSizeBytes, nullptr, usage);

            if (mapFlags != 0)
            {
                map(mapFlags);
            }
        }

        void map(GLenum mapFlags)
        {
            assert(mappedPtr == nullptr);
            mappedPtr = (T*)buffer.MapRange(0, PageSizeBytes, mapFlags);
            assert(mappedPtr != nullptr);
            dirtyBit = false;
        }

        void unmap()
        {
            assert(mappedPtr != nullptr);
            mappedPtr = nullptr;
            buffer.Unmap();
            dirtyBit = false;
        }
    };


    using Page = PageType<T, MappedInterface>;

    std::deque<Page> pageVector;

    Page allocatePage()
    {
        return Page(usage, mapFlags);
    }


    template<typename = std::enable_if_t<MappedInterface == false>>
    void setData(const T& dat, const PageIndex& idx)
    {
        static_assert(MappedInterface == false);

        gl::Buffer& b = pageVector[idx.first].buffer;
        b.SubData(idx.second * sizeof(T), sizeof(T), &dat);
    }

    template<typename = std::enable_if_t<MappedInterface == false>>
    T getData(const PageIndex& idx)
    {
        static_assert(MappedInterface == false);

        T rval;
        gl::Buffer& b = pageVector[idx.first].buffer;
        b.GetSubData(idx.second * sizeof(T), sizeof(T), &rval);
        return rval;
    }

    void shrinkBack()
    {
        std::size_t extraPages = pageVector.size() - (cursor + 1);

        for (int i = 0; i < extraPages; i++)
        {
            pageVector.pop_back();
        }
    }

    void shrinkFront()
    {
        std::size_t extraPagesFront = begin.first;

        for (int i = 0; i < extraPagesFront; i++)
        {
            pageVector.pop_front();
        }

        cursor.first -= extraPagesFront;
        begin.first -= extraPagesFront;
    }
};

template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& operator-=(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& index, const std::size_t& n)
{
    constexpr std::size_t CountPerPage = GPUDeque<T, MinPageSize>::CountPerPage;

    std::size_t linear = index.first * CountPerPage + index.second;

    assert(linear >= n);

    linear -= n;

    index.first = linear / CountPerPage;
    index.second = linear % CountPerPage;

    return index;
}


template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& operator+=(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& index, const std::size_t& n)
{
    constexpr std::size_t CountPerPage = GPUDeque<T, MinPageSize>::CountPerPage;

    index.second += n;

    int divisibleCount = index.second / CountPerPage;

    index.first += divisibleCount;
    index.second -= CountPerPage * divisibleCount;

    return index;
}

template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex operator+(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex index, const std::size_t& n)
{
    index += n;
    return index;
}

template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex operator-(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex index, const std::size_t& n)
{
    index -= n;
    return index;
}

// postfix
template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex operator++(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& index, int)
{
    typename GPUDeque<T, MinPageSize>::PageIndex temp = index;
    ++index;
    return temp;
}

template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex operator--(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& index, int)
{
    typename GPUDeque<T, MinPageSize>::PageIndex temp = index;
    --index;
    return temp;
}

// prefix
template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& operator++(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& index)
{
    index += 1;
    return index;
}

template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& operator--(typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& index)
{
    index -= 1;
    return index;
}

template<typename T, bool MappedInterface, std::size_t MinPageSize>
typename GPUDeque<T, MappedInterface, MinPageSize>::std::ptrdiff_t operator-(const typename GPUDeque<T, MappedInterface, MinPageSize>::PageIndex& lhs, const typename GPUDeque<T, MinPageSize>::PageIndex& rhs)
{
    constexpr std::size_t CountPerPage = GPUDeque<T, MinPageSize>::CountPerPage;
    return (lhs.first * CountPerPage + lhs.second) - (rhs.first * CountPerPage + rhs.second);
}
