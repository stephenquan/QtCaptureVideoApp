#ifndef __CaptureVideoFilter__
#define __CaptureVideoFilter__

#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>
#include <QQuickImageProvider>
#include <QUrl>

class CaptureVideoFilter : public QAbstractVideoFilter
{
    Q_OBJECT

    Q_PROPERTY(qint64 interval MEMBER m_Interval NOTIFY intervalChanged)
    Q_PROPERTY(QString image MEMBER m_Image NOTIFY imageChanged)

signals:
    void intervalChanged();
    void imageChanged();

public:
    CaptureVideoFilter(QObject* parent = nullptr);
    QVideoFilterRunnable* createFilterRunnable() Q_DECL_OVERRIDE;

protected:
    qint64 interval() const { return m_Interval; }
    void setInterval(qint64 interval);

    QString image() const { return m_Image; }
    void setImage(const QString& image);

protected:
    qint64 m_Interval;
    QString m_Image;
};

class CaptureVideoFilterRunnable : public QVideoFilterRunnable
{
public:
    CaptureVideoFilterRunnable(CaptureVideoFilter* filter);
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) Q_DECL_OVERRIDE;

protected:
    CaptureVideoFilter* m_Filter;
    qint64 m_LastCapture;
};

class CaptureVideoImageProvider : public QQuickImageProvider
{
public:
    CaptureVideoImageProvider();
    ~CaptureVideoImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) Q_DECL_OVERRIDE;
    QString addImage(const QImage& image);
    static CaptureVideoImageProvider* instance() { return g_this; }

protected:
    static CaptureVideoImageProvider* g_this;
    QVector<QImage> m_Images;
    int m_Id;
};

#endif
