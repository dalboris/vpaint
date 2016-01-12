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

#include <QVector>
#include <QString>
#include <QImage>
#include <Eigen/Core>

class XmlStreamWriter;
class XmlStreamReader;

class Background: public QObject
{
    Q_OBJECT

public:
    // Constructor
    Background(QObject * parent = 0);

    // Copy constructor and assignment operator. Semantics:
    //   * Copy data
    //   * Clear cache
    //   * Don't copy QObject "identity" (parent, name, etc...)
    //   * assignment emit changed()
    Background(const Background & other, QObject * parent = 0);
    Background & operator=(const Background & other);

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

    // Cache
    void clearCache();

    // Emit signals
    void emitCheckpoint() {emit checkpoint();}

    // Read/Write XML
    void write(XmlStreamWriter & xml);
    void read(XmlStreamReader & xml);

signals:
    // Signal emitted whenever any value is changed
    void changed();

    // Signal emitted whenever the user manually changed
    // a value, and therefore should add an item in the undo stack
    void checkpoint();

    // Signals emitted when specific values are changed
    void colorChanged(Color newColor);
    void imageUrlChanged(QString newUrl);
    void positionChanged(const Eigen::Vector2d & newPosition);
    void sizeTypeChanged(SizeType newSizeType);
    void sizeChanged(const Eigen::Vector2d & newSize);
    void repeatTypeChanged(RepeatType newRepeatType);
    void opacityChanged(double newOpacity);
    void holdChanged(bool newHold);

    // Signal emitted when the cache is cleared.
    // Clients performing further caching of images should listen to this
    // and clear their own cache when the signal is emitted
    void cacheCleared();

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

    // Cache
    void updateCache_() const;
    void computeCache_() const;
    mutable bool cached_;
    mutable int minFrame_;
    mutable QString filePathsPrefix_;
    mutable QString filePathsSuffix_;
    mutable QVector<QString> filePathsWildcards_;
};

#endif // BACKGROUND_H
