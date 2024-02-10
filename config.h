#include <X11/XF86keysym.h>
/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappih    = 4;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 4;       /* vert inner gap between windows */
static const unsigned int gappoh    = 4;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 4;       /* vert outer gap between windows and screen edge */
static const int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static const int swallowfloating    = 1;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = {
    "CaskaydiaCoveNerdFont:size=14",
    "AppleColorEmoji:size=14",
    "NotoColorEmoji:size=14"
};
static const char dmenufont[]       = "CaskaydiaCoveNerdFont:size=14";
static unsigned int baralpha        = 0xd0;
static unsigned int borderalpha     = OPAQUE;
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#B5BFE2";
static const char *colors[][3]      = {
    /*               fg         bg         border   */
    [SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
    [SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
    [SchemeStatus]  = { "#E5C890", "#51576D",  "#000000"  }, // Statusbar right {text,background,not used but cannot be empty}
             [SchemeTagsSel]  = { "#B5BFE2", "#5B6078",  "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
             [SchemeTagsNorm]  = { "#E5C890", "#51576D",  "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
             [SchemeInfoSel]  = { "#B5BFE2", "#51576D",  "#000000"  }, // infobar middle  selected {text,background,not used but cannot be empty}
             [SchemeInfoNorm]  = { "#B5BFE2", "#51576D",  "#000000"  }, // infobar middle  unselected {text,background,not used but cannot be empty}
};

typedef struct {
    const char *name;
    const void *cmd;
} Sp;
const char *spcmd1[] = {"st", "-n", "spterm", "-g", "120x34", NULL };
const char *spcmd2[] = {"st", "-n", "spfm", "-g", "144x41", "-e", "ranger", NULL };
const char *spcmd3[] = {"gnome-calculator", NULL };
const char *spcmd4[] = {"st", "-n", "spdmenu", "-g", "120x34", "-e", "/home/reiter/scripts/dmenu/shellcmd.sh", NULL };
static Sp scratchpads[] = {
    /* name          cmd  */
    {"spterm",      spcmd1},
    {"spranger",    spcmd2},
    {"gnome-calculator",   spcmd3},
    {"spdmenu", spcmd4},
};

/* tagging */
static const char *tags[] = { "󰣇", "󰈹", "3", "4", "5", "6", "7", "󰓇", "󰙯" };

static const Rule rules[] = {
    /* xprop(1):
     *	WM_CLASS(STRING) = instance, class
     *	WM_NAME(STRING) = title
     */
    /* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
    { "Spotify", NULL,     NULL,           1 << 7,    0,          0,           0,        -1 },
    { "discord", NULL,     NULL,           1 << 8,    0,          0,           0,        -1 },
    { "kitty",   NULL,     NULL,           0,         0,          1,           0,        -1 },
    { "st-256color",NULL,  NULL,           0,         0,          1,           0,        -1 },
    { "spfm",    NULL,     NULL,           0,         0,          1,           0,        -1 },
    { "ranger",  NULL,     NULL,           0,         0,          1,           0,        -1 },
    { "Evince",  NULL,     NULL,           0,         0,          0,           1,        -1 },
    { NULL,	  "spterm",	   NULL,		SPTAG(0),     1,	      1,           0,		 -1 },
    { NULL,	  "spfm",	   NULL,		SPTAG(1),	  1,	      1,	       0,        -1 },
    { "gnome-calculator", NULL, NULL,	SPTAG(2),	  1,	      0,           0,        -1 },
    { NULL,     "spdmenu", NULL,	    SPTAG(3),	  1,	      1,           0,        -1 },
    { NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
    /* symbol     arrange function */
    { "[]=",      tile },    /* first entry is default */
    { "><>",      NULL },    /* no layout function means floating behavior */
    { "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG)                                                                                               \
    &((Keychord){1, {{MODKEY, KEY}},                                        view,           {.ui = 1 << TAG} }), \
    &((Keychord){1, {{MODKEY|ControlMask, KEY}},                            toggleview,     {.ui = 1 << TAG} }), \
    &((Keychord){1, {{MODKEY|ShiftMask, KEY}},                              tag,            {.ui = 1 << TAG} }), \
    &((Keychord){1, {{MODKEY|ControlMask|ShiftMask, KEY}},                  toggletag,      {.ui = 1 << TAG} }),

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, NULL}; //, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };
// audio control commands
static const char *mutecmd[] = { "/home/reiter/scripts/control/volumeControl.sh", "mute", NULL };
static const char *volupcmd[] = { "/home/reiter/scripts/control/volumeControl.sh", "up", NULL };
static const char *voldowncmd[] = { "/home/reiter/scripts/control/volumeControl.sh", "down", NULL };
static const char *playpausecmd[] = { "playerctl", "play-pause", NULL };
static const char *playnextcmd[] = { "playerctl", "next", NULL };
static const char *playprevcmd[] = { "playerctl", "previous", NULL };
//backlight control commands
static const char *brupcmd[] = { "/home/reiter/scripts/control/brightnessControl.sh", "up", NULL };
static const char *brdowncmd[] = { "/home/reiter/scripts/control/brightnessControl.sh", "down", NULL };
//screenshot commands
static const char *copyselss[] = {"/home/reiter/scripts/screenshots/copyselectscreenshot.sh", NULL};
static const char *saveselss[] = {"/home/reiter/scripts/screenshots/saveselectscreenshot.sh", NULL};
static const char *copyfulss[] = {"/home/reiter/scripts/screenshots/copyfullscreenshot.sh", NULL};
static const char *savefulss[] = {"/home/reiter/scripts/screenshots/savefullscreenshot.sh", NULL};
//shortcuts
static const char *explorercmd[] = {"st", "-e", "ranger", NULL };
static const char *browsercmdcollege[] = {"/home/reiter/scripts/launch/browser-college.sh", NULL};
static const char *browsercmdpersonal[] = {"/home/reiter/scripts/launch/browser-personal.sh", NULL};
//emoji
static const char *emotecmd[] = {"/home/reiter/scripts/dmenu/emote.sh", NULL};
//server
static const char *servcmd[] = {"/home/reiter/scripts/server/connectivity.sh", NULL};
static const char *servkillcmd[] = {"/home/reiter/scripts/server/kill.sh", NULL};
static const char *servopencmd[] = {"/home/reiter/scripts/server/openfrontend.sh", NULL};
//wifi
static const char *wificmd[] = {"/home/reiter/scripts/wifi/wificonnect.sh", NULL};

static Keychord *keychords[] = {
    /*           KC Length             Keychord                                     function              argument */
    &((Keychord){    2,  {{MODKEY|ShiftMask,XK_q},{MODKEY|ShiftMask,XK_q}},          quit,               {0}   }),                            
    &((Keychord){    1,  {{MODKEY,XK_p}},                                            spawn,              {.v   =    dmenucmd             }    }),
    &((Keychord){    1,  {{MODKEY,XK_x}},                                            spawn,              {.v   =    termcmd              }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_e}},                                  spawn,              {.v   =    explorercmd}         }),  
    &((Keychord){    2,  {{MODKEY,XK_b}, {MODKEY,XK_b}},                             spawn,              {.v   =    browsercmdpersonal}  }),  
    &((Keychord){    2,  {{MODKEY,XK_b}, {MODKEY,XK_w}},                             spawn,              {.v   =    browsercmdcollege}   }),  
    &((Keychord){    2,  {{MODKEY,XK_s},{MODKEY,XK_s}},                              spawn,              {.v   =    copyfulss}           }),  
    &((Keychord){    2,  {{MODKEY,XK_s},{MODKEY,XK_a}},                              spawn,              {.v   =    savefulss}           }),  
    &((Keychord){    2,  {{MODKEY|ShiftMask,XK_s},{MODKEY|ShiftMask,XK_s}},          spawn,              {.v   =    copyselss}           }),  
    &((Keychord){    2,  {{MODKEY|ShiftMask,XK_s},{MODKEY|ShiftMask,XK_a}},          spawn,              {.v   =    saveselss}           }),  
    &((Keychord){    1,  {{MODKEY,XK_semicolon}},                                    spawn,              {.v   =    emotecmd             }    }),
    &((Keychord){    1,  {{MODKEY,XK_KP_Begin}},                                     spawn,              {.v   =    servcmd              }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_KP_Begin}},                           spawn,              {.v   =    servkillcmd          }    }),
    &((Keychord){    1,  {{MODKEY|ControlMask,XK_KP_Begin}},                         spawn,              {.v   =    servopencmd          }    }),
    &((Keychord){    1,  {{0,XK_KP_Delete}},                                         spawn,              {.v   =    wificmd              }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_b}},                                  togglebar,          {0}   }),                            
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_h}},                                  incnmaster,         {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_l}},                                  incnmaster,         {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY,XK_h}},                                            setmfact,           {.f   =    -0.05}               }),  
    &((Keychord){    1,  {{MODKEY,XK_l}},                                            setmfact,           {.f   =    +0.05}               }),  
    &((Keychord){    1,  {{MODKEY,XK_k}},                                            setcfact,           {.f   =    +0.25}               }),  
    &((Keychord){    1,  {{MODKEY,XK_j}},                                            setcfact,           {.f   =    -0.25}               }),  
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_o}},                                  setcfact,           {.f   =    0.00}                }),  
    &((Keychord){    1,  {{MODKEY,XK_Return}},                                       zoom,               {0}   }),                            
    &((Keychord){    1,  {{ALTKEY,XK_Tab}},                                          focusstack,         {.i   =    +1                   }    }),
    &((Keychord){    1,  {{ALTKEY|ShiftMask,XK_Tab}},                                focusstack,         {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_x}},                                  killclient,         {0}   }),                            
    &((Keychord){    1,  {{MODKEY,XK_t}},                                            setlayout,          {.v   =    &layouts[0]}         }),  
    &((Keychord){    1,  {{MODKEY,XK_f}},                                            setlayout,          {.v   =    &layouts[1]}         }),  
    &((Keychord){    1,  {{MODKEY,XK_m}},                                            setlayout,          {.v   =    &layouts[2]}         }),  
    //&((Keychord){  1,  {{MODKEY,XK_space}},                                        setlayout,          {0}   }),                            
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_space}},                              togglesticky,       {0}   }),                            
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_f}},                                  togglefloating,     {0}   }),                            
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_m}},                                  togglealwaysontop,  {0}   }),                            
    &((Keychord){    1,  {{MODKEY,XK_0}},                                            view,               {.ui  =    ~0                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_0}},                                  tag,                {.ui  =    ~0                   }    }),
    &((Keychord){    1,  {{MODKEY,XK_comma}},                                        focusmon,           {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY,XK_period}},                                       focusmon,           {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_comma}},                              tagmon,             {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_period}},                             tagmon,             {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask|ControlMask,XK_comma}},                  focusmon,           {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask|ControlMask,XK_period}},                 focusmon,           {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask|ControlMask,XK_comma}},                  tagmon,             {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask|ControlMask,XK_period}},                 tagmon,             {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY,XK_c}},                                            togglescratch,      {.ui  =    0                    }    }),
    &((Keychord){    1,  {{MODKEY,XK_e}},                                            togglescratch,      {.ui  =    1                    }    }),
    &((Keychord){    1,  {{0,XK_KP_Insert}},                                         togglescratch,      {.ui  =    2                    }    }),
    &((Keychord){    1,  {{MODKEY,XK_r}},                                            togglescratch,      {.ui  =    3                    }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask,XK_h}},                                   incrgaps,           {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask,XK_l}},                                   incrgaps,           {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask|ShiftMask,XK_h}},                         incrogaps,          {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask|ShiftMask,XK_l}},                         incrogaps,          {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask|ControlMask,XK_h}},                       incrigaps,          {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask|ControlMask,XK_l}},                       incrigaps,          {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask,XK_0}},                                   togglegaps,         {0}   }),                            
    &((Keychord){    1,  {{MODKEY|Mod1Mask|ShiftMask,XK_0}},                         defaultgaps,        {0}   }),                            
    &((Keychord){    1,  {{MODKEY,XK_y}},                                            incrihgaps,         {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY,XK_o}},                                            incrihgaps,         {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|ControlMask,XK_y}},                                incrivgaps,         {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|ControlMask,XK_o}},                                incrivgaps,         {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask,XK_y}},                                   incrohgaps,         {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|Mod1Mask,XK_o}},                                   incrohgaps,         {.i   =    -1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_y}},                                  incrovgaps,         {.i   =    +1                   }    }),
    &((Keychord){    1,  {{MODKEY|ShiftMask,XK_o}},                                  incrovgaps,         {.i   =    -1                   }    }),
    &((Keychord){    1,  {{0,XF86XK_AudioMute}},                                     spawn,              {.v   =    mutecmd              }    }),
    &((Keychord){    1,  {{0,XF86XK_AudioLowerVolume}},                              spawn,              {.v   =    voldowncmd           }    }),
    &((Keychord){    1,  {{0,XF86XK_AudioRaiseVolume}},                              spawn,              {.v   =    volupcmd             }    }),
    &((Keychord){    1,  {{0,XF86XK_AudioPlay}},                                     spawn,              {.v   =    playpausecmd         }    }),
    &((Keychord){    1,  {{0,XF86XK_AudioNext}},                                     spawn,              {.v   =    playnextcmd          }    }),
    &((Keychord){    1,  {{0,XF86XK_AudioPrev}},                                     spawn,              {.v   =    playprevcmd          }    }),
    &((Keychord){    1,  {{0,XF86XK_MonBrightnessUp}},                               spawn,              {.v   =    brupcmd}             }),  
    &((Keychord){    1,  {{0,XF86XK_MonBrightnessDown}},                             spawn,              {.v   =    brdowncmd}           }),  

    TAGKEYS(                        XK_1,                      0)
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    TAGKEYS(                        XK_7,                      6)
    TAGKEYS(                        XK_8,                      7)
    TAGKEYS(                        XK_9,                      8)
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
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
