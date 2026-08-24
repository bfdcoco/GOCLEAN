#include "GEnemy/Components/MaidenGhost.h"
#include "GEnemy/EvidenceBehaviors/Components/LeaveBloodFootstep.h"
#include "GEnemy/EvidenceBehaviors/Components/LeaveFrost.h"
#include "GEnemy/EvidenceBehaviors/Components/SpillWaterBucket.h"
#include "GEnemy/EvidenceBehaviors/Components/RestoreWaste.h"

AMaidenGhost::AMaidenGhost()
{
	StatsComp->InitGhostStats(3.0f, 4.5f, 10.0f, 110.0f, 60.0f, 0.0f, 7.0f);
}

void AMaidenGhost::BeginPlay()
{
	Super::BeginPlay();

	EvidenceBehaviors.Add(NewObject<ULeaveFrost>(this));
	EvidenceBehaviors.Add(NewObject<ULeaveBloodFootstep>(this));
	EvidenceBehaviors.Add(NewObject<USpillWaterBucket>(this));
	EvidenceBehaviors.Add(NewObject<URestoreWaste>(this));
}