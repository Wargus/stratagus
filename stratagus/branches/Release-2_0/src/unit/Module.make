MODULE= src/unit
MSRC=	script_unit.c script_unittype.c depend.c unit_ai.c unit_save.c unittype.c upgrade.c \
	unit.c unit_draw.c unit_find.c unit_cache.c

SRC+=	$(addprefix $(MODULE)/,$(MSRC))
HDRS+=
