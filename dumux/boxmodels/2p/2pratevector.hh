/*****************************************************************************
 *   Copyright (C) 2011 by Andreas Lauser                                    *
 *   Institute for Modelling Hydraulic and Environmental Systems             *
 *   University of Stuttgart, Germany                                        *
 *   email: <givenname>.<name>@iws.uni-stuttgart.de                          *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
/*!
 * \file
 *
 * \brief Implements a vector representing mass rates.
 *
 * This class is basically a Dune::FieldVector which can be set using
 * either mass, molar or volumetric rates.
 */
#ifndef DUMUX_BOX_2P_RATE_VECTOR_HH
#define DUMUX_BOX_2P_RATE_VECTOR_HH

#include <dune/common/fvector.hh>

#include <dumux/common/valgrind.hh>
#include <dumux/material/constraintsolvers/ncpflash.hh>

#include "2pvolumevariables.hh"

namespace Dumux
{
/*!
 * \ingroup 2PModel
 *
 * \brief Implements a vector representing mass rates.
 *
 * This class is basically a Dune::FieldVector which can be set using
 * either mass, molar or volumetric rates.
 */
template <class TypeTag>
class TwoPRateVector
    : public Dune::FieldVector<typename GET_PROP_TYPE(TypeTag, Scalar),
                               GET_PROP_VALUE(TypeTag, NumEq) >
{
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    enum { numEq = GET_PROP_VALUE(TypeTag, NumEq) };
    typedef Dune::FieldVector<Scalar, numEq> ParentType;

    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;

    typedef typename GET_PROP_TYPE(TypeTag, TwoPIndices) Indices;
    enum { conti0EqIdx = Indices::conti0EqIdx };

    enum { numPhases = GET_PROP_VALUE(TypeTag, NumPhases) };
    enum { numComponents = GET_PROP_VALUE(TypeTag, NumComponents) };
    typedef Dune::FieldVector<Scalar, numComponents> ComponentVector;

    typedef typename GET_PROP_TYPE(TypeTag, VolumeVariables) EnergyModule;

public:
    /*!
     * \brief Default constructor
     */
    TwoPRateVector()
        : ParentType()
    { Valgrind::SetUndefined(*this); };

    /*!
     * \brief Constructor with assignment from scalar
     */
    TwoPRateVector(Scalar value)
        : ParentType(value)
    { };

    /*!
     * \brief Copy constructor
     */
    TwoPRateVector(const TwoPRateVector &value)
        : ParentType(value)
    { };

    /*!
     * \brief Set a mass rate of the conservation quantities.
     *
     * Enthalpy is _not_ taken into account seperately here. This
     * means that it must be set to the desired value in the
     * parameter.
     */
    void setMassRate(const ParentType &value)
    {
        ParentType::operator=(value);
    };

    /*!
     * \brief Set a molar rate of the conservation quantities.
     *
     * Enthalpy is _not_ taken into account seperately here. This
     * means that it must be set to the desired value in the
     * parameter.
     */
    void setMolarRate(const ParentType &value)
    {
        // convert to mass rates
        ParentType massRate(value);
        for (int compIdx = 0; compIdx < numComponents; ++compIdx)
            massRate[conti0EqIdx + compIdx] *= FluidSystem::molarMass(compIdx);
        
        // set the mass rate
        setMassRate(massRate);
    };

    /*!
     * \brief Set a volumetric rate of a phase.
     *
     * Enthalpy is taken into account here.
     */
    template <class FluidState>
    void setVolumetricRate(const FluidState &fluidState, 
                           int phaseIdx, 
                           Scalar volume)
    {
        for (int compIdx = 0; compIdx < numComponents; ++compIdx)
            (*this)[conti0EqIdx + compIdx] = 
                fluidState.density(phaseIdx, compIdx)
                * fluidState.massFraction(phaseIdx, compIdx)
                * volume;
        
        EnergyModule::setEnthalpyRate(*this, fluidState, phaseIdx, volume);
    };
};

} // end namepace

#endif