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
    , m_circleEntity(nullptr)
    , m_cylinderEntity(nullptr)
{
    // World Axis
    Qt3DRender::QGeometryRenderer *line_mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(line_mesh);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

    QByteArray vertexBufferData;
    vertexBufferData.resize(6 * (3 + 3) * sizeof(float));

    QVector<QVector3D> vertices = QVector<QVector3D>()
       << QVector3D (-100000.0f, 0.0f, 0.0f) << QVector3D (1.0f, 0.0f, 0.0f)
       << QVector3D (100000.0f, 0.0f, 0.0f)  << QVector3D (1.0f, 0.0f, 0.0f)
       << QVector3D (0.0f, -100000.0f, 0.0f) << QVector3D (0.0f, 1.0f, 0.0f)
       << QVector3D (0.0f, 100000.0f, 0.0f)  << QVector3D (0.0f, 1.0f, 0.0f)
       << QVector3D (0.0f, 0.0f, -100000.0f) << QVector3D (0.0f, 0.0f, 1.0f)
       << QVector3D (0.0f, 0.0f, 100000.0f)  << QVector3D (0.0f, 0.0f, 1.0f);

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

    // Axis Entity
    Qt3DCore::QEntity *m_axisEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_axisEntity->addComponent(line_mesh);
    m_axisEntity->addComponent(material);

    // Cuboid shape data
    Qt3DExtras::QCuboidMesh *cuboid = new Qt3DExtras::QCuboidMesh();
    cuboid->setXYMeshResolution(QSize(2, 2));
    cuboid->setYZMeshResolution(QSize(2, 2));
    cuboid->setXZMeshResolution(QSize(2, 2));
    // CuboidMesh Transform
    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(1.0f);
    cuboidTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    caoMaterial = new Qt3DExtras::QPhongMaterial();
    caoMaterial->setDiffuse(QColor(QRgb(0xbeb32b)));

    //Cuboid
    m_cuboidEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_cuboidEntity->addComponent(cuboid);
    m_cuboidEntity->addComponent(caoMaterial);
    m_cuboidEntity->addComponent(cuboidTransform);
}

SceneModifier::~SceneModifier()
{
    delete m_facePickers;
    delete m_cuboidEntity;
    delete m_caoEntity;
    delete m_cylinderEntity;
    delete m_circleEntity;
}

int SceneModifier::primitiveType(QString &type)
{
    const QStringList t_defined = {"3DPoints",
                                   "3DLines",
                                   "Facesfrom3Dlines",
                                   "Facesfrom3Dpoints",
                                   "3Dcylinders",
                                   "3Dcircles"};
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
    QList<QVector3D> vertices;
    m_facePickers = new QList<Qt3DRender::QObjectPicker *>();

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
                case 5 : m_template = "3D_CYL";
                         break;
                case 6 : m_template = "3D_CIR";
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

        else if(m_template == "3D_CYL")
        {
            data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
            }
            else if(data.count() == 3)
            {
                int indx1 = data[0].toInt()*3, indx2 = data[1].toInt()*3;
                QVector3D axis_1(vertexRawData[indx1], vertexRawData[indx1+1], vertexRawData[indx1+2]);
                QVector3D axis_2(vertexRawData[indx2], vertexRawData[indx2+1], vertexRawData[indx2+2]);
                this->createCylinder(axis_1, axis_2, data[2].toFloat());
            }
        }

        else if(m_template == "3D_CIR")
        {
            data = fields[0].split(" ");
            if(data.count() == 1)
            {
                idx = 0;
            }
            else if(data.count() == 4)
            {
                int indx1 = data[2].toInt()*3, indx2 = data[3].toInt()*3, indx3 = data[1].toInt()*3;
                QVector3D circum_1(vertexRawData[indx1], vertexRawData[indx1+1], vertexRawData[indx1+2]);
                QVector3D circum_2(vertexRawData[indx2], vertexRawData[indx2+1], vertexRawData[indx2+2]);
                QVector3D center(vertexRawData[indx3], vertexRawData[indx3+1], vertexRawData[indx3+2]);
                this->createCircle(circum_1, circum_2, center, data[0].toFloat());
            }
        }
    }

    float* vertexMapData;
    vertexMapData = (facelpointRawData.isEmpty() ? (facelineRawData.isEmpty() ?
                                    (lineRawData ? new float[lineNum*3] : vertexRawData) : new float[facelineNum*6])
                                 : new float[facepointNum*3]);
    vertexNum = (facelpointRawData.isEmpty() ?
                     (facelineRawData.isEmpty() ?
                          (lineRawData ?
                               lineNum*3 : vertexNum) : facelineNum*6): facepointNum*3);

//#ifndef (facelpointRawData)
//    qInfo() << "facepoint";
//#endif
    qInfo() << vertexNum;
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

    if(!facelpointRawData.isEmpty() || !facelineRawData.isEmpty() || lineRawData)
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

    Qt3DCore::QEntity *m_caoEntity = new Qt3DCore::QEntity(m_rootEntity);

    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(m_caoEntity);
    picker->setHoverEnabled(true);
    picker->setObjectName(QStringLiteral("__internal object picker ") + m_caoEntity->objectName());
    m_facePickers->append(picker);

    m_caoEntity->addComponent(meshRenderer);
    m_caoEntity->addComponent(caoMaterial);
    m_caoEntity->addComponent(m_facePickers->last());

    connect(m_facePickers->last(), &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePickerPress);
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

    m_cylinderEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_cylinderEntity->addComponent(cylinder);
    m_cylinderEntity->addComponent(caoMaterial);
    m_cylinderEntity->addComponent(cylinderTransform);

    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(m_cylinderEntity);
    picker->setObjectName(QStringLiteral("__internal object picker ") + m_cylinderEntity->objectName());
//    m_facePickers->append(picker);
    m_cylinderEntity->addComponent(picker);

    connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePickerPress);
}

void SceneModifier::createCircle(QVector3D circum_1, QVector3D circum_2, QVector3D center, float radius)
{

    Qt3DExtras::QTorusMesh *circle = new Qt3DExtras::QTorusMesh();
    circle->setRadius(radius);
    circle->setMinorRadius(radius/100);
    circle->setRings(100);
    circle->setSlices(20);

    // CircleMesh Transform
    Qt3DCore::QTransform *circleTransform = new Qt3DCore::QTransform();
    circleTransform->setTranslation(center);

    m_circleEntity = new Qt3DCore::QEntity(m_rootEntity);

    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(m_circleEntity);
    picker->setObjectName(QStringLiteral("__internal object picker ") + m_circleEntity->objectName());

    m_circleEntity->addComponent(circle);
    m_circleEntity->addComponent(caoMaterial);
    m_circleEntity->addComponent(circleTransform);
    m_circleEntity->addComponent(picker);

    connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePickerPress);
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
           qInfo() << pressedEntity->isEnabled() << pressedEntity->objectName() << " STATE" ;
//           pressedEntity->setEnabled(!pressedEntity->isEnabled());
           caoMaterial->setDiffuse(QColor(200,100,12,255));
        }
    }
}

void SceneModifier::mouseControls(Qt3DInput::QKeyEvent *event)
{
    qInfo() << event->key();
}
