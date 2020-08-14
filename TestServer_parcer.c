#include <gst/gst.h>

int
main(int argc, char* argv[])
{
    GstElement* pipeline;
    GstElement* filesrc;
    GstMessage* msg;
    GstBus* bus;
    GError* error = NULL;

    gst_init(&argc, &argv);

    /*if (argc != 2) {
        g_print("usage: %s <filename>\n", argv[0]);
        return -1;
    }*/

    pipeline = gst_parse_launch("videotestsrc ! video/x-raw,width=320,height=240 ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=169.254.34.88 port=10000", &error);
    if (!pipeline) {
        g_print("Parse error: %s\n", error->message);
        exit(1);
    }

    /*filesrc = gst_bin_get_by_name(GST_BIN(pipeline), "my_filesrc");
    g_object_set(filesrc, "location", argv[1], NULL);
    g_object_unref(filesrc);*/

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);

    /* wait until we either get an EOS or an ERROR message. Note that in a real
     * program you would probably not use gst_bus_poll(), but rather set up an
     * async signal watch on the bus and run a main loop and connect to the
     * bus's signals to catch certain messages or all messages */
    msg = gst_bus_poll(bus, GST_MESSAGE_EOS | GST_MESSAGE_ERROR, -1);

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS: {
        g_print("EOS\n");
        break;
    }
    case GST_MESSAGE_ERROR: {
        GError* err = NULL; /* error to show to users                 */
        gchar* dbg = NULL;  /* additional debug string for developers */

        gst_message_parse_error(msg, &err, &dbg);
        if (err) {
            g_printerr("ERROR: %s\n", err->message);
            g_error_free(err);
        }
        if (dbg) {
            g_printerr("[Debug details: %s]\n", dbg);
            g_free(dbg);
        }
    }
    default:
        g_printerr("Unexpected message of type %d", GST_MESSAGE_TYPE(msg));
        break;
    }
    gst_message_unref(msg);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);

    return 0;
}
