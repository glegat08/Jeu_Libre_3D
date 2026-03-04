#include "Weapon.h"

const KGR::GameLib::WeaponData& KGR::GameLib::WeaponComponent::GetCurrentWeaponData() const
{
	switch (current)
	{
	case WeaponType::Shotgun:
		return shotgun;
	case WeaponType::Auto:
		return autoRifle;
	case WeaponType::Sniper:
		return sniper;
	default:
		return autoRifle;
	}
}

void KGR::GameLib::WeaponComponent::SwitchWeapon(WeaponType newType)
{
	current = newType;
	currentAmmo = GetCurrentWeaponData().maxAmmo;
	cooldown = 0.0f;
}