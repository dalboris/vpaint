// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QObject>

#include "Color.h"

#include <QString>
#include <QImage>
#include <Eigen/Core>

class Background: public QObject
{
    Q_OBJECT

public:
    // Constructor / Copy constructor
    Background();

    // Assignement operator. Re-implemented to emit changed()
    void operator=(const Background & other);

    // Color
    Color color() const;
    void setColor(const Color & newColor);

    // Image(s)
    QString imageUrl() const;
    void setImageUrl(const QString & newUrl);
    QImage image(int frame) const;

    // Position
    Eigen::Vector2d position() const;
    void setPosition(const Eigen::Vector2d & newPosition);

    // Size
    enum SizeType {
        Cover = 0,
        Manual = 1
    };
    SizeType sizeType() const;
    Eigen::Vector2d size() const;
    void setSizeType(SizeType newSizeType);
    void setSize(const Eigen::Vector2d & newSize);

    // Repeat
    enum RepeatType {
        NoRepeat = 0,
        RepeatX = 1,
        RepeatY = 2,
        Repeat = 3
    };
    RepeatType repeatType() const;
    void setRepeatType(RepeatType newRepeatType);

    // Opacity
    double opacity() const;
    void setOpacity(double newOpacity);

    // Hold
    bool hold() const;
    void setHold(bool newHold);

signals:
    // signal emitted whenever any value is changed
    void changed();

    // signals emitted when specific values are changed
    void colorChanged(Color newColor);
    void imageUrlChanged(QString newUrl);
    void positionChanged(const Eigen::Vector2d & newPosition);
    void sizeTypeChanged(SizeType newSizeType);
    void sizeChanged(const Eigen::Vector2d & newSize);
    void repeatTypeChanged(RepeatType newRepeatType);
    void opacityChanged(double newOpacity);
    void holdChanged(bool newHold);

private:
    // Color
    Color color_;

    // Image(s)
    QString imageUrl_;

    // Position
    Eigen::Vector2d position_;

    // Size
    SizeType sizeType_;
    Eigen::Vector2d size_;

    // Repeat
    RepeatType repeatType_;

    // Opacity
    double opacity_;

    // Hold
    bool hold_;
};

#endif // BACKGROUND_H
