/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
#include "movestack.c"
#include "tcl.c"

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int gappx     = 5;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;    /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "ArimoNerdFont:size=11" };
// static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char col_black[]        = "#000000";
static const char cn_bg[]           = "#1e1e2e";
static const char cn_sapphire[]     = "#74c7ec";
static const char *colors[][3]      = {
	/*                  fg              bg              border   */
	[SchemeNorm] =      { col_gray3,    cn_bg,          col_gray2 },
	[SchemeSel]  =      { col_gray4,    col_cyan,       cn_sapphire  },
	[SchemeStatus]  =   { col_gray3,    cn_bg,          "#000000"  }, // Statusbar right {text,background,not used but cannot be empty}
	[SchemeTagsSel]  =  { col_black,    cn_sapphire,    "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
	[SchemeTagsNorm]  = { col_gray3,    cn_bg,          "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
	[SchemeInfoSel]  =  { col_black,    cn_sapphire,    "#000000"  }, // infobar middle  selected {text,background,not used but cannot be empty}
	[SchemeInfoNorm]  = { col_gray3,    cn_bg,          "#000000"  }, // infobar middle  unselected {text,background,not used but cannot be empty}
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd[] = {"alacritty", "--class", "spterm", "-o", "window.dimensions.columns=150", "-o", "window.dimensions.lines=40", NULL };
// const char *spcmd2[] = {"st", "-n", "spfm", "-g", "144x41", "-e", "ranger", NULL };
// const char *spcmd3[] = {"keepassxc", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"spterm",      spcmd},
	// {"spranger",    spcmd2},
	// {"keepassxc",   spcmd3},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   isterminal    noswallow     monitor */
	{ "Gimp",      NULL,        NULL,       0,              1,           0,           0,            -1 },
	{ "Firefox",   NULL,        NULL,       1 << 8,         0,           0,          -1,            -1 },
 	{ NULL,		   "spterm",	NULL,		SPTAG(0),		1,			 0,           0,            -1 },
 	{ NULL,		   "spfm",		NULL,		SPTAG(1),		1,			 0,           0,            -1 },
 	{ NULL,		   "keepassxc",	NULL,		SPTAG(2),		0,			 0,           0,            -1 },
 	{ NULL,        NULL,        "Event Tester", 0,          0,           0,           1,            -1 }, /* xev */
 	{ "Alacritty", NULL,        NULL,           0,          0,           1,           0,            -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "  ",      tile },    /* first entry is default */
	{ "  ",      NULL },    /* no layout function means floating behavior */
	{ "|M|",      monocle },
	{ "|||",      tcl },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *dmenucmd[] = { "rofi", "-show", "run", NULL };
static const char *clipcmd[] = { "rofi", "-modi", "\"clipboard:greenclip print\"", "-show", "clipboard", "-run-command", "'{cmd}'", NULL };
static const char *passcmd[] = { "rofi-pass", NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *scrlocker[] = { "betterlockscreen", "-l", NULL };
 
static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_r,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawnsshaware,  {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
 	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
 	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	// { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	// { MODKEY|ShiftMask,             XK_v,      togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
 	{ MODKEY,            			XK_u,  	   togglescratch,  {.ui = 0 } },
    { MODKEY,                       XK_v,      spawn,          SHCMD("gopass pwgen -1 26 | head -1 | xlip -selection clipboard") },
	// { MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	// { MODKEY,                       XK_equal,  setgaps,        {.i = +1 } },
	// { MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = 0  } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_e,      quit,           {0} },
    // custom commands
    { ALTKEY,                       XK_h,      spawn,          {.v = clipcmd } },
    { ALTKEY,                       XK_p,      spawn,          {.v = passcmd } },
    { ALTKEY,                       XK_l,      spawn,          {.v = scrlocker } },
    // mediakeys
    { 0, XF86XK_AudioRaiseVolume,              spawn,          SHCMD("pactl set-sink-volume @DEFAULT_SINK@ +10%; kill -37 $(pidof dwmblocks)") },
    { 0, XF86XK_AudioLowerVolume,              spawn,          SHCMD("pactl set-sink-volume @DEFAULT_SINK@ -10%; kill -37 $(pidof dwmblocks)") },
    { 0, XF86XK_MonBrightnessUp,               spawn,          SHCMD("light -A 10; kill -36 $(pidof dwmblocks)") },
    { 0, XF86XK_MonBrightnessDown,             spawn,          SHCMD("light -U 10; kill -36 $(pidof dwmblocks)") },
    // { 0, XF86XK_MonBrightnessUp,               spawn,          {.v = (const char*[]){ "light", "-A", "10" } } },
    // { 0, XF86XK_MonBrightnessDown,             spawn,          {.v = (const char*[]){ "light", "-U", "10" } } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
