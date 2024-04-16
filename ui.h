/* date = October 24th 2023 11:48 pm */

#ifndef UI_H
#define UI_H

#define FONT_LABEL_SIZE 32
#define FONT_GLYPH_COUNT 256
#define UI_BUFFER_VERTICES_COUNT 4096
#define MAX_STACK_SIZE 32

#define ui_DeferLoop(begin, end) for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define ui_Context(v) ui_DeferLoop(ui_PushContext(v), ui_PopContext())
#define ui_Parent() ui_DeferLoop(ui_PushParentStack(), ui_PopParentStack())
#define ui_Frame(v) ui_DeferLoop(ui_StartFrame(v), ui_PopParentStack())
#define ui_TextColor(v) ui_DeferLoop(ui_PushTextColor(v), ui_PopTextColor())
#define ui_BackgroundColor(v) ui_DeferLoop(ui_PushBackgroundColor(v), ui_PopBackgroundColor())
#define ui_Style(v) ui_DeferLoop(ui_PushStyleStack(v), ui_PopStyleStack())
#define CheckFlag(s, f) (((s) & (f)) == (f))

/*
List of widgets?
- window
- text
- button
- input
- slider

Features:
- Auto layout
*/

enum ui_sizekind
{
	UI_SIZEKIND_NULL,
	UI_SIZEKIND_PIXELS,
	UI_SIZEKIND_TEXTCONTENT,
	UI_SIZEKIND_PERCENTOFPARENT,
	UI_SIZEKIND_CHILDRENSUM,
	UI_SIZEKIND_EQUALSUM,
	
	UI_SIZEKIND_COUNT,
};

enum ui_axis2
{
	UI_AXIS2_X,
	UI_AXIS2_Y,
	UI_AXIS2_COUNT,
};

struct ui_key
{
};

struct ui_size
{
	ui_sizekind kind;
	r32 value;
	r32 strictness;
};

typedef u32 ui_box_flags;
enum
{
	UI_BOXFLAG_CLICKABLE       = (1<<0),
	UI_BOXFLAG_VIEWSCROLL      = (1<<1),
	UI_BOXFLAG_DRAWTEXT        = (1<<2),
	UI_BOXFLAG_DRAWBORDER      = (1<<3),
	UI_BOXFLAG_DRAWBACKGROUND  = (1<<4),
	UI_BOXFLAG_DRAWDROPSHADOW  = (1<<5),
	UI_BOXFLAG_CLIP            = (1<<6),
	UI_BOXFLAG_HOTANIMATION    = (1<<7),
	UI_BOXFLAG_ACTIVEANIMATION = (1<<8),
	
	UI_BOXFLAG_COUNT,
};

typedef u32 ui_layout_flags;
enum
{
	UI_LAYOUTFLAG_EXPAND_H       = (1<<0),
	UI_LAYOUTFLAG_EXPAND_V       = (1<<1),
	
	UI_LAYOUTFLAG_COUNT,
};

struct ui_box
{
	// tree links
	ui_box *first;
	ui_box *last;
	ui_box *next;
	ui_box *prev;
	ui_box *parent;
	u32 childCount;
	
	// hash links
	ui_box *hash_next;
	ui_box *hash_prev;
	
	// Key+generation info
	ui_key key;
	u64 lastFrameTouchedIndex;
	
	// Computed every frame
	r32 computedRelPositions[UI_AXIS2_COUNT]; // position relative to the parent
	r32 computedSize[UI_AXIS2_COUNT]; // size in pixels
	v4  rect; // on screen coordinates 
	
	// Per-frame info provided by builders
	string label;
	ui_box_flags flags;
	ui_layout_flags layoutFlags;
	ui_size semanticSize[UI_AXIS2_COUNT];
	
	// Persistent data
	r32 hot_t;
	r32 active_t;
};

struct ui_style
{
	v4 color;
	ui_size size[UI_AXIS2_COUNT];
	ui_layout_flags layoutFlags;
};

struct ui_comm
{
	ui_box *box;
	v2     mouse;
	v2     drag_delta;
	b8     clicked;
	b8     double_clicked;
	b8     right_clicked;
	b8     pressed;
	b8     released;
	b8     dragging;
	b8     hovering;
};

struct ui_font_glyph
{
	char symbol;
	r32  positions[UI_AXIS2_COUNT];
	r32  sizes[UI_AXIS2_COUNT];
};

struct ui_context
{
	b32    show;
	
	// Storage
	ui_box *nodes;
	u32    nodeCount;
	
	// computed every frame
	vertex_ui *frameVertexBuffer;
	u32 currentDepth;
	u32 maxDepth;
	
	// Stack storage
	ui_box    *parentStackStorage_[MAX_STACK_SIZE];
	ui_style  *styleStackStorage_[MAX_STACK_SIZE];
	ui_box    **parentStack;
	ui_style  **styleStack;
	
	// NOTE(Ecy): font, only do monospace for simplicity
	struct
	{
		vertex_ui *vertices;
		u32 vertexCount;
		
		char fontName[FONT_LABEL_SIZE];
		r32  fontSize;
		r32  renderSize;
		r32  padding;
		
		font_glyph glyphs[FONT_GLYPH_COUNT];
		u32 startGlyphCharIndex;
	} font;
	
	struct
	{
		v2 pos;
		b8 leftClicked;
		b8 rightClicked;
		b8 middleClicked;
		b8 forwardClicked;
		b8 backwardClicked;
		
	} mouseEvent;
};

#endif //UI_H
