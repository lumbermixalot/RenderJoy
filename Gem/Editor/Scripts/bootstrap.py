if __name__ == "__main__":
    try:
        print("RenderJoy Gem will add 'Tools' -> 'RenderJoy Wizard' menu.")
        import az_qt_helpers
        #import azlmbr.editor as editor
        from shader_wizard_ui import ShaderWizardDialog
        # Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
        # options = editor.ViewPaneOptions()
        # options.showOnToolsToolbar = True
        # options.toolbarIcon = ":/${Name}/toolbar_icon.svg"
        az_qt_helpers.register_view_pane('RenderJoy Wizard', ShaderWizardDialog)
    except:
        print('Skipping registraion of RenderJoy Wizard menu with the Editor.')
