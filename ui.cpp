#include "ui.h"

global ui_context uiContext = {};
// TODO(Ecy): to remove later
global vertex_ui uiMem[1024];
global vertex_ui fontMem[4096];

inline ui_comm 
ui_CommFromBox(ui_box *box)
{
	ui_comm comm = {};
	comm.box = box;
	
	// TODO(Ecy): remove this
	comm.mouse.x = uiContext.mouseEvent.pos.x;
	comm.mouse.y = uiContext.mouseEvent.pos.y;
	
	v2 p0 = V2(box->rect[0], box->rect[1]);
	v2 p1 = V2(box->rect[2], box->rect[3]);
	
	if(comm.mouse.x >= p0.x && comm.mouse.y >= p0.y &&
	   comm.mouse.x <= p1.x && comm.mouse.y <= p1.y)
	{
		comm.hovering = true;
		comm.clicked = uiContext.mouseEvent.leftClicked;
		// pressed
		// released
	}
	
	return comm;
}

// basic key type helpers
ui_key ui_KeyNull(void);
ui_key ui_KeyFromString(string str);
b32 ui_KeyMatch(ui_key a, ui_key b);

// construct a box, looking up from the cache if
// possible, and pushing it as a new child of the
// active parent.
internal ui_box*
ui_BoxMake(ui_box_flags flags, string label)
{
	ui_style *style = *uiContext.styleStack;
	
	ui_box *box = &uiContext.nodes[uiContext.nodeCount++];
	box->parent = *uiContext.parentStack;
	ui_box *parent = box->parent;
	if(parent)
	{
		if(parent->last)
		{
			box->prev = parent->last;
			parent->last->next = box;
			parent->last = box;
		}
		else
		{
			parent->first = box;
			parent->last = box;
		}
		
		parent->childCount++;
	}
	
	box->flags = flags;
	box->label = label;
	if(style)
	{
		box->semanticSize[UI_AXIS2_X] = style->size[UI_AXIS2_X];
		box->semanticSize[UI_AXIS2_Y] = style->size[UI_AXIS2_Y];
		box->layoutFlags = style->layoutFlags;
	}
	
	return box;
}

//ui_box* ui_BoxMakeF(ui_box_flags flags, char *fmt, ...);

// some other possible building parameterizations
void ui_BoxEquipDisplayString(ui_box *box, string str);
void ui_BoxEquipChildLayoutAxis(ui_box *box, ui_axis2 axis);

inline i32 
ui_PushParentStack()
{
	uiContext.parentStack++;
	
	*uiContext.parentStack = &uiContext.nodes[uiContext.nodeCount - 1];
	
	if(uiContext.currentDepth + 1 > uiContext.maxDepth)
	{
		uiContext.maxDepth++;
	}
	
	return 0;
}

void ui_PopParentStack()
{
	uiContext.parentStack--;
}

i32 ui_PushStyleStack(ui_style *style)
{
	uiContext.styleStack++;
	
	*uiContext.styleStack = style;
	return 0;
}

void ui_PopStyleStack()
{
	uiContext.styleStack--;
}

inline b32

ui_boxIsNil(ui_box* box)
{
	return box == 0;
}

inline ui_box*
ui_GetNextNode(ui_box* node)
{
	if(node->first)
	{
		uiContext.currentDepth++;
		return node->first;
	}
	
	if(node->next)
	{
		return node->next;
	}
	
	uiContext.currentDepth--;
	return node->parent->next;
}

internal u32
ui_EstimateTextWidth(string str)
{
	u32 length = 0;
	
	// TODO(Ecy): adapt to context font ratio 
	for(u32 index = 0;
		index < str.size;
		++index)
	{
		char currentGlyph = str.contents[index];
		font_glyph *glyph = &uiContext.font.glyphs[(u32)currentGlyph - uiContext.font.startGlyphCharIndex];
		length += glyph->xadvance * uiContext.font.renderSize / uiContext.font.fontSize;
	}
	
	return length;
}

internal ui_comm
ui_Text(string str)
{
	ui_box *box = ui_BoxMake(UI_BOXFLAG_DRAWTEXT, str);
	box->semanticSize[UI_AXIS2_X] = { UI_SIZEKIND_TEXTCONTENT, 0.0f, 0.0f };
	box->semanticSize[UI_AXIS2_Y] = { UI_SIZEKIND_TEXTCONTENT, 0.0f, 0.0f };
	
	ui_comm comm = ui_CommFromBox(box);
	return comm;
}

inline ui_comm 
ui_StartFrame(string str)
{
	ui_box *box = ui_BoxMake(UI_BOXFLAG_CLICKABLE|
							 UI_BOXFLAG_DRAWBORDER|
							 UI_BOXFLAG_DRAWBACKGROUND,
							 str);
	box->layoutFlags = UI_LAYOUTFLAG_EXPAND_V;
	
	ui_PushParentStack();
	ui_Text(str);
	
	ui_comm comm = ui_CommFromBox(box);
	return comm;
}

inline ui_comm 
ui_button(string str)
{
	ui_box *box = ui_BoxMake(UI_BOXFLAG_CLICKABLE|
							 UI_BOXFLAG_DRAWBORDER|
							 UI_BOXFLAG_DRAWTEXT|
							 UI_BOXFLAG_DRAWBACKGROUND|
							 UI_BOXFLAG_HOTANIMATION|
							 UI_BOXFLAG_ACTIVEANIMATION,
							 str);
	
	ui_comm comm = ui_CommFromBox(box);
	return comm;
}

internal void
UIInit(platform_engine *engine)
{
	memory_context *memory = &engine->memory;
	input_context  *input = &engine->input;
	timer_context  *clock = &engine->clock;
	//asset_context  *render = &platform->render;
	
	// TODO(Ecy): initiate ui layer here
	uiContext.nodes = (ui_box*)uiMem;
	uiContext.parentStack = (ui_box**)uiContext.parentStackStorage_;
	uiContext.styleStack = (ui_style**)uiContext.styleStackStorage_;
	uiContext.font.vertices = fontMem;

#if 0	
	// load font 
	{
		debug_read_file_result fontData = Win32WriteEntireFile(NULL, "bakedMap.data");
		Assert(fontData.contents);
		
		u8 *cursor = (u8*)fontData.contents;
		font_asset *header = (font_asset*)cursor;
		uiContext.font.fontSize = header->baseSize;
		uiContext.font.renderSize = 24.0f;
		uiContext.font.padding = 1.35f;
		uiContext.font.startGlyphCharIndex = 0;
		cursor += sizeof(font_asset);
		memcpy(uiContext.font.glyphs, cursor, sizeof(font_glyph) * 256);
	}
#endif

}

internal void
ui_Update(ui_box* node)
{
	for(ui_box* box = node->first;
		box != NULL;
		box = ui_GetNextNode(box))
	{
		ui_size *sizeX = &box->semanticSize[UI_AXIS2_X];
		ui_size *sizeY = &box->semanticSize[UI_AXIS2_Y];
		
		switch(sizeX->kind)
		{
			// TODO(Ecy): compute font size information
			case UI_SIZEKIND_TEXTCONTENT: 
			{
				u32 length = ui_EstimateTextWidth(box->label);
				box->computedSize[UI_AXIS2_X] = length;
			}break;
			case UI_SIZEKIND_PIXELS:
			{
				box->computedSize[UI_AXIS2_X] = sizeX->value;
			}break;
			
			default:
			{
			}break;
		}
		
		switch(sizeY->kind)
		{
			// TODO(Ecy): compute font size information
			case UI_SIZEKIND_TEXTCONTENT:
			{
				box->computedSize[UI_AXIS2_Y] = uiContext.font.renderSize;
			}break;
			case UI_SIZEKIND_PIXELS:
			{
				box->computedSize[UI_AXIS2_Y] = sizeY->value;
			}break;
			
			default:
			{
			}break;
		}
	}
	
	// Calculate upwards-dependent sizes: percentParents
	for(ui_box* box = node->first; 
		box != NULL;
		box = ui_GetNextNode(box))
	{
		ui_size *sizeX = &box->semanticSize[UI_AXIS2_X];
		ui_size *sizeY = &box->semanticSize[UI_AXIS2_Y];
		
		switch(sizeX->kind)
		{
			// TODO(Ecy): compute font size information
			case UI_SIZEKIND_PERCENTOFPARENT:
			{
				box->computedSize[UI_AXIS2_X] = sizeX->value * box->parent->computedSize[UI_AXIS2_X];
			}break;
			
			case UI_SIZEKIND_EQUALSUM:
			{
				box->computedSize[UI_AXIS2_X] = box->parent->computedSize[UI_AXIS2_X] / (r32)box->parent->childCount;
			}break;
			
			default:
			{
			}break;
		}
		
		switch(sizeY->kind)
		{
			// TODO(Ecy): compute font size information
			case UI_SIZEKIND_PERCENTOFPARENT:
			{
				box->computedSize[UI_AXIS2_Y] = sizeY->value * box->parent->computedSize[UI_AXIS2_Y];
			}break;
			
			case UI_SIZEKIND_EQUALSUM:
			{
				box->computedSize[UI_AXIS2_Y] = box->parent->computedSize[UI_AXIS2_Y] / (r32)box->parent->childCount;
			}break;
			
			default:
			{
			}break;
		}
	}
	
	// compute relative position of each box. Can also compute the final screen coordinates
	// NOTE(Ecy): exclude root node
	for(ui_box* box = node->first;
		box != NULL;
		box = ui_GetNextNode(box))
	{
		ui_size *sizeX = &box->semanticSize[UI_AXIS2_X];
		ui_size *sizeY = &box->semanticSize[UI_AXIS2_Y];
		
		r32 offset[UI_AXIS2_COUNT] = 
		{ 
			box->parent->computedRelPositions[UI_AXIS2_X],
			box->parent->computedRelPositions[UI_AXIS2_Y]
		};
		
		if(box->prev)
		{
			if(CheckFlag(box->layoutFlags, UI_LAYOUTFLAG_EXPAND_V))
			{
				offset[UI_AXIS2_Y] = box->prev->computedRelPositions[UI_AXIS2_Y] + box->prev->computedSize[UI_AXIS2_Y];
			}
			else
			{
				offset[UI_AXIS2_X] = box->prev->computedRelPositions[UI_AXIS2_X] + box->prev->computedSize[UI_AXIS2_X];
			}
		}
		
		if(CheckFlag(box->flags, UI_BOXFLAG_DRAWTEXT))
		{
			r32 scale = uiContext.font.renderSize / uiContext.font.fontSize;
			u32 length = uiContext.font.renderSize * uiContext.font.padding / 2.0f;
			for(u32 index = 0;
				index < box->label.size;
				++index)
			{
				char currentGlyph = box->label.contents[index];
				font_glyph *glyph = &uiContext.font.glyphs[(u32)currentGlyph - uiContext.font.startGlyphCharIndex];
				
				vertex_ui *v = &uiContext.font.vertices[uiContext.font.vertexCount++];
				v->p0 = 
				{ 
					offset[UI_AXIS2_X] + length + glyph->xoff * scale, 
					offset[UI_AXIS2_Y] + glyph->yoff * scale + uiContext.font.renderSize
				};
				v->p1 = 
				{ 
					(r32)(offset[UI_AXIS2_X] + length) + glyph->xoff2 * scale,
					(r32)(offset[UI_AXIS2_Y]) + glyph->yoff2 * scale + uiContext.font.renderSize
				};
				v->t0 = { (r32)glyph->x0, (r32)glyph->y0 };
				v->t1 = { (r32)glyph->x1, (r32)glyph->y1 };
				v->cornerRadius = 0.0f;
				v->edgeSoftness = 0.0f;
				v->borderThickness = 0.0f;
				v->color[0] = V4(1.0f, 1.0f, 1.0f, 1.0f);
				v->color[1] = V4(1.0f, 1.0f, 1.0f, 1.0f);
				v->color[2] = V4(1.0f, 1.0f, 1.0f, 1.0f);
				v->color[3] = V4(1.0f, 1.0f, 1.0f, 1.0f);
				
				length += glyph->xadvance * scale;
			}
			
			box->computedSize[UI_AXIS2_X] += uiContext.font.renderSize * uiContext.font.padding;
			box->computedSize[UI_AXIS2_Y] *= uiContext.font.padding;
		}
		
		box->rect = V4(offset[UI_AXIS2_X], offset[UI_AXIS2_Y],
					   offset[UI_AXIS2_X] + box->computedSize[UI_AXIS2_X], 
					   offset[UI_AXIS2_Y] + box->computedSize[UI_AXIS2_Y]);
		
		box->computedRelPositions[UI_AXIS2_X] += offset[UI_AXIS2_X];
		box->computedRelPositions[UI_AXIS2_Y] += offset[UI_AXIS2_Y];
	}
}

internal void
UIBegin(platform_engine *platform)
{
	game_memory *memory = &platform->memory;
	game_input  *input = &platform->input;
	game_clock  *clock = &platform->clock;
	game_render *render = &platform->render;
	
	// NOTE(Ecy): reset ui box tree
	memset(uiContext.nodes, 0, sizeof(ui_box) * uiContext.nodeCount);
	uiContext.nodeCount = 0;
	uiContext.maxDepth = 1;
	uiContext.currentDepth = 1;
	uiContext.font.vertexCount = 0;
	
	// NOTE(Ecy): map input data to context (per frame data)
	uiContext.mouseEvent.pos.x       = input->mouseX;
	uiContext.mouseEvent.pos.y       = input->mouseY;
	uiContext.mouseEvent.leftClicked = input->mouseButtons.left.endedDown;
	
	ui_box *root = ui_BoxMake(0, {});
	root->computedSize[UI_AXIS2_X] = platform->width;
	root->computedSize[UI_AXIS2_Y] = platform->height;
	root->rect = { 0, 0, (r32)platform->width, (r32)platform->height };
}

internal void
UIUpdateAndRender(game_platform *platform)
{
	game_memory *memory = &platform->memory;
	game_input  *input = &platform->input;
	game_clock  *clock = &platform->clock;
	game_render *render = &platform->render;
	
	ui_Update(uiContext.nodes);
	
	vertex_ui dest[1024] = {};
	vertex_ui *cursor = dest;
	u32 renderGroupIndices[32] = {};
	
	// TODO(Ecy): potentially no need to render per layer ...
	for(u32 index = 1;
		index <= uiContext.maxDepth;
		++index)
	{
		uiContext.currentDepth = 1;
		r32 ratio = (r32)(index + 1) / (r32)uiContext.maxDepth;
		
		u32 nodeCount = 0;
		for(ui_box* box = uiContext.nodes->first;
			box != NULL;
			box = ui_GetNextNode(box))
		{
			if(uiContext.currentDepth == index)
			{
				vertex_ui *v = cursor++;
				
				v->p0 = { box->rect[0], box->rect[1] };
				v->p1 = { box->rect[2], box->rect[3] };
				v->t0 = { 0.0f, 0.0f };
				v->t1 = { 512, 512 };
				v->cornerRadius = 20.0f;
				v->edgeSoftness = 2.0f;
				v->borderThickness = 0.0f;
				v->color[0] = V4(0.3f, 0.3f, 0.3f, 0.4f);
				v->color[1] = V4(0.3f, 0.3f, 0.3f, 0.4f);
				v->color[2] = V4(0.3f, 0.3f, 0.3f, 0.4f);
				v->color[3] = V4(0.3f, 0.3f, 0.3f, 0.4f);
				
				nodeCount++;
			}
		}
		
		renderGroupIndices[index] = nodeCount;
	}
	memcpy(cursor, uiContext.font.vertices, sizeof(vertex_ui) * uiContext.font.vertexCount);
	cursor += uiContext.font.vertexCount;
	
	render->updateUI(render->context, dest, uiContext.nodeCount + uiContext.font.vertexCount);
	
	render_draw_command_ui command = {};
	command.textureAssetId = whiteTex->id;
	
	u32 sum = 0;
	for(u32 index = 1;
		index <= uiContext.maxDepth;
		++index)
	{
		command.drawStartIndex = sum;
		command.drawCount = renderGroupIndices[index];
		
		render->queueDrawUICommand(&render->context->uiGroup, &command);
		
		sum += renderGroupIndices[index];
	}
	
	if(uiContext.font.vertexCount > 0)
	{
		// TODO(Ecy): temporary, remove this later
		command.textureAssetId = 5;
		command.drawStartIndex = sum;
		command.drawCount = uiContext.font.vertexCount;
		command.isText = true;
		
		render->queueDrawUICommand(&render->context->uiGroup, &command);
	}
}
