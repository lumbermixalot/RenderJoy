
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: ly_get_list_relative_pal_filename will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/RenderJoy/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/RenderJoy/Code
ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} ${o3de_gem_restricted_path} ${o3de_gem_path} ${o3de_gem_name})

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# If we are on a host platform, we want to add the host tools targets like the RenderJoy.Editor target which
# will also depend on RenderJoy.Static
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME RenderJoy.Editor.Static STATIC
        NAMESPACE Gem
        AUTOMOC
        AUTOUIC
        FILES_CMAKE
            renderjoy_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                .
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzCore
                AZ::AzFramework
                AZ::AzToolsFramework
                Gem::AtomToolsFramework.Static
                Gem::AtomToolsFramework.Editor
                Gem::Atom_RHI.Public
                Gem::Atom_RPI.Public
                Legacy::Editor.Headers
                Legacy::EditorLib
    )

    ly_add_target(
        NAME RenderJoy.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            renderjoy_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                Gem::RenderJoy.Editor.Static
    )

    # By default, we will specify that the above target RenderJoy would be used by
    # Tool and Builder type targets when this gem is enabled.  If you don't want it
    # active in Tools or Builders by default, delete one of both of the following lines:
    ly_create_alias(NAME RenderJoy.Tools    NAMESPACE Gem TARGETS Gem::RenderJoy.Editor)
    ly_create_alias(NAME RenderJoy.Builders NAMESPACE Gem TARGETS Gem::RenderJoy.Editor)


endif()

################################################################################
# Tests
################################################################################
# See if globally, tests are supported
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED)
    # If we are a host platform we want to add tools test like editor tests here
    if(PAL_TRAIT_BUILD_HOST_TOOLS)
        # We are a host platform, see if Editor tests are supported on this platform
        if(PAL_TRAIT_RENDERJOY_EDITOR_TEST_SUPPORTED)
            # We support RenderJoy.Editor.Tests on this platform, add RenderJoy.Editor.Tests target which depends on RenderJoy.Editor
            ly_add_target(
                NAME RenderJoy.Editor.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
                NAMESPACE Gem
                FILES_CMAKE
                    renderjoy_editor_tests_files.cmake
                INCLUDE_DIRECTORIES
                    PRIVATE
                        Tests
                        Source
                BUILD_DEPENDENCIES
                    PRIVATE
                        AZ::AzTest
                        Gem::RenderJoy.Editor
            )

            # Add RenderJoy.Editor.Tests to googletest
            ly_add_googletest(
                NAME Gem::RenderJoy.Editor.Tests
            )
        endif()
    endif()
endif()
