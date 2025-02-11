# See https://cmake-format.readthedocs.io/en/latest/configuration.html for all options

# -----------------------------
# Options affecting formatting.
# -----------------------------
with section("format"):
    # Disable formatting entirely, making cmake-format a no-op
    disable = False

    # How wide to allow formatted cmake files
    line_width = 135

    # How many spaces to tab for indent
    tab_size = 2

    # If a statement is wrapped to more than one line, than dangle the closing
    # parenthesis on its own line.
    dangle_parens = True

# ------------------------------------------------
# Options affecting comment reflow and formatting.
# ------------------------------------------------
with section("markup"):
    # enable comment markup parsing and reflow
    enable_markup = False

# ----------------------------
# Options affecting the linter
# ----------------------------
with section("lint"):

  # a list of lint codes to disable
  disabled_codes = ['C0111']
