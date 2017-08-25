/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan
****************************************************************************/

#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow() : useBlenderFrame(false)
{
    createActions();
    createStatusBar();

    readSettings();

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &MainWindow::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    xmlWin = new XmlEditor();
    xmlWin->show();
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadCaoFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

bool MainWindow::removeConfirm()
{
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("Deleting all the elements in the Scene.."),
                               QMessageBox::Apply | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Apply:
        modifier->removeSceneElements();
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::qcameraDialog()
{
    dialog = new QDialog(this);
    form = new QFormLayout(dialog);
    form->addRow(new QLabel("QCamera Settings"));

    qcamera_fields.clear();
    QPushButton *openXML = new QPushButton("Choose XML",dialog);
//    openXML->setGeometry(QRect(QPoint(30, 30),
//                               QSize(20, 50)));
    form->addRow(openXML);
    QObject::connect(openXML, SIGNAL (released()), this, SLOT(parseXML()));


    if (dialog->exec() == QDialog::Accepted)
    {
    }
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}

void MainWindow::createActions()
{
    QMenu *caoMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *caoToolBar = addToolBar(tr("File"));


    QAction *openAct = new QAction(QIcon(":/images/icon_import_cao.png"), tr("&Import ViSP .cao"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Import a .cao file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    caoMenu->addAction(openAct);
    caoToolBar->addAction(openAct);

    QCheckBox *blenderFrameCB = new QCheckBox(tr("Use Blender frame"));
    blenderFrameCB->setStatusTip(tr("Use Blender frame when importing ViSP .cao"));
    connect(blenderFrameCB, &QCheckBox::stateChanged, this, &MainWindow::blenderFrameStateChanged);
    caoToolBar->addWidget(blenderFrameCB);


    QAction *saveAct = new QAction(QIcon::fromTheme("document-save", QIcon(":/images/save.png")), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    caoMenu->addAction(saveAct);
    caoToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = caoMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    caoToolBar->addAction(saveAsAct);

    caoToolBar->addSeparator();

    QAction *clearScene = new QAction(QIcon(":/images/icon_clean.svg"), tr("&Clear Scene"), this);
    clearScene->setStatusTip(tr("Clear Scene"));
    connect(clearScene, &QAction::triggered, this, &MainWindow::removeConfirm);
    caoToolBar->addAction(clearScene);

    QToolBar *xmlToolBar = addToolBar(tr("XML"));

    QAction *newAct = new QAction(QIcon(":/images/icon_xml.png"), tr("&Open XML Editor"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Open XML Editor"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    caoMenu->addAction(newAct);
    xmlToolBar->addAction(newAct);

    QAction *qCam = new QAction(QIcon(":/images/icon_qcamera.svg"), tr("&QCamera-XML Settings"), this);
    qCam->setStatusTip(tr("QCamera-XML Settings"));
    connect(qCam, &QAction::triggered, this, &MainWindow::qcameraDialog);
    xmlToolBar->addAction(qCam);

    QToolBar *pointToolBar = addToolBar(tr("Point Select"));

//    QCheckBox *displayModelsAct = new QCheckBox(tr("Display Objects"));
//    connect(displayModelsAct, &QCheckBox::stateChanged, this, &MainWindow::toggleModels);
//    pointToolBar->addWidget(displayModelsAct);

    QCheckBox *displayInitAct = new QCheckBox(tr("Display Vertices"));
    connect(displayInitAct, &QCheckBox::stateChanged, this, &MainWindow::toggleVertices);
    pointToolBar->addWidget(displayInitAct);

    QAction *getinitAct = new QAction(QIcon(":/images/icon_init_points.png"), tr("&Reset Init Points"), this);
    getinitAct->setStatusTip(tr("Reset Init Points"));
    connect(getinitAct, &QAction::triggered, this, &MainWindow::resetInitPoints);
    pointToolBar->addAction(getinitAct);

    QAction *saveinitAct = new QAction(QIcon(":/images/icon_save_init.png"), tr("&Save Init Points"), this);
    saveinitAct->setStatusTip(tr("Save Init Points"));
    connect(saveinitAct, &QAction::triggered, this, &MainWindow::saveInitPoints);
    pointToolBar->addAction(saveinitAct);

    caoMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = caoMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);

    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));


    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::blenderFrameStateChanged(int state)
{
    useBlenderFrame = (state == Qt::CheckState::Checked);
}

void MainWindow::toggleModels(int state)
{
    Q_FOREACH (Qt3DCore::QEntity *entity, modifier->scene_entities)
    {
        entity->setEnabled((state == Qt::CheckState::Checked));
    }
}

void MainWindow::toggleVertices(int state)
{
    Q_FOREACH (Qt3DCore::QEntity *entity, modifier->scene_points)
    {
        entity->setEnabled((state == Qt::CheckState::Checked));
    }
}

void MainWindow::resetInitPoints()
{
    Q_FOREACH (Qt3DCore::QEntity *entity, modifier->scene_points)
    {
        Qt3DExtras::QPhongMaterial *sphereColor = new Qt3DExtras::QPhongMaterial();
        sphereColor->setDiffuse(QColor(255,0,0,255));
        entity->addComponent(sphereColor);
        entity->setObjectName("");
    }
    modifier->init_points->clear();
}

bool MainWindow::saveInitPoints()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    const QString fileName = dialog.selectedFiles().first();
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
            QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName),
                                      file.errorString()));
            return false;
    }
    QTextStream out(&file);
    out << modifier->init_points->length() << "\n";

    for(unsigned int i=0;i<modifier->init_points->length();i++)
    {
        QVector3D v = modifier->init_points->at(i);
        out << v.x() << " " << v.y() << " " << v.z() << "\n";
    }

    file.close();
    return true;
}

bool MainWindow::maybeSave()
{
    //Check if any modifications
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadCaoFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
    modifier->parse3DFile(in, useBlenderFrame);

    file.close();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

float MainWindow::getCameraProjection(QString &line, QString op_tag, QString cl_tag)
{
    return line.split(op_tag)[1].split(cl_tag)[0].toFloat();
}

bool MainWindow::verifyXmlTag(const QString tag, QString &line)
{
    return (line.split(tag).length() == 2);
}

void MainWindow::formAddField(const QString tag, const QString text)
{
    QLineEdit *lineEdit = new QLineEdit(dialog);
    lineEdit->setText(text);
    form->addRow(tag, lineEdit);
    qcamera_fields << lineEdit;
}

void MainWindow::parseXML()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    curXML = fileName;

    float u0 = 0, v0 = 0, px = 0, py = 0, near_clipping = 0, far_clipping = 0;

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Application"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
            return;
        }
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QString line = in.readLine();

            if(verifyXmlTag("<u0>",line)) {u0 = getCameraProjection(line, "<u0>","</u0>");}

            else if(verifyXmlTag("<v0>",line)) {v0 = getCameraProjection(line, "<v0>", "</v0>");}

            else if(verifyXmlTag("<px>",line)) {px = getCameraProjection(line, "<px>", "</px>");}

            else if(verifyXmlTag("<py>",line)) {py = getCameraProjection(line, "<py>", "</py>");}

            else if(verifyXmlTag("<near_clipping>",line)) {near_clipping = getCameraProjection(line, "<near_clipping>", "</near_clipping>");}

            else if(verifyXmlTag("<far_clipping>",line)) {far_clipping = getCameraProjection(line, "<far_clipping>", "</far_clipping>");}

        }
        file.close();
    }

    this->formAddField(QString("u0 "), QString::number(u0));
    this->formAddField(QString("v0 "), QString::number(v0));
    this->formAddField(QString("px "), QString::number(px));
    this->formAddField(QString("py "), QString::number(py));
    this->formAddField(QString("near_clipping "), QString::number(near_clipping));
    this->formAddField(QString("far_clipping "), QString::number(far_clipping));

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    form->addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    QPushButton *saveSett = new QPushButton("OK",dialog);
    form->addRow(saveSett);
    QObject::connect(saveSett, SIGNAL (released()), this, SLOT(updateCameraProjection()));
}

float MainWindow::qcameraFieldVal(const int index)
{
    return qcamera_fields.at(index)->text().toFloat();
}

void MainWindow::updateCameraProjection()
{
    Qt3DRender::QCameraLens *lens =  cameraEntity->lens();
    float fov = (!qcamera_fields.at(3)->text().isEmpty() ?
                     360*qAtan(qcameraFieldVal(1)/qcameraFieldVal(3))/M_PI : lens->fieldOfView());

    float aspectRatio = (!qcamera_fields.at(1)->text().isEmpty() ?
                             (qcameraFieldVal(0)*qcameraFieldVal(3))/(qcameraFieldVal(1)*qcameraFieldVal(2)) : lens->aspectRatio());

    float nearPlane = (!qcamera_fields.at(4)->text().isEmpty() ?
                             qcameraFieldVal(4) : lens->nearPlane());

    float farPlane = (!qcamera_fields.at(5)->text().isEmpty() ?
                             qcameraFieldVal(5) : lens->farPlane());

    cameraEntity->lens()->setPerspectiveProjection(fov, aspectRatio, nearPlane, farPlane);

    QString fileName = curXML;
    QFile fileread(fileName);
    if (!fileread.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), fileread.errorString()));
        return;
    }
    QTextStream in(&fileread);

    QFile filewrite("temp.xml");
    if (!filewrite.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), filewrite.errorString()));
        return;
    }

    QTextStream out(&filewrite);

    while(!in.atEnd())
    {
        QString line = in.readLine();

        if(verifyXmlTag("<u0>",line))
            out << "<u0>" << QString::number(qcameraFieldVal(0)) << "</u0>" << "\n";

        else if(verifyXmlTag("<v0>",line))
            out << "<v0>" << QString::number(qcameraFieldVal(1)) << "</v0>" << "\n";

        else if(verifyXmlTag("<px>",line))
            out << "<px>" << QString::number(qcameraFieldVal(2)) << "</px>" << "\n";

        else if(verifyXmlTag("<py>",line))
            out << "<py>" << QString::number(qcameraFieldVal(3)) << "</py>" << "\n";

        else if(verifyXmlTag("<near_clipping>",line))
            out << "<near_clipping>" << QString::number(qcameraFieldVal(4)) << "</near_clipping>" << "\n";

        else if(verifyXmlTag("<far_clipping>",line))
            out << "<far_clipping>" << QString::number(qcameraFieldVal(5)) << "</far_clipping>" << "\n";
        else
            out << line << "\n";
    }

    fileread.close();
    fileread.remove();

    filewrite.rename("temp.xml", fileName);
    filewrite.close();
    dialog->accept();
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);

    QStringList data;


    // Header
    out << "# ViSP Qt Editor CAO File\nV1\n";
    // 3D points
    out << "# 3D points\n" << modifier->vertices->length() << "\n";
    for(int i=0;i<modifier->vertices->length();i++)
    {
        QVector3D v = modifier->vertices->at(i);
        out << v.x() << " " << v.y() << " " << v.z() << "\n";
    }

    // 3D lines
    out << "# 3D lines\n" << modifier->lineRawData->length() << "\n";
    for(int i=0;i<modifier->lineRawData->length();i++)
    {
        QVector2D l = modifier->lineRawData->at(i);
        data = modifier->line_param->at(i).split("+");
        out << l.x() << " " << l.y() << " " << data[1] << "\n";
    }

    // Faces from 3D lines
    out << "# Faces from 3D lines\n" << modifier->facelineRawData.length() << "\n";
    for(int i=0;i<modifier->facelineRawData.length();i++)
    {
        QList<int> faceMap_list = modifier->facelineRawData.at(i);
        QString faceMap_str;
        for(int i=0; i<faceMap_list.length(); i++)
        {
            faceMap_str += QString::number(faceMap_list[i]);
            faceMap_str += " " ;
        }
        data = modifier->faceline_param->at(i).split("+");
        out << faceMap_str << data[1] << "\n";
    }

    // Faces from 3D points
    out << "# Faces from 3D points\n" << modifier->facelpointRawData.length() << "\n";
    for(int i=0;i<modifier->facelpointRawData.length();i++)
    {
        QList<int> faceMap_list = modifier->facelpointRawData.at(i);
        QString faceMap_str;
        for(int i=0; i<faceMap_list.length(); i++)
        {
            faceMap_str += QString::number(faceMap_list[i]);
            faceMap_str += " " ;
        }
        data = modifier->facepoint_param->at(i).split("+");
        out << faceMap_str << data[1] << "\n";
    }

    // 3D cylinders
    out << "# 3D cylinders\n" << modifier->cylinder->length() << "\n";
    for(int i=0;i<modifier->cylinder->length();i++)
    {
        QVector3D cyl = modifier->cylinder->at(i);
        data = modifier->cylinder_param->at(i).split("+");
        out << cyl.x() << " " << cyl.y() << " " << cyl.z() << " " << data[1] << "\n";
    }

    // 3D circles
    out << "# 3D circles\n" << modifier->circle->length() << "\n";
    for(int i=0;i<modifier->circle->length();i++)
    {
        QVector4D cir = modifier->circle->at(i);
        data = modifier->circle_param->at(i).split("+");
        out << cir.w() << " " << cir.x() << " " << cir.y() << " " << cir.z() << " " << data[1] << "\n";
    }
    file.close();
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}



void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.cao";
    setWindowFilePath("ViSP CAO Editor - " + shownName);
}



QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!maybeSave())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
        // if (textEdit->document()->isModified())
            // save();
    }
}
#endif
