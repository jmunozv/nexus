// ----------------------------------------------------------------------------
// nexus | RandomUtils.h
//
// This class defines commonly used functions.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include <G4VisAttributes.hh>
#include <Randomize.hh>
#include <G4ThreeVector.hh>
#include "CLHEP/Units/SystemOfUnits.h"


#ifndef RAND_U_H
#define RAND_U_H

namespace nexus {
  inline G4double RandomEnergy(G4double energy_max,G4double energy_min) //const
  { if (energy_max == energy_min){
    return energy_min;}
  else{
    return (G4UniformRand()*(energy_max - energy_min) + energy_min);}}

  inline G4ThreeVector Direction(G4double costheta_min, G4double costheta_max,
                                 G4double phi_min, G4double phi_max)
  //phi_max and phi_min are intended to be angles in the range [0,2*pi].
  {G4bool dir = false;
    while (dir == false) {
      G4double cosTheta  = 2.*G4UniformRand()-1.;
      if (cosTheta > costheta_min && cosTheta < costheta_max){
	       G4double sinTheta2 = 1. - cosTheta*cosTheta;
	       if (sinTheta2 < 0.)  sinTheta2 = 0.;
	       G4double sinTheta  = std::sqrt(sinTheta2);
	       G4double phi = CLHEP::twopi*G4UniformRand();
	       if (phi > phi_min && phi < phi_max){
            return G4ThreeVector (sinTheta*std::cos(phi),
		                              sinTheta*std::sin(phi),
                                  cosTheta).unit();
	       }
       }
     }
    }

}  // end namespace nexus

#endif
