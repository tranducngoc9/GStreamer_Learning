#include <gst/gst.h>
#include <iostream>
int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);
    // Tạo phần tử
    GstElement *pipeline = gst_pipeline_new("host-pipeline");
    GstElement *source = gst_element_factory_make("udpsrc", "udp-source");
    GstElement *depay = gst_element_factory_make("rtph264depay", "depayloader");
    GstElement *queue1 = gst_element_factory_make("queue", "queue1");
    GstElement *decoder = gst_element_factory_make("avdec_h264", "decoder");
    GstElement *queue2 = gst_element_factory_make("queue", "queue2");
    GstElement *convert = gst_element_factory_make("videoconvert", "converter");
    GstElement *sink = gst_element_factory_make("autovideosink", "video-output"); // thử thay bằng glimagesink nếu dùng GUI
    if (!pipeline || !source || !depay || !queue1 || !decoder || !queue2 || !convert || !sink)
    {
        std::cerr << "Lỗi: Không thể tạo phần tử pipeline." << std::endl;
        return -1;
    }
    // Cấu hình udpsrc
    g_object_set(source, "port", 5000, nullptr);
    GstCaps *caps = gst_caps_from_string(
        "application/x-rtp, media=video, encoding-name=H264, payload=96");
    g_object_set(source, "caps", caps, nullptr);
    gst_caps_unref(caps);
    // Thêm các phần tử vào pipeline
    gst_bin_add_many(GST_BIN(pipeline),
                     source, depay, queue1, decoder, queue2, convert, sink, nullptr);
    // Nối các phần tử
    if (!gst_element_link_many(source, depay, queue1, decoder, queue2, convert, sink, nullptr))
    {
        std::cerr << "Lỗi: Không thể nối các phần tử pipeline." << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }
    // Đưa pipeline vào trạng thái PLAYING
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        std::cerr << "Lỗi: Không thể đưa pipeline vào trạng thái PLAYING." << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }
    std::cout << "HOST đang nhận và hiển thị video..." << std::endl;
    // Vòng lặp chính
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    std::cout << "HOST nhận xong video." << std::endl;
    // Dọn dẹp
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);
    return 0;
}