#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QSharedDataPointer>

#include "globals.h"

class QEvent;
#define gracefulTheme GracefulTheme::currentTheme()

namespace graceful
{
class GlobalSettings;
class SettingsPrivate;

class GRACEFUL_API Settings : public QSettings
{
    Q_OBJECT
public:
    explicit Settings(const QString& module, QObject* parent = nullptr);
    explicit Settings(const QSettings* parentSettings, const QString& subGroup, QObject* parent=nullptr);
    explicit Settings(const QSettings& parentSettings, const QString& subGroup, QObject* parent=nullptr);
    Settings(const QString &fileName, QSettings::Format format, QObject *parent = nullptr);
    ~Settings() override;

    static const GlobalSettings *globalSettings();

    QVariant localizedValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    void setLocalizedValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void settingsChanged();
    void settingsChangedByApp();
    void settingsChangedFromExternal();

protected:
    bool event(QEvent *event) override;

protected Q_SLOTS:
    virtual void fileChanged();

private Q_SLOTS:
    void _fileChanged(const QString& path);

private:
    void addWatchedFile(QString const & path);

private:
    Q_DISABLE_COPY(Settings)

    SettingsPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(Settings)
};

class GracefulThemeData;

class GRACEFUL_API GracefulTheme
{
public:
    GracefulTheme();
    GracefulTheme(const QString &path);
    GracefulTheme(const GracefulTheme &other);

    GracefulTheme& operator=(const GracefulTheme &other);
    ~GracefulTheme();

    QString name() const;
    QString path() const;

    QString previewImage() const;

    bool isValid() const;
    QString qss(const QString& module) const;
    QString desktopBackground(int screen=-1) const;

    static const GracefulTheme &currentTheme();

    static QList<GracefulTheme> allThemes();

private:
    static GracefulTheme*                   gInstance;
    QSharedDataPointer<GracefulThemeData>   d;
};

#define gracefulTheme GracefulTheme::currentTheme()


class GRACEFUL_API SettingsCache
{
public:
    explicit SettingsCache(QSettings &settings);
    explicit SettingsCache(QSettings *settings);
    virtual ~SettingsCache() {}

    void loadFromSettings();
    void loadToSettings();

private:
    QSettings &mSettings;
    QHash<QString, QVariant> mCache;
};

class GlobalSettingsPrivate;

class GRACEFUL_API GlobalSettings : public Settings
{
    Q_OBJECT
public:
    GlobalSettings();
    ~GlobalSettings() override;

Q_SIGNALS:
    void iconThemeChanged();

    void gracefulThemeChanged();

protected Q_SLOTS:
    void fileChanged() override;

private:
    GlobalSettingsPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(GlobalSettings)
};

}

#endif // SETTINGS_H
