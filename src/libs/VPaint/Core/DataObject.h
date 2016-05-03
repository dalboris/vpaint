// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QObject>

/**************************** DataObjectBase *********************************/

/// \class DataObjectBase
/// \brief An implementation detail for DataObject.
///
/// We need this class because slot/signals cannot be defined in class
/// templates.
///
class DataObjectBase: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(DataObjectBase)

public:
    DataObjectBase() {}

signals:
    void changed();
};


/****************************** DataObject ***********************************/

/// \class DataObject
/// \brief A class template to define QObjects representing scene data.
///
/// The DataObject class template is the foundation of the object model that is
/// used in VPaint to represent scene data.
///
/// <H1> Introduction </H1>
///
/// Every entity in the scene, for instance a "layer", is implemented using
/// two classes:
///
///   - A class "Layer", inheriting from QObject, which doesn't support copy
///     and assignements (i.e., they have an "identity", which is a core
///     concept of the Qt object model to implement signals and slots)
///
///   - A struct-like class "LayerData", not inheriting from anything, and
///     which supports copy and assignement.
///
/// See below an example of how this layer entity would be implemented
/// manually, i.e. without using the DataObject class template:
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     // ...
/// };
///
/// class Layer: public QObject
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
///
/// public:
///     const LayerData & data() const { return data_; }
///     void setData(const LayerData & data) { data_ = data; emit changed(); }
///
/// signal:
///     void changed();
///
/// private:
///     LayerData data_;
/// };
///
/// The Layer class has a LayerData member variable, and the two member
/// functions data() and setData() to access and modify this data. The
/// changed() signal is emitted when the data has changed.
///
/// However, for convenience and maintainability, we factorize all this
/// boilerplate common to all entities via the he DataObject class template.
/// Therefore, here is how you would actually define this layer entity
/// (note that the LayerData struct stays the same):
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     // ...
/// };
///
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
/// };
/// \endcode
///
/// Yes, that's all! Indeed, the member variable data_, the member function
/// data() and setData(), and the signal changed() are implemented in
/// ObjectData<LayerData> and inherited by Layer.
///
/// In practice, the actual Layer class used in VPaint is more complex and
/// define additional signals and slots, but this is the general idea.
///
///
/// <H1> Owned data objects </H1>
///
/// In practice, it is often the case that a data object "owns" other data
/// objects. For example, loosely speaking, a layer is composed, among other
/// things, of a background. Therefore, we say that the layer data object is
/// the "owner" of this background data object. Since this background is
/// part of the data of the layer, one might consider simply add BackgroundData
/// as an attribute to LayerData:
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     BackgroundData background; // BAD
///     // ...
/// };
/// \endcode
///
/// This seems very clean and respects the value semantics of "Data".
/// However, we still need to store a "Background" object (inheriting
/// QObject) somewhere, to be able to use signals and slots. Its appropriate
/// location would be as part of the Layer class:
///
/// \code
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
///
///     Background background_; // or possibly "Background*", "UniquePtr<Background>", etc.
/// };
/// \endcode
///
/// This object model would introduce two issues:
///
///    - Each time we want to add a "data class" to LayerData, we need to add
///      its corresponding "object class" to Layer, which is error-prone.
///
///    - More importantly, the BackgroundData object would now be a member
///      variable of LayerData, which means that it cannot be a member variable
///      of Background as well. Instead, Background would have to indirectly
///      observe it as a "BackgroundData *" member variable. In terms of C++
///      object ownership, this means that the owner of the BackgroundData is
///      the LayerData, not the Background, and therefore LayerData, or Layer,
///      can change BackgroundData directly, without having to do it indirectly
///      via Background. So BackgroundData may possibly change without
///      notifying Background, so Background::changed() wouldn't be emitted.
///
/// Therefore, we discard this model as unsuitable. Another possibility would then
/// be to define Background inside Layer, and not in LayerData:
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     // ...
/// };
///
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
///
///     Background background_; // BAD
/// };
/// \endcode
///
/// But this is not suitable for two reasons:
///
///   - It makes the semantics less clean and the code less readable. Indeed,
///     the background entity really is part of the data of the layer, and
///     therefore this information should appear in LayerData
///
///   - Calling setData() is now not enough to change and copy *all* the data
///     stored in the layer.
///
/// Therefore, the general approach that we adopt is to store the Background
/// object inside the LayerData object. This means that LayerData is not "pure"
/// data anymore (it has QObject has a dependency), but is the better trade-off
/// between convenience, maintainability, and proper semantics. Here is how
/// we would naively do it:
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     Background background_; // BAD
/// };
///
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
/// };
/// \endcode
///
/// However, this doesn't work because Background inherits from QObject and
/// therefore disables the copy and assignement operator, which makes
/// LayerData not copyable as well, while we do want LayerData to have value
/// semantics. Instead, we could do:
///
/// \code
/// class LayerData // BAD
/// {
///     bool isVisible = true;
///     Background * background;
/// };
///
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
/// };
/// \endcode
///
/// But this is bad as well since copying the LayerData would copy the address
/// of the background pointer which is not the semantics we want. Also, the
/// pointer might be uninitialized or null, which doesn't make sense: each
/// layer do have a valid background, always. To fix this, one would have to
/// define an appropriate constructor, copy/assignement operator, and
/// destructor, and a move constructor (possibly implemented using the
/// copy-and-swap idiom):
///
/// \code
/// class LayerData // OK-ish
/// {
///     bool isVisible = true;
///     Background * background;
///
///     LayerData()
///     {
///         background = new Background();
///     }
///
///     LayerData(const LayerData & other) : LayerData()
///     {
///         background->setData(other.background->data());
///     }
///
///     ~LayerData()
///     {
///         delete background;
///     }
///
///     friend void swap(LayerData & first, LayerData & second)
///     {
///         swap(first.isVisible, second.isVisible);
///         swap(first.background, second.background);
///     }
///
///     LayerData & operator=(LayerData other)
///     {
///         swap(*this, other);
///         return this;
///     }
///
///     LayerData(LayerData && other) : LayerData()
///     {
///         swap(*this, other);
///     }
/// };
///
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
/// };
/// \endcode
///
/// This works perfectly, but however, there is a lot of boileplate to add
/// which is bug-prone. It is very easy to forget to update one of the
/// constructor/destructor/copy/assignement/destructor, and manually using
/// new and delete is bad practice.
///
/// Therefore, as a convenience, we introduce the DataObjectPtr smart pointer
/// which implements the above semantics automatically:
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     DataObjectPtr<Background> background; // GOOD :-)
/// };
///
/// class Layer: public DataObject<LayerData>
/// {
/// private:
///     Q_OBJECT
///     Q_DISABLE_COPY(Layer)
/// };
/// \endcode
///
/// DataObjectPtr<Background> takes care of allocating on the heap a Background
/// object on construction, copying its data (but not its address) on copy and
/// assignment, and deallocating the Background object on destruction.
///
/// Therefore, the default constructor/destructor/copy/assignement/destructor
/// of LayerData now exactly implements the semantics that we want, without
/// having to do any explicit work for it.
///
///
/// <H1> Observed data objects </H1>
///
/// Sometimes, a data object should "observe" another data object, i.e. use
/// it as a reference but without having the ownership of it. In this case,
/// simply use a raw pointer to the data object:
///
/// \code
/// class LayerData
/// {
///     bool isVisible = true;
///     DataObjectPtr<Background> background;
///
///     std::vector<DataObjectPtr<Layer>> childLayers;
///     Layer * parentLayer = nullptr;
/// };
/// \endcode
///
/// However, be aware that this means that the address is what is being copied
/// by setData(). It may or may not be what you want. For instance, if you
/// duplicate a layer, then the parent of the new layer (layer2) is indeed the
/// same (its address/identity is equal) as the original layer (layer1).
/// However, the parentLayer of all the childLayers of layer2 needs now to
/// point to layer2 instead of to layer1 (and so on recursively). It is the
/// responsibility of the Layer class to modify the LayerData in a way which is
/// appropriate for every given operation, which may involve remapping observed
/// pointers. The LayerData class does not have enough information to do this.
///
template <class TData>
class DataObject: public DataObjectBase
{
private:
    Q_DISABLE_COPY(DataObject)

public:
    DataObject() {}

    const TData & data() const { return data_; }
    void setData(const TData & data) { data_ = data; emit changed(); }

protected:
    TData data_;
};


/**************************** DataObjectPtr **********************************/

/// \class DataObjectPtr
/// \brief A smart pointer to implement the semantics for owned sub-objects
///
/// See the documentation of DataObject for more information
///
template <class T>
class DataObjectPtr
{
public:
    // Object lifetime management
    DataObjectPtr()
    {
        p_ = new T();
    }
    ~DataObjectPtr()
    {
        delete p_;
    }

    // Copy semantics
    DataObjectPtr(const DataObjectPtr<T> & other) : DataObjectPtr()
    {
        p_->setData(other.p_->data());
    }
    friend void swap(DataObjectPtr<T> & first, DataObjectPtr<T> & second)
    {
        swap(first.p_, second.p_);
    }
    DataObjectPtr<T> & operator=(DataObjectPtr<T> other)
    {
        swap(*this, other);
        return this;
    }
    DataObjectPtr<T>(DataObjectPtr<T> && other) : DataObjectPtr()
    {
        swap(*this, other);
    }

    // Get underlying pointer
    T * get() const { return p_; }
    T & operator*() const { return *p_; }
    T * operator->() const { return p_; }

    // Compare with other pointers
    bool operator==(const DataObjectPtr<T> & other)
    {
        return p_ == other.p_;
    }
    bool operator!=(const DataObjectPtr<T> & other)
    {
        return p_ != other.p_;
    }

private:
    T * p_;
};

#endif // DATAOBJECT_H
