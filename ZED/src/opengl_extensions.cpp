#include "../include/opengl_extensions.h"
#ifdef LINUX
#include <GL/glx.h>
#define wglGetProcAddress glXGetProcAddress
#endif



using namespace ZED;
using namespace std::literals;



OpenGLExtensions::OpenGLExtensions()
{
    if (!glGetString(GL_EXTENSIONS))//Returns null pointer if no OpenGL context has been created
        return;

    const std::string all_extensions((const char*)glGetString(GL_EXTENSIONS));
    std::string current_extension;
    
    for (size_t i = 0; i < all_extensions.length(); i++)
    {
        if (all_extensions[i] != ' ')
            current_extension += all_extensions[i];
        else
        {
            m_SupportedExtensions.emplace_back(current_extension);
            current_extension.clear();
        }
    }



    for (const auto& extension : m_SupportedExtensions)
    {
        if (extension == "GL_ARB_framebuffer_object"s)
        {
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)wglGetProcAddress("glFramebufferTexture1D");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)wglGetProcAddress("glFramebufferTexture3D");
            glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)wglGetProcAddress("glFramebufferTextureLayer");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebuffer");
            glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
            glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");
        }
        else if (extension == "GL_ARB_multisample"s)
        {
            glSampleCoverageARB = (PFNGLSAMPLECOVERAGEARBPROC)wglGetProcAddress("glSampleCoverageARB");
        }
        else if (extension == "GL_ARB_multitexture"s)
        {
            glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
        }
        else if (extension == "GL_ARB_vertex_buffer_object"s) // same as PBO
        {
            glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
            glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
            glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
            glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
            glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
            glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetBufferParameteriv");
            glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
            glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
        }
        else if (extension == "GL_ARB_shader_objects"s)
        {
            // core version
            glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
            glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
            glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
            glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
            glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
            glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
            glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
            glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)wglGetProcAddress("glGetActiveUniform");
            glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)wglGetProcAddress("glGetAttachedShaders");
            glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
            glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
            glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
            glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
            glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)wglGetProcAddress("glGetShaderSource");
            glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
            glGetUniformfv = (PFNGLGETUNIFORMFVPROC)wglGetProcAddress("glGetUniformfv");
            glGetUniformiv = (PFNGLGETUNIFORMIVPROC)wglGetProcAddress("glGetUniformiv");
            glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
            glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
            glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
            glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
            glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
            glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
            glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
            glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
            glUniform2i = (PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i");
            glUniform3i = (PFNGLUNIFORM3IPROC)wglGetProcAddress("glUniform3i");
            glUniform4i = (PFNGLUNIFORM4IPROC)wglGetProcAddress("glUniform4i");
            glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
            glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
            glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
            glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
            glUniform1iv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1iv");
            glUniform2iv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2iv");
            glUniform3iv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3iv");
            glUniform4iv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4iv");
            glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)wglGetProcAddress("glUniformMatrix2fv");
            glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
            glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
            glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)wglGetProcAddress("glValidateProgram");
        }
        else if (extension == "GL_ARB_sync"s)
        {
            glFenceSync = (PFNGLFENCESYNCPROC)wglGetProcAddress("glFenceSync");
            glIsSync = (PFNGLISSYNCPROC)wglGetProcAddress("glIsSync");
            glDeleteSync = (PFNGLDELETESYNCPROC)wglGetProcAddress("glDeleteSync");
            glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)wglGetProcAddress("glClientWaitSync");
            glWaitSync = (PFNGLWAITSYNCPROC)wglGetProcAddress("glWaitSync");
            glGetInteger64v = (PFNGLGETINTEGER64VPROC)wglGetProcAddress("glGetInteger64v");
            glGetSynciv = (PFNGLGETSYNCIVPROC)wglGetProcAddress("glGetSynciv");
        }
        else if (extension == "GL_ARB_vertex_array_object"s)
        {
            glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
            glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
            glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
            glIsVertexArray = (PFNGLISVERTEXARRAYPROC)wglGetProcAddress("glIsVertexArray");
        }
        else if (extension == "GL_ARB_vertex_shader"s) // also GL_ARB_fragment_shader
        {
            glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
            glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)wglGetProcAddress("glGetActiveAttrib");
            glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
        }
        else if (extension == "GL_ARB_vertex_program"s) // also GL_ARB_fragment_program
        {
            glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
            glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
            glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)wglGetProcAddress("glGetVertexAttribdv");
            glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)wglGetProcAddress("glGetVertexAttribfv");
            glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)wglGetProcAddress("glGetVertexAttribiv");
            glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)wglGetProcAddress("glGetVertexAttribPointerv");
            glIsProgram = (PFNGLISPROGRAMPROC)wglGetProcAddress("glIsProgram");
            glIsShader = (PFNGLISSHADERPROC)wglGetProcAddress("glIsShader");
            glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)wglGetProcAddress("glVertexAttrib1d");
            glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)wglGetProcAddress("glVertexAttrib1dv");
            glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)wglGetProcAddress("glVertexAttrib1f");
            glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)wglGetProcAddress("glVertexAttrib1fv");
            glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)wglGetProcAddress("glVertexAttrib1s");
            glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)wglGetProcAddress("glVertexAttrib1sv");
            glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)wglGetProcAddress("glVertexAttrib2d");
            glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)wglGetProcAddress("glVertexAttrib2dv");
            glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)wglGetProcAddress("glVertexAttrib2f");
            glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)wglGetProcAddress("glVertexAttrib2fv");
            glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)wglGetProcAddress("glVertexAttrib2s");
            glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)wglGetProcAddress("glVertexAttrib2sv");
            glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)wglGetProcAddress("glVertexAttrib3d");
            glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)wglGetProcAddress("glVertexAttrib3dv");
            glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)wglGetProcAddress("glVertexAttrib3f");
            glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)wglGetProcAddress("glVertexAttrib3fv");
            glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)wglGetProcAddress("glVertexAttrib3s");
            glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)wglGetProcAddress("glVertexAttrib3sv");
            glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)wglGetProcAddress("glVertexAttrib4Nbv");
            glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)wglGetProcAddress("glVertexAttrib4Niv");
            glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)wglGetProcAddress("glVertexAttrib4Nsv");
            glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)wglGetProcAddress("glVertexAttrib4Nub");
            glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)wglGetProcAddress("glVertexAttrib4Nubv");
            glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)wglGetProcAddress("glVertexAttrib4Nuiv");
            glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)wglGetProcAddress("glVertexAttrib4Nusv");
            glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)wglGetProcAddress("glVertexAttrib4bv");
            glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)wglGetProcAddress("glVertexAttrib4d");
            glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)wglGetProcAddress("glVertexAttrib4dv");
            glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)wglGetProcAddress("glVertexAttrib4f");
            glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)wglGetProcAddress("glVertexAttrib4fv");
            glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)wglGetProcAddress("glVertexAttrib4iv");
            glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)wglGetProcAddress("glVertexAttrib4s");
            glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)wglGetProcAddress("glVertexAttrib4sv");
            glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)wglGetProcAddress("glVertexAttrib4ubv");
            glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)wglGetProcAddress("glVertexAttrib4uiv");
            glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)wglGetProcAddress("glVertexAttrib4usv");
            glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");            
        }
        else if (extension == "GL_ARB_debug_output")
        {
            glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)wglGetProcAddress("glDebugMessageControl");
            glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)wglGetProcAddress("glDebugMessageInsert");
            glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
            glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)wglGetProcAddress("glGetDebugMessageLog");            
        }
        else if (extension == "GL_ARB_direct_state_access")
        {
            // for transform feedback object
            glCreateTransformFeedbacks = (PFNGLCREATETRANSFORMFEEDBACKSPROC)wglGetProcAddress("glCreateTransformFeedbacks");
            glTransformFeedbackBufferBase = (PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC)wglGetProcAddress("glTransformFeedbackBufferBase");
            glTransformFeedbackBufferRange = (PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC)wglGetProcAddress("glTransformFeedbackBufferRange");
            glGetTransformFeedbackiv = (PFNGLGETTRANSFORMFEEDBACKIVPROC)wglGetProcAddress("pglGetTransformFeedbackiv");
            glGetTransformFeedbacki_v = (PFNGLGETTRANSFORMFEEDBACKI_VPROC)wglGetProcAddress("pglGetTransformFeedbacki_v");
            glGetTransformFeedbacki64_v = (PFNGLGETTRANSFORMFEEDBACKI64_VPROC)wglGetProcAddress("pglGetTransformFeedbacki64_v");
            // for buffer object
            glCreateBuffers = (PFNGLCREATEBUFFERSPROC)wglGetProcAddress("glCreateBuffers");
            glNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGEPROC)wglGetProcAddress("glNamedBufferStorage");
            glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC)wglGetProcAddress("glNamedBufferData");
            glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC)wglGetProcAddress("glNamedBufferSubData");
            glCopyNamedBufferSubData = (PFNGLCOPYNAMEDBUFFERSUBDATAPROC)wglGetProcAddress("glCopyNamedBufferSubData");
            glClearNamedBufferData = (PFNGLCLEARNAMEDBUFFERDATAPROC)wglGetProcAddress("glClearNamedBufferData");
            glClearNamedBufferSubData = (PFNGLCLEARNAMEDBUFFERSUBDATAPROC)wglGetProcAddress("glClearNamedBufferSubData");
            glMapNamedBuffer = (PFNGLMAPNAMEDBUFFERPROC)wglGetProcAddress("glMapNamedBuffer");
            glMapNamedBufferRange = (PFNGLMAPNAMEDBUFFERRANGEPROC)wglGetProcAddress("glMapNamedBufferRange");
            glUnmapNamedBuffer = (PFNGLUNMAPNAMEDBUFFERPROC)wglGetProcAddress("glUnmapNamedBuffer");
            glFlushMappedNamedBufferRange = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC)wglGetProcAddress("glFlushMappedNamedBufferRange");
            glGetNamedBufferParameteriv = (PFNGLGETNAMEDBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetNamedBufferParameteriv");
            glGetNamedBufferParameteri64v = (PFNGLGETNAMEDBUFFERPARAMETERI64VPROC)wglGetProcAddress("glGetNamedBufferParameteri64v");
            glGetNamedBufferPointerv = (PFNGLGETNAMEDBUFFERPOINTERVPROC)wglGetProcAddress("glGetNamedBufferPointerv");
            glGetNamedBufferSubData = (PFNGLGETNAMEDBUFFERSUBDATAPROC)wglGetProcAddress("glGetNamedBufferSubData");
            // for framebuffer object
            glCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERSPROC)wglGetProcAddress("glCreateFramebuffers");
            glNamedFramebufferRenderbuffer = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glNamedFramebufferRenderbuffer");
            glNamedFramebufferParameteri = (PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC)wglGetProcAddress("glNamedFramebufferParameteri");
            glNamedFramebufferTexture = (PFNGLNAMEDFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glNamedFramebufferTexture");
            glNamedFramebufferTextureLayer = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC)wglGetProcAddress("glNamedFramebufferTextureLayer");
            glNamedFramebufferDrawBuffer = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC)wglGetProcAddress("glNamedFramebufferDrawBuffer");
            glNamedFramebufferDrawBuffers = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC)wglGetProcAddress("glNamedFramebufferDrawBuffers");
            glNamedFramebufferReadBuffer = (PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC)wglGetProcAddress("glNamedFramebufferReadBuffer");
            glInvalidateNamedFramebufferData = (PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC)wglGetProcAddress("glInvalidateNamedFramebufferData");
            glInvalidateNamedFramebufferSubData = (PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC)wglGetProcAddress("glInvalidateNamedFramebufferSubData");
            glClearNamedFramebufferiv = (PFNGLCLEARNAMEDFRAMEBUFFERIVPROC)wglGetProcAddress("glClearNamedFramebufferiv");
            glClearNamedFramebufferuiv = (PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC)wglGetProcAddress("glClearNamedFramebufferuiv");
            glClearNamedFramebufferfv = (PFNGLCLEARNAMEDFRAMEBUFFERFVPROC)wglGetProcAddress("glClearNamedFramebufferfv");
            glClearNamedFramebufferfi = (PFNGLCLEARNAMEDFRAMEBUFFERFIPROC)wglGetProcAddress("glClearNamedFramebufferfi");
            glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC)wglGetProcAddress("glBlitNamedFramebuffer");
            glCheckNamedFramebufferStatus = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckNamedFramebufferStatus");
            glGetNamedFramebufferParameteriv = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetNamedFramebufferParameteriv");
            glGetNamedFramebufferAttachmentParameteriv = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetNamedFramebufferAttachmentParameteriv");
            // for renderbuffer object
            glCreateRenderbuffers = (PFNGLCREATERENDERBUFFERSPROC)wglGetProcAddress("glCreateRenderbuffers");
            glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glNamedRenderbufferStorage");
            glNamedRenderbufferStorageMultisample = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glNamedRenderbufferStorageMultisample");
            glGetNamedRenderbufferParameteriv = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetNamedRenderbufferParameteriv");
            // for texture object
            glCreateTextures = (PFNGLCREATETEXTURESPROC)wglGetProcAddress("glCreateTextures");
            glTextureBuffer = (PFNGLTEXTUREBUFFERPROC)wglGetProcAddress("glTextureBuffer");
            glTextureBufferRange = (PFNGLTEXTUREBUFFERRANGEPROC)wglGetProcAddress("glTextureBufferRange");
            glTextureStorage1D = (PFNGLTEXTURESTORAGE1DPROC)wglGetProcAddress("glTextureStorage1D");
            glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)wglGetProcAddress("glTextureStorage2D");
            glTextureStorage3D = (PFNGLTEXTURESTORAGE3DPROC)wglGetProcAddress("glTextureStorage3D");
            glTextureStorage2DMultisample = (PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC)wglGetProcAddress("glTextureStorage2DMultisample");
            glTextureStorage3DMultisample = (PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC)wglGetProcAddress("glTextureStorage3DMultisample");
            glTextureSubImage1D = (PFNGLTEXTURESUBIMAGE1DPROC)wglGetProcAddress("glTextureSubImage1D");
            glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)wglGetProcAddress("glTextureSubImage2D");
            glTextureSubImage3D = (PFNGLTEXTURESUBIMAGE3DPROC)wglGetProcAddress("glTextureSubImage3D");
            glCompressedTextureSubImage1D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC)wglGetProcAddress("glCompressedTextureSubImage1D");
            glCompressedTextureSubImage2D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC)wglGetProcAddress("glCompressedTextureSubImage2D");
            glCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC)wglGetProcAddress("glCompressedTextureSubImage3D");
            glCopyTextureSubImage1D = (PFNGLCOPYTEXTURESUBIMAGE1DPROC)wglGetProcAddress("glCopyTextureSubImage1D");
            glCopyTextureSubImage2D = (PFNGLCOPYTEXTURESUBIMAGE2DPROC)wglGetProcAddress("glCopyTextureSubImage2D");
            glCopyTextureSubImage3D = (PFNGLCOPYTEXTURESUBIMAGE3DPROC)wglGetProcAddress("glCopyTextureSubImage3D");
            glTextureParameterf = (PFNGLTEXTUREPARAMETERFPROC)wglGetProcAddress("glTextureParameterf");
            glTextureParameterfv = (PFNGLTEXTUREPARAMETERFVPROC)wglGetProcAddress("glTextureParameterfv");
            glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)wglGetProcAddress("glTextureParameteri");
            glTextureParameterIiv = (PFNGLTEXTUREPARAMETERIIVPROC)wglGetProcAddress("glTextureParameterIiv");
            glTextureParameterIuiv = (PFNGLTEXTUREPARAMETERIUIVPROC)wglGetProcAddress("glTextureParameterIuiv");
            glTextureParameteriv = (PFNGLTEXTUREPARAMETERIVPROC)wglGetProcAddress("glTextureParameteriv");
            glGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC)wglGetProcAddress("glGenerateTextureMipmap");
            glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)wglGetProcAddress("glBindTextureUnit");
            glGetTextureImage = (PFNGLGETTEXTUREIMAGEPROC)wglGetProcAddress("glGetTextureImage");
            glGetCompressedTextureImage = (PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC)wglGetProcAddress("glGetCompressedTextureImage");
            glGetTextureLevelParameterfv = (PFNGLGETTEXTURELEVELPARAMETERFVPROC)wglGetProcAddress("glGetTextureLevelParameterfv");
            glGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIVPROC)wglGetProcAddress("glGetTextureLevelParameteriv");
            glGetTextureParameterfv = (PFNGLGETTEXTUREPARAMETERFVPROC)wglGetProcAddress("glGetTextureParameterfv");
            glGetTextureParameterIiv = (PFNGLGETTEXTUREPARAMETERIIVPROC)wglGetProcAddress("glGetTextureParameterIiv");
            glGetTextureParameterIuiv = (PFNGLGETTEXTUREPARAMETERIUIVPROC)wglGetProcAddress("glGetTextureParameterIuiv");
            glGetTextureParameteriv = (PFNGLGETTEXTUREPARAMETERIVPROC)wglGetProcAddress("glGetTextureParameteriv");
            // for vertex array object
            glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)wglGetProcAddress("glCreateVertexArrays");
            glDisableVertexArrayAttrib = (PFNGLDISABLEVERTEXARRAYATTRIBPROC)wglGetProcAddress("glDisableVertexArrayAttrib");
            glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC)wglGetProcAddress("glEnableVertexArrayAttrib");
            glVertexArrayElementBuffer = (PFNGLVERTEXARRAYELEMENTBUFFERPROC)wglGetProcAddress("glVertexArrayElementBuffer");
            glVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFERPROC)wglGetProcAddress("glVertexArrayVertexBuffer");
            glVertexArrayVertexBuffers = (PFNGLVERTEXARRAYVERTEXBUFFERSPROC)wglGetProcAddress("glVertexArrayVertexBuffers");
            glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC)wglGetProcAddress("glVertexArrayAttribBinding");
            glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC)wglGetProcAddress("glVertexArrayAttribFormat");
            glVertexArrayAttribIFormat = (PFNGLVERTEXARRAYATTRIBIFORMATPROC)wglGetProcAddress("glVertexArrayAttribIFormat");
            glVertexArrayAttribLFormat = (PFNGLVERTEXARRAYATTRIBLFORMATPROC)wglGetProcAddress("glVertexArrayAttribLFormat");
            glVertexArrayBindingDivisor = (PFNGLVERTEXARRAYBINDINGDIVISORPROC)wglGetProcAddress("glVertexArrayBindingDivisor");
            glGetVertexArrayiv = (PFNGLGETVERTEXARRAYIVPROC)wglGetProcAddress("glGetVertexArrayiv");
            glGetVertexArrayIndexediv = (PFNGLGETVERTEXARRAYINDEXEDIVPROC)wglGetProcAddress("glGetVertexArrayIndexediv");
            glGetVertexArrayIndexed64iv = (PFNGLGETVERTEXARRAYINDEXED64IVPROC)wglGetProcAddress("glGetVertexArrayIndexed64iv");
            // for sampler object
            glCreateSamplers = (PFNGLCREATESAMPLERSPROC)wglGetProcAddress("glCreateSamplers");
            // for program pipeline object
            glCreateProgramPipelines = (PFNGLCREATEPROGRAMPIPELINESPROC)wglGetProcAddress("glCreateProgramPipelines");
            // for query object
            glCreateQueries = (PFNGLCREATEQUERIESPROC)wglGetProcAddress("glCreateQueries");
            glGetQueryBufferObjectiv = (PFNGLGETQUERYBUFFEROBJECTIVPROC)wglGetProcAddress("glGetQueryBufferObjectiv");
            glGetQueryBufferObjectuiv = (PFNGLGETQUERYBUFFEROBJECTUIVPROC)wglGetProcAddress("glGetQueryBufferObjectuiv");
            glGetQueryBufferObjecti64v = (PFNGLGETQUERYBUFFEROBJECTI64VPROC)wglGetProcAddress("glGetQueryBufferObjecti64v");
            glGetQueryBufferObjectui64v = (PFNGLGETQUERYBUFFEROBJECTUI64VPROC)wglGetProcAddress("glGetQueryBufferObjectui64v");
        }
    }
}



const OpenGLExtensions& OpenGLExtensions::GetInstance()
{
	static OpenGLExtensions sOpenGLExtensions;
	return sOpenGLExtensions;
}



bool OpenGLExtensions::IsSupported(const std::string& ExtensionName) const
{
	for (auto& extension : m_SupportedExtensions)
		if (extension == ExtensionName)
			return true;

	return false;
}