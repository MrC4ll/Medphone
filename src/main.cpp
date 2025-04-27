// External Libs
#include <gtk/gtk.h>
#include <cairo.h>

//Internal Libs
#include "audio_processing.h" //FFT & ESP32 thread manager

// GUI state
typedef struct {
    GtkWidget *listbox;
    GtkWidget *drawing_area;
} AppData;

// Callback to update the match list (thread-safe via GLib)
static void update_match_list(gpointer user_data) {
    AppData *app = (AppData*)user_data;
    arma::uvec *top5 = (arma::uvec*)audio_get_top5_matches();  // Implement in audio_processing.c

    // Clear existing items
    GtkListBox *list = GTK_LIST_BOX(app->listbox);
    while (GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(list))) {
        gtk_list_box_remove(list, child);
    }

    // Add new matches
    for (int i = 0; i < 5; i++) {
        char label[64];
        snprintf(label, sizeof(label), "Match %d: Track ID %d", i+1, (*top5)[i]);
        GtkWidget *row = gtk_label_new(label);
        gtk_list_box_append(list, row);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppData *app_data = g_new0(AppData, 1);

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "ESP32 Spectrogram (Bluetooth)");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // Spectrogram drawing area
    app_data->drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_data->drawing_area), 
                                  draw_spectrogram, NULL, NULL);
    gtk_box_append(GTK_BOX(vbox), app_data->drawing_area);

    // Match list
    app_data->listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(app_data->listbox), GTK_SELECTION_NONE);
    gtk_box_append(GTK_BOX(vbox), app_data->listbox);

    // Start threads and UI updates
    start_audio_processing_threads();
    g_timeout_add(16, update_fft_data, app_data->drawing_area);
    g_timeout_add(1000, update_match_list, app_data);  // Update matches every 1s

    gtk_widget_show(window);
}