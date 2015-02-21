#=========================================================================
# GLV main makefile
#=========================================================================

include Makefile.config

SRCS = 	glv_buttons.cpp \
	glv_color.cpp \
	glv_core.cpp \
	glv_draw.cpp \
	glv_font.cpp \
	glv_glv.cpp \
	glv_grid.cpp \
	glv_inputdevice.cpp \
	glv_layout.cpp \
	glv_model.cpp \
 	glv_plots.cpp \
	glv_sliders.cpp \
	glv_texture.cpp \
	glv_textview.cpp \
	glv_view.cpp \
	glv_view3D.cpp \
	glv_widget.cpp

ifdef WINDOW_BINDING
	SRCS += glv_binding.cpp $(BINDING_SRC)
endif

OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= $(EXT_CPPFLAGS)
LDFLAGS		+= $(EXT_LDFLAGS)

CFLAGS		+= $(addprefix -I, $(INC_DIRS))
LDFLAGS		:= $(addprefix -L, $(LIB_DIRS)) $(LDFLAGS)

CFLAGS		:= $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS)

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------
.PHONY: clean test

# Build object file from C++ source
$(OBJ_DIR)%.o: %.cpp
	@echo CXX $< $@
	@$(CXX) -c $(CFLAGS) $< -o $@

# Build static library
$(SLIB_PATH): createFolders $(addprefix $(OBJ_DIR), $(OBJS))
	@echo AR $@
	@rm -f $@
	@$(AR) $@ $(filter %.o, $^)
	@$(RANLIB) $@

all: $(SLIB_PATH) test

# Remove active build configuration binary files
clean:
	@$(RM) $(OBJ_DIR)* $(OBJ_DIR) $(BIN_DIR)* $(BIN_DIR)

# Remove all built binary files
cleanall:
	@$(MAKE) clean BUILD_CONFIG=release
	@$(MAKE) clean BUILD_CONFIG=debug
	@$(RM) $(BUILD_DIR)* $(BUILD_DIR)

createFolders:
	@mkdir -p $(OBJ_DIR)

# Create file with settings for linking to external libraries
external:
	@echo '\
CPPFLAGS += $(EXT_CPPFLAGS) \r\n\
LDFLAGS += $(EXT_LDFLAGS) \
'> Makefile.external

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/LIB_NAME and
# library files are copied to DESTDIR/lib
install: $(SLIB_PATH)
#	@echo 'INSTALL $(DESTDIR)'
	@$(INSTALL) -d $(DESTDIR)
	@$(INSTALL) -d $(DESTDIR)lib
	@$(INSTALL) -d $(DESTDIR)include/$(LIB_NAME)
	@$(INSTALL) -c -m 644 $(SLIB_PATH) $(DESTDIR)lib
	@$(INSTALL) -c -m 644 $(INC_DIR)/*.h $(DESTDIR)include/$(LIB_NAME)
	@$(RANLIB) $(DESTDIR)lib/$(SLIB_FILE)

test: $(SLIB_PATH)
	@$(MAKE) -C $(TEST_DIR)

# Compile and run source files in example/ or test/ folder
example/%.cpp test/%.cpp: $(SLIB_PATH)
	@$(CXX) $(CFLAGS) -o $(BIN_DIR)$(*F) $@ $(LDFLAGS) $(SLIB_PATH)
ifneq ($(AUTORUN), 0)
	@$(BIN_DIR)$(*F) &
endif
