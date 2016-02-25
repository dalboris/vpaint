// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPERATOR_H
#define OPENVAC_OPERATOR_H

namespace OpenVAC
{

class VAC;

class Operator
{
public:
    // Constructor
    Operator(VAC * vac);

    // Returns the VAC this operator is bound to
    VAC * vac() const;

    // Checks whether the operation is valid
    bool isValid();

    // Applies operation to VAC
    bool apply();

protected:
    // Methods that must be implemented by derived classes
    virtual bool isValid_()=0;
    virtual void compute_()=0;

private:
    VAC * vac_;

    // Validity
    bool isValidated_;
    bool valid_;

    // Computation
    bool isComputed_;

    // Application
    void apply_();

};

}

#endif // OPENVAC_OPERATOR_H
