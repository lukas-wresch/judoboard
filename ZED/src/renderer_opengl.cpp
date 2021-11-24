#include "../include/renderer_opengl.h"
#include "../include/opengl_extensions.h"
#include "../include/log.h"
#define SDL_DISABLE_IMMINTRIN_H
#include "../external/SDL2-2.0.14/include/SDL_opengl.h"
#include "../external/SDL2-2.0.14/include/SDL.h"
//#include "../external/SDL2-2.0.14/include/SDL_image.h"

#pragma comment(lib, "opengl32.lib")


using namespace ZED;



unsigned int ShaderOpenGL::GetUniformLocation(const char* VariableName) const
{
	return OpenGLExtensions::glGetUniformLocation(m_ProgramID, VariableName);
}



void ShaderOpenGL::SetUniform(unsigned int Location, unsigned int Value) const
{
	OpenGLExtensions::glUniform1i(Location, Value);
}



void ShaderOpenGL::SetUniform(unsigned int Location, unsigned int Value1, unsigned int Value2) const
{
	OpenGLExtensions::glUniform2i(Location, Value1, Value2);
}



void ShaderOpenGL::SetUniform(unsigned int Location, float Value) const
{
	OpenGLExtensions::glUniform1f(Location, Value);
}



void ShaderOpenGL::SetUniform(unsigned int Location, float Value1, float Value2) const
{
	OpenGLExtensions::glUniform2f(Location, Value1, Value2);
}



void ShaderOpenGL::SetShader() const
{
	OpenGLExtensions::glUseProgram(m_ProgramID);
}



#ifdef _WIN32
bool ZED::RendererOpenGL::Init(HWND Hwnd)
#else
bool ZED::RendererOpenGL::Init(GtkWindow* DrawingArea)
#endif
{
#ifdef _WIN32
	if (!RendererSDL2::Init(Hwnd))
#else
	if (!RendererSDL2::Init(DrawingArea))
#endif
		return false;


	//Only needed for OpenGL
	m_glcontext = SDL_GL_CreateContext(m_SDL_window);

	if (!m_glcontext)
		return false;

	//SDL_GL_SetSwapInterval(0);//Disable Vsync

	glEnable(GL_TEXTURE_2D);//Enable Texture Mapping
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);//Clear The Background Color To Black 
	glClearDepth(0.0f);//Clearing Of The Depth Buffer
	glDepthFunc(GL_ALWAYS);
	//glEnable(GL_DEPTH_TEST);//Enables Depth Testing
	glDisable(GL_DEPTH_TEST);//Disable Depth test
	glDepthMask(GL_FALSE);//Disable writing to ZBuffer
	glShadeModel(GL_SMOOTH);//Enables Smooth Color Shading

	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT);//Clear The Screen And The Depth Buffer

	glClearStencil(0x00);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0xFF);//Enable writing
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glLoadIdentity();//Reset the view

	ResetViewPort(m_screen_w, m_screen_h);

	ZED::Log::Info("OpenGL renderer initialized");
	return true;
}



void ZED::RendererOpenGL::ResetViewPort(unsigned int width, unsigned int height)
{
	if (!m_glcontext)
		return;

	m_screen_w = width;
	m_screen_h = height;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();//Reset The Projection Matrix

	glOrtho(0.0f, width, height, 0.0f, 1.0f, -1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	auto& ext = OpenGLExtensions::GetInstance();
	const bool pboSupported = ext.IsSupported("GL_ARB_pixel_buffer_object") && ext.IsSupported("GL_ARB_vertex_buffer_object");

	if (pboSupported)
	{
		if (m_PBO[0])
			OpenGLExtensions::glDeleteBuffers(2, m_PBO);

		const unsigned int data_size = (width+1) * (height+1) * 4;//The +1 helps with crashes due to data alignment (only in release builds)
		OpenGLExtensions::glGenBuffers(2, m_PBO);
		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO[0]);
		OpenGLExtensions::glBufferData(GL_PIXEL_PACK_BUFFER, data_size, 0, GL_STREAM_READ);
		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO[1]);
		OpenGLExtensions::glBufferData(GL_PIXEL_PACK_BUFFER, data_size, 0, GL_STREAM_READ);

		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	ZED::Log::Debug("Screen resized to " + std::to_string(m_screen_w) + "x" + std::to_string(m_screen_h));
}



void ZED::RendererOpenGL::ClearDisplay() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}



void RendererOpenGL::SetClearColor(const ColorF& Color)
{
	glClearColor(Color.r, Color.g, Color.b, 0.0f);
}



void ZED::RendererOpenGL::UpdateDisplay() const
{
	SDL_GL_SwapWindow(m_SDL_window);
}



void ZED::RendererOpenGL::Release()
{
	m_screen_w = 0;
	m_screen_h = 0;

	SDL_GL_DeleteContext(m_glcontext);
}



Ref<Texture> ZED::RendererOpenGL::RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const
{
	auto temp = RendererSDL2::RenderFont(Size, Text, Textcolor);
	if (!temp)
		return nullptr;
	TextureOpenGL* ret = new TextureOpenGL(*(TextureSDL2*)temp.data);
	return ret;
}



Image ZED::RendererOpenGL::TakeScreenshot() const
{
	//static thread_local int index = 0;
	static int index = 0;
	int nextIndex = 0;//pbo index used for next frame


	//increment current index first then get the next index
	//"index" is used to read pixels from a framebuffer to a PBO
	//"nextIndex" is used to process pixels in the other PBO
	index = (index + 1) % 2;
	nextIndex = (index + 1) % 2;

	//set the framebuffer to read
	glReadBuffer(GL_FRONT);

	const auto& ext = OpenGLExtensions::GetInstance();
	if (ext.IsSupported("GL_ARB_pixel_buffer_object"))//with PBOs
	{
		//copy pixels from framebuffer to PBO
		//Use offset instead of ponter.
		//OpenGL should perform asynch DMA transfer, so glReadPixels() will return immediately.
		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO[index]);
		//glReadPixels(0, 0, m_screen_w, m_screen_h, GL_BGR, GL_UNSIGNED_BYTE, 0);
		glReadPixels(0, 0, m_screen_w, m_screen_h, GL_RGBA, GL_UNSIGNED_BYTE, 0);


		//map the PBO that contain framebuffer pixels before processing it
		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO[nextIndex]);
		const GLubyte* src = (GLubyte*)OpenGLExtensions::glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		if (!src)
		{
			OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
			return Image();
		}

		auto data = Blob(src, m_screen_w * m_screen_h * 4);

		OpenGLExtensions::glUnmapBuffer(GL_PIXEL_PACK_BUFFER);//release pointer to the mapped buffer

		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		return Image(std::move(data), m_screen_w, m_screen_h, ColorType::R8G8B8A8);
	}



	else//without PBOs
	{
		Blob data(m_screen_w * m_screen_h * 4);
		//glReadPixels(0, 0, m_screen_w, m_screen_h, GL_BGR, GL_UNSIGNED_BYTE, data);
		glReadPixels(0, 0, m_screen_w, m_screen_h, GL_RGBA, GL_UNSIGNED_BYTE, data);
		return Image(std::move(data), m_screen_w, m_screen_h, ColorType::R8G8B8A8);
		//Image.ConvertTo(ColorType::R8G8B8);
		//Image.Flip();
		//Image.Save(Filename);
	}
}



void ZED::RendererOpenGL::TakeScreenshotPNG(const char* Filename) const
{
	PNG img = PNG(TakeScreenshot());
	if (img)
	{
		img.ConvertTo(ColorType::R8G8B8);
		img.Flip();
		img.Save(Filename);
	}
}



Ref<Shader> ZED::RendererOpenGL::CompileShader(const char* VertexShaderSource, const char* PixelShaderSource) const
{
	auto& ext = OpenGLExtensions::GetInstance();
	
	if (!OpenGLExtensions::glCreateProgram)
		return nullptr;

	GLuint ProgramID = OpenGLExtensions::glCreateProgram();


	//Create vertex shader
	GLuint vertexShader = OpenGLExtensions::glCreateShader(GL_VERTEX_SHADER);

	//Set fragment source
	//const GLchar* VS_sources[] = { "#version 140\n#extension GL_ARB_explicit_attrib_location : enable\n#extension GL_ARB_shading_language_420pack : enable\n#define VERTEX", (GLchar*)VertexShaderSource };
	const GLchar* VS_sources[] = { "#version 130\n#define VERTEX", (GLchar*)VertexShaderSource };
	OpenGLExtensions::glShaderSource(vertexShader, 2, VS_sources, NULL);

	//Compile fragment source
	OpenGLExtensions::glCompileShader(vertexShader);

	//Check fragment shader for errors
	GLint fShaderCompiled = GL_FALSE;
	OpenGLExtensions::glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		Log::Error("Unable to compile vertex shader!");

		GLint infoLogLen = 0;
		OpenGLExtensions::glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLen);
		if (infoLogLen > 0)
		{
			GLchar* infoLog = new GLchar[infoLogLen];
			OpenGLExtensions::glGetShaderInfoLog(vertexShader, infoLogLen, NULL, infoLog);
			Log::Error("Could not compile vertex shader:\n" + std::string(infoLog));
			delete[] infoLog;
		}
		OpenGLExtensions::glDeleteShader(vertexShader);
		return nullptr;
	}

	//Attach fragment shader to program
	OpenGLExtensions::glAttachShader(ProgramID, vertexShader);


	//Create pixel shader
	GLuint fragmentShader = OpenGLExtensions::glCreateShader(GL_FRAGMENT_SHADER);

	//Set fragment source
	const GLchar* PS_sources[] = { "#version 130\n#define FRAGMENT", (GLchar*)PixelShaderSource };
	OpenGLExtensions::glShaderSource(fragmentShader, 2, PS_sources, NULL);

	//Compile fragment source
	OpenGLExtensions::glCompileShader(fragmentShader);

	//Check fragment shader for errors
	fShaderCompiled = GL_FALSE;
	OpenGLExtensions::glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		Log::Warn("Unable to compile pixel shader!");

		GLint infoLogLen = 0;
		OpenGLExtensions::glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLen);
		if (infoLogLen > 0)
		{
			GLchar* infoLog = new GLchar[infoLogLen];
			OpenGLExtensions::glGetShaderInfoLog(fragmentShader, infoLogLen, NULL, infoLog);
			Log::Warn("Could not compile pixel shader:\n" + std::string(infoLog));
			delete[] infoLog;
		}
		OpenGLExtensions::glDeleteShader(fragmentShader);
		return nullptr;
	}

	//Attach fragment shader to program
	OpenGLExtensions::glAttachShader(ProgramID, fragmentShader);

	//Link program
	OpenGLExtensions::glLinkProgram(ProgramID);

	//Check for errors
	GLint programSuccess = GL_FALSE;
	OpenGLExtensions::glGetProgramiv(ProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		Log::Warn("Error linking program!");

		GLint infoLogLen = 0;
		OpenGLExtensions::glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &infoLogLen);

		//The maxLength includes the NULL character
		GLchar* infoLog = new GLchar[infoLogLen];
		OpenGLExtensions::glGetProgramInfoLog(ProgramID, infoLogLen, &infoLogLen, &infoLog[0]);
		Log::Warn("Could not link shaders:\n" + std::string(infoLog));
		delete[] infoLog;

		//The program is useless now. So delete it
		OpenGLExtensions::glDeleteProgram(ProgramID);
		return nullptr;
	}

	return new ShaderOpenGL(ProgramID);
}



void ZED::RendererOpenGL::SetShader(const ZED::Shader& NewShader) const
{
	NewShader.SetShader();
	m_CurrentShader = &NewShader;
}



void ZED::RendererOpenGL::FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a) const
{
	m_DrawCalls++;

	glPushMatrix();

	OpenGLExtensions::glUseProgram(0);

	glColor4ub(r, g, b, a);
	glBindTexture(GL_TEXTURE_2D, 0);

	glTranslatef((float)dstrect.x, (float)dstrect.y, 0.0f);

	glBegin(GL_QUADS);

	glVertex3f(0.0f, 0.0f, 0.0f);		   //Top Left
	glVertex3f((float)dstrect.w, 0.0f, 0.0f);	   //Top Right
	glVertex3f((float)dstrect.w, (float)dstrect.h, 0.0f);//Bottom Right
	glVertex3f(0.0f, (float)dstrect.h, 0.0f);	   //Bottom Left
	glEnd();

	glPopMatrix();

	if (m_CurrentShader)
		SetShader(*m_CurrentShader);
}



void ZED::RendererOpenGL::Render(const Texture& tex, int x, int y, bool render_hit) const
{
	m_DrawCalls++;

	WorldToScreen(x, y, x, y);
	float w = tex.GetWidth()  * m_zoom;
	float h = tex.GetHeight() * m_zoom;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

	if (m_CurrentShader)
		m_CurrentShader->SetUniform(m_CurrentShader->GetUniformLocation("TextureSize"), (float)tex.GetWidth(), (float)tex.GetHeight());

	glPushMatrix();

	glTranslatef((float)x, (float)y, 0.0f);

	//scale
	glScalef(tex.m_sizeX, tex.m_sizeY, 1.0);

	//alpha
	glColor4ub(255, 255, 255, tex.m_alpha);

	tex.BindTexture();

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, h, 0.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, 0.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad

	glEnd();

	glPopMatrix();
}



void ZED::RendererOpenGL::RenderPartial(const Texture& tex, int x, int y, int width, int height, bool render_hit) const
{
	m_DrawCalls++;

	/*if (!m_surface)
		if (!ResourceManager::GetInstance().RequestAsset(this))
			return;*/


	WorldToScreen(x, y, x, y);
	float w = width  * m_zoom;
	float h = height * m_zoom;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

	if (m_CurrentShader)
		m_CurrentShader->SetUniform(m_CurrentShader->GetUniformLocation("TextureSize"), (float)tex.GetWidth(), (float)tex.GetHeight());

	glPushMatrix();

	glTranslatef((float)x, (float)y, 0.0f);

	//scale
	glScalef(tex.m_sizeX, tex.m_sizeY, 1.0);

	//alpha
	glColor4ub(255, 255, 255, tex.m_alpha);

	tex.BindTexture();

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, h / (float)tex.GetHeight()); glVertex3f(0.0f, h, 0.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f((float)w / (float)tex.GetWidth(), (float)h / (float)tex.GetHeight()); glVertex3f(w, h, 0.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f((float)w / (float)tex.GetWidth(), 0.0f); glVertex3f(w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad

	glEnd();

	glPopMatrix();
}



void ZED::RendererOpenGL::RenderRotated(const Texture& tex, int x, int y, bool render_hit) const
{
	m_DrawCalls++;

	WorldToScreen(x, y, x, y);
	float w = tex.GetWidth() * m_zoom;
	float h = tex.GetHeight() * m_zoom;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

	if (m_CurrentShader)
		m_CurrentShader->SetUniform(m_CurrentShader->GetUniformLocation("TextureSize"), (float)tex.GetWidth(), (float)tex.GetHeight());

	glPushMatrix();

	glTranslatef((float)x, (float)y, 0.0f);

	//scale
	glScalef(tex.m_sizeX, tex.m_sizeY, 1.0);

	//rotate about center
	glTranslatef((float)(tex.GetWidth() >> 1), (float)(tex.GetHeight() >> 1), 0.0f);
	glRotatef(-tex.m_angle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-(float)(tex.GetWidth() >> 1), -(float)(tex.GetHeight() >> 1), 0.0f);

	//alpha
	glColor4ub(255, 255, 255, tex.m_alpha);

	tex.BindTexture();

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, h, 0.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, 0.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad

	glEnd();

	glPopMatrix();
}



void ZED::RendererOpenGL::RenderTransformed(const Texture& tex, int x, int y) const
{
	//ZED::Log::Debug("Rendering " + std::string(tex.m_image_filename) + " to " + std::to_string(x) + "," + std::to_string(y));

	m_DrawCalls++;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

	glPushMatrix();

	glTranslatef((float)x, (float)y, 0.0f);

	//scale about the center
	//glTranslatef((m_surface->w >> 1), (m_surface->h >> 1), 0.0f);
	glScalef(tex.m_sizeX / (float)tex.m_quality, tex.m_sizeY / (float)tex.m_quality, 1.0f);
	//glTranslatef(-(m_surface->w >> 1), -(m_surface->h >> 1), 0.0f);

	//alpha
	glColor4ub(255, 255, 255, tex.m_alpha);

	tex.BindTexture();

	glBegin(GL_QUADS);


	//if (!srcrect)
	{
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, (float)tex.GetHeight(), 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f((float)tex.GetWidth(), (float)tex.GetHeight(), 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f((float)tex.GetWidth(), 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}
	/*else
	{
		if (srcrect->x + srcrect->w > m_surface->w) srcrect->w = m_surface->w - srcrect->x;
		if (srcrect->y + srcrect->h > m_surface->h) srcrect->h = m_surface->h - srcrect->y;

		GLfloat tex_x = (1.0 * srcrect->x) / m_surface->w;
		GLfloat tex_x2 = (1.0 * srcrect->x + srcrect->w) / m_surface->w;
		GLfloat tex_y = (1.0 * srcrect->y) / m_surface->h;
		GLfloat tex_y2 = (1.0 * srcrect->y + srcrect->h) / m_surface->h;

		glTexCoord2f(tex_x, tex_y2); glVertex3f(0.0f, srcrect->h, 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(tex_x2, tex_y2); glVertex3f(srcrect->w, srcrect->h, 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(tex_x2, tex_y); glVertex3f(srcrect->w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(tex_x, tex_y); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}*/

	glEnd();

	glPopMatrix();
}



void ZED::RendererOpenGL::RenderTransformedPartial(const Texture& tex, int x, int y, int w, int h) const
{
	m_DrawCalls++;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

	glPushMatrix();

	glTranslatef((float)x, (float)y, 0.0f);

	//scale about the center
	glScalef(tex.m_sizeX / (float)tex.m_quality, tex.m_sizeY / (float)tex.m_quality, 1.0f);

	//alpha
	glColor4ub(255, 255, 255, tex.m_alpha);

	tex.BindTexture();

	glBegin(GL_QUADS);


	//if (!srcrect)
	{
		glTexCoord2f(0.0f, (float)h / (float)tex.GetHeight()); glVertex3f(0.0f, (float)h, 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((float)w / (float)tex.GetWidth(), (float)h / (float)tex.GetHeight()); glVertex3f((float)w, (float)h, 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((float)w / (float)tex.GetWidth(), 0.0f); glVertex3f((float)w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}
	/*else
	{
		if (srcrect->x + srcrect->w > m_surface->w) srcrect->w = m_surface->w - srcrect->x;
		if (srcrect->y + srcrect->h > m_surface->h) srcrect->h = m_surface->h - srcrect->y;

		GLfloat tex_x = (1.0 * srcrect->x) / m_surface->w;
		GLfloat tex_x2 = (1.0 * srcrect->x + srcrect->w) / m_surface->w;
		GLfloat tex_y = (1.0 * srcrect->y) / m_surface->h;
		GLfloat tex_y2 = (1.0 * srcrect->y + srcrect->h) / m_surface->h;

		glTexCoord2f(tex_x, tex_y2); glVertex3f(0.0f, srcrect->h, 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(tex_x2, tex_y2); glVertex3f(srcrect->w, srcrect->h, 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(tex_x2, tex_y); glVertex3f(srcrect->w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(tex_x, tex_y); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}*/

	glEnd();

	glPopMatrix();
}



void ZED::RendererOpenGL::RenderTransformedRotated(const Texture& tex, int x, int y) const
{
	m_DrawCalls++;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

	glPushMatrix();

	glTranslatef((float)x, (float)y, 0.0f);

	//scale about the center
	//glTranslatef((m_surface->w >> 1), (m_surface->h >> 1), 0.0f);
	glScalef(tex.m_sizeX / (float)tex.m_quality, tex.m_sizeY / (float)tex.m_quality, 1.0f);
	//glTranslatef(-(m_surface->w >> 1), -(m_surface->h >> 1), 0.0f);

	//rotate about center
	glTranslatef((float)(tex.GetWidth() >> 1), (float)(tex.GetHeight() >> 1), 0.0f);
	glRotatef(-tex.m_angle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-(float)(tex.GetWidth() >> 1), -(float)(tex.GetHeight() >> 1), 0.0f);

	//alpha
	glColor4ub(255, 255, 255, tex.m_alpha);

	tex.BindTexture();

	glBegin(GL_QUADS);


	//if (!srcrect)
	{
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, (float)tex.GetHeight(), 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f((float)tex.GetWidth(), (float)tex.GetHeight(), 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f((float)tex.GetWidth(), 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}
	/*else
	{
		if (srcrect->x + srcrect->w > m_surface->w) srcrect->w = m_surface->w - srcrect->x;
		if (srcrect->y + srcrect->h > m_surface->h) srcrect->h = m_surface->h - srcrect->y;

		GLfloat tex_x = (1.0 * srcrect->x) / m_surface->w;
		GLfloat tex_x2 = (1.0 * srcrect->x + srcrect->w) / m_surface->w;
		GLfloat tex_y = (1.0 * srcrect->y) / m_surface->h;
		GLfloat tex_y2 = (1.0 * srcrect->y + srcrect->h) / m_surface->h;

		glTexCoord2f(tex_x, tex_y2); glVertex3f(0.0f, srcrect->h, 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(tex_x2, tex_y2); glVertex3f(srcrect->w, srcrect->h, 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(tex_x2, tex_y); glVertex3f(srcrect->w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(tex_x, tex_y); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}*/

	glEnd();

	glPopMatrix();
}



ZED::TextureOpenGL::TextureOpenGL(const std::string& Filename) : TextureSDL2(Filename)
{
	if (m_surface)
	{
		m_Width  = m_surface->w;
		m_Height = m_surface->h;
	}
}



ZED::TextureOpenGL::TextureOpenGL(const TextureSDL2& Surface) : ZED::TextureSDL2()
{
	Surface.CloneTo(*this);
	if (m_surface)
	{
		m_Width  = m_surface->w;
		m_Height = m_surface->h;
	}
}



/*ZED::TextureOpenGL::TextureOpenGL(const ZED::TextureOpenGL& Original)
{
	m_surface = Original.m_surface;
	m_gl_texture = Original.m_gl_texture;

	Original.m_surface = nullptr;
	Original.m_gl_texture = 0;
}*/



void ZED::TextureOpenGL::Unload()
{
	if (IsTextureLoaded())
	{
		glDeleteTextures(1, &m_gl_texture);
		m_gl_texture = 0;
	}

	TextureSDL2::Unload();
}



void ZED::TextureOpenGL::BindTexture(unsigned int Slot) const
{
	if (Slot == 0)
		glBindTexture(GL_TEXTURE_2D, m_gl_texture);
	else
	{
		OpenGLExtensions::glActiveTexture(GL_TEXTURE0 + Slot);//Texture unit slot
		glBindTexture(GL_TEXTURE_2D, m_gl_texture);
		OpenGLExtensions::glActiveTexture(GL_TEXTURE0);//Texture unit 0
	}
}



bool TextureOpenGL::LoadTexture() const
{
	//delete texture if it is already loaded
	if (IsTextureLoaded())
		glDeleteTextures(1, &m_gl_texture);

	if (!m_surface)
	{
		ZED::Log::Error("Could not create texture: surface data is invalid");
		return false;
	}

	GLint nOfColors = m_surface->format->BytesPerPixel;
	GLenum texture_format;
	GLint internal_format;
	switch (nOfColors)
	{
	case 4:
		internal_format = GL_RGBA8;
		//if (m_surface->format->Rmask == 0x000000ff)
			texture_format = GL_RGBA;
		//else
			//texture_format = GL_BGRA;
		break;
	case 3:
		internal_format = GL_RGB8;
		//if (m_surface->format->Rmask == 0x000000ff)
		texture_format = GL_RGB;
		//else
			//texture_format = GL_BGR;
		break;
	default:
		Log::Warn("Not a proper image format");
		return false;
		break;
	}

	// Have OpenGL generate a texture object handle for us
	glGenTextures(1, &m_gl_texture);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, m_gl_texture);

	/*if (!OverwriteDefault)//Take default?
		LinearInterpolation = Options::IsBilinearFilteringEnabled();

	//Set the texture's stretching properties
	if (LinearInterpolation)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/*}

	if (!Wrapping)*/
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_surface->w, m_surface->h, 0, texture_format, GL_UNSIGNED_BYTE, m_surface->pixels);

	//glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	ZED::Log::Debug("Texture id="+std::to_string(m_gl_texture)+" created with size"+std::to_string(m_surface->w)+"x"+ std::to_string(m_surface->h));

	//(const_cast<TextureOpenGL*>(this))->TextureSDL2::Unload();

	return true;
}