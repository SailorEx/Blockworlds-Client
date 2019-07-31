#include <engine/textrender.h>
#include <engine/client.h>
#include <game/extras.h>

#include "editor.h"


int CEditor::ms_ExtrasPopupSize[NUM_EXTRAS][2] = { {0,0}, { 145, 50},{ 145, 50 },{ 145, 100 }, {45, 25},{ 60, 25 },
{145, 125}, {145, 100}, { 0, 0 },{ 85, 25 },{ 85, 25 },{ 85, 25 },{ 145, 50 },{ 145, 50 },{ 145, 50 },{ 145, 50 },
{ 145, 50 },{ 145, 50 },{ 75, 25 } ,{ 100, 25 } ,{ 95, 25 },{ 95, 25 },{ 95, 25 },{ 0, 0 },{ 145, 50 },
{ 230, 75 }, { 145, 75 } };
static const char* gs_ExtrasHeader[NUM_EXTRAS] = { 0x0, "Teleport From", "Teleport To", "Speedup", "Freeze", "Unfreeze",
"Door", "Door Handle", "", "Protectionzone", "Spawnzone", "Untouchzone", "Map Transmission", "Sell Skinmani", "Sell Gundesign", "Sell Knockouts",
"Sell Extras", "Playercount", "Hookthrough", "Hookthrough-Top", "Hooktrough-Bot", "Hookthrough-Left", "Hooktrough-Right", "", "Level Info",
"Lasergun (Type: 0=Frz 1=Unfrz 2=Explode)", "Lasergun-Trigger"
};

CLayerExtras::CLayerExtras(int w, int h)
 : CLayerTiles(w, h)
 {
	str_copy(m_aName, "Extas", sizeof(m_aName));
	m_Game = 0;
	m_Type = LAYERTYPE_EXTRAS;
	
		m_pExtrasData = new CExtrasData[m_Width * m_Height];
	mem_zero(m_pExtrasData, m_Width * m_Height * sizeof(CTile));
	}

CLayerExtras::~CLayerExtras()
 {
	delete[] m_pExtrasData;
	}


void CLayerExtras::RenderColumn(CEditor* pEditor, CUIRect View, int Index)
 {
	CUIRect Button;
	int Tile = m_pTiles[Index].m_Index;
	
		int TotalSize = 0;
	for (int i = 0; i < EXTRATILE_DATA / 2; i++)
		 TotalSize += gs_ExtrasSizes[Tile][i];
	
		if (TotalSize > EXTRATILE_DATA)
		 {
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Error %i/%i Datasize", TotalSize, EXTRATILE_DATA);
		pEditor->UI()->DoLabel(&View, aBuf, 10.0f, -1);
		return;
		}
	
		View.HSplitTop(20.0f, &Button, &View);
	pEditor->UI()->DoLabel(&Button, gs_ExtrasHeader[Tile], 10.0f, -1);
	
		int CurSize = 0;
	for (int i = 0; i < EXTRATILE_DATA / 2; i++)
		 {
		if (gs_ExtrasSizes[Tile][i] == 0 || gs_ExtrasNames[Tile][i] == 0x0/* || gs_ExtrasNames[Tile][i] == '\0'*/)
			 break;
		
			View.HSplitTop(20.0f, &Button, &View);
		Button.HSplitBottom(5.0f, &Button, 0x0);
		pEditor->UI()->DoLabel(&Button, gs_ExtrasNames[Tile][i], 8.0f, -1);
		int w = TextRender()->TextWidth(0, 8.0f, gs_ExtrasNames[Tile][i], -1);
		Button.VSplitLeft(w + 2.0f, 0x0, &Button);
		static float s_aValueBoxIDs[EXTRATILE_DATA / 2] = { };
		pEditor->DoEditBox(&s_aValueBoxIDs[i], &Button, m_pExtrasData[Index].m_aData + CurSize, gs_ExtrasSizes[Tile][i], 8.0f, &s_aValueBoxIDs[i]);
		CurSize += gs_ExtrasSizes[Tile][i];
		}
	}

int CLayerExtras::RenderEditExtra(CEditor* pEditor, CUIRect View, int Index)
{
	if (gs_ExtrasSizes[m_pTiles[Index].m_Index][0] == 0)// no arguments
		return 1;
	
		EXTRAS_TELEPORT_FROM: RenderColumn(pEditor, View, Index);
	return 0;
	}

int CLayerExtras::BrushGrab(CLayerGroup* pBrush, CUIRect Rect)
 {
	RECTi r;
	Convert(Rect, &r);
	Clamp(&r);
	
		if (!r.w || !r.h)
			return 0;
	
		   // create new layers
		CLayerExtras * pGrabbed = new CLayerExtras(r.w, r.h);
	pGrabbed->m_pEditor = m_pEditor;
	pGrabbed->m_TexID = m_TexID;
	pGrabbed->m_Image = m_Image;
	pGrabbed->m_Game = m_Game;
	pBrush->AddLayer(pGrabbed);
	
		   // copy the tiles
		for (int y = 0; y < r.h; y++)
		 for (int x = 0; x < r.w; x++)
		 pGrabbed->m_pTiles[y * pGrabbed->m_Width + x] = m_pTiles[(r.y + y) * m_Width + (r.x + x)];
	
		for (int y = 0; y < r.h; y++)
		 for (int x = 0; x < r.w; x++)
		 pGrabbed->m_pExtrasData[y * pGrabbed->m_Width + x] = m_pExtrasData[(r.y + y) * m_Width + (r.x + x)];
	
		return 1;
	}

void CLayerExtras::FillSelection(bool Empty, CLayer* pBrush, CUIRect Rect)
 {
	if (m_Readonly)
		return;
	
		Snap(&Rect);
	
		int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);
	
		CLayerExtras * pLe = static_cast<CLayerExtras*>(pBrush);
	
		for (int y = 0; y < h; y++)
		 {
		for (int x = 0; x < w; x++)
			 {
			int fx = x + sx;
			int fy = y + sy;
			
				if (fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				 continue;
			
				if (Empty)
				{
				m_pTiles[fy * m_Width + fx].m_Index = 1;
				mem_zero(&m_pExtrasData[fy * m_Width + fx], sizeof(CExtrasData));
				}
			else
				 {
				m_pTiles[fy * m_Width + fx] = pLe->m_pTiles[(y * pLe->m_Width + x % pLe->m_Width) % (pLe->m_Width * pLe->m_Height)];
				if (pBrush->m_Type != LAYERTYPE_EXTRAS)
					 mem_zero(&m_pExtrasData[fy * m_Width + fx], sizeof(CExtrasData));
				else
					 m_pExtrasData[fy * m_Width + fx] = pLe->m_pExtrasData[(y * pLe->m_Width + x % pLe->m_Width) % (pLe->m_Width * pLe->m_Height)];
				
					}
			}
		}
	m_pEditor->m_Map.m_Modified = true;
	}

void CLayerExtras::BrushDraw(CLayer* pBrush, float wx, float wy)
 {
	if (m_Readonly)
		return;
	
		   //
		CLayerExtras * l = (CLayerExtras*)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);
	
		for (int y = 0; y < l->m_Height; y++)
		 for (int x = 0; x < l->m_Width; x++)
		 {
		int fx = x + sx;
		int fy = y + sy;
		if (fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
			 continue;
		
			m_pTiles[fy * m_Width + fx] = l->m_pTiles[y * l->m_Width + x];
		if (pBrush->m_Type != LAYERTYPE_EXTRAS)
			 mem_zero(&m_pExtrasData[fy * m_Width + fx], sizeof(CExtrasData));
		else
			 m_pExtrasData[fy * m_Width + fx] = l->m_pExtrasData[y * l->m_Width + x];
		}
	m_pEditor->m_Map.m_Modified = true;
	}

void CLayerExtras::BrushFlipX()
 {
	for (int y = 0; y < m_Height; y++)
		 for (int x = 0; x < m_Width / 2; x++)
		 {
		CExtrasData TmpEx = m_pExtrasData[y * m_Width + x];
		CTile Tmp = m_pTiles[y * m_Width + x];
		m_pTiles[y * m_Width + x] = m_pTiles[y * m_Width + m_Width - 1 - x];
		m_pTiles[y * m_Width + m_Width - 1 - x] = Tmp;
		
			m_pExtrasData[y * m_Width + x] = m_pExtrasData[y * m_Width + m_Width - 1 - x];
		m_pExtrasData[y * m_Width + m_Width - 1 - x] = TmpEx;
		}
	}

void CLayerExtras::BrushFlipY()
 {
	for (int y = 0; y < m_Height / 2; y++)
		 for (int x = 0; x < m_Width; x++)
		 {
		CExtrasData TmpEx = m_pExtrasData[y * m_Width + x];
		CTile Tmp = m_pTiles[y * m_Width + x];
		m_pTiles[y * m_Width + x] = m_pTiles[(m_Height - 1 - y) * m_Width + x];
		m_pTiles[(m_Height - 1 - y) * m_Width + x] = Tmp;
		
			m_pExtrasData[y * m_Width + x] = m_pExtrasData[(m_Height - 1 - y) * m_Width + x];
		m_pExtrasData[(m_Height - 1 - y) * m_Width + x] = TmpEx;
		}
	}

void CLayerExtras::BrushRotate(float Amount)
 {
	int Rotation = (round_to_int(360.0f * Amount / (pi * 2)) / 90) % 4;   // 0=0°, 1=90°, 2=180°, 3=270°
	if (Rotation < 0)
		 Rotation += 4;
	
		if (Rotation == 1 || Rotation == 3)
		 {
		       // 90° rotation
			CTile * pTempTiles = new CTile[m_Width * m_Height];
		mem_copy(pTempTiles, m_pTiles, m_Width * m_Height * sizeof(CTile));
		CExtrasData * pTempData = new CExtrasData[m_Width * m_Height];
		mem_copy(pTempData, m_pExtrasData, m_Width * m_Height * sizeof(CExtrasData));
		CTile * pDst = m_pTiles;
		CExtrasData * pDstEx = m_pExtrasData;
		for (int x = 0; x < m_Width; ++x)
			 for (int y = m_Height - 1; y >= 0; --y, ++pDst)
			 {
			*pDst = pTempTiles[y * m_Width + x];
			*pDstEx = pTempData[y * m_Width + x];
			}
		
			int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempTiles;
		delete[] pTempData;
		}
	
		if (Rotation == 2 || Rotation == 3)
		 {
		BrushFlipX();
		BrushFlipY();
		}
	}

void CLayerExtras::Resize(int NewW, int NewH)
 {
	CTile * pNewTiles = new CTile[NewW * NewH];
	CExtrasData * pNewData = new CExtrasData[NewW * NewH];
	mem_zero(pNewData, NewW * NewH * sizeof(CTile));
	mem_zero(pNewTiles, NewW * NewH * sizeof(CExtrasData));
	
		   // copy old data
		for (int y = 0; y < min(NewH, m_Height); y++)
		 {
		mem_copy(&pNewTiles[y * NewW], &m_pTiles[y * m_Width], min(m_Width, NewW) * sizeof(CTile));
		mem_copy(&pNewData[y * NewW], &m_pExtrasData[y * m_Width], min(m_Width, NewW) * sizeof(CExtrasData));
		}
	
		   // replace old
		delete[] m_pTiles;
	m_pTiles = pNewTiles;
	delete[] m_pExtrasData;
	m_pExtrasData = pNewData;
	m_Width = NewW;
	m_Height = NewH;
	}

void CLayerExtras::Shift(int Direction)
 {
	switch (Direction)
		{
	case 1:
		{
			       // left
				for (int y = 0; y < m_Height; ++y)
				 {
				mem_move(&m_pTiles[y * m_Width], &m_pTiles[y * m_Width + 1], (m_Width - 1) * sizeof(CTile));
				mem_move(&m_pExtrasData[y * m_Width], &m_pExtrasData[y * m_Width + 1], (m_Width - 1) * sizeof(CExtrasData));
				}
			}
		break;
	case 2:
		{
			       // right
				for (int y = 0; y < m_Height; ++y)
				 {
				mem_move(&m_pTiles[y * m_Width + 1], &m_pTiles[y * m_Width], (m_Width - 1) * sizeof(CTile));
				mem_move(&m_pExtrasData[y * m_Width + 1], &m_pExtrasData[y * m_Width], (m_Width - 1) * sizeof(CExtrasData));
				}
			}
		break;
	case 4:
		{
			       // up
				for (int y = 0; y < m_Height - 1; ++y)
				 {
				mem_copy(&m_pTiles[y * m_Width], &m_pTiles[(y + 1) * m_Width], m_Width * sizeof(CTile));
				mem_copy(&m_pExtrasData[y * m_Width], &m_pExtrasData[(y + 1) * m_Width], m_Width * sizeof(CExtrasData));
				}
			}
		break;
	case 8:
		{
			       // down
				for (int y = m_Height - 1; y > 0; --y)
				 {
				mem_copy(&m_pTiles[y * m_Width], &m_pTiles[(y - 1) * m_Width], m_Width * sizeof(CTile));
				mem_copy(&m_pExtrasData[y * m_Width], &m_pExtrasData[(y - 1) * m_Width], m_Width * sizeof(CExtrasData));
				}
			}
		}
	}

void CLayerExtras::ShowInfo()
{
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	Graphics()->TextureSet(m_pEditor->Client()->GetDebugFont());
	Graphics()->QuadsBegin();
	
	int StartY = max(0, (int)(ScreenY0 / 32.0f) - 1);
	int StartX = max(0, (int)(ScreenX0 / 32.0f) - 1);
	int EndY = min((int)(ScreenY1 / 32.0f) + 1, m_Height);
	int EndX = min((int)(ScreenX1 / 32.0f) + 1, m_Width);
	
		for (int y = StartY; y < EndY; y++)
			for (int x = StartX; x < EndX; x++)
			{
				int c = x + y * m_Width;
				if (m_pTiles[c].m_Index)
				{
						char aBuf[64];
					str_format(aBuf, sizeof(aBuf), "%i", m_pTiles[c].m_Index);
					m_pEditor->Graphics()->QuadsText(x * 32, y * 32, 16.0f, aBuf);
			
						m_pEditor->Graphics()->QuadsText(x * 32, y * 32 + 16, 8.0f, m_pExtrasData[c].m_aData);
			
						for (int i = 0; i < EXTRATILE_DATA; i++)
							dbg_msg(0, "%i %c", i, m_pExtrasData[c].m_aData[i]);
				}
				x += m_pTiles[c].m_Skip;
			}
	
		Graphics()->QuadsEnd();
	Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
	}

int CLayerExtras::RenderProperties(CUIRect* pToolbox)
{
	int r = CLayerTiles::RenderProperties(pToolbox);
	m_Image = -1;
	return r;
}