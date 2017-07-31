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

# define M_PI 3.14159

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity)
    : m_rootEntity(rootEntity)
    , m_facePickers(nullptr)
    , m_caoEntity(nullptr)
{
    Qt3DRender::QGeometryRenderer *line_mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(line_mesh);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

    QByteArray vertexBufferData;
    vertexBufferData.resize(6 * (3 + 3) * sizeof(float));

    // Vertices
    QVector3D v0(-100000.0f, 0.0f, 0.0f);
    QVector3D v1(100000.0f, 0.0f, 0.0f);
    QVector3D v2(0.0f, -100000.0f, 0.0f);
    QVector3D v3(0.0f, 100000.0f, 0.0f);
    QVector3D v4(0.0f, 0.0f, -100000.0f);
    QVector3D v5(0.0f, 0.0f, 100000.0f);

    // Colors
    QVector3D red(1.0f, 0.0f, 0.0f);
    QVector3D green(0.0f, 1.0f, 0.0f);
    QVector3D blue(0.0f, 0.0f, 1.0f);

    QVector<QVector3D> vertices = QVector<QVector3D>()
       << v0 << red
       << v1 << red
       << v2 << green
       << v3 << green
       << v4 << blue
       << v5 << blue;

    float *rawVertexArray = reinterpret_cast<float *>(vertexBufferData.data());
    int idx = 0;

    Q_FOREACH (const QVector3D &v, vertices) {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
    }

    QByteArray indexBufferData;
    indexBufferData.resize(6 * 3 * sizeof(ushort));
    ushort *rawIndexArray = reinterpret_cast<ushort *>(indexBufferData.data());

    // X-axis
    rawIndexArray[0] = 0;
    rawIndexArray[1] = 1;
    // Y-axis
    rawIndexArray[2] = 2;
    rawIndexArray[3] = 3;
    // Z-axis
    rawIndexArray[4] = 4;
    rawIndexArray[5] = 5;

    vertexDataBuffer->setData(vertexBufferData);
    indexDataBuffer->setData(indexBufferData);

    // Attributes
    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexDataBuffer);
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setByteOffset(0);
    positionAttribute->setByteStride(6 * sizeof(float));
    positionAttribute->setCount(6);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *colorAttribute = new Qt3DRender::QAttribute();
    colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    colorAttribute->setBuffer(vertexDataBuffer);
    colorAttribute->setDataType(Qt3DRender::QAttribute::Float);
    colorAttribute->setDataSize(3);
    colorAttribute->setByteOffset(3 * sizeof(float));
    colorAttribute->setByteStride(6 * sizeof(float));
    colorAttribute->setCount(6);
    colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());

    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute();
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexDataBuffer);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedShort);
    indexAttribute->setDataSize(1);
    indexAttribute->setByteOffset(0);
    indexAttribute->setByteStride(0);
    indexAttribute->setCount(6);

    geometry->addAttribute(positionAttribute);
    geometry->addAttribute(colorAttribute);
    geometry->addAttribute(indexAttribute);

    line_mesh->setInstanceCount(1);
    line_mesh->setIndexOffset(0);
    line_mesh->setFirstInstance(0);
    line_mesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    line_mesh->setGeometry(geometry);
    line_mesh->setVertexCount(6);

    // Material
    Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial(m_rootEntity);

    // Transform
    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform;

    // Custom mesh TetraHedron
    Qt3DCore::QEntity *m_axisEntity = new Qt3DCore::QEntity(m_rootEntity);

    m_axisEntity->addComponent(line_mesh);
    m_axisEntity->addComponent(material);
    m_axisEntity->addComponent(transform);

    // Cuboid shape data
    Qt3DExtras::QCuboidMesh *cuboid = new Qt3DExtras::QCuboidMesh();
    cuboid->setXYMeshResolution(QSize(2, 2));
    cuboid->setYZMeshResolution(QSize(2, 2));
    cuboid->setXZMeshResolution(QSize(2, 2));
    // CuboidMesh Transform
    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(1.0f);
    cuboidTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

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
    if(m_cuboidEntity->isEnabled())
        m_cuboidEntity->setEnabled(false);

    QString m_template =  "";
    int idx = 0;
    float* vertexRawData;
    int* lineRawData;
    QList<int> facelineRawData;
    QList<int> facelpointRawData;
    int vertexNum = 0;
    int lineNum = 0;
    int facelineNum = 0;
    int facepointNum = 0;
    int cylinderNum = 0;
    QList<QVector3D> vertices;

    while(!input.atEnd())
    {
        QString line = input.readLine();
        QStringList fields = line.split("#");
        QStringList data = {};
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

        if(fields[0].isEmpty())
            continue;

        if(m_template == "3D_PTS")
        {
            data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
                vertexNum = data[0].toInt() * 3;
                vertexRawData = new float[vertexNum];
            }
            else if(data.count() >= 3)
            {
                vertexRawData[idx++] = data[0].toFloat();vertexRawData[idx++] = data[1].toFloat();vertexRawData[idx++] = data[2].toFloat();
                QVector3D v(data[0].toFloat(),data[1].toFloat(),data[2].toFloat());
                vertices.append(v);
            }
        }

        else if(m_template == "3D_LNS")
        {
            data = fields[0].split(" ");
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

        else if(m_template == "3D_F_LNS")
        {
            data = fields[0].split(" ");
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

        else if(m_template == "3D_F_PTS")
        {
            data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
                facepointNum = 0;
            }
            else if(data.count() > 3)
            {
                facepointNum += data[0].toInt();
                for(int i=1;i<=data[0].toInt();i++)
                    facelpointRawData.append(data[i].toInt());
            }
        }

        else if(m_template == "3Dcylinders")
        {
            data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
                cylinderNum = 0;
            }
            else if(data.count() == 3)
            {
                int indx1 = data[0].toInt()*3, indx2 = data[1].toInt()*3;
                QVector3D axis_1(vertexRawData[indx1], vertexRawData[indx1+1], vertexRawData[indx1+2]);
                QVector3D axis_2(vertexRawData[indx2], vertexRawData[indx2+1], vertexRawData[indx2+2]);
                this->createCylinder(axis_1, axis_2, data[2].toFloat());
            }
        }
    }

    float* vertexMapData;
    vertexMapData = (facelpointRawData.isEmpty() ? (facelineRawData.isEmpty() ? (lineRawData ? new float[lineNum*3] : vertexRawData) : new float[facelineNum*6]) : new float[facepointNum*3]);
    vertexNum = (facelpointRawData.isEmpty() ? (facelineRawData.isEmpty() ? (lineRawData ? lineNum*3 : vertexNum) : facelineNum*6): facepointNum*3);

    if(!facelpointRawData.isEmpty())
        for(int i = 0; i < facepointNum; i++)
        {
            int index = facelpointRawData[i]*3;
            vertexMapData[i*3] = vertexRawData[index];
            vertexMapData[i*3+1] = vertexRawData[index+1];
            vertexMapData[i*3+2] = vertexRawData[index+2];
        }

    else if(lineRawData && facelineRawData.isEmpty())
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

    if(!facelpointRawData.isEmpty() && !facelineRawData.isEmpty())
        this->createMesh(vertexMapData, vertexNum);
}

void SceneModifier::createMesh(float* vertexMapData,int vertexNum)
{

    // Mesh Creation
    meshRenderer = new Qt3DRender::QGeometryRenderer;
    geometry = new Qt3DRender::QGeometry(meshRenderer);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);

    QByteArray ba1;
    int bufferSize = vertexNum * sizeof(float);
    ba1.resize(bufferSize);
    memcpy(ba1.data(), reinterpret_cast<const char*>(vertexMapData), bufferSize);
    vertexDataBuffer->setData(ba1);

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

void SceneModifier::createCylinder(QVector3D axis_1, QVector3D axis_2, float radius)
{
    QVector3D main_axis(axis_1[0] - axis_2[0], axis_1[1] - axis_2[1], axis_1[2] - axis_2[2]);
    QVector3D mid_point((axis_1[0] + axis_2[0])/2, (axis_1[1] + axis_2[1])/2, (axis_1[2] + axis_2[2])/2);
    QVector3D main_axis_norm = main_axis.normalized();

    Qt3DExtras::QCylinderMesh *cylinder = new Qt3DExtras::QCylinderMesh();
    cylinder->setRadius(radius);
    cylinder->setLength(qSqrt(qPow(main_axis[0],2) + qPow(main_axis[1],2) + qPow(main_axis[2],2)));
    cylinder->setRings(100);
    cylinder->setSlices(20);

    // CylinderMesh Transform
    Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform();

    cylinderTransform->setRotationX(qAcos(QVector3D::dotProduct(main_axis_norm,QVector3D(1.0f, 0.0f, 0.0f)))*180/M_PI);
    cylinderTransform->setRotationY(qAcos(QVector3D::dotProduct(main_axis_norm,QVector3D(0.0f, 1.0f, 0.0f)))*180/M_PI);
    cylinderTransform->setRotationZ(qAcos(QVector3D::dotProduct(main_axis_norm,QVector3D(0.0f, 0.0f, 1.0f)))*180/M_PI);
    qInfo() << cylinderTransform->rotationX() << cylinderTransform->rotationY() << cylinderTransform->rotationZ() << main_axis[2];
    //    cylinderTransform->setRotationX(qAcos(main_axis[2]/cylinder->length())*180/M_PI); //theta
    //    cylinderTransform->setRotationY(qAtan2(main_axis[1],main_axis[0])*180/M_PI);      //phi

    //    cylinderTransform->setRotation(QQuaternion::fromAxisAndAngle(main_axis.normalized(),0.0f));
    cylinderTransform->setTranslation(mid_point);

    Qt3DExtras::QPhongMaterial *cylinderMaterial = new Qt3DExtras::QPhongMaterial();
    cylinderMaterial->setDiffuse(QColor(QRgb(0x928327)));

    Qt3DCore::QEntity *m_cylinderEntity = new Qt3DCore::QEntity(m_rootEntity);

//    Qt3DRender::QObjectPicker *picker2 = new Qt3DRender::QObjectPicker(m_cylinderEntity);
//    picker2->setHoverEnabled(false);
//    picker2->setObjectName(QStringLiteral("__internal object picker ") + m_cylinderEntity->objectName());
//    m_facePickers->append(picker2);

    m_cylinderEntity->addComponent(cylinder);
    m_cylinderEntity->addComponent(cylinderMaterial);
    m_cylinderEntity->addComponent(cylinderTransform);
//    m_cylinderEntity->addComponent(m_facePickers->at(1));
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
           qInfo() << pressedEntity->isEnabled() << " STATE" ;
           pressedEntity->setEnabled(!pressedEntity->isEnabled());
            caoMaterial->setDiffuse(QColor(200,100,12,255));
        }
    }
}

void SceneModifier::mouseControls(Qt3DInput::QKeyEvent *event)
{
    qInfo() << event->key();
}
