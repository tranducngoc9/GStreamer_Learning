#include <gst/gst.h>
#include <iostream>
int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);
    // Tên file video để mô phỏng (phải tồn tại)
    const char *filename = "sintel_trailer-480p.webm";
    // Kiểm tra file có tồn tại không
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        std::cerr << "Lỗi: Không tìm thấy file video: " << filename << std::endl;
        return -1;
    }
    fclose(file);
    // Tạo từng phần tử pipeline
    GstElement *pipeline = gst_pipeline_new("uav-pipeline");
    GstElement *source = gst_element_factory_make("filesrc", "file-source");
    GstElement *decode = gst_element_factory_make("decodebin", "decoder");
    GstElement *convert = gst_element_factory_make("videoconvert", "converter");
    GstElement *encoder = gst_element_factory_make("x264enc", "encoder");
    GstElement *payloader = gst_element_factory_make("rtph264pay", "payloader");
    GstElement *sink = gst_element_factory_make("udpsink", "udp-sink");
    if (!pipeline || !source || !decode || !convert || !encoder || !payloader || !sink)
    {
        std::cerr << "Lỗi: Không thể tạo một hoặc nhiều phần tử trong pipeline." << std::endl;
        return -1;
    }
    // Cấu hình các phần tử
    g_object_set(source, "location", filename, nullptr);
    g_object_set(encoder, "tune", "zerolatency", "bitrate", 2048, "speed-preset", "superfast", nullptr);
    g_object_set(payloader, "config-interval", 1, "pt", 96, nullptr);
    g_object_set(sink, "host", "172.29.115.192", "port", 5000, nullptr);
    // Thêm các phần tử vào pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, decode, convert, encoder, payloader, sink, nullptr);
    // Nối các phần tử có thể nối trước
    if (!gst_element_link(source, decode))
    {
        std::cerr << "Lỗi: Không thể nối source với decodebin." << std::endl;
        return -1;
    }
    // decodebin kết nối động, cần callback để nối tiếp
    g_signal_connect(decode, "pad-added", G_CALLBACK(+[](GstElement *src, GstPad *pad, gpointer data)
                                                     {
                                                         GstElement *convert = static_cast<GstElement *>(data);
                                                         GstPad *sinkpad = gst_element_get_static_pad(convert, "sink");
                                                         if (gst_pad_is_linked(sinkpad))
                                                         {
                                                             gst_object_unref(sinkpad);
                                                             return;
                                                         }
                                                         if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
                                                         {
                                                             g_printerr("Không thể nối pad decode -> convert\n");
                                                         }
                                                         gst_object_unref(sinkpad);
                                                     }),
                     convert);
    // Nối phần còn lại
    if (!gst_element_link_many(convert, encoder, payloader, sink, nullptr))
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
    std::cout << "Đang mô phỏng UAV gửi video..." << std::endl;
    // Main loop
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    // Dọn dẹp
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);
    return 0;
}