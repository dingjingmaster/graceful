#include "settings.h"

#include <QDir>
#include <QDebug>
#include <QEvent>
#include <QMutex>
#include <XdgDirs>
#include <QStringList>
#include <QSharedData>
#include <QTimerEvent>
#include <QFileSystemWatcher>

graceful::GracefulTheme* graceful::GracefulTheme::gInstance = nullptr;

using namespace graceful;


class graceful::SettingsPrivate
{
public:
    SettingsPrivate(Settings* parent) : mFileChangeTimer(0), mAppChangeTimer(0), mAddWatchTimer(0), mParent(parent)
    {
        // HACK: we need to ensure that the user (~/.config/graceful/<module>.conf)
        //       exists to have functional mWatcher
        if (!mParent->contains("__userfile__")) {
            mParent->setValue("__userfile__", true);
            mParent->sync();
        }
        mWatcher.addPath(mParent->fileName());
        QObject::connect(&(mWatcher), &QFileSystemWatcher::fileChanged, mParent, &Settings::_fileChanged);
    }

    QString localizedKey(const QString& key) const;

    QFileSystemWatcher mWatcher;
    int mFileChangeTimer;
    int mAppChangeTimer;
    int mAddWatchTimer;

private:
    Settings* mParent;
};

class graceful::GracefulThemeData: public QSharedData
{
public:
    GracefulThemeData(): mValid(false) {}
    QString loadQss(const QString& qssFile) const;
    QString findTheme(const QString &themeName);

    bool    mValid;
    QString mName;
    QString mPath;
    QString mPreviewImg;
};


class graceful::GlobalSettingsPrivate
{
public:
    GlobalSettingsPrivate(GlobalSettings *parent) : mParent(parent), mThemeUpdated(0ull)
    {

    }

    GlobalSettings* mParent;
    QString         mIconTheme;
    QString         mGracefulTheme;
    qlonglong       mThemeUpdated;
};


Settings::Settings(const QString& module, QObject* parent) : QSettings("graceful", module, parent), d_ptr(new SettingsPrivate(this))
{

}


Settings::Settings(const QString &fileName, QSettings::Format format, QObject *parent) : QSettings(fileName, format, parent), d_ptr(new SettingsPrivate(this))
{

}


Settings::Settings(const QSettings* parentSettings, const QString& subGroup, QObject* parent) : QSettings(parentSettings->organizationName(), parentSettings->applicationName(), parent), d_ptr(new SettingsPrivate(this))
{
    beginGroup(subGroup);
}

Settings::Settings(const QSettings& parentSettings, const QString& subGroup, QObject* parent) : QSettings(parentSettings.organizationName(), parentSettings.applicationName(), parent), d_ptr(new SettingsPrivate(this))
{
    beginGroup(subGroup);
}


Settings::~Settings()
{
    // because in the Settings::Settings(const QString& module, QObject* parent)
    // constructor there is no beginGroup() called...
    if (!group().isEmpty())
        endGroup();

    delete d_ptr;
}

bool Settings::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        // delay the settingsChanged* signal emitting for:
        //  - checking in _fileChanged
        //  - merging emitting the signals
        if(d_ptr->mAppChangeTimer)
            killTimer(d_ptr->mAppChangeTimer);
        d_ptr->mAppChangeTimer = startTimer(100);
    } else if (event->type() == QEvent::Timer) {
        const int timer = static_cast<QTimerEvent*>(event)->timerId();
        killTimer(timer);
        if (timer == d_ptr->mFileChangeTimer) {
            d_ptr->mFileChangeTimer = 0;
            fileChanged(); // invoke the real fileChanged() handler.
        } else if (timer == d_ptr->mAppChangeTimer) {
            d_ptr->mAppChangeTimer = 0;
            // do emit the signals
            Q_EMIT settingsChangedByApp();
            Q_EMIT settingsChanged();
        } else if (timer == d_ptr->mAddWatchTimer) {
            d_ptr->mAddWatchTimer = 0;
            //try to re-add filename for watching
            addWatchedFile(fileName());
        }
    }

    return QSettings::event(event);
}

void Settings::fileChanged()
{
    sync();
    Q_EMIT settingsChangedFromExternal();
    Q_EMIT settingsChanged();
}

void Settings::_fileChanged(const QString& path)
{
    // check if the file isn't changed by our logic
    // FIXME: this is poor implementation; should we rather compute some hash of values if changed by external?
    if (0 == d_ptr->mAppChangeTimer) {
        // delay the change notification for 100 ms to avoid
        // unnecessary repeated loading of the same config file if
        // the file is changed for several times rapidly.
        if(d_ptr->mFileChangeTimer)
            killTimer(d_ptr->mFileChangeTimer);
        d_ptr->mFileChangeTimer = startTimer(1000);
    }

    addWatchedFile(path);
}

void Settings::addWatchedFile(QString const & path)
{
    if(!d_ptr->mWatcher.files().contains(path))
        // in some situations adding fails because of non-existing file (e.g. editting file in external program)
        if (!d_ptr->mWatcher.addPath(path) && 0 == d_ptr->mAddWatchTimer)
            d_ptr->mAddWatchTimer = startTimer(100);

}


const GlobalSettings *Settings::globalSettings()
{
    static QMutex mutex;
    static GlobalSettings *instance = nullptr;
    if (!instance) {
        mutex.lock();

        if (!instance)
            instance = new GlobalSettings();

        mutex.unlock();
    }

    return instance;
}


/************************************************
 LC_MESSAGES value      Possible keys in order of matching
 lang_COUNTRY@MODIFIER  lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang,
                        default value
 lang_COUNTRY           lang_COUNTRY, lang, default value
 lang@MODIFIER          lang@MODIFIER, lang, default value
 lang                   lang, default value
 ************************************************/
QString SettingsPrivate::localizedKey(const QString& key) const
{

    QString lang = QString::fromLocal8Bit(qgetenv("LC_MESSAGES"));

    if (lang.isEmpty())
        lang = QString::fromLocal8Bit(qgetenv("LC_ALL"));

    if (lang.isEmpty())
        lang = QString::fromLocal8Bit(qgetenv("LANG"));


    QString modifier = lang.section('@', 1);
    if (!modifier.isEmpty())
        lang.truncate(lang.length() - modifier.length() - 1);

    QString encoding = lang.section('.', 1);
    if (!encoding.isEmpty())
        lang.truncate(lang.length() - encoding.length() - 1);


    QString country = lang.section('_', 1);
    if (!country.isEmpty())
        lang.truncate(lang.length() - country.length() - 1);



    //qDebug() << "LC_MESSAGES: " << getenv("LC_MESSAGES");
    //qDebug() << "Lang:" << lang;
    //qDebug() << "Country:" << country;
    //qDebug() << "Encoding:" << encoding;
    //qDebug() << "Modifier:" << modifier;

    if (!modifier.isEmpty() && !country.isEmpty()) {
        QString k = QString::fromLatin1("%1[%2_%3@%4]").arg(key, lang, country, modifier);
        //qDebug() << "\t try " << k << mParent->contains(k);
        if (mParent->contains(k))
            return k;
    }

    if (!country.isEmpty()) {
        QString k = QString::fromLatin1("%1[%2_%3]").arg(key, lang, country);
        //qDebug() << "\t try " << k  << mParent->contains(k);
        if (mParent->contains(k))
            return k;
    }

    if (!modifier.isEmpty()) {
        QString k = QString::fromLatin1("%1[%2@%3]").arg(key, lang, modifier);
        //qDebug() << "\t try " << k  << mParent->contains(k);
        if (mParent->contains(k))
            return k;
    }

    QString k = QString::fromLatin1("%1[%2]").arg(key, lang);
    //qDebug() << "\t try " << k  << mParent->contains(k);
    if (mParent->contains(k))
        return k;


    //qDebug() << "\t try " << key  << mParent->contains(key);
    return key;
}

QVariant Settings::localizedValue(const QString& key, const QVariant& defaultValue) const
{
    Q_D(const Settings);
    return value(d->localizedKey(key), defaultValue);
}


void Settings::setLocalizedValue(const QString &key, const QVariant &value)
{
    Q_D(const Settings);
    setValue(d->localizedKey(key), value);
}


GracefulTheme::GracefulTheme() : d(new GracefulThemeData)
{

}


GracefulTheme::GracefulTheme(const QString &path) : d(new GracefulThemeData)
{
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    if (fi.isAbsolute()) {
        d->mPath = path;
        d->mName = fi.fileName();
        d->mValid = fi.isDir();
    } else {
        d->mName = path;
        d->mPath = d->findTheme(path);
        d->mValid = !(d->mPath.isEmpty());
    }

    if (QDir(path).exists("preview.png"))
        d->mPreviewImg = path + "/preview.png";
}


QString GracefulThemeData::findTheme(const QString &themeName)
{
    if (themeName.isEmpty())
        return QString();

    QStringList paths;
//    QLatin1String fallback(GRACEFUL_INSTALL_PREFIX);

    paths << XdgDirs::dataHome(false);
    paths << XdgDirs::dataDirs();

    for(const QString &path : qAsConst(paths)) {
        QDir dir(QString::fromLatin1("%1/graceful/themes/%2").arg(path, themeName));
        if (dir.isReadable())
            return dir.absolutePath();
    }

    return QString();
}

GracefulTheme::GracefulTheme(const GracefulTheme &other) : d(other.d)
{

}


GracefulTheme::~GracefulTheme() = default;


GracefulTheme& GracefulTheme::operator=(const GracefulTheme &other)
{
    if (this != &other)
        d = other.d;

    return *this;
}


bool GracefulTheme::isValid() const
{
    return d->mValid;
}

QString GracefulTheme::name() const
{
    return d->mName;
}

QString GracefulTheme::path() const
{
    return d->mPath;
}


QString GracefulTheme::previewImage() const
{
    return d->mPreviewImg;
}

QString GracefulTheme::qss(const QString& module) const
{
    return d->loadQss(QStringLiteral("%1/%2.qss").arg(d->mPath, module));
}

QString GracefulThemeData::loadQss(const QString& qssFile) const
{
    QFile f(qssFile);
    if (! f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QString qss = QString::fromLocal8Bit(f.readAll());
    f.close();

    if (qss.isEmpty())
        return QString();

    // handle relative paths
    QString qssDir = QFileInfo(qssFile).canonicalPath();
    qss.replace(QRegExp("url.[ \\t\\s]*", Qt::CaseInsensitive, QRegExp::RegExp2), "url(" + qssDir + '/');

    return qss;
}


QString GracefulTheme::desktopBackground(int screen) const
{
    QString wallpaperCfgFileName = QString::fromLatin1("%1/wallpaper.cfg").arg(d->mPath);

    if (wallpaperCfgFileName.isEmpty())
        return QString();

    QSettings s(wallpaperCfgFileName, QSettings::IniFormat);
    QString themeDir = QFileInfo(wallpaperCfgFileName).absolutePath();
    // There is something strange... If I remove next line the wallpapers array is not found...
    s.childKeys();
    s.beginReadArray("wallpapers");

    s.setArrayIndex(screen - 1);
    if (s.contains("file"))
        return QString::fromLatin1("%1/%2").arg(themeDir, s.value("file").toString());

    s.setArrayIndex(0);
    if (s.contains("file"))
        return QString::fromLatin1("%1/%2").arg(themeDir, s.value("file").toString());

    return QString();
}


const GracefulTheme &GracefulTheme::currentTheme()
{
    static GracefulTheme theme;
    QString name = Settings::globalSettings()->value("theme").toString();
    if (theme.name() != name) {
        theme = GracefulTheme(name);
    }
    return theme;
}


QList<GracefulTheme> GracefulTheme::allThemes()
{
    QList<GracefulTheme> ret;
    QSet<QString> processed;

    QStringList paths;
    paths << XdgDirs::dataHome(false);
    paths << XdgDirs::dataDirs();

    for(const QString &path : qAsConst(paths)) {
        QDir dir(QString::fromLatin1("%1/graceful/themes").arg(path));
        const QFileInfoList dirs = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

        for (const QFileInfo &dir : dirs) {
            if (!processed.contains(dir.fileName()) && QDir(dir.absoluteFilePath()).exists("graceful-panel.qss")) {
                processed << dir.fileName();
                ret << GracefulTheme(dir.absoluteFilePath());
            }
        }
    }

    return ret;
}

SettingsCache::SettingsCache(QSettings &settings) : mSettings(settings)
{
    loadFromSettings();
}


SettingsCache::SettingsCache(QSettings *settings) : mSettings(*settings)
{
    loadFromSettings();
}


void SettingsCache::loadFromSettings()
{
    const QStringList keys = mSettings.allKeys();

    const int N = keys.size();
    for (int i = 0; i < N; ++i) {
        mCache.insert(keys.at(i), mSettings.value(keys.at(i)));
    }
}


void SettingsCache::loadToSettings()
{
    QHash<QString, QVariant>::const_iterator i = mCache.constBegin();

    while(i != mCache.constEnd()) {
        mSettings.setValue(i.key(), i.value());
        ++i;
    }

    mSettings.sync();
}


GlobalSettings::GlobalSettings() : Settings("graceful"), d_ptr(new GlobalSettingsPrivate(this))
{
    if (value("icon_theme").toString().isEmpty()) {
        qWarning() << QString::fromLatin1("Icon Theme not set. Fallbacking to Oxygen, if installed");
        const QString fallback(QLatin1String("oxygen"));
        const QDir dir(QLatin1String(GRACEFUL_DATA_DIR) + QLatin1String("/icons"));
        if (dir.exists(fallback))
        {
            setValue("icon_theme", fallback);
            sync();
        } else {
            qWarning() << QString::fromLatin1("Fallback Icon Theme (Oxygen) not found");
        }
    }

    fileChanged();
}

GlobalSettings::~GlobalSettings()
{
    delete d_ptr;
}


void GlobalSettings::fileChanged()
{
    Q_D(GlobalSettings);
    sync();


    QString it = value("icon_theme").toString();
    if (d->mIconTheme != it) {
        Q_EMIT iconThemeChanged();
    }

    QString rt = value("theme").toString();
    qlonglong themeUpdated = value("__theme_updated__").toLongLong();
    if ((d->mGracefulTheme != rt) || (d->mThemeUpdated != themeUpdated)) {
        d->mGracefulTheme = rt;
        Q_EMIT gracefulThemeChanged();
    }

    Q_EMIT settingsChangedFromExternal();
    Q_EMIT settingsChanged();
}

