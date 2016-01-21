#ifndef VERSION_H
#define VERSION_H

#include <QString>

class Version
{
public:
    Version(short major = 0, short minor = 0, short patch = 0): major_(major), minor_(minor), patch_(patch) {}
    Version(QString str);

    QString toString();

    short getMajor() const;
    short getMinor() const;
    short getPatch() const;

    void setMajor(short val);
    void setMinor(short val);
    void setPatch(short val);

    // Comparison operators
    inline bool operator==(const Version& other) { return getMajor() == other.getMajor() && getMinor() == other.getMinor() && getPatch() == other.getPatch(); }
    bool operator< (const Version& other) const;
    bool operator!=(const Version& other) { return !((*this)==other); }
    bool operator> (const Version& other) { return other<(*this); }
    bool operator<=(const Version& other) { return !(other<(*this)); }
    bool operator>=(const Version& other) { return !((*this)<other); }

private:
    short major_, minor_, patch_;
};

#endif // VERSION_H
