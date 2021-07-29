#include "CaptureVideoFilter.h"
#include <QBuffer>
#include <QByteArray>
#include <QDateTime>

CaptureVideoFilter::CaptureVideoFilter(QObject* parent)
    : QAbstractVideoFilter(parent),
      m_Interval(1000)
{
}

QVideoFilterRunnable* CaptureVideoFilter::createFilterRunnable()
{
    return new CaptureVideoFilterRunnable(this);
}

CaptureVideoFilterRunnable::CaptureVideoFilterRunnable(CaptureVideoFilter* filter) :
    QVideoFilterRunnable(),
    m_Filter(filter),
    m_LastCapture(0)
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

    qint64 interval = m_Filter->property("interval").toLongLong();

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now < m_LastCapture + interval)
    {
        return *input;
    }

    QImage image = input->image();
    QString id = CaptureVideoImageProvider::instance()->addImage(image);
    QMetaEnum formatEnum = QMetaEnum::fromType<QImage::Format>();
    QMetaEnum pixelFormatEnum = QMetaEnum::fromType<CaptureVideoFilter::PixelFormat>();
    QString imageInfo =
            QString("input:")
            + pixelFormatEnum.valueToKey(static_cast<CaptureVideoFilter::PixelFormat>(input->pixelFormat()))
            + ":"
            + QString::number(input->width())
            + "x"
            + QString::number(input->height())
            + " "
            + QString("image:")
            + formatEnum.valueToKey(image.format())
            + ":"
            + QString::number(image.width())
            + "x"
            + QString::number(image.height())
            ;

    m_Filter->setProperty("image", "image://captureVideo/" + id);
    m_Filter->setProperty("imageInfo", imageInfo);
    m_LastCapture = now;

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
