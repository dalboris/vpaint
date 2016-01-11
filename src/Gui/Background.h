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

class XmlStreamWriter;
class XmlStreamReader;

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

    // Size Type
    enum SizeType {
        Cover = 0,
        Manual = 1
    };
    SizeType sizeType() const;
    void setSizeType(SizeType newSizeType);

    // Size as authored (ignore canvas size even if sizeType() == Cover)
    Eigen::Vector2d size() const;
    void setSize(const Eigen::Vector2d & newSize);

    // Actual size taking into account sizeType(), size(), and canvasSize.
    Eigen::Vector2d computedSize(const Eigen::Vector2d & canvasSize) const;

    // Repeat
    enum RepeatType {
        NoRepeat = 0,
        RepeatX = 1,
        RepeatY = 2,
        Repeat = 3 // == (RepeatX | RepeatY) if enum interpreted as bit flags
    };
    RepeatType repeatType() const;
    void setRepeatType(RepeatType newRepeatType);
    bool repeatX() const; // true iff either RepeatX or Repeat
    bool repeatY() const; // true iff either RepeatY or Repeat

    // Opacity
    double opacity() const;
    void setOpacity(double newOpacity);

    // Hold
    bool hold() const;
    void setHold(bool newHold);

    // Emit signals
    void emitCheckpoint() {emit checkpoint();}

    // Read/Write XML
    void write(XmlStreamWriter & xml);
    void read(XmlStreamReader & xml);

signals:
    // signal emitted whenever any value is changed
    void changed();

    // signal emitted whenever the user manually changed
    // a value, and therefore should add an item in the undo stack
    void checkpoint();

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
