MODULE= src/unit
MSRC=	ccl_unit.c ccl_unittype.c depend.c unit_ai.c unit_save.c unittype.c upgrade.c \
	unit.c unit_draw.c unit_find.c unitcache.c

SRC+=	$(addprefix $(MODULE)/,$(MSRC))
HDRS+=
