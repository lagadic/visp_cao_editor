/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
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
{
    // Cuboid shape data
    Qt3DExtras::QCuboidMesh *cuboid = new Qt3DExtras::QCuboidMesh();

    // CuboidMesh Transform
    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(0.1f);
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
}

void SceneModifier::createMesh(QTextStream &input)
{
    m_cuboidEntity->setEnabled(false);
    QString m_template =  "";
    int idx = 0;
    float* vertexRawData;
    int vertexNum = 0;
    while(!input.atEnd())
    {
        QString line = input.readLine();
        QStringList fields = line.split("#");
        if(fields.count() == 2)
        {
            QString type = fields[1].replace(" ","");
            if(!type.compare("3DPoints", Qt::CaseInsensitive))
                m_template = "3D_PTS";
            else if(!type.compare("3DLines", Qt::CaseInsensitive))
                m_template = "3D_LNS";
            else if(!type.compare("Facesfrom3Dlines", Qt::CaseInsensitive))
                m_template = "3D_F_LNS";
            else if(!type.compare("Facesfrom3Dpoints", Qt::CaseInsensitive))
                m_template = "3D_F_PTS";
            else if(!type.compare("3Dcylinders", Qt::CaseInsensitive))
                m_template = "3D_CYL";
            else if(!type.compare("3Dcircles", Qt::CaseInsensitive))
                m_template = "3D_CIR";
        }

        if(m_template == "3D_PTS" && !fields[0].isEmpty())
        {
            QStringList data = fields[0].split(" ");
            if(data.count() == 1)
            {
                vertexNum = data[0].toInt();
                vertexRawData = new float[vertexNum*3];
                qInfo() << vertexNum << fields;
            }
            else if(data.count() >= 3)
            {
                vertexRawData[idx++] = data[0].toFloat();vertexRawData[idx++] = data[1].toFloat();vertexRawData[idx++] = data[2].toFloat();
            }
        }
    }

    //Borrowed Mesh Cration
    meshRenderer = new Qt3DRender::QGeometryRenderer;
    geometry = new Qt3DRender::QGeometry(meshRenderer);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);


    QByteArray ba;
    int bufferSize = vertexNum * sizeof(float);
    ba.resize(bufferSize);
    memcpy(ba.data(), reinterpret_cast<const char*>(vertexRawData), bufferSize);
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
    meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    meshRenderer->setGeometry(geometry);
    meshRenderer->setVertexCount(vertexNum / 2);


    // TorusMesh Transform
    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform();
    torusTransform->setScale(3.0f);
//    torusTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 25.0f));
    torusTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    torusMaterial = new Qt3DExtras::QPhongMaterial();
    torusMaterial->setDiffuse(QColor(QRgb(0xbeb32b)));

    // Torus
    Qt3DCore::QEntity *m_torusEntity = new Qt3DCore::QEntity(m_rootEntity);

    m_facePickers = new QList<Qt3DRender::QObjectPicker *>();
    Qt3DRender::QObjectPicker *picker1 = new Qt3DRender::QObjectPicker(m_torusEntity);
    picker1->setHoverEnabled(true);
    picker1->setObjectName(QStringLiteral("__internal object picker ") + m_torusEntity->objectName());
    m_facePickers->append(picker1);
    m_torusEntity->addComponent(meshRenderer);
    m_torusEntity->addComponent(torusMaterial);
    m_torusEntity->addComponent(torusTransform);
    m_torusEntity->addComponent(m_facePickers->at(0));
    connect(m_facePickers->at(0), &Qt3DRender::QObjectPicker::pressed, this, &SceneModifier::handlePickerPress);
}

void SceneModifier::enableTorus(bool enabled)
{
    m_torusEntity->setEnabled(enabled);
}

void SceneModifier::handlePickerPress(Qt3DRender::QPickEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        if (pressedEntity && pressedEntity->isEnabled())
        {
//           pressedEntity->setEnabled(!pressedEntity->isEnabled());
            torusMaterial->setDiffuse(QColor(200,100,12,255));
        }
    }
}

void SceneModifier::mouseControls(Qt3DInput::QKeyEvent *event)
{
    qInfo() << event->key();
}
