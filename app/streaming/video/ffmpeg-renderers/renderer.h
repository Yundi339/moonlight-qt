#pragma once

#include <SDL.h>

#include "streaming/video/decoder.h"
#include "streaming/video/overlaymanager.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#ifdef HAVE_EGL
#ifdef HAVE_EGL
#define MESA_EGL_NO_X11_HEADERS
#define EGL_NO_X11
#include <SDL_egl.h>

#ifndef EGL_VERSION_1_5
typedef intptr_t EGLAttrib;
typedef void *EGLImage;
typedef EGLImage (EGLAPIENTRYP PFNEGLCREATEIMAGEPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEPROC) (EGLDisplay dpy, EGLImage image);
#endif

#ifndef EGL_KHR_image
// EGL_KHR_image technically uses EGLImageKHR instead of EGLImage, but they're compatible
// so we swap them here to avoid mixing them all over the place
typedef EGLImage (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImage image);
#endif

#ifndef EGL_EXT_image_dma_buf_import
#define EGL_LINUX_DMA_BUF_EXT             0x3270
#define EGL_LINUX_DRM_FOURCC_EXT          0x3271
#define EGL_DMA_BUF_PLANE0_FD_EXT         0x3272
#define EGL_DMA_BUF_PLANE0_OFFSET_EXT     0x3273
#define EGL_DMA_BUF_PLANE0_PITCH_EXT      0x3274
#endif

#ifndef EGL_EXT_image_dma_buf_import_modifiers
#define EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT 0x3443
#define EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT 0x3444
#endif

#define EGL_MAX_PLANES 4
#endif

class EGLExtensions {
public:
    EGLExtensions(EGLDisplay dpy);
    ~EGLExtensions() {}
    bool isSupported(const QString &extension) const;
private:
    const QStringList m_Extensions;
};

#endif

#define RENDERER_ATTRIBUTE_FULLSCREEN_ONLY 0x01
#define RENDERER_ATTRIBUTE_1080P_MAX 0x02

class IFFmpegRenderer : public Overlay::IOverlayRenderer {
public:
    virtual bool initialize(PDECODER_PARAMETERS params) = 0;
    virtual bool prepareDecoderContext(AVCodecContext* context, AVDictionary** options) = 0;
    virtual void renderFrame(AVFrame* frame) = 0;

    virtual bool needsTestFrame() {
        // No test frame required by default
        return false;
    }

    virtual int getDecoderCapabilities() {
        // No special capabilities by default
        return 0;
    }

    virtual int getRendererAttributes() {
        // No special attributes by default
        return 0;
    }

    virtual int getDecoderColorspace() {
        // Rec 601 is default
        return COLORSPACE_REC_601;
    }

    virtual bool isRenderThreadSupported() {
        // Render thread is supported by default
        return true;
    }

    virtual bool isDirectRenderingSupported() {
        // The renderer can render directly to the display
        return true;
    }

    virtual enum AVPixelFormat getPreferredPixelFormat(int videoFormat) {
        if (videoFormat == VIDEO_FORMAT_H265_MAIN10) {
            // 10-bit YUV 4:2:0
            return AV_PIX_FMT_P010;
        }
        else {
            // Planar YUV 4:2:0
            return AV_PIX_FMT_YUV420P;
        }
    }

    virtual bool isPixelFormatSupported(int videoFormat, enum AVPixelFormat pixelFormat) {
        // By default, we only support the preferred pixel format
        return getPreferredPixelFormat(videoFormat) == pixelFormat;
    }

    // IOverlayRenderer
    virtual void notifyOverlayUpdated(Overlay::OverlayType) override {
        // Nothing
    }

#ifdef HAVE_EGL
    // By default we can't do EGL
    virtual bool canExportEGL() {
        return false;
    }

    virtual bool initializeEGL(EGLDisplay,
                               const EGLExtensions &) {
        return false;
    }

    virtual ssize_t exportEGLImages(AVFrame *,
                                    EGLDisplay,
                                    EGLImage[EGL_MAX_PLANES]) {
        return -1;
    }

    // Free the ressources allocated during the last `exportEGLImages` call
    virtual void freeEGLImages(EGLDisplay, EGLImage[EGL_MAX_PLANES]) {}
#endif
};
