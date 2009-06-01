/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2006 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Id$
 *
 */

#include "ProjectileServer.h"
#include "Interface.h"
#include "ResourceMgr.h"
#include "SymbolMgr.h"
#include "ProjectileMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_PROJ_IDX  1023

ProjectileServer::ProjectileServer()
{
	projectilecount = -1;
	projectiles = NULL;
	explosioncount = -1;
	explosions = NULL;
}

ProjectileServer::~ProjectileServer()
{
	if (projectiles) {
		delete[] projectiles;
	}
	if (explosions) {
		delete[] explosions;
	}
}

Projectile *ProjectileServer::CreateDefaultProjectile(unsigned int idx)
{
	Projectile *pro = new Projectile();
	int strlength = (ieByte *) (&pro->Extension)-(ieByte *) (&pro->Type);
	memset(&pro->Type, 0, strlength );

	//take care, this projectile is not freed up by the server
	if(idx==(unsigned int) ~0 ) {
		return pro;
	}

	projectiles[idx].projectile = pro;
	pro->SetIdentifiers(projectiles[idx].resname, idx);
	return ReturnCopy(idx);
}

//this function can return only projectiles listed in projectl.ids
Projectile *ProjectileServer::GetProjectileByName(const ieResRef resname)
{
	if (!core->IsAvailable(IE_PRO_CLASS_ID)) {
		return NULL;
	}
	unsigned int idx=GetHighestProjectileNumber();
	while(idx--) {
		if (!strnicmp(resname, projectiles[idx].resname,8) ) {
			return GetProjectile(idx);
		}
	}
	return NULL;
}

Projectile *ProjectileServer::GetProjectileByIndex(unsigned int idx)
{
	if (!core->IsAvailable(IE_PRO_CLASS_ID)) {
		return NULL;
	}
	if (idx>=GetHighestProjectileNumber()) {
		return GetProjectile(0);
	}

	return GetProjectile(idx);
}

Projectile *ProjectileServer::ReturnCopy(unsigned int idx)
{
	Projectile *pro = new Projectile();
	int strlength = (ieByte *) (&pro->Extension)-(ieByte *) (&pro->Type);
	Projectile *old = projectiles[idx].projectile;
	memcpy(&pro->Type, &old->Type, strlength );
	//FIXME: copy extension data too, or don't alter the extension
	if (old->Extension) {
		pro->Extension = old->Extension;
	}
	pro->SetIdentifiers(projectiles[idx].resname, idx);
	return pro;
}

Projectile *ProjectileServer::GetProjectile(unsigned int idx)
{
	if (projectiles[idx].projectile) {
		return ReturnCopy(idx);
	}
	DataStream* str = core->GetResourceMgr()->GetResource( projectiles[idx].resname, IE_PRO_CLASS_ID );
	ProjectileMgr* sm = ( ProjectileMgr* ) core->GetInterface( IE_PRO_CLASS_ID );
	if (sm == NULL) {
		delete ( str );
		return CreateDefaultProjectile(idx);
	}
	if (!sm->Open( str, true )) {
		core->FreeInterface( sm );
		return CreateDefaultProjectile(idx);
	}
	Projectile *pro = new Projectile();
	projectiles[idx].projectile = pro;
	pro->SetIdentifiers(projectiles[idx].resname, idx);

	sm->GetProjectile( pro );
	core->FreeInterface( sm );

	pro->autofree = true;
	return ReturnCopy(idx);
}

int ProjectileServer::InitExplosion()
{
	if (explosioncount>=0) {
		return explosioncount;
	}

	AutoTable explist("areapro");
	if (explist) {
		explosioncount = 0;

		unsigned int rows = (unsigned int) explist->GetRowCount();
		explosioncount = rows;
		explosions = new ExplosionEntry[rows];

		while(rows--) {
			int i;

			for(i=0;i<AP_RESCNT;i++) {
				strnuprcpy(explosions[rows].resources[i], explist->QueryField(rows, i), 8);
			}
			//using i so the flags field will always be after the resources
			explosions[rows].flags = atoi(explist->QueryField(rows,i));
		}
	}
	return explosioncount;
}

unsigned int ProjectileServer::GetHighestProjectileNumber()
{
	if (projectilecount>=0) {
		return (unsigned int) projectilecount;
	}

	unsigned int resource = core->LoadSymbol("projectl");
	SymbolMgr *projlist = core->GetSymbol(resource);
	if (projlist) {
		projectilecount = 0;

		unsigned int rows = (unsigned int) projlist->GetSize();
		while(rows--) {
			unsigned int value = projlist->GetValueIndex(rows);
			if (value>MAX_PROJ_IDX) {
				value = MAX_PROJ_IDX;
				printMessage("ProjectileServer","Too high projectilenumber\n", YELLOW);
			}
			if (value>(unsigned int) projectilecount) {
				projectilecount = (unsigned int) value+1;
			}
		}
		projectiles = new ProjectileEntry[projectilecount];

		rows = (unsigned int) projlist->GetSize();
		while(rows--) {
			strnuprcpy(projectiles[ projlist->GetValueIndex(rows)].resname, projlist->GetStringIndex(rows), 8);
		}
		core->DelSymbol(resource);
	} else {
		projectilecount=1;
		projectiles = new ProjectileEntry[projectilecount];
	}
	return (unsigned int) projectilecount;
}

//return various flags for the explosion type
int ProjectileServer::GetExplosionFlags(unsigned int idx)
{
	if (explosioncount==-1) {
		if (InitExplosion()<0) {
			printMessage("ProjectileServer","Problem with explosions\n", RED);
			explosioncount=0;
		}
	}
	if (idx>=(unsigned int) explosioncount) {
		return 0;
	}

	return explosions[idx].flags;
}

ieResRef const *ProjectileServer::GetExplosion(unsigned int idx, int type)
{
	if (explosioncount==-1) {
		if (InitExplosion()<0) {
			printMessage("ProjectileServer","Problem with explosions\n", RED);
			explosioncount=0;
		}
	}
	if (idx>=(unsigned int) explosioncount) {
		return NULL;
	}
	ieResRef const *ret = NULL;

	ret = &explosions[idx].resources[type];
	if (ret && *ret[0]=='*') ret = NULL;

	return ret;
}
