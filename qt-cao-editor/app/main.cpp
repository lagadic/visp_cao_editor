/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan
****************************************************************************/

#include <QApplication>
#include <QCommandLineParser>

#include "mainwindow.h"
#include "scenemodifier.h"


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(app);

    MainWindow mainWin;

    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(200,207,200,255));
    QWidget *sceneContainer = QWidget::createWindowContainer(view);
    QSize screenSize = view->screen()->size();
    sceneContainer->setMinimumSize(QSize(200, 100));
    sceneContainer->setMaximumSize(screenSize);
    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    view->registerAspect(input);

     // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

    // Camera
    mainWin.cameraEntity = view->camera();

//    cameraEntity->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
    mainWin.cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    mainWin.cameraEntity->setPosition(QVector3D(20.0f, 10.0f, 0.0f));
    mainWin.cameraEntity->setUpVector(QVector3D(0, 1, 0));
    mainWin.cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(mainWin.cameraEntity->position());
    lightEntity->addComponent(lightTransform);

    // For camera controls
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(mainWin.cameraEntity);

    // Set root object of the scene
    view->setRootEntity(rootEntity);

    // Scenemodifier
    mainWin.modifier = new SceneModifier(rootEntity, sceneContainer);


    if (!parser.positionalArguments().isEmpty())
        mainWin.loadCaoFile(parser.positionalArguments().first());

    mainWin.setCentralWidget(sceneContainer);
    mainWin.resize(800, 600);
    mainWin.show();

    return app.exec();
}
