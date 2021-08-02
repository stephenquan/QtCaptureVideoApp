#include "CaptureVideoFilter.h"
#include <QBuffer>
#include <QByteArray>
#include <QDateTime>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQmlContext>

QString errorString;

QUrl QImageToDataUri(const QImage& image)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");
    return QString("data:image/png;base64,") + byteArray.toBase64();
}

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
    imageInfo["QVideoFrame.pixelFormat"] = pixelFormatEnum.valueToKey(static_cast<CaptureVideoFilter::PixelFormat>(input->pixelFormat()));
    imageInfo["QVideoFrame.resolution"] = QString::number(input->width()) + "x" + QString::number(input->height());

    CaptureVideoFilter::ConversionMethod method = static_cast<CaptureVideoFilter::ConversionMethod>(m_Filter->property("conversionMethod").toInt());
    switch (method)
    {
    case CaptureVideoFilter::ConversionMethodNone:
        imageInfo["ConversionMethod"] = "None";
        break;
    case CaptureVideoFilter::ConversionMethodQt:
        imageInfo["ConversionMethod"] = "QVideoFrame::image()";
        break;
    case CaptureVideoFilter::ConversionMethodMap:
        imageInfo["ConversionMethod"] = "QVideoFrame::map()";
        break;
    case CaptureVideoFilter::ConversionMethodOpenGL:
        imageInfo["ConversionMethod"] = "GL_FRAMEBUFFER";
        break;
    }

    QImage image;
    switch (method)
    {
    case CaptureVideoFilter::ConversionMethodQt:
        image = input->image()
                .mirrored(
                    false,
                    surfaceFormat.scanLineDirection() == QVideoSurfaceFormat::BottomToTop);
        break;
    case CaptureVideoFilter::ConversionMethodMap:
        if (!QVideoFrameToQImageUsingMap(input, image, surfaceFormat))
        {
            imageInfo["errorString"] = errorString;
            m_Filter->setProperty("imageInfo", imageInfo);
            return *input;
        }
        break;
    case CaptureVideoFilter::ConversionMethodOpenGL:
        QVideoFrameToQImageUsingOpenGL(input, image);
        break;
    default:
        break;
    }

    if (!image.isNull())
    {
        QMetaEnum formatEnum = QMetaEnum::fromType<QImage::Format>();
        imageInfo["QImage.format"] = formatEnum.valueToKey(image.format());
        imageInfo["QImage.resolution"] = QString::number(image.width()) + "x" + QString::number(image.height());
        if (m_Filter->property("capturing").toBool())
        {
            emit m_Filter->captured(QImageToDataUri(image));
            m_Filter->setProperty("capturing", false);
        }
    }
    m_Filter->setProperty("imageInfo", imageInfo);
    emit m_Filter->frame();

    return !image.isNull() ? image : *input;
}
