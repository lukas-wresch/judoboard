#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <string>
#include <vector>
#include <GL/gl.h>
#include "../external/GL/glext.h"



namespace ZED
{
	class OpenGLExtensions
	{
	public:
		static const OpenGLExtensions& GetInstance();

		bool IsSupported(const std::string& ExtensionName) const;

		// GL_ARB_framebuffer_object
		static inline PFNGLGENFRAMEBUFFERSPROC                        glGenFramebuffers = 0;                     // FBO name generation procedure
		static inline PFNGLDELETEFRAMEBUFFERSPROC                     glDeleteFramebuffers = 0;                  // FBO deletion procedure
		static inline PFNGLBINDFRAMEBUFFERPROC                        glBindFramebuffer = 0;                     // FBO bind procedure
		static inline PFNGLCHECKFRAMEBUFFERSTATUSPROC                 glCheckFramebufferStatus = 0;              // FBO completeness test procedure
		static inline PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC    glGetFramebufferAttachmentParameteriv = 0; // return various FBO parameters
		static inline PFNGLGENERATEMIPMAPPROC                         glGenerateMipmap = 0;                      // FBO automatic mipmap generation procedure
		static inline PFNGLFRAMEBUFFERTEXTURE1DPROC                   glFramebufferTexture1D = 0;                // FBO 1D texture attachement procedure
		static inline PFNGLFRAMEBUFFERTEXTURE2DPROC                   glFramebufferTexture2D = 0;                // FBO 2D texture attachement procedure
		static inline PFNGLFRAMEBUFFERTEXTURE3DPROC                   glFramebufferTexture3D = 0;                // FBO 3D texture attachement procedure
		static inline PFNGLFRAMEBUFFERTEXTURELAYERPROC                glFramebufferTextureLayer = 0;             // FBO 3D texture layer attachement procedure
		static inline PFNGLFRAMEBUFFERRENDERBUFFERPROC                glFramebufferRenderbuffer = 0;             // FBO renderbuffer attachement procedure
		static inline PFNGLISFRAMEBUFFERPROC                          glIsFramebuffer = 0;                       // FBO state = true/false
		static inline PFNGLBLITFRAMEBUFFERPROC                        glBlitFramebuffer = 0;                     // FBO copy
		static inline PFNGLGENRENDERBUFFERSPROC                       glGenRenderbuffers = 0;                    // renderbuffer generation procedure
		static inline PFNGLDELETERENDERBUFFERSPROC                    glDeleteRenderbuffers = 0;                 // renderbuffer deletion procedure
		static inline PFNGLBINDRENDERBUFFERPROC                       glBindRenderbuffer = 0;                    // renderbuffer bind procedure
		static inline PFNGLRENDERBUFFERSTORAGEPROC                    glRenderbufferStorage = 0;                 // renderbuffer memory allocation procedure
		static inline PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC         glRenderbufferStorageMultisample = 0;      // renderbuffer memory allocation with multisample
		static inline PFNGLGETRENDERBUFFERPARAMETERIVPROC             glGetRenderbufferParameteriv = 0;          // return various renderbuffer parameters
		static inline PFNGLISRENDERBUFFERPROC                         glIsRenderbuffer = 0;                      // determine renderbuffer object type

		// GL_ARB_multisample
		static inline PFNGLSAMPLECOVERAGEARBPROC  glSampleCoverageARB = 0;

		// GL_ARB_multitexture
		//@@ v1.2.1 core version
		static inline PFNGLACTIVETEXTUREPROC   glActiveTexture = 0;

		// GL_ARB_pixel_buffer_objects & GL_ARB_vertex_buffer_object
		//@@ v2.1 core version
		static inline PFNGLGENBUFFERSPROC              glGenBuffers = 0;             // VBO Name Generation Procedure
		static inline PFNGLBINDBUFFERPROC              glBindBuffer = 0;             // VBO Bind Procedure
		static inline PFNGLBUFFERDATAPROC              glBufferData = 0;             // VBO Data Loading Procedure
		static inline PFNGLBUFFERSUBDATAPROC           glBufferSubData = 0;          // VBO Sub Data Loading Procedure
		static inline PFNGLDELETEBUFFERSPROC           glDeleteBuffers = 0;          // VBO Deletion Procedure
		static inline PFNGLGETBUFFERPARAMETERIVPROC    glGetBufferParameteriv = 0;   // return various parameters of VBO
		static inline PFNGLMAPBUFFERPROC               glMapBuffer = 0;              // map VBO procedure
		static inline PFNGLUNMAPBUFFERPROC             glUnmapBuffer = 0;            // unmap VBO procedure

		// GL_ARB_shader_objects
		//@@ v2.0 core version:
		static inline PFNGLATTACHSHADERPROC           glAttachShader = 0;        // attach a shader to a program
		static inline PFNGLCOMPILESHADERPROC          glCompileShader = 0;       // compile shader source
		static inline PFNGLCREATEPROGRAMPROC          glCreateProgram = 0;       // create a program object
		static inline PFNGLCREATESHADERPROC           glCreateShader = 0;        // create a shader object
		static inline PFNGLDELETEPROGRAMPROC          glDeleteProgram = 0;       // delete shader program
		static inline PFNGLDELETESHADERPROC           glDeleteShader = 0;        // delete shader object
		static inline PFNGLDETACHSHADERPROC           glDetachShader = 0;        // detatch a shader object from a program
		static inline PFNGLGETACTIVEUNIFORMPROC       glGetActiveUniform = 0;    // get info of uniform var
		static inline PFNGLGETATTACHEDSHADERSPROC     glGetAttachedShaders = 0;  // get attached shaders to a program
		static inline PFNGLGETPROGRAMIVPROC           glGetProgramiv = 0;        // return param of program object
		static inline PFNGLGETPROGRAMINFOLOGPROC      glGetProgramInfoLog = 0;   // return info log of program
		static inline PFNGLGETSHADERIVPROC            glGetShaderiv = 0;         // return param of shader object
		static inline PFNGLGETSHADERINFOLOGPROC       glGetShaderInfoLog = 0;    // return info log of shader
		static inline PFNGLGETSHADERSOURCEPROC        glGetShaderSource = 0;     // get shader source codes
		static inline PFNGLGETUNIFORMLOCATIONPROC     glGetUniformLocation = 0;  // get index of uniform var
		static inline PFNGLGETUNIFORMFVPROC           glGetUniformfv = 0;        // get value of uniform var
		static inline PFNGLGETUNIFORMIVPROC           glGetUniformiv = 0;        //
		static inline PFNGLLINKPROGRAMPROC            glLinkProgram = 0;         // link a program
		static inline PFNGLSHADERSOURCEPROC           glShaderSource = 0;        // set a shader source(codes)
		static inline PFNGLUSEPROGRAMPROC             glUseProgram = 0;          // use a program
		static inline PFNGLUNIFORM1FPROC              glUniform1f = 0;           //
		static inline PFNGLUNIFORM2FPROC              glUniform2f = 0;           //
		static inline PFNGLUNIFORM3FPROC              glUniform3f = 0;           //
		static inline PFNGLUNIFORM4FPROC              glUniform4f = 0;           //
		static inline PFNGLUNIFORM1IPROC              glUniform1i = 0;           //
		static inline PFNGLUNIFORM2IPROC              glUniform2i = 0;           //
		static inline PFNGLUNIFORM3IPROC              glUniform3i = 0;           //
		static inline PFNGLUNIFORM4IPROC              glUniform4i = 0;           //
		static inline PFNGLUNIFORM1FVPROC             glUniform1fv = 0;          //
		static inline PFNGLUNIFORM2FVPROC             glUniform2fv = 0;          //
		static inline PFNGLUNIFORM3FVPROC             glUniform3fv = 0;          //
		static inline PFNGLUNIFORM4FVPROC             glUniform4fv = 0;          //
		static inline PFNGLUNIFORM1FVPROC             glUniform1iv = 0;          //
		static inline PFNGLUNIFORM2FVPROC             glUniform2iv = 0;          //
		static inline PFNGLUNIFORM3FVPROC             glUniform3iv = 0;          //
		static inline PFNGLUNIFORM4FVPROC             glUniform4iv = 0;          //
		static inline PFNGLUNIFORMMATRIX2FVPROC       glUniformMatrix2fv = 0;    //
		static inline PFNGLUNIFORMMATRIX3FVPROC       glUniformMatrix3fv = 0;    //
		static inline PFNGLUNIFORMMATRIX4FVPROC       glUniformMatrix4fv = 0;    //
		static inline PFNGLVALIDATEPROGRAMPROC        glValidateProgram = 0;     // validate a program

		// GL_ARB_sync extension
		static inline PFNGLFENCESYNCPROC          glFenceSync = 0;
		static inline PFNGLISSYNCPROC             glIsSync = 0;
		static inline PFNGLDELETESYNCPROC         glDeleteSync = 0;
		static inline PFNGLCLIENTWAITSYNCPROC     glClientWaitSync = 0;
		static inline PFNGLWAITSYNCPROC           glWaitSync = 0;
		static inline PFNGLGETINTEGER64VPROC      glGetInteger64v = 0;
		static inline PFNGLGETSYNCIVPROC          glGetSynciv = 0;

		// GL_ARB_vertex_array_object
		static inline PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays = 0;     // VAO name generation procedure
		static inline PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = 0;  // VAO deletion procedure
		static inline PFNGLBINDVERTEXARRAYPROC    glBindVertexArray = 0;     // VAO bind procedure
		static inline PFNGLISVERTEXARRAYPROC      glIsVertexArray = 0;       // VBO query procedure


		// GL_ARB_vertex_shader and GL_ARB_fragment_shader extensions
		//@@ v2.0 core version
		static inline PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = 0;  // bind vertex attrib var with index
		static inline PFNGLGETACTIVEATTRIBPROC    glGetActiveAttrib = 0;     // get attrib value
		static inline PFNGLGETATTRIBLOCATIONPROC  glGetAttribLocation = 0;   // get lndex of attrib var

		// GL_ARB_vertex_program and GL_ARB_fragment_program
		//@@ v2.0 core version
		static inline PFNGLDISABLEVERTEXATTRIBARRAYPROC   glDisableVertexAttribArray = 0;
		static inline PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray = 0;
		static inline PFNGLGETVERTEXATTRIBDVPROC          glGetVertexAttribdv = 0;
		static inline PFNGLGETVERTEXATTRIBFVPROC          glGetVertexAttribfv = 0;
		static inline PFNGLGETVERTEXATTRIBIVPROC          glGetVertexAttribiv = 0;
		static inline PFNGLGETVERTEXATTRIBPOINTERVPROC    glGetVertexAttribPointerv = 0;
		static inline PFNGLISPROGRAMPROC                  glIsProgram = 0;
		static inline PFNGLISSHADERPROC                   glIsShader = 0;
		static inline PFNGLVERTEXATTRIB1DPROC             glVertexAttrib1d = 0;
		static inline PFNGLVERTEXATTRIB1DVPROC            glVertexAttrib1dv = 0;
		static inline PFNGLVERTEXATTRIB1FPROC             glVertexAttrib1f = 0;
		static inline PFNGLVERTEXATTRIB1FVPROC            glVertexAttrib1fv = 0;
		static inline PFNGLVERTEXATTRIB1SPROC             glVertexAttrib1s = 0;
		static inline PFNGLVERTEXATTRIB1SVPROC            glVertexAttrib1sv = 0;
		static inline PFNGLVERTEXATTRIB2DPROC             glVertexAttrib2d = 0;
		static inline PFNGLVERTEXATTRIB2DVPROC            glVertexAttrib2dv = 0;
		static inline PFNGLVERTEXATTRIB2FPROC             glVertexAttrib2f = 0;
		static inline PFNGLVERTEXATTRIB2FVPROC            glVertexAttrib2fv = 0;
		static inline PFNGLVERTEXATTRIB2SPROC             glVertexAttrib2s = 0;
		static inline PFNGLVERTEXATTRIB2SVPROC            glVertexAttrib2sv = 0;
		static inline PFNGLVERTEXATTRIB3DPROC             glVertexAttrib3d = 0;
		static inline PFNGLVERTEXATTRIB3DVPROC            glVertexAttrib3dv = 0;
		static inline PFNGLVERTEXATTRIB3FPROC             glVertexAttrib3f = 0;
		static inline PFNGLVERTEXATTRIB3FVPROC            glVertexAttrib3fv = 0;
		static inline PFNGLVERTEXATTRIB3SPROC             glVertexAttrib3s = 0;
		static inline PFNGLVERTEXATTRIB3SVPROC            glVertexAttrib3sv = 0;
		static inline PFNGLVERTEXATTRIB4NBVPROC           glVertexAttrib4Nbv = 0;
		static inline PFNGLVERTEXATTRIB4NIVPROC           glVertexAttrib4Niv = 0;
		static inline PFNGLVERTEXATTRIB4NSVPROC           glVertexAttrib4Nsv = 0;
		static inline PFNGLVERTEXATTRIB4NUBPROC           glVertexAttrib4Nub = 0;
		static inline PFNGLVERTEXATTRIB4NUBVPROC          glVertexAttrib4Nubv = 0;
		static inline PFNGLVERTEXATTRIB4NUIVPROC          glVertexAttrib4Nuiv = 0;
		static inline PFNGLVERTEXATTRIB4NUSVPROC          glVertexAttrib4Nusv = 0;
		static inline PFNGLVERTEXATTRIB4BVPROC            glVertexAttrib4bv = 0;
		static inline PFNGLVERTEXATTRIB4DPROC             glVertexAttrib4d = 0;
		static inline PFNGLVERTEXATTRIB4DVPROC            glVertexAttrib4dv = 0;
		static inline PFNGLVERTEXATTRIB4FPROC             glVertexAttrib4f = 0;
		static inline PFNGLVERTEXATTRIB4FVPROC            glVertexAttrib4fv = 0;
		static inline PFNGLVERTEXATTRIB4IVPROC            glVertexAttrib4iv = 0;
		static inline PFNGLVERTEXATTRIB4SPROC             glVertexAttrib4s = 0;
		static inline PFNGLVERTEXATTRIB4SVPROC            glVertexAttrib4sv = 0;
		static inline PFNGLVERTEXATTRIB4UBVPROC           glVertexAttrib4ubv = 0;
		static inline PFNGLVERTEXATTRIB4UIVPROC           glVertexAttrib4uiv = 0;
		static inline PFNGLVERTEXATTRIB4USVPROC           glVertexAttrib4usv = 0;
		static inline PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer = 0;

		// GL_ARB_debug_output
		//@@ v4.3 core version
		static inline PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl = 0;
		static inline PFNGLDEBUGMESSAGEINSERTPROC   glDebugMessageInsert = 0;
		static inline PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = 0;
		static inline PFNGLGETDEBUGMESSAGELOGPROC   glGetDebugMessageLog = 0;

		// GL_ARB_direct_state_access
		static inline PFNGLCREATETRANSFORMFEEDBACKSPROC                 glCreateTransformFeedbacks = 0; // for transform feedback object
		static inline PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC              glTransformFeedbackBufferBase = 0;
		static inline PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC             glTransformFeedbackBufferRange = 0;
		static inline PFNGLGETTRANSFORMFEEDBACKIVPROC                   glGetTransformFeedbackiv = 0;
		static inline PFNGLGETTRANSFORMFEEDBACKI_VPROC                  glGetTransformFeedbacki_v = 0;
		static inline PFNGLGETTRANSFORMFEEDBACKI64_VPROC                glGetTransformFeedbacki64_v = 0;
		static inline PFNGLCREATEBUFFERSPROC                            glCreateBuffers = 0;          // for buffer object
		static inline PFNGLNAMEDBUFFERSTORAGEPROC                       glNamedBufferStorage = 0;
		static inline PFNGLNAMEDBUFFERDATAPROC                          glNamedBufferData = 0;
		static inline PFNGLNAMEDBUFFERSUBDATAPROC                       glNamedBufferSubData = 0;
		static inline PFNGLCOPYNAMEDBUFFERSUBDATAPROC                   glCopyNamedBufferSubData = 0;
		static inline PFNGLCLEARNAMEDBUFFERDATAPROC                     glClearNamedBufferData = 0;
		static inline PFNGLCLEARNAMEDBUFFERSUBDATAPROC                  glClearNamedBufferSubData = 0;
		static inline PFNGLMAPNAMEDBUFFERPROC                           glMapNamedBuffer = 0;
		static inline PFNGLMAPNAMEDBUFFERRANGEPROC                      glMapNamedBufferRange = 0;
		static inline PFNGLUNMAPNAMEDBUFFERPROC                         glUnmapNamedBuffer = 0;
		static inline PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC              glFlushMappedNamedBufferRange = 0;
		static inline PFNGLGETNAMEDBUFFERPARAMETERIVPROC                glGetNamedBufferParameteriv = 0;
		static inline PFNGLGETNAMEDBUFFERPARAMETERI64VPROC              glGetNamedBufferParameteri64v = 0;
		static inline PFNGLGETNAMEDBUFFERPOINTERVPROC                   glGetNamedBufferPointerv = 0;
		static inline PFNGLGETNAMEDBUFFERSUBDATAPROC                    glGetNamedBufferSubData = 0;
		static inline PFNGLCREATEFRAMEBUFFERSPROC                       glCreateFramebuffers = 0;     // for framebuffer object
		static inline PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC             glNamedFramebufferRenderbuffer = 0;
		static inline PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC               glNamedFramebufferParameteri = 0;
		static inline PFNGLNAMEDFRAMEBUFFERTEXTUREPROC                  glNamedFramebufferTexture = 0;
		static inline PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC             glNamedFramebufferTextureLayer = 0;
		static inline PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC               glNamedFramebufferDrawBuffer = 0;
		static inline PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC              glNamedFramebufferDrawBuffers = 0;
		static inline PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC               glNamedFramebufferReadBuffer = 0;
		static inline PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC           glInvalidateNamedFramebufferData = 0;
		static inline PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC        glInvalidateNamedFramebufferSubData = 0;
		static inline PFNGLCLEARNAMEDFRAMEBUFFERIVPROC                  glClearNamedFramebufferiv = 0;
		static inline PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC                 glClearNamedFramebufferuiv = 0;
		static inline PFNGLCLEARNAMEDFRAMEBUFFERFVPROC                  glClearNamedFramebufferfv = 0;
		static inline PFNGLCLEARNAMEDFRAMEBUFFERFIPROC                  glClearNamedFramebufferfi = 0;
		static inline PFNGLBLITNAMEDFRAMEBUFFERPROC                     glBlitNamedFramebuffer = 0;
		static inline PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC              glCheckNamedFramebufferStatus = 0;
		static inline PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC           glGetNamedFramebufferParameteriv = 0;
		static inline PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetNamedFramebufferAttachmentParameteriv = 0;
		static inline PFNGLCREATERENDERBUFFERSPROC                      glCreateRenderbuffers = 0;    // for renderbuffer object
		static inline PFNGLNAMEDRENDERBUFFERSTORAGEPROC                 glNamedRenderbufferStorage = 0;
		static inline PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC      glNamedRenderbufferStorageMultisample = 0;
		static inline PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC          glGetNamedRenderbufferParameteriv = 0;
		static inline PFNGLCREATETEXTURESPROC                           glCreateTextures = 0;         // for texture object
		static inline PFNGLTEXTUREBUFFERPROC                            glTextureBuffer = 0;
		static inline PFNGLTEXTUREBUFFERRANGEPROC                       glTextureBufferRange = 0;
		static inline PFNGLTEXTURESTORAGE1DPROC                         glTextureStorage1D = 0;
		static inline PFNGLTEXTURESTORAGE2DPROC                         glTextureStorage2D = 0;
		static inline PFNGLTEXTURESTORAGE3DPROC                         glTextureStorage3D = 0;
		static inline PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC              glTextureStorage2DMultisample = 0;
		static inline PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC              glTextureStorage3DMultisample = 0;
		static inline PFNGLTEXTURESUBIMAGE1DPROC                        glTextureSubImage1D = 0;
		static inline PFNGLTEXTURESUBIMAGE2DPROC                        glTextureSubImage2D = 0;
		static inline PFNGLTEXTURESUBIMAGE3DPROC                        glTextureSubImage3D = 0;
		static inline PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC              glCompressedTextureSubImage1D = 0;
		static inline PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC              glCompressedTextureSubImage2D = 0;
		static inline PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC              glCompressedTextureSubImage3D = 0;
		static inline PFNGLCOPYTEXTURESUBIMAGE1DPROC                    glCopyTextureSubImage1D = 0;
		static inline PFNGLCOPYTEXTURESUBIMAGE2DPROC                    glCopyTextureSubImage2D = 0;
		static inline PFNGLCOPYTEXTURESUBIMAGE3DPROC                    glCopyTextureSubImage3D = 0;
		static inline PFNGLTEXTUREPARAMETERFPROC                        glTextureParameterf = 0;
		static inline PFNGLTEXTUREPARAMETERFVPROC                       glTextureParameterfv = 0;
		static inline PFNGLTEXTUREPARAMETERIPROC                        glTextureParameteri = 0;
		static inline PFNGLTEXTUREPARAMETERIIVPROC                      glTextureParameterIiv = 0;
		static inline PFNGLTEXTUREPARAMETERIUIVPROC                     glTextureParameterIuiv = 0;
		static inline PFNGLTEXTUREPARAMETERIVPROC                       glTextureParameteriv = 0;
		static inline PFNGLGENERATETEXTUREMIPMAPPROC                    glGenerateTextureMipmap = 0;
		static inline PFNGLBINDTEXTUREUNITPROC                          glBindTextureUnit = 0;
		static inline PFNGLGETTEXTUREIMAGEPROC                          glGetTextureImage = 0;
		static inline PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC                glGetCompressedTextureImage = 0;
		static inline PFNGLGETTEXTURELEVELPARAMETERFVPROC               glGetTextureLevelParameterfv = 0;
		static inline PFNGLGETTEXTURELEVELPARAMETERIVPROC               glGetTextureLevelParameteriv = 0;
		static inline PFNGLGETTEXTUREPARAMETERFVPROC                    glGetTextureParameterfv = 0;
		static inline PFNGLGETTEXTUREPARAMETERIIVPROC                   glGetTextureParameterIiv = 0;
		static inline PFNGLGETTEXTUREPARAMETERIUIVPROC                  glGetTextureParameterIuiv = 0;
		static inline PFNGLGETTEXTUREPARAMETERIVPROC                    glGetTextureParameteriv = 0;
		static inline PFNGLCREATEVERTEXARRAYSPROC                       glCreateVertexArrays = 0;     // for vertex array object
		static inline PFNGLDISABLEVERTEXARRAYATTRIBPROC                 glDisableVertexArrayAttrib = 0;
		static inline PFNGLENABLEVERTEXARRAYATTRIBPROC                  glEnableVertexArrayAttrib = 0;
		static inline PFNGLVERTEXARRAYELEMENTBUFFERPROC                 glVertexArrayElementBuffer = 0;
		static inline PFNGLVERTEXARRAYVERTEXBUFFERPROC                  glVertexArrayVertexBuffer = 0;
		static inline PFNGLVERTEXARRAYVERTEXBUFFERSPROC                 glVertexArrayVertexBuffers = 0;
		static inline PFNGLVERTEXARRAYATTRIBBINDINGPROC                 glVertexArrayAttribBinding = 0;
		static inline PFNGLVERTEXARRAYATTRIBFORMATPROC                  glVertexArrayAttribFormat = 0;
		static inline PFNGLVERTEXARRAYATTRIBIFORMATPROC                 glVertexArrayAttribIFormat = 0;
		static inline PFNGLVERTEXARRAYATTRIBLFORMATPROC                 glVertexArrayAttribLFormat = 0;
		static inline PFNGLVERTEXARRAYBINDINGDIVISORPROC                glVertexArrayBindingDivisor = 0;
		static inline PFNGLGETVERTEXARRAYIVPROC                         glGetVertexArrayiv = 0;
		static inline PFNGLGETVERTEXARRAYINDEXEDIVPROC                  glGetVertexArrayIndexediv = 0;
		static inline PFNGLGETVERTEXARRAYINDEXED64IVPROC                glGetVertexArrayIndexed64iv = 0;
		static inline PFNGLCREATESAMPLERSPROC                           glCreateSamplers = 0;         // for sampler object
		static inline PFNGLCREATEPROGRAMPIPELINESPROC                   glCreateProgramPipelines = 0; // for program pipeline object
		static inline PFNGLCREATEQUERIESPROC                            glCreateQueries = 0;          // for query object
		static inline PFNGLGETQUERYBUFFEROBJECTIVPROC                   glGetQueryBufferObjectiv = 0;
		static inline PFNGLGETQUERYBUFFEROBJECTUIVPROC                  glGetQueryBufferObjectuiv = 0;
		static inline PFNGLGETQUERYBUFFEROBJECTI64VPROC                 glGetQueryBufferObjecti64v = 0;
		static inline PFNGLGETQUERYBUFFEROBJECTUI64VPROC                glGetQueryBufferObjectui64v = 0;

	private:
		OpenGLExtensions();

		std::vector <std::string> m_SupportedExtensions;
	};
}