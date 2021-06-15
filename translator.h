#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "globals.h"
#include <QStringList>


namespace graceful
{
class GRACEFUL_API Translator
{
public:
    /**
     * @brief
     * Returns a list of paths that the application will search translations files.
     */
    static QStringList translationSearchPaths();

    /**
     * Sets the list of directories to search translations. All existing paths
     * will be deleted and the path list will consist of the paths given in paths.
     */
    static void setTranslationSearchPaths(const QStringList &paths);

    /**
     * @brief
     * Loads translations for application. If applicationName is not specified,
     * then basename of QCoreApplication::applicationFilePath() is used.
     * Returns true if the translation is successfully loaded; otherwise returns false.
     */
    static bool translateApplication(const QString &applicationName = QString());

    /**
     * @brief
     * Loads translations for application. If applicationName is not specified,
     * then basename of QCoreApplication::applicationFilePath() is used.
     * Returns true if the translation is successfully loaded; otherwise returns false.
     */
    static bool translateLibrary(const QString &libraryName = QString());

    static bool translatePlugin(const QString &pluginName, const QString& type);
};

}



#endif // TRANSLATOR_H
