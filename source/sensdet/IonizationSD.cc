// ----------------------------------------------------------------------------
//  $Id: IonizationSD.cc 2025 2009-07-17 11:11:39Z jmalbos $
//
//  Author : J Martin-Albo <jmalbos@ific.uv.es>
//  Created: 27 Apr 2009
//
//  Copyright (c) 2009, 2010 NEXT Collaboration
// ----------------------------------------------------------------------------

#include "IonizationSD.h"

#include "IonizationHit.h"


#include <G4SDManager.hh>
#include <G4Step.hh>





namespace nexus {
  
  
  IonizationSD::IonizationSD(const G4String& name, const G4String& det_name):
    G4VSensitiveDetector(name), _det_name(det_name)
  {
    collectionName.insert("IonizationHitsCollection");
  }
  
  
  
  void IonizationSD::Initialize(G4HCofThisEvent* hce)
  {
    // Create a collection of ionization hits (since the collection
    // won't be used outside this class object, it isn't added
    // to the G4HCofThisEvent object)
    _IHC = 
      new IonizationHitsCollection(SensitiveDetectorName, collectionName[0]);

    G4int hcid =
      G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection(hcid, _IHC);
  }
  
  
  
  G4bool IonizationSD::ProcessHits(G4Step* step, G4TouchableHistory*)
  {
    G4double edep = step->GetTotalEnergyDeposit();
    
    // Discard steps where no energy was deposited in the detector
    if (edep <= 0.) return false;

    // Create a hit and set its properties
    IonizationHit* hit = new IonizationHit();
    hit->SetTrackID(step->GetTrack()->GetTrackID());
    hit->SetTime(step->GetTrack()->GetGlobalTime());
    hit->SetEnergyDeposit(edep);
    hit->SetPosition(step->GetPostStepPoint()->GetPosition());
    
    // // Add hit to collection
    _IHC->insert(hit);

    return true;
  }
  
  

  void IonizationSD::EndOfEvent(G4HCofThisEvent*)
  {
//    bhep::event& bevt = bhep::bhep_svc::instance()->get_event();

    G4double total_energy = 0.;

    for (G4int i=0; i<_IHC->entries(); i++) {
      
      IonizationHit* hit = (*_IHC)[i];
      
      total_energy += hit->GetEnergyDeposit();

//      bhep::hit* bhit = hit->ToBhep(_det_name);
//      bevt.add_true_hit(_det_name, bhit);
    }

    // G4String prop_name = "EnergyDep";
    // if (_det_name != "")
    //   prop_name += "_" + _det_name;

    // bevt.add_property(prop_name, total_energy);
  }
  
  
} // end namespace nexus

