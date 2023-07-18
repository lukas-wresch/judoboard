#include "window.h"
#include "ZED/include/log.h"
#include "app.h"
#include <gdk/gdkx.h>



using namespace Judoboard;



gboolean Judoboard::on_destroy(GtkWidget* widget, GdkEventAny* event)
{
	return true;
}



gboolean Judoboard::on_delete(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	Judoboard::Window* win = (Judoboard::Window*)data;
	gtk_widget_hide(GTK_WIDGET(win->m_Hwnd));
	gtk_widget_destroy(GTK_WIDGET(win->m_Hwnd));

	for (int i = 0; i < 100; i++)
	{
		win->HandleEvents();
		ZED::Core::Pause(10);
	}

	win->m_IsRunning = false;
	return true;
}



gboolean Judoboard::on_configure(GtkWidget* widget, GdkEventConfigure* event, gpointer data)
{
	Judoboard::Window* win = (Judoboard::Window*)data;
	win->GetRenderer().ResetViewPort(event->width, event->height);
	return false;
}



bool Judoboard::Window::OpenWindow()
{
	if (!IsDisplayConnected())
	{
		ZED::Log::Info("No display connected, simulating window");
		m_Renderer->Init(NULL);
		m_IsRunning = true;
		return true;
	}

	ZED::Log::Info("Starting GTK3");
	gtk_init(NULL, NULL);

	m_Hwnd = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);


	//gtk_widget_hide(GTK_WIDGET(m_Hwnd));


	gtk_window_set_default_size(GTK_WINDOW(m_Hwnd), 1024, 768);
	gtk_window_fullscreen(GTK_WINDOW(m_Hwnd));
	gtk_widget_show_all(GTK_WIDGET(m_Hwnd));


	ZED::Log::Info("Window created");

	//g_sciter_hwnd = SAPI()->SciterCreateWidget(NULL);//Create sciter widget
	//g_sciter_hwnd_backup = (GtkWidget*)g_object_ref(g_sciter_hwnd);//Create backup of sciter widget

	//SciterSetOption(g_sciter_hwnd, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);
	//SciterSetCallback(g_sciter_hwnd, SciterCallback, 0);
	//sciter::attach_dom_event_handler(g_sciter_hwnd, &g_ti_script);

	//SciterLoadFile(g_sciter_hwnd, WSTR("ui/index.html"));//Open URL
	//g_root = sciter::dom::element::root_element(g_sciter_hwnd);//Get DOM root
	//OutputDebugStringA("index.html opened\n");

	//gtk_container_add(GTK_CONTAINER(g_Hwnd), g_sciter_hwnd);

	//GtkWidget* gtk_da = gtk_drawing_area_new();//Create drawing area for SDL2
	//gtk_da_backup = (GtkWidget*)g_object_ref(gtk_da);
	//gtk_container_add(GTK_CONTAINER(m_Hwnd), gtk_da);



	//gtk_widget_set_events(g_Hwnd, GDK_CONFIGURE | GDK_STRUCTURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);
	//gtk_widget_add_events(GTK_WIDGET(g_Hwnd), GDK_CONFIGURE | GDK_STRUCTURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(GTK_WIDGET(m_Hwnd), GDK_CONFIGURE | GDK_STRUCTURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	//g_idle_add(&on_idle, 0);
	g_signal_connect(G_OBJECT(m_Hwnd), "destroy", G_CALLBACK(on_destroy), NULL);
	g_signal_connect(G_OBJECT(m_Hwnd), "delete-event", G_CALLBACK(on_delete), this);
	g_signal_connect(G_OBJECT(m_Hwnd), "configure-event", G_CALLBACK(on_configure), this);
	//g_signal_connect(G_OBJECT(m_Hwnd), "key_press_event", G_CALLBACK(on_keydown), NULL);
	//g_signal_connect(G_OBJECT(m_Hwnd), "key_release_event", G_CALLBACK(on_keyup), NULL);
	//g_signal_connect(G_OBJECT(g_Hwnd), "motion_notify_event", G_CALLBACK(on_motion), NULL);
	//g_signal_connect(G_OBJECT(m_Hwnd), "button_press_event", G_CALLBACK(on_button_press), NULL);
	//g_signal_connect(G_OBJECT(m_Hwnd), "button_release_event", G_CALLBACK(on_button_release), NULL);
	//g_signal_connect(G_OBJECT(m_Hwnd), "window-state-event", G_CALLBACK(on_window_state_change), NULL);


	gtk_main_iteration();


	if (!m_Renderer || !m_Renderer->Init(m_Hwnd))
		return false;


	//Remove cursor
	GdkCursor* cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
	GdkWindow* win = gtk_widget_get_window(GTK_WIDGET(m_Hwnd));
	gdk_window_set_cursor(win, cursor);


	m_IsRunning = true;

	return true;
}



bool Judoboard::Window::HandleEvents() const
{
	if (!m_IsRunning)
		return false;

	if (m_Hwnd && gtk_events_pending())
		gtk_main_iteration();

	return true;
}



bool Judoboard::Window::IsRunning() const
{
	return m_IsRunning;
}



void Judoboard::Window::CloseWindow()
{
	if (!m_Renderer) return;

	//m_Renderer->Lock();

	if (m_Hwnd)
	{
		gtk_widget_hide(GTK_WIDGET(m_Hwnd));
		gtk_widget_destroy(GTK_WIDGET(m_Hwnd));
	}

	for (int i = 0; i < 100; i++)
	{
		HandleEvents();
		ZED::Core::Pause(10);
	}

	m_IsRunning = false;

	//m_Renderer->Unlock();
}



void Judoboard::Window::Fullscreen(bool Enabled) const
{
	if (!m_Hwnd) return;

	if (Enabled)
		gtk_window_fullscreen(GTK_WINDOW(m_Hwnd));
	else
		gtk_window_unfullscreen(GTK_WINDOW(m_Hwnd));
}



unsigned int Judoboard::Window::GetDisplayWidth() const
{
	if (!IsDisplayConnected) return 0;

	GdkRectangle workarea = { 0 };
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	//gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	return workarea.width;
}



unsigned int Judoboard::Window::GetDisplayHeight() const
{
	if (!IsDisplayConnected) return 0;

	GdkRectangle workarea = { 0 };
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	//gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), &workarea);
	return workarea.height;
}



bool Judoboard::Window::IsDisplayConnected()
{
	return !Application::NoWindow;
}