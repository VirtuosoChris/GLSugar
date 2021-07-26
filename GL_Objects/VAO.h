#pragma once

/// Helper for static (known at compile time) VBO/VAO definitions.
///\todo support formats :
/// GL_FIXED,
/// GL_HALF_FLOAT,
/// GL_INT_2_10_10_10_REV,
/// GL_UNSIGNED_INT_2_10_10_10_REV,
/// GL_UNSIGNED_INT_10F_11F_11F_REV

namespace glSugar
{
    namespace attrib
    {
        template <typename T>
        struct GLEnumType;

        #define DEFINE_TYPE_ENUM(x, y) template<> struct GLEnumType< x > { const static GLenum enumType = y; };

        DEFINE_TYPE_ENUM(GLfloat, GL_FLOAT);
        DEFINE_TYPE_ENUM(GLdouble, GL_DOUBLE);
        DEFINE_TYPE_ENUM(GLubyte, GL_UNSIGNED_BYTE);
        DEFINE_TYPE_ENUM(GLbyte, GL_BYTE);
        DEFINE_TYPE_ENUM(GLint, GL_INT);
        DEFINE_TYPE_ENUM(GLuint, GL_UNSIGNED_INT);
        DEFINE_TYPE_ENUM(GLshort, GL_SHORT);
        DEFINE_TYPE_ENUM(GLushort, GL_UNSIGNED_SHORT);

        template <typename T, std::size_t length, GLenum Normalized = GL_FALSE>
        struct AttribVec : public std::array<T, length>
        {
            typedef std::array<T, length> ArrayType;
            constexpr static GLenum typeEnum = GLEnumType<T>::enumType;
            constexpr static GLenum normalized = Normalized;
            constexpr static GLint  size = length;

            using ArrayType::ArrayType;
            using ArrayType::operator=;

            //AttribVec() {}

            //template <typename B>
            //AttribVec(const std::initializer_list<B>& l) : ArrayType(l) {}
        };


        template <typename T, GLenum Normalized>
        struct AttribVec <T,1,Normalized>
        {
            T value;
        };


        typedef AttribVec<GLfloat, 1> Float;
        typedef AttribVec<GLfloat, 2> Float2;
        typedef AttribVec<GLfloat, 3> Float3;
        typedef AttribVec<GLfloat, 4> Float4;

        typedef AttribVec<GLdouble, 1> Double;
        typedef AttribVec<GLdouble, 2> Double2;
        typedef AttribVec<GLdouble, 3> Double3;
        typedef AttribVec<GLdouble, 4> Double4;

        typedef AttribVec<GLubyte, 1> Ubyte;
        typedef AttribVec<GLubyte, 2> Ubyte2;
        typedef AttribVec<GLubyte, 3> Ubyte3;
        typedef AttribVec<GLubyte, 4> Ubyte4;
        typedef AttribVec<GLubyte, 1, GL_TRUE> Ubyten;
        typedef AttribVec<GLubyte, 2, GL_TRUE> Ubyte2n;
        typedef AttribVec<GLubyte, 3, GL_TRUE> Ubyte3n;
        typedef AttribVec<GLubyte, 4, GL_TRUE> Ubyte4n;

        typedef AttribVec<GLbyte, 1> Byte;
        typedef AttribVec<GLbyte, 2> Byte2;
        typedef AttribVec<GLbyte, 3> Byte3;
        typedef AttribVec<GLbyte, 4> Byte4;
        typedef AttribVec<GLbyte, 1, GL_TRUE> Byten;
        typedef AttribVec<GLbyte, 2, GL_TRUE> Byte2n;
        typedef AttribVec<GLbyte, 3, GL_TRUE> Byte3n;
        typedef AttribVec<GLbyte, 4, GL_TRUE> Byte4n;

        typedef AttribVec<GLushort, 1> Ushort;
        typedef AttribVec<GLushort, 2> Ushort2;
        typedef AttribVec<GLushort, 3> Ushort3;
        typedef AttribVec<GLushort, 4> Ushort4;
        typedef AttribVec<GLushort, 1, GL_TRUE> Ushortn;
        typedef AttribVec<GLushort, 2, GL_TRUE> Ushort2n;
        typedef AttribVec<GLushort, 3, GL_TRUE> Ushort3n;
        typedef AttribVec<GLushort, 4, GL_TRUE> Ushort4n;

        typedef AttribVec<GLshort, 1> Short;
        typedef AttribVec<GLshort, 2> Short2;
        typedef AttribVec<GLshort, 3> Short3;
        typedef AttribVec<GLshort, 4> Short4;
        typedef AttribVec<GLshort, 1, GL_TRUE> Shortn;
        typedef AttribVec<GLshort, 2, GL_TRUE> Short2n;
        typedef AttribVec<GLshort, 3, GL_TRUE> Short3n;
        typedef AttribVec<GLshort, 4, GL_TRUE> Short4n;

        typedef AttribVec<GLuint, 1> Uint;
        typedef AttribVec<GLuint, 2> Uint2;
        typedef AttribVec<GLuint, 3> Uint3;
        typedef AttribVec<GLuint, 4> Uint4;
        typedef AttribVec<GLuint, 1, GL_TRUE> Uintn;
        typedef AttribVec<GLuint, 2, GL_TRUE> Uint2n;
        typedef AttribVec<GLuint, 3, GL_TRUE> Uint3n;
        typedef AttribVec<GLuint, 4, GL_TRUE> Uint4n;

        typedef AttribVec<GLint, 1> Int;
        typedef AttribVec<GLint, 2> Int2;
        typedef AttribVec<GLint, 3> Int3;
        typedef AttribVec<GLint, 4> Int4;
        typedef AttribVec<GLint, 1, GL_TRUE> Intn;
        typedef AttribVec<GLint, 2, GL_TRUE> Int2n;
        typedef AttribVec<GLint, 3, GL_TRUE> Int3n;
        typedef AttribVec<GLint, 4, GL_TRUE> Int4n;
    }

template <typename AttribType, int offset>
void populateAttrib(gl::VertexArray& vao, int bindingIndex, int& attribIndex)
{
    vao.EnableAttrib(attribIndex);
    vao.AttribFormat(attribIndex, AttribType::size, AttribType::typeEnum, AttribType::normalized, offset);
    vao.AttribBinding(attribIndex, bindingIndex);
    attribIndex++;
}

#define ATTRIB(y) glSugar::populateAttrib<decltype(y), offsetof(VertexFormat, y)>(vao, bindingIndex, attribIndex);
#define VAO_INIT(x) using VertexFormat = x; static void initVAO(gl::VertexArray & vao, int bindingIndex, int& attribIndex)

template <typename T, typename ...Args>
void initVAO(gl::VertexArray& vao, int bindingIndex, int& attribIndex)
{
    T::initVAO(vao, bindingIndex, attribIndex);

    if constexpr (sizeof...(Args))
    {
        initVAO<Args...>(vao, bindingIndex + 1, attribIndex);
    }
}

template <std::size_t I, typename T, typename ...Ts>
struct nth_element_impl {
    using type = typename nth_element_impl<I - 1, Ts...>::type;
};

template <typename T, typename ...Ts>
struct nth_element_impl<0, T, Ts...> {
    using type = T;
};

template <std::size_t I, typename ...Ts>
using nth_element = typename nth_element_impl<I, Ts...>::type;

template <class... types>
class Vao
{
protected:
    gl::VertexArray vao;

public:
    Vao()
    {
        int bindingIndex = 0, attribIndex = 0;
        initVAO<types...>(vao, bindingIndex, attribIndex);
    }

    gl::VertexArray& Get() { return vao; }

    void bind()
    {
        vao.Bind();
    }

    void indexBuffer(gl::Buffer& indexBuffer)
    {
        vao.ElementBuffer(indexBuffer);
    }

    template <int BINDING_INDEX=0>
    void vertexBuffer(gl::Buffer& buffer, int offset = 0)
    {
        int stride = sizeof(nth_element<BINDING_INDEX, types...>);
        vao.VertexBuffer(BINDING_INDEX, buffer, offset, stride);
    }

    void vertexBuffer(gl::Buffer& buffer, int vertexBufferBindingIndex, int stride, int offset = 0)
    {
        vao.VertexBuffer(vertexBufferBindingIndex, buffer, offset, stride);
    }
};
}
