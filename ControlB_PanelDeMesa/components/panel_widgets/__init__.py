"""Panel Widgets — ESPHome External Component
Custom widget overlay for Panel de Mesa Control B.
Provides a slide-out panel with scene buttons, light controls,
media controls, and status indicators overlaid on the RemoteWebView output.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import display, touchscreen

CODEOWNERS = ["@ydiaz1699"]
DEPENDENCIES = ["display", "touchscreen"]

panel_widgets_ns = cg.esphome_ns.namespace("panel_widgets")
PanelWidgets = panel_widgets_ns.class_("PanelWidgets", cg.Component)

CONF_DISPLAY = "display"
CONF_TOUCHSCREEN = "touchscreen"
CONF_BAR_WIDTH = "bar_width"
CONF_BAR_POSITION = "bar_position"
CONF_SCENES = "scenes"
CONF_SCENE_NAME = "name"
CONF_SCENE_ICON = "icon"
CONF_SCENE_SERVICE = "service"
CONF_SCENE_ENTITY = "entity"

SCENE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SCENE_NAME): cv.string,
        cv.Required(CONF_SCENE_ICON): cv.string,
        cv.Required(CONF_SCENE_SERVICE): cv.string,
        cv.Required(CONF_SCENE_ENTITY): cv.string,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PanelWidgets),
        cv.Required(CONF_DISPLAY): cv.use_id(display.Display),
        cv.Required(CONF_TOUCHSCREEN): cv.use_id(touchscreen.Touchscreen),
        cv.Optional(CONF_BAR_WIDTH, default=80): cv.int_range(min=40, max=200),
        cv.Optional(CONF_BAR_POSITION, default="left"): cv.one_of("left", "right", lower=True),
        cv.Optional(CONF_SCENES, default=[]): cv.ensure_list(SCENE_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    disp = await cg.get_variable(config[CONF_DISPLAY])
    cg.add(var.set_display(disp))

    ts = await cg.get_variable(config[CONF_TOUCHSCREEN])
    cg.add(var.set_touchscreen(ts))

    cg.add(var.set_bar_width(config[CONF_BAR_WIDTH]))
    cg.add(var.set_bar_position(config[CONF_BAR_POSITION] == "right"))

    for scene_cfg in config.get(CONF_SCENES, []):
        cg.add(
            var.add_scene(
                scene_cfg[CONF_SCENE_NAME],
                scene_cfg[CONF_SCENE_ICON],
                scene_cfg[CONF_SCENE_SERVICE],
                scene_cfg[CONF_SCENE_ENTITY],
            )
        )
