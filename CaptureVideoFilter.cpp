#include "CaptureVideoFilter.h"
#include <QBuffer>
#include <QByteArray>
#include <QDateTime>

CaptureVideoFilter::CaptureVideoFilter( QObject* parent )
    : QAbstractVideoFilter( parent ),
      m_Interval( 1000 )
{
}

void CaptureVideoFilter::setInterval(const int interval)
{
    if (m_Interval == interval)
    {
        return;
    }

    m_Interval = interval;

    emit intervalChanged();
}

void CaptureVideoFilter::setImage(const QString& image)
{
    if (m_Image == image)
    {
        return;
    }

    m_Image = image;

    emit imageChanged();
}

QVideoFilterRunnable* CaptureVideoFilter::createFilterRunnable()
{
    return new CaptureVideoFilterRunnable( this );
}

CaptureVideoFilterRunnable::CaptureVideoFilterRunnable( CaptureVideoFilter* filter ) :
    QVideoFilterRunnable(),
    m_Filter( filter ),
    m_LastCapture( 0 )
{
}

QVideoFrame CaptureVideoFilterRunnable::run( QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags )
{
    Q_UNUSED( surfaceFormat )
    Q_UNUSED( flags )

    if ( !input )
    {
        return QVideoFrame();
    }

    if ( !m_Filter )
    {
        return *input;
    }

    int interval = m_Filter->interval();

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now < m_LastCapture + interval)
    {
        return *input;
    }

    QImage image = input->image();
    QString id = CaptureVideoImageProvider::instance()->addImage(image);

    m_Filter->setImage("image://captureVideo/" + id);
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
