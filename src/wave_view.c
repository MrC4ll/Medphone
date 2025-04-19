#include "wave_view.h"
#include <cairo.h>

struct _WaveView {
    GtkWidget parent;
    float *samples;
    size_t sample_count;
    int sample_rate;
};

G_DEFINE_TYPE(WaveView, wave_view, GTK_TYPE_WIDGET)

static void wave_view_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
    WaveView *self = WAVE_VIEW(widget);
    
    // Get widget dimensions
    int width = gtk_widget_get_width(widget);
    int height = gtk_widget_get_height(widget);
    
    // Create Cairo context
    cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &GRAPHENE_RECT_INIT(0, 0, width, height));
    
    // Draw waveform
    if (self->samples && self->sample_count > 0) {
        cairo_set_source_rgb(cr, 0, 0.5, 1.0);
        cairo_set_line_width(cr, 1.0);
        
        // Scale samples to view height
        float y_scale = height / 2.0;
        float x_scale = (float)width / self->sample_count;
        
        cairo_move_to(cr, 0, height/2 - self->samples[0] * y_scale);
        for (size_t i = 1; i < self->sample_count; i++) {
            cairo_line_to(cr, i * x_scale, height/2 - self->samples[i] * y_scale);
        }
        cairo_stroke(cr);
    }
    
    cairo_destroy(cr);
}