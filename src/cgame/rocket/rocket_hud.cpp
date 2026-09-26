/*
===========================================================================

Unvanquished GPL Source Code
Copyright (C) 2012 Unvanquished Developers

This file is part of the Unvanquished GPL Source Code (Unvanquished Source Code).

Unvanquished Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Unvanquished Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Unvanquished Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Unvanquished Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the Unvanquished
Source Code.  If not, please request a copy in writing from id Software at the address
below.

If you have questions concerning this license or the applicable additional terms, you
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville,
Maryland 20850 USA.

===========================================================================
*/

#include "common/Common.h"
#include "rocket.h"
#include "../cg_local.h"

struct HudUnit
{
	HudUnit( Rml::ElementDocument *doc ) : unit( doc )
	{
		load = true;
	}

	Rml::ElementDocument *unit; // Element document that holds the unit
	bool load; // Whether to load or not
};

using RocketHud = std::list<HudUnit>;
RocketHud *activeHud = nullptr;
std::vector<RocketHud> huds;


// Initialized to WP_NUM_WEAPONS, which may be different based on different mods
void Rocket_InitializeHuds( int size )
{
	huds.clear();

	for ( int i = 0; i < size; ++i )
	{
		huds.emplace_back();
	}

	activeHud = nullptr;
}

void Rocket_LoadUnit( const char *path )
{
	Rml::ElementDocument *document = hudContext->LoadDocument( path );

	Rml::ElementDocument* other;

	if ( document )
	{
		Rocket_SetDocumentScale( *document );
		hudContext->PullDocumentToFront( document );

		// Close any other documents which may have the same ID
		other = hudContext->GetDocument( document->GetId() );
		if ( other && other != document )
		{
			other->Close();
		}

		document->Hide();
	}
}

void Rocket_AddUnitToHud( int weapon, const char *id )
{
	if ( id && *id )
	{
		Rml::ElementDocument *doc = hudContext->GetDocument( id );
		if ( doc )
		{
			huds[ weapon ].emplace_back( doc );
		}
	}
}

// See if unit exists in a hud
static HudUnit *findUnit( RocketHud *in, HudUnit &find )
{
	for ( RocketHud::iterator it = in->begin(); it != in->end(); ++it )
	{
		if ( it->unit == find.unit )
		{
			return &*it;
		}
	}

	return nullptr;
}

void Rocket_ShowHud( int weapon )
{
	RocketHud *currentHud = &huds[ weapon ];

	if ( activeHud )
	{
		for ( RocketHud::iterator it = activeHud->begin(); it != activeHud->end(); ++it )
		{
			HudUnit *unit;
			// Check if the new HUD needs this unit
			if ( ( unit = findUnit( currentHud, *it ) ) )
			{
				unit->load = false;

				continue;
			}

			// Nope. Doesn't need it
			it->unit->Hide();
		}
	}

	for ( RocketHud::iterator it = currentHud->begin(); it != currentHud->end(); ++it )
	{
		if ( it->load )
		{
			it->unit->Show();
		}

		// No matter what, reset load status
		it->load = true;
	}

	activeHud = currentHud;
}

void Rocket_ClearHud( unsigned weapon )
{
	if ( weapon < huds.size() )
	{
		huds[ weapon ].clear();
	}
}

void Rocket_ShowScoreboard( const char *name, bool show, bool intermission, bool intermissionIntro )
{
	Rml::ElementDocument* doc = hudContext->GetDocument( name );

	if ( doc )
	{
		Rml::Element* scoreboard = doc->GetElementById( "scoreboard" );

		if ( scoreboard )
		{
			scoreboard->SetClass( "intermission", intermission );
			scoreboard->SetClass( "intermission-intro", intermissionIntro );
			scoreboard->SetClass( "aliens-win", ui_winner.Get() == "Aliens Win" );
			scoreboard->SetClass( "humans-win", ui_winner.Get() == "Humans Win" );
			scoreboard->SetProperty( "background-color", intermissionIntro ? "rgba(0%, 0%, 0%, 0%)" : "rgba(10%, 10%, 10%, 70%)" );

			for ( Rml::Element* parent = scoreboard->GetParentNode(); parent; parent = parent->GetParentNode() )
			{
				if ( parent->GetTagName() == "window" )
				{
					parent->SetClass( "intermission-intro", intermissionIntro );
					parent->SetProperty( "background-color", intermissionIntro ? "rgba(0%, 0%, 0%, 0%)" : "rgba(10%, 10%, 10%, 95%)" );
					parent->SetProperty( "border-left", intermissionIntro ? "0" : ".17em rgba(0%, 0%, 0%, 35%)" );
					parent->SetProperty( "border-right", intermissionIntro ? "0" : ".17em rgba(0%, 0%, 0%, 35%)" );
					parent->SetProperty( "border-top", intermissionIntro ? "0" : ".17em rgba(0%, 0%, 0%, 35%)" );
					parent->SetProperty( "border-bottom", intermissionIntro ? "0" : ".17em rgba(0%, 0%, 0%, 35%)" );
					break;
				}
			}

			const char* introDisplay = intermissionIntro ? "block" : "none";
			const char* resultsDisplay = intermissionIntro ? "none" : "block";

			if ( Rml::Element* element = doc->GetElementById( "intro-winner" ) )
			{
				element->SetProperty( "display", introDisplay );
			}

			if ( Rml::Element* element = doc->GetElementById( "scoreboard-winner" ) )
			{
				element->SetProperty( "display", resultsDisplay );
			}

			if ( Rml::Element* element = doc->GetElementById( "scoreboard-content" ) )
			{
				element->SetProperty( "display", resultsDisplay );
			}
		}

		if ( show )
		{
			doc->Show();
		}
		else
		{
			doc->Hide();
		}
	}
}
