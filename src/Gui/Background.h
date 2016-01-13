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
    // Enums
    enum class SizeType {
        Cover = 0,
        Manual = 1
    };
    enum class RepeatType {
        NoRepeat = 0,
        RepeatX = 1,
        RepeatY = 2,
        Repeat = 3
    };

    // Data
    struct Data
    {
        Data();
        bool operator==(const Data & other) const;
        bool operator!=(const Data & other) const;

        Color color;
        QString imageUrl;
        Eigen::Vector2d position;
        SizeType sizeType;
        Eigen::Vector2d size;
        RepeatType repeatType;
        double opacity;
        bool hold;
    };

    // Constructor
    Background(QObject * parent = 0);

    // Copy constructor and assignment operator. Semantics:
    //   * Copy data
    //   * Clear cache
    //   * Don't copy QObject "identity" (parent, name, etc...)
    //   * assignment emit changed()
    Background(const Background & other, QObject * parent = 0);
    Background & operator=(const Background & other);

    // Data
    Data data() const;
    void setData(const Data & newData);

    // Color
    Color color() const;
    void setColor(const Color & newColor);

    // Image(s)
    QString imageUrl() const;
    void setImageUrl(const QString & newUrl);

    // The method 'image(f)' gives you the image to draw at frame f. If 'hold()'
    // is true, or if 'imageUrl()' doesn't contain a wildcard, then this image
    // may be identical to the one for another frame, which we refer to as
    // 'referenceFrame(f)'. Clients drawing the background may use this
    // function for caching purposes.
    //
    // If imageUrl() has a wildcard, hold() == false, and f is a frame with no
    // corresponding image on disk, then referenceFrame(f) returns minFrame-1,
    // where minFrame is the smallest frame with an image on disk. This way,
    // all "empty" frames share the same reference frame (we cannot use 0, or -1
    // from this, since the frame 0 or even -1 may have an image on disk)
    QImage image(int frame) const;
    int referenceFrame(int frame) const;

    // Position
    Eigen::Vector2d position() const;
    void setPosition(const Eigen::Vector2d & newPosition);

    // Size Type
    SizeType sizeType() const;
    void setSizeType(SizeType newSizeType);

    // Size as authored (ignore canvas size even if sizeType() == Cover)
    Eigen::Vector2d size() const;
    void setSize(const Eigen::Vector2d & newSize);

    // Actual size taking into account sizeType(), size(), and canvasSize.
    Eigen::Vector2d computedSize(const Eigen::Vector2d & canvasSize) const;

    // Repeat
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
    //
    // XXX This should be refactored out of this class. It should
    // be the 'widget editing the object' that issues checkpoint()
    // without going through the object.
    //
    // Right now, we can see that checkpoint() is only emitted
    // by the object when the widget tells it to, by calling
    // emitCheckpoint(). This is a clear sign that emitting this
    // signal is not the responsability of the object. Therefore,
    // the undo manager should listen to a checkpoint from the widget
    // directly, not from the object.
    void checkpoint();

    // Signals emitted when individual values are changed
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
    // Data
    Data data_;

    // Cache
    void updateCache_() const;
    void computeCache_() const;
    mutable bool cached_;
    mutable int minFrame_;
    mutable QString filePathsPrefix_;
    mutable QString filePathsSuffix_;
    mutable QVector<QString> filePathsWildcards_;
    mutable QVector<int> referenceFrames_;
};

#endif // BACKGROUND_H
