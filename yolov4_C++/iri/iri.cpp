#include <opencv2/opencv.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudacodec.hpp>
#include <Windows.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <chrono>

constexpr float CONFIDENCE_THRESHOLD = 0;
constexpr float NMS_THRESHOLD = 0.4;
constexpr int NUM_CLASSES = 2;
constexpr float AX = 35.0;
constexpr float AY = 58.0;

const cv::Scalar colors[] = {
    {0, 255, 255},
    {255, 255, 0},
    {0, 255, 0},
    {255, 0, 0}
};
const auto NUM_COLORS = sizeof(colors) / sizeof(colors[0]);

int main()
{
    cv::Mat blob, frame_RGB, frame_RGBA;
    cv::Point closest, current;
    std::vector<cv::Mat> detections;
    std::vector<std::vector<int>> results, clear_results(20, std::vector<int>(4)); //20 x 4
    std::string move;
    std::wstring comport;
    int count, closest_index, FOV_x = 400, FOV_y = 400, screen_width = 1920, screen_height = 1080;

    std::vector<std::string> class_names;
    {
        std::ifstream class_file("D:\coco.labels");
        std::string line;

        while (std::getline(class_file, line)) {
            class_names.push_back(line);
        }
    }

    HBITMAP hBitmap;

    HDC hdcSys = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcSys);
    void* ptrBitmapPixels;

    BITMAPINFO bi; HDC hdc;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = FOV_x;
    bi.bmiHeader.biHeight = -FOV_y;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    hdc = GetDC(NULL);
    hBitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &ptrBitmapPixels, NULL, 0);

    SelectObject(hdcMem, hBitmap);
    frame_RGBA = cv::Mat(FOV_y, FOV_x, CV_8UC4, ptrBitmapPixels, 0);

    auto net = cv::dnn::readNetFromDarknet("D:\pp5.cfg", "D:\pp4.weights");
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    auto output_names = net.getUnconnectedOutLayersNames();

    comport = L"COM3";

    HANDLE hSerial;

    hSerial = CreateFile(comport.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    SetCommState(hSerial, &dcbSerialParams);

    while (cv::waitKey(1) < 1)
    {
        auto start = std::chrono::high_resolution_clock::now();
        count = 0;
        results = clear_results;

        BitBlt(hdcMem, 0, 0, FOV_x, FOV_y, hdcSys, screen_width / 2 - FOV_x / 2, screen_height / 2 - FOV_y / 2, SRCCOPY);
        cv::cvtColor(frame_RGBA, frame_RGB, cv::COLOR_RGBA2RGB);

        cv::dnn::blobFromImage(frame_RGB, blob, 0.00392, cv::Size(416, 416), cv::Scalar(), true, false, CV_32F);
        net.setInput(blob);
        net.forward(detections, output_names);

        std::vector<int> indices[NUM_CLASSES];
        std::vector<cv::Rect> boxes[NUM_CLASSES];
        std::vector<float> scores[NUM_CLASSES];

        for (auto& output : detections)
        {
            const auto num_boxes = output.rows;
            for (int i = 0; i < num_boxes; i++)
            {
                auto x = output.at<float>(i, 0) * frame_RGB.cols;
                auto y = output.at<float>(i, 1) * frame_RGB.rows;
                auto width = output.at<float>(i, 2) * frame_RGB.cols;
                auto height = output.at<float>(i, 3) * frame_RGB.rows;
                cv::Rect rect(x - width / 2, y - height / 1.145, width, height);

                for (int c = 0; c < NUM_CLASSES; c++)
                {
                    auto confidence = *output.ptr<float>(i, 5 + c);
                    if (confidence >= CONFIDENCE_THRESHOLD)
                    {
                        boxes[c].push_back(rect);
                        scores[c].push_back(confidence);
                    }
                }
            }
        }

        for (int c = 0; c < NUM_CLASSES; c++) {
            cv::dnn::NMSBoxes(boxes[c], scores[c], 0.0, NMS_THRESHOLD, indices[c]);
        }

        for (int c = 0; c < NUM_CLASSES; c++)
        {
            for (size_t i = 0; i < indices[c].size(); ++i)
            {
                const auto color = colors[c % NUM_COLORS];

                auto idx = indices[c][i];
                const auto& rect = boxes[c][idx];
                cv::rectangle(frame_RGB, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), color, 3);

                results[count][0] = rect.x;
                results[count][1] = rect.y;
                results[count][2] = rect.width;
                results[count][3] = rect.height;

                count = count + 1;

                std::ostringstream label_ss;
                label_ss << class_names[c] << ": " << std::fixed << std::setprecision(2) << scores[c][idx];
                auto label = label_ss.str();

                int baseline;
                auto label_bg_sz = cv::getTextSize(label.c_str(), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, 1, &baseline);
                cv::rectangle(frame_RGB, cv::Point(rect.x, rect.y - label_bg_sz.height - baseline - 10), cv::Point(rect.x + label_bg_sz.width, rect.y), color, cv::FILLED);
                cv::putText(frame_RGB, label.c_str(), cv::Point(rect.x, rect.y - baseline - 5), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 0, 0));
            }
        }

        closest = cv::Point(FOV_x, FOV_y);
        bool targetFound = false;

        for (int i = 0; i < count; i++)
        {
            current = cv::Point(results[i][0] + results[i][2] / 2, results[i][1] + results[i][3] / 2);

            double distanceX = std::abs(current.x - FOV_x / 2);
            double distanceY = std::abs(current.y - FOV_y / 2);

            if (distanceX < AX && distanceY < AY)
            {
                targetFound = true;

                double distanceClosestX = std::abs(closest.x - FOV_x / 2);
                double distanceClosestY = std::abs(closest.y - FOV_y / 2);

                if (distanceX < distanceClosestX || distanceY < distanceClosestY)
                {
                    closest = current;
                    closest_index = i;
                }
            }
        }

        if (targetFound)
        {
            move = std::to_string(results[closest_index][0] + results[closest_index][2] / 2 - FOV_x / 2) + ":" +
                std::to_string(results[closest_index][1] + results[closest_index][3] / 2 - FOV_y / 2);

            if (move != std::to_string(FOV_x / 2 * -1) + ":" + std::to_string(FOV_y / 2 * -1))
            {
                if ((GetKeyState('P') & 0x8000) || (GetKeyState('O') & 0x8000)) //  GetKeyState('O')
                {
                    WriteFile(hSerial, &move, sizeof(move), NULL, NULL);
                }

                cv::line(frame_RGB, cv::Point(FOV_x / 2, FOV_y / 2), closest, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
            }
        }


        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        cv::putText(frame_RGB, std::to_string(1000000 / duration.count()), cv::Point(10, 15), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 0, 255));

        constexpr int length = 15;
        const auto characters = TEXT("abcdefghi9172345jklmnopqrstuv2819359604773wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
        TCHAR title[length + 1]{};

        for (int j = 0; j != length; j++)
        {
            title[j] += characters[rand() % 80];
        }

        SetConsoleTitle(title);
    }

    cv::destroyAllWindows();
}
