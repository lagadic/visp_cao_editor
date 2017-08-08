/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan
****************************************************************************/

#include "scenemodifier.h"

#include <QtCore/QDebug>

#include <QMaterial>
#include <QEffect>

#include <QTechnique>
#include <QRenderPass>
#include <QShaderProgram>


SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity, QWidget *parentWidget)
    : m_rootEntity(rootEntity)
    , m_parentWidget(parentWidget)
    , m_lineEntity(nullptr)
    , m_cylinderEntity(nullptr)
    , m_circleEntity(nullptr)
{
    // World Axis
    this->createLines(QVector3D(-100000.0f, 0.0f, 0.0f), QVector3D(100000.0f, 0.0f, 0.0f), 0, true, ""); //X-axis
    this->createLines(QVector3D(0.0f, -100000.0f, 0.0f), QVector3D(0.0f, 100000.0f, 0.0f), 1, true, ""); //Y-axis
    this->createLines(QVector3D(0.0f, 0.0f, -100000.0f), QVector3D(0.0f, 0.0f, 100000.0f), 2, true, ""); //Z-axis

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
    delete m_cuboidEntity;
    delete m_cylinderEntity;
    delete m_circleEntity;
}

int SceneModifier::primitiveType(const QString &type)
{
    const QStringList t_defined = {"3DPoints",
                                   "3DLines",
                                   "Facesfrom3Dlines",
                                   "Facesfrom3Dpoints",
                                   "3Dcylinders",
                                   "3Dcircles"};
    for (unsigned int i=1; i<=6; i++)
        if(!type.compare(t_defined[i-1], Qt::CaseInsensitive))
            return i;
    return 0;
}

QString SceneModifier::getLodParameters(QStringList data, const QString type,
                                        const unsigned int idx1, const unsigned int idx2)
{
    QString lod_param = type + "+";
    for(unsigned int i=idx1; i<idx2; i++)
    {
        if(!data[i].compare("#"))
            break;
        lod_param += data[i] + " ";
    }
    return lod_param;
}

void SceneModifier::parse3DFile(QTextStream &input)
{
    if(m_cuboidEntity->isEnabled())
        m_cuboidEntity->setEnabled(false);

    QString m_template =  "";

    vertices = new QList<QVector3D>();
    lineRawData = new QList<QVector2D>();
    cylinder = new QList<QVector3D>();
    circle = new QList<QVector4D>();

    line_param = new QList<QString>();
    faceline_param = new QList<QString>();
    facepoint_param = new QList<QString>();
    cylinder_param = new QList<QString>();
    circle_param = new QList<QString>();

    //m_facePickers = new QList<Qt3DRender::QObjectPicker *>();

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
                case 5 : m_template = "3D_CYL";
                         break;
                case 6 : m_template = "3D_CIR";
                         break;
            }
        }

        if(fields[0].isEmpty())
            continue;

        QStringList data = fields[0].split(" ");
        unsigned int data_size = data.count();
        QString lod_param = "+";

        if(!m_template.compare("3D_PTS", Qt::CaseSensitive) && data_size >= 3)
        {
            QVector3D v(data[0].toFloat(),data[1].toFloat(),data[2].toFloat());
            vertices->append(v);
        }

        else if(!m_template.compare("3D_LNS", Qt::CaseSensitive) && data_size >= 2)
        {
            lod_param = getLodParameters(data, m_template, 2, data_size);
            QVector2D line1(data[0].toInt(), data[1].toInt());
            lineRawData->append(line1);
            line_param->append(lod_param);
        }

        else if(!m_template.compare("3D_F_LNS", Qt::CaseSensitive))
        {
            if (data[0].toInt() == 0)
            {
                for(int i=0;i<lineRawData->length();i++)
                {
                    QVector2D l(lineRawData->at(i));
                    this->createLines(vertices->at(l[0]), vertices->at(l[1]), i, false, line_param->at(i));
                }
            }

            else if(data_size > 3)
            {
                QList<int> faceMap;
                unsigned int num_pts = data[0].toInt();
                faceMap.append(num_pts);
                lod_param = getLodParameters(data, m_template, num_pts+1, data_size);

                for(unsigned int i=1;i<=num_pts;i++)
                {
                    QVector2D line = lineRawData->at(data[i].toInt());
                    faceMap.append(data[i].toInt());
                    this->createLines(vertices->at(line[0]), vertices->at(line[1]), facelineRawData.length(), false, lod_param);
                }
                facelineRawData.append(faceMap);
                faceline_param->append(lod_param);
            }
        }

        else if(!m_template.compare("3D_F_PTS", Qt::CaseSensitive) && data_size > 3)
        { 
            //QRegExp re(".*[a-z]");
            QList<int> faceMap;
            unsigned int num_pts = data[0].toInt();
            faceMap.append(num_pts);
            lod_param = getLodParameters(data, m_template, num_pts+1, data_size);

            for(unsigned int i=1;i<num_pts;i++)
            {
                QVector2D line1(data[i].toInt(), data[i+1].toInt());
                faceMap.append(line1[0]);
                this->createLines(vertices->at(line1[0]), vertices->at(line1[1]), facelpointRawData.length(), false, lod_param);
            }

            QVector2D line1(data[num_pts].toInt(), data[1].toInt());
            faceMap.append(line1[0]);
            this->createLines(vertices->at(line1[0]), vertices->at(line1[1]), facelpointRawData.length(), false, lod_param);
            facelpointRawData.append(faceMap);
            facepoint_param->append(lod_param);
        }

        else if(!m_template.compare("3D_CYL", Qt::CaseSensitive) && data_size >= 3)
        {
            lod_param = getLodParameters(data, m_template, 3, data_size);
            unsigned int idx1 = data[0].toInt();
            unsigned int idx2 = data[1].toInt();
            this->createCylinder(vertices->at(idx1), vertices->at(idx2), cylinder->length(), data[2].toFloat(), lod_param);
            cylinder->append(QVector3D(idx1,idx2, data[2].toFloat()));
            cylinder_param->append(lod_param);
        }

        else if(!m_template.compare("3D_CIR", Qt::CaseSensitive) && data_size >= 4)
        {
            lod_param = getLodParameters(data, m_template, 4, data_size);
            unsigned int idx1 = data[2].toInt();
            unsigned int idx2 = data[3].toInt();
            unsigned int idx3 = data[1].toInt();
            this->createCircle(vertices->at(idx1), vertices->at(idx2), vertices->at(idx3), circle->length(), data[0].toFloat(), lod_param);
            circle->append(QVector4D(data[0].toFloat(), idx3, idx1, idx2));
            circle_param->append(lod_param);
        }
    }
}

void SceneModifier::createLines(const QVector3D v0, const QVector3D v1,
                                const unsigned int index, const bool axis, QString lod_param)
{
    Qt3DRender::QGeometryRenderer *line_mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(line_mesh);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

    QByteArray vertexBufferData;
    vertexBufferData.resize(2 * (3 + 3) * sizeof(float));


    QVector<QVector3D> vertices = (!axis ? QVector<QVector3D>() << v0 << QVector3D (0.5f, 0.0f, 0.5f) << v1  << QVector3D (0.5f, 0.0f, 0.5f)
                                         : (index==0 ? QVector<QVector3D>() << v0 << QVector3D (1.0f, 0.0f, 0.0f) << v1  << QVector3D (1.0f, 0.0f, 0.0f)
                                                      : (index==1 ? QVector<QVector3D>() << v0 << QVector3D (0.0f, 1.0f, 0.0f) << v1  << QVector3D (0.0f, 1.0f, 0.0f)
                                                                  : (index==2 ? QVector<QVector3D>() << v0 << QVector3D (0.0f, 0.0f, 1.0f) << v1  << QVector3D (0.0f, 0.0f, 1.0f)
                                                                              : QVector<QVector3D>() << v0 << QVector3D (0.0f, 1.0f, 1.0f) << v1  << QVector3D (0.0f, 1.0f, 1.0f)))));

    float *rawVertexArray = reinterpret_cast<float *>(vertexBufferData.data());
    int idx = 0;

    Q_FOREACH (const QVector3D &v, vertices) {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
    }

    QByteArray indexBufferData;
    indexBufferData.resize(2 * 3 * sizeof(ushort));
    ushort *rawIndexArray = reinterpret_cast<ushort *>(indexBufferData.data());

    rawIndexArray[0] = 0;
    rawIndexArray[1] = 1;

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
    positionAttribute->setCount(2);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *colorAttribute = new Qt3DRender::QAttribute();
    colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    colorAttribute->setBuffer(vertexDataBuffer);
    colorAttribute->setDataType(Qt3DRender::QAttribute::Float);
    colorAttribute->setDataSize(3);
    colorAttribute->setByteOffset(3 * sizeof(float));
    colorAttribute->setByteStride(6 * sizeof(float));
    colorAttribute->setCount(2);
    colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());

    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute();
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexDataBuffer);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedShort);
    indexAttribute->setDataSize(1);
    indexAttribute->setByteOffset(0);
    indexAttribute->setByteStride(0);
    indexAttribute->setCount(2);

    geometry->addAttribute(positionAttribute);
    geometry->addAttribute(colorAttribute);
    geometry->addAttribute(indexAttribute);

    line_mesh->setInstanceCount(1);
    line_mesh->setIndexOffset(0);
    line_mesh->setFirstInstance(0);
    line_mesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    line_mesh->setGeometry(geometry);
    line_mesh->setVertexCount(2);

    // Material
    Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial(m_rootEntity);

    // Line Entity
    m_lineEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_lineEntity->addComponent(line_mesh);
    m_lineEntity->addComponent(material);

    if(!axis)
    {
        m_lineEntity->setObjectName(QString::number(index)+ ":" +lod_param);
        createObjectPickerForEntity(m_lineEntity);
    }
}

void SceneModifier::createCylinder( const QVector3D axis_1, const QVector3D axis_2,
                                   unsigned int index,float radius, QString load_param)
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

    m_cylinderEntity->setObjectName(QString::number(index)+ ":" +load_param);
    createObjectPickerForEntity(m_cylinderEntity);
}

void SceneModifier::createCircle(const QVector3D circum_1, const QVector3D circum_2, const QVector3D center,
                                 unsigned int index, float radius, QString load_param)
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

    m_circleEntity->addComponent(circle);
    m_circleEntity->addComponent(caoMaterial);
    m_circleEntity->addComponent(circleTransform);

    m_circleEntity->setObjectName(QString::number(index) + ":" + load_param);
    createObjectPickerForEntity(m_circleEntity);
}

Qt3DRender::QObjectPicker *SceneModifier::createObjectPickerForEntity(Qt3DCore::QEntity *entity)
{
    Qt3DRender::QObjectPicker *picker = nullptr;
    picker = new Qt3DRender::QObjectPicker(entity);
    picker->setHoverEnabled(false);
    entity->addComponent(picker);
    connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePickerPress);
    scene_entities.append(entity);
    return picker;
}

void SceneModifier::removeSceneElements()
{
    Q_FOREACH (Qt3DCore::QEntity *entity, scene_entities)
    {
        entity->setEnabled(false);
    }
}

bool SceneModifier::handleMousePress(QMouseEvent *event)
{
    m_mouseButton = event->button();
    return false;
}

void SceneModifier::handlePickerPress(Qt3DRender::QPickEvent *event)
{
//    switch (event->type())
//    {
//        case QEvent::MouseButtonPress:
//            handleMousePress(static_cast<QMouseEvent *>(event));
//            break;

//        default:
//            break;
//    }

    if (event->button() == Qt3DRender::QPickEvent::RightButton)
    {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        if (pressedEntity && pressedEntity->isEnabled())
        {
            QStringList data = pressedEntity->objectName().split("+");
            QString m_template = data[0].split(":")[1];
            int index = data[0].split(":")[0].toInt();
            qInfo() << data << m_template << index;

            QStringList lod_param = data[1].split(" ");

            for(int i=0;i<lod_param.length();i++)
                if(lod_param[i]=="")
                    lod_param.removeAt(i);

            QString lod_name = (lod_param.length() >= 1 ? (lod_param[0].split("name=").length() == 2 ? lod_param[0].split("name=")[1] : "") : "");
            QString lod_useLod = (lod_param.length() >= 2 ?
                                      (lod_param[1].split("useLod=").length() == 2 ?
                                          lod_param[1].split("useLod=")[1] :
                                  (lod_param[0].split("useLod=").length() == 2 ?
                                      lod_param[0].split("useLod=")[1] : "")) : "");

            QString lod_minLine = (lod_param.length() >= 2 ?
                                       (lod_param[2].split("minLineLengthThreshold=").length() == 2 ?
                                           lod_param[2].split("minLineLengthThreshold=")[1] :
                                   (lod_param[1].split("minLineLengthThreshold=").length() == 2 ?
                                       lod_param[1].split("minLineLengthThreshold=")[1] : "")) : "");

            QString lod_minPoly = (lod_param.length() >= 2 ?
                                       (lod_param[2].split("minPolygonAreaThreshold=").length() == 2 ?
                                           lod_param[2].split("minPolygonAreaThreshold=")[1] :
                                   (lod_param[1].split("minPolygonAreaThreshold=").length() == 2 ?
                                       lod_param[1].split("minPolygonAreaThreshold=")[1] : "")) : "");

            QDialog dialog(m_parentWidget);
            QFormLayout form(&dialog);

            form.addRow(new QLabel("LOD parameters"));

            QList<QLineEdit *> fields;

            QLineEdit *lineEdit1 = new QLineEdit(&dialog);
            lineEdit1->setText(lod_name);
            form.addRow(QString("name "), lineEdit1);
            fields << lineEdit1;

            QLineEdit *lineEdit2 = new QLineEdit(&dialog);
            lineEdit2->setText(lod_useLod);
            form.addRow(QString("useLod "), lineEdit2);
            fields << lineEdit2;

            QLineEdit *lineEdit3 = new QLineEdit(&dialog);
            lineEdit3->setText(lod_minLine);
            form.addRow(QString("minLineLengthThreshold "), lineEdit3);
            fields << lineEdit3;

            QLineEdit *lineEdit4 = new QLineEdit(&dialog);
            lineEdit4->setText(lod_minPoly);
            form.addRow(QString("minPolygonAreaThreshold "), lineEdit4);
            fields << lineEdit4;

            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                       Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

            QString new_lod_param = "+";
            if (dialog.exec() == QDialog::Accepted)
            {
                int idx = 0;
                // If user hits OK
                foreach(QLineEdit * lineEdit, fields)
                {
                    if(!lineEdit->text().isEmpty())
                    {
                        if(idx==0) new_lod_param += "name=" + lineEdit->text() + " ";
                        else if(idx==1) new_lod_param += "useLod=" + lineEdit->text() + " ";
                        else if(idx==2) new_lod_param += "minLineLengthThreshold=" + lineEdit->text() + " ";
                        else if(idx==3) new_lod_param += "minPolygonAreaThreshold=" + lineEdit->text();
                    }
                    idx++;
                }
            }

            qInfo() << index << m_template << new_lod_param;


            if(!m_template.compare("3D_LNS", Qt::CaseSensitive))
                line_param->replace(index, new_lod_param);

            else if(!m_template.compare("3D_F_LNS", Qt::CaseSensitive))
                faceline_param->replace(index, new_lod_param);

            else if(!m_template.compare("3D_F_PTS", Qt::CaseSensitive))
                facepoint_param->replace(index, new_lod_param);

            else if(!m_template.compare("3D_CYL", Qt::CaseSensitive))
                cylinder_param->replace(index, new_lod_param);

            else if(!m_template.compare("3D_CIR", Qt::CaseSensitive))
                circle_param->replace(index, new_lod_param);
        }
    }
}

void SceneModifier::mouseControls(Qt3DInput::QKeyEvent *event)
{
    qInfo() << event->key();
}
