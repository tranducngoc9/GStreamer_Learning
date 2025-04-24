#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    // Tạo các phần tử pipeline
    GstElement *pipeline = gst_pipeline_new("camera-pipeline");
    GstElement *source = gst_element_factory_make("v4l2src", "camera-source");
    GstElement *convert = gst_element_factory_make("videoconvert", "converter");
    GstElement *sink = gst_element_factory_make("autovideosink", "video-output");

    if (!pipeline || !source || !convert || !sink)
    {
        std::cerr << "Lỗi: Không thể tạo một hoặc nhiều phần tử trong pipeline." << std::endl;
        return -1;
    }

    // (Tùy chọn) chọn thiết bị camera nếu không phải /dev/video0
    // g_object_set(source, "device", "/dev/video1", nullptr);

    // Thêm phần tử vào pipeline và nối chúng
    gst_bin_add_many(GST_BIN(pipeline), source, convert, sink, nullptr);
    if (!gst_element_link_many(source, convert, sink, nullptr))
    {
        std::cerr << "Lỗi: Không thể nối các phần tử." << std::endl;
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

    std::cout << "Đang hiển thị video từ camera..." << std::endl;

    // Tạo vòng lặp chính
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // Dọn dẹp
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}
