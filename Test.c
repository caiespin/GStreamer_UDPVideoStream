#define HOST	"169.254.34.88" 
#define PORT 10000

#include <gst/gst.h>

int main(int argc, char* argv[]) {
	GstElement *pipeline, *source, *encoder, *RTPencoder, *sink;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;

	/* Initialize GStreamer */
	gst_init(&argc, &argv);

	/* Check input arguments */
	/*if (argc != 2) {
		g_printerr("Usage: %s <filename>\n", argv[0]);
		return -1;
	}*/

	/* Create the elements */
	source = gst_element_factory_make("videotestsrc", "source");
	//source = gst_element_factory_make("filesrc", "file-source");
	encoder = gst_element_factory_make("x264enc", "H264-enc");
	RTPencoder = gst_element_factory_make("rtph264pay", "RTP-enc");
	sink = gst_element_factory_make("udpsink", "sink");

	/* Create pipeline */
	pipeline = gst_pipeline_new("test-pipeline");

	if (!pipeline || !source || !encoder || !RTPencoder || !sink) {
		g_printerr("Not all elements could be created. Exiting.\n");
		return -1;
	}

	/* Set up the pipeline */
	/* we set the input filename to the source element */
	//g_object_set(G_OBJECT(source), "location", argv[1], NULL);
	//g_object_set(source, "pattern", 0, NULL);
	//g_object_set(G_OBJECT(encoder), "tune", 0x00000004, NULL);

	g_object_set(G_OBJECT(sink), "port", PORT, "host", HOST, NULL);

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(pipeline), source, encoder, RTPencoder, sink, NULL);
	if (gst_element_link(source, sink) != TRUE) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* Start playing */
	g_print("Now playing\n");
	ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* Wait until error or EOS */
	bus = gst_element_get_bus(pipeline);
	msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

	/* Parse message */
	if (msg != NULL) {
		GError* err;
		gchar* debug_info;

		switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_ERROR:
			gst_message_parse_error(msg, &err, &debug_info);
			g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
			g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
			g_clear_error(&err);
			g_free(debug_info);
			break;
		case GST_MESSAGE_EOS:
			g_print("End-Of-Stream reached.\n");
			break;
		default:
			/* We should not reach here because we only asked for ERRORs and EOS */
			g_printerr("Unexpected message received.\n");
			break;
		}
		gst_message_unref(msg);
	}

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	return 0;
}