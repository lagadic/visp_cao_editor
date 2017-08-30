/****************************************************************************
**
** Copyright (C) 2017 Vikas Thamizharasan
****************************************************************************/

#include "scenemodifier.h"

#include <QtCore/QDebug>
#include <QMaterial>


SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity, QWidget *parentWidget)
    : m_rootEntity(rootEntity)
    , m_lineEntity(nullptr)
    , m_cylinderEntity(nullptr)
    , m_circleEntity(nullptr)
    , m_parentWidget(parentWidget)
{
    // World Axis
    //X-axis
    this->createLines(QVector3D(-0.6f, 0.0f, -0.6f), QVector3D(0.6f, 0.0f, -0.6f), 1, true, "");
    this->createLines(QVector3D(-0.6f, 0.0f, -0.3f), QVector3D(0.6f, 0.0f, -0.3f), 1, true, "");
    this->createLines(QVector3D(-0.6f, 0.0f, 0.0f),  QVector3D(0.6f, 0.0f, 0.0f), 0, true, "");
    this->createLines(QVector3D(-0.6f, 0.0f, 0.3f),  QVector3D(0.6f, 0.0f, 0.3f), 1, true, "");
    this->createLines(QVector3D(-0.6f, 0.0f, 0.6f),  QVector3D(0.6f, 0.0f, 0.6f), 1, true, "");

    //Z-axis
    this->createLines(QVector3D(-0.6f, 0.0f, -0.6f), QVector3D(-0.6f, 0.0f, 0.6f), 1, true, "");
    this->createLines(QVector3D(-0.3f, 0.0f, -0.6f), QVector3D(-0.3f, 0.0f, 0.6f), 1, true, "");
    this->createLines(QVector3D(0.0f, 0.0f, -0.6f),  QVector3D(0.0f, 0.0f, 0.6f), 2, true, "");
    this->createLines(QVector3D(0.3f, 0.0f, -0.6f),  QVector3D(0.3f, 0.0f, 0.6f), 1, true, "");
    this->createLines(QVector3D(0.6f, 0.0f, -0.6f),  QVector3D(0.6f, 0.0f, 0.6f), 1, true, "");

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
    scene_entities.append(m_cuboidEntity);
    this->initData();
}

SceneModifier::~SceneModifier()
{
}

void SceneModifier::initData()
{
    vertices = new QList<QVector3D>();
    lineRawData = new QList<QVector2D>();
    cylinder = new QList<QVector3D>();
    circle = new QList<QVector4D>();
    init_points = new QList<QVector3D>();

    line_param = new QList<QString>();
    faceline_param = new QList<QString>();
    facepoint_param = new QList<QString>();
    cylinder_param = new QList<QString>();
    circle_param = new QList<QString>();

    vertices_index = 0;
    lineRawData_index = 0;
    facelineRawData_index = 0;
    facepointRawData_index = 0;
    cylinder_index = 0;
    circle_index = 0;
}

QString SceneModifier::getLodParameters(const QStringList &data, const QString &type,
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

void SceneModifier::parse3DFile(QTextStream &input, const bool useBlenderFrame)
{
    if(m_cuboidEntity->isEnabled())
        m_cuboidEntity->setEnabled(false);

    const QStringList t_defined = {"3D_PTS",
                                   "3D_LNS",
                                   "3D_F_LNS",
                                   "3D_F_PTS",
                                   "3D_CYL",
                                   "3D_CIR"};
    QString m_template =  "";
    unsigned int index = 0;
    unsigned int number_lines =0;
    QRegExp re("\\d*");

    while(!input.atEnd())
    {
        QString line = input.readLine();
        QStringList fields = line.split("#");

        if(fields[0].isEmpty())
            continue;

        QStringList temp_data = fields[0].split(" ");
        QStringList data;

        for(int i=0;i<temp_data.length();i++)
            if(!temp_data[i].isEmpty())
                data.append(temp_data[i]);

        unsigned int data_size = data.count();
        QString lod_param = "+";

        if(data_size == 1)
            if (re.exactMatch(data[0]))
                m_template = t_defined[index++];

        if(!m_template.compare("3D_PTS", Qt::CaseSensitive) && data_size >= 3)
        {
            if (useBlenderFrame)
            {
                //Transformation between Blender and Qt
                // 1  0  0
                // 0  0  1
                // 0 -1  0
                QVector3D v(data[0].toFloat(), data[2].toFloat(), -data[1].toFloat());
                vertices->append(v);
            }
            else
            {
                QVector3D v(data[0].toFloat(), data[1].toFloat(), data[2].toFloat());
                vertices->append(v);
            }
            this->createPoints(vertices->last(), QString::number(vertices->length()-1));
        }

        else if(!m_template.compare("3D_LNS", Qt::CaseSensitive) && data_size >= 2)
        {
            lod_param = getLodParameters(data, m_template, 2, data_size);
            QVector2D line1(data[0].toInt()+vertices_index, data[1].toInt()+vertices_index);
            lineRawData->append(line1);
            line_param->append(lod_param);
        }

        else if(!m_template.compare("3D_F_LNS", Qt::CaseSensitive))
        {
            if (data[0].toInt() == 0)
            {
                number_lines = lineRawData->length() - lineRawData_index;
                for(int i=0;i<number_lines;i++)
                {
                    QVector2D l(lineRawData->at(i+lineRawData_index));
                    this->createLines(vertices->at(l[0]), vertices->at(l[1]), i+lineRawData_index, false, line_param->at(i+lineRawData_index));
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
                    QVector2D line = lineRawData->at(data[i].toInt()+lineRawData_index);
                    faceMap.append(data[i].toInt()+lineRawData_index);
                    this->createLines(vertices->at(line[0]), vertices->at(line[1]), facelineRawData.length() + facelineRawData_index, false, lod_param);
                }
                facelineRawData.append(faceMap);
                faceline_param->append(lod_param);
            }
        }

        else if(!m_template.compare("3D_F_PTS", Qt::CaseSensitive) && data_size > 3)
        { 
            QList<int> faceMap;
            unsigned int num_pts = data[0].toInt();
            faceMap.append(num_pts);
            lod_param = getLodParameters(data, m_template, num_pts+1, data_size);

            for(unsigned int i=1;i<num_pts;i++)
            {
                QVector2D line1(data[i].toInt()+vertices_index, data[i+1].toInt()+vertices_index);
                faceMap.append(line1[0]);
                this->createLines(vertices->at(line1[0]), vertices->at(line1[1]), facepointRawData.length()+facepointRawData_index, false, lod_param);
            }

            QVector2D line1(data[num_pts].toInt()+vertices_index, data[1].toInt()+vertices_index);
            faceMap.append(line1[0]);
            this->createLines(vertices->at(line1[0]), vertices->at(line1[1]), facepointRawData.length()+facepointRawData_index, false, lod_param);
            facepointRawData.append(faceMap);
            facepoint_param->append(lod_param);
        }

        else if(!m_template.compare("3D_CYL", Qt::CaseSensitive) && data_size >= 3)
        {
            lod_param = getLodParameters(data, m_template, 3, data_size);
            unsigned int idx1 = data[0].toInt()+vertices_index;
            unsigned int idx2 = data[1].toInt()+vertices_index;
            this->createCylinder(vertices->at(idx1), vertices->at(idx2), cylinder->length()+cylinder_index, data[2].toFloat(), lod_param);
            cylinder->append(QVector3D(idx1,idx2, data[2].toFloat()));
            cylinder_param->append(lod_param);
        }

        else if(!m_template.compare("3D_CIR", Qt::CaseSensitive) && data_size >= 4)
        {
            lod_param = getLodParameters(data, m_template, 4, data_size);
            unsigned int idx1 = data[2].toInt()+vertices_index;
            unsigned int idx2 = data[3].toInt()+vertices_index;
            unsigned int idx3 = data[1].toInt()+vertices_index;
            this->createCircle(vertices->at(idx1), vertices->at(idx2), vertices->at(idx3), circle->length()+circle_index, data[0].toFloat(), lod_param);
            circle->append(QVector4D(data[0].toFloat(), idx3, idx1, idx2));
            circle_param->append(lod_param);
        }
    }
    vertices_index = vertices->length();
    lineRawData_index = lineRawData->length();
    facelineRawData_index = facelineRawData.length();
    facepointRawData_index = facepointRawData.length();
    cylinder_index = cylinder->length();
    circle_index = circle->length();
}

void SceneModifier::createLines(const QVector3D &v0, const QVector3D &v1,
                                const unsigned int index, const bool axis, const QString &lod_param)
{
    Qt3DRender::QGeometryRenderer *line_mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(line_mesh);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

    QByteArray vertexBufferData;
    vertexBufferData.resize(2 * (3 + 3) * sizeof(float));

    QVector<QVector3D> vertices = (!axis ? QVector<QVector3D>() << v0 << QVector3D (0.5f, 0.0f, 0.5f) << v1  << QVector3D (0.5f, 0.0f, 0.5f)
                                         : (index==0 ? QVector<QVector3D>() << v0 << QVector3D (1.0f, 0.0f, 0.0f) << v1  << QVector3D (1.0f, 0.0f, 0.0f)
                                                      : (index==1 ? QVector<QVector3D>() << v0 << QVector3D (0.0f, 0.0f, 0.0f) << v1  << QVector3D (0.0f, 0.0f, 0.0f)
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

void SceneModifier::createCylinder(const QVector3D &axis_1, const QVector3D &axis_2,
                                   const unsigned int index, const float radius, const QString &lod_param)
{
    QVector3D main_axis = axis_2 - axis_1;
    QVector3D mid_point = (axis_1 + axis_2) / 2;
    QVector3D main_axis_norm = main_axis.normalized();

    Qt3DExtras::QCylinderMesh *cylinder = new Qt3DExtras::QCylinderMesh();
    cylinder->setRadius(radius);
    cylinder->setLength(qSqrt(qPow(main_axis[0],2) + qPow(main_axis[1],2) + qPow(main_axis[2],2)));
    cylinder->setRings(100);
    cylinder->setSlices(20);

    // CylinderMesh Transform
    Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform();
    QVector3D cylinderAxisRef(0, 1, 0);
    cylinderTransform->setRotation(QQuaternion::rotationTo(cylinderAxisRef, main_axis_norm));
    cylinderTransform->setTranslation(mid_point);

    m_cylinderEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_cylinderEntity->addComponent(cylinder);
    m_cylinderEntity->addComponent(caoMaterial);
    m_cylinderEntity->addComponent(cylinderTransform);

    m_cylinderEntity->setObjectName(QString::number(index)+ ":" +lod_param);
    createObjectPickerForEntity(m_cylinderEntity);
}

void SceneModifier::createCircle(const QVector3D &circum_1, const QVector3D &circum_2, const QVector3D &center,
                                 const unsigned int index, const float radius, const QString &lod_param)
{
    Qt3DExtras::QTorusMesh *circle = new Qt3DExtras::QTorusMesh();
    circle->setRadius(radius);
    circle->setMinorRadius(radius/100);
    circle->setRings(100);
    circle->setSlices(20);

    // CircleMesh Transform
    Qt3DCore::QTransform *circleTransform = new Qt3DCore::QTransform();
    QVector3D vec1 = (circum_1 - center).normalized(), vec2 = (circum_2 - center).normalized();
    QVector3D vec3 = QVector3D::crossProduct(vec1, vec2).normalized();
    QVector3D circleAxisRef(0, 0, 1);
    circleTransform->setRotation(QQuaternion::rotationTo(circleAxisRef, vec3));
    circleTransform->setTranslation(center);

    m_circleEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_circleEntity->addComponent(circle);
    m_circleEntity->addComponent(caoMaterial);
    m_circleEntity->addComponent(circleTransform);

    m_circleEntity->setObjectName(QString::number(index) + ":" + lod_param);
    createObjectPickerForEntity(m_circleEntity);
}

void SceneModifier::createPoints(const QVector3D &point, const QString index)
{
    Qt3DExtras::QSphereMesh *sphere = new Qt3DExtras::QSphereMesh();
    sphere->setRadius(0.4);

    Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform();
    sphereTransform->setTranslation(point);

    Qt3DExtras::QPhongMaterial *sphereColor = new Qt3DExtras::QPhongMaterial();
    sphereColor->setDiffuse(QColor(255,0,0,255));

    Qt3DCore::QEntity *m_sphereEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_sphereEntity->setObjectName(index);
    m_sphereEntity->addComponent(sphere);
    m_sphereEntity->addComponent(sphereTransform);
    m_sphereEntity->addComponent(sphereColor);
    m_sphereEntity->setEnabled(false);

    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(m_sphereEntity);
    picker->setHoverEnabled(false);
    m_sphereEntity->addComponent(picker);
    connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePointSelect);

    scene_points.append(m_sphereEntity);
}

Qt3DRender::QObjectPicker *SceneModifier::createObjectPickerForEntity(Qt3DCore::QEntity *entity)
{
    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(entity);
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
    scene_entities.clear();
    Q_FOREACH (Qt3DCore::QEntity *entity, scene_points)
    {
        entity->setEnabled(false);
    }
    scene_points.clear();
    init_points->clear();
    this->initData();
}

void SceneModifier::getLineLength()
{

}

bool SceneModifier::handleMousePress(QMouseEvent *event)
{
    m_mouseButton = event->button();
    return false;
}

void SceneModifier::formAddField(QDialog *dialog, QFormLayout *form,
                                 const QString tag, const QString text)
{
    QLineEdit *lineEdit = new QLineEdit(dialog);
    lineEdit->setText(text);
    form->addRow(tag, lineEdit);
    fields << lineEdit;
}

void SceneModifier::handlePointSelect(Qt3DRender::QPickEvent *event)
{
    if (event->button() == Qt3DRender::QPickEvent::RightButton)
    {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        if (pressedEntity && pressedEntity->isEnabled())
        {
            Qt3DExtras::QPhongMaterial *sphereColor = new Qt3DExtras::QPhongMaterial();
            sphereColor->setDiffuse(QColor(0,255,0,255));
            pressedEntity->addComponent(sphereColor);
            init_points->append(vertices->at(pressedEntity->objectName().toInt()));
        }
    }
}

void SceneModifier::handlePickerPress(Qt3DRender::QPickEvent *event)
{
    if (event->button() == Qt3DRender::QPickEvent::RightButton)
    {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        if (pressedEntity && pressedEntity->isEnabled())
        {
            QStringList data = pressedEntity->objectName().split("+");
            QString m_template = data[0].split(":")[1];
            QStringList lod_param = data[1].split(" ");
            int index = data[0].split(":")[0].toInt();
            fields.clear();
            QString lod_name, lod_useLod, lod_minLine, lod_minPoly;

            for(int i=0;i<lod_param.length();i++)
                if(lod_param[i]=="")
                    lod_param.removeAt(i);

            if (lod_param.length())
            {
                if (lod_param[0][5]=="\"")
                {
                    lod_param[0] += " " + lod_param[1];
                    lod_param.removeAt(1);
                }

                for(int i=0;i<lod_param.length();i++)
                {
                    if(lod_param[i].contains("name="))
                        lod_name = lod_param[i].split("name=")[1];
                    else if(lod_param[i].contains("useLod="))
                        lod_useLod = lod_param[i].split("useLod=")[1];
                    else if(lod_param[i].contains("minLineLengthThreshold="))
                        lod_minLine = lod_param[i].split("minLineLengthThreshold=")[1];
                    else if(lod_param[i].contains("minPolygonAreaThreshold="))
                        lod_minPoly = lod_param[i].split("minPolygonAreaThreshold=")[1];
                }

            }

            QDialog dialog(m_parentWidget);
            QFormLayout form(&dialog);

            form.addRow(new QLabel("LOD parameters"));

            this->formAddField(&dialog, &form, QString("name "), lod_name);
            this->formAddField(&dialog, &form, QString("useLod "), lod_useLod);
            this->formAddField(&dialog, &form, QString("minLineLengthThreshold "), lod_minLine);
            this->formAddField(&dialog, &form, QString("minPolygonAreaThreshold "), lod_minPoly);

//            QPushButton *lineLength = new QPushButton("Update", &dialog);
//            QObject::connect(lineLength, SIGNAL (released()), this, SLOT(getLineLength()));
//            QHBoxLayout hBoxLayout;
//            hBoxLayout.addWidget(lineEdit3);
//            hBoxLayout.addWidget(lineLength);
//            QWidget container;
//            container.setLayout(&hBoxLayout);

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
                pressedEntity->setObjectName(data[0]+new_lod_param);

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
                qInfo() << index << new_lod_param;
            }
        }
    }
}

void SceneModifier::mouseControls(Qt3DInput::QKeyEvent *event)
{
    qInfo() << event->key();
}
