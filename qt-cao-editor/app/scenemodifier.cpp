/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan
****************************************************************************/

#include "scenemodifier.h"

#include <QtCore/QDebug>

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QFileDialog>
#include <QMaterial>
#include <QEffect>
#include <QTechnique>
#include <QRenderPass>
#include <QShaderProgram>

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity)
    : m_rootEntity(rootEntity)
    , m_facePickers(nullptr)
    , m_caoEntity(nullptr)
{
    // Cuboid shape data
    Qt3DExtras::QCuboidMesh *cuboid = new Qt3DExtras::QCuboidMesh();
    cuboid->setXYMeshResolution(QSize(2, 2));
    cuboid->setYZMeshResolution(QSize(2, 2));
    cuboid->setXZMeshResolution(QSize(2, 2));
    // CuboidMesh Transform
    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(1.0f);
    cuboidTransform->setTranslation(QVector3D(0.0f, -2.0f, 0.0f));

    Qt3DExtras::QPhongMaterial *cuboidMaterial = new Qt3DExtras::QPhongMaterial();
    cuboidMaterial->setDiffuse(QColor(100,100,100,255));

    //Cuboid
    m_cuboidEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_cuboidEntity->addComponent(cuboid);
    m_cuboidEntity->addComponent(cuboidMaterial);
    m_cuboidEntity->addComponent(cuboidTransform);
//    m_cuboidEntity->setEnabled(false);
}

SceneModifier::~SceneModifier()
{
    delete m_facePickers;
    delete m_cuboidEntity;
    delete m_caoEntity;
}

int SceneModifier::primitiveType(QString &type)
{
    const QStringList t_defined = {"3DPoints","3DLines","Facesfrom3Dlines","Facesfrom3Dpoints","3Dcylinders","3Dcircles"};
    for(int i=1;i<=6;i++)
        if(!type.compare(t_defined[i-1], Qt::CaseInsensitive))
            return i;
    return 0;
}

void SceneModifier::parse3DFile(QTextStream &input)
{
    if(m_cuboidEntity)
        delete m_cuboidEntity;

    QString m_template =  "";
    int idx = 0;
    float* vertexRawData;
    int* lineRawData;
    QList<int> facelineRawData;
    int vertexNum = 0;
    int lineNum = 0;
    int facelineNum = 0;

    while(!input.atEnd())
    {
        QString line = input.readLine();
        QStringList fields = line.split("#");

        if(fields.count() == 2)
        {
            QString type = fields[1].replace(" ","");
            switch(primitiveType(type))
            {
                case 1 : m_template = "3D_PTS";
                         break;
                case 2 : m_template = "3D_LNS";
                         break;
                case 3 : m_template = "3D_F_LNS";
                         break;
                case 4 : m_template = "3D_F_PTS";
                         break;
                case 5 : m_template = "3Dcylinders";
                         break;
                case 6 : m_template = "3Dcircles";
                         break;
            }
        }

        if(m_template == "3D_PTS" && !fields[0].isEmpty())
        {
            QStringList data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
                vertexNum = data[0].toInt() * 3;
                vertexRawData = new float[vertexNum];
                qInfo() << vertexNum << fields;
            }
            else if(data.count() >= 3)
            {
                vertexRawData[idx++] = data[0].toFloat();vertexRawData[idx++] = data[1].toFloat();vertexRawData[idx++] = data[2].toFloat();
            }
        }

        else if(m_template == "3D_LNS" && !fields[0].isEmpty())
        {
            QStringList data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
                lineNum = data[0].toInt()*2;
                lineRawData = new int[lineNum];
            }
            else if(data.count() >= 2)
            {
                lineRawData[idx++] = data[0].toInt();lineRawData[idx++] = data[1].toInt();
            }
        }

        else if(m_template == "3D_F_LNS" && !fields[0].isEmpty())
        {
            QStringList data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
                facelineNum = 0;
            }
            else if(data.count() > 3)
            {
                facelineNum += data[0].toInt();
                for(int i=1;i<=data[0].toInt();i++)
                    facelineRawData.append(data[i].toInt());
            }
        }
    }

    float* vertexMapData;
    vertexMapData = (facelineRawData.isEmpty() ? (lineRawData ? new float[lineNum*3] : vertexRawData) : new float[facelineNum*6]);
    vertexNum = (facelineRawData.isEmpty() ? (lineRawData ? lineNum*3 : vertexNum) : facelineNum*6);

    if(lineRawData && facelineRawData.isEmpty())
        for(int i = 0; i < lineNum; i++)
        {
            int index = lineRawData[i]*3;
            vertexMapData[i*3] = vertexRawData[index];
            vertexMapData[i*3+1] = vertexRawData[index+1];
            vertexMapData[i*3+2] = vertexRawData[index+2];
        }

    else
        for(int i = 0; i < facelineNum; i++)
        {
            int index = lineRawData[facelineRawData[i]*2]*3;
            vertexMapData[i*6] = vertexRawData[index];vertexMapData[i*6+1] = vertexRawData[index+1];vertexMapData[i*6+2] = vertexRawData[index+2];
            index = lineRawData[facelineRawData[i]*2+1]*3;
            vertexMapData[i*6+3] = vertexRawData[index];vertexMapData[i*6+4] = vertexRawData[index+1];vertexMapData[i*6+5] = vertexRawData[index+2];
        }

    this->createMesh(vertexMapData, vertexNum);
}

void SceneModifier::createMesh(float* vertexMapData,int vertexNum)
{

    // Mesh Creation
    meshRenderer = new Qt3DRender::QGeometryRenderer;
    geometry = new Qt3DRender::QGeometry(meshRenderer);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);


    QByteArray ba;
    int bufferSize = vertexNum * sizeof(float);
    ba.resize(bufferSize);
    memcpy(ba.data(), reinterpret_cast<const char*>(vertexMapData), bufferSize);
    vertexDataBuffer->setData(ba);

    int stride = 3 * sizeof(float);

    // Attributes
    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexDataBuffer);
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setByteOffset(0);
    positionAttribute->setByteStride(stride);
    positionAttribute->setCount(vertexNum);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    geometry->addAttribute(positionAttribute);

    meshRenderer->setInstanceCount(1);
    meshRenderer->setIndexOffset(0);
    meshRenderer->setFirstInstance(0);
    meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    meshRenderer->setGeometry(geometry);
    meshRenderer->setVertexCount(vertexNum/3);

    // Mesh Transform
    Qt3DCore::QTransform *caoTransform = new Qt3DCore::QTransform();
    caoTransform->setScale(2.0f);
//    caoTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 25.0f));
    caoTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    caoMaterial = new Qt3DExtras::QPhongMaterial();
    caoMaterial->setDiffuse(QColor(QRgb(0xbeb32b)));

    Qt3DCore::QEntity *m_caoEntity = new Qt3DCore::QEntity(m_rootEntity);

    m_facePickers = new QList<Qt3DRender::QObjectPicker *>();
    Qt3DRender::QObjectPicker *picker1 = new Qt3DRender::QObjectPicker(m_caoEntity);
    picker1->setHoverEnabled(true);
    picker1->setObjectName(QStringLiteral("__internal object picker ") + m_caoEntity->objectName());
    m_facePickers->append(picker1);
    m_caoEntity->addComponent(meshRenderer);
    m_caoEntity->addComponent(caoMaterial);
//    m_caoEntity->addComponent(caoTransform);
    m_caoEntity->addComponent(m_facePickers->at(0));
    m_caoEntity->setEnabled(true);
    connect(m_facePickers->at(0), &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePickerPress);
}

void SceneModifier::enableCaoMesh(bool enabled)
{
    m_caoEntity->setEnabled(enabled);
}

void SceneModifier::handlePickerPress(Qt3DRender::QPickEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        if (pressedEntity && pressedEntity->isEnabled())
        {
//           pressedEntity->setEnabled(!pressedEntity->isEnabled());
            caoMaterial->setDiffuse(QColor(200,100,12,255));
        }
    }
}

void SceneModifier::mouseControls(Qt3DInput::QKeyEvent *event)
{
    qInfo() << event->key();
}
