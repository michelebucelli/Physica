/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "physics_gfx.h"

int shadowAlpha = 0xFF;

void printEntity(SDL_Renderer* target, entity* e, Uint32 color, int x, int y, bool border){
	if (e->eType == etype_box){
		box* b = (box*) e;
		
		Sint16 px[] = {floor(b->point[0].x) + x, floor(b->point[1].x) + x, floor(b->point[2].x) + x, floor(b->point[3].x) + x};
		Sint16 py[] = {floor(b->point[0].y) + y, floor(b->point[1].y) + y, floor(b->point[2].y) + y, floor(b->point[3].y) + y};
		
		filledPolygonColor(target, px, py, 4, color);
		if (border){
			for ( int i = 0; i < 3; i++ )
				aalineColor ( target, px[i], py[i], px[i+1], py[i+1], color );
				
			aalineColor ( target, px[0], py[0], px[3], py[3], color );
			
		}// aapolygonColor(target, px, py, 4, 0xFF0000FF);
	}
}

void printScene(SDL_Renderer* target, scene* s, int x, int y, bool hidden){
	list <entity*> :: iterator i;
	list <phLink*>::iterator j;
	
	SDL_Rect vp;
	SDL_RenderGetViewport(target, &vp);
	
	vect offset (x, y);
	
	//Prints shadows
	
	for (list<entity*>::iterator e = s->entities.begin(); e != s->entities.end(); e++){
		if ((*e)->light) for (i = s->entities.begin(); i != s->entities.end(); i++){
			if ((*i)->eType != etype_box || !(*i)->print || i == e) continue;
			
			box* b = (box*) *i;
			
			vect light = (*e)->position;
			vect max = b->point[0] - light, min = b->point[0] - light;
			int maxIndex = 0, minIndex = 0;
			
			Uint32 lightColor = (shadowAlpha << 24) + COMPLEMENTARY((*e)->lightColor);
			vect d = (*i)->position - light;
			
			for (int n = 0; n < 4; n++){
				vect r = b->point[n] - light;
				
				double maxAngle = max.angle();
				double minAngle = min.angle();
				double rAngle = r.angle();
				
				double dMax = (rAngle > PI - rAngle) == (maxAngle > PI - maxAngle) ? rAngle - maxAngle : PI - rAngle + maxAngle;
				double dMin = (rAngle > PI - rAngle) == (minAngle > PI - minAngle) ? rAngle - minAngle : PI - rAngle + minAngle;
				
				if (dMax > 0){ max = r; maxIndex = n; }
				if (dMin < 0){ min = r; minIndex = n; }
			}
			
			vect shadowPoints[4];
			
			shadowPoints[0] = max + light;
			shadowPoints[1] = min + light;
			
			double a = intersection (light + offset, min, vect(0,0), vect(1,0));
			shadowPoints[2] = light + offset + min * a;
			
			int p2 = 0;
			
			vect vpPoints[4] = { vect(0,0), vect(vp.w, 0), vect(vp.w, vp.h), vect(0, vp.h) };
			
			for (int n = 1; n < 4; n++){
				
				vect corner, side;
				
				corner = vpPoints[n];
				
				if (n == 1) side = vect (0, 1); 
				if (n == 2) side = vect (-1, 0);
				if (n == 3) side = vect (0, -1);
				
				double b = intersection (light + offset, min, corner, side);
				
				if (b >= 0 && (b < a || a < 0)){
					a = b;
					shadowPoints[2] = light + offset + min * a;
					p2 = n;
				}
			}
			
			a = intersection (light + offset, max, vect(0,0), vect(1,0));
			shadowPoints[3] = light + offset + max * a;
			
			int p3 = 0;
			
			for (int n = 1; n < 4; n++){
				
				vect corner, side;
				
				corner = vpPoints[n];
				
				if (n == 1) side = vect (0, 1); 
				if (n == 2) side = vect (-1, 0);
				if (n == 3) side = vect (0, -1);
				
				double b = intersection (light + offset, max, corner, side);
				
				if (b >= 0 && (b < a || a < 0)){
					a = b;
					shadowPoints[3] = light + offset + max * a;
					p3 = n;
				}				
			}
			
			switch (p3 - p2){
				case 0:
				{
					Sint16 spX[4] = { shadowPoints[0].x + x, shadowPoints[1].x + x, shadowPoints[2].x, shadowPoints[3].x};
					Sint16 spY[4] = { shadowPoints[0].y + y, shadowPoints[1].y + y, shadowPoints[2].y, shadowPoints[3].y};
				
					filledPolygonColor(target, spX, spY, 4, lightColor);
				}
				
				break;
				
				
				case 1: case -1:
				{
					Sint16 spX[5] = { shadowPoints[0].x + x, shadowPoints[1].x + x, shadowPoints[2].x, vpPoints[int(double(p2 + p3) / 2 + 0.5)].x, shadowPoints[3].x};
					Sint16 spY[5] = { shadowPoints[0].y + y, shadowPoints[1].y + y, shadowPoints[2].y, vpPoints[int(double(p2 + p3) / 2 + 0.5)].y, shadowPoints[3].y};
					
					filledPolygonColor(target, spX, spY, 5, lightColor);
				}
				
				break;
				
				case 2:
				case -2:
				{
					Sint16 spX[6] = { shadowPoints[1].x + x, shadowPoints[0].x + x, shadowPoints[3].x, vpPoints[(p3 + 1) % 4].x, vpPoints[(p3 + 2) % 4].x, shadowPoints[2].x};
					Sint16 spY[6] = { shadowPoints[1].y + y, shadowPoints[0].y + y, shadowPoints[3].y, vpPoints[(p3 + 1) % 4].y, vpPoints[(p3 + 2) % 4].y, shadowPoints[2].y};
					
					filledPolygonColor(target, spX, spY, 6, lightColor);
				}
				
				break;
				
				case -3:
				{
					Sint16 spX[7] = { shadowPoints[1].x + x, shadowPoints[0].x + x, shadowPoints[3].x, vpPoints[(p3 + 1) % 4].x, vpPoints[(p3 + 2) % 4].x, vpPoints[(p3 + 3) % 4].x, shadowPoints[2].x};
					Sint16 spY[7] = { shadowPoints[1].y + y, shadowPoints[0].y + y, shadowPoints[3].y, vpPoints[(p3 + 1) % 4].y, vpPoints[(p3 + 2) % 4].y, vpPoints[(p3 + 3) % 4].y, shadowPoints[2].y};
					
					filledPolygonColor(target, spX, spY, 7, lightColor);
				}
				
				break;
				
				case 3:
				{
					Sint16 spX[5] = { shadowPoints[0].x + x, shadowPoints[1].x + x, shadowPoints[2].x, vpPoints[0].x, shadowPoints[3].x};
					Sint16 spY[5] = { shadowPoints[0].y + y, shadowPoints[1].y + y, shadowPoints[2].y, vpPoints[0].y, shadowPoints[3].y};
					
					filledPolygonColor(target, spX, spY, 5, lightColor);
				}
				
				break;
			}
		}
	}
	
	//Prints entities
	
	for (i = s->entities.begin(); i != s->entities.end(); i++)
		if ((*i)->print) printEntity(target, *i, (*i)->color + 0xFF000000, x, y);
		else if (hidden) printEntity(target, *i, (*i)->color + 0x7F000000, x, y, false);

	//Prints links
	//The link gets printed only if both linked entities are visible
		
	for (j = s->links.begin(); j != s->links.end(); j++){
		if (!(*j)->a || !(*j)->b) continue;
		
		if ((((*j)->a->print && (*j)->b->print)) || hidden){
			vect ap = (*j)->a->position + (*j)->a_point.rotate((*j)->a->theta);
			vect bp = (*j)->b->position + (*j)->b_point.rotate((*j)->b->theta);
			
			lineColor(target, x + ap.x, y + ap.y, x + bp.x, y + bp.y, 0xFF606060);
		}
	}
}
