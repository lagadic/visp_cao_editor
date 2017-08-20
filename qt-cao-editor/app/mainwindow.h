/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <Qt3DCore/qaspectengine.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCommandLinkButton>
#include <QtGui/QScreen>

#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QKeyboardHandler>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QPointLight>

#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QFirstPersonCameraController>

#include "scenemodifier.h"
#include "xmleditor.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void loadCaoFile(const QString &fileName);
    SceneModifier *modifier;
    Qt3DRender::QCamera *cameraEntity;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    bool removeConfirm();
    void about();
    void parseXML();
    void qcameraDialog();
    void updateCameraProjection();
    float getCameraProjection(QString line, QString op_tag, QString cl_tag);
#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif

private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void blenderFrameStateChanged(int state);
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QString curFile;

    XmlEditor *xmlWin;

    QDialog *dialog;
    QFormLayout *form;
    QList<QLineEdit *> qcamera_fields;
    bool useBlenderFrame;
};
//! [0]

#endif
