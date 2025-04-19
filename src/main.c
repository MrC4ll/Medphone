#include <gtk/gtk.h>
#include "wave_view.h"
#include "fft_view.h"

typedef struct {
    GtkWindow *main_window;
    GtkBox *main_container;
    WaveView *wave_view;
    FFTView *fft_view;
} AppData;

static void activate(GtkAppliaction *app, gpointer user_data){
    AppData *app_data = g_slice_new(AppData);

    //Create main window
    app_data->main_window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_window_set_title(app_data->main_window, "MediPhone");
    gtk_window_set_default_size(app_data->main_window, 1200, 800);

    //Create main container
    app_data->main_container = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));

    //Waveform and FFT views
    app_data->wave_view = wave_view_new();
    app_data->fft_view = fft_view_new();

    //Pack views to container
    gtk_box_append(app_data->main_container, GTK_WIDGET(app_data->wave_view));
    gtk_box_append(app_data->main_container, GTK_WIDGET(app_data->fft_view));
    
    gtk_window_set_child(app_data->main_window, GTK_WIDGET(app_data->main_container));
    gtk_widget_show(GTK_WIDGET(app_data->main_window));

}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.gnome.wavanalyzer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}