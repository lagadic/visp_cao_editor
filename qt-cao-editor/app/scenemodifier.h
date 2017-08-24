/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan.

****************************************************************************/

#ifndef SCENEMODIFIER_H
#define SCENEMODIFIER_H

#include <QtCore/QObject>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <QtCore/QtMath>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>

#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPerVertexColorMaterial>

#include <Qt3DInput/QKeyboardHandler>

#include <Qt3DRender/QMesh>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>


#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>

class SceneModifier : public QObject
{
    Q_OBJECT

public:
    explicit SceneModifier(Qt3DCore::QEntity *rootEntity, QWidget *parentWidget);
    ~SceneModifier();

    QList<QVector3D>* vertices;
    QList<QVector2D>* lineRawData;
    QList<QList<int>> facelineRawData;
    QList<QList<int>> facelpointRawData;
    QList<QVector3D>* cylinder;
    QList<QVector4D>* circle;

    QList<QString>* line_param;
    QList<QString>* faceline_param;
    QList<QString>* facepoint_param;
    QList<QString>* cylinder_param;
    QList<QString>* circle_param;

    Qt3DRender::QObjectPicker *createObjectPickerForEntity(Qt3DCore::QEntity *entity);

public slots:
    void mouseControls(Qt3DInput::QKeyEvent *event);
    void parse3DFile(QTextStream &input, const bool useBlenderFrame);
    void removeSceneElements();

private slots:
    void handlePickerPress(Qt3DRender::QPickEvent *event);
    void createCylinder(const QVector3D &axis_1, const QVector3D &axis_2,
                        const unsigned int index, const float radius, const QString &load_param);
    void createCircle(const QVector3D &circum_1, const QVector3D &circum_2, const QVector3D &center,
                      const unsigned int index, const float radius, const QString &load_param);
    void createLines(const QVector3D &v0, const QVector3D &v1,
                     const unsigned int index, const bool axis, const QString &lod_param);
    void getLineLength();

    void formAddField(QDialog *dialog, QFormLayout *form,
                      const QString tag, const QString text);
    int primitiveType(const QString &type);
    QString getLodParameters(const QStringList &data, const QString &type,
                          const unsigned int idx1, const unsigned int idx2);

private:
    bool handleMousePress(QMouseEvent *event);

    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_lineEntity;
    Qt3DCore::QEntity *m_cuboidEntity;
    Qt3DCore::QEntity *m_cylinderEntity;
    Qt3DCore::QEntity *m_circleEntity;
    Qt3DExtras::QPhongMaterial *caoMaterial;
    QWidget *m_parentWidget;

    Qt::MouseButton m_mouseButton;

    QList<Qt3DCore::QEntity *> scene_entities;
    QList<QLineEdit *> fields;
};

#endif // SCENEMODIFIER_H
