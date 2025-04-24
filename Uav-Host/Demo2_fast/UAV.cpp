#include <gst/gst.h>
#include <iostream>
int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);
    const char *filename = "sintel_trailer-480p.webm";
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        std::cerr << "Lỗi: Không tìm thấy file video: " << filename << std::endl;
        return -1;
    }
    fclose(file);
    // Tạo pipeline
    GstElement *pipeline = gst_pipeline_new("uav-pipeline");
    GstElement *source = gst_element_factory_make("filesrc", "file-source");
    GstElement *decode = gst_element_factory_make("decodebin", "decoder");
    GstElement *queue1 = gst_element_factory_make("queue", "queue1");
    GstElement *convert = gst_element_factory_make("videoconvert", "converter");
    GstElement *encoder = gst_element_factory_make("x264enc", "encoder");
    GstElement *payloader = gst_element_factory_make("rtph264pay", "payloader");
    GstElement *sink = gst_element_factory_make("udpsink", "udp-sink");
    if (!pipeline || !source || !decode || !queue1 || !convert || !encoder || !payloader || !sink)
    {
        std::cerr << "Lỗi: Không thể tạo phần tử trong pipeline." << std::endl;
        return -1;
    }
    // Cấu hình
    g_object_set(source, "location", filename, nullptr);
    g_object_set(encoder, "tune", "zerolatency", "bitrate", 512, "speed-preset", "ultrafast", nullptr);
    g_object_set(payloader, "config-interval", 1, "pt", 96, nullptr);
    g_object_set(sink, "host", "172.19.107.255", "port", 5000, "sync", FALSE, nullptr); // sync = false để truyền ngay
    // Ghép pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, decode, queue1, convert, encoder, payloader, sink, nullptr);
    if (!gst_element_link(source, decode))
    {
        std::cerr << "Lỗi: Không thể nối source với decodebin." << std::endl;
        return -1;
    }
    // Xử lý pad động của decodebin
    g_signal_connect(decode, "pad-added", G_CALLBACK(+[](GstElement *src, GstPad *pad, gpointer data)
                                                     {
                                                         GstElement *queue1 = static_cast<GstElement *>(data);
                                                         GstPad *sinkpad = gst_element_get_static_pad(queue1, "sink");
                                                         if (!gst_pad_is_linked(sinkpad))
                                                         {
                                                             if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
                                                             {
                                                                 g_printerr("Không thể nối decode -> queue\n");
                                                             }
                                                         }
                                                         gst_object_unref(sinkpad);
                                                     }),
                     queue1);
    if (!gst_element_link_many(queue1, convert, encoder, payloader, sink, nullptr))
    {
        std::cerr << "Lỗi: Không thể nối các phần còn lại." << std::endl;
        return -1;
    }
    // Chạy pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        std::cerr << "Lỗi: Không thể đưa pipeline vào trạng thái PLAYING." << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }
    std::cout << "Đang mô phỏng UAV gửi video (tối ưu)..." << std::endl;
    // Main loop
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    // Dọn dẹp
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);
    return 0;
}