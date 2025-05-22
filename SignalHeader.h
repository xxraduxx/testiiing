// 시그널 타입 열거
enum SignalTypeEnum {
	STE_DnNullSignal = 0,
	STE_Input = 1,
	STE_CanMove = 2,
	STE_Hit = 4,
	STE_VelocityAccel = 5,
	STE_Jump = 6,
	STE_State = 9,
	STE_CanHit = 10,
	STE_Sound = 11,
	STE_Particle = 12,
	STE_SendAction_Weapon = 14,
	STE_Projectile = 15,
	STE_EnvironmentEffect = 16,
	STE_ShowWeapon = 17,
	STE_Destroy = 18,
	STE_AlphaBlending = 19,
	STE_CanDefense = 20,
	STE_AttachTrail = 21,
	STE_CustomAction = 23,
	STE_MotionSpeed = 25,
	STE_SuperAmmor = 27,
	STE_InputHasPassiveSkill = 29,
	STE_CameraEffect_Shake = 31,
	STE_ResetVelocity = 32,
	STE_FX = 33,
	STE_PickupItem = 34,
};

// 시그널 별 스트럭쳐
struct DnNullSignalStruct {
	char cReserved1[256];
};

struct InputStruct {
	int nButton;
	char *szChangeAction;
	char cReserved1[4];
	int nBlendFrame;
	int nChangeActionFrame;
	char *szMaintenanceBoneName;
	char *szActionBoneName;
	int nEventType;
	int nMinTime;
	int nMaxTime;
	char cReserved2[216];
};

struct CanMoveStruct {
	BOOL bCanMove;
	char cReserved1[4];
	char *szActionBoneName;
	char *szMaintenanceBoneName;
	char cReserved2[240];
};

struct HitStruct {
	float fDamageProb;
	char *szTargetHitAction;
	float fDistanceMax;
	float fDistanceMin;
	float fHeightMax;
	float fHeightMin;
	float fCenterAngle;
	float fAngle;
	EtVector3 *vVelocity;
	EtVector3 *vResistance;
	BOOL bIncludeWeaponLength;
	char cReserved1[4];
	float fDurabilityDamageProb;
	int nStateCondition;
	float fStiffProb;
	EtVector3 *vOffset;
	char cReserved2[192];
};

struct VelocityAccelStruct {
	EtVector3 *vVelocity;
	EtVector3 *vResistance;
	char cReserved1[248];
};

struct JumpStruct {
	float fJumpVelocity;
	float fJumpResistance;
	BOOL bResetPrevVelocity;
	char cReserved1[244];
};

struct StateStruct {
	int nStateOne;
	int nStateTwo;
	int nStateThree;
	char cReserved1[244];
};

struct CanHitStruct {
	BOOL bHittable;
	char cReserved1[252];
};

struct SoundStruct {
	char *szFileName;
	float fVolume;
	float fRange;
	float fRollOff;
	int nSoundIndex;
	int nRandom;
	char cReserved1[232];
};

struct ParticleStruct {
	char *szFileName;
	BOOL bLoop;
	BOOL bIterator;
	EtVector3 *vPosition;
	int nParticleDataIndex;
	char cReserved1[4];
	BOOL bLinkObject;
	int nCullDistance;
	BOOL bClingTerrain;
	EtVector3 *vRotation;
	char *szBoneName;
	float fScale;
	BOOL bDefendenceParent;
	char cReserved2[204];
};

struct SendAction_WeaponStruct {
	char *szActionName;
	int nWeaponIndex;
	char cReserved1[248];
};

struct ProjectileStruct {
	int nWeaponTableID;
	EtVector3 *vOffset;
	char cReserved1[20];
	int nProjectileIndex;
	int nOrbitType;
	int nTargetType;
	int nDestroyOrbitType;
	float fSpeed;
	float fResistance;
	char cReserved2[4];
	EtVector3 *vDirection;
	char cReserved3[4];
	BOOL bIncludeMainWeaponLength;
	char cReserved4[188];
};

struct EnvironmentEffectStruct {
	int nCondition;
	char *szBoneName;
	EtVector3 *vOffset;
	char cReserved1[244];
};

struct ShowWeaponStruct {
	int nEquipIndex;
	BOOL bEnable;
	char cReserved1[248];
};

struct DestroyStruct {
	char cReserved1[256];
};

struct AlphaBlendingStruct {
	float fStartAlpha;
	float fEndAlpha;
	char cReserved1[248];
};

struct CanDefenseStruct {
	int nProb;
	char cReserved1[252];
};

struct AttachTrailStruct {
	char *szTrailTextureName;
	EtVector3 *vOffset;
	float fLifeTime;
	float fTrailWidth;
	float fMinSegment;
	int nTextureIndex;
	char cReserved1[232];
};

struct CustomActionStruct {
	char *szChangeAction;
	int nBlendFrame;
	int nChangeActionFrame;
	char *szMaintenanceBoneName;
	char *szActionBoneName;
	char cReserved1[236];
};

struct MotionSpeedStruct {
	float fFrame;
	char cReserved1[252];
};

struct SuperAmmorStruct {
	BOOL bEnable;
	int nTime;
	float fBreakDurability;
	char cReserved1[244];
};

struct InputHasPassiveSkillStruct {
	int nButton;
	int nSkillIndex;
	char *szChangeAction;
	int nBlendFrame;
	int nChangeActionFrame;
	int nEventType;
	int nMinTime;
	int nMaxTime;
	char cReserved1[224];
};

struct CameraEffect_ShakeStruct {
	float fShakeValue;
	float fRange;
	float fRolloff;
	float fBeginRatio;
	float fEndRatio;
	int nCameraEffectRefIndex;
	char cReserved1[232];
};

struct ResetVelocityStruct {
	BOOL bXAxis;
	BOOL bYAxis;
	BOOL bZAxis;
	char cReserved1[244];
};

struct FXStruct {
	char *szFileName;
	int nFXDataIndex;
	EtVector3 *vOffset;
	BOOL bLinkObject;
	BOOL bLoop;
	EtVector3 *vRotation;
	char *szBoneName;
	float fScale;
	char cReserved1[224];
};

struct PickupItemStruct {
	char cReserved1[256];
};

