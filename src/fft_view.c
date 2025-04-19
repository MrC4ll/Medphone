#include "fft_view.h"
#include <cairo.h>
#include <math.h>

struct _FFTView {
    GtkWidget parent;
    float *fft_data;
    size_t fft_size;
    int sample_rate;
};

G_DEFINE_TYPE(FFTView, fft_view, GTK_TYPE_WIDGET)

static void fft_view_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
    FFTView *self = FFT_VIEW(widget);
    
    int width = gtk_widget_get_width(widget);
    int height = gtk_widget_get_height(widget);
    
    cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &GRAPHENE_RECT_INIT(0, 0, width, height));
    
    if (self->fft_data && self->fft_size > 0) {
        cairo_set_source_rgb(cr, 1.0, 0.5, 0);
        cairo_set_line_width(cr, 1.0);
        
        float x_scale = (float)width / (self->fft_size / 2);
        float y_scale = height / 20.0; // Adjust based on expected FFT magnitude
        
        // Only plot first half (real FFT output)
        for (size_t i = 0; i < self->fft_size / 2; i++) {
            float magnitude = 10 * log10f(self->fft_data[i] + 1e-6); // dB scale
            float bar_height = fminf(magnitude * y_scale, height);
            
            cairo_rectangle(cr, i * x_scale, height - bar_height, 
                          x_scale, bar_height);
            cairo_fill(cr);
        }
    }
    
    cairo_destroy(cr);
}
