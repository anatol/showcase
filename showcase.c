#include <assert.h>
#include <dbus/dbus.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "config.h"

static struct {
    WebKitWebView *browser;
    unsigned int page_current;
} state;

static gboolean showcase_next_page(gpointer unused)
{
    struct showcase_page *page = &config.pages[state.page_current];
    int wait_time = page->wait_time;
    if (wait_time == 0)
        wait_time = config.wait_time;

    state.page_current++;
    if (state.page_current >= config.pages_num)
        state.page_current = 0;

    webkit_web_view_load_uri(state.browser, page->url);

    gdk_threads_add_timeout_seconds(wait_time, showcase_next_page, NULL);
    return FALSE;
}

static void powermanagement_inhibit()
{
    DBusError err;
    dbus_error_init(&err);

    DBusConnection *conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (!conn) {
        fprintf(stderr, "cannot connect to session bus: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.PowerManagement.Inhibit",
        "/org/freedesktop/PowerManagement/Inhibit",
        "org.freedesktop.PowerManagement.Inhibit.Inhibit",
        "Inhibit");
    if (!msg) {
        fprintf(stderr, "DBus inhibit message is not created\n");
        goto err_conn;
    }

    const char *app = "Showcase";
    const char *reason = "Showing pages";
    dbus_bool_t ret = dbus_message_append_args(msg,
        DBUS_TYPE_STRING, &app,
        DBUS_TYPE_STRING, &reason,
        DBUS_TYPE_INVALID);
    if (!ret)
        goto err_msg;

    dbus_connection_send(conn, msg, NULL);

err_msg:
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
err_conn:
    dbus_connection_close(conn);
    dbus_connection_unref(conn);
}

int main(int argc, char* argv[])
{
    config_parse();
    powermanagement_inhibit();

    gtk_init(&argc, &argv);

    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // TODO: RaspberryPi allows writing to framebuffer directly, maybe we can do the same here?

    // TODO: how to check if the app is running by X directly?
    GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(main_window));
    gint width = gdk_screen_get_width(screen);
    gint height = gdk_screen_get_height(screen);
    gtk_window_set_default_size(GTK_WINDOW(main_window), width, height);

    // This works in case of DE
    gtk_window_fullscreen(GTK_WINDOW(main_window));

    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(web_view));
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(web_view, "close", G_CALLBACK(gtk_widget_destroy), main_window);
    state.browser = web_view;

    showcase_next_page(NULL);

    gtk_widget_show_all(main_window);

    gtk_main();

    config_free();
    return 0;
}
