#ifndef __CaptureVideoFilter__
#define __CaptureVideoFilter__

#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>
#include <QQuickImageProvider>
#include <QVideoFrame>
#include <QUrl>

class CaptureVideoFilter : public QAbstractVideoFilter
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap imageInfo MEMBER m_ImageInfo NOTIFY imageInfoChanged)
    Q_PROPERTY(ConversionMethod conversionMethod MEMBER m_ConversionMethod NOTIFY conversionMethodChanged)

public:
    enum ConversionMethod
    {
        ConversionMethodNone,
        ConversionMethodQt,
        ConversionMethodMap,
        ConversionMethodOpenGL
    };

    Q_ENUM(ConversionMethod)

public:
    enum PixelFormat
    {
        Format_Invalid = QVideoFrame::Format_Invalid,
        Format_ARGB32 = QVideoFrame::Format_ARGB32,
        Format_ARGB32_Premultiplied = QVideoFrame::Format_ARGB32_Premultiplied,
        Format_RGB32 = QVideoFrame::Format_RGB32,
        Format_RGB24 = QVideoFrame::Format_RGB24,
        Format_RGB565 = QVideoFrame::Format_RGB565,
        Format_RGB555 = QVideoFrame::Format_RGB555,
        Format_ARGB8565_Premultiplied = QVideoFrame::Format_ARGB8565_Premultiplied,
        Format_BGRA32 = QVideoFrame::Format_BGRA32,
        Format_BGRA32_Premultiplied = QVideoFrame::Format_BGRA32_Premultiplied,
        Format_BGR32 = QVideoFrame::Format_BGR32,
        Format_BGR24 = QVideoFrame::Format_BGR24,
        Format_BGR565 = QVideoFrame::Format_BGR565,
        Format_BGR555 = QVideoFrame::Format_BGR555,
        Format_BGRA5658_Premultiplied = QVideoFrame::Format_BGRA5658_Premultiplied,

        Format_AYUV444 = QVideoFrame::Format_AYUV444,
        Format_AYUV444_Premultiplied = QVideoFrame::Format_AYUV444_Premultiplied,
        Format_YUV444 = QVideoFrame::Format_YUV444,
        Format_YUV420P = QVideoFrame::Format_YUV420P,
        Format_YV12 = QVideoFrame::Format_YV12,
        Format_UYVY = QVideoFrame::Format_UYVY,
        Format_YUYV = QVideoFrame::Format_YUYV,
        Format_NV12 = QVideoFrame::Format_NV12,
        Format_NV21 = QVideoFrame::Format_NV21,
        Format_IMC1 = QVideoFrame::Format_IMC1,
        Format_IMC2 = QVideoFrame::Format_IMC2,
        Format_IMC3 = QVideoFrame::Format_IMC3,
        Format_IMC4 = QVideoFrame::Format_IMC4,
        Format_Y8 = QVideoFrame::Format_Y8,
        Format_Y16 = QVideoFrame::Format_Y16,

        Format_Jpeg = QVideoFrame::Format_Jpeg,

        Format_CameraRaw = QVideoFrame::Format_CameraRaw,
        Format_AdobeDng = QVideoFrame::Format_AdobeDng,
        Format_ABGR32 = QVideoFrame::Format_ABGR32, // ### Qt 6: reorder
        Format_YUV422P = QVideoFrame::Format_YUV422P,

#ifndef Q_QDOC
        NPixelFormats = QVideoFrame::NPixelFormats,
#endif
        Format_User = QVideoFrame::Format_User
    };
    Q_ENUM(PixelFormat)

public:
    CaptureVideoFilter(QObject* parent = nullptr);
    QVideoFilterRunnable* createFilterRunnable() Q_DECL_OVERRIDE;

signals:
    void frame();
    void imageInfoChanged();
    void conversionMethodChanged();

protected:
    QVariantMap m_ImageInfo;
    ConversionMethod m_ConversionMethod;
};

class CaptureVideoFilterRunnable : public QVideoFilterRunnable
{
public:
    CaptureVideoFilterRunnable(CaptureVideoFilter* filter);
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) Q_DECL_OVERRIDE;

protected:
    CaptureVideoFilter* m_Filter;
};

#endif
