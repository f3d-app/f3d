file(READ ${srcDir}/resources/cli-options.json CLI_OPTIONS_JSON)

configure_file(${srcDir}/application/F3DCLIOptions.h.in
  ${binaryDir}/F3DCLIOptions.h)
