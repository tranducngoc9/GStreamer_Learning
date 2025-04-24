#ifndef CUSTOMDATA_H
#define CUSTOMDATA_H
#include <gst/gst.h>
#include <QObject>
/* Structure to contain all our information, so we can pass it around */
typedef struct _CustomData
{
    GstElement playbin;
    / Our one and only pipeline * /
        gulong slider_update_signal_id; /* Signal ID for the slider update signal */
    GstState state;                     /* Current state of the pipeline */
    gint64 duration;                    /* Duration of the clip, in nanoseconds */
} CustomData;
/* This function is called when new metadata is discovered in the stream */
static void tags_cb(GstElement *playbin, gint stream, CustomData *data)
{
    /* We are possibly in a GStreamer working thread, so we notify the main
     * thread of this event through a message in the bus */
    gst_element_post_message(playbin,
                             gst_message_new_application(GST_OBJECT(playbin),
                                                         gst_structure_new_empty("tags-changed")));
}
/* This function is called when an error message is posted on the bus */
static void error_cb(GstBus *bus, GstMessage *msg, CustomData *data)
{
    GError *err;
    gchar *debug_info;
    /* Print error details on the screen */
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);
    /* Set the pipeline to READY (which stops playback) */
    gst_element_set_state(data->playbin, GST_STATE_READY);
}
/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb(GstBus *bus, GstMessage *msg, CustomData *data)
{
    g_print("End-Of-Stream reached.\n");
    gst_element_set_state(data->playbin, GST_STATE_READY);
}
/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
static void state_changed_cb(GstBus *bus, GstMessage *msg, CustomData *data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->playbin))
    {
        data->state = new_state;
        g_print("State set to %s\n", gst_element_state_get_name(new_state));
        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
        {
            /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
            //      refresh_ui (data);
        }
    }
}
/* This function is called when an "application" message is posted on the bus.
 * Here we retrieve the message posted by the tags_cb callback */
static void application_cb(GstBus *bus, GstMessage *msg, CustomData *data)
{
    if (g_strcmp0(gst_structure_get_name(gst_message_get_structure(msg)), "tags-changed") == 0)
    {
        /* If the message is the "tags-changed" (only one we are currently issuing), update
         * the stream info GUI */
        //    analyze_streams (data);
    }
}
class MyCppClass : public QObject
{
    Q_OBJECT
public:
    CustomData data;
    GstElement *gtkglsink, *videosink;
    GstStateChangeReturn ret;
    GstBus *bus;
    explicit MyCppClass(QObject *parent = nullptr) : QObject(parent)
    {
        data.duration = GST_CLOCK_TIME_NONE;
        /* Create the elements */
        data.playbin = gst_element_factory_make("playbin", "playbin");
        videosink = gst_element_factory_make("qtglsink", "qtglsink");
        /* Set the URI to play */
        g_object_set(data.playbin, "uri", "file:////home/qwerty/TEST_CODE/TestSocket/sintel_trailer-480p.webm", NULL);
        /* Set the video-sink  */
        g_object_set(data.playbin, "video-sink", videosink, NULL);
        /* Connect to interesting signals in playbin */
        g_signal_connect(G_OBJECT(data.playbin), "video-tags-changed", (GCallback)tags_cb, &data);
        g_signal_connect(G_OBJECT(data.playbin), "audio-tags-changed", (GCallback)tags_cb, &data);
        g_signal_connect(G_OBJECT(data.playbin), "text-tags-changed", (GCallback)tags_cb, &data);
        /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
        bus = gst_element_get_bus(data.playbin);
        gst_bus_add_signal_watch(bus);
        g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)error_cb, &data);
        g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback)eos_cb, &data);
        g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)state_changed_cb, &data);
        g_signal_connect(G_OBJECT(bus), "message::application", (GCallback)application_cb, &data);
        gst_object_unref(bus);
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.playbin), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline_debug1");
    }
public slots:
    void play_cb()
    {
        gst_element_set_state(data.playbin, GST_STATE_PLAYING);
    }
    void pause_cb()
    {
        gst_element_set_state(data.playbin, GST_STATE_PAUSED);
    }
    void stop_cb()
    {
        gst_element_set_state(data.playbin, GST_STATE_READY);
    }
    void seek_cb()
    {
        gst_element_seek_simple(data.playbin, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, (gint64)(20 * GST_SECOND));
    }
};
#endif // CUSTOMDATA_H