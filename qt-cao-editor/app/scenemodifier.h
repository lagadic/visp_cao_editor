/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan.

****************************************************************************/

#ifndef SCENEMODIFIER_H
#define SCENEMODIFIER_H

#include <QtCore/QObject>

#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>

#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>

#include <Qt3DInput/QKeyboardHandler>

#include <Qt3DRender/QMesh>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>


class SceneModifier : public QObject
{
    Q_OBJECT

public:
    explicit SceneModifier(Qt3DCore::QEntity *rootEntity);
    ~SceneModifier();

public slots:
    void enableCaoMesh(bool enabled);
    void mouseControls(Qt3DInput::QKeyEvent *event);
    void parse3DFile(QTextStream &input);

private slots:
    void handlePickerPress(Qt3DRender::QPickEvent *event);
    void createMesh(float* vertexMapData,int vertexNum);
    int primitiveType(QString &type);

private:
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_caoEntity;
    Qt3DCore::QEntity *m_cuboidEntity;
    QList<Qt3DRender::QObjectPicker *> *m_facePickers;
    Qt3DExtras::QPhongMaterial *caoMaterial;
    Qt3DRender::QGeometryRenderer *meshRenderer;
    Qt3DRender::QGeometry *geometry;
};

#endif // SCENEMODIFIER_H
