include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "PortfolioRecommender")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "PortfolioRecommender")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Yield-targeted portfolio optimization with Beta minimization")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "PortfolioRecommender")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Portfolio Recommender")
    set(CPACK_NSIS_PACKAGE_NAME "Portfolio Recommender")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Portfolio Recommender.lnk' '$INSTDIR\\\\bin\\\\PortfolioRecommenderApp.exe'"
    )
endif()

include(CPack)
