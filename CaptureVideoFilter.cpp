#include "CaptureVideoFilter.h"
#include <QBuffer>
#include <QByteArray>
#include <QDateTime>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQmlContext>

QString errorString;

bool QVideoFrameToQImageUsingMap(QVideoFrame* input, QImage& image, const QVideoSurfaceFormat &surfaceFormat)
{
    errorString.clear();
    QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat());
    if (format == QImage::Format_Invalid)
    {
        if (input->pixelFormat() != QVideoFrame::Format_ABGR32)
        {
            QMetaEnum pixelFormatEnum = QMetaEnum::fromType<CaptureVideoFilter::PixelFormat>();
            errorString = QStringLiteral("")
                    + pixelFormatEnum.valueToKey(input->pixelFormat())
                    + QStringLiteral(" ")
                    + QStringLiteral("(")
                    + QString::number(input->pixelFormat())
                    + QStringLiteral(")")
                    + QStringLiteral(" not supported")
                    ;
            return false;
        }
        if (!input->map(QAbstractVideoBuffer::ReadOnly))
        {
            errorString = QStringLiteral("QVideoFrame::map() not supported");
            return false;
        }
        image = QImage(
                    input->bits(),
                    input->width(),
                    input->height(),
                    input->bytesPerLine(),
                    QImage::Format_ARGB32)
                       .rgbSwapped()
                       .mirrored(false,
                                 true
                                 );
        input->unmap();
        return true;
    }

    if (!input->map(QAbstractVideoBuffer::ReadOnly))
    {
        errorString = QStringLiteral("QVideoFrame::map() not supported");
        return false;
    }

    image = QImage(
                input->bits(),
                input->width(),
                input->height(),
                input->bytesPerLine(),
                format)
                .copy()
                .mirrored(
                    false,
                    surfaceFormat.scanLineDirection() == QVideoSurfaceFormat::BottomToTop)
            ;
    input->unmap();
    return true;
}

bool QVideoFrameToQImageUsingOpenGL(QVideoFrame* input, QImage& image)
{
    image = QImage(input->width(), input->height(), QImage::Format_ARGB32);
    GLuint textureId = static_cast<GLuint>( input->handle().toInt() );
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions* f = ctx->functions();
    GLuint fbo;
    f->glGenFramebuffers( 1, &fbo );
    GLint prevFbo;
    f->glGetIntegerv( GL_FRAMEBUFFER_BINDING, &prevFbo );
    f->glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    f->glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0 );
    f->glReadPixels( 0, 0,  input->width(),  input->height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits() );
    f->glBindFramebuffer( GL_FRAMEBUFFER, static_cast<GLuint>( prevFbo ) );
    image = image.rgbSwapped();
    return true;
}


CaptureVideoFilter::CaptureVideoFilter(QObject* parent)
    : QAbstractVideoFilter(parent),
      m_ConversionMethod(ConversionMethodQt)
{
}

QVideoFilterRunnable* CaptureVideoFilter::createFilterRunnable()
{
    return new CaptureVideoFilterRunnable(this);
}

CaptureVideoFilterRunnable::CaptureVideoFilterRunnable(CaptureVideoFilter* filter) :
    QVideoFilterRunnable(),
    m_Filter(filter)
{
}

QVideoFrame CaptureVideoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags)
{
    Q_UNUSED(surfaceFormat)
    Q_UNUSED(flags)

    if (!input)
    {
        return QVideoFrame();
    }

    if (!m_Filter)
    {
        return *input;
    }

    QVariantMap imageInfo;

    QMetaEnum pixelFormatEnum = QMetaEnum::fromType<CaptureVideoFilter::PixelFormat>();
    imageInfo["qvideoFramePixelFormat"] = pixelFormatEnum.valueToKey(static_cast<CaptureVideoFilter::PixelFormat>(input->pixelFormat()));
    imageInfo["qvideoFrameResolution"] = QString::number(input->width()) + "x" + QString::number(input->height());

    CaptureVideoFilter::ConversionMethod method = static_cast<CaptureVideoFilter::ConversionMethod>(m_Filter->property("conversionMethod").toInt());
    switch (method)
    {
    case CaptureVideoFilter::ConversionMethodQt:
        imageInfo["method"] = "QVideoFrame::image()";
        break;
    case CaptureVideoFilter::ConversionMethodMap:
        imageInfo["method"] = "QVideoFrame::map()";
        break;
    case CaptureVideoFilter::ConversionMethodOpenGL:
        imageInfo["method"] = "GL_FRAMEBUFFER";
        break;
    }

    QImage image;

    if (method == CaptureVideoFilter::ConversionMethodQt)
    {
        image = input->image()
                .mirrored(
                    false,
                    surfaceFormat.scanLineDirection() == QVideoSurfaceFormat::BottomToTop);
    }
    if (method == CaptureVideoFilter::ConversionMethodMap)
    {
        if (!QVideoFrameToQImageUsingMap(input, image, surfaceFormat))
        {
            imageInfo["errorString"] = errorString;
            m_Filter->setProperty("imageInfo", imageInfo);
            return *input;
        }
    }
    if (method == CaptureVideoFilter::ConversionMethodOpenGL)
    {
        QVideoFrameToQImageUsingOpenGL(input, image);
    }

    QString id = CaptureVideoImageProvider::instance()->addImage(image);
    QMetaEnum formatEnum = QMetaEnum::fromType<QImage::Format>();
    imageInfo["qimageFormat"] = formatEnum.valueToKey(image.format());
    imageInfo["qimageResolution"] = QString::number(image.width()) + "x" + QString::number(image.height());

    m_Filter->setProperty("image", "image://captureVideo/" + id);
    m_Filter->setProperty("imageInfo", imageInfo);

    return *input;
}

CaptureVideoImageProvider * CaptureVideoImageProvider::g_this = nullptr;

CaptureVideoImageProvider::CaptureVideoImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Image),
    m_Id(0)
{
    g_this = this;
}

CaptureVideoImageProvider::~CaptureVideoImageProvider()
{
    g_this = nullptr;
}

QString CaptureVideoImageProvider::addImage(const QImage& image)
{
    while (m_Images.size() > 10)
    {
        m_Images.pop_front();
    }

    m_Id++;
    QString id = QString::number(m_Id);
    QImage copiedImage(image);
    copiedImage.setText("id", id);
    m_Images.push_back(copiedImage);
    return id;
}

QImage CaptureVideoImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)

    foreach (QImage image, m_Images)
    {
        if (image.text("id") == id)
        {
            if (size)
            {
                *size = QSize(image.width(), image.height());
            }
            return image;
        }
    }

    return QImage();
}
