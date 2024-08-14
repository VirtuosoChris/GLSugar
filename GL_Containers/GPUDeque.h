#pragma once

template<typename T, std::size_t MinPageSize = 0xffff>
struct GPUDeque
{
    constexpr static std::size_t PageSizeBytes = std::max<std::size_t>(MinPageSize, sizeof(T));
    constexpr static std::size_t CountPerPage = PageSizeBytes / sizeof(T);

    const GLenum usage;

    using PageIndex = std::pair<std::size_t, std::size_t>;

    PageIndex cursor = { 0u,0u }; // one past the end
    PageIndex begin = { 0u,0u };

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

    GPUDeque(GLenum usageIn) : usage(usageIn)
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
            pageVector.push_back(allocatePage());
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

    void shrinkToFit()
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

    const T& operator[](const std::size_t& i) const
    {
        assert(i < Size());

        PageIndex c = begin + i;

        return getData(c);
    }

    void write(const T& value, const std::size_t& i)
    {
        assert(i < Size());

        PageIndex c = begin + i;

        setData(value, c);
    }

private:

    std::deque<gl::Buffer> pageVector;

    gl::Buffer allocatePage()
    {
        gl::Buffer rval;
        rval.Data(PageSizeBytes, nullptr, usage);
        return rval;
    }

    void setData(const T& dat, const PageIndex& idx)
    {
        gl::Buffer& b = pageVector[idx.first];
        b.SubData(idx.second * sizeof(T), sizeof(T), &dat);
    }

    T getData(const PageIndex& idx)
    {
        T rval;
        gl::Buffer& b = pageVector[idx.first];
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

template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex& operator-=(typename GPUDeque<T, MinPageSize>::PageIndex& index, const std::size_t& n)
{
    constexpr std::size_t CountPerPage = GPUDeque<T, MinPageSize>::CountPerPage;

    std::size_t linear = index.first * CountPerPage + index.second;

    assert(linear >= n);

    linear -= n;

    index.first = linear / CountPerPage;
    index.second = linear % CountPerPage;

    return index;
}


template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex& operator+=(typename GPUDeque<T, MinPageSize>::PageIndex& index, const std::size_t& n)
{
    constexpr std::size_t CountPerPage = GPUDeque<T, MinPageSize>::CountPerPage;

    index.second += n;

    int divisibleCount = index.second / CountPerPage;

    index.first += divisibleCount;
    index.second -= CountPerPage * divisibleCount;

    return index;
}

template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex operator+(typename GPUDeque<T, MinPageSize>::PageIndex index, const std::size_t& n)
{
    index += n;
    return index;
}

template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex operator-(typename GPUDeque<T, MinPageSize>::PageIndex index, const std::size_t& n)
{
    index -= n;
    return index;
}

// postfix
template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex operator++(typename GPUDeque<T, MinPageSize>::PageIndex& index, int)
{
    typename GPUDeque<T, MinPageSize>::PageIndex temp = index;
    ++index;
    return temp;
}

template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex operator--(typename GPUDeque<T, MinPageSize>::PageIndex& index, int)
{
    typename GPUDeque<T, MinPageSize>::PageIndex temp = index;
    --index;
    return temp;
}

// prefix
template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex& operator++(typename GPUDeque<T, MinPageSize>::PageIndex& index)
{
    index += 1;
    return index;
}

template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::PageIndex& operator--(typename GPUDeque<T, MinPageSize>::PageIndex& index)
{
    index -= 1;
    return index;
}

template<typename T, std::size_t MinPageSize>
typename GPUDeque<T, MinPageSize>::std::ptrdiff_t operator-(const typename GPUDeque<T, MinPageSize>::PageIndex& lhs, const typename GPUDeque<T, MinPageSize>::PageIndex& rhs)
{
    constexpr std::size_t CountPerPage = GPUDeque<T, MinPageSize>::CountPerPage;
    return (lhs.first * CountPerPage + lhs.second) - (rhs.first * CountPerPage + rhs.second);
}
