#pragma once

#include <QMainWindow>
#include "settings/Settings.h"
#include "assets/AssetImporter.h"
#include "engine/EngineWindow.h"
#include "widgets/Browser.h"
#include "widgets/Hierarchy.h"
#include "widgets/Inspector.h"
#include "assets/AssetDecodePopup.h"
#include "containers/memory.h"

class ToolBar;
QT_FORWARD_DECLARE_CLASS(QMenu)

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    typedef QMap<QString, QSize> CustomSizeHintMap;
    explicit MainWindow(const CustomSizeHintMap& customSizeHints, const QString& projectPath,
        QWidget* parent = Q_NULLPTR,
        Qt::WindowFlags flags = 0);

private:
    void setupToolBar();
    void setupMenuBar();

public slots:
    void moveClick(bool value);
    void rotateClick(bool value);
    void resizeClick(bool value);
    void playClick(bool value);

    void onAssetWorkStarted(QString sourceFilePath) const;
    void onAssetWorkStatusChange(QString sourceFilePath, QString msg) const;
    void onAssetWorkProgress(QString sourceFilePath, float progress) const;
    void onAssetWorkStopped(QString sourceFilePath) const;


public slots:
    void newScene();
    void saveScene();
    void loadScene();
/*    void actionTriggered(QAction *action);
    void saveLayout();
    void loadLayout();
    void switchLayoutDirection();
    void setDockOptions();

    void createDockWidget();
    void destroyDockWidget(QAction *action);

private:
    void setupToolBar();
    void setupMenuBar();
    void setupDockWidgets(const CustomSizeHintMap &customSizeHints);
    */
private:
    QString m_projectPath;

    Settings m_settings;
    engine::unique_ptr<AssetImporter> m_assetImporter;
    engine::unique_ptr<EngineWindow> m_engineWindow;
    engine::unique_ptr<Browser> m_browser;
    engine::unique_ptr<Hierarchy> m_hierarchy;
    engine::unique_ptr<Inspector> m_inspector;
    engine::unique_ptr<AssetDecodePopup> m_decodePopup;

    QList<QDockWidget*> m_dockWidgets;
    QList<QToolBar*> m_toolBars;

    engine::unique_ptr<QDialog> m_progressDialog;

    /*QList<ToolBar*> toolBars;
    QMenu *dockWidgetMenu;
    QMenu *mainWindowMenu;
    QList<QDockWidget *> extraDockWidgets;
    QMenu *destroyDockWidgetMenu;*/
};
