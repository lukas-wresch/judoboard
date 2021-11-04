#include "../include/renderer_opengles1_1.h"
//#include "../include/opengl_extensions.h"
#include "../include/log.h"
#include "../include/image.h"
#include <EGL/egl.h>
#include <GL/gl.h>
#ifndef _WIN32
//#include <bcm_host.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>//Include last because of macros
#endif

#pragma comment(lib, "libEGL.lib")


using namespace ZED;



#ifdef _WIN32
bool RendererOpenGLES1_1::Init(HWND Hwnd)
#else
bool RendererOpenGLES1_1::Init(GtkWindow* Window)
#endif
{
//#ifdef _WIN32
	//if (!RendererSDL2::Init(Hwnd))
//#else
	//if (!RendererSDL2::Init(Window))
//#endif
		//return false;

#ifdef LINUX
	//bcm_host_init();
#endif

	m_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	if (!eglInitialize(m_Display, nullptr, nullptr))
	{
		Log::Error("Could not initialize OpenGL ES");
		return false;
	}

	int num_configs;
	eglGetConfigs(m_Display, nullptr, 0, &num_configs);
	Log::Info("EGL has " + std::to_string(num_configs) + " many configurations");

	EGLConfig* configs = new EGLConfig[num_configs];
	eglGetConfigs(m_Display, configs, num_configs, &num_configs);

	for (int i = 0; i < num_configs; i++)
	{
		Log::Info("Configuration " + std::to_string(i) + ":");
		
		int value;
		eglGetConfigAttrib(m_Display, configs[i], EGL_RED_SIZE, &value);
		Log::Info("Red Size " + std::to_string(value));

		eglGetConfigAttrib(m_Display, configs[i], EGL_GREEN_SIZE, &value);
		Log::Info("Green Size " + std::to_string(value));

		eglGetConfigAttrib(m_Display, configs[i], EGL_BLUE_SIZE, &value);
		Log::Info("Blue Size " + std::to_string(value));

		eglGetConfigAttrib(m_Display, configs[i], EGL_BUFFER_SIZE, &value);
		Log::Info("Buffer Size " + std::to_string(value));

		eglGetConfigAttrib(m_Display, configs[i], EGL_BIND_TO_TEXTURE_RGB, &value);
		Log::Info(std::string("RGB Textures: ") + (value ? "Yes" : "No"));

		eglGetConfigAttrib(m_Display, configs[i], EGL_BIND_TO_TEXTURE_RGBA, &value);
		Log::Info(std::string("RGBA Textures: ") + (value ? "Yes" : "No"));
	}

	delete[] configs;



	const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	const EGLint context_attributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
	};


	//get an appropriate EGL configuration - just use the first available
	EGLConfig config;
	if (!eglChooseConfig(m_Display, attribute_list, &config, 1, &num_configs))
	{
		Log::Error("Could not find appropriate configuration");
		return false;
	}

	//Choose the OpenGL ES API
	if (!eglBindAPI(EGL_OPENGL_ES_API))
	{
		Log::Error("Could not bind API");
		return false;
	}

#ifdef _WIN32
	m_Surface = eglCreateWindowSurface(m_Display, config, Hwnd, nullptr);
#else
	void* gdk_window = gtk_widget_get_window(GTK_WIDGET(Window));
	m_Surface = eglCreateWindowSurface(m_Display, config, gdk_x11_window_get_xid(gdk_window), nullptr);
#endif

	if (!m_Surface)
	{
		Log::Error("Could not create surface for window");
		return false;
	}

	//Create an EGL rendering context
	m_glcontext = eglCreateContext(m_Display, config, EGL_NO_CONTEXT, context_attributes);
	if (!m_glcontext)
	{
		Log::Error("Failed to create EGL context");
		return false;
	}

	eglMakeCurrent(m_Display, m_Surface, m_Surface, m_glcontext);


	//SDL_GL_SetSwapInterval(0);//Disable Vsync

	glEnable(GL_TEXTURE_2D);//Enable Texture Mapping
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);//Clear The Background Color To Black 
	glClearDepth(0.0f);//Disables(?) Clearing Of The Depth Buffer
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

	ZED::Log::Info("OpenGL ES 1.1 renderer initialized");
	return true;
}



void ZED::RendererOpenGLES1_1::ResetViewPort(unsigned int width, unsigned int height)
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

	/*auto& ext = OpenGLExtensions::GetInstance();
	const bool pboSupported = ext.IsSupported("GL_ARB_pixel_buffer_object") && ext.IsSupported("GL_ARB_vertex_buffer_object");

	if (pboSupported)
	{
		if (m_PBO[0])
			OpenGLExtensions::glDeleteBuffers(2, m_PBO);

		const unsigned int data_size = (width+1) * (height+1) * 3;//The +1 helps with crashes due to data alignment (only in release builds)
		OpenGLExtensions::glGenBuffers(2, m_PBO);
		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO[0]);
		OpenGLExtensions::glBufferData(GL_PIXEL_PACK_BUFFER, data_size, 0, GL_STREAM_READ);
		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO[1]);
		OpenGLExtensions::glBufferData(GL_PIXEL_PACK_BUFFER, data_size, 0, GL_STREAM_READ);

		OpenGLExtensions::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}*/

	ZED::Log::Debug("Screen resized to " + std::to_string(m_screen_w) + "x" + std::to_string(m_screen_h));
}



void ZED::RendererOpenGLES1_1::ClearDisplay() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}



void RendererOpenGLES1_1::SetClearColor(const ColorF& Color)
{
	glClearColor(Color.r, Color.g, Color.b, 0.0f);
}



void ZED::RendererOpenGLES1_1::UpdateDisplay() const
{
	eglSwapBuffers(m_Display, m_Surface);
}



void ZED::RendererOpenGLES1_1::Release()
{
	m_screen_w = 0;
	m_screen_h = 0;
}



/*Ref<Texture> RendererOpenGLES1_1::RenderFont(FontSize Size, const std::string& Text, ZED::Color Textcolor) const
{
	auto temp = RendererSDL2::RenderFont(Size, Text, Textcolor);
	TextureOpenGL* ret = new TextureOpenGL(*(TextureSDL2*)temp.data);
	return ret;
}*/



void ZED::RendererOpenGLES1_1::FillRect(const Rect& dstrect, unsigned char r, unsigned char g, unsigned char b, unsigned char a) const
{
	m_DrawCalls++;

	glPushMatrix();

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
}



/*void ZED::RendererOpenGL::Render(const Texture& tex, int x, int y, bool render_hit) const
{
	m_DrawCalls++;

	WorldToScreen(x, y, x, y);
	float w = tex.GetWidth()  * m_zoom;
	float h = tex.GetHeight() * m_zoom;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

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

	WorldToScreen(x, y, x, y);
	float w = width  * m_zoom;
	float h = height * m_zoom;

	if (!tex.IsTextureLoaded() && !tex.LoadTexture()) return;

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



void ZED::RendererOpenGLES1_1::RenderTransformed(const Texture& tex, int x, int y) const
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


	{
		glTexCoord2f(0.0f, (float)h / (float)tex.GetHeight()); glVertex3f(0.0f, (float)h, 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((float)w / (float)tex.GetWidth(), (float)h / (float)tex.GetHeight()); glVertex3f((float)w, (float)h, 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((float)w / (float)tex.GetWidth(), 0.0f); glVertex3f((float)w, 0.0f, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	}


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



	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, (float)tex.GetHeight(), 0.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f((float)tex.GetWidth(), (float)tex.GetHeight(), 0.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f((float)tex.GetWidth(), 0.0f, 0.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);	// Top Left Of The Texture and Quad
	

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
	m_Width  = m_surface->w;
	m_Height = m_surface->h;
}*/



/*TextureOpenGLES1_1::TextureOpenGLES1_1(const TextureOpenGLES1_1& Original)
{
	m_surface = Original.m_surface;
	m_gl_texture = Original.m_gl_texture;

	Original.m_surface = nullptr;
	Original.m_gl_texture = 0;
}*/



void ZED::TextureOpenGLES1_1::Unload()
{
	if (IsTextureLoaded())
	{
		glDeleteTextures(1, &m_gl_texture);
		m_gl_texture = 0;
	}
}



void ZED::TextureOpenGLES1_1::BindTexture(unsigned int Slot) const
{
	if (Slot == 0)
		glBindTexture(GL_TEXTURE_2D, m_gl_texture);
	else
	{
		//OpenGLExtensions::glActiveTexture(GL_TEXTURE0 + Slot);//Texture unit slot
		glBindTexture(GL_TEXTURE_2D, m_gl_texture);
		//OpenGLExtensions::glActiveTexture(GL_TEXTURE0);//Texture unit 0
	}
}



TextureOpenGLES1_1::TextureOpenGLES1_1(const Image& Image) : m_Width(Image.GetWidth()), m_Height(Image.GetHeight())
{
	//delete texture if it is already loaded
	//if (IsTextureLoaded())
		//glDeleteTextures(1, &m_gl_texture);

	if (!Image)
		ZED::Log::Error("Could not create texture: image data is invalid");

	GLenum texture_format;
	GLint internal_format;
	switch (Image.GetColorType())
	{
	case ColorType::R8G8B8A8:
		internal_format = GL_RGBA8;
		//if (m_surface->format->Rmask == 0x000000ff)
			texture_format = GL_RGBA;
		//else
			//texture_format = GL_BGRA;
		break;
	case ColorType::R8G8B8:
		internal_format = GL_RGB8;
		//if (m_surface->format->Rmask == 0x000000ff)
		texture_format = GL_RGB;
		//else
			//texture_format = GL_BGR;
		break;
	default:
		Log::Warn("Not a proper image format");
		break;
	}

	// Have OpenGL generate a texture object handle for us
	glGenTextures(1, &m_gl_texture);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, m_gl_texture);

	/*if (!OverwriteDefault)//Take default?
		LinearInterpolation = Options::IsBilinearFilteringEnabled();

	//Set the texture's stretching properties
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
		

	/*if (!Wrapping)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}*/

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_Width, m_Height, 0, texture_format, GL_UNSIGNED_BYTE, Image);

	//glGenerateMipmap(GL_TEXTURE_2D);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	ZED::Log::Debug("Texture id="+std::to_string(m_gl_texture)+" created with size"+std::to_string(m_Width)+"x"+ std::to_string(m_Height));

	//(const_cast<TextureOpenGL*>(this))->TextureSDL2::Unload();
}