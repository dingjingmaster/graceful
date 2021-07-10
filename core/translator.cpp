#include "translator.h"

#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QStringList>
#include <QStringBuilder>
#include <QFileInfo>

#include <XdgDirs>

using namespace graceful;

bool translate(const QString &name, const QString &owner = QString());

QStringList *getSearchPaths()
{
    static QStringList *searchPath = nullptr;

    if (searchPath == nullptr) {
        searchPath = new QStringList();
        *searchPath << XdgDirs::dataDirs('/' + GRACEFUL_RELATIVE_SHARE_TRANSLATIONS_DIR);
        *searchPath << GRACEFUL_SHARE_TRANSLATIONS_DIR;
        searchPath->removeDuplicates();
    }

    return searchPath;
}


QStringList Translator::translationSearchPaths()
{
    return *(getSearchPaths());
}


void Translator::setTranslationSearchPaths(const QStringList &paths)
{
    QStringList *p = getSearchPaths();
    p->clear();
    *p << paths;
}

bool translate(const QString &name, const QString &owner)
{
    const QString locale = QLocale::system().name();
    QTranslator *appTranslator = new QTranslator(qApp);

    QStringList *paths = getSearchPaths();
    for(const QString &path : qAsConst(*paths)) {
        QStringList subPaths;

        if (!owner.isEmpty()) {
            subPaths << path + QL1C('/') + owner + QL1C('/') + name;
        } else {
            subPaths << path + QL1C('/') + name;
            subPaths << path;
        }

        for(const QString &p : qAsConst(subPaths)) {
            if (appTranslator->load(name + QL1C('_') + locale, p)) {
                QCoreApplication::installTranslator(appTranslator);
                return true;
            } else if (locale == QLatin1String("C") || locale.startsWith(QLatin1String("en"))) {
                // English is the default. Even if there isn't an translation
                // file, we return true. It's translated anyway.
                delete appTranslator;
                return true;
            }
        }
    }

    // If we got here, no translation was loaded. appTranslator has no use.
    delete appTranslator;

    return false;
}

bool Translator::translateApplication(const QString &applicationName)
{
    const QString locale = QLocale::system().name();
    QTranslator *qtTranslator = new QTranslator(qApp);

    if (qtTranslator->load(QL1S("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        qApp->installTranslator(qtTranslator);
    } else {
        delete qtTranslator;
    }

    if (!applicationName.isEmpty()) {
        return translate(applicationName);
    } else {
        return translate(QFileInfo(QCoreApplication::applicationFilePath()).baseName());
    }
}


bool Translator::translateLibrary(const QString &libraryName)
{
    static QSet<QString> loadedLibs;

    if (loadedLibs.contains(libraryName)) {
        return true;
    }

    loadedLibs.insert(libraryName);

    return translate(libraryName);
}

bool Translator::translatePlugin(const QString &pluginName, const QString& type)
{
    static QSet<QString> loadedPlugins;

    const QString fullName = type % QL1C('/') % pluginName;
    if (loadedPlugins.contains(fullName)) {
        return true;
    }

    loadedPlugins.insert(pluginName);
    return translate(pluginName, type);
}

static void loadSelfTranslation()
{
    Translator::translateLibrary(QLatin1String("graceful"));
}

Q_COREAPP_STARTUP_FUNCTION(loadSelfTranslation)
