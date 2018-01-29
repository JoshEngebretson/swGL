﻿#include <memory>
#include <algorithm>
#include "Defines.h"
#include "DrawSurface.h"

namespace SWGL {

#if !SWGL_USE_HARDWARE_GAMMA
    GammaRamp DrawSurface::m_gammaRamp;
#endif

    DrawSurface::DrawSurface()

        : m_width(0),
          m_height(0) {

        // Calculate how often the drawing buffer can be subdivided with respect
        // to the number of drawing threads
        int numSubDivX = 1, numSubDivY = 1;
        for (int n = SWGL_NUM_DRAW_THREADS; n > 1; ) {

            numSubDivX++; n >>= 1;
            if (n == 1) break;
            numSubDivY++; n >>= 1;
        }
        m_numBuffersInX = 1 << (numSubDivX - 1);
        m_numBuffersInY = 1 << (numSubDivY - 1);

        // Initialize drawing buffer
        for (auto i = 0U; i < SWGL_NUM_DRAW_THREADS; i++) {

            m_buffer[i] = std::make_shared<DrawBuffer>();
        }
    }



    void DrawSurface::setHDC(HDC hdc) {

        m_hdc = hdc;

        RECT r;
        GetClientRect(WindowFromDC(m_hdc), &r);

        int width = static_cast<int>(r.right - r.left);
        int height = static_cast<int>(r.bottom - r.top);

        // Resize the drawing surface if needed
        if (width != m_width || height != m_height) {

            m_width = width;
            m_height = height;

            // Make sure that the surface can be evenly divided between the buffers
            width = std::max((((width / m_numBuffersInX) + 3) & ~3) * m_numBuffersInX, m_numBuffersInX * 2);
            height = std::max((((height / m_numBuffersInY) + 3) & ~3) * m_numBuffersInY, m_numBuffersInY * 2);

            // Init storage in which the unswizzled color buffer gets written into
            m_unswizzledColor.resize(width * height);

            // Setup bitmap info structure which is needed for SetDIBitsToDevice()
            memset(&m_bmi, 0, sizeof(BITMAPINFO));
            m_bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
            m_bmi.bmiHeader.biWidth = width;
            m_bmi.bmiHeader.biHeight = height;
            m_bmi.bmiHeader.biPlanes = 1;
            m_bmi.bmiHeader.biBitCount = 32;
            m_bmi.bmiHeader.biCompression = BI_RGB;

            // Initialize the buffer for each drawing thread
            m_bufferWidth = width / m_numBuffersInX;
            m_bufferHeight = height / m_numBuffersInY;

            for (int y = 0, idx = 0; y < m_numBuffersInY; y++) {

                for (int x = 0; x < m_numBuffersInX; x++) {

                    auto minX = x * m_bufferWidth;
                    auto minY = y * m_bufferHeight;
                    auto maxX = (x == m_numBuffersInX - 1) ? width : (x + 1) * m_bufferWidth;
                    auto maxY = (y == m_numBuffersInY - 1) ? height : (y + 1) * m_bufferHeight;

                    m_buffer[idx++]->resize(minX, minY, maxX, maxY);
                }
            }
        }
    }



    void DrawSurface::swap() {

        // Unswizzle color buffer
        auto width = m_bmi.bmiHeader.biWidth;
        auto height = m_bmi.bmiHeader.biHeight;
        auto dst = m_unswizzledColor.data();

        for (auto &buffer : m_buffer) {

            buffer->unswizzleColor(dst, width);
        }

#if !SWGL_USE_HARDWARE_GAMMA
        // Software emulated gamma correction
        m_gammaRamp.correct(

            m_unswizzledColor.data(), m_unswizzledColor.size()
        );
#endif

        // Blit pixels to device
        SetDIBitsToDevice(

            m_hdc,
            0, 0,
            width, height,
            0, 0,
            0, height,
            reinterpret_cast<void *>(dst),
            &m_bmi,
            DIB_RGB_COLORS
        );
    }
}
